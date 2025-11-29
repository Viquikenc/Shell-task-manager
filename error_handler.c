#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "error_handler.h"

void write_log(char *message) {
  FILE *err_file = fopen("errors.log", "a+");
  fprintf(err_file, "%s\n\n", message);
  fclose(err_file);
}
void err_set(err_enum err_code, const char *file, uint64_t line,
             const char *func) {
  char message[256] = {0};
  switch (err_code) {
  case SUCCESS:
    break;
  case ERR_OPEN_FILE:
    sprintf(message, "[%s] Error in [%lu : %s] Opening File Failed\n-->%s",
            file, line, func, strerror(errno));
    printf("%s", message);
    write_log(message);
    break;
  case ERR_SCAN_FILE:
    sprintf(message, "[%s] Error in [%lu : %s] Scaning File Failed\n-->%s",
            file, line, func, strerror(errno));
    printf("%s", message);
    write_log(message);
    break;
  case ERR_READ_FILE:
    sprintf(message, "[%s] Error in [%lu : %s] Reading File Failed\n-->%s",
            file, line, func, strerror(errno));
    printf("%s", message);
    write_log(message);
    break;
  case ERR_PERMMISSION_DENIED:
    sprintf(message, "[%s] Error in [%lu : %s] Permission Denied\n-->%s", file,
            line, func, strerror(errno));
    printf("%s", message);
    write_log(message);
    break;
  case ERR_NOT_EXIST_FILE:
    sprintf(message,
            "[%s] Error in [%lu : %s] File or Directory Does Not Exist\n-->%s",
            file, line, func, strerror(errno));
    printf("%s", message);
    write_log(message);
    break;
  default:
    sprintf(message, "[%s] Unknown Error [%lu : %s] | %s\n", file, line, func,
            strerror(errno));
    printf("%s", message);
    write_log(message);
    break;
  }
}
