

#include "DebugNew.h"
#ifndef linux
#include <Windows.h>
#endif

#include <cstdio>
#include "dprintf.h"
#include <iostream>

bool sEnableDebugPrintf = true;
bool sDebugToConsole = false;
void DebugPrintf(__format_string const char* fmt, ...)
{
#ifndef linux
  if (sEnableDebugPrintf)
  {
    bool errorState = false;
    static const char* errorFormat = "error in format string or argument list: %s: %s";
    std::string error;
    va_list args;
    va_start(args, fmt);
    DWORD threadId = ::GetCurrentThreadId();
    DWORD processId = ::GetCurrentProcessId();
    int bufsize1 = _scprintf("[%ul:%ul] ", processId, threadId);
    int bufsize2 = _vscprintf(fmt, args);
    if (bufsize2 < 0)
    {
      errorState = true;
      error = strerror(errno);
      bufsize2 = _scprintf(errorFormat, fmt, error.c_str());
    }
    int bufsize = bufsize1 + bufsize2 + 1;

    char* buffer = new char[bufsize];
    _snprintf(buffer, bufsize1, "[%ul:%ul] ", processId, threadId);
    if (!errorState)
    {
      _vsnprintf(buffer + bufsize1, bufsize2 + 1, fmt, args);
    }
    else
    {
      bufsize2 = _snprintf(buffer + bufsize1, bufsize2 + 1, errorFormat, fmt, error.c_str());
    }
    va_end(args);
    //fputs(buffer, stdout);
    OutputDebugStringA(buffer);
    if (sDebugToConsole)
    {
      std::cout << buffer;
    }
    delete [] buffer;
  }
#endif
}

std::string string_vprintf(const char* fmt, va_list args)
{
#ifndef linux
  int bufsiz = _vscprintf(fmt, args) + 1;
  char* buffer = new char[bufsiz];
  _vsnprintf(buffer, bufsiz, fmt, args);
  std::string result(buffer);
  delete [] buffer;
  return result;
#else
  return "";
#endif
}

std::string string_printf(__format_string const char* fmt, ...)
{
#ifndef linux
  va_list args;
  va_start(args, fmt);
  int bufsiz = _vscprintf(fmt, args) + 1;
  char* buffer = new char[bufsiz];
  _vsnprintf(buffer, bufsiz, fmt, args);
  va_end(args);
  std::string result(buffer);
  delete [] buffer;
  return result;
#else
  return "";
#endif
}
