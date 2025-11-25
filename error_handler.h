#ifndef __ERR_HANDLE
#define __ERR_HANDLE

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

typedef enum err_enum {
  SUCCESS = 0,
  ERR_OPEN_FILE,
  ERR_SCAN_FILE,
  ERR_READ_FILE,
  ERR_PERMMISSION_DENIED
} err_enum;

typedef struct err_handl {
  err_enum err;
  char *file;
  uint64_t line;
  char *func;
} err_handl;

void err_set(err_enum err_code, const char *file, uint64_t line,
             const char *func);

#define ERR_SET(err) err_set(err, __FILE__, __LINE__, __func__)

#endif
