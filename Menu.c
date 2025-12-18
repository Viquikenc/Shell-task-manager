#include <ncurses.h>
#include <pwd.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "Menu.h"
#include "error_handler.h"

#define BASE_10 10

#define uint128_t unsigned long long

union DataType {
  long num_val;
  char *string_val;
};

enum DataEnum { NUM, STRING };

struct DataStruct {
  enum DataEnum DATA;
  union DataType data;
};

typedef enum ProcessElementsEnum {
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
  _COMMAND
} ProcessElementsEnum;

typedef struct ProccessProperties {
  char format[8];
  char order;
  NewProccessElement process_info;
} ProccessProperties;

/* Gets the user name of a process from the given uid */
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

/* it gets the shared memory of process process_pid and puts the result in
 * sharedmem */
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

/* it gets how much cpu does a process uses and puts it in cpu_usage in
 * percentage. And if you wonder why it takes that much parameters, actually I
 * have no idea, because it turns out you need so much infomation about time
 * and cpu clock cycles and some other stuff and I recommand to not waste your
 * time understanding it to just calculate the percentage in cpu_usage of a
 * certain process */
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

/* it gets the full executable of a process with its flags from pid and puts
 * result in exe_path*/
int GetProcessFullExcutable(const pid_t pid, char *exe_path) {
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

/* it gets how much ram does a process uses and puts it in ram_usage in
 * percentage */
void GetProcessRAMusage(float *ram_usage, const uint64_t resident) {

  extern uint64_t total_mem;
  *ram_usage = 100 * ((float)resident / total_mem);
}

/* this shit is for getting all the information about a process (ex. pid,
 * ram_usage, ...) from /proc/pid/stat */
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
      if (GetProcessFullExcutable(Process->pid, Process->command_path) !=
          SUCCESS) {
        fclose(pid_file);
        return ERR_UNKNOWN;
      }
      if (GetProcessCPUusage(&Process->cpu, utime, stime, cutime, cstime,
                             starttime) != SUCCESS) {
        fclose(pid_file);
        return ERR_UNKNOWN;
      }
      GetProcessRAMusage(&Process->mem, Process->resident);
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

/* it takes your stupid variable which you type its properties in the data
 * struct and then calculates how many characters in it (also with numbers) */
static void GetNumOfDigits(const struct DataStruct data, uint8_t *num_digit) {
  *num_digit = 1;
  long result;
  switch (data.DATA) {
  case STRING:
    *num_digit = (uint8_t)strlen(data.data.string_val);
    break;
  case NUM:
    result = data.data.num_val;
    while (result > 9 || result < -9) {
      result /= 10;
      ++*num_digit;
    }
    *num_digit += ((data.data.num_val < 0) ? 1 : 0);
    break;
  }
}

/* this is for displaying the process in your shit screen, this is one of the
   reasons why GUI shouldn't exist */
int WinCreateProccessItem(WINDOW *win, uint16_t xpos, const uint16_t ypos,
                          NewProccessElement ProccessElement) {
  /* an array that stores all the properties in one array so you could access
    easily anything about a process */
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

  /* since I can't pass the strings into the array directly due to C weird
   rules, I am copying them to struct elment in the array */

  (void)strncpy(process[_NAME].process_info.name, ProccessElement.name,
                NAME_SIZE - 1);
  (void)strncpy(process[_USER].process_info.user, ProccessElement.user,
                USER_SIZE - 1);
  (void)strncpy(process[_COMMAND].process_info.command_path,
                ProccessElement.command_path, CMD_PATH_SIZE - 1);
  for (short i = _PID; i <= _COMMAND; ++i) {
    uint8_t digit_len = 0;
    /* this is a switch where it when it enters to a certain case (_PID, _NAME,
     _USER, ...) it does some calculation to make the curses ended up pointing
     exactly at the beginning of the next property of the process */
    switch (i) {
    case _PID:
      GetNumOfDigits(
          (struct DataStruct){
              NUM,
              (union DataType){.num_val = (long)process[i].process_info.pid}},
          &digit_len);
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.pid);
      xpos = getcurx(win) + (PID_MAX - digit_len);
      break;
    case _NAME:
      GetNumOfDigits(
          (struct DataStruct){
              STRING,
              (union DataType){.string_val = process[i].process_info.name}},
          &digit_len);
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.name);
      xpos = getcurx(win) + (NAME__MAX - digit_len);
      break;
    case _USER:
      GetNumOfDigits(
          (struct DataStruct){
              STRING,
              (union DataType){.string_val = process[i].process_info.user}},
          &digit_len);
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.user);
      xpos = getcurx(win) + (USER_MAX - digit_len);
      break;
    case _PRI:
      GetNumOfDigits(
          (struct DataStruct){
              NUM,
              (union DataType){.num_val =
                                   (long)process[i].process_info.priority}},
          &digit_len);
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.priority);
      xpos = getcurx(win) + (PRI_MAX - digit_len);
      break;
    case _NI:
      GetNumOfDigits(
          (struct DataStruct){
              NUM,
              (union DataType){.num_val = (long)process[i].process_info.nice}},
          &digit_len);
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.nice);
      xpos = getcurx(win) + (NI_MAX - digit_len);
      break;
    case _VIRT:
      GetNumOfDigits(
          (struct DataStruct){
              NUM,
              (union DataType){.num_val =
                                   (long)process[i].process_info.virtualmem}},
          &digit_len);
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.virtualmem);
      xpos = getcurx(win) + (VIRT_MAX - digit_len);
      break;
    case _RES:
      GetNumOfDigits(
          (struct DataStruct){
              NUM,
              (union DataType){.num_val = process[i].process_info.resident}},
          &digit_len);
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.resident);
      xpos = getcurx(win) + (RES_MAX + 1 - digit_len);
      break;
    case _SHR:
      GetNumOfDigits(
          (struct DataStruct){
              NUM,
              (union DataType){.num_val =
                                   (long)process[i].process_info.sharemem}},
          &digit_len);
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.sharemem);
      xpos = getcurx(win) + (SHR_MAX - digit_len);
      break;
    case _S:
      digit_len = 1;
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.state);
      xpos = getcurx(win) + (S_MAX - digit_len);
      break;
    case _CPU:
      digit_len = 4;
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.cpu);
      xpos = getcurx(win) + (CPU_MAX - digit_len);
      break;
    case _MEM:
      digit_len = 4;
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.mem);
      xpos = getcurx(win) + (MEM_MAX - digit_len);
      break;
    case _TIME:
      GetNumOfDigits(
          (struct DataStruct){
              NUM,
              (union DataType){.num_val = (long)process[i].process_info.time}},
          &digit_len);
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.time);
      xpos = getcurx(win) + (TIME_MAX - digit_len);
      break;
    case _COMMAND:
      mvwprintw(win, ypos, xpos, process[i].format,
                process[i].process_info.command_path);
      xpos = getcurx(win);
      break;
    }
  }
  return SUCCESS;
}
