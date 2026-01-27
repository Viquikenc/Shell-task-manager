#ifndef __MENU_H
#define __MENU_H

#include <ncurses.h>
#include <string.h>
#include <sys/types.h>

#define PID_F "%d"
#define NAME_F "%s"
#define USER_F "%s"
#define PRIORITY_F "%ld"
#define NICE_F "%ld"
#define VIRTUALMEM_F "%lu"
#define RESIDENT_F "%ld"
#define SHAREMEM_F "%u"
#define STATE_F "%c"
#define CPU_F "%.2f"
#define MEM_F "%.2f"
#define TIME_F "%ld"
#define COMMAND_PATH_F "%s"

#define NUM_FLAG (1 << 0)
#define STR_FLAG (1 << 1)

#define NAME_SIZE 32
#define USER_SIZE 16
#define CMD_PATH_SIZE 64
#define FORMAT_SIZE 32

#define CHECK_NULL_(data)                                                      \
  do {                                                                         \
    if ((data) == NULL) {                                                      \
      return NULL;                                                             \
    }                                                                          \
  } while (0)

typedef enum TableHeaderElementsMarginEnum {
  PID_MARG = 4,
  NAME_MARG = 18,
  USER_MARG = 5,
  PRI_MARG = 2,
  NI_MARG = 1,
  VIRT_MARG = 8,
  RES_MARG = 5,
  SHR_MARG = 4,
  S_MARG = 1,
  CPU_MARG = 2,
  MEM_MARG = 2,
  TIME_MARG = 5
} TableHeaderElementsMarginEnum;

typedef enum TableHeaderElementsEnum {
  PID,
  NAME,
  USER,
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

typedef enum ProcessDataEnum {
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
  _MAX
} ProcessDataEnum;

typedef struct TableHeaderElementStruct {
  char *name;
  int str_size;
  int margin;
  TableHeaderElementsEnum pos;
} TableHeaderElementStruct;

typedef struct ProcStatProperties {
  union {
    pid_t pid_type;                        // int
    char name_type[NAME_SIZE];             // string
    char user_type[USER_SIZE];             // string
    int64_t priority_type;                 // signed long
    int64_t nice_type;                     // signed long
    uint64_t virtualmem_type;              // unsigned long
    int64_t resident_type;                 // signed long
    uint64_t sharemem_type;                // unsigned
    char state_type;                       // char
    float cpu_type;                        // float
    float mem_type;                        // float
    time_t time_type;                      // long
    char command_path_type[CMD_PATH_SIZE]; // string
  };
  char format[FORMAT_SIZE];
  void *data;
} ProcStatProperties;

typedef struct Process {
  ProcStatProperties pid;          // int
  ProcStatProperties name;         // string
  ProcStatProperties user;         // string
  ProcStatProperties priority;     // signed long
  ProcStatProperties nice;         // signed long
  ProcStatProperties virtualmem;   // unsigned long
  ProcStatProperties resident;     // signed long
  ProcStatProperties sharemem;     // unsigned
  ProcStatProperties state;        // char
  ProcStatProperties cpu;          // float
  ProcStatProperties mem;          // float
  ProcStatProperties time;         // long
  ProcStatProperties command_path; // string
} Process;

int GetUserFromUid(const pid_t uid, char user[USER_SIZE]);

int GetProcessInfoFromFile(Process **, const pid_t pid);

void ProcessMemCpy(Process *from, Process **to);

Process *InitProcess();

void FreeProcess(Process *);

void PrintProcessItem(WINDOW *, const Process, const int at_y);

void CreateProcessItem(Process ProccessElement, const int print_y,
                       const int print_x);

#endif
