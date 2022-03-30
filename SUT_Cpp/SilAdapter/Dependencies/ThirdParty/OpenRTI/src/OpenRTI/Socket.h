/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich 
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

#ifndef OpenRTI_Socket_h
#define OpenRTI_Socket_h

#include "Export.h"
#include "Referenced.h"

namespace OpenRTI {

class SocketEventDispatcher;

class OPENRTI_API Socket : public Referenced {
public:
  virtual ~Socket() noexcept;
  static void destruct(Socket* socket) noexcept;

  bool isOpen() const noexcept;
  virtual void close();

  void setWriteable() { _mIsWritable = true; }
  bool isWritable() const { return _mIsWritable; }
protected:
  struct PrivateData;
  PrivateData* _privateData;
  bool _mIsWritable;

  Socket(PrivateData* privateData);

private:
  Socket(const Socket &sock) = delete;
  void operator=(const Socket &sock) = delete;

  friend class SocketEventDispatcher;
};

} // namespace OpenRTI

#endif
