#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "error_handler.h"

void write_log(char message[MSG_MAX]) {
  extern FILE *err_file;
  fprintf(err_file, "%s\n\n", message);
}

void err_set(err_enum err_code, flag_t flag, const char *file, uint64_t line) {
  char message[MSG_MAX] = {0};
  if ((flag & FATAL) || (flag & WARNING))
    switch (err_code) {
    case SUCCESS:
      break;
    case ERR_OPEN_FILE:
      snprintf(message, MSG_MAX - 1,
               "%s:[%s] Error in [%lu] Opening File Failed\n-->%s",
               (flag | WARNING) ? "WARNING" : "FATAL", file, line,
               strerror(errno));
      write_log(message);
      break;
    case ERR_SCAN_FILE:
      snprintf(
          message, 255, "%s:[%s] Error in [%lu] Scaning File Failed\n-->%s",
          (flag | WARNING) ? "WARNING" : "FATAL", file, line, strerror(errno));
      write_log(message);
      break;
    case ERR_READ_FILE:
      snprintf(
          message, 255, "%s:[%s] Error in [%lu] Reading File Failed\n-->%s",
          (flag | WARNING) ? "WARNING" : "FATAL", file, line, strerror(errno));
      write_log(message);
      break;
    case ERR_PERMMISSION_DENIED:
      snprintf(message, 255, "%s:[%s] Error in [%lu] Permission Denied\n-->%s",
               (flag | WARNING) ? "WARNING" : "FATAL", file, line,
               strerror(errno));
      write_log(message);
      break;
    case ERR_NOT_EXIST_FILE:
      snprintf(message, 255,
               "%s:[%s] Error in [%lu] File or Directory Does Not Exist\n-->%s",
               (flag | WARNING) ? "WARNING" : "FATAL", file, line,
               strerror(errno));
      write_log(message);
      break;
    default:
      snprintf(message, 255, "%s:[%s] Unknown Error [%lu] | %s\n",
               (flag | WARNING) ? "WARNING" : "FATAL", file, line,
               strerror(errno));
      write_log(message);
      break;
    }
}
