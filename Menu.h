#ifndef __MENU_H
#define __MENU_H

#include <ncurses.h>
#include <sys/types.h>

#define UINT32_F "%u"
#define UINT64_F "%lu"
#define STRING_F "%s"
#define CHAR_F "%c"
#define INT8_F "%hhd"
#define UINT16_F "%hu"
#define FLOAT_F "%f"
#define LONG_F "%ld"

#define uint128_t unsigned long long

typedef struct NewProccessElement {
  uint32_t pid;        // unsigned int
  char *name;          // string
  char *user;          // string
  int64_t priority;    // signed char
  int64_t nice;        // signed char
  uint64_t virtualmem; // unsigned long
  int64_t resident;    // unsigned long
  uint64_t sharemem;   // unsigned short
  char state;          // char
  float cpu;           // float
  float mem;           // float
  time_t time;         // long
  char *command_path;  // string
} NewProccessElement;

int GetUserFromUid(uid_t uid, char **user);

int GetSharedMemSize(unsigned long *sharedmem, const pid_t process_id);

int GetProcessCPUusage(float *cpu_usage, const time_t utime, const time_t stime,
                       const time_t cutime, const time_t cstime,
                       const uint128_t starttime);

int GetProcessRAMusage(float *ram_usage, const pid_t pid,
                       const uint64_t resident);

int GetProcessInfoFromFile(NewProccessElement *Process, pid_t pid);

int WinCreateProccessItem(WINDOW *win, uint16_t xpos, const uint16_t ypos,
                          const NewProccessElement ProccessElement);

int GetProcessFullPath(const char *process_path, char **exe_path);

#endif
