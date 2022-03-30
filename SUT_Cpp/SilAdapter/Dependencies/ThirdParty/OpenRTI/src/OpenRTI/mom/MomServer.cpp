

#include "DebugNew.h"
#include "MomServer.h"
#include "ServerModel.h"
#include "AbstractMessageEncoding.h"
#include "MessageEncodingRegistry.h"
#include "MomManager.h"
#include "MomFederate.h"
#include "MomFederation.h"
#include "InternalTimeManagement.h"
#include "MomFederateMetrics.h"

#pragma warning(disable : 4996)

namespace OpenRTI {

class OPENRTI_LOCAL ConvertAttributeHandleVector : public OpenRTI::AttributeHandleVector
{
  public:
    ConvertAttributeHandleVector(const AttributeHandleSet& attributeHandleSet)
    {
      reserve(attributeHandleSet.size() + 1);
      for (AttributeHandleSet::const_iterator i = attributeHandleSet.begin(); i != attributeHandleSet.end(); ++i)
        push_back(*i);
    }
};

class OPENRTI_LOCAL ConvertAttributeHandleSet : public OpenRTI::AttributeHandleSet
{
  public:
    ConvertAttributeHandleSet(const AttributeHandleVector& attributes)
    {
      insert(attributes.begin(), attributes.end());
    }
};

class OPENRTI_LOCAL ConvertAttributeValueVector : public OpenRTI::AttributeValueVector
{
  public:
    ConvertAttributeValueVector(const AttributeHandleValueMap& attributeHandleValueMap)
    {
      reserve(attributeHandleValueMap.size());
      for (AttributeHandleValueMap::const_iterator i = attributeHandleValueMap.begin();
           i != attributeHandleValueMap.end(); ++i)
      {
        push_back(OpenRTI::AttributeValue());
        back().setAttributeHandle((i->first));
        back().setValue(i->second);
      }
    }
};

class OPENRTI_LOCAL ConvertParameterValueVector : public OpenRTI::ParameterValueVector
{
  public:
    ConvertParameterValueVector(const ParameterHandleValueMap& parameterHandleValueMap)
    {
      reserve(parameterHandleValueMap.size());
      for (ParameterHandleValueMap::const_iterator i = parameterHandleValueMap.begin();
           i != parameterHandleValueMap.end(); ++i)
      {
        push_back(OpenRTI::ParameterValue());
        back().setParameterHandle(i->first);
        back().setValue(i->second);
      }
    }
};

class OPENRTI_LOCAL ConvertParameterHandleValueMap : public OpenRTI::ParameterHandleValueMap
{
  public:
    ConvertParameterHandleValueMap(const ParameterValueVector& parameters)
    {
      for (auto& pv : parameters)
      {
        insert(std::make_pair(pv.getParameterHandle(), pv.getValue()));
      }
    }
};


class OPENRTI_LOCAL MomServer::_InternalMessageDispatchFunctor {
public:
  _InternalMessageDispatchFunctor(MomServer& momServer)
    : _momServer(momServer)
  { }
  template<typename M>
  void operator()(const M& message) const { _momServer.accept(message); }
private:
  MomServer& _momServer;
};


MomServer::MomServer(ServerModel::Federation* federation, AbstractServer* server, bool isRoot, FederateHandle ownerFederateHandle)
  : _isRoot(isRoot)
  , _federation(federation)
  , _ownerFederate(ownerFederateHandle)
{
  //DebugPrintf(">>> %s(federation=%s isRoot=%d)\n", __FUNCTION__, federation->getName().c_str(), isRoot);
  //DebugPrintf("%s: this=%p federation=%s isRoot=%d ownerFederate=%s\n", __FUNCTION__, this, federation->getName().c_str(), isRoot, _ownerFederate.toString().c_str());
  connect(server);
  joinFederationExecution(federation);
  //DebugPrintf("<<< %s\n", __FUNCTION__);
}

MomServer::~MomServer()
{
  //DebugPrintf("%s: this=%p ownerFederate=%s isRoot=%d\n", __FUNCTION__, this, _ownerFederate.toString().c_str(), _isRoot);
  assert(_rtiFederate == nullptr);
  assert(!_ownerFederate.valid());
}

class OPENRTI_LOCAL MomServer::_MomMessageDispatcher : public AbstractMessageSender {
public:
  _MomMessageDispatcher(MomServer& momServer) : _momServer(momServer) {}
  virtual ~_MomMessageDispatcher() {}
  virtual void send(const SharedPtr<const AbstractMessage>& message) override
  {
    _InternalMessageDispatchFunctor functor(_momServer);
    message->dispatch(FunctorMessageDispatcher<MomServer::_InternalMessageDispatchFunctor>(functor));
  }
  virtual void close() noexcept override
  {
    //DebugPrintf("%s\n");
  }
  AbstractServer* getServer() const override { return nullptr; }
private:
  MomServer& _momServer;
};

void MomServer::connect(AbstractServer* server)
{
  //DebugPrintf(">>> %s\n", __FUNCTION__);
  StringStringListMap connectOptions;
  _dispatcher = MakeShared<_MomMessageDispatcher>(*this);
  std::list<std::string> protocolVersions{OPENRTI_ENCODING_VERSION_STRING};
  connectOptions.insert(std::make_pair("version", protocolVersions));
  _connect = server->sendDirectConnect(_dispatcher, connectOptions);
  //DebugPrintf("<<< %s\n", __FUNCTION__);
}

FederateHandle MomServer::getFederateHandle(const std::string& name)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  if (_rtiFederate->getFederateName() == name)
    return _rtiFederate->getFederateHandle();
  FederateHandle federateHandle = _rtiFederate->getFederateHandle(name);
  if (!federateHandle.valid())
    throw NameNotFound(name);
  return federateHandle;
}

void MomServer::accept(const AbstractMessage& /*message*/)
{
  //DebugPrintf("%s: dispatch message=%s\n", __FUNCTION__, message.toString().c_str());
}

void MomServer::accept(const InsertFederationExecutionMessage& message)
{
  //DebugPrintf("%s(InsertFederationExecutionMessage): this=%p ownerFederate=%s dispatch message=%s\n", __FUNCTION__, this, _ownerFederate.toString().c_str(), message.toString().c_str());
  // IMPORTANT NOTE: this object's data will be completed later in 
  // accept(const JoinFederationExecutionResponseMessage& message)
  _rtiFederate = MakeShared<Federate>();
  _rtiFederate->setFederationHandle(message.getFederationHandle());
  _rtiFederate->setLogicalTimeFactoryName(message.getLogicalTimeFactoryName());
}

void MomServer::accept(const EraseFederationExecutionMessage& message)
{
  //DebugPrintf("%s(EraseFederationExecutionMessage): this=%p ownerFederate=%s dispatch message=%s\n", __FUNCTION__, this, _ownerFederate.toString().c_str(), message.toString().c_str());
  _federation = nullptr;
  SharedPtr<ReleaseFederationHandleMessage> release = MakeShared<ReleaseFederationHandleMessage>();
  release->setFederationHandle(message.getFederationHandle());
  sendRequest(release);
}

void MomServer::accept(const JoinFederationExecutionResponseMessage& message)
{
  //DebugPrintf("%s(JoinFederationExecutionResponseMessage): this=%p ownerFederate=%s dispatch message=%s\n", __FUNCTION__, this, _ownerFederate.toString().c_str(), message.toString().c_str());
  _joinResponse = message.getJoinFederationExecutionResponseType();
  switch (_joinResponse)
  {
    case JoinFederationExecutionResponseSuccess:
    {
      Federate* federate = getRtiFederate();
      if (!federate)
        return;
      federate->setFederateHandle(message.getFederateHandle());
      federate->setFederateName(message.getFederateName());
      federate->setFederateType(message.getFederateType());

      notifyFederationJoined(message.getFederateHandle(), message.getFederateName(), message.getFederateType());
      break;
    }
    case JoinFederationExecutionResponseFederateNameAlreadyInUse:
    {
      // The join went wrong, probably because of a race condition related to the usage of the owner federate handle?
      //DebugPrintf("%s(JoinFederationExecutionResponseMessage): this=%p ownerFederate=%s join FAILED: FederateNameAlreadyInUse: %s\n\n", __FUNCTION__, this, _ownerFederate.toString().c_str(), message.getFederateName().c_str());
      // since we didn't join, there will be no resign message, and the _ownerFederate owner federate won't be cleared before the destructor is called.
      _ownerFederate = FederateHandle();
    }
    default:
      // The join went wrong
      //DebugPrintf("%s(JoinFederationExecutionResponseMessage): this=%p ownerFederate=%s join FAILED: dispatch message=%s\n\n", __FUNCTION__, this, _ownerFederate.toString().c_str(), message.toString().c_str());
      _ownerFederate = FederateHandle();
  }
}

void MomServer::accept(const JoinFederateNotifyMessage& message)
{
  //DebugPrintf("%s(JoinFederateNotifyMessage): this=%p ownerFederate=%s dispatch message=%s\n", __FUNCTION__, this, _ownerFederate.toString().c_str(), message.toString().c_str());
  _rtiFederate->insertFederate(message.getFederateHandle(), message.getFederateName());
  if (!_isRoot)
  {
    if (_momManager != nullptr)
    {
      assert(message.getFederateHandle() != _ownerFederate);
    }
  }
}

void MomServer::accept(const ResignFederateNotifyMessage& message)
{
  //DebugPrintf("%s(ResignFederateNotifyMessage): this=%p ownerFederate=%s message.federateHandle=%s\n", __FUNCTION__, this, _ownerFederate.toString().c_str(), message.getFederateHandle().toString().c_str());
  notifyFederateResigned(message.getFederateHandle());
  if (_ownerFederate.valid() && message.getFederateHandle() == _ownerFederate)
  {
    //DebugPrintf("%s: owner federate resigned\n", __FUNCTION__);
    _ownerFederate = FederateHandle();
  }
  if (_rtiFederate == nullptr)
  {
    return;
  }
  if (message.getFederateHandle() == _rtiFederate->getFederateHandle())
  {
    // It's us again - now we're off.
    //DebugPrintf("%s: resigned from federation\n", __FUNCTION__);
    _rtiFederate.reset();
    _federation = nullptr;
    //_connect = nullptr;
  }
  else
  {
    _rtiFederate->eraseFederate(message.getFederateHandle());
  }
}

void MomServer::accept(const EnableTimeConstrainedNotifyMessage& message)
{
  //DebugPrintf("%s: dispatch message=%s\n", __FUNCTION__, message.toString().c_str());
  if (_momManager != nullptr && message.getFederateHandle() == _ownerFederate)
  {
    if (_momManager->getFederate() != nullptr)
    {
      _momManager->getFederate()->SetTimeConstrained(true);
    }
    else
    {
      //DebugPrintf("%s(EnableTimeConstrainedNotifyMessage): owner federate not yet present\n", __FUNCTION__);
    }
  }
}

void MomServer::accept(const EnableTimeRegulationRequestMessage& message)
{
  assert(getRtiFederate()->getFederateHandle() != message.getFederateHandle());
  // If we are in the state of a fresh joined federate which is still collecting initial information
  // we should skip sending the response.
  if (getRtiFederate()->getFederateHandle().valid())
  {

    SharedPtr<EnableTimeRegulationResponseMessage> response;
    response = MakeShared<EnableTimeRegulationResponseMessage>();
    response->setFederationHandle(getRtiFederate()->getFederationHandle());
    response->setFederateHandle(message.getFederateHandle());
    response->setRespondingFederateHandle(getRtiFederate()->getFederateHandle());
    response->setTimeStamp(message.getTimeStamp());
    response->setTimeStampValid(false);
    sendRequest(response);
  }
}

// we should probably completely ignore these - we never receive updates for our own objects
void MomServer::accept(const AttributeUpdateMessage& message)
{
  if (!_rtiFederate.valid())
    return;
  // Look for the known object class of this object instance.
  // Is required for the right subset of attributes that are reflected
  ObjectInstanceHandle objectInstanceHandle = message.getObjectInstanceHandle();
  const Federate::ObjectInstance* objectInstance = _rtiFederate->getObjectInstance(objectInstanceHandle);
  if (!objectInstance)
    return;
  Federate::ObjectClass* objectClass = _rtiFederate->getObjectClass(objectInstance->getObjectClassHandle());
  if (!objectClass)
    return;
  //reflectAttributeValues(*objectClass, message.getObjectInstanceHandle(), message.getAttributeValues(), message.getTag(),
  //                       OpenRTI::RECEIVE, message.getTransportationType(), message.getFederateHandle());
}

void MomServer::accept(const RequestAttributeUpdateMessage& message)
{
  _momManager->provideAttributeValueUpdate(message.getObjectInstanceHandle(), ConvertAttributeHandleSet(message.getAttributeHandles()));
}

void MomServer::accept(const InteractionMessage& message)
{
  _momManager->receiveInteraction(message.getInteractionClassHandle(), ConvertParameterHandleValueMap(message.getParameterValues()));
}

void MomServer::accept(const ReserveObjectInstanceNameResponseMessage& message)
{
  if (!_rtiFederate.valid())
    return;
  auto& objectInstanceHandleNamePair = message.getObjectInstanceHandleNamePair();
  ////DebugPrintf("%s: success=%d %s => %s\n", __FUNCTION__, 
  //            message.getSuccess(), objectInstanceHandleNamePair.second.c_str(), objectInstanceHandleNamePair.first.toString().c_str());
  if (message.getSuccess())
  {
    _rtiFederate->insertReservedObjectInstanceHandleNamePair(message.getObjectInstanceHandleNamePair());
    auto iter = mPendingNameReservations.find(objectInstanceHandleNamePair.second);
    if (iter != mPendingNameReservations.end())
    {
      iter->second();
    }
    else
    {
      //DebugPrintf("no pending object instance name reservation for %s\n", objectInstanceHandleNamePair.second.c_str());
    }
  }
  else
  {
    //DebugPrintf("%s: ReserveObjectInstanceName FAILED: %s\n", __FUNCTION__, objectInstanceHandleNamePair.second.c_str());
  }
}

void MomServer::accept(const InsertObjectInstanceMessage& message)
{
  if (!_rtiFederate.valid())
    return;
  ObjectClassHandle objectClassHandle = message.getObjectClassHandle();
  Federate::ObjectClass* objectClass = _rtiFederate->getObjectClass(objectClassHandle);
  if (!objectClass)
    return;
  while (Unsubscribed == objectClass->getEffectiveSubscriptionType())
  {
    objectClassHandle = objectClass->getParentObjectClassHandle();
    objectClass = _rtiFederate->getObjectClass(objectClassHandle);
    if (!objectClass)
      return;
  }
  // Ok we get duplicate inserts.
  if (_rtiFederate->getObjectInstance(message.getObjectInstanceHandle()))
    return;
  _rtiFederate->insertObjectInstance(message.getObjectInstanceHandle(), message.getName(), objectClassHandle, false);
  _momManager->discoverObjectInstance(message.getObjectInstanceHandle(), objectClassHandle, message.getName());
}

void MomServer::accept(const DeleteObjectInstanceMessage& message)
{
  if (!_rtiFederate.valid())
    return;
  _momManager->removeObjectInstance(message.getObjectInstanceHandle());
  _releaseObjectInstance(message.getObjectInstanceHandle());
}

void MomServer::accept(const InsertModulesMessage& message)
{
  Federate* federate = getRtiFederate();
  if (!federate)
    return;
  federate->insertFOMModuleList(message.getFOMModuleList());
  // NOTE: we won't call _momManager->setFOMModuleList here, because this message is usually
  // dispatched before having joined the federation (and thus before the _momManager actually exists)
}

void MomServer::accept(const ReserveObjectInstanceNameRequestMessage& /*message*/)
{
  //DebugPrintf("%s: dispatch message=%s\n", __FUNCTION__, message.toString().c_str());
}


void MomServer::joinFederationExecution(ServerModel::Federation* federation)
{
  // The join request message
  SharedPtr<JoinFederationExecutionRequestMessage> request;
  request = MakeShared<JoinFederationExecutionRequestMessage>();
  request->setFederationExecution(federation->getName());
  request->setFederateType("RTI");
  std::string federateName = _ownerFederate.valid() 
    ? "RTI-" + std::to_string(_ownerFederate.getHandle())
    : "RTI-" + federation->getName();
  request->setFederateName(federateName);
  request->setIsInternal(true);
  // Send this message and wait for the response
  sendRequest(request);
  // Actually we can't wait for a message here, because we are in the midst of 
  // handling a create or join message in the ServerNode, in the same thread.
  // Control wouldn't return to the message reception/handling loop.
  /*
  //std::pair<JoinFederationExecutionResponseType, std::string> response;
  //Clock abstime = Clock::max();
  //response = dispatchWaitJoinFederationExecutionResponse(abstime);
  switch (_joinResponse)
  {
    case JoinFederationExecutionResponseFederateNameAlreadyInUse:
      //_federate = 0;
      throw FederateNameAlreadyInUse("RTI");
      break;
    case JoinFederationExecutionResponseFederationExecutionDoesNotExist:
      //_federate = 0;
      throw FederationExecutionDoesNotExist(federation->getName());
      break;
    case JoinFederationExecutionResponseSaveInProgress:
      //_federate = 0;
      throw SaveInProgress();
      break;
    case JoinFederationExecutionResponseRestoreInProgress:
      //_federate = 0;
      throw RestoreInProgress();
      break;
    case JoinFederationExecutionResponseInconsistentFDD:
      //_federate = 0;
      //throw InconsistentFDD(response.second);
      break;
    default:
      break;
  }
  */
}


void MomServer::resignFederationExecution()
{
  if (!_rtiFederate.valid())
    return;

  // different handling required in the root server - ResignFederationExecutionLeafRequestMessage must only
  // be sent from leaf nodes connected to a parent node
  if (_isRoot)
  {
    SharedPtr<ResignFederationExecutionRequestMessage> resign = MakeShared<ResignFederationExecutionRequestMessage>();
    resign->setFederationHandle(getFederationHandle());
    resign->setFederateHandle(getFederateHandle());
    sendRequest(resign);
  }
  else
  {
    SharedPtr<ResignFederationExecutionLeafRequestMessage> resign = MakeShared<ResignFederationExecutionLeafRequestMessage>();
    resign->setFederationHandle(getFederationHandle());
    resign->setFederateHandle(getFederateHandle());
    resign->setResignAction(DELETE_OBJECTS);
    sendRequest(resign);
  }

  //_federate = nullptr;
}

void MomServer::publishObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributes)
{
  // At first the complete error checks
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  Federate::ObjectClass* objectClass = _rtiFederate->getObjectClass(objectClassHandle);
  if (!objectClass)
    throw ObjectClassNotDefined(objectClassHandle.toString());
  ConvertAttributeHandleVector attributeHandleVector(attributes);
  for (AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
    if (!objectClass->getAttribute(*i))
      throw AttributeNotDefined(i->toString());

  bool objectClassFreshPublished = false;
  // Mark the objectclass itself as published.
  // Append this to the request if this publication has changed
  AttributeHandleVector::iterator j = attributeHandleVector.begin();
  if (objectClass->setPublicationType(Published))
  {
    objectClassFreshPublished = true;
    if (attributeHandleVector.empty() || attributeHandleVector.front() != AttributeHandle(0))
      j = attributeHandleVector.insert(attributeHandleVector.begin(), AttributeHandle(0));
    ++j;
  }
  for (AttributeHandleVector::const_iterator i = j; i != attributeHandleVector.end(); ++i)
  {
    // returns true if there is a change in the publication state
    if (!objectClass->setAttributePublicationType(*i, Published))
      continue;
    if (i != j)
      *j = *i;
    ++j;
  }
  if (j != attributeHandleVector.end())
    attributeHandleVector.erase(j, attributeHandleVector.end());
  // If there has nothing changed, don't send anything.
  if (attributeHandleVector.empty())
    return;

  SharedPtr<ChangeObjectClassPublicationMessage> request = MakeShared<ChangeObjectClassPublicationMessage>();
  request->setFederationHandle(getFederationHandle());
  request->setObjectClassHandle(objectClassHandle);
  request->setPublicationType(Published);
  request->getAttributeHandles().swap(attributeHandleVector);
  sendRequest(request);

  // Simple implementation only listening to the ambassadors own publication
  // see comment in enableObjectClassRelevanceAdvisorySwitch()
  if (objectClassFreshPublished && _rtiFederate->getObjectClassRelevanceAdvisorySwitchEnabled())
  {
    SharedPtr<RegistrationForObjectClassMessage> message = MakeShared<RegistrationForObjectClassMessage>();
    message->setObjectClassHandle(objectClassHandle);
    message->setStart(true);

    //queueCallback(message);
  }
}

void MomServer::unpublishObjectClass(ObjectClassHandle objectClassHandle) /* throw (ObjectClassNotDefined, */ /* OwnershipAcquisitionPending, */ /* FederateNotExecutionMember, */ /* SaveInProgress, */ /* RestoreInProgress, */ /* NotConnected, */ // RTIinternalError)
{
  // At first the complete error checks
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  Federate::ObjectClass* objectClass = _rtiFederate->getObjectClass(objectClassHandle);
  if (!objectClass)
    throw ObjectClassNotDefined(objectClassHandle.toString());

  // now that we know not to throw, handle the request
  AttributeHandleVector attributeHandleVector;
  attributeHandleVector.reserve(objectClass->getNumAttributes());
  // Mark the objectclass itself as unpublished.
  // Append this to the request if this publication has changed
  if (objectClass->setPublicationType(Unpublished))
    attributeHandleVector.push_back(AttributeHandle(0));
  for (uint32_t i = 0; i < objectClass->getNumAttributes(); ++i)
  {
    // returns true if there is a change in the publication state
    if (!objectClass->setAttributePublicationType(AttributeHandle(i), Unpublished))
      continue;
    attributeHandleVector.push_back(AttributeHandle(i));
  }
  // If there has nothing changed, don't send anything.
  if (attributeHandleVector.empty())
    return;

  SharedPtr<ChangeObjectClassPublicationMessage> request = MakeShared<ChangeObjectClassPublicationMessage>();
  request->setFederationHandle(getFederationHandle());
  request->setObjectClassHandle(objectClassHandle);
  request->setPublicationType(Unpublished);
  request->getAttributeHandles().swap(attributeHandleVector);
  sendRequest(request);

  // No advisory callbacks in unpublished state
}

void MomServer::subscribeObjectClassAttributes(ObjectClassHandle objectClassHandle,
    AttributeHandleSet const& attributes,
    bool active)
{
  ConvertAttributeHandleVector attributeHandleVector(attributes);
  // At first the complete error checks
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  Federate::ObjectClass* objectClass = _rtiFederate->getObjectClass(objectClassHandle);
  if (!objectClass)
    throw ObjectClassNotDefined(objectClassHandle.toString());
  for (AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
    if (!objectClass->getAttribute(*i))
      throw AttributeNotDefined(i->toString());

  // now that we know not to throw, handle the request
  SubscriptionType subscriptionType;
  if (active)
  {
    subscriptionType = SubscribedActive;
  }
  else
  {
    subscriptionType = SubscribedPassive;
  }

  // now filter out all class attributes from attributeHandleVector,
  // where subscription didn't change by this request.

  AttributeHandleVector::iterator j = attributeHandleVector.begin();
  if (objectClass->setSubscriptionType(subscriptionType))
  {
    // Class subscription changed: insert AttributeHandle(0) (HLAprivilegeToDeleteObject) to front, if not existing
    if (attributeHandleVector.empty() || attributeHandleVector.front() != AttributeHandle(0))
      j = attributeHandleVector.insert(attributeHandleVector.begin(), AttributeHandle(0));
    ++j;
  }
  // j should now point to next attribute after HLAprivilegeToDeleteObject.
  // NOTE: if class has been subscribed already, it will actually point to begin of list. bug?
  // Iterate through remaining class attributes, updating their subscriptions
  for (AttributeHandleVector::const_iterator i = j; i != attributeHandleVector.end(); ++i)
  {
    // returns true if there is a change in the subscription state
    if (!objectClass->setAttributeSubscriptionType(*i, subscriptionType))
      continue;
    if (i != j)
      *j = *i;
    ++j;
  }
  // j now points to last attribute in attributeHandleVector, where subscription changed.
  // Remove remaining attributes (with unchanged subscriptions) from list.
  if (j != attributeHandleVector.end())
    attributeHandleVector.erase(j, attributeHandleVector.end());
  // If there has nothing changed, don't send anything.
  if (attributeHandleVector.empty())
    return;

  SharedPtr<ChangeObjectClassSubscriptionMessage> request = MakeShared<ChangeObjectClassSubscriptionMessage>();
  request->setFederationHandle(getFederationHandle());
  request->setObjectClassHandle(objectClassHandle);
  request->getAttributeHandles().swap(attributeHandleVector);
  request->setSubscriptionType(subscriptionType);
  sendRequest(request);
}

ObjectInstanceHandle MomServer::registerObjectInstance(ObjectClassHandle objectClassHandle, const std::string& objectInstanceName)
{
  // At first the complete error checks
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  Federate::ObjectClass* objectClass = _rtiFederate->getObjectClass(objectClassHandle);
  if (!objectClass)
    throw ObjectClassNotDefined(objectClassHandle.toString());
  if (Published != objectClass->getEffectivePublicationType())
    throw ObjectClassNotPublished(objectClass->getName());
  // Do not allow empty object instance names
  if (objectInstanceName.empty())
    throw ObjectInstanceNameNotReserved(objectInstanceName);

  // The already available objectInstanceHandle should be stored here.
  ObjectInstanceHandle objectInstanceHandle;
  objectInstanceHandle = _rtiFederate->takeReservedObjectInstanceName(objectInstanceName);

  // Check if we already have the object instance name reserved
  if (!objectInstanceHandle.valid())
  {
    throw ObjectInstanceNameNotReserved(objectInstanceName);
  }

  // Once we have survived, we know that the objectInstanceName given in the argument is unique and ours.
  // Also the object instance handle in this local scope must be valid and ours.

  _rtiFederate->insertObjectInstance(objectInstanceHandle, objectInstanceName, objectClassHandle, true);

  SharedPtr<InsertObjectInstanceMessage> request;
  request = MakeShared<InsertObjectInstanceMessage>();
  request->setFederationHandle(getFederationHandle());
  request->setObjectClassHandle(objectClassHandle);
  request->setObjectInstanceHandle(objectInstanceHandle);
  request->setName(objectInstanceName);
  uint32_t numAttributes = objectClass->getNumAttributes();
  request->getAttributeStateVector().reserve(numAttributes);
  AttributeHandleVector attributeHandleVector;
  attributeHandleVector.reserve(numAttributes);
  for (uint32_t i = 0; i < numAttributes; ++i)
  {
    if (!objectClass->isAttributePublished(AttributeHandle(i)))
      continue;
    AttributeState attributeState;
    attributeState.setAttributeHandle(AttributeHandle(i));
    attributeState.setOwnerFederate(_rtiFederate->getFederateHandle());
    request->getAttributeStateVector().push_back(attributeState);
    attributeHandleVector.push_back(AttributeHandle(i));
  }
  sendRequest(request);
  return objectInstanceHandle;
}


void MomServer::deleteObjectInstance(ObjectInstanceHandle objectInstanceHandle)
{
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  Federate::ObjectInstance* objectInstance = _rtiFederate->getObjectInstance(objectInstanceHandle);
  if (!objectInstance)
    throw ObjectInstanceNotKnown(objectInstanceHandle.toString());
  if (!objectInstance->isOwnedByFederate())
    throw DeletePrivilegeNotHeld(objectInstanceHandle.toString());

  SharedPtr<DeleteObjectInstanceMessage> request;
  request = MakeShared<DeleteObjectInstanceMessage>();
  request->setFederationHandle(getFederationHandle());
  request->setFederateHandle(getFederateHandle());
  request->setObjectInstanceHandle(objectInstanceHandle);

  sendRequest(request);

  _releaseObjectInstance(objectInstanceHandle);
}

void MomServer::updateAttributeValues(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleValueMap& attributes)
{
  ConvertAttributeValueVector attributeValues(attributes);
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  Federate::ObjectInstance* objectInstance = _rtiFederate->getObjectInstance(objectInstanceHandle);
  if (!objectInstance)
    throw ObjectInstanceNotKnown(objectInstanceHandle.toString());
  // passels
  AttributeValueVector passels[2];
  for (std::vector<OpenRTI::AttributeValue>::iterator i = attributeValues.begin(); i != attributeValues.end(); ++i)
  {
    const Federate::InstanceAttribute* instanceAttribute = objectInstance->getInstanceAttribute(i->getAttributeHandle());
    if (!instanceAttribute)
      throw AttributeNotDefined(i->getAttributeHandle().toString());
    if (!instanceAttribute->getIsOwnedByFederate())
      throw AttributeNotOwned(i->getAttributeHandle().toString());
#ifdef DEBUG_PRINTF
    if (objectClass->getEffectiveAttributeSubscriptionType(i->getAttributeHandle()) == Unsubscribed)
    {
      Federate::Attribute* classAttribute = objectClass->getAttribute(i->getAttributeHandle());
      DebugPrintf("%s: InstanceAttribute %s::%s of %s is unsubscribed\n", __FUNCTION__, objectClass->getFQName().c_str(), classAttribute->getName().c_str(), objectInstance->getName().c_str());
    }
#endif
    unsigned index = instanceAttribute->getTransportationType();
    passels[index].reserve(attributeValues.size());
    passels[index].push_back(AttributeValue());
    passels[index].back().setAttributeHandle(i->getAttributeHandle());
    passels[index].back().getValue().swap(i->getValue());
  }

  for (unsigned i = 0; i < 2; ++i)
  {
    if (passels[i].empty())
      continue;
    SharedPtr<AttributeUpdateMessage> request;
    request = MakeShared<AttributeUpdateMessage>();
    request->setFederationHandle(getFederationHandle());
    request->setFederateHandle(getFederateHandle());
    request->setObjectInstanceHandle(objectInstanceHandle);
    request->getAttributeValues().swap(passels[i]);
    request->setTransportationType(TransportationType(i));
    sendRequest(request);
  }
}

void MomServer::requestAttributeValueUpdate(ObjectInstanceHandle objectInstanceHandle, AttributeHandleSet const& attributes)
{
  ConvertAttributeHandleVector attributeHandleVector(attributes);
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  Federate::ObjectInstance* objectInstance = _rtiFederate->getObjectInstance(objectInstanceHandle);
  if (!objectInstance)
    throw ObjectInstanceNotKnown(objectInstanceHandle.toString());
  for (AttributeHandleVector::const_iterator j = attributeHandleVector.begin(); j != attributeHandleVector.end(); ++j)
  {
    if (!objectInstance->getInstanceAttribute(j->getHandle()))
      throw AttributeNotDefined(j->toString());
  }

  SharedPtr<RequestAttributeUpdateMessage> request;
  request = MakeShared<RequestAttributeUpdateMessage>();
  request->setFederationHandle(getFederationHandle());
  request->setObjectInstanceHandle(objectInstanceHandle);
  request->getAttributeHandles().swap(attributeHandleVector);
  sendRequest(request);
}

void MomServer::publishInteractionClass(InteractionClassHandle interactionClassHandle)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  Federate::InteractionClass* interactionClass = _rtiFederate->getInteractionClass(interactionClassHandle);
  if (!interactionClass)
    throw InteractionClassNotDefined(interactionClassHandle.toString());
  if (!interactionClass->setPublicationType(Published))
    return;

  SharedPtr<ChangeInteractionClassPublicationMessage> request = MakeShared<ChangeInteractionClassPublicationMessage>();
  request->setFederationHandle(getFederationHandle());
  request->setInteractionClassHandle(interactionClassHandle);
  request->setPublicationType(Published);

  sendRequest(request);

  // Simple implementation only listening to the ambassadors own publication
  // see comment in enableInteractionRelevanceAdvisorySwitch()
  if (_rtiFederate->getInteractionRelevanceAdvisorySwitchEnabled())
  {
    SharedPtr<TurnInteractionsOnMessage> message = MakeShared<TurnInteractionsOnMessage>();
    message->setInteractionClassHandle(interactionClassHandle);
    message->setOn(true);

    //queueCallback(message);
  }
}

void MomServer::subscribeInteractionClassWithFilter(
  InteractionClassHandle interactionClassHandle,
  ParameterHandleValueMap filterValues,
  bool active)
{
  ConvertParameterValueVector filterParameterValueVector(filterValues);
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  Federate::InteractionClass* interactionClass = _rtiFederate->getInteractionClass(interactionClassHandle);
  if (!interactionClass)
    throw InteractionClassNotDefined(interactionClassHandle.toString());

  SubscriptionType subscriptionType;
  if (active)
  {
    subscriptionType = SubscribedActive;
  }
  else
  {
    subscriptionType = SubscribedPassive;
  }

  bool subscribeResult = interactionClass->setSubscriptionType(subscriptionType, filterParameterValueVector);
  if (subscribeResult)
  {
    // maybe we want to store parameter filters in the federate's InteractionClass as well.
    SharedPtr<ChangeInteractionClassSubscriptionMessage> request = MakeShared<ChangeInteractionClassSubscriptionMessage>();
    request->setFederationHandle(getFederationHandle());
    request->setInteractionClassHandle(interactionClassHandle);
    request->setSubscriptionType(subscriptionType);
    request->getParameterFilterValues().swap(filterParameterValueVector);

    sendRequest(request);
  }
}

void MomServer::subscribeInteractionClass(InteractionClassHandle interactionClassHandle, bool active)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  Federate::InteractionClass* interactionClass = _rtiFederate->getInteractionClass(interactionClassHandle);
  if (!interactionClass)
    throw InteractionClassNotDefined(interactionClassHandle.toString());

  SubscriptionType subscriptionType;
  if (active)
  {
    subscriptionType = SubscribedActive;
  }
  else
  {
    subscriptionType = SubscribedPassive;
  }
  if (!interactionClass->setSubscriptionType(subscriptionType, ParameterValueVector()))
    return;

  SharedPtr<ChangeInteractionClassSubscriptionMessage> request = MakeShared<ChangeInteractionClassSubscriptionMessage>();
  request->setFederationHandle(getFederationHandle());
  request->setInteractionClassHandle(interactionClassHandle);
  request->setSubscriptionType(subscriptionType);
  sendRequest(request);
}

void MomServer::unsubscribeInteractionClass(InteractionClassHandle interactionClassHandle)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  Federate::InteractionClass* interactionClass = _rtiFederate->getInteractionClass(interactionClassHandle);
  if (!interactionClass)
    throw InteractionClassNotDefined(interactionClassHandle.toString());

  if (!interactionClass->setSubscriptionType(Unsubscribed, ParameterValueVector()))
    return;

  SharedPtr<ChangeInteractionClassSubscriptionMessage> request = MakeShared<ChangeInteractionClassSubscriptionMessage>();
  request->setFederationHandle(getFederationHandle());
  request->setInteractionClassHandle(interactionClassHandle);
  request->setSubscriptionType(Unsubscribed);

  sendRequest(request);
}

void MomServer::sendInteraction(InteractionClassHandle interactionClassHandle, const ParameterHandleValueMap& parameterValues)
{
  ConvertParameterValueVector parameterValueVector(parameterValues);
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  const Federate::InteractionClass* interactionClass = _rtiFederate->getInteractionClass(interactionClassHandle);
  if (!interactionClass)
    throw InteractionClassNotDefined(interactionClassHandle.toString());
  if (!interactionClass->isPublished())
    throw InteractionClassNotPublished(interactionClassHandle.toString());
  for (std::vector<ParameterValue>::const_iterator i = parameterValueVector.begin(); i != parameterValueVector.end(); ++i)
    if (!interactionClass->getParameter(i->getParameterHandle()))
      throw InteractionParameterNotDefined(i->getParameterHandle().toString());

  SharedPtr<InteractionMessage> request;
  request = MakeShared<InteractionMessage>();
  request->setFederationHandle(getFederationHandle());
  request->setFederateHandle(getFederateHandle());
  request->setInteractionClassHandle(interactionClassHandle);
  request->setTransportationType(interactionClass->getTransportationType());
  request->getParameterValues().swap(parameterValueVector);
  sendRequest(request);
}

std::shared_ptr<AbstractFederateMetrics> MomServer::getFederateMetrics()
{
  if (_momManager != nullptr)
  {
    return _momManager->getFederateMetrics();
  }
  return std::shared_ptr<AbstractFederateMetrics>();
}


void MomServer::sendRequest(const SharedPtr<const AbstractMessage>& message)
{
  if (_connect != nullptr)
  {
    _connect->send(message);
  }
  else
  {
    //DebugPrintf("%s: connect already closed ...\n", __FUNCTION__);
  }
}


bool MomServer::getIsTimeConstrained(FederateHandle federateHandle) const
{
  OpenRTIAssert(_federation != nullptr && _federation->getFederate(federateHandle) != nullptr);
  return _federation->getFederate(federateHandle)->getIsTimeConstrained();
}


bool MomServer::getIsTimeRegulating(FederateHandle federateHandle) const
{
  OpenRTIAssert(_federation != nullptr && _federation->getFederate(federateHandle) != nullptr);
  return _federation->getFederate(federateHandle)->getIsTimeRegulating();
}


void MomServer::reserveObjectInstanceName(const std::string& objectInstanceName, std::function<void()> callback)
{
  Federate* federate = getRtiFederate();
  OpenRTIAssert(federate);
  mPendingNameReservations[objectInstanceName] = callback;
  SharedPtr<ReserveObjectInstanceNameRequestMessage> request;
  request = MakeShared<ReserveObjectInstanceNameRequestMessage>();
  request->setFederationHandle(federate->getFederationHandle());
  request->setFederateHandle(federate->getFederateHandle());
  request->setName(objectInstanceName);
  request->setIsInternal(true);
  sendRequest(request);
}

void MomServer::_releaseObjectInstance(const ObjectInstanceHandle& objectInstanceHandle)
{
  // Remove the instance from the object model
  _rtiFederate->eraseObjectInstance(objectInstanceHandle);

  // Unreference the object instance handle resource
  SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message;
  message = MakeShared<ReleaseMultipleObjectInstanceNameHandlePairsMessage>();
  message->setFederationHandle(getFederationHandle());
  message->getObjectInstanceHandleVector().push_back(objectInstanceHandle);
  sendRequest(message);
}

ObjectInstanceHandle MomServer::getObjectInstanceHandle(const std::string& objectInstanceName)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  ObjectInstanceHandle objectInstanceHandle = _rtiFederate->getObjectInstanceHandle(objectInstanceName);
  if (!objectInstanceHandle.valid())
    throw ObjectInstanceNotKnown(objectInstanceName);
  return objectInstanceHandle;
}

const std::string& MomServer::getObjectInstanceName(ObjectInstanceHandle objectInstanceHandle)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  const Federate::ObjectInstance* objectInstance = _rtiFederate->getObjectInstance(objectInstanceHandle);
  if (!objectInstance)
    throw ObjectInstanceNotKnown(objectInstanceHandle.toString());
  return objectInstance->getName();
}

ObjectClassHandle MomServer::getKnownObjectClassHandle(ObjectInstanceHandle objectInstanceHandle)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  const Federate::ObjectInstance* objectInstance = _rtiFederate->getObjectInstance(objectInstanceHandle);
  if (!objectInstance)
    throw ObjectInstanceNotKnown(objectInstanceHandle.toString());
  return objectInstance->getObjectClassHandle();
}

const std::string& MomServer::getInteractionClassName(InteractionClassHandle interactionClassHandle)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  const Federate::InteractionClass* interactionClass = _rtiFederate->getInteractionClass(interactionClassHandle);
  if (!interactionClass)
    throw InvalidInteractionClassHandle(interactionClassHandle.toString());
  return interactionClass->getName();
}

const std::string& MomServer::getFederateName(FederateHandle federateHandle)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  if (_rtiFederate->getFederateHandle() == federateHandle)
    return _rtiFederate->getFederateName();
  const Federate::_Federate* federate = _rtiFederate->getFederate(federateHandle);
  if (!federate)
    throw InvalidFederateHandle(federateHandle.toString());
  return federate->getName();
}


const std::string& MomServer::getFederateType(FederateHandle federateHandle)
{
  static std::string empty;
  if (_rtiFederate->getFederateHandle() == federateHandle)
    return _rtiFederate->getFederateType();
  const Federate::_Federate* federate = _rtiFederate->getFederate(federateHandle);
  if (!federate)
    throw InvalidFederateHandle(federateHandle.toString());
  return empty;
  //return federate->getType();
}

const std::string& MomServer::getObjectClassName(ObjectClassHandle objectClassHandle)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  const Federate::ObjectClass* objectClass = _rtiFederate->getObjectClass(objectClassHandle);
  if (!objectClass)
    throw InvalidObjectClassHandle(objectClassHandle.toString());
  return objectClass->getName();
}


ObjectClassHandle MomServer::getObjectClassHandle(const std::string& name)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  ObjectClassHandle objectClassHandle = _rtiFederate->getObjectClassHandle(name);
  if (!objectClassHandle.valid())
    throw NameNotFound(name);
  return objectClassHandle;
}

AttributeHandle MomServer::getAttributeHandle(ObjectClassHandle objectClassHandle, const std::string& name)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  const Federate::ObjectClass* objectClass = _rtiFederate->getObjectClass(objectClassHandle);
  if (!objectClass)
    throw InvalidObjectClassHandle(objectClassHandle.toString());
  AttributeHandle attributeHandle = objectClass->getAttributeHandle(name);
  if (!attributeHandle.valid())
    throw NameNotFound(name);
  return attributeHandle;
}

InteractionClassHandle MomServer::getInteractionClassHandle(const std::string& name)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  InteractionClassHandle interactionClassHandle;
  interactionClassHandle = _rtiFederate->getInteractionClassHandle(name);
  if (!interactionClassHandle.valid())
    throw NameNotFound(name);
  return interactionClassHandle;
}

ParameterHandle MomServer::getParameterHandle(InteractionClassHandle interactionClassHandle, const std::string& name)
{
  //if (!isConnected())
  //  throw NotConnected();
  if (!_rtiFederate.valid())
    throw FederateNotExecutionMember();
  const Federate::InteractionClass* interactionClass = _rtiFederate->getInteractionClass(interactionClassHandle);
  if (!interactionClass)
    throw InvalidInteractionClassHandle(interactionClassHandle.toString());
  ParameterHandle parameterHandle;
  parameterHandle = interactionClass->getParameterHandle(name);
  if (!parameterHandle.valid())
    throw NameNotFound(name);
  return parameterHandle;
}

// called when the RTI federate successfully joined the federation
void MomServer::notifyFederationJoined(FederateHandle federateHandle, const std::string& federateName, const std::string& federateType)
{
  _momManager = std::make_shared<MomManager>(this, _ownerFederate);
  _momManager->initialize();
  _momManager->publishSubscribeObjects();
  if (_isRoot)
  {
    _momManager->notifyFederationCreated(_federation->getName());
  }
  _momManager->notifyFederationJoined(federateHandle, federateName, federateType);
  //FederateHandleSet existingFederates;
  //_federate->getFederateHandleSet(existingFederates);
  if (!_isRoot)
  {
    ServerModel::Federate* ownerFederate = _federation->getFederate(_ownerFederate);
    _momManager->notifyOwnerFederateJoined(ownerFederate->getFederateHandle(), ownerFederate->getName(), ownerFederate->getFederateType());
  }
  if (_isRoot)
  {
    FOMModuleList moduleList;
    _federation->getModuleList(moduleList);
    std::ostringstream currentFDD;
    _federation->writeCurrentFDD(currentFDD);
    _momManager->setFOMModuleList(moduleList, currentFDD.str());
  }
}

void MomServer::notifyFederateResigned(FederateHandle federateHandle)
{
  if (_momManager != nullptr)
  {
    _momManager->notifyFederateResigned(federateHandle);
  }
}

}