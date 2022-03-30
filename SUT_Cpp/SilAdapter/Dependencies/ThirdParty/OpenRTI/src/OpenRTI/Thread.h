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

#ifndef OpenRTI_Thread_h
#define OpenRTI_Thread_h

#include "Export.h"
#include "SharedPtr.h"
#include "WeakReferenced.h"
#include <string>

namespace OpenRTI {

class OPENRTI_API Thread : public WeakReferenced {
public:
  Thread() noexcept;

  static void destruct(Thread* thread) noexcept;

  bool start();
  void wait() noexcept;

  void setName(const std::string& name) { _name = name; }
  std::string getName() { return _name; }
protected:
  virtual ~Thread() noexcept;

  virtual void run() = 0;

private:
  Thread(const Thread&) = delete;
  Thread(Thread&&) = delete;
  Thread& operator=(const Thread&) = delete;
  Thread& operator=(Thread&&) = delete;

  struct PrivateData;
  PrivateData* _privateData;
  std::string _name;
};

} // namespace OpenRTI

#endif
