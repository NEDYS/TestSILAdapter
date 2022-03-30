/* -*-c++-*- OpenRTI - Copyright (C) 2009-2015 Mathias Froehlich
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

#ifndef OpenRTI_MessageQueue_h
#define OpenRTI_MessageQueue_h

#include "AbstractMessageQueue.h"
#include "Condition.h"
#include "PooledMessageList.h"
#include "Mutex.h"
#include "ScopeLock.h"
#include "AbsTimeout.h"

//#include "dprintf.h"

namespace OpenRTI {

// Unlocked queue implementation,
// For single threaded use.
// The timeout in the receive call is just ignored since
// single threaded use does not have any chance to fill the queue
// if that single thread is waiting for exactly that queue.
class OPENRTI_LOCAL LocalMessageQueue final : public AbstractMessageQueue {
public:
  LocalMessageQueue() :
    _isClosed(false)
  { }
  SharedPtr<const AbstractMessage> receive() override
  {
    return _messageList.pop_front();
  }
  SharedPtr<const AbstractMessage> receive(const AbsTimeout&) override
  {
    return _messageList.pop_front();
  }
  bool isOpen() const override { return !_isClosed; }
  bool empty() const override { return _messageList.empty(); }
  virtual size_t getBytesQueued() const override { return _messageList.byteSize(); }
  void setNotificationHandle(std::shared_ptr<AbstractNotificationHandle>) override
  {
    assert(!"not implemented");
  }

protected:
  void append(const SharedPtr<const AbstractMessage>& message) override
  {
    //CondDebugPrintf("%s: message=%s\n", __FUNCTION__, message->toString().c_str());
    _messageList.push_back(message);
  }
  void close() override { _isClosed = true; }

private:
  PooledMessageList _messageList;
  bool _isClosed;
};

// Thread safe queue with condition/mutex based signaling of new messages
class OPENRTI_LOCAL ThreadMessageQueue final : public AbstractMessageQueue {
public:
  ThreadMessageQueue() :
    _isClosed(false)
  { }
  virtual SharedPtr<const AbstractMessage> receive() override
  {
    ScopeLock scopeLock(_mutex);
    if (_messageList.empty())
      return SharedPtr<const AbstractMessage>();
    SharedPtr<const AbstractMessage> message = _messageList.pop_front();
    //DebugPrintf("%s: message=%s\n", __FUNCTION__, message->toString().c_str());
    return message;
  }
  virtual SharedPtr<const AbstractMessage> receive(const AbsTimeout& timeout) override
  {
    ScopeLock scopeLock(_mutex);
    while (_messageList.empty()) {
      // We must not rely on the timeout return before checking the predicate.
      bool signaledOrSpurious = _condition.wait_until(scopeLock, timeout);
      if (!_messageList.empty())
        break;
      if (_isClosed)
        return SharedPtr<const AbstractMessage>();
      // Timeout was hit
      if (!signaledOrSpurious)
        return SharedPtr<const AbstractMessage>();
    }
    SharedPtr<const AbstractMessage> message = _messageList.pop_front();
    //DebugPrintf("%s: message=%s\n", __FUNCTION__, message->toString().c_str());
    return message;
  }
  virtual bool isOpen() const override
  {
    ScopeLock scopeLock(_mutex);
    return !_isClosed;
  }
  virtual bool empty() const override
  {
    ScopeLock scopeLock(_mutex);
    return _messageList.empty();
  }
  virtual size_t getBytesQueued() const override
  {
    ScopeLock scopeLock(_mutex);
    return _messageList.byteSize();
  }
protected:
  virtual void setNotificationHandle(std::shared_ptr<AbstractNotificationHandle> h) override
  {
    _notificationHandle = h;
    if (_notificationHandle != nullptr)
    {
      ScopeLock scopeLock(_mutex);
      if (!_messageList.empty())
      {
        //CondDebugPrintf("%s: signal _notificationHandle=%p\n", __FUNCTION__, _notificationHandle.get());
        _notificationHandle->Signal();
      }
    }
  }
  void append(const SharedPtr<const AbstractMessage>& message) override
  {
    ScopeLock scopeLock(_mutex);
    bool needSignal = _messageList.empty();
    _messageList.push_back(message);
    if (needSignal)
    {
      _condition.notify_one();
    }
    //CondDebugPrintf("%s: signal _notificationHandle=%p empty=%d needSignal=%d\n", __FUNCTION__,
    //            _notificationHandle.get(), _messageList.empty(), needSignal);
    if (!_messageList.empty() && _notificationHandle != nullptr)
    {
      _notificationHandle->Signal();
    }
  }
  void close() override
  {
    ScopeLock scopeLock(_mutex);
    _isClosed = true;
    _condition.notify_one();
  }

private:
  mutable Mutex _mutex;
  Condition _condition;
  PooledMessageList _messageList;
  bool _isClosed;
  std::shared_ptr<AbstractNotificationHandle> _notificationHandle;
};

} // namespace OpenRTI

#endif
