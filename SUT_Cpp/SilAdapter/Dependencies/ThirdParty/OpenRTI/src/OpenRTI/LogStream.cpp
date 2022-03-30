/* -*-c++-*- OpenRTI - Copyright (C) 2004-2012 Mathias Froehlich
 *
 * This file is part of OpenRTI.
 *
 * OpenRTI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * OpenRTI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenRTI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if defined(_WIN32)
#include <windows.h>
#include <processenv.h>
#endif

#include "DebugNew.h"
#include "LogStream.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include "Mutex.h"
#include "Referenced.h"
#include "ScopeLock.h"
#include "SharedPtr.h"
#include "StringUtils.h"
#include "ThreadLocal.h"

#if defined(_WIN32)
#if __CPlusPlusStd >= 201703L
#include <filesystem>
#if _MSC_VER >= 1920
namespace fs = std::filesystem;
#else
namespace fs = std::experimental::filesystem;
#endif
#endif
#endif

namespace OpenRTI {

class OPENRTI_LOCAL LogStream::StreamPair {
public:
  // The reason for that not just being a static is that
  // we can guarantee that the mutex only dies if its last
  // user has died. This is hard to guarantee if it's just
  // static in here.
  struct OPENRTI_LOCAL ReferencedMutex : public Referenced {
    Mutex _mutex;
  };
  static SharedPtr<ReferencedMutex>& getReferencedMutex()
  {
    static SharedPtr<ReferencedMutex> referencedMutex = MakeShared<ReferencedMutex>();
    return referencedMutex;
  }

  struct OPENRTI_LOCAL StreamBuf : public std::stringbuf {
  public:
    StreamBuf(std::ostream& /*stream*/) :
      _referencedMutex(getReferencedMutex())
    {
      //AddStream(stream);
    }
    void AddStream(std::ostream& stream)
    {
      _streams.push_back(&stream);
    }
    void RemoveStream(std::ostream& stream)
    {
      _streams.remove(&stream);
    }
    virtual ~StreamBuf()
    {
      sync();
    }

  protected:
    int sync() override
    {
      // Not having the mutex here can only happen in weired circumstances.
      // But if so, do not hold back the message.
      if (_referencedMutex.valid()) {
        ScopeLock scopeLock(_referencedMutex->_mutex);
        return _syncUnlocked();
      } else {
        return _syncUnlocked();
      }
    }

    int _syncUnlocked()
    {
      const char* base = pbase();
      size_t count = pptr() - base;
      if (count <= 0)
        return 0;
      for (auto stream : _streams)
      {
        stream->write(base, count);
      }
      pubseekpos(0, std::ios_base::out);
      return 0;
    }

  private:
    //std::ostream* _stream = nullptr;
    std::list<std::ostream*> _streams;
    SharedPtr<ReferencedMutex> _referencedMutex;
  };

  struct OPENRTI_LOCAL Stream {
    Stream(std::ostream& stream) :
      _streamBuf(stream),
      _stream(&_streamBuf)
    { }

    // The order of these fields is to make sure initialization
    // does not use an uninitialized streambuf.
    StreamBuf _streamBuf;
    std::ostream _stream;
  };

  StreamPair() :
    _outStream(std::cout),
    _errStream(std::cerr)
  { }

  void AddOutStream(std::ostream& stream)
  {
    _outStream._streamBuf.AddStream(stream);
  }
  void RemoveOutStream(std::ostream& stream)
  {
    _outStream._streamBuf.RemoveStream(stream);
  }
  void AddErrStream(std::ostream& stream)
  {
    _errStream._streamBuf.AddStream(stream);
  }
  void RemoveErrStream(std::ostream& stream)
  {
    _errStream._streamBuf.RemoveStream(stream);
  }
  std::ostream* getOutStream()  { return &_outStream._stream; }
  std::ostream* getErrStream()  { return &_errStream._stream; }
  static StreamPair* getStreamPair()
  {
    return _Instance.instance();
  }

private:
  Stream _outStream;
  Stream _errStream;
  static ThreadLocal<StreamPair> _Instance;
};

ThreadLocal<LogStream::StreamPair> LogStream::StreamPair::_Instance;

static unsigned
atou(const char* s)
{
  if (!s)
    return 0u;

  std::stringstream strstream(s);
  unsigned value;
  strstream >> value;
  if (!strstream)
    return 0u;

  return value;
}

void
LogStream::setCategory(LogStream::Category category)
{
  Instance().mCategory = category;
}

void
LogStream::setCategoryEnable(LogStream::Category category, bool enable)
{
  LogStream& logger = Instance();
  if (enable) {
    logger.mCategory |= category;
  } else {
    logger.mCategory &= ~category;
  }
}

void
LogStream::setCategoryDisable(LogStream::Category category)
{
  setCategoryEnable(category, false);
}

void
LogStream::setPriority(LogStream::Priority priority)
{
  LogStream& logger = Instance();
  logger.mPriority = priority;
}

void LogStream::AddLogFile(const std::string& path)
{
  std::string expandedPath = path;
#if defined(_WIN32) 
#if __CPlusPlusStd >= 201703L
  DWORD requiredSize = ExpandEnvironmentStringsA(path.c_str(), NULL, 0);
  char* buffer = new char[requiredSize];
  DWORD expandedSize = ExpandEnvironmentStringsA(path.c_str(), buffer, requiredSize);
  if (expandedSize == requiredSize - 1)
  {
    expandedPath = buffer;
  }
  fs::path directory = fs::path(expandedPath).parent_path();
  if (!directory.empty() && !fs::exists(directory))
  {
    fs::create_directories(directory);
  }
#endif
#endif
  LogStream& logger = Instance();
  std::cout << "Logging to \"" << expandedPath << "\"" << std::endl;
  logger.mLogFile.open(expandedPath);
  StreamPair::getStreamPair()->AddOutStream(logger.mLogFile);
}

void LogStream::EnableLogToConsole(bool enable)
{
  //LogStream& logger = Instance();
  if (enable)
  {
    std::cout << "Console logging enabled" << std::endl;
    StreamPair::getStreamPair()->AddOutStream(std::cout);
    StreamPair::getStreamPair()->AddErrStream(std::cerr);
  }
  else
  {
    std::cout << "Console logging disabled" << std::endl;
    StreamPair::getStreamPair()->RemoveOutStream(std::cout);
    StreamPair::getStreamPair()->RemoveErrStream(std::cerr);
  }
}

LogStream&
LogStream::Instance()
{
  static LogStream logStreamInstance;
  return logStreamInstance;
}

std::ostream*
LogStream::getStream(Category category, LogStream::Priority priority)
{
  if (!getEnabled(category, priority))
    return 0;

  StreamPair* streamPair = StreamPair::getStreamPair();
  if (!streamPair)
    return 0;
  if (Info <= priority)
    return streamPair->getOutStream();
  else
    return streamPair->getErrStream();
}

LogStream::LogStream() :
  mCategory(~0u),
  mPriority(Error)
{
  // Set some defaults from the environment
  unsigned value = atou(std::getenv("OPENRTI_DEBUG_PRIORITY"));
  if (value)
    mPriority = value;

  const char* env = std::getenv("OPENRTI_DEBUG_CATEGORY");
  if (env) {
    std::vector<std::string> categories = split(env, ", |");
    if (!categories.empty()) {
      mCategory = 0;
      for (std::vector<std::string>::const_iterator i = categories.begin(); i != categories.end(); ++i) {

        static const struct {
          const char* name;
          Category bit;
        } sCategories[] = {
#define CATEGORY(name) { #name, name }
          CATEGORY(Assert),
          CATEGORY(Network),
          CATEGORY(MessageCoding),
          CATEGORY(FederateAmbassador),
          CATEGORY(ServerConnect),
          CATEGORY(ServerFederation),
          CATEGORY(ServerFederate),
          CATEGORY(ServerSyncronization),
          CATEGORY(ServerTime),
          CATEGORY(ServerObjectInstance)
#undef CATEGORY
        };

        for (unsigned j = 0; j < sizeof(sCategories)/sizeof(sCategories[0]); ++j) {
          if (*i == sCategories[j].name)
            mCategory |= sCategories[j].bit;
        }
      }
    }
    // last resort, if nothing matched
    if (mCategory == 0) {
      mCategory = Category(atou(env));
    }
  }
}

} // namespace OpenRTI
