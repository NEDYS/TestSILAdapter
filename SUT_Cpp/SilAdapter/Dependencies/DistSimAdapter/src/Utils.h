/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <locale>
#include <unordered_map>

//// Process ID
//#if defined(_WIN32)
//#include <process.h>
//#elif defined (__linux__)
//#include <unistd.h>
//#endif

// Thread ID
#include <atomic>

#include "RTI/time/HLAinteger64Time.h"
#include "RTI/Handle.h"

#include "DistSimIBConfig.h"

#define SDebug    "[ Debug    ] "
#define SInfo     "[ Info     ] "
#define SError    "[ Error    ] "
#define SWarn     "[ Warning  ] "
#define SResolve  "[ Resolved ] "


// =========================================
// std::unordered_map extensions

// Provide a hash for ObjectInstanceHandle to use it as key in unordered_map
template<>
struct std::hash<rti::ObjectInstanceHandle>
{
  std::size_t operator()(const rti::ObjectInstanceHandle& handle) const
  {
    return handle.hash();
  }
};

namespace NDistSimIB
{

  // =========================================
// Debug Output

  static std::atomic<unsigned long long> thread_counter;
  unsigned long long thread_id();

  #ifndef SILENT
  #define DebugPrintf(severity, fmt, ...) DebugMessage(severity, fmt, ## __VA_ARGS__);
  #else
  #define DebugPrintf(severity, fmt, ...)
  #endif

  #ifndef SUPPRESS_ERRORS
  #define Error(fmt, ...) ErrorMessage(fmt, ## __VA_ARGS__);
  #else
  #define Error(fmt, ...)
  #endif

  template<typename... ArgT>
  void DebugMessage(const char* severity, const char* fmt, ArgT... args)
  {
    //// Process ID
    //std::string msg = std::string(severity) + " [ " + std::to_string(getpid()) + " ] " + std::string(fmt);
    //fprintf(stderr, msg.c_str(), args...);

    // Thread ID
    std::string msg = std::string(severity) + " [ " + std::to_string(thread_id()) + " ] " + std::string(fmt);
    fprintf(stderr, msg.c_str(), args...);
    //fflush(stderr);

    //fprintf(stderr, severity);
    //fprintf(stderr, fmt, args...);
  }

  template<typename... ArgT>
  void ErrorMessage(const char* fmt, ArgT... args)
  {
    std::string msg = std::string(SError) + " [ " + std::to_string(thread_id()) + " ] " + std::string(fmt);
    fprintf(stderr, msg.c_str(), args...);

    //fprintf(stderr, SError);
    //fprintf(stderr, fmt, args...);
  }

  // Notify once
  enum class ResolvableWarning
  {
    NoRtinodeConnection
  };

  template<typename... ArgT>
  void RaiseResolvableWarning(const ResolvableWarning incident, std::unordered_map<ResolvableWarning, bool>& notificationRaised, const char* fmt, ArgT... args)
  {
    if (!notificationRaised[incident])
    {
      notificationRaised[incident] = true;
      fprintf(stderr, SWarn);
      fprintf(stderr, fmt, args...);
    }
  }

  template<typename... ArgT>
  void SettleResolvableWarning(const ResolvableWarning incident, std::unordered_map<ResolvableWarning, bool>& notificationRaised, const char* fmt, ArgT... args)
  {
    if (notificationRaised[incident])
    {
      notificationRaised[incident] = false;
      fprintf(stderr, SResolve);
      fprintf(stderr, fmt, args...);
    }
  }

  // =========================================
  // Inlines

  // Time conversion
  inline int64_t convertTime(rti::LogicalTime const& theTime)
  {
    rti::HLAinteger64Time castedTime = (rti::HLAinteger64Time)theTime;
    return castedTime.getTime();
  }

  // =========================================
  // String conversion

  inline std::wstring string2wstring(const std::string& str)
  {
    if (str.empty()) return std::wstring();
    const std::ctype<wchar_t>& CType = std::use_facet<std::ctype<wchar_t> >(std::locale());
    std::vector<wchar_t> wideStringBuffer(str.length());
    CType.widen(str.data(), str.data() + str.length(), &wideStringBuffer[0]);
    return std::wstring(&wideStringBuffer[0], wideStringBuffer.size());
  }

  inline std::string wstring2string(const std::wstring& str)
  {
    if (str.empty()) return std::string();
    const std::ctype<wchar_t>& CType = std::use_facet<std::ctype<wchar_t> >(std::locale());
    std::vector<char> stringBuffer(str.length());
    CType.narrow(str.data(), str.data() + str.length(), '_', &stringBuffer[0]);
    return std::string(&stringBuffer[0], stringBuffer.size());
  }

  inline bool endsWith(std::string const& fullString, std::string const& ending) {
    if (fullString.length() >= ending.length()) {
      return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else {
      return false;
    }
  }


} //namespace NDistSimIB