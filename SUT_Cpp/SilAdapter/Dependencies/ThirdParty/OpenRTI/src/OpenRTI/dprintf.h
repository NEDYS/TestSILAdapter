
#pragma once

#include <string>

#undef _DEBUG_PRINTF

#ifdef linux
#define __format_string
#define __noop
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

// very simple, basic and safe (but somewhat slow!) printf-like function for dumping stuff to debug output (visual studio output window or dbgview)
// No restrictions on string sizes. Don't use inside time-critical code. You may want to enclose calls into specific #ifdefs
void DebugPrintf(__format_string const char* fmt, ...);
// conditional debug printing
#ifdef _DEBUG_PRINTF
#define CondDebugPrintf(...) DebugPrintf(__VA_ARGS__)
#else
#define CondDebugPrintf(...)
#endif

// similar, but returns the formatted std::string instead
std::string string_printf(__format_string const char* fmt, ...);
// similar, but with already opened va_list
std::string string_vprintf(const char* fmt, va_list args);

extern bool sEnableDebugPrintf;
extern bool sDebugToConsole;

