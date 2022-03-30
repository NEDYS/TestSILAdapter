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

#ifndef OpenRTI_AbstractMessageReceiver_h
#define OpenRTI_AbstractMessageReceiver_h

#include "Referenced.h"
#include "SharedPtr.h"
#include "AbstractNotificationHandle.h"
#include <memory>

namespace OpenRTI {

class AbstractMessage;
class Clock;
class AbsTimeout;

template<typename T>
struct NotificationHandleWrapper final : public AbstractNotificationHandle
{
  NotificationHandleWrapper(T* handle) 
  {
    _notificationHandleImpl = handle;
  }
  ~NotificationHandleWrapper() noexcept {}
  void Signal() override
  {
    if (_notificationHandleImpl != nullptr)
    {
      _notificationHandleImpl->Signal();
    }
  }
  // pointer borrowed from application
  T* _notificationHandleImpl;
};

class OPENRTI_API AbstractMessageReceiver : public Referenced {
public:
  virtual ~AbstractMessageReceiver() noexcept {}
  virtual SharedPtr<const AbstractMessage> receive() = 0;
  virtual SharedPtr<const AbstractMessage> receive(const AbsTimeout& timeout) = 0;
  virtual bool empty() const = 0;
  virtual bool isOpen() const = 0;
  virtual size_t getBytesQueued() const = 0;
  virtual void setNotificationHandle(std::shared_ptr<AbstractNotificationHandle> h) = 0;
};

} // namespace OpenRTI

#endif
