/* -*-c++-*- OpenRTI - Copyright (C) 2012-2014 Mathias Froehlich
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

#ifndef OpenRTI_SingletonPtr_h
#define OpenRTI_SingletonPtr_h

#include "Atomic.h"
#include "Clock.h"
#include "SharedPtr.h"

namespace OpenRTI {

// !!! DO NOT USE !!!
// This class is considered generally unsafe, because get() may be called before the constructor has been called.
// This may happen if get() is called from a static initializer, before the corresponding static variable of type SingletonPtr<T> has been called.
// The constructor will then wipe SingletonPtr<T>::_ptr, leaving a memory leak. Happened in AbstractThreadLocal::_Provider/Win32 version.
// Better rely on C++11 Magic Statics (N2660, http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2660.htm).

template<typename T>
class SingletonPtr {
public:
  SingletonPtr()
  { }
  ~SingletonPtr()
  { }

  SharedPtr<T> get()
  {
    // If the pointer is not already initialized
    while (unsigned(_initialized) != 2) {
      // We expect 0 == uninitalized and write there 1 == initializing
      if (!_initialized.compareAndExchange(0, 1, Atomic::MemoryOrderAcqRel)) {
        Clock::sleep_for(Clock::fromSeconds(1e-6));
      } else {
        _ptr = new T();
        // We know that it must be 1 and write 2 there
        _initialized.compareAndExchange(1, 2, Atomic::MemoryOrderAcqRel);
      }
    }
    return _ptr;
  }

  void reset()
  {
    _ptr.reset();
    while (unsigned(_initialized) != 0) {
      _initialized.decFetch();
    }
  }
private:
  SingletonPtr(const SingletonPtr&);
  SingletonPtr& operator=(const SingletonPtr&);

  Atomic _initialized;
  SharedPtr<T> _ptr;
};

} // namespace OpenRTI

#endif
