#include <ncurses.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Menu.h"
#include "error_handler.h"

#define BASE_10 10

#define uint128_t unsigned long long

typedef enum TableHeaderElementsEnum {
  PID = 3,
  NAME = 4,
  USER = 4,
  PRI = 3,
  NI = 2,
  VIRT = 4,
  RES = 3,
  SHR = 3,
  S = 1,
  CPU = 3,
  MEM = 3,
  TIME = 4,
  COMMAND = 7,
} TableHeaderElementsEnum;

typedef enum ProccessElementsEnum {
  _PID,
  _NAME,
  _USER,
  _PRI,
  _NI,
  _VIRT,
  _RES,
  _SHR,
  _S,
  _CPU,
  _MEM,
  _TIME,
  _COMMAND,
} ProccessElementsEnum;

typedef struct ProccessProperties {
  char format[8];
  char order;
  NewProccessElement process_info;
} ProccessProperties;

// Gets the username from the given uid
int GetUserFromUid(const pid_t uid, char user[USER_SIZE]) {
  struct passwd *pwd = getpwuid(uid);
  if (pwd != NULL) {
    strncpy(user, pwd->pw_name, USER_SIZE - 1);
    user[USER_SIZE] = '\0';
    return SUCCESS;
  } else {
    ERR_SET(ERR_UNKNOWN, IGNORED);
    free(pwd);
    return ERR_UNKNOWN;
  }
}

int GetSharedMemSize(unsigned long *sharedmem, const pid_t process_id) {
  char path[32];
  FILE *file;
  (void)snprintf(path, sizeof(path), "/proc/%d/statm", process_id);
  if ((file = fopen(path, "r")) != NULL) {
    if (fscanf(file, "%*d %*d %lu", sharedmem) == EOF) {
      ERR_SET(ERR_SCAN_FILE, WARNING);
      fclose(file);
      return ERR_SCAN_FILE;
    } else {
      fclose(file);
      return SUCCESS;
    }
  } else {
    ERR_SET(ERR_OPEN_FILE, WARNING);
    return ERR_OPEN_FILE;
  }
}

int GetProcessCPUusage(float *cpu_usage, const time_t utime, const time_t stime,
                       const time_t cutime, const time_t cstime,
                       const uint128_t starttime) {
  FILE *file;
  float uptime;
  const int64_t Hertz = sysconf(_SC_CLK_TCK);
  uint64_t total_time = cutime + cstime + utime + stime;
  if ((file = fopen("/proc/uptime", "r")) != NULL) {
    if (fscanf(file, "%f", &uptime) == EOF) {
      fclose(file);
      ERR_SET(ERR_SCAN_FILE, WARNING);
      return ERR_SCAN_FILE;
    }
    fclose(file);
  } else {
    ERR_SET(ERR_OPEN_FILE, WARNING);
    return ERR_OPEN_FILE;
  }
  float seconds = uptime - ((float)starttime / (float)Hertz);
  *cpu_usage = 100 * (((float)total_time / (float)Hertz) / (float)seconds);
  return SUCCESS;
}

int GetProcessFullPath(const pid_t pid, char *exe_path) {
  char process_path[CMD_PATH_SIZE];
  (void)snprintf(process_path, sizeof(process_path), "/proc/%d/cmdline", pid);
  errno = 0;
  FILE *process_path_cmd = fopen(process_path, "r");
  if (process_path_cmd != NULL) {
    size_t readen_bytes =
        fread(process_path, 1, sizeof(process_path), process_path_cmd);
    if (readen_bytes > 0) {
      for (size_t i = 0; i < readen_bytes; i++)
        exe_path[i] = (process_path[i] == '\0') ? ' ' : process_path[i];
      exe_path[readen_bytes] = '\0';
      fclose(process_path_cmd);
      return SUCCESS;
    } else {
      fclose(process_path_cmd);
      ERR_SET(ERR_UNKNOWN, WARNING);
      return ERR_UNKNOWN;
    }
  } else {
    ERR_SET(ERR_UNKNOWN, IGNORED);
    return ERR_UNKNOWN;
  }
}

inline int GetProcessRAMusage(float *ram_usage, const pid_t pid,
                              const uint64_t resident) {
  FILE *total_mem_file;
  uint64_t total_mem;
  if ((total_mem_file = fopen("/proc/meminfo", "r")) != NULL) {
    if (fscanf(total_mem_file, "%lu", &total_mem) != EOF) {
      *ram_usage = 100.0 * ((float)resident / total_mem);
      fclose(total_mem_file);
      return SUCCESS;
    } else {
      ERR_SET(ERR_SCAN_FILE, WARNING);
      fclose(total_mem_file);
      return ERR_SCAN_FILE;
    }
  } else {
    ERR_SET(ERR_OPEN_FILE, WARNING);
    return ERR_OPEN_FILE;
  }
}

/* this shit is for getting all the information about a process (ex. pid,
 * ram_usage, ...)*/
int GetProcessInfoFromFile(NewProccessElement *Process, const pid_t pid) {
  pid_t process_uid;
  char path[120];
  unsigned long stime;
  unsigned long utime;
  time_t cutime;
  time_t cstime;
  uint128_t starttime;
  (void)snprintf(path, sizeof(path), "/proc/%d/stat", pid);
  FILE *pid_file;
  if ((pid_file = fopen(path, "r")) != NULL) {
    int throw = fscanf(
        pid_file,
        "%d %s %c %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu %lu "
        "%ld %ld %ld %ld %*d %*d %llu %lu %ld",
        &Process->pid, Process->name, &Process->state, &process_uid, &utime,
        &stime, &cutime, &cstime, &Process->priority, &Process->nice,
        &starttime, &Process->virtualmem, &Process->resident);
    Process->name[NAME_SIZE - 1] = '\0';
    if (throw != EOF) {
      Process->time = utime + stime;
      if (GetUserFromUid(process_uid, Process->user) != SUCCESS) {
        fclose(pid_file);
        return ERR_UNKNOWN;
      }
      if (GetSharedMemSize(&Process->sharemem, Process->pid) != SUCCESS) {
        fclose(pid_file);
        return ERR_UNKNOWN;
      }
      if (GetProcessFullPath(Process->pid, Process->command_path) != SUCCESS) {
        fclose(pid_file);
        return ERR_UNKNOWN;
      }
      if (GetProcessCPUusage(&Process->cpu, utime, stime, cutime, cstime,
                             starttime) != SUCCESS) {
        fclose(pid_file);
        return ERR_UNKNOWN;
      }
      if (GetProcessRAMusage(&Process->mem, Process->pid, Process->resident) !=
          SUCCESS) {
        fclose(pid_file);
        return ERR_UNKNOWN;
      }
      fclose(pid_file);
      return SUCCESS;
    } else {
      ERR_SET(ERR_SCAN_FILE, WARNING);
      fclose(pid_file);
      return ERR_SCAN_FILE;
    }
  } else {
    ERR_SET(ERR_OPEN_FILE, WARNING);
    return ERR_OPEN_FILE;
  }
}

/* this is for displaying the process in your shit screen, this is one of the
 * reasons why GUI shouldn't exist */
int WinCreateProccessItem(WINDOW *win, uint16_t xpos, const uint16_t ypos,
                          NewProccessElement ProccessElement) {
  /* an array that stores all the properties in one array so you could access
   * easily anything about a process */
  ProccessProperties process[_COMMAND + 1] = {
      {INT_F, _PID, {.pid = ProccessElement.pid}},
      {STRING_F, _NAME},
      {STRING_F, _USER},
      {INT64_F, _PRI, {.priority = ProccessElement.priority}},
      {INT64_F, _NI, {.nice = ProccessElement.nice}},
      {UINT64_F, _VIRT, {.virtualmem = ProccessElement.virtualmem}},
      {INT64_F, _RES, {.resident = ProccessElement.resident}},
      {UINT64_F, _SHR, {.sharemem = ProccessElement.sharemem}},
      {CHAR_F, _S, {.state = ProccessElement.state}},
      {FLOAT_F, _CPU, {.cpu = ProccessElement.cpu}},
      {FLOAT_F, _MEM, {.mem = ProccessElement.mem}},
      {LONG_F, _TIME, {.time = ProccessElement.time}},
      {STRING_F, _COMMAND}};

  (void)strncpy(process[_NAME].process_info.name, ProccessElement.name,
                NAME_SIZE - 1);
  (void)strncpy(process[_USER].process_info.user, ProccessElement.user,
                USER_SIZE - 1);
  (void)strncpy(process[_COMMAND].process_info.command_path,
                ProccessElement.command_path, CMD_PATH_SIZE - 1);
  process[_COMMAND].process_info.command_path[CMD_PATH_SIZE - 1] = '\0';
  for (short i = _PID; i <= _COMMAND; ++i) {
    // FIXME: change the inaccurate coordination for printing a
    // process in the screen
    switch (i) {
    case _PID:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.pid);
      xpos = getcurx(win);
      break;
    case _NAME:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.name);
      xpos = getcurx(win);
      break;
    case _USER:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.user);
      xpos = getcurx(win);
      break;
    case _PRI:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.priority);
      xpos = getcurx(win);
      break;
    case _NI:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.nice);
      xpos = getcurx(win);
      break;
    case _VIRT:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.virtualmem);
      xpos = getcurx(win);
      break;
    case _RES:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.resident);
      xpos = getcurx(win);
      break;
    case _SHR:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.sharemem);
      xpos = getcurx(win);
      break;
    case _S:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.state);
      xpos = getcurx(win);
      break;
    case _CPU:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.cpu);
      xpos = getcurx(win);
      break;
    case _MEM:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.mem);
      xpos = getcurx(win);
      break;
    case _TIME:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.time);
      xpos = getcurx(win);
      break;
    case _COMMAND:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.command_path);
      xpos = getcurx(win);
      break;
    }
    xpos += 2;
  }
  return SUCCESS;
}
