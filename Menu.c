#include <pwd.h>
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
  char format[5];
  char order;
  NewProccessElement process_info;
} ProccessProperties;

// Gets the username from the given uid
int GetUserFromUid(const pid_t uid, char user[16]) {
  struct passwd *pwd = getpwuid(uid);
  if (pwd) {
    strncpy(user, pwd->pw_name, USER_SIZE);
    return SUCCESS;
  } else {
    ERR_SET(ERR_UNKNOWN);
    return ERR_UNKNOWN;
  }
}

int GetSharedMemSize(unsigned long *sharedmem, const pid_t process_id) {
  char path[256];
  FILE *file;
  (void)snprintf(path, sizeof(path), "/proc/%d/statm", process_id);
  if ((file = fopen(path, "r"))) {
    if (!fscanf(file, "%*d %*d %lu", sharedmem)) {
      ERR_SET(ERR_SCAN_FILE);
      return ERR_SCAN_FILE;
    } else {
      fclose(file);
      return SUCCESS;
    }
  } else {
    ERR_SET(ERR_OPEN_FILE);
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
  if ((file = fopen("/proc/uptime", "r")) == SUCCESS) {
    if (fscanf(file, "%f", &uptime) != SUCCESS) {
      ERR_SET(ERR_SCAN_FILE);
      return ERR_SCAN_FILE;
    }
  } else {
    ERR_SET(ERR_OPEN_FILE);
    return ERR_OPEN_FILE;
  }
  fclose(file);
  float seconds = uptime - ((float)starttime / Hertz);
  *cpu_usage = 100 * (((float)total_time / Hertz) / seconds);
  return SUCCESS;
}

int GetProcessFullPath(const pid_t pid, char *exe_path) {
  char process_path[CMD_PATH_SIZE];
  snprintf(process_path, sizeof(process_path), "/proc/%d/exe", pid);
  ssize_t len = readlink(process_path, exe_path, CMD_PATH_SIZE - 1);
  switch (len) {
  case EIO:
    ERR_SET(ERR_READ_FILE);
    return ERR_READ_FILE;
    break;
  case ENOENT:
    ERR_SET(ERR_NOT_EXIST_FILE);
    return ERR_NOT_EXIST_FILE;
    break;
  case -1:
    break;
  default:
    exe_path[len] = '\0';
    break;
  }
  return SUCCESS;
}

int GetProcessRAMusage(float *ram_usage, const pid_t pid,
                       const uint64_t resident) {
  FILE *total_mem_file;
  uint64_t total_mem;
  if ((total_mem_file = fopen("/proc/meminfo", "r")) == SUCCESS) {
    if (fscanf(total_mem_file, "%lu", &total_mem) == SUCCESS)
      *ram_usage = 100 * ((float)resident / (float)total_mem);
    else {
      ERR_SET(ERR_SCAN_FILE);
      return ERR_SCAN_FILE;
    }
  } else {
    ERR_SET(ERR_OPEN_FILE);
    return ERR_OPEN_FILE;
  }
  fclose(total_mem_file);
  return SUCCESS;
}

/* this shit is for getting all the information about a process (ex. pid,
 * ram_usage, ...)*/
int GetProcessInfoFromFile(NewProccessElement *Process, pid_t pid) {
  pid_t process_uid;
  char path[120];
  unsigned long stime;
  unsigned long utime;
  time_t cutime;
  time_t cstime;
  uint128_t starttime;
  snprintf(path, sizeof(path), "/proc/%d/stat", pid);
  FILE *pid_file;
  if ((pid_file = fopen(path, "r"))) {
    int throw = fscanf(
        pid_file,
        "%d %s %c %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu %lu "
        "%ld %ld %ld %ld %*d %*d %llu %lu %ld",
        &Process->pid, Process->name, &Process->state, &process_uid, &utime,
        &stime, &cutime, &cstime, &Process->priority, &Process->nice,
        &starttime, &Process->virtualmem, &Process->resident);
    if (throw != EOF) {
      Process->time = utime + stime;
      if (GetUserFromUid(process_uid, Process->user) != SUCCESS)
        return ERR_UNKNOWN;
      if (GetSharedMemSize(&Process->sharemem, Process->pid) != SUCCESS)
        return ERR_UNKNOWN;
      if (GetProcessFullPath(Process->pid, Process->command_path) != SUCCESS)
        return ERR_UNKNOWN;
      if (GetProcessCPUusage(&Process->cpu, utime, stime, cutime, cstime,
                             starttime) != SUCCESS)
        return ERR_UNKNOWN;
      if (GetProcessRAMusage(&Process->mem, Process->pid, Process->resident) !=
          SUCCESS)
        return ERR_UNKNOWN;
    } else {
      ERR_SET(ERR_SCAN_FILE);
      return ERR_SCAN_FILE;
    }
  } else {
    ERR_SET(ERR_OPEN_FILE);
    return ERR_OPEN_FILE;
  }
  fclose(pid_file);
  return SUCCESS;
}

/* this is for displaying the process in your shit screen, this is one of the
 * reasons why GUI shouldn't exist */
int WinCreateProccessItem(WINDOW *win, uint16_t xpos, const uint16_t ypos,
                          NewProccessElement ProccessElement) {
  /* an array that stores all the properties in one array so you could access
   * easily anything about a process */
  ProccessProperties process[_COMMAND + 1] = {
      {UINT32_F, _PID, {.pid = ProccessElement.pid}},
      {STRING_F, _NAME},
      {STRING_F, _USER},
      {INT8_F, _PRI, {.priority = ProccessElement.priority}},
      {INT8_F, _NI, {.nice = ProccessElement.nice}},
      {UINT64_F, _VIRT, {.virtualmem = ProccessElement.virtualmem}},
      {UINT64_F, _RES, {.resident = ProccessElement.resident}},
      {UINT64_F, _SHR, {.sharemem = ProccessElement.sharemem}},
      {CHAR_F, _S, {.state = ProccessElement.state}},
      {FLOAT_F, _CPU, {.cpu = ProccessElement.cpu}},
      {FLOAT_F, _MEM, {.mem = ProccessElement.mem}},
      {LONG_F, _TIME, {.time = ProccessElement.time}},
      {STRING_F, _COMMAND}};

  strncpy(process[_NAME].process_info.name, ProccessElement.name,
          NAME_SIZE - 1);
  strncpy(process[_USER].process_info.user, ProccessElement.user,
          USER_SIZE - 1);
  strncpy(process[_COMMAND].process_info.command_path,
          ProccessElement.command_path, CMD_PATH_SIZE);

  // WARN: this only for testing and showing the necessary information, should
  // be DELETED after

  // printf(
  //     "\n pid: %d name: %s user: %s pri: %ld ni: %ld virt: %lu res: %ld shr:
  //     "
  //     "%lu s: %c cpu: %.1f mem: %.1f time: %ld cmd: %s the new cmd : %s\n",
  //     ProccessElement.pid, ProccessElement.name, ProccessElement.user,
  //     ProccessElement.priority, ProccessElement.nice,
  //     ProccessElement.virtualmem, ProccessElement.resident,
  //     ProccessElement.sharemem, ProccessElement.state, ProccessElement.cpu,
  //     ProccessElement.mem, ProccessElement.time,
  //     ProccessElement.command_path,
  //     process[_COMMAND].process_info.command_path);

  refresh();
  for (short i = _PID; i <= _COMMAND; ++i) {
    // FIXME: change the inaccurate coordination for printing a
    // process in the screen
    mvwprintw(win, xpos, ypos, process[i].format, process[i].process_info);
    xpos += 8;
  }
  wrefresh(win);
  refresh();
  return SUCCESS;
}
