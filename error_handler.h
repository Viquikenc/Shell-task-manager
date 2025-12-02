#ifndef __ERR_HANDLE
#define __ERR_HANDLE

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

typedef enum flag_t {
  IGNORED = (1 << 0),
  WARNING = (1 << 1),
  FATAL = (1 << 2)
} flag_t;

typedef enum err_enum {
  SUCCESS = 0,
  ERR_OPEN_FILE,
  ERR_SCAN_FILE,
  ERR_READ_FILE,
  ERR_PERMMISSION_DENIED,
  ERR_NOT_EXIST_FILE,
  ERR_UNKNOWN
} err_enum;

typedef struct err_handl {
  err_enum err;
  char *file;
  uint64_t line;
  char *func;
} err_handl;

void err_set(err_enum err_code, flag_t flag, const char *file, uint64_t line);
void write_log(char message[1024]);

#define ERR_SET(err, flag) err_set(err, flag, __FILE__, __LINE__)

#endif
