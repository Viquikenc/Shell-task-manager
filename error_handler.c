#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "error_handler.h"

void err_set(err_enum err_code, const char *file, uint64_t line,
             const char *func) {
  switch (err_code) {
  case SUCCESS:
    break;
  case ERR_OPEN_FILE:
    printf("[%s] Error in [%lu : %s] Opening File Failed\n", file, line, func);
    printf("%s", strerror(errno));
    break;
  case ERR_SCAN_FILE:
    printf("[%s] Error in [%lu : %s] Scaning File Failed\n", file, line, func);
    printf("%s", strerror(errno));
    break;
  case ERR_READ_FILE:
    printf("[%s] Error in [%lu : %s] Reading File Failed\n", file, line, func);
    printf("%s", strerror(errno));
    break;
  case ERR_PERMMISSION_DENIED:
    printf("[%s] Error in [%lu : %s] Permission Denied\n", file, line, func);
    printf("%s", strerror(errno));
    break;
  case ERR_NOT_EXIST_FILE:
    printf("[%s] Error in [%lu : %s] File or Directory Does Not Exist\n", file,
           line, func);
    printf("%s", strerror(errno));
    break;
  default:
    printf("[%s] Unknown Error [%lu : %s]\n", file, line, func);
    printf("%s", strerror(errno));
    break;
  }
}
