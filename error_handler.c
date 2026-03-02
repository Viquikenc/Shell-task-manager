#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_handler.h"

void write_log(char message[MSG_MAX]) {
  extern FILE *err_file;
  fprintf(err_file, "%s\n", message);
}

void _err_set(err_enum err_code, flag_t flag, const char *file,
              const char *func, uint64_t line) {
  char message[MSG_MAX] = {0};
  if (!(flag & IGNORED))
    switch (err_code) {
    case SUCCESS:
      break;
    case ERR_FILE_OPEN_FAIL:
      snprintf(message, MSG_MAX - 1,
               "%s: %s Error in [%s] line %lu - Opening File Failed\n-->%s",
               (flag | WARNING) ? "WARNING" : "FATAL", file, func, line,
               strerror(errno));
      write_log(message);
      break;
    case ERR_FILE_SCAN_FAIL:
      snprintf(message, 255,
               "%s: %s Error in [%s] line %lu - Scaning File Failed\n-->%s",
               (flag | WARNING) ? "WARNING" : "FATAL", file, func, line,
               strerror(errno));
      write_log(message);
      break;
    case ERR_FILE_READ_FAIL:
      snprintf(message, 255,
               "%s: %s Error in [%s] line %lu - Reading File Failed\n-->%s",
               (flag | WARNING) ? "WARNING" : "FATAL", file, func, line,
               strerror(errno));
      write_log(message);
      break;
    case ERR_PERMMISSION_DENIED:
      snprintf(message, 255,
               "%s: %s] Error in [%s] line %lu - Permission Denied\n-->%s",
               (flag | WARNING) ? "WARNING" : "FATAL", file, func, line,
               strerror(errno));
      write_log(message);
      break;
    case ERR_FILE_NOT_EXIST:
      snprintf(message, 255,
               "%s: %s Error in [%s] line %lu - File or Directory Does Not "
               "Exist\n-->%s",
               (flag | WARNING) ? "WARNING" : "FATAL", file, func, line,
               strerror(errno));
      write_log(message);
      break;
    default:
      snprintf(message, 255, "%s: %s Unknown Error in [%s] line %lu\n-->%s",
               (flag | WARNING) ? "WARNING" : "FATAL", file, func, line,
               strerror(errno));
      write_log(message);
      break;
    }
  if (flag & FATAL) {
    fputs("FATAL error occured, please check \"errors.log\" for debuging",
          stderr);
    exit(-1);
  }
}
