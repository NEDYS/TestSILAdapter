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

#include "DebugNew.h"
#include "Thread.h"

#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <windows.h>
#include "Exception.h"
#include "Export.h"
#include <processthreadsapi.h>
#include <vector>
#include <locale>

namespace OpenRTI {

inline std::wstring to_wstring(const std::string& str)
{
  if (str.empty()) return std::wstring();
  const std::ctype<wchar_t>& CType = std::use_facet<std::ctype<wchar_t> >(std::locale());
  std::vector<wchar_t> wideStringBuffer(str.length());
  CType.widen(str.data(), str.data() + str.length(), &wideStringBuffer[0]);
  return std::wstring(&wideStringBuffer[0], wideStringBuffer.size());
}

struct OPENRTI_LOCAL Thread::PrivateData {
  PrivateData() : _handle(INVALID_HANDLE_VALUE)
  {
  }
  PrivateData(const PrivateData&) = delete;
  ~PrivateData() noexcept
  {
    try {
      if (_handle == INVALID_HANDLE_VALUE)
        return;
      CloseHandle(_handle);
      _handle = INVALID_HANDLE_VALUE;
    }
    catch (...)
    {
    }
  }

  static DWORD WINAPI start_routine(LPVOID data)
  {
    Thread* thread = static_cast<Thread*>(data);
    thread->run();
    if (!Thread::decRef(thread))
      Thread::destruct(thread);
    return 0;
  }

  bool start(Thread& thread)
  {
    if (_handle != INVALID_HANDLE_VALUE)
      return false;
    incRef(&thread);
    _handle = CreateThread(0, 0, start_routine, &thread, 0, 0);
    if (_handle == INVALID_HANDLE_VALUE) {
      decRef(&thread);
      return false;
    }
#if defined(_WIN32_WINNT_WIN10) && (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
    if (!thread.getName().empty())
    {
      HRESULT r;
      r = SetThreadDescription(GetCurrentThread(), to_wstring(thread.getName()).c_str());
    }
#endif
    return true;
  }

  void wait() noexcept
  {
    if (_handle == INVALID_HANDLE_VALUE)
      return;
    DWORD ret = WaitForSingleObject(_handle, INFINITE);
    if (ret != WAIT_OBJECT_0)
      return;
    CloseHandle(_handle);
    _handle = INVALID_HANDLE_VALUE;
  }

  HANDLE _handle;
};

Thread::Thread() noexcept :
  _privateData(new PrivateData)
{
}

Thread::~Thread() noexcept
{
  OpenRTIAssert(!Thread::count(this));
  delete _privateData;
  _privateData = 0;
}

void Thread::destruct(Thread* thread) noexcept
{
  delete thread;
}

bool
Thread::start()
{
  return _privateData->start(*this);
}

void
Thread::wait() noexcept
{
  _privateData->wait();
}

} // namespace OpenRTI
