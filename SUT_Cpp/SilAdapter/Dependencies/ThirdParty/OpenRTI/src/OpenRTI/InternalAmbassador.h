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

#ifndef OpenRTI_InternalAmbassador_h
#define OpenRTI_InternalAmbassador_h

#include "AbstractConnect.h"
#include "Message.h"
#include "StringUtils.h"
#include "AbsTimeout.h"
#include "Mutex.h"
#include "ScopeLock.h"

namespace OpenRTI {

class InternalTimeManagement;
class Federate;
class URL;
class AbstractServer; 
class ImmediateCallbackDispatcher;

class OPENRTI_API InternalAmbassador {
public:
  InternalAmbassador();
  virtual ~InternalAmbassador() noexcept;

  /// Handle the connection to the ServerNodes.
  bool isConnected() const;
  void connect(const URL& url, const StringStringListMap& parameterMap, uint32_t timeoutMilliSeconds);
  void disconnect();
  void shutdown();
  /// Send a message
  void send(const SharedPtr<const AbstractMessage>& message);

  /// Receive a message and put it into the internal message processing,
  /// Returns true if there is an other pending message.
  bool receiveAndDispatchInternalMessage(const AbsTimeout& timeout);
  template<typename F>
  bool receiveAndDispatch(const AbsTimeout& timeout, F& functor) {
    return _receiveAndDispatch(timeout, FunctorMessageDispatcher<F>(functor));
  }
  template<typename F>
  bool receiveAndDispatch(const AbsTimeout& timeout, const F& functor) {
    return _receiveAndDispatch(timeout, ConstFunctorMessageDispatcher<F>(functor));
  }
  bool _receiveAndDispatch(const AbsTimeout& timeout, const AbstractMessageDispatcher& dispatcher);

  // receiveAndDispatch w.o. timeout, caution: must be used with notification handle
  bool receiveAndDispatchInternalMessage();
  template<typename F>
  bool receiveAndDispatch(F& functor) {
    return _receiveAndDispatch(FunctorMessageDispatcher<F>(functor));
  }
  template<typename F>
  bool receiveAndDispatch(const F& functor) {
    return _receiveAndDispatch(ConstFunctorMessageDispatcher<F>(functor));
  }
  bool _receiveAndDispatch(const AbstractMessageDispatcher& dispatcher);

  void flushAndDispatchInternalMessage();
  template<typename F>
  void flushReceiveAndDispatch(const F& functor) {
    _flushReceiveAndDispatch(ConstFunctorMessageDispatcher<F>(functor));
  }
  void _flushReceiveAndDispatch(const AbstractMessageDispatcher& dispatcher);
  //// FIXME rethink this all?!

  virtual bool dispatchCallback() = 0;

  /// Default internal message processing method
  void acceptInternalMessage(const AbstractMessage& message);
  /// Internal message processing methods
  void acceptInternalMessage(const ConnectionLostMessage& message);
  void acceptInternalMessage(const EnumerateFederationExecutionsResponseMessage& message);
  virtual void acceptInternalMessage(const InsertFederationExecutionMessage& message) = 0;
  void acceptInternalMessage(const ShutdownFederationExecutionMessage& message);
  void acceptInternalMessage(const EraseFederationExecutionMessage& message);
  void acceptInternalMessage(const DestroyFederationExecutionResponseMessage& /*message*/);
  void acceptInternalMessage(const ReleaseFederationHandleMessage& message);
  void acceptInternalMessage(const InsertModulesMessage& message);
  void acceptInternalMessage(const InsertModules2Message& message);
  virtual void acceptInternalMessage(const JoinFederationExecutionResponseMessage& message) = 0;
  void acceptInternalMessage(const JoinFederateNotifyMessage& message);
  void acceptInternalMessage(const ResignFederateNotifyMessage& message);
  void acceptInternalMessage(const ChangeAutomaticResignDirectiveMessage& message);
  void acceptInternalMessage(const RegisterFederationSynchronizationPointResponseMessage& message);
  void acceptInternalMessage(const AnnounceSynchronizationPointMessage& message);
  void acceptInternalMessage(const SynchronizationPointAchievedMessage& message);
  void acceptInternalMessage(const FederationSynchronizedMessage& message);
  void acceptInternalMessage(const EnableTimeRegulationRequestMessage& message);
  void acceptInternalMessage(const EnableTimeRegulationResponseMessage& message);
  void acceptInternalMessage(const DisableTimeRegulationRequestMessage& message);
  void acceptInternalMessage(const CommitLowerBoundTimeStampMessage& message);
  void acceptInternalMessage(const CommitLowerBoundTimeStampResponseMessage& message);
  void acceptInternalMessage(const LockedByNextMessageRequestMessage& message);
  void acceptInternalMessage(const InsertRegionMessage& message);
  void acceptInternalMessage(const CommitRegionMessage& message);
  void acceptInternalMessage(const EraseRegionMessage& message);
  void acceptInternalMessage(const ChangeInteractionClassPublicationMessage& message);
  void acceptInternalMessage(const ChangeObjectClassPublicationMessage& message);
  void acceptInternalMessage(const ChangeInteractionClassSubscriptionMessage& message);
  void acceptInternalMessage(const ChangeObjectClassSubscriptionMessage& message);
  void acceptInternalMessage(const ChangeObjectInstanceSubscriptionMessage& message);
  void acceptInternalMessage(const RegistrationForObjectClassMessage& message);
  void acceptInternalMessage(const TurnInteractionsOnMessage& message);
  void acceptInternalMessage(const InteractionMessage& message);
  void acceptInternalMessage(const TimeStampedInteractionMessage& message);
  void acceptInternalMessage(const ObjectInstanceHandlesResponseMessage& message);
  void acceptInternalMessage(const ReserveObjectInstanceNameResponseMessage& message);
  void acceptInternalMessage(const ReserveMultipleObjectInstanceNameResponseMessage& message);
  void acceptInternalMessage(const InsertObjectInstanceMessage& message);
  void acceptInternalMessage(const DeleteObjectInstanceMessage& message);
  void acceptInternalMessage(const TimeStampedDeleteObjectInstanceMessage& message);
  void acceptInternalMessage(const AttributeUpdateMessage& message);
  void acceptInternalMessage(const TimeStampedAttributeUpdateMessage& message);
  void acceptInternalMessage(const RequestAttributeUpdateMessage& message);
  void acceptInternalMessage(const RequestClassAttributeUpdateMessage& message);
  void acceptInternalMessage(const QueryAttributeOwnershipRequestMessage& message);
  void acceptInternalMessage(const EnableTimeConstrainedNotifyMessage& message);
  void acceptInternalMessage(const DisableTimeConstrainedNotifyMessage& message);


  std::pair<CreateFederationExecutionResponseType, std::string>
  dispatchWaitCreateFederationExecutionResponse(const AbsTimeout& timeout);
  DestroyFederationExecutionResponseType
  dispatchWaitDestroyFederationExecutionResponse(const AbsTimeout& timeout);

  std::pair<JoinFederationExecutionResponseType, std::string>
  dispatchWaitJoinFederationExecutionResponse(const AbsTimeout& timeout, std::string federateName);
  bool dispatchWaitEraseFederationExecutionResponse(const AbsTimeout& timeout);

  /// Tries to reserve the object instance name in the federate and returns
  /// an object handle for this reserved name if successful.
  ObjectInstanceHandle dispatchWaitReserveObjectInstanceName(const AbsTimeout& timeout, const std::string& objectInstanceName);


  /// Get access to the federate
  virtual Federate* getFederate() = 0;

  /// To factor out the management stuff and make this adaptable to the implementation
  virtual InternalTimeManagement* getTimeManagement() = 0;

  ///////////////////////////////////////////////////////////////////
  // processing of callback messages - this is what the ambassador user sees
  void queueCallback(const SharedPtr<const AbstractMessage>& message)
  {
    ScopeLock lock(_callbackMessageListMutex);
    if (_messageListPool.empty()) {
      _callbackMessageList.push_back(message);
    } else {
      _callbackMessageList.splice(_callbackMessageList.end(), _messageListPool, _messageListPool.begin());
      _callbackMessageList.back() = message;
    }
    if (_notificationHandle != nullptr)
    {
      //CondDebugPrintf("%s(TID=%d): _notificationHandle=%p\n", __FUNCTION__, GetCurrentThreadId(), _notificationHandle.get());
      _notificationHandle->Signal();
    }
  }
  void queueCallback(const AbstractMessage& message)
  {
    ScopeLock lock(_callbackMessageListMutex);
    if (_messageListPool.empty()) {
      _callbackMessageList.push_back(SharedPtr<const AbstractMessage>(&message));
    } else {
      _callbackMessageList.splice(_callbackMessageList.end(), _messageListPool, _messageListPool.begin());
      _callbackMessageList.back() = SharedPtr<const AbstractMessage>(&message);
    }
    if (_notificationHandle != nullptr)
    {
      //CondDebugPrintf("%s(TID=%d): _notificationHandle=%p\n", __FUNCTION__, GetCurrentThreadId(), _notificationHandle.get());
      _notificationHandle->Signal();
    }
  }
  void queueTimeStampedMessage(const VariableLengthData& timeStamp, const AbstractMessage& message, bool loopback=false);
  void queueReceiveOrderMessage(const AbstractMessage& message);
  bool _dispatchCallbackMessage(const AbstractMessageDispatcher& messageDispatcher);
  bool _callbackMessageAvailable();

  uint32_t getProtocolVersion() const;

  void enableCallbacks() noexcept
    // throw (SaveInProgress,
    //        RestoreInProgress,
    //        RTIinternalError)
  {
    _callbacksEnabled = true;
  }

  void disableCallbacks() noexcept
    // throw (SaveInProgress,
    //        RestoreInProgress,
    //        RTIinternalError)
  {
    _callbacksEnabled = false;
  }

  bool getCallbacksEnabled() const noexcept { return _callbacksEnabled; }

  void enableImmediateProcessing();
  void disableImmediateProcessing();
  CallbackModel getCallbackModel() const noexcept { return _callbackModel; }

  void _setNotificationHandle(std::shared_ptr<AbstractNotificationHandle> h);
  std::shared_ptr<AbstractNotificationHandle> _getNotificationHandle() const noexcept
  {
    return _notificationHandle;
  }
private:
  InternalAmbassador(const InternalAmbassador&) = delete;
  InternalAmbassador& operator=(const InternalAmbassador&) = delete;

  class _InternalMessageDispatchFunctor;
  class _CreateFederationExecutionFunctor;
  class _JoinFederationExecutionFunctor;
  class _DestroyFederationExecutionFunctor;
  class _EraseFederationExecutionFunctor;
  class _ReserveObjectInstanceNameFunctor;

  // The connect to the ServerNodes
  SharedPtr<AbstractConnect> _connect;

  // List for receive order messages already queued for callback
  MessageList _callbackMessageList;
  // List elements for reuse
  MessageList _messageListPool;
  Mutex _callbackMessageListMutex;

  std::shared_ptr<AbstractNotificationHandle> _notificationHandle;
  CallbackModel _callbackModel = HLA_EVOKED;
  SharedPtr<ImmediateCallbackDispatcher> _immediateCallbackDispatcher;
  
protected:
  // True if callback dispatch is enabled or if callbacks are held back
  bool _callbacksEnabled = true;
  // the big huge lock around the time management
  RecursiveMutex _timeManagementMutex;
};

} // namespace OpenRTI

#endif
