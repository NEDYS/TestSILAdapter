/* -*-c++-*- OpenRTI - Copyright (C) 2009-2013 Mathias Froehlich
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
#include "InternalAmbassador.h"

#include "InternalTimeManagement.h"
#include "Federate.h"
#include "Exception.h"
#include "Export.h"
#include "LogStream.h"
#include "LeafServerThread.h"
#include "Message.h"

#include "dprintf.h"
#include "AbstractServer.h"
#include "ThreadLocal.h"
#include "NetworkServer.h"
#include "AbsTimeout.h"
#include "ImmediateCallbackDispatcher.h"

namespace OpenRTI {

class OPENRTI_LOCAL InternalAmbassador::_InternalMessageDispatchFunctor {
public:
  _InternalMessageDispatchFunctor(InternalAmbassador& basicAmbassador) :
    _basicAmbassador(basicAmbassador)
  { }
  template<typename M>
  void operator()(const M& message) const
  { _basicAmbassador.acceptInternalMessage(message); }
private:
  InternalAmbassador& _basicAmbassador;
};

InternalAmbassador::InternalAmbassador()
  : _callbacksEnabled(true)
{
}


void InternalAmbassador::enableImmediateProcessing()
{
  _callbackModel = HLA_IMMEDIATE;
  _immediateCallbackDispatcher = MakeShared<ImmediateCallbackDispatcher>(*this);
  _immediateCallbackDispatcher->start();
}


void InternalAmbassador::disableImmediateProcessing()
{
  _immediateCallbackDispatcher->stop();
  _immediateCallbackDispatcher->wait();
  _callbackModel = HLA_EVOKED;
}

InternalAmbassador::~InternalAmbassador() noexcept
{
  try
  {
    if (_connect.valid())
      _connect->close();
  }
  catch (Exception& e)
  {
    DebugPrintf("%s: %s\n", __FUNCTION__, e.getReason().c_str());
  }
  _connect.reset();
}

bool
InternalAmbassador::isConnected() const
{
  return _connect.valid();
}

void
InternalAmbassador::connect(const URL& url, const StringStringListMap& parameterMap, uint32_t timeoutMilliSeconds)
{
  _connect = LeafServerThread::connect(url, parameterMap, timeoutMilliSeconds);
}

void
InternalAmbassador::disconnect()
{
  OpenRTIAssert(_connect.valid());
  _connect->close();
  _connect.reset();
}


void InternalAmbassador::shutdown()
{
  LeafServerThread::shutdown();
  AbstractThreadLocal::shutdown();
}

void
InternalAmbassador::send(const SharedPtr<const AbstractMessage>& message)
{
  OpenRTIAssert(_connect.valid());
  _connect->send(message);
}

bool
InternalAmbassador::receiveAndDispatchInternalMessage(const AbsTimeout& timeout)
{
  _InternalMessageDispatchFunctor functor(*this);
  return receiveAndDispatch(timeout, functor);
}

bool
InternalAmbassador::_receiveAndDispatch(const AbsTimeout& timeout, const AbstractMessageDispatcher& dispatcher)
{
  OpenRTIAssert(_connect.valid());
  SharedPtr<const AbstractMessage> message = _connect->receive(timeout);
  if (!message.valid())
    return false;
  message->dispatch(dispatcher);
  return true;
}

bool
InternalAmbassador::receiveAndDispatchInternalMessage()
{
  _InternalMessageDispatchFunctor functor(*this);
  return receiveAndDispatch(functor);
}

bool
InternalAmbassador::_receiveAndDispatch(const AbstractMessageDispatcher& dispatcher)
{
  if (!_connect.valid())
    return false;
  SharedPtr<const AbstractMessage> message = _connect->receive();
  if (!message.valid())
    return false;
  message->dispatch(dispatcher);
  return true;
}

void
InternalAmbassador::flushAndDispatchInternalMessage()
{
  _InternalMessageDispatchFunctor functor(*this);
  flushReceiveAndDispatch(functor);
}

void
InternalAmbassador::_flushReceiveAndDispatch(const AbstractMessageDispatcher& dispatcher)
{
  for (;;) {
    if (!_connect.valid())
      break;
    SharedPtr<const AbstractMessage> message = _connect->receive();
    if (!message.valid())
      return;
    message->dispatch(dispatcher);
  }
}

void
InternalAmbassador::acceptInternalMessage(const AbstractMessage& message)
{
  std::string msg = std::string("Unexpected message in internal message processing!") + message.toString();
  throw RTIinternalError(msg);
}

void
InternalAmbassador::acceptInternalMessage(const ConnectionLostMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const EnumerateFederationExecutionsResponseMessage& message)
{
  queueCallback(message);
}

// void
// InternalAmbassador::acceptInternalMessage(const InsertFederationExecutionMessage& message)
// {
// }

void
InternalAmbassador::acceptInternalMessage(const ShutdownFederationExecutionMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const DestroyFederationExecutionResponseMessage& /*message*/)
{
  // spurious, after disconnect?
}

void
InternalAmbassador::acceptInternalMessage(const EraseFederationExecutionMessage& /*message*/)
{
  // FIXME?!
  // deleteFederate();
}

void
InternalAmbassador::acceptInternalMessage(const ReleaseFederationHandleMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const InsertModulesMessage& message)
{
  Federate* federate = getFederate();
  if (!federate)
    return;
  federate->insertFOMModuleList(message.getFOMModuleList());
}

void
InternalAmbassador::acceptInternalMessage(const InsertModules2Message& message)
{
  Federate* federate = getFederate();
  if (!federate)
    return;
  federate->insertFOMModule2List(message.getFOMModule2List());
}

void
InternalAmbassador::acceptInternalMessage(const JoinFederateNotifyMessage& message)
{
  if (Federate* federate = getFederate())
    federate->insertFederate(message.getFederateHandle(), message.getFederateName());
  if (InternalTimeManagement* timeManagement = getTimeManagement())
  {
    OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
    timeManagement->acceptInternalMessage(*this, message);
  }
}

void
InternalAmbassador::acceptInternalMessage(const ResignFederateNotifyMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
  {
    OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
    timeManagement->acceptInternalMessage(*this, message);
  }
  if (Federate* federate = getFederate())
    federate->eraseFederate(message.getFederateHandle());
}

void
InternalAmbassador::acceptInternalMessage(const ChangeAutomaticResignDirectiveMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const RegisterFederationSynchronizationPointResponseMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const AnnounceSynchronizationPointMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const SynchronizationPointAchievedMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const FederationSynchronizedMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const EnableTimeRegulationRequestMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
  {
    OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
    timeManagement->acceptInternalMessage(*this, message);
  }
}

void
InternalAmbassador::acceptInternalMessage(const EnableTimeRegulationResponseMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
  {
    OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
    timeManagement->acceptInternalMessage(*this, message);
  }
}

void
InternalAmbassador::acceptInternalMessage(const DisableTimeRegulationRequestMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
  {
    OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
    timeManagement->acceptInternalMessage(*this, message);
  }
}

void
InternalAmbassador::acceptInternalMessage(const EnableTimeConstrainedNotifyMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const DisableTimeConstrainedNotifyMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const CommitLowerBoundTimeStampMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
  {
    OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
    timeManagement->acceptInternalMessage(*this, message);
  }
}

void
InternalAmbassador::acceptInternalMessage(const CommitLowerBoundTimeStampResponseMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
  {
    OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
    timeManagement->acceptInternalMessage(*this, message);
  }
}

void
InternalAmbassador::acceptInternalMessage(const LockedByNextMessageRequestMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
  {
    OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
    timeManagement->acceptInternalMessage(*this, message);
  }
}

void
InternalAmbassador::acceptInternalMessage(const InsertRegionMessage& message)
{
  Federate* federate = getFederate();
  if (!federate)
    return;
  federate->insertRegions(message.getRegionHandleDimensionHandleSetPairVector());
}

void
InternalAmbassador::acceptInternalMessage(const CommitRegionMessage& message)
{
  Federate* federate = getFederate();
  if (!federate)
    return;
  federate->commitRegions(message.getRegionHandleRegionValuePairVector());
}

void
InternalAmbassador::acceptInternalMessage(const EraseRegionMessage& message)
{
  Federate* federate = getFederate();
  if (!federate)
    return;
  federate->eraseRegions(message.getRegionHandleVector());
}

void
InternalAmbassador::acceptInternalMessage(const ChangeInteractionClassPublicationMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const ChangeObjectClassPublicationMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const ChangeInteractionClassSubscriptionMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const ChangeObjectClassSubscriptionMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const ChangeObjectInstanceSubscriptionMessage& message)
{
  // probably the wrong place to do this -
  // maybe this should only get through when there is no subscriber left at all.

  ObjectClassHandle objectClassHandle = message.getObjectClassHandle();
  ObjectInstanceHandle objectInstanceHandle = message.getObjectInstanceHandle();
  const Federate::ObjectClass* objectClass = getFederate()->getObjectClass(objectClassHandle);
  Federate::ObjectInstance* objectInstance = getFederate()->getObjectInstance(objectInstanceHandle);

  // The object or class could be removed meanwhile
  if (!objectInstance)
    return;
  if (!objectClass)
    return;

  SubscriptionType subscriptionType = message.getSubscriptionType();
  if (objectInstance->setSubscriptionType(subscriptionType))
  {
    SharedPtr<TurnUpdatesOnForInstanceMessage> updatesMessage = MakeShared<TurnUpdatesOnForInstanceMessage>();
    updatesMessage->setObjectInstanceHandle(message.getObjectInstanceHandle());
    updatesMessage->setAttributeHandles(objectClass->getAttributeHandles());
    updatesMessage->setOn(subscriptionType != Unsubscribed);
    // message->setUpdateRate(/*FIXME*/);
    queueCallback(updatesMessage);
  }
}

void
InternalAmbassador::acceptInternalMessage(const RegistrationForObjectClassMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const TurnInteractionsOnMessage& /*message*/)
{
}

void
InternalAmbassador::acceptInternalMessage(const InteractionMessage& message)
{
  queueReceiveOrderMessage(message);
}

void
InternalAmbassador::acceptInternalMessage(const TimeStampedInteractionMessage& message)
{
  if (message.getOrderType() == TIMESTAMP)
    queueTimeStampedMessage(message.getTimeStamp(), message);
  else
    queueReceiveOrderMessage(message);
}

void
InternalAmbassador::acceptInternalMessage(const ObjectInstanceHandlesResponseMessage& message)
{
  Federate* federate = getFederate();
  if (!federate)
    return;
  federate->insertObjectInstanceHandleNamePairs(message.getObjectInstanceHandleNamePairVector());
}

void
InternalAmbassador::acceptInternalMessage(const ReserveObjectInstanceNameResponseMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const ReserveMultipleObjectInstanceNameResponseMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const InsertObjectInstanceMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const DeleteObjectInstanceMessage& message)
{
  queueReceiveOrderMessage(message);
}

void
InternalAmbassador::acceptInternalMessage(const TimeStampedDeleteObjectInstanceMessage& message)
{
  if (message.getOrderType() == TIMESTAMP)
    queueTimeStampedMessage(message.getTimeStamp(), message);
  else
    queueReceiveOrderMessage(message);
}

void
InternalAmbassador::acceptInternalMessage(const AttributeUpdateMessage& message)
{
  queueReceiveOrderMessage(message);
}

void
InternalAmbassador::acceptInternalMessage(const TimeStampedAttributeUpdateMessage& message)
{
  if (message.getOrderType() == TIMESTAMP)
    queueTimeStampedMessage(message.getTimeStamp(), message);
  else
    queueReceiveOrderMessage(message);
}

void
InternalAmbassador::acceptInternalMessage(const RequestAttributeUpdateMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const RequestClassAttributeUpdateMessage& message)
{
  Federate* federate = getFederate();
  OpenRTIAssert(federate);

  for (Federate::ObjectInstanceHandleMap::const_iterator i = federate->getObjectInstanceHandleMap().begin();
       i != federate->getObjectInstanceHandleMap().end(); ++i) {
    const Federate::ObjectInstance* objectInstance = i->second.get();
    if (!objectInstance)
      continue;
    // FIXME have a list of object instances by class
    ObjectClassHandle objectClassHandle = objectInstance->getObjectClassHandle();
    while (objectClassHandle.valid()) {
      if (objectClassHandle == message.getObjectClassHandle())
        break;
      const Federate::ObjectClass* objectClass = federate->getObjectClass(objectClassHandle);
      OpenRTIAssert(objectClass);
      objectClassHandle = objectClass->getParentObjectClassHandle();
    }
    if (!objectClassHandle.valid())
      continue;

    SharedPtr<RequestAttributeUpdateMessage> request;
    request = MakeShared<RequestAttributeUpdateMessage>();
    request->setFederationHandle(federate->getFederationHandle());
    request->setObjectInstanceHandle(i->first);
    request->setAttributeHandles(message.getAttributeHandles());
    request->setTag(message.getTag());
    queueCallback(request);
  }
}


void InternalAmbassador::acceptInternalMessage(const QueryAttributeOwnershipRequestMessage& /*message*/)
{

}

class OPENRTI_LOCAL InternalAmbassador::_CreateFederationExecutionFunctor {
public:
  _CreateFederationExecutionFunctor(InternalAmbassador& basicAmbassador) :
    _done(false),
    _responseTypeStringPair(CreateFederationExecutionResponseTimeout, "Timeout while waiting for CreateFederationExecutionResponse"),
    _basicAmbassador(basicAmbassador)
  { }
  void operator()(const ConnectionLostMessage& message)
  {
    _basicAmbassador.acceptInternalMessage(message);
    _responseTypeStringPair.first = CreateFederationExecutionResponseRTIinternalError;
    _responseTypeStringPair.second = message.getFaultDescription();
    _done = true;
  }
  void operator()(const CreateFederationExecutionResponseMessage& message)
  {
    _responseTypeStringPair.first = message.getCreateFederationExecutionResponseType();
    _responseTypeStringPair.second = message.getExceptionString();
    _done = true;
  }
  template<typename M>
  void operator()(const M& message)
  { _basicAmbassador.acceptInternalMessage(message); }

  bool _done;
  std::pair<CreateFederationExecutionResponseType, std::string> _responseTypeStringPair;

private:
  InternalAmbassador& _basicAmbassador;
};

std::pair<CreateFederationExecutionResponseType, std::string>
InternalAmbassador::dispatchWaitCreateFederationExecutionResponse(const AbsTimeout& timeout)
{
  _CreateFederationExecutionFunctor functor(*this);
  disableCallbacks();
  while (!functor._done && !timeout.isExpired())
    receiveAndDispatch(timeout, functor);
  enableCallbacks();
  return functor._responseTypeStringPair;
}

class OPENRTI_LOCAL InternalAmbassador::_DestroyFederationExecutionFunctor {
public:
  _DestroyFederationExecutionFunctor(InternalAmbassador& basicAmbassador) :
    _done(false),
    _responseType(DestroyFederationExecutionResponseRTIinternalError),
    _basicAmbassador(basicAmbassador)
  { }
  void operator()(const ConnectionLostMessage& message)
  {
    _basicAmbassador.acceptInternalMessage(message);
    _responseType = DestroyFederationExecutionResponseRTIinternalError;
    _done = true;
  }
  void operator()(const DestroyFederationExecutionResponseMessage& message)
  {
    _responseType = message.getDestroyFederationExecutionResponseType();
    _done = true;
  }
  template<typename M>
  void operator()(const M& message) const
  { _basicAmbassador.acceptInternalMessage(message); }

  bool _done;
  DestroyFederationExecutionResponseType _responseType;

private:
  InternalAmbassador& _basicAmbassador;
};

DestroyFederationExecutionResponseType
InternalAmbassador::dispatchWaitDestroyFederationExecutionResponse(const AbsTimeout& timeout)
{
  _DestroyFederationExecutionFunctor functor(*this);
  disableCallbacks();
  while (!functor._done && !timeout.isExpired())
    receiveAndDispatch(timeout, functor);
  enableCallbacks();
  return functor._responseType;
}

class OPENRTI_LOCAL InternalAmbassador::_JoinFederationExecutionFunctor {
public:
  _JoinFederationExecutionFunctor(InternalAmbassador& basicAmbassador, std::string federateName) :
    _done(false),
    _response(JoinFederationExecutionResponseTimeout, "Timeout while waiting for JoinFederationExecutionResponse" ),
    _basicAmbassador(basicAmbassador), _federateName(federateName)
  { }
  void operator()(const ConnectionLostMessage& message)
  {
    _basicAmbassador.acceptInternalMessage(message);
    _response.first = JoinFederationExecutionResponseFederationExecutionDoesNotExist;
    _response.second = message.getFaultDescription();
    _done = true;
  }
  void operator()(const InsertFederationExecutionMessage& message)
  {
    _basicAmbassador.acceptInternalMessage(message);
  }
  void operator()(const JoinFederationExecutionResponseMessage& message)
  {
    _basicAmbassador.acceptInternalMessage(message);
    if (message.getFederateName() == _federateName || (_federateName.empty() && message.getFederateName().compare(0, 11, "HLAfederate") == 0))
    {
      _response.first = message.getJoinFederationExecutionResponseType();
      _response.second = message.getExceptionString();
      _done = true;
    }
  }
  void operator()(const CreateFederationExecutionResponseMessage& /*message*/)
  {
    // Could be a late response from a second call to createFederation, just ignore this
  }
  template<typename M>
  void operator()(const M& message) const
  { _basicAmbassador.acceptInternalMessage(message); }

  bool _done;
  std::pair<JoinFederationExecutionResponseType, std::string> _response;

private:
  InternalAmbassador& _basicAmbassador;
  std::string _federateName;
};

std::pair<JoinFederationExecutionResponseType, std::string>
InternalAmbassador::dispatchWaitJoinFederationExecutionResponse(const AbsTimeout& timeout, std::string federateName)
{
  _JoinFederationExecutionFunctor functor(*this, federateName);
  disableCallbacks();
  while (!functor._done && !timeout.isExpired())
    receiveAndDispatch(timeout, functor);
  enableCallbacks();
  return functor._response;
}

class OPENRTI_LOCAL InternalAmbassador::_EraseFederationExecutionFunctor {
public:
  _EraseFederationExecutionFunctor(InternalAmbassador& basicAmbassador) :
    _done(false),
    _basicAmbassador(basicAmbassador)
  { }
  void operator()(const ConnectionLostMessage& message)
  {
    _basicAmbassador.acceptInternalMessage(message);
    _done = true;
  }
  void operator()(const EraseFederationExecutionMessage& message)
  {
    SharedPtr<ReleaseFederationHandleMessage> release = MakeShared<ReleaseFederationHandleMessage>();
    release->setFederationHandle(message.getFederationHandle());
    _basicAmbassador.send(release);
    _done = true;
  }
  template<typename M>
  void operator()(const M& message) const
  { }

  bool _done;

private:
  InternalAmbassador& _basicAmbassador;
};

bool
InternalAmbassador::dispatchWaitEraseFederationExecutionResponse(const AbsTimeout& timeout)
{
  _EraseFederationExecutionFunctor functor(*this);
  disableCallbacks();
  while (!functor._done && !timeout.isExpired())
    receiveAndDispatch(timeout, functor);
  enableCallbacks();
  return functor._done;
}


class OPENRTI_LOCAL InternalAmbassador::_ReserveObjectInstanceNameFunctor {
public:
  _ReserveObjectInstanceNameFunctor(InternalAmbassador& basicAmbassador, const std::string& objectInstanceName) :
    _done(false),
    _basicAmbassador(basicAmbassador),
    _objectInstanceName(objectInstanceName)
  { }
  void operator()(const ConnectionLostMessage& message)
  {
    _basicAmbassador.acceptInternalMessage(message);
    _done = true;
  }
  void operator()(const ReserveObjectInstanceNameResponseMessage& message)
  {
    // Intercept only those responses which match our name.
    // The rest is given to the underlying ambassador/federate.
    if (_objectInstanceName == message.getObjectInstanceHandleNamePair().second) {
      _objectInstanceHandle = message.getObjectInstanceHandleNamePair().first;
      _done = true;
    } else {
      _basicAmbassador.acceptInternalMessage(message);
    }
  }
  template<typename M>
  void operator()(const M& message) const
  { _basicAmbassador.acceptInternalMessage(message); }

  bool _done;
  ObjectInstanceHandle _objectInstanceHandle;

private:
  InternalAmbassador& _basicAmbassador;
  std::string _objectInstanceName;
};

ObjectInstanceHandle
InternalAmbassador::dispatchWaitReserveObjectInstanceName(const AbsTimeout& timeout, const std::string& objectInstanceName)
{
  const Federate* federate = getFederate();
  OpenRTIAssert(federate);

  SharedPtr<ReserveObjectInstanceNameRequestMessage> request;
  request = MakeShared<ReserveObjectInstanceNameRequestMessage>();
  request->setFederationHandle(federate->getFederationHandle());
  request->setFederateHandle(federate->getFederateHandle());
  request->setName(objectInstanceName);
  send(request);

  _ReserveObjectInstanceNameFunctor functor(*this, objectInstanceName);
  while (!functor._done && !timeout.isExpired())
    receiveAndDispatch(timeout, functor);

  return functor._objectInstanceHandle;
}

void
InternalAmbassador::queueTimeStampedMessage(const VariableLengthData& timeStamp, const AbstractMessage& message, bool loopback)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
  {
    OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
    timeManagement->queueTimeStampedMessage(*this, timeStamp, message, loopback);
  }
}

void
InternalAmbassador::queueReceiveOrderMessage(const AbstractMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
  {
    OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
    timeManagement->queueReceiveOrderMessage(*this, message);
  }
}

bool
InternalAmbassador::_dispatchCallbackMessage(const AbstractMessageDispatcher& messageDispatcher)
{
  if (!_callbackMessageList.empty()) {
    SharedPtr<const AbstractMessage> message;
    {
      ScopeLock lock(_callbackMessageListMutex);
      message.swap(_callbackMessageList.front());
      _messageListPool.splice(_messageListPool.begin(), _callbackMessageList, _callbackMessageList.begin());
    }
    try
    {
      message->dispatch(messageDispatcher);
    }
    catch (const OpenRTI::Exception& e)
    {
      DebugPrintf("%s: OpenRTI::Exception %s\n", __FUNCTION__, e.what());
    }
    catch (const std::exception& e)
    {
      DebugPrintf("%s: exception %s\n", __FUNCTION__, e.what());
    }
    catch (...)
    {
      DebugPrintf("%s: unknown exception\n", __FUNCTION__);
    }
    return true;
  }
  InternalTimeManagement* timeManagement = getTimeManagement();
  if (timeManagement != nullptr)
  {
    try
    {
      OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
      if (timeManagement->dispatchCallback(messageDispatcher)) {
        return true;
      }
    }
    catch (const OpenRTI::Exception& e)
    {
      DebugPrintf("%s: OpenRTI::Exception %s\n", __FUNCTION__, e.what());
    }
    catch (const std::exception& e)
    {
      DebugPrintf("%s: exception %s\n", __FUNCTION__, e.what());
    }
    catch (...)
    {
      DebugPrintf("%s: unknown exception\n", __FUNCTION__);
    }
  }
  return false;
}

bool
InternalAmbassador::_callbackMessageAvailable()
{
  if (!_callbackMessageList.empty())
    return true;
  OpenRTI::RecursiveScopeLock timeManagementLock(_timeManagementMutex);
  InternalTimeManagement* timeManagement = getTimeManagement();
  if (timeManagement && timeManagement->callbackMessageAvailable())
    return true;
  return false;
}


uint32_t InternalAmbassador::getProtocolVersion() const
{
  const AbstractServer* server = _connect->getMessageSender()->getServer();
  return server->getProtocolVersion();
}

void InternalAmbassador::_setNotificationHandle(std::shared_ptr<AbstractNotificationHandle> h)
{
  if (_connect.valid() && _connect->getMessageReceiver() != nullptr)
  {
    if (_connect->getMessageReceiver() != nullptr)
    {
    _connect->getMessageReceiver()->setNotificationHandle(h);
    }
  }
  _notificationHandle = h;
  if (_notificationHandle != nullptr && !_callbackMessageList.empty())
  {
    _notificationHandle->Signal();
  }
};

}
