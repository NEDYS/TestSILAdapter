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

#ifndef OpenRTI_AbstractSocketEvent_h
#define OpenRTI_AbstractSocketEvent_h

#include <list>
#include "Clock.h"
#include "Exception.h"
#include "Referenced.h"
#include "SharedPtr.h"

namespace OpenRTI {

class AbstractSocketEvent;
class Socket;
class SocketEventDispatcher;

typedef std::list<SharedPtr<AbstractSocketEvent> > SocketEventList;

class OPENRTI_API AbstractSocketEvent : public Referenced {
public:
  AbstractSocketEvent();
  virtual ~AbstractSocketEvent() noexcept = default;

  // Is called from the parent protocol layer when there is data to read
  virtual void read(SocketEventDispatcher& dispatcher) = 0;
  virtual bool getEnableRead() const = 0;

  // Is called from the parent protocol layer when there is space to write something
  virtual void write(SocketEventDispatcher& dispatcher) = 0;
  virtual bool getEnableWrite() const = 0;

  virtual size_t getBytesQueued() const = 0;

  // Is called when an unrecoverable error happens.
  virtual void error(const Exception& e) = 0;

  virtual Socket* getSocket() const = 0;

private:
  /// The event dispatcher this event is attached to
  SocketEventDispatcher* _socketEventDispatcher;
  /// If attached, the iterator into the dispatchers SocketEventList
  SocketEventList::iterator _iterator;
  friend class SocketEventDispatcher;
};

} // namespace OpenRTI

#endif
