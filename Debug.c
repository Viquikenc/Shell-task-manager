#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

void DebugWriteNumInfo(const long num, const uint32_t lines) {
  FILE *debugfile = fopen("debug.log", "a+");
  (void)fprintf(debugfile, "[%u] Info Num: %ld\n", lines, num);
  fclose(debugfile);
}
void DebugWriteStringInfo(const char *string, const uint32_t lines) {
  FILE *debugfile = fopen("debug.log", "a+");
  (void)fprintf(debugfile, "[%u] Info String: %s\n", lines, string);
  fclose(debugfile);
}
void DebugWriteDoubleInfo(const double num, const uint32_t lines) {
  FILE *debugfile = fopen("debug.log", "a+");
  (void)fprintf(debugfile, "[%u] Info Double : %f\n", lines, num);
  fclose(debugfile);
}
