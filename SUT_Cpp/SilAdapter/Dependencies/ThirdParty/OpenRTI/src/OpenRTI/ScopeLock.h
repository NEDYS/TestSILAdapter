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

#ifndef OpenRTI_ScopeLock_h
#define OpenRTI_ScopeLock_h

#include "Mutex.h"

#ifndef __CPlusPlusStd
#error "must include OpenRTIConfig.h!"
#endif

namespace OpenRTI {

class ScopeLock : public std::unique_lock<std::mutex> {
public:
  ScopeLock(Mutex& mutex) : std::unique_lock<std::mutex>(mutex._mutex) { }
  ~ScopeLock() noexcept { }

private:
  ScopeLock() = delete;
  ScopeLock(const ScopeLock&) = delete;
  ScopeLock(ScopeLock&&) = delete;
  ScopeLock& operator=(const ScopeLock&) = delete;
  ScopeLock& operator=(ScopeLock&&) = delete;
};

class RecursiveScopeLock : public std::unique_lock<std::recursive_mutex> {
public:
  RecursiveScopeLock(RecursiveMutex& mutex) : std::unique_lock<std::recursive_mutex>(mutex._mutex)
  { }
  ~RecursiveScopeLock() noexcept
  { }

private:
  RecursiveScopeLock() = delete;
  RecursiveScopeLock(const RecursiveScopeLock&) = delete;
  RecursiveScopeLock(RecursiveScopeLock&&) = delete;
  RecursiveScopeLock& operator=(const RecursiveScopeLock&) = delete;
  RecursiveScopeLock& operator=(RecursiveScopeLock&&) = delete;
};

} // namespace OpenRTI

#endif
