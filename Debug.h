#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdint.h>

void DebugWriteNumInfo(const long num, const uint32_t lines);
void DebugWriteStringInfo(const char *string, const uint32_t lines);
void DebugWriteDoubleInfo(const double num, const uint32_t lines);

#define DebugWriteNumInfo(num) DebugWriteNumInfo(num, __LINE__)
#define DebugWriteStringInfo(string) DebugWriteStringInfo(string, __LINE__)
#define DebugWriteDoubleInfo(num) DebugWriteDoubleInfo(num, __LINE__)
#endif
