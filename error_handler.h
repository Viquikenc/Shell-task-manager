#ifndef __ERR_HANDLE_H
#define __ERR_HANDLE_H

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#define MSG_MAX 256

#define _CHECK_NULL_(data)                                                     \
  do {                                                                         \
    if ((data) == NULL) {                                                      \
      return NULL;                                                             \
    }                                                                          \
  } while (0)

typedef enum flag_t {
  IGNORED = (1 << 0),
  WARNING = (1 << 1),
  FATAL = (1 << 2)
} flag_t;

typedef enum err_enum {
  SUCCESS = 0,
  ERR_FILE_OPEN_FAIL,
  ERR_FILE_SCAN_FAIL,
  ERR_FILE_READ_FAIL,
  ERR_FILE_NOT_EXIST,
  ERR_PERMISSION_DENIED,
  ERR_PROCESS_INIT_FAIL,
  ERR_WIN_OVERWRITE_FAIL,
  ERR_MENU_INIT_FAIL,
  ERR_UNKNOWN
} err_enum;

typedef struct err_handl {
  err_enum err;
  char *file;
  uint64_t line;
  char *func;
} err_handl;

void _err_set(err_enum err_code, flag_t flag, const char *file,
              const char *func, uint64_t line);
void write_log(char message[MSG_MAX]);

#define ERR_SET(err, flag) _err_set((err), (flag), __FILE__, __func__, __LINE__)

#endif
