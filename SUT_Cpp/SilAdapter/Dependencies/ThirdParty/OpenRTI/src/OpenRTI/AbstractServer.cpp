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

#include "DebugNew.h"
#include "AbstractServer.h"

#include "Exception.h"
#include "Condition.h"
#include "MessageQueue.h"
#include "Mutex.h"
#include "ScopeLock.h"

namespace OpenRTI {

AbstractServer::_Operation::~_Operation()
{
}

void
AbstractServer::_Queue::swap(_Queue& queue)
{
  _messageConnectHandlePairList.swap(queue._messageConnectHandlePairList);
  _operationList.swap(queue._operationList);
}

bool
AbstractServer::_Queue::empty() const
{
  return _messageConnectHandlePairList.empty();
}

void
AbstractServer::_Queue::send(AbstractServer& serverLoop)
{
  for (auto& messageConnectHandlePair: _messageConnectHandlePairList) {
    if (messageConnectHandlePair.first.valid()) {
      serverLoop._sendMessage(messageConnectHandlePair);
      messageConnectHandlePair.first.reset();
    } else {
      serverLoop._sendOperation(*_operationList.front());
      _operationList.pop_front();
    }
  }
}

void
AbstractServer::_Queue::push_back(const _MessageConnectHandlePair& messageConnectHandlePair, _MessageConnectHandlePairList& pool)
{
  if (pool.empty()) {
    _messageConnectHandlePairList.push_back(messageConnectHandlePair);
  } else {
    _messageConnectHandlePairList.splice(_messageConnectHandlePairList.end(), pool, pool.begin());
    _messageConnectHandlePairList.back() = messageConnectHandlePair;
  }
}

void
AbstractServer::_Queue::push_back(const SharedPtr<_Operation>& operation, _MessageConnectHandlePairList& pool)
{
  push_back(_MessageConnectHandlePair(), pool);
  _operationList.push_back(operation);
}

void
AbstractServer::_Queue::recycle(_MessageConnectHandlePairList& pool)
{
  pool.splice(pool.begin(), _messageConnectHandlePairList);
}

class OPENRTI_LOCAL AbstractServer::_Connect : public AbstractConnect {
public:
  _Connect(const SharedPtr<AbstractMessageSender>& messageSender,
           const SharedPtr<AbstractMessageReceiver>& messageReceiver);
  virtual ~_Connect() noexcept;

  AbstractMessageSender* getMessageSender() override;
  AbstractMessageReceiver* getMessageReceiver() override;

private:
  SharedPtr<AbstractMessageSender> _messageSender;
  SharedPtr<AbstractMessageReceiver> _messageReceiver;
};

AbstractServer::_Connect::_Connect(const SharedPtr<AbstractMessageSender>& messageSender,
                                   const SharedPtr<AbstractMessageReceiver>& messageReceiver) :
  _messageSender(messageSender),
  _messageReceiver(messageReceiver)
{
}

AbstractServer::_Connect::~_Connect() noexcept
{
  if (_messageSender.valid())
    _messageSender->close();
  _messageSender.reset();
  _messageReceiver.reset();
}

AbstractMessageSender*
AbstractServer::_Connect::getMessageSender()
{
  return _messageSender.get();
}

AbstractMessageReceiver*
AbstractServer::_Connect::getMessageReceiver()
{
  return _messageReceiver.get();
}

// an AbstractConnect without a receiver
class OPENRTI_LOCAL AbstractServer::_OneWayConnect : public AbstractConnect {
public:
  _OneWayConnect(const SharedPtr<AbstractMessageSender>& messageSender);
  virtual ~_OneWayConnect();

  AbstractMessageSender* getMessageSender() override;
  AbstractMessageReceiver* getMessageReceiver() override;

private:
  SharedPtr<AbstractMessageSender> _messageSender;
};

AbstractServer::_OneWayConnect::_OneWayConnect(const SharedPtr<AbstractMessageSender>& messageSender)
  : _messageSender(messageSender)
{
}

AbstractServer::_OneWayConnect::~_OneWayConnect()
{
  /*
  if (_messageSender.valid())
    _messageSender->close();
  */
  _messageSender.reset();
}

AbstractMessageSender*
AbstractServer::_OneWayConnect::getMessageSender()
{
  return _messageSender.get();
}

AbstractMessageReceiver*
AbstractServer::_OneWayConnect::getMessageReceiver()
{
  return nullptr;
}

class OPENRTI_LOCAL AbstractServer::_ConnectOperation final : public AbstractServer::_Operation {
public:
  _ConnectOperation(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions);
  virtual ~_ConnectOperation();

  void operator()(AbstractServer& serverLoop) override;
  void wait();
  const ConnectHandle& getConnectHandle() const;

private:
  SharedPtr<AbstractMessageSender> _messageSender;
  StringStringListMap _clientOptions;

  Mutex _mutex;
  Condition _condition;
  bool _done;

  ConnectHandle _connectHandle;
};

AbstractServer::_ConnectOperation::_ConnectOperation(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions) :
  _messageSender(messageSender),
  _clientOptions(clientOptions),
  _done(false)
{
}

AbstractServer::_ConnectOperation::~_ConnectOperation()
{
}

void
AbstractServer::_ConnectOperation::operator()(AbstractServer& serverLoop)
{
  _connectHandle = serverLoop._sendConnect(_messageSender, _clientOptions, false /*parent*/);

  ScopeLock scopeLock(_mutex);
  _done = true;
  _condition.notify_one();
}

void
AbstractServer::_ConnectOperation::wait()
{
  ScopeLock scopeLock(_mutex);
  while (!_done)
    _condition.wait(scopeLock);
}

const ConnectHandle&
AbstractServer::_ConnectOperation::getConnectHandle() const
{
  return _connectHandle;
}

class OPENRTI_LOCAL AbstractServer::_DisconnectOperation final : public AbstractServer::_Operation {
public:
  _DisconnectOperation(const ConnectHandle& connectHandle);
  virtual ~_DisconnectOperation();
  void operator()(AbstractServer& serverLoop) override;

private:
  ConnectHandle _connectHandle;
};

AbstractServer::_DisconnectOperation::_DisconnectOperation(const ConnectHandle& connectHandle) :
  _connectHandle(connectHandle)
{
}

AbstractServer::_DisconnectOperation::~_DisconnectOperation()
{
}

void
AbstractServer::_DisconnectOperation::operator()(AbstractServer& serverLoop)
{
  serverLoop._sendDisconnect(_connectHandle);
}

class OPENRTI_LOCAL AbstractServer::_DoneOperation : public AbstractServer::_Operation {
public:
  _DoneOperation();
  virtual ~_DoneOperation();
  void operator()(AbstractServer& serverLoop) override;
};

AbstractServer::_DoneOperation::_DoneOperation()
{
}

AbstractServer::_DoneOperation::~_DoneOperation()
{
}

void
AbstractServer::_DoneOperation::operator()(AbstractServer& serverLoop)
{
  serverLoop._sendDone(true);
}

class OPENRTI_LOCAL AbstractServer::_PostingMessageSender : public AbstractMessageSender {
public:
  _PostingMessageSender(const SharedPtr<AbstractServer>& serverLoop, const ConnectHandle& connectHandle);
  virtual ~_PostingMessageSender();

  virtual void send(const SharedPtr<const AbstractMessage>& message) override;
  virtual void close() noexcept override;
  AbstractServer* getServer() const override { return _serverLoop.get(); }
private:
  SharedPtr<AbstractServer> _serverLoop;
  ConnectHandle _connectHandle;
};

AbstractServer::_PostingMessageSender::_PostingMessageSender(const SharedPtr<AbstractServer>& serverLoop, const ConnectHandle& connectHandle) :
  _serverLoop(serverLoop),
  _connectHandle(connectHandle)
{
  OpenRTIAssert(serverLoop.valid());
}

AbstractServer::_PostingMessageSender::~_PostingMessageSender()
{
}

void
AbstractServer::_PostingMessageSender::send(const SharedPtr<const AbstractMessage>& message)
{
  if (!_connectHandle.valid())
    return;
  //DebugPrintf("%s: message=%s\n", __FUNCTION__, message.valid() ? message->toString().c_str() : "null");
  _serverLoop->_postMessage(_MessageConnectHandlePair(message, _connectHandle));
}

void
AbstractServer::_PostingMessageSender::close() noexcept
{
  if (!_connectHandle.valid())
    return;
  _serverLoop->_postDisconnect(_connectHandle);
  _connectHandle = ConnectHandle();
}

class OPENRTI_LOCAL AbstractServer::_SendingMessageSender : public AbstractMessageSender {
public:
  _SendingMessageSender(const SharedPtr<AbstractServerNode>& serverNode, const ConnectHandle& connectHandle);
  virtual ~_SendingMessageSender() noexcept;
  virtual void send(const SharedPtr<const AbstractMessage>& message) override;
  virtual void close() noexcept override;
  //void setConnectHandle(ConnectHandle handle) { _connectHandle = handle; }
  virtual AbstractServer* getServer() const override
  {
    return _serverNode->getServer();
  }
private:
  SharedPtr<AbstractServerNode> _serverNode;
  ConnectHandle _connectHandle;
};

AbstractServer::_SendingMessageSender::_SendingMessageSender(const SharedPtr<AbstractServerNode>& serverNode, const ConnectHandle& connectHandle) :
  _serverNode(serverNode),
  _connectHandle(connectHandle)
{
}

AbstractServer::_SendingMessageSender::~_SendingMessageSender() noexcept
{
  close();
}

void
AbstractServer::_SendingMessageSender::send(const SharedPtr<const AbstractMessage>& message)
{
  if (!_serverNode.valid())
    throw RTIinternalError("Trying to send message to a closed MessageSender");
  if (!message.valid())
    return;
  _serverNode->_dispatchMessage(message.get(), _connectHandle);
}

void
AbstractServer::_SendingMessageSender::close() noexcept
{
  if (!_serverNode.valid())
    return;
  _serverNode->_eraseConnect(_connectHandle);
  _serverNode.reset();
  _connectHandle = ConnectHandle();
}

AbstractServer::AbstractServer(const SharedPtr<AbstractServerNode>& serverNode) :
  _serverNode(serverNode),
  _done(false)
{
  OpenRTIAssert(_serverNode.valid());
  serverNode->setServer(this);
}

AbstractServer::~AbstractServer() noexcept
{
}

const AbstractServerNode&
AbstractServer::getServerNode() const noexcept
{
  return *_serverNode;
}

AbstractServerNode&
AbstractServer::getServerNode()
{
  return *_serverNode;
}

SharedPtr<AbstractConnect>
AbstractServer::postConnect(const StringStringListMap& clientOptions)
{
  SharedPtr<ThreadMessageQueue> messageQueue = MakeShared<ThreadMessageQueue>();
  ConnectHandle connectHandle = _postConnect(messageQueue->getMessageSender(), clientOptions);
  if (!connectHandle.valid())
    return SharedPtr<AbstractConnect>();
  SharedPtr<AbstractMessageSender> messageSender;
  messageSender = MakeShared<_PostingMessageSender>(SharedPtr<AbstractServer>(this), connectHandle);
  return MakeShared<_Connect>(messageSender, messageQueue);
}

SharedPtr<AbstractConnect>
AbstractServer::sendConnect(const StringStringListMap& optionMap, bool parent)
{
  SharedPtr<LocalMessageQueue> messageQueue = MakeShared<LocalMessageQueue>();
  ConnectHandle connectHandle = _sendConnect(SharedPtr<AbstractMessageSender>(messageQueue->getMessageSender()), optionMap, parent);
  if (!connectHandle.valid())
    return SharedPtr<AbstractConnect>();
  SharedPtr<AbstractMessageSender> messageSender;
  messageSender = MakeShared<_SendingMessageSender>(SharedPtr<AbstractServerNode>(&getServerNode()), connectHandle);
  return MakeShared<_Connect>(messageSender, messageQueue);
}

SharedPtr<AbstractConnect>
AbstractServer::sendDirectConnect(SharedPtr<AbstractMessageSender> sender, const StringStringListMap& optionMap)
{
  ConnectHandle connectHandle = getServerNode()._insertConnect(sender, optionMap);
  if (!connectHandle.valid())
    return SharedPtr<AbstractConnect>();
  SharedPtr<AbstractMessageSender> messageSender;
  messageSender = MakeShared<_SendingMessageSender>(SharedPtr<AbstractServerNode>(&getServerNode()), connectHandle);
  return MakeShared<_OneWayConnect>(messageSender);
}

ConnectHandle
AbstractServer::_sendConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions, bool parent)
{
  OpenRTIAssert(messageSender.valid());
  if (getDone())
    return ConnectHandle();
  if (parent) {
    return getServerNode()._insertParentConnect(messageSender, clientOptions);
  } else {
    return getServerNode()._insertConnect(messageSender, clientOptions);
  }
}

void
AbstractServer::_sendEraseConnect(const ConnectHandle& connectHandle)
{
  OpenRTIAssert(connectHandle.valid());
  getServerNode()._eraseConnect(connectHandle);
}

void
AbstractServer::_sendDisconnect(const ConnectHandle& connectHandle)
{
  _sendEraseConnect(connectHandle);
  if (getServerNode().isIdle())
    setDone(true);
}

void
AbstractServer::_sendMessage(const _MessageConnectHandlePair& messageConnectHandlePair)
{
  OpenRTIAssert(messageConnectHandlePair.first.valid());
  OpenRTIAssert(messageConnectHandlePair.second.valid());
  getServerNode()._dispatchMessage(messageConnectHandlePair.first.get(), messageConnectHandlePair.second);
}

void
AbstractServer::_sendOperation(_Operation& operation)
{
  operation(*this);
}

void
AbstractServer::_sendDone(bool done)
{
  _done = done;
}

ConnectHandle
AbstractServer::_postConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions)
{
  SharedPtr<_ConnectOperation> connectOperation;
  connectOperation = MakeShared<_ConnectOperation>(messageSender, clientOptions);
  _postOperation(connectOperation);
  connectOperation->wait();
  return connectOperation->getConnectHandle();
}

void
AbstractServer::_postDisconnect(const ConnectHandle& connectHandle)
{
  if (!connectHandle.valid())
    return;
  SharedPtr<_DisconnectOperation> disconnectOperation;
  disconnectOperation = MakeShared<_DisconnectOperation>(connectHandle);
  _postOperation(disconnectOperation);
}

void
AbstractServer::_postDone()
{
  _postOperation(MakeShared<_DoneOperation>());
}

} // namespace OpenRTI
