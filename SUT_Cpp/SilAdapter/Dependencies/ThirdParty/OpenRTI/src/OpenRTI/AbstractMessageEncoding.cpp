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

#include "DebugNew.h"
#include "AbstractMessageEncoding.h"

#include "Message.h"

namespace OpenRTI {

bool
AbstractMessageEncoding::getEnableRead() const noexcept
{
  return true;
}

void
AbstractMessageEncoding::writePacket()
{
  SharedPtr<const AbstractMessage> message = _connect->receive();
  if (!message.valid())
    return;
  writeMessage(*message);
}

bool AbstractMessageEncoding::getMoreToSend() const
{
  return !_connect->empty();
}

size_t AbstractMessageEncoding::getBytesQueued() const
{
  size_t bytesQueued = _connect->getMessageReceiver()->getBytesQueued();
  size_t bytesBuffered = getBytesBuffered();
  //DebugPrintf("%s: bytesQueued=%d bytesBuffered=%d\n", __FUNCTION__, bytesQueued, bytesBuffered);
  return bytesQueued + bytesBuffered;
}

void
AbstractMessageEncoding::error(const Exception& e)
{
  /// FIXME!!!, just have a ConnectionClosed message. Depending on the server nodes state, it can decide what to do
  SharedPtr<ConnectionLostMessage> message = MakeShared<ConnectionLostMessage>();
  message->setFaultDescription(e.getReason());
  _connect->send(message);
  _connect->close();
}

} // namespace OpenRTI
