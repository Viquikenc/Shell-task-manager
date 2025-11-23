#include <dirent.h>
#include <ncurses.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define UINT32_F "%u"
#define UINT64_F "%lu"
#define STRING_F "%s"
#define CHAR_F "%c"
#define INT8_F "%hhd"
#define UINT16_F "%hu"
#define FLOAT_F "%.1f"
#define LONG_F "%ld"

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

typedef struct NewProccessElement {
  pid_t pid;              //  int
  char name[32];          // string
  char user[16];          // string
  int64_t priority;       // signed long
  int64_t nice;           // signed long
  uint64_t virtualmem;    // unsigned long
  int64_t resident;       // signed long
  uint64_t sharemem;      // unsigned
  char state;             // char
  float cpu;              // float
  float mem;              // float
  time_t time;            // long
  char command_path[256]; // string
} NewProccessElement;

typedef struct ProccessProperties {
  char format[5];
  char order;
  NewProccessElement process_info;
} ProccessProperties;

// Gets the username from the given uid
int GetUserFromUid(const pid_t uid, char user[16]) {
  struct passwd *pwd = getpwuid(uid);
  if (pwd) {
    strcpy(user, pwd->pw_name);
    return 0;
  } else {
    return -3;
  }
  return 0;
}

int GetSharedMemSize(unsigned long *sharedmem, const pid_t process_id) {
  char path[256];
  FILE *file;
  printw("I get to here finallyy!!");
  refresh();
  snprintf(path, sizeof(path), "/proc/%d/statm", process_id);
  if ((file = fopen(path, "r")))
    fscanf(file, "%*d %*d %lu", sharedmem);
  else {
    return -4;
  }
  fclose(file);
  return 0;
}

int GetProcessCPUusage(float *cpu_usage, const time_t utime, const time_t stime,
                       const time_t cutime, const time_t cstime,
                       const uint128_t starttime) {
  FILE *file;
  float uptime;
  const int64_t Hertz = sysconf(_SC_CLK_TCK);
  uint64_t total_time = cutime + cstime + utime + stime;
  if ((file = fopen("/proc/uptime", "r")))
    fscanf(file, "%f", &uptime);
  else
    return -2;
  fclose(file);
  float seconds = uptime - ((float)starttime / Hertz);
  *cpu_usage = 100 * (((float)total_time / Hertz) / seconds);
  return 0;
}

int GetProcessFullPath(const char *process_path, char exe_path[256]) {
  const uint16_t buf_size = 128;
  if (readlink(process_path, exe_path, buf_size))
    return 0;
  else
    return -14;
  return 0;
}

int GetProcessRAMusage(float *ram_usage, const pid_t pid,
                       const uint64_t resident) {
  FILE *total_mem_file;
  uint64_t total_mem;
  if ((total_mem_file = fopen("/proc/meminfo", "r"))) {
    fscanf(total_mem_file, "%lu", &total_mem);
    *ram_usage = 100 * ((float)resident / (float)total_mem);
  } else
    return -4;
  fclose(total_mem_file);
  return 0;
}

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
    fscanf(pid_file,
           "%d %s %c %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu %lu "
           "%ld %ld %ld %ld %*d %*d %llu %lu %ld",
           &Process->pid, Process->name, &Process->state, &process_uid, &utime,
           &stime, &cutime, &cstime, &Process->priority, &Process->nice,
           &starttime, &Process->virtualmem, &Process->resident);
    Process->time = utime + stime;
    GetUserFromUid(process_uid, Process->user);
    GetSharedMemSize(&Process->sharemem, pid);
    GetProcessFullPath(path, Process->command_path);
    GetProcessCPUusage(&Process->cpu, utime, stime, cutime, cstime, starttime);
    GetProcessRAMusage(&Process->mem, pid, Process->resident);
  } else {
    perror("Error");
    return -4;
  }
  fclose(pid_file);
  return 0;
}

int WinCreateProccessItem(WINDOW *win, uint16_t xpos, const uint16_t ypos,
                          NewProccessElement ProccessElement) {
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
  strcpy(process[_NAME].process_info.name, ProccessElement.name);
  strcpy(process[_USER].process_info.user, ProccessElement.user);
  strcpy(process[_COMMAND].process_info.command_path,
         ProccessElement.command_path);
  printw(
      "\n pid: %d name: %s user: %s pri: %ld ni: %ld virt: %lu res: %ld shr: "
      "%lu s: %c cpu: %.1f mem: %.1f time: %ld cmd: %s",
      ProccessElement.pid, ProccessElement.name, ProccessElement.user,
      ProccessElement.priority, ProccessElement.nice,
      ProccessElement.virtualmem, ProccessElement.resident,
      ProccessElement.sharemem, ProccessElement.state, ProccessElement.cpu,
      ProccessElement.mem, ProccessElement.time, ProccessElement.command_path);
  refresh();
  for (short i = _PID; i <= _COMMAND; ++i) {
    mvwprintw(win, xpos, ypos, process[i].format, process[i].process_info);
    xpos += 8;
  }
  wrefresh(win);
  refresh();
  return 0;
}
