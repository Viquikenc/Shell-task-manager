#ifndef __MENU_H
#define __MENU_H

#include <ncurses.h>
#include <string.h>
#include <sys/types.h>

#define INT_F "%d"
#define UINT64_F "%lu"
#define INT64_F "%ld"
#define STRING_F "%s"
#define CHAR_F "%c"
#define FLOAT_F "%.2f"
#define LONG_F "%ld"

#define uint128_t unsigned long long

#define NAME_SIZE 32
#define USER_SIZE 16
#define CMD_PATH_SIZE 256

typedef enum TableHeaderElementsMarginEnum {
  PID_MARG = 4,
  NAME_MARG = 18,
  USER_MARG = 5,
  PRI_MARG = 2,
  NI_MARG = 1,
  VIRT_MARG = 8,
  RES_MARG = 4,
  SHR_MARG = 4,
  S_MARG = 1,
  CPU_MARG = 2,
  MEM_MARG = 2,
  TIME_MARG = 5
} TableHeaderElementsMarginEnum;

typedef enum TableHeaderElementsEnum {
  PID,
  USER,
  NAME,
  PRI,
  NI,
  VIRT,
  RES,
  SHR,
  S,
  CPU,
  MEM,
  TIME,
  COMMAND,
  MAX
} TableHeaderElementsEnum;

typedef enum MaxTableElementsEnum {
  PID_MAX = PID_MARG + strlen("PID"),
  NAME__MAX = NAME_MARG + strlen("NAME"),
  USER_MAX = USER_MARG + strlen("USER"),
  PRI_MAX = PRI_MARG + strlen("PRI"),
  NI_MAX = NI_MARG + strlen("NI"),
  VIRT_MAX = VIRT_MARG + strlen("VIRT"),
  RES_MAX = RES_MARG + strlen("RES"),
  SHR_MAX = SHR_MARG + strlen("SHR"),
  S_MAX = S_MARG + strlen("S"),
  CPU_MAX = CPU_MARG + strlen("CPU%"),
  MEM_MAX = MEM_MARG + strlen("MEM%"),
  TIME_MAX = TIME_MARG + strlen("TIME")
} MaxTableElementsEnum;

typedef struct TableHeaderElementStruct {
  char *name;
  int str_size;
  int margin;
  TableHeaderElementsEnum pos;
} TableHeaderElementStruct;

typedef struct NewProccessElement {
  pid_t pid;                        // int
  char name[NAME_SIZE];             // string
  char user[USER_SIZE];             // string
  int64_t priority;                 // signed long
  int64_t nice;                     // signed long
  uint64_t virtualmem;              // unsigned long
  int64_t resident;                 // signed long
  uint64_t sharemem;                // unsigned
  char state;                       // char
  float cpu;                        // float
  float mem;                        // float
  time_t time;                      // long
  char command_path[CMD_PATH_SIZE]; // string
} NewProccessElement;

int GetUserFromUid(const pid_t uid, char user[USER_SIZE]);

int GetSharedMemSize(unsigned long *sharedmem, const pid_t process_id);

int GetProcessCPUusage(float *cpu_usage, const time_t utime, const time_t stime,
                       const time_t cutime, const time_t cstime,
                       const uint128_t starttime);

void GetProcessRAMusage(float *ram_usage,
                                      const uint64_t resident);

int GetProcessInfoFromFile(NewProccessElement *Process, const pid_t pid);

int WinCreateProccessItem(WINDOW *win, uint16_t xpos, const uint16_t ypos,
                          const NewProccessElement ProccessElement);

int GetProcessFullPath(const pid_t pid, char *exe_path);

#endif
