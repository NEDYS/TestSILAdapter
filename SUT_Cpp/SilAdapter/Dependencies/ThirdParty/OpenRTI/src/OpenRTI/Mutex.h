/* -*-c++-*- OpenRTI - Copyright (C) 2004-2015 Mathias Froehlich
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

#ifndef OpenRTI_Mutex_h
#define OpenRTI_Mutex_h

#include "Export.h"
#include "OpenRTIConfig.h"

#ifndef __CPlusPlusStd
#error "must include OpenRTIConfig.h!"
#endif

#include <mutex>

namespace OpenRTI {

class Condition;
class ScopeLock;

class OPENRTI_API Mutex {
public:
  Mutex() noexcept { }
  ~Mutex() noexcept { }

  void lock() { _mutex.lock(); }
  void unlock() { _mutex.unlock(); }

private:
  Mutex(const Mutex&) = delete;
  Mutex(Mutex&&) = delete;
  Mutex& operator=(const Mutex&) = delete;
  Mutex& operator=(Mutex&&) = delete;

  std::mutex _mutex;

  friend class Condition;
  friend class ScopeLock;
};

class OPENRTI_API RecursiveMutex {
public:
  RecursiveMutex() noexcept { }
  ~RecursiveMutex() noexcept { }

  void lock()
  {
    _mutex.lock();
  }
  void unlock()
  {
    _mutex.unlock();
  }

private:
  RecursiveMutex(const RecursiveMutex&) = delete;
  RecursiveMutex(RecursiveMutex&&) = delete;
  RecursiveMutex& operator=(const RecursiveMutex&) = delete;
  RecursiveMutex& operator=(RecursiveMutex&&) = delete;

  std::recursive_mutex _mutex;

  friend class Condition;
  friend class RecursiveScopeLock;
};
} // namespace OpenRTI

#endif
