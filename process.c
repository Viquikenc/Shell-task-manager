#include <ncurses.h>
#include <pwd.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "error_handler.h"
#include "process.h"

#define BASE_10 10

/* Inspecting the user database and stores process's user in [user] with the
 * id of [uid]
 */
int GetUserFromUid(const pid_t uid, char user[USER_SIZE]) {
  struct passwd *pwd = getpwuid(uid);
  if (pwd == NULL) {
    ERR_SET(ERR_UNKNOWN, IGNORED);
    return ERR_UNKNOWN;
  }
  strncpy(user, pwd->pw_name, USER_SIZE - 1);
  return SUCCESS;
}

/* Obtains the shared memory of [process_id] and stores the result in
 * [sharedmem]. Returns 0 on success
 * */
static int GetSharedMemSize(unsigned long *sharedmem, const pid_t process_id) {
  char path[32];
  FILE *file;
  (void)snprintf(path, sizeof(path), "/proc/%d/statm", process_id);
  if ((file = fopen(path, "r")) == NULL) {
    ERR_SET(ERR_OPEN_FILE, WARNING);
    return ERR_OPEN_FILE;
  }
  if (fscanf(file, "%*d %*d %lu", sharedmem) == EOF) {
    ERR_SET(ERR_SCAN_FILE, WARNING);
    fclose(file);
    return ERR_SCAN_FILE;
  }
  fclose(file);
  return SUCCESS;
}

/* Measures cpu usage time by a processand stores it in [cpu_usage] in
 * percentage (0 - 100), with the need of so much infomation ([utime], [stime],
 * [cutime], [cstime]) about time and cpu clock cycles and some other stuff
 * which I recommand to not waste your time understanding it. Returns 0 on
 * success
 */
static int GetProcessCPUusage(float *cpu_usage, const time_t utime,
                              const time_t stime, const time_t cutime,
                              const time_t cstime, const uint64_t starttime) {
  FILE *file;
  float uptime = 0.0;
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

/* Obtains the full executable path with its flage from [pid] and stores
 * it in [exe_path]. It returns 0 on success
 */
static int GetProcessFullExcutable(const pid_t pid, char *exe_path) {
  char process_path[CMD_PATH_SIZE];
  (void)snprintf(process_path, sizeof(process_path), "/proc/%d/cmdline", pid);
  errno = 0;
  FILE *process_path_cmd = fopen(process_path, "r");
  if (process_path_cmd == NULL) {
    ERR_SET(ERR_UNKNOWN, IGNORED);
    return ERR_UNKNOWN;
  }
  size_t readed_bytes =
      fread(process_path, 1, sizeof(process_path), process_path_cmd);
  if (readed_bytes > 0) {
    for (size_t i = 0; i < readed_bytes; i++)
      exe_path[i] = (process_path[i] == '\0') ? ' ' : process_path[i];
    exe_path[readed_bytes] = '\0';
    fclose(process_path_cmd);
    return SUCCESS;
  } else {
    fclose(process_path_cmd);
    ERR_SET(ERR_UNKNOWN, WARNING);
    return ERR_UNKNOWN;
  }
}

/* Calculating how much ram has been used by a process from its [resident] and
 * stores the value in [ram_usage] in percentage (0 - 100)
 */
static inline void GetProcessRAMusage(float *ram_usage,
                                      const uint64_t resident) {
  extern uint64_t total_mem;
  *ram_usage = 100 * ((float)resident / total_mem);
}

/* Copies all data/memory from [from] to [to]
 */
void ProcessMemCpy(Process *from, Process **to) {
  memcpy((*to)->pid.data, from->pid.data, sizeof(from->pid.pid_type));
  memcpy((*to)->name.data, from->name.data, sizeof(from->name.name_type));
  memcpy((*to)->user.data, from->user.data, sizeof(from->user.user_type));
  memcpy((*to)->priority.data, from->priority.data,
         sizeof(from->priority.priority_type));
  memcpy((*to)->nice.data, from->nice.data, sizeof(from->nice.nice_type));
  memcpy((*to)->virtualmem.data, from->virtualmem.data,
         sizeof(from->virtualmem.virtualmem_type));
  memcpy((*to)->resident.data, from->resident.data,
         sizeof(from->resident.resident_type));
  memcpy((*to)->sharemem.data, from->sharemem.data,
         sizeof(from->sharemem.sharemem_type));
  memcpy((*to)->state.data, from->state.data, sizeof(from->state.state_type));
  memcpy((*to)->cpu.data, from->cpu.data, sizeof(from->cpu.cpu_type));
  memcpy((*to)->mem.data, from->mem.data, sizeof(from->mem.mem_type));
  memcpy((*to)->time.data, from->time.data, sizeof(from->time.time_type));
  memcpy((*to)->command_path.data, from->command_path.data,
         sizeof(from->command_path.command_path_type));
}

/* Allocating memory for the process struct and its elements (its stats) and
 * fill some infomation by default, and return a pointer to it, otherwise NULL
 * if an error occured
 */
Process *InitProcess() {
  Process *process = malloc(sizeof(Process));
  CHECK_NULL_(process);
  process->pid.data = malloc(sizeof(process->pid.pid_type));
  CHECK_NULL_(process->pid.data);
  process->name.data = malloc(sizeof(process->name.name_type));
  CHECK_NULL_(process->name.data);
  process->user.data = malloc(sizeof(process->user.user_type));
  CHECK_NULL_(process->user.data);
  process->priority.data = malloc(sizeof(process->priority.priority_type));
  CHECK_NULL_(process->priority.data);
  process->nice.data = malloc(sizeof(process->nice.nice_type));
  CHECK_NULL_(process->nice.data);
  process->virtualmem.data =
      malloc(sizeof(process->virtualmem.virtualmem_type));
  CHECK_NULL_(process->virtualmem.data);
  process->resident.data = malloc(sizeof(process->resident.resident_type));
  CHECK_NULL_(process->resident.data);
  process->sharemem.data = malloc(sizeof(process->sharemem.sharemem_type));
  CHECK_NULL_(process->user.data);
  process->state.data = malloc(sizeof(process->state.state_type));
  CHECK_NULL_(process->state.data);
  process->cpu.data = malloc(sizeof(process->cpu.cpu_type));
  CHECK_NULL_(process->cpu.data);
  process->mem.data = malloc(sizeof(process->mem.mem_type));
  CHECK_NULL_(process->mem.data);
  process->time.data = malloc(sizeof(process->time.time_type));
  CHECK_NULL_(process->time.data);
  process->command_path.data =
      malloc(sizeof((process->command_path.command_path_type)));
  CHECK_NULL_(process->command_path.data);

  strcpy(process->pid.format, PID_F);
  strcpy(process->name.format, NAME_F);
  strcpy(process->user.format, USER_F);
  strcpy(process->priority.format, PRIORITY_F);
  strcpy(process->nice.format, NICE_F);
  strcpy(process->virtualmem.format, VIRTUALMEM_F);
  strcpy(process->resident.format, RESIDENT_F);
  strcpy(process->sharemem.format, SHAREMEM_F);
  strcpy(process->state.format, STATE_F);
  strcpy(process->cpu.format, CPU_F);
  strcpy(process->mem.format, MEM_F);
  strcpy(process->time.format, TIME_F);
  strcpy(process->command_path.format, COMMAND_PATH_F);

  return process;
}

/* Freeing all data in the [process] and itself
 */
void FreeProcess(Process *process) {
  if (process == NULL)
    return;
  free(process->pid.data);
  free(process->name.data);
  free(process->user.data);
  free(process->priority.data);
  free(process->nice.data);
  free(process->virtualmem.data);
  free(process->resident.data);
  free(process->sharemem.data);
  free(process->state.data);
  free(process->cpu.data);
  free(process->mem.data);
  free(process->time.data);
  free(process->command_path.data);
  free(process);
}

/* Collecting that stats of a process of id [pid] and store them in
 * [process]. Returns 0 on success
 */
int GetProcessInfoFromFile(Process **process, const pid_t pid) {
  Process *temp_process = InitProcess();
  char path[120];
  unsigned long stime = 0, utime = 0;
  time_t cutime = 0, cstime = 0;
  uint64_t starttime = 0;
  pid_t process_uid = 0;

  (void)snprintf(path, sizeof(path), "/proc/%d/stat", pid);
  FILE *pid_file;
  if ((pid_file = fopen(path, "r")) == NULL) {
    ERR_SET(ERR_OPEN_FILE, WARNING);
    FreeProcess(temp_process);
    return ERR_OPEN_FILE;
  }
  int throw = fscanf(
      pid_file,
      "%d %s %c %d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu %lu "
      "%ld %ld %ld %ld %*d %*d %llu %lu %ld",
      (pid_t *)temp_process->pid.data, (char *)temp_process->name.data,
      (char *)temp_process->state.data, &process_uid, &utime, &stime, &cutime,
      &cstime, (int64_t *)temp_process->priority.data,
      (int64_t *)temp_process->nice.data, (unsigned long long *)&starttime,
      (uint64_t *)temp_process->virtualmem.data,
      (int64_t *)temp_process->resident.data);

  *(time_t *)temp_process->time.data = utime + stime;

  if (throw == EOF) {
    ERR_SET(ERR_SCAN_FILE, WARNING);
    fclose(pid_file);
    FreeProcess(temp_process);
    return ERR_SCAN_FILE;
  }
  if (GetUserFromUid(process_uid, (char *)temp_process->user.data) != SUCCESS) {
    fclose(pid_file);
    FreeProcess(temp_process);
    return ERR_UNKNOWN;
  }
  if (GetSharedMemSize((uint64_t *)temp_process->sharemem.data,
                       *(pid_t *)temp_process->pid.data) != SUCCESS) {
    fclose(pid_file);
    FreeProcess(temp_process);
    return ERR_UNKNOWN;
  }
  if (GetProcessFullExcutable(*(pid_t *)temp_process->pid.data,
                              (char *)temp_process->command_path.data) !=
      SUCCESS) {
    fclose(pid_file);
    FreeProcess(temp_process);
    return ERR_UNKNOWN;
  }
  if (GetProcessCPUusage((float *)temp_process->cpu.data, utime, stime, cutime,
                         cstime, starttime) != SUCCESS) {
    fclose(pid_file);
    FreeProcess(temp_process);
    return ERR_UNKNOWN;
  }
  GetProcessRAMusage((float *)temp_process->mem.data,
                     *(uint64_t *)temp_process->resident.data);

  ProcessMemCpy(temp_process, process);
  fclose(pid_file);
  FreeProcess(temp_process);
  return SUCCESS;
}

/* Printing the process stats provided in [process] at x and [at_y] in the
 * [win] window of the terminal
 */
void PrintProcessItem(WINDOW *win, const Process process, const int at_y) {
  int at_x = 1;
  mvwprintw(win, at_y, at_x, process.pid.format, *(pid_t *)process.pid.data);
  at_x += PID_MAX;
  mvwprintw(win, at_y, at_x, process.name.format, (char *)process.name.data);
  at_x += NAME__MAX;
  mvwprintw(win, at_y, at_x, process.user.format, (char *)process.user.data);
  at_x += USER_MAX;
  mvwprintw(win, at_y, at_x, process.priority.format,
            *(long *)process.priority.data);
  at_x += PRI_MAX;
  mvwprintw(win, at_y, at_x, process.nice.format, *(long *)process.nice.data);
  at_x += NI_MAX;
  mvwprintw(win, at_y, at_x, process.virtualmem.format,
            *(unsigned long *)process.virtualmem.data);
  at_x += VIRT_MAX;
  mvwprintw(win, at_y, at_x, process.resident.format,
            *(long *)process.resident.data);
  at_x += RES_MAX;
  mvwprintw(win, at_y, at_x, process.sharemem.format,
            *(unsigned *)process.sharemem.data);
  at_x += SHR_MAX;
  mvwprintw(win, at_y, at_x, process.state.format, *(char *)process.state.data);
  at_x += S_MAX;
  mvwprintw(win, at_y, at_x, process.cpu.format, *(float *)process.cpu.data);
  at_x += CPU_MAX;
  mvwprintw(win, at_y, at_x, process.mem.format, *(float *)process.mem.data);
  at_x += MEM_MAX;
  mvwprintw(win, at_y, at_x, process.time.format, *(long *)process.time.data);
  at_x += TIME_MAX;
  mvwprintw(win, at_y, at_x, process.command_path.format,
            (char *)process.command_path.data);
}
