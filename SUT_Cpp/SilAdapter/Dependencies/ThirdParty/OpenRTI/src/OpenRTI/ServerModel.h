/* -*-c++-*- OpenRTI - Copyright (C) 2009-2014 Mathias Froehlich
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

#ifndef OpenRTI_ServerModel_h
#define OpenRTI_ServerModel_h

#include <list>
#include <string>

#include "AbstractMessageSender.h"
#include "Exception.h"
#include "Handle.h"
#include "HandleAllocator.h"
#include "IntrusiveList.h"
#include "IntrusiveUnorderedMap.h"
#include "VariableLengthDataTuple.h"
#include "Referenced.h"
#include "RegionSet.h"
#include "ServerOptions.h"
#include "StringUtils.h"
#include "Types.h"
#include "LogStream.h"

#include <memory>

namespace OpenRTI {

class MomServer;
class AbstractServer;
struct AbstractFederateMetrics;

namespace ServerModel {

// Helper return type for some publish/subscribe stuff.
// Define outside the template class to avoid having different types of this per template
enum PropagationType {
  PropagateBroadcast,
  PropagateSend,
  PropagateNone
};

inline std::string to_string(PropagationType value)
{
  switch (value)
  {
  case PropagateBroadcast : return "PropagateBroadcast";
  case PropagateSend : return "PropagateSend";
  case PropagateNone : return "PropagateNone";
  default : return "<Invalid PropagationType>";
  }
}

typedef std::pair<PropagationType, ConnectHandle> PropagationTypeConnectHandlePair;

class OPENRTI_LOCAL BroadcastConnectHandleSet {
 public:
  // Returns what to do with the other connects in the server node.
  // If .first is PropagateBroadcast, tell all but this one that sends this publication about the publication change.
  // If .first is PropagateSend, tell the connect at .second about the change.
  // If .first is PropagateNone, nothing has changed for the connects at this server node.
  PropagationTypeConnectHandlePair insert(const ConnectHandle& connectHandle)
  {
    // Mark this connect as publishing
    size_t initialConnectHandleSetSize = _connectHandleSet.size();
    ConnectHandle previousExclusiveConnectHandle;
    if (initialConnectHandleSetSize == 1)
      previousExclusiveConnectHandle = *_connectHandleSet.begin();

    // Did that change something? If not, propagate nothing
    if (!_connectHandleSet.insert(connectHandle).second)
      return PropagationTypeConnectHandlePair(PropagateNone, ConnectHandle());

    switch (initialConnectHandleSetSize) {
    case 0:
      // Nobody was publishing this so far. Tell all *others* about that
      return PropagationTypeConnectHandlePair(PropagateBroadcast, ConnectHandle());
    case 1:
      // One was publishing this so far. Due to not getting the loopback message in the 0 case just send that
      // remaining connect that does not yet know that this server node is already publishing this the publication
      // notification
      return PropagationTypeConnectHandlePair(PropagateSend, previousExclusiveConnectHandle);
    default:
      // Nothing changes, nothing todo
      return PropagationTypeConnectHandlePair(PropagateNone, ConnectHandle());
    }
  }

  // Returns what to do with the other connects in the server node.
  // If .first is PropagateBroadcast, tell all but this one that sends this publication about the publication change.
  // If .first is PropagateSend, tell the connect at .second about the change.
  // If .first is PropagateNone, nothing has changed for the connects at this server node.
  PropagationTypeConnectHandlePair erase(const ConnectHandle& connectHandle)
  {
    // Did that change something? If not, propagate nothing
    if (_connectHandleSet.erase(connectHandle) == 0)
      return PropagationTypeConnectHandlePair(PropagateNone, ConnectHandle());

    switch (_connectHandleSet.size()) {
    case 0:
      return PropagationTypeConnectHandlePair(PropagateBroadcast, ConnectHandle());
    case 1:
      return PropagationTypeConnectHandlePair(PropagateSend, *_connectHandleSet.begin());
    default:
      // Nothing changes, nothing todo
      return PropagationTypeConnectHandlePair(PropagateNone, ConnectHandle());
    }
  }

  // Returns if any existing connect in this server node publishes this
  bool empty() const noexcept { return _connectHandleSet.empty(); }

  // Returns if the given connect handle publishes this
  bool contains(const ConnectHandle& connectHandle) const
  { return _connectHandleSet.find(connectHandle) != _connectHandleSet.end(); }
  // Returns the subscription type of all connects except the given one
  bool containsMoreThan(const ConnectHandle& connectHandle) const
  {
    size_t size = _connectHandleSet.size();
    if (1 < size)
      return true;
    if (size == 1 && *_connectHandleSet.begin() != connectHandle)
      return true;
    return false;
  }

  const ConnectHandleSet& getConnectHandleSet() const noexcept { return _connectHandleSet; }

 private:
  ConnectHandleSet _connectHandleSet;
};

class OPENRTI_LOCAL PublishSubscribe {
public:

  // Change publication state of the given connect.
  // Returns what to do with the other connects in the server node.
  // If .first is PropagateBroadcast, tell all but this one that sends this publication about the publication change.
  // If .first is PropagateSend, tell the connect at .second about the change.
  // If .first is PropagateNone, nothing has changed for the connects at this server node.
  PropagationTypeConnectHandlePair setPublicationType(const ConnectHandle& connectHandle, PublicationType publicationType)
  {
    if (publicationType == Published) {
      return _publishedConnects.insert(connectHandle);
    } else {
      return _publishedConnects.erase(connectHandle);
    }
  }
  // Returns if the given connect handle publishes this
  PublicationType getPublicationType(const ConnectHandle& connectHandle) const
  {
    if (_publishedConnects.contains(connectHandle))
      return Published;
    else
      return Unpublished;
  }
  // Returns if any existing connect in this server node publishes this
  PublicationType getPublicationType() const
  {
    if (_publishedConnects.empty())
      return Unpublished;
    else
      return Published;
  }
  const ConnectHandleSet& getPublishingConnectHandleSet() const
  { return _publishedConnects.getConnectHandleSet(); }

  // FIXME currently only passive subscriptions are propagated

  // Change publication state of the given connect.
  // Returns what to do with the other connects in the server node.
  // If .first is PropagateBroadcast, tell all but this one that sends this publication about the publication change.
  // If .first is PropagateSend, tell the connect at .second about the change.
  // If .first is PropagateNone, nothing has changed for the connects at this server node.
  PropagationTypeConnectHandlePair setSubscriptionType(const ConnectHandle& connectHandle, SubscriptionType subscriptionType)
  {
    // if (subscriptionType == Subscribed) {
    if (subscriptionType != Unsubscribed) {
      return _subscribedConnects.insert(connectHandle);
    } else {
      return _subscribedConnects.erase(connectHandle);
    }
  }

  SubscriptionType getSubscriptionType() const
  {
    if (!_activeSubscribedConnects.empty())
      return SubscribedActive;
    else if (!_subscribedConnects.empty())
      return SubscribedPassive;
    else
      return Unsubscribed;
  }

  SubscriptionType getSubscriptionType(const ConnectHandle& connectHandle) const
  {
    if (_activeSubscribedConnects.contains(connectHandle))
      return SubscribedActive;
    else if (_subscribedConnects.contains(connectHandle))
      return SubscribedPassive;
    else
      return Unsubscribed;
  }
  // Returns the subscription type of all connects except the given one
  SubscriptionType getSubscriptionTypeToConnect(const ConnectHandle& connectHandle) const
  {
    if (_activeSubscribedConnects.containsMoreThan(connectHandle))
      return SubscribedActive;
    if (_subscribedConnects.containsMoreThan(connectHandle))
      return SubscribedPassive;
    return Unsubscribed;
  }

  // FIXME This returns only the passive subscriptions
  const ConnectHandleSet& getSubscribedConnectHandleSet() const
  { return _subscribedConnects.getConnectHandleSet(); }

  // FIXME This returns only the passive subscriptions
  void getSubscribedAndIntersectingConnectHandleSet(ConnectHandleSet& connectHandleSet, const RegionSet& /*regionSet*/) const
  { connectHandleSet = _subscribedConnects.getConnectHandleSet(); }

  void removeConnect(const ConnectHandle& connectHandle)
  {
    // FIXME, should not use this
    setSubscriptionType(connectHandle, Unsubscribed);

    // The unpublish must have happened before, so just make sure that it is empty
    OpenRTIAssert(!_publishedConnects.contains(connectHandle));
    // The unsubscribe must have happened before, so just make sure that it is empty
    OpenRTIAssert(!_activeSubscribedConnects.contains(connectHandle));
    OpenRTIAssert(!_subscribedConnects.contains(connectHandle));
  }

  // returns true if something changed
  bool updateCumulativeSubscribedConnectHandleSet(const ConnectHandle& connectHandle, bool subscribe)
  {
    if (subscribe) {
      return _cumulativeSubscribedConnectHandleSet.insert(connectHandle).second;
    } else {
      return 0 != _cumulativeSubscribedConnectHandleSet.erase(connectHandle);
    }
  }

  /// The cumulative set of connects that subscribe to this.
  /// Cumulative in the sense that also subscriptions to base classes are included here.
  ConnectHandleSet _cumulativeSubscribedConnectHandleSet;

 private:
  // All the connects that publish this
  BroadcastConnectHandleSet _publishedConnects;

  // All the connects that are regionless subscribed at this
  // Since region subscriptions should not interfere with normal subscriptions, have these two and something for
  // regions set subscriptions without any active/passive flag
  BroadcastConnectHandleSet _subscribedConnects;
  BroadcastConnectHandleSet _activeSubscribedConnects;
};

///// FIXME above here should also move into a clean referencing scheme to the connects and what not.
////////////////////////////////////////////////////////////

template<typename T, typename H>
class OPENRTI_LOCAL HandleEntity : public IntrusiveUnorderedMap<H, T>::Hook {
public:
  typedef IntrusiveUnorderedMap<H, T> HandleMap;

protected:
  HandleEntity()
  { }
  HandleEntity(const H& handle) :
    IntrusiveUnorderedMap<H, T>::Hook(handle)
  { }

  const H& _getHandle() const
  { return HandleMap::Hook::getKey(); }
  void _setHandle(const H& handle)
  { HandleMap::Hook::setKey(handle); }
};

////////////////////////////////////////////////////////////

template<typename T, typename H>
class OPENRTI_LOCAL HandleListEntity : public IntrusiveUnorderedMap<H, T>::Hook, public IntrusiveList<T, 0>::Hook {
public:
  typedef IntrusiveUnorderedMap<H, T> HandleMap;
  typedef IntrusiveList<T, 0> FirstList;

protected:
  HandleListEntity() noexcept { }
  HandleListEntity(const H& handle)
    : IntrusiveUnorderedMap<H, T>::Hook(handle)
  { }

  const H& _getHandle() const { return HandleMap::Hook::getKey(); }
  void _setHandle(const H& handle) { HandleMap::Hook::setKey(handle); }
};

////////////////////////////////////////////////////////////

template<typename T, typename H, typename S = std::string>
class OPENRTI_LOCAL HandleStringEntity : public IntrusiveUnorderedMap<H, T>::Hook, public IntrusiveUnorderedMap<S, T>::Hook {
public:
  typedef IntrusiveUnorderedMap<H, T> HandleMap;
  typedef IntrusiveUnorderedMap<S, T> StringMap;

protected:
  HandleStringEntity() noexcept { }
  HandleStringEntity(const H& handle, const S& name)
    : IntrusiveUnorderedMap<H, T>::Hook(handle)
    , IntrusiveUnorderedMap<S, T>::Hook(name)
  { }

  const H& _getHandle() const { return HandleMap::Hook::getKey(); }
  void _setHandle(const H& handle) { HandleMap::Hook::setKey(handle); }

  const S& _getString() const { return StringMap::Hook::getKey(); }
  void _setString(const S& string) { StringMap::Hook::setKey(string); }
};

////////////////////////////////////////////////////////////

template<typename T>
class OPENRTI_LOCAL ListPair : public IntrusiveList<T, 0>::Hook, public IntrusiveList<T, 1>::Hook {
public:
  typedef IntrusiveList<T, 0> FirstList;
  typedef IntrusiveList<T, 1> SecondList;
};

////////////////////////////////////////////////////////////

// FIXME: make the Region some fixed definition but the RegionSubscription, RegionAssociation
// a tree element that is fast in its lookup!!

// IDEA, put often used query combinations in a hash and try to cache the results - at least for interaction delivery.

// class OPENRTI_LOCAL RegionSubscription {
// };

class OPENRTI_LOCAL Region : public HandleEntity<Region, LocalRegionHandle> {
public:
  typedef HandleEntity<Region, LocalRegionHandle>::HandleMap HandleMap;

  Region();
  ~Region();

  const LocalRegionHandle& getRegionHandle() const
  { return HandleEntity<Region, LocalRegionHandle>::_getHandle(); }
  void setRegionHandle(const LocalRegionHandle& regionHandle);

  RegionValue _regionValue;
  DimensionHandleSet _dimensionHandleSet;

private:
  Region(const Region&) = delete;
  Region& operator=(const Region&) = delete;
};

////////////////////////////////////////////////////////////

class ClassAttribute;
class ObjectInstance;

class OPENRTI_LOCAL InstanceAttribute : public HandleEntity<InstanceAttribute, AttributeHandle> {
public:
  typedef HandleEntity<InstanceAttribute, AttributeHandle>::HandleMap HandleMap;

  InstanceAttribute(ObjectInstance& objectInstance, ClassAttribute& classAttribute);
  ~InstanceAttribute();

  const AttributeHandle& getAttributeHandle() const {
    return HandleEntity<InstanceAttribute, AttributeHandle>::_getHandle();
  }
  void setAttributeHandle(const AttributeHandle& attributeHandle);

  const ObjectInstance& getObjectInstance() const noexcept { return _objectInstance; }
  ObjectInstance& getObjectInstance() noexcept { return _objectInstance; }

  const ClassAttribute& getClassAttribute() const noexcept { return _classAttribute; }
  ClassAttribute& getClassAttribute() noexcept { return _classAttribute; }

  /// Get the ConnectHandle this attribute is owned
  const ConnectHandle& getOwnerConnectHandle() const noexcept { return _ownerConnectHandle; }
  void setOwnerConnectHandle(const ConnectHandle& connectHandle)
  {
    _receivingConnects.erase(connectHandle);
    _ownerConnectHandle = connectHandle;
  }
  const FederateHandle& getOwnerFederate() const noexcept { return _ownerFederate; }
  void setOwnerFederate(const FederateHandle& federateHandle);

  void removeConnect(const ConnectHandle& connectHandle);

  // Because of attribute ownership, it is clear for an object attribute where the update
  // stems from, so just have a set of connect handles that want to receive the updates
  ConnectHandleSet _receivingConnects;

  /// The connect this attribute is owned by
  ConnectHandle _ownerConnectHandle;

  /// The federate this attribute is owned by
  FederateHandle _ownerFederate;

private:
  InstanceAttribute(const InstanceAttribute&) = delete;
  InstanceAttribute& operator=(const InstanceAttribute&) = delete;

  ObjectInstance& _objectInstance;
  ClassAttribute& _classAttribute;
};

////////////////////////////////////////////////////////////

class FederationConnect;
class ObjectInstance;

class OPENRTI_LOCAL ObjectInstanceConnect : public HandleListEntity<ObjectInstanceConnect, ConnectHandle> {
public:
  typedef HandleListEntity<ObjectInstanceConnect, ConnectHandle>::HandleMap HandleMap;
  typedef HandleListEntity<ObjectInstanceConnect, ConnectHandle>::FirstList FirstList;

  ObjectInstanceConnect(ObjectInstance& objectInstance, FederationConnect& federationConnect);
  ~ObjectInstanceConnect();

  /// The connect handle to identify this connect
  const ConnectHandle& getConnectHandle() const noexcept { return HandleMap::Hook::getKey(); }
  void setConnectHandle(const ConnectHandle& connectHandle);

  const ObjectInstance& getObjectInstance() const noexcept { return _objectInstance; }
  ObjectInstance& getObjectInstance() noexcept { return _objectInstance; }

  const FederationConnect& getFederationConnect() const noexcept { return _federationConnect; }
  FederationConnect& getFederationConnect() noexcept { return _federationConnect; }

private:
  ObjectInstanceConnect(const ObjectInstanceConnect&) = delete;
  ObjectInstanceConnect& operator=(const ObjectInstanceConnect&) = delete;

  ObjectInstance& _objectInstance;
  FederationConnect& _federationConnect;
};

////////////////////////////////////////////////////////////

class Federation;
class ObjectClass;

class OPENRTI_LOCAL ObjectInstance : public HandleStringEntity<ObjectInstance, ObjectInstanceHandle>, public IntrusiveList<ObjectInstance, 0>::Hook,
                                     public PublishSubscribe {
public:
  typedef HandleStringEntity<ObjectInstance, ObjectInstanceHandle>::HandleMap HandleMap;
  typedef HandleStringEntity<ObjectInstance, ObjectInstanceHandle>::StringMap NameMap;
  typedef IntrusiveList<ObjectInstance, 0> FirstList;

  ObjectInstance(Federation& federation);
  ~ObjectInstance();
  std::string getEntityName() const {
    return std::string("<ObjectInstance ") + getName() + std::string(">");
  }

  const ObjectInstanceHandle& getObjectInstanceHandle() const
  { return HandleStringEntity<ObjectInstance, ObjectInstanceHandle>::_getHandle(); }
  void setObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle);

  const std::string& getName() const
  { return HandleStringEntity<ObjectInstance, ObjectInstanceHandle>::_getString(); }
  void setName(const std::string& name);

  const Federation& getFederation() const noexcept { return _federation; }
  Federation& getFederation() noexcept { return _federation; }

  //void insert(InstanceAttribute& instanceAttribute);
  InstanceAttribute* getInstanceAttribute(const AttributeHandle& attributeHandle);
  InstanceAttribute* getPrivilegeToDeleteInstanceAttribute();
  InstanceAttribute::HandleMap& getAttributeHandleInstanceAttributeMap() noexcept { return _attributeHandleInstanceAttributeMap; }

  ObjectInstanceConnect::HandleMap& getConnectHandleObjectInstanceConnectMap() noexcept { return _connectHandleObjectInstanceConnectMap; }
  /// Mark the name handle pair also represented with this as used in the federationConnect
  void reference(FederationConnect& federationConnect);
  /// Releases the ObjectInstanceConnect entry belonging to the connectHandle
  bool unreference(const ConnectHandle& connectHandle);

  void removeConnect(const ConnectHandle& connectHandle);

  ObjectClass* getObjectClass() const
  { return _objectClass; }
  void setObjectClass(ObjectClass* objectClass);

  /// Return the connect that owns this object
  ConnectHandle getOwnerConnectHandle()
  {
    const InstanceAttribute* instanceAttribute = getInstanceAttribute(AttributeHandle(0));
    if (!instanceAttribute)
      return ConnectHandle();
    return instanceAttribute->getOwnerConnectHandle();
  }
  void setOwnerConnectHandle(const ConnectHandle& connectHandle)
  {
    InstanceAttribute* instanceAttribute = getInstanceAttribute(AttributeHandle(0));
    if (!instanceAttribute)
      return;
    instanceAttribute->setOwnerConnectHandle(connectHandle);
  }

  FederateHandle getOwnerFederate()
  {
    const InstanceAttribute* instanceAttribute = getInstanceAttribute(AttributeHandle(0));
    if (!instanceAttribute)
      return FederateHandle();
    return instanceAttribute->getOwnerFederate();
  }
  void setOwnerFederate(const FederateHandle& federateHandle)
  {
    InstanceAttribute* instanceAttribute = getInstanceAttribute(AttributeHandle(0));
    if (!instanceAttribute)
      return;
    instanceAttribute->setOwnerFederate(federateHandle);
  }

private:
  ObjectInstance(const ObjectInstance&) = delete;
  ObjectInstance& operator=(const ObjectInstance&) = delete;

  Federation& _federation;

  /// The pointer to the object class this object is an instance of, can be zero
  ObjectClass* _objectClass;

  /// All the instance attributes here
  InstanceAttribute::HandleMap _attributeHandleInstanceAttributeMap;

  // List of object instance handle/name references at this connect.
  ObjectInstanceConnect::HandleMap _connectHandleObjectInstanceConnectMap;
};

////////////////////////////////////////////////////////////

class Federate;
class Synchronization;

class OPENRTI_LOCAL SynchronizationFederate : public HandleListEntity<SynchronizationFederate, FederateHandle>  {
public:
  typedef HandleListEntity<SynchronizationFederate, FederateHandle>::HandleMap HandleMap;
  typedef HandleListEntity<SynchronizationFederate, FederateHandle>::FirstList FirstList;

  SynchronizationFederate(Synchronization& synchronization, Federate& federate);
  ~SynchronizationFederate();

  const FederateHandle& getFederateHandle() const
  { return HandleListEntity<SynchronizationFederate, FederateHandle>::_getHandle(); }
  void setFederateHandle(const FederateHandle& federateHandle);

  const Synchronization& getSynchronization() const noexcept { return _synchronization; }
  Synchronization& getSynchronization() noexcept { return _synchronization; }
  const Federate& getFederate() const noexcept { return _federate; }
  Federate& getFederate() noexcept { return _federate; }

  bool getSuccessful() const noexcept { return _successful; }
  void setSuccessful(bool successful);

private:
  SynchronizationFederate(const SynchronizationFederate&) = delete;
  SynchronizationFederate& operator=(const SynchronizationFederate&) = delete;

  Synchronization& _synchronization;
  Federate& _federate;

  bool _successful;
};

////////////////////////////////////////////////////////////

class OPENRTI_LOCAL Synchronization : public IntrusiveUnorderedMap<std::string, Synchronization>::Hook {
public:
  typedef IntrusiveUnorderedMap<std::string, Synchronization> NameMap;

  Synchronization();
  ~Synchronization();

  const std::string& getLabel() const
  { return NameMap::Hook::getKey(); }
  void setLabel(const std::string& label);

  const VariableLengthData& getTag() const noexcept { return _tag; }
  void setTag(const VariableLengthData& tag);

  bool getAddJoiningFederates() const noexcept { return _addJoiningFederates; }
  void setAddJoiningFederates(bool addJoiningFederates);

  bool getIsWaitingFor(const FederateHandle& federateHandle);

  void insert(Federate& federate);
  void achieved(const FederateHandle& federateHandle, bool successful);

  // private:

  // The FederateHandle to SyncronizationFederate map of federates
  // waiting for this synchronization point
  SynchronizationFederate::HandleMap _waitingFederateSyncronizationMap;
  // The FederateHandle to SyncronizationFederate map of federates
  // that have achieved the synchronization point
  SynchronizationFederate::HandleMap _achievedFederateSyncronizationMap;

private:
  Synchronization(const Synchronization&) = delete;
  Synchronization& operator=(const Synchronization&) = delete;

  VariableLengthData _tag;

  bool _addJoiningFederates;
};

////////////////////////////////////////////////////////////

class NodeConnect;
class Federation;
class FederationConnect;

class OPENRTI_LOCAL Federate : public HandleStringEntity<Federate, FederateHandle>, public IntrusiveList<Federate, 0>::Hook, public IntrusiveList<Federate, 1>::Hook, public IntrusiveList<Federate, 2>::Hook {
public:
  typedef HandleStringEntity<Federate, FederateHandle>::HandleMap HandleMap;
  typedef HandleStringEntity<Federate, FederateHandle>::StringMap NameMap;
  typedef IntrusiveList<Federate, 0> FirstList; // Used to access federates from the FederationConnect
  typedef IntrusiveList<Federate, 1> TimeRegulatingList; // Used to access time regulating federates from the FederationConnect
  typedef IntrusiveList<Federate, 2> TimeConstrainedList; // Used to access time constrained federates from the FederationConnect

  Federate(Federation& federation);
  ~Federate();

  const Federation& getFederation() const noexcept { return _federation; }
  Federation& getFederation() noexcept { return _federation; }

  const std::string& getName() const
  { return HandleStringEntity<Federate, FederateHandle>::_getString(); }
  void setName(const std::string& name);

  const FederateHandle& getFederateHandle() const
  { return HandleStringEntity<Federate, FederateHandle>::_getHandle(); }
  void setFederateHandle(const FederateHandle& federateHandle);

  const std::string& getFederateType() const
  { return _federateType; }
  void setFederateType(const std::string& federateType);

  ResignAction getResignAction() const noexcept { return _resignAction; }
  void setResignAction(ResignAction resignAction);

  bool getResignPending() const noexcept { return _resignPending; }
  void setResignPending(bool resignPending);

  void insert(SynchronizationFederate& synchronizationFederate);
  SynchronizationFederate::FirstList& getSynchronizationFederateList()
  { return _synchronizationFederateList; }

  Region* getRegion(const LocalRegionHandle& regionHandle);
  void insert(Region& region)
  { _regionHandleRegionMap.insert(region); }
  // void erase(Region& region)
  // { Region::HandleMap::dispose(region); }
  Region::HandleMap& getRegionHandleRegionMap()
  { return _regionHandleRegionMap; }

  bool getIsTimeRegulating() const;
  bool getIsTimeConstrained() const;

  const VariableLengthData& getTimeAdvanceTimeStamp() const
  { return _timeAdvanceTimeStamp; }
  void setTimeAdvanceTimeStamp(const VariableLengthData& timeAdvanceTimeStamp);

  const VariableLengthData& getNextMessageTimeStamp() const
  { return _nextMessageTimeStamp; }
  void setNextMessageTimeStamp(const VariableLengthData& nextMessageTimeStamp);

  Unsigned getCommitId() const
  { return _commitId; }
  void setCommitId(Unsigned commitId);

  void setFederationConnect(FederationConnect* federationConnect)
  {
    OpenRTIAssert(federationConnect || !getIsTimeRegulating());
    _federationConnect = federationConnect;
  }
  FederationConnect* getFederationConnect() const
  { return _federationConnect; }
  ConnectHandle getConnectHandle() const;

  void setIsInternal(bool isInternal) noexcept { _isInternal=isInternal; }
  bool getIsInternal() const noexcept { return _isInternal; }
  void send(const SharedPtr<const AbstractMessage>& message);

private:
  Federate(const Federate&) = delete;
  Federate& operator=(const Federate&) = delete;

  Federation& _federation;
  std::string _federateType;
  ResignAction _resignAction;
  bool _resignPending;
  FederationConnect* _federationConnect;
  SynchronizationFederate::FirstList _synchronizationFederateList;
  Region::HandleMap _regionHandleRegionMap;

  // Time constrained federates current state
  VariableLengthData _timeAdvanceTimeStamp;
  VariableLengthData _nextMessageTimeStamp;
  Unsigned _commitId;
  bool _isInternal;
};

////////////////////////////////////////////////////////////

class Dimension;
class Module;

class OPENRTI_LOCAL DimensionModule : public ListPair<DimensionModule> {
public:
  DimensionModule(Dimension& dimension, Module& module);
  ~DimensionModule();

  const Dimension& getDimension() const
  { return _dimension; }
  Dimension& getDimension()
  { return _dimension; }
  const Module& getModule() const
  { return _module; }
  Module& getModule()
  { return _module; }

private:
  DimensionModule(const DimensionModule&) = delete;
  DimensionModule& operator=(const DimensionModule&) = delete;

  Dimension& _dimension;
  Module& _module;
};

////////////////////////////////////////////////////////////

template<typename T, typename H, typename S = std::string>
class OPENRTI_LOCAL ModuleEntity : public HandleStringEntity<T, H, S> {
public:

protected:
  ModuleEntity()
  { }
  ModuleEntity(const H& handle, const S& name) :
    HandleStringEntity<T, H, S>(handle, name)
  { }
};

////////////////////////////////////////////////////////////

template<typename T, typename H, typename S = StringVector>
class ModuleClassEntity : public ModuleEntity<T, H, S>, public IntrusiveList<T, 0>::Hook {
public:
  typedef IntrusiveList<T, 0> ChildList;

protected:
  ModuleClassEntity()
  { }
  ModuleClassEntity(const H& handle, const S& name) :
    ModuleEntity<T, H, S>(handle, name)
  { }
};

////////////////////////////////////////////////////////////

class Federation;

class OPENRTI_LOCAL Dimension : public ModuleEntity<Dimension, DimensionHandle> {
public:
  typedef ModuleEntity<Dimension, DimensionHandle>::HandleMap HandleMap;
  typedef ModuleEntity<Dimension, DimensionHandle>::StringMap NameMap;

  Dimension(Federation& federation);
  ~Dimension();

  const Federation& getFederation() const
  { return _federation; }
  Federation& getFederation()
  { return _federation; }

  const std::string& getName() const
  { return ModuleEntity<Dimension, DimensionHandle>::_getString(); }
  void setName(const std::string& name);

  const DimensionHandle& getDimensionHandle() const
  { return ModuleEntity<Dimension, DimensionHandle>::_getHandle(); }
  void setDimensionHandle(const DimensionHandle& dimensionHandle);

  const Unsigned& getUpperBound() const
  { return _upperBound; }
  void setUpperBound(const Unsigned& upperBound);

  bool getIsReferencedByAnyModule() const;

  void insert(DimensionModule& dimensionModule)
  { _dimensionModuleList.push_back(dimensionModule); }

private:
  Dimension(const Dimension&) = delete;
  Dimension& operator=(const Dimension&) = delete;

  Federation& _federation;

  // The upper bound
  Unsigned _upperBound;

  // The list of modules referencing this dimension.
  DimensionModule::SecondList _dimensionModuleList;
};

////////////////////////////////////////////////////////////

class Module;
class UpdateRate;

class OPENRTI_LOCAL UpdateRateModule : public ListPair<UpdateRateModule> {
public:
  UpdateRateModule(UpdateRate& updateRate, Module& module) : _updateRate(updateRate), _module(module) { }
  ~UpdateRateModule() {}

  const UpdateRate& getUpdateRate() const
  { return _updateRate; }
  UpdateRate& getUpdateRate()
  { return _updateRate; }
  const Module& getModule() const
  { return _module; }
  Module& getModule()
  { return _module; }

private:
  UpdateRateModule() = delete;
  UpdateRateModule(const UpdateRateModule&) = delete;
  UpdateRateModule(UpdateRateModule&&) = delete;
  UpdateRateModule& operator=(const UpdateRateModule&) = delete;
  UpdateRateModule& operator=(UpdateRateModule&&) = delete;

  UpdateRate& _updateRate;
  Module& _module;
};

////////////////////////////////////////////////////////////

class Federation;

class OPENRTI_LOCAL UpdateRate : public ModuleEntity<UpdateRate, UpdateRateHandle> {
public:
  typedef ModuleEntity<UpdateRate, UpdateRateHandle>::HandleMap HandleMap;
  typedef ModuleEntity<UpdateRate, UpdateRateHandle>::StringMap NameMap;

  UpdateRate(Federation& federation);
  ~UpdateRate();

  const Federation& getFederation() const { return _federation; }
  Federation& getFederation() { return _federation; }

  const std::string& getName() const { return ModuleEntity<UpdateRate, UpdateRateHandle>::_getString(); }

  void setName(const std::string& name) { ModuleEntity<UpdateRate, UpdateRateHandle>::_setString(name); }

  const UpdateRateHandle& getUpdateRateHandle() const { return ModuleEntity<UpdateRate, UpdateRateHandle>::_getHandle(); }
  void setUpdateRateHandle(const UpdateRateHandle& updateRateHandle) { ModuleEntity<UpdateRate, UpdateRateHandle>::_setHandle(updateRateHandle); }

  const double& getRate() const
  { return _rate; }
  void setRate(const double& rate);


  bool getIsReferencedByAnyModule() const { return !_updateRateModuleList.empty(); }

  void insert(UpdateRateModule& updateRateModule)
  { _updateRateModuleList.push_back(updateRateModule); }

private:
  UpdateRate(const UpdateRate&) = delete;
  UpdateRate& operator=(const UpdateRate&) = delete;

  Federation& _federation;

  double _rate;

  UpdateRateModule::SecondList _updateRateModuleList;
};

class BasicDataType;

class OPENRTI_LOCAL BasicDataTypeModule : public ListPair<BasicDataTypeModule> {
public:
  BasicDataTypeModule(BasicDataType& simpleDataType, Module& module) : _basicDataType(simpleDataType), _module(module) {}
  ~BasicDataTypeModule() {}

  const BasicDataType& getBasicDataType() const
  { return _basicDataType; }
  BasicDataType& getBasicDataType()
  { return _basicDataType; }
  const Module& getModule() const
  { return _module; }
  Module& getModule()
  { return _module; }

private:
  BasicDataTypeModule(const BasicDataTypeModule&) = delete;
  BasicDataTypeModule& operator=(const BasicDataTypeModule&) = delete;

  BasicDataType& _basicDataType;
  Module& _module;
};

class OPENRTI_LOCAL BasicDataType : public ModuleEntity<BasicDataType, BasicDataTypeHandle> {
public:
  typedef ModuleEntity<BasicDataType, BasicDataTypeHandle>::HandleMap HandleMap;
  typedef ModuleEntity<BasicDataType, BasicDataTypeHandle>::StringMap NameMap;

  BasicDataType(Federation& federation) : _federation(federation) { }
  ~BasicDataType() {}

  const Federation& getFederation() const
  { return _federation; }
  Federation& getFederation()
  { return _federation; }

  const std::string& getName() const
  { return ModuleEntity<BasicDataType, BasicDataTypeHandle>::_getString(); }
  void setName(const std::string& name) { ModuleEntity<BasicDataType, BasicDataTypeHandle>::_setString(name); }

  const BasicDataTypeHandle& getHandle() const
  { return ModuleEntity<BasicDataType,BasicDataTypeHandle>::_getHandle(); }
  void setHandle(const BasicDataTypeHandle& simpleDataTypeHandle) { ModuleEntity<BasicDataType,BasicDataTypeHandle>::_setHandle(simpleDataTypeHandle); }

  bool getIsReferencedByAnyModule() const { return !_basicDataTypeModuleList.empty(); }

  void insert(BasicDataTypeModule& simpleDataTypeModule)
  { _basicDataTypeModuleList.push_back(simpleDataTypeModule); }
  void writeCurrentFDD(std::ostream& out, unsigned int level) const;

  uint32_t getSize() const {
    return _size;
  }
  void setSize(uint32_t val) {
    _size = val;
  }
  OpenRTI::Endianness getEndian() const {
    return _endian;
  }
  void setEndian(OpenRTI::Endianness val) {
    _endian = val;
  }
private:
  BasicDataType(const BasicDataType&) = delete;
  BasicDataType(BasicDataType&&) = delete;
  BasicDataType& operator=(const BasicDataType&) = delete;
  BasicDataType& operator=(BasicDataType&&) = delete;

  Federation& _federation;
  uint32_t _size = 0;
  Endianness _endian = Endianness::BigEndian;
  BasicDataTypeModule::SecondList _basicDataTypeModuleList;
};

class SimpleDataType;

class OPENRTI_LOCAL SimpleDataTypeModule : public ListPair<SimpleDataTypeModule> {
public:
  SimpleDataTypeModule(SimpleDataType& simpleDataType, Module& module) : _simpleDataType(simpleDataType), _module(module) {}
  ~SimpleDataTypeModule() {}

  const SimpleDataType& getSimpleDataType() const
  { return _simpleDataType; }
  SimpleDataType& getSimpleDataType()
  { return _simpleDataType; }
  const Module& getModule() const
  { return _module; }
  Module& getModule()
  { return _module; }

private:
  SimpleDataTypeModule(const SimpleDataTypeModule&) = delete;
  SimpleDataTypeModule& operator=(const SimpleDataTypeModule&) = delete;

  SimpleDataType& _simpleDataType;
  Module& _module;
};

class OPENRTI_LOCAL SimpleDataType : public ModuleEntity<SimpleDataType, SimpleDataTypeHandle> {
public:
  typedef ModuleEntity<SimpleDataType, SimpleDataTypeHandle>::HandleMap HandleMap;
  typedef ModuleEntity<SimpleDataType, SimpleDataTypeHandle>::StringMap NameMap;

  SimpleDataType(Federation& federation) : _federation(federation) { }
  ~SimpleDataType() {}

  const Federation& getFederation() const
  { return _federation; }
  Federation& getFederation()
  { return _federation; }

  const std::string& getName() const
  { return ModuleEntity<SimpleDataType, SimpleDataTypeHandle>::_getString(); }
  void setName(const std::string& name) { ModuleEntity<SimpleDataType, SimpleDataTypeHandle>::_setString(name); }

  const std::string& getRepresentation() const { return _representation; }
  void setRepresentation(const std::string& s) { _representation = s; }

  const SimpleDataTypeHandle& getHandle() const
  { return ModuleEntity<SimpleDataType,SimpleDataTypeHandle>::_getHandle(); }
  void setHandle(const SimpleDataTypeHandle& simpleDataTypeHandle) { ModuleEntity<SimpleDataType,SimpleDataTypeHandle>::_setHandle(simpleDataTypeHandle); }

  bool getIsReferencedByAnyModule() const { return !_simpleDataTypeModuleList.empty(); }

  void insert(SimpleDataTypeModule& simpleDataTypeModule)
  { _simpleDataTypeModuleList.push_back(simpleDataTypeModule); }

  void writeCurrentFDD(std::ostream& out, unsigned int level) const;
private:
  SimpleDataType(const SimpleDataType&) = delete;
  SimpleDataType(SimpleDataType&&) = delete;
  SimpleDataType& operator=(const SimpleDataType&) = delete;
  SimpleDataType& operator=(SimpleDataType&&) = delete;

  Federation& _federation;
  std::string _representation;
  SimpleDataTypeModule::SecondList _simpleDataTypeModuleList;
};

class EnumeratedDataType;

class OPENRTI_LOCAL EnumeratedDataTypeModule : public ListPair<EnumeratedDataTypeModule> {
public:
  EnumeratedDataTypeModule(EnumeratedDataType& simpleDataType, Module& module) : _simpleDataType(simpleDataType), _module(module) {}
  ~EnumeratedDataTypeModule() {}

  const EnumeratedDataType& getEnumeratedDataType() const
  { return _simpleDataType; }
  EnumeratedDataType& getEnumeratedDataType()
  { return _simpleDataType; }
  const Module& getModule() const
  { return _module; }
  Module& getModule()
  { return _module; }

private:
  EnumeratedDataTypeModule(const EnumeratedDataTypeModule&) = delete;
  EnumeratedDataTypeModule(EnumeratedDataTypeModule&&) = delete;
  EnumeratedDataTypeModule& operator=(const EnumeratedDataTypeModule&) = delete;
  EnumeratedDataTypeModule& operator=(EnumeratedDataTypeModule&&) = delete;

  EnumeratedDataType& _simpleDataType;
  Module& _module;
};

class OPENRTI_LOCAL Enumerator
{
public:
  Enumerator(const std::string& name , uint32_t value) : _name(name), _value(value) {}

  std::string getName() const {
    return _name;
  }
  void setName(std::string val) {
    _name = val;
  }
  uint32_t getValue() const {
    return _value;
  }
  void setValue(uint32_t val) {
    _value = val;
  }
  std::string _name;
  uint32_t _value;
};

class OPENRTI_LOCAL EnumeratedDataType : public ModuleEntity<EnumeratedDataType, EnumeratedDataTypeHandle> {
public:
  typedef ModuleEntity<EnumeratedDataType, EnumeratedDataTypeHandle>::HandleMap HandleMap;
  typedef ModuleEntity<EnumeratedDataType, EnumeratedDataTypeHandle>::StringMap NameMap;

  EnumeratedDataType(Federation& federation) : _federation(federation) { }
  ~EnumeratedDataType() {}

  const Federation& getFederation() const
  { return _federation; }
  Federation& getFederation()
  { return _federation; }

  const std::string& getName() const
  { return ModuleEntity<EnumeratedDataType, EnumeratedDataTypeHandle>::_getString(); }
  void setName(const std::string& name) { ModuleEntity<EnumeratedDataType, EnumeratedDataTypeHandle>::_setString(name); }

  const EnumeratedDataTypeHandle& getHandle() const
  { return ModuleEntity<EnumeratedDataType,EnumeratedDataTypeHandle>::_getHandle(); }
  void setHandle(const EnumeratedDataTypeHandle& simpleDataTypeHandle) { ModuleEntity<EnumeratedDataType,EnumeratedDataTypeHandle>::_setHandle(simpleDataTypeHandle); }

  bool getIsReferencedByAnyModule() const { return !_simpleDataTypeModuleList.empty(); }

  void insert(EnumeratedDataTypeModule& simpleDataTypeModule)
  { _simpleDataTypeModuleList.push_back(simpleDataTypeModule); }
  void writeCurrentFDD(std::ostream& out, unsigned int level) const;

  void addEnumerator(const std::string name, uint32_t value)
  {
    _enumerators.push_back(Enumerator(name, value));
  }
  std::list<OpenRTI::ServerModel::Enumerator> getEnumerators() const {
    return _enumerators;
  }
  void setEnumerators(std::list<OpenRTI::ServerModel::Enumerator> val) {
    _enumerators = val;
  }
  std::string getRepresentation() const {
    return _representation;
  }
  void setRepresentation(std::string val) {
    _representation = val;
  }
private:
  EnumeratedDataType(const EnumeratedDataType&) = delete;
  EnumeratedDataType(EnumeratedDataType&&) = delete;
  EnumeratedDataType& operator=(const EnumeratedDataType&) = delete;
  EnumeratedDataType& operator=(EnumeratedDataType&&) = delete;

  Federation& _federation;
  std::list<Enumerator> _enumerators;
  std::string _representation;

  EnumeratedDataTypeModule::SecondList _simpleDataTypeModuleList;
};

class ArrayDataType;

class OPENRTI_LOCAL ArrayDataTypeModule : public ListPair<ArrayDataTypeModule> {
public:
  ArrayDataTypeModule(ArrayDataType& simpleDataType, Module& module) : _dataType(simpleDataType), _module(module) {}
  ~ArrayDataTypeModule() {}

  const ArrayDataType& getArrayDataType() const
  { return _dataType; }
  ArrayDataType& getArrayDataType()
  { return _dataType; }
  const Module& getModule() const
  { return _module; }
  Module& getModule()
  { return _module; }

private:
  ArrayDataTypeModule(const ArrayDataTypeModule&) = delete;
  ArrayDataTypeModule& operator=(const ArrayDataTypeModule&) = delete;

  ArrayDataType& _dataType;
  Module& _module;
};

class OPENRTI_LOCAL ArrayDataType : public ModuleEntity<ArrayDataType, ArrayDataTypeHandle> {
public:
  typedef ModuleEntity<ArrayDataType, ArrayDataTypeHandle>::HandleMap HandleMap;
  typedef ModuleEntity<ArrayDataType, ArrayDataTypeHandle>::StringMap NameMap;

  ArrayDataType(Federation& federation) : _federation(federation) { }
  ~ArrayDataType() {}

  const Federation& getFederation() const
  { return _federation; }
  Federation& getFederation()
  { return _federation; }

  const std::string& getName() const
  { return ModuleEntity<ArrayDataType, ArrayDataTypeHandle>::_getString(); }
  void setName(const std::string& name) { ModuleEntity<ArrayDataType, ArrayDataTypeHandle>::_setString(name); }

  std::string getDataType() const noexcept { return _dataType; }
  void setDataType(const std::string newValue) {
    _dataType = newValue;
  }

  ArrayDataTypeEncoding getEncoding() const noexcept { return _encoding; }
  void setEncoding(ArrayDataTypeEncoding newValue) noexcept {
    _encoding = newValue;
  }

  std::string getCardinality() const {
    return _cardinality;
  }
  void setCardinality(std::string val) {
    _cardinality = val;
  }

  const ArrayDataTypeHandle& getHandle() const
  { return ModuleEntity<ArrayDataType,ArrayDataTypeHandle>::_getHandle(); }
  void setHandle(const ArrayDataTypeHandle& simpleDataTypeHandle) { ModuleEntity<ArrayDataType,ArrayDataTypeHandle>::_setHandle(simpleDataTypeHandle); }

  bool getIsReferencedByAnyModule() const { return !_moduleList.empty(); }

  void insert(ArrayDataTypeModule& dataTypeModule)
  { _moduleList.push_back(dataTypeModule); }

  void writeCurrentFDD(std::ostream& out, unsigned int level) const;

private:
  ArrayDataType(const ArrayDataType&) = delete;
  ArrayDataType(ArrayDataType&&) = delete;
  ArrayDataType& operator=(const ArrayDataType&) = delete;
  ArrayDataType& operator=(ArrayDataType&&) = delete;

  Federation& _federation;
  std::string _dataType;
  std::string _cardinality;
  ArrayDataTypeEncoding _encoding = ArrayDataTypeEncoding::FixedArrayDataTypeEncoding;
  ArrayDataTypeModule::SecondList _moduleList;
};

class FixedRecordDataType;

class OPENRTI_LOCAL FixedRecordDataTypeModule : public ListPair<FixedRecordDataTypeModule> {
public:
  FixedRecordDataTypeModule(FixedRecordDataType& fixedRecordDataType, Module& module) : _dataType(fixedRecordDataType), _module(module) {}
  ~FixedRecordDataTypeModule() {}

  const FixedRecordDataType& getFixedRecordDataType() const
  { return _dataType; }
  FixedRecordDataType& getFixedRecordDataType()
  { return _dataType; }
  const Module& getModule() const
  { return _module; }
  Module& getModule()
  { return _module; }

private:
  FixedRecordDataTypeModule(const FixedRecordDataTypeModule&) = delete;
  FixedRecordDataTypeModule(FixedRecordDataTypeModule&&) = delete;
  FixedRecordDataTypeModule& operator=(const FixedRecordDataTypeModule&) = delete;
  FixedRecordDataTypeModule& operator=(FixedRecordDataTypeModule&&) = delete;

  FixedRecordDataType& _dataType;
  Module& _module;
};

class FixedRecordField
{
public:
  FixedRecordField(const std::string name, const std::string& dataType, uint32_t version) : _name(name), _dataType(dataType), _version(version) {}
  std::string getName() const {
    return _name;
  }
  void setName(std::string val) {
    _name = val;
  }
  std::string getDataType() const {
    return _dataType;
  }
  void setDataType(std::string val) {
    _dataType = val;
  }
  std::string _name;
  std::string _dataType;
  uint32_t _version;
  uint32_t getVersion() const {
    return _version;
  }
  void setVersion(uint32_t val) {
    _version = val;
  }
};

class OPENRTI_LOCAL FixedRecordDataType : public ModuleEntity<FixedRecordDataType, FixedRecordDataTypeHandle> {
public:
  typedef ModuleEntity<FixedRecordDataType, FixedRecordDataTypeHandle>::HandleMap HandleMap;
  typedef ModuleEntity<FixedRecordDataType, FixedRecordDataTypeHandle>::StringMap NameMap;

  FixedRecordDataType(Federation& federation) : _federation(federation) { }
  ~FixedRecordDataType() {}

  const Federation& getFederation() const
  { return _federation; }
  Federation& getFederation()
  { return _federation; }

  const std::string& getName() const
  { return ModuleEntity<FixedRecordDataType, FixedRecordDataTypeHandle>::_getString(); }
  void setName(const std::string& name) { ModuleEntity<FixedRecordDataType, FixedRecordDataTypeHandle>::_setString(name); }

  const FixedRecordDataTypeHandle& getHandle() const
  { return ModuleEntity<FixedRecordDataType,FixedRecordDataTypeHandle>::_getHandle(); }
  void setHandle(const FixedRecordDataTypeHandle& handle) { ModuleEntity<FixedRecordDataType,FixedRecordDataTypeHandle>::_setHandle(handle); }

  bool getIsReferencedByAnyModule() const { return !_fixedRecordDataTypeModuleList.empty(); }

  void insert(FixedRecordDataTypeModule& module)
  { _fixedRecordDataTypeModuleList.push_back(module); }
  void writeCurrentFDD(std::ostream& out, unsigned int level) const;

  void addField(const std::string name, const std::string& dataType, uint32_t version=0)
  {
    _fields.push_back(FixedRecordField(name, dataType, version));
  }
  std::list<OpenRTI::ServerModel::FixedRecordField> getFields() const {
    return _fields;
  }
  void setFields(const std::list<OpenRTI::ServerModel::FixedRecordField>& val) {
    _fields = val;
  }
  std::string getEncoding() const {
    return _encoding;
  }
  void setEncoding(std::string val) {
    _encoding = val;
  }
  std::string getInclude() const {
    return _include;
  }
  void setInclude(std::string val) {
    _include = val;
  }
  uint32_t getVersion() const {
    return _version;
  }
  void setVersion(uint32_t val) {
    _version = val;
  }
private:
  FixedRecordDataType(const FixedRecordDataType&) = delete;
  FixedRecordDataType(FixedRecordDataType&&) = delete;
  FixedRecordDataType& operator=(const FixedRecordDataType&) = delete;
  FixedRecordDataType& operator=(FixedRecordDataType&&) = delete;

  Federation& _federation;

  std::list<FixedRecordField> _fields;
  std::string _encoding;
  std::string _include;
  uint32_t _version = 0;
  FixedRecordDataTypeModule::SecondList _fixedRecordDataTypeModuleList;
};

class VariantRecordDataType;

class OPENRTI_LOCAL VariantRecordDataTypeModule : public ListPair<VariantRecordDataTypeModule> {
public:
  VariantRecordDataTypeModule(VariantRecordDataType& simpleDataType, Module& module) : _variantRecordDataType(simpleDataType), _module(module) {}
  ~VariantRecordDataTypeModule() {}

  const VariantRecordDataType& getVariantRecordDataType() const { return _variantRecordDataType; }
  VariantRecordDataType& getVariantRecordDataType() { return _variantRecordDataType; }
  const Module& getModule() const { return _module; }
  Module& getModule() { return _module; }

private:
  VariantRecordDataTypeModule(const VariantRecordDataTypeModule&) = delete;
  VariantRecordDataTypeModule(VariantRecordDataTypeModule&&) = delete;
  VariantRecordDataTypeModule& operator=(const VariantRecordDataTypeModule&) = delete;
  VariantRecordDataTypeModule& operator=(VariantRecordDataTypeModule&&) = delete;

  VariantRecordDataType& _variantRecordDataType;
  Module& _module;
};

class OPENRTI_LOCAL VariantRecordAlternative
{
public:
  VariantRecordAlternative(const std::string& name, const std::string& enumerator, const std::string& dataType)
    : _enumerator(enumerator), _name(name), _dataType(dataType)
  {
  }
  std::string getDataType() const {
    return _dataType;
  }
  void setDataType(std::string val) {
    _dataType = val;
  }
private:
  std::string _enumerator;
  std::string _name;
  std::string _dataType;
};

class OPENRTI_LOCAL VariantRecordDataType : public ModuleEntity<VariantRecordDataType, VariantRecordDataTypeHandle> {
public:
  typedef ModuleEntity<VariantRecordDataType, VariantRecordDataTypeHandle>::HandleMap HandleMap;
  typedef ModuleEntity<VariantRecordDataType, VariantRecordDataTypeHandle>::StringMap NameMap;

  VariantRecordDataType(Federation& federation) : _federation(federation) { }
  ~VariantRecordDataType() {}

  const Federation& getFederation() const
  { return _federation; }
  Federation& getFederation()
  { return _federation; }

  const std::string& getName() const
  { return ModuleEntity<VariantRecordDataType, VariantRecordDataTypeHandle>::_getString(); }
  void setName(const std::string& name) { ModuleEntity<VariantRecordDataType, VariantRecordDataTypeHandle>::_setString(name); }

  std::string getDataType() const {
    return _dataType;
  }
  void setDataType(std::string val) {
    _dataType = val;
  }

  const VariantRecordDataTypeHandle& getHandle() const { return ModuleEntity<VariantRecordDataType,VariantRecordDataTypeHandle>::_getHandle(); }
  void setHandle(const VariantRecordDataTypeHandle& handle) {
    ModuleEntity<VariantRecordDataType,VariantRecordDataTypeHandle>::_setHandle(handle);
  }

  bool getIsReferencedByAnyModule() const { return !_variantRecordDataTypeModuleList.empty(); }

  void insert(VariantRecordDataTypeModule& module)
  { _variantRecordDataTypeModuleList.push_back(module); }
  void writeCurrentFDD(std::ostream& out, unsigned int level) const;

  std::string getDiscriminant() const {
    return _discriminant;
  }
  void setDiscriminant(std::string val) {
    _discriminant = val;
  }
  std::string getEncoding() const {
    return _encoding;
  }
  void setEncoding(std::string val) {
    _encoding = val;
  }
  std::list<OpenRTI::ServerModel::VariantRecordAlternative> getAlternatives() const {
    return _alternatives;
  }
  void setAlternatives(const std::list<OpenRTI::ServerModel::VariantRecordAlternative>& val) {
    _alternatives = val;
  }
  void setAlternatives(std::list<OpenRTI::ServerModel::VariantRecordAlternative>&& val) {
    _alternatives = std::move(val);
  }
private:
  VariantRecordDataType(const VariantRecordDataType&) = delete;
  VariantRecordDataType(VariantRecordDataType&&) = delete;
  VariantRecordDataType& operator=(const VariantRecordDataType&) = delete;
  VariantRecordDataType& operator=(VariantRecordDataType&&) = delete;

  Federation& _federation;

  std::string _discriminant;
  std::string _dataType;
  std::string _encoding;
  std::list<VariantRecordAlternative> _alternatives;

  VariantRecordDataTypeModule::SecondList _variantRecordDataTypeModuleList;
};

////////////////////////////////////////////////////////////

class InteractionClass;
class Module;

class OPENRTI_LOCAL InteractionClassModule : public ListPair<InteractionClassModule> {
public:
  InteractionClassModule(InteractionClass& interactionClass, Module& module);
  ~InteractionClassModule();

  const InteractionClass& getInteractionClass() const
  { return _interactionClass; }
  InteractionClass& getInteractionClass()
  { return _interactionClass; }
  const Module& getModule() const
  { return _module; }
  Module& getModule()
  { return _module; }

private:
  InteractionClassModule(const InteractionClassModule&);
  InteractionClassModule& operator=(const InteractionClassModule&);

  InteractionClass& _interactionClass;
  Module& _module;
};

////////////////////////////////////////////////////////////

class InteractionClass;
class Module;

class OPENRTI_LOCAL ParameterDefinitionModule : public ListPair<ParameterDefinitionModule> {
public:
  ParameterDefinitionModule(InteractionClass& interactionClass, Module& module);
  ~ParameterDefinitionModule();

  const InteractionClass& getInteractionClass() const
  { return _interactionClass; }
  InteractionClass& getInteractionClass()
  { return _interactionClass; }
  const Module& getModule() const
  { return _module; }
  Module& getModule()
  { return _module; }

private:
  ParameterDefinitionModule(const ParameterDefinitionModule&);
  ParameterDefinitionModule& operator=(const ParameterDefinitionModule&);

  InteractionClass& _interactionClass;
  Module& _module;
};

////////////////////////////////////////////////////////////

class InteractionClass;
class ParameterDefinition;

class OPENRTI_LOCAL ClassParameter : public HandleListEntity<ClassParameter, ParameterHandle> {
public:
  typedef HandleListEntity<ClassParameter, ParameterHandle>::HandleMap HandleMap;
  typedef HandleListEntity<ClassParameter, ParameterHandle>::FirstList FirstList;

  ClassParameter(InteractionClass& interactionClass, ParameterDefinition& parameterDefinition);
  ~ClassParameter();

  const ParameterHandle& getParameterHandle() const
  { return HandleListEntity<ClassParameter, ParameterHandle>::_getHandle(); }
  void setParameterHandle(const ParameterHandle& parameterHandle);

  const InteractionClass& getInteractionClass() const
  { return _interactionClass; }
  InteractionClass& getInteractionClass()
  { return _interactionClass; }

  const ParameterDefinition& getParameterDefinition() const
  { return _parameterDefinition; }
  ParameterDefinition& getParameterDefinition()
  { return _parameterDefinition; }

private:
  ClassParameter(const ClassParameter&);
  ClassParameter& operator=(const ClassParameter&);

  InteractionClass& _interactionClass;
  ParameterDefinition& _parameterDefinition;
};

////////////////////////////////////////////////////////////

class OPENRTI_LOCAL ParameterDefinition : public HandleStringEntity<ParameterDefinition, ParameterHandle> {
public:
  typedef HandleStringEntity<ParameterDefinition, ParameterHandle>::HandleMap HandleMap;
  typedef HandleStringEntity<ParameterDefinition, ParameterHandle>::StringMap NameMap;

  ParameterDefinition(InteractionClass& interactionClass);
  ~ParameterDefinition();

  const std::string& getName() const
  { return HandleStringEntity<ParameterDefinition, ParameterHandle>::_getString(); }
  void setName(const std::string& name);

  const std::string& getDataType() const { return _dataType; }
  void setDataType(const std::string& dataType) { _dataType = dataType; }

  const ParameterHandle& getParameterHandle() const
  { return HandleStringEntity<ParameterDefinition, ParameterHandle>::_getHandle(); }
  void setParameterHandle(const ParameterHandle& parameterHandle);

  const InteractionClass& getInteractionClass() const
  { return _interactionClass; }
  InteractionClass& getInteractionClass()
  { return _interactionClass; }

  void insert(ClassParameter& classParameter)
  { _classParameterList.push_back(classParameter); }
  ClassParameter::FirstList& getClassParameterList()
  { return _classParameterList; }

  void writeCurrentFDD(std::ostream& out, unsigned int level) const;

private:
  ParameterDefinition(const ParameterDefinition&);
  ParameterDefinition& operator=(const ParameterDefinition&);

  InteractionClass& _interactionClass;

  ClassParameter::FirstList _classParameterList;
  std::string _dataType;
};

////////////////////////////////////////////////////////////

// a set of data vectors
// associates parameter handles with filter value list
// associates federates with parameter filter maps
typedef std::map<ConnectHandle, VariableLengthDataTupleSet> ParameterFilterMapByConnect;

class OPENRTI_LOCAL InteractionClass : public ModuleClassEntity<InteractionClass, InteractionClassHandle>, public PublishSubscribe /*FIXME*/ {
public:
  typedef ModuleClassEntity<InteractionClass, InteractionClassHandle>::HandleMap HandleMap;
  typedef ModuleClassEntity<InteractionClass, InteractionClassHandle>::StringMap NameMap;

  InteractionClass(Federation& federation, InteractionClass* parentInteractionClass = 0);
  ~InteractionClass();

  const Federation& getFederation() const
  { return _federation; }
  Federation& getFederation()
  { return _federation; }

  const StringVector& getName() const
  { return ModuleClassEntity<InteractionClass, InteractionClassHandle>::_getString(); }
  std::string getFQName() const;

  void setName(const StringVector& name);

  const InteractionClassHandle& getInteractionClassHandle() const
  { return ModuleClassEntity<InteractionClass, InteractionClassHandle>::_getHandle(); }
  void setInteractionClassHandle(const InteractionClassHandle& interactionClassHandle);

  OrderType getOrderType() const
  { return _orderType; }
  void setOrderType(OrderType orderType);

  TransportationType getTransportationType() const
  { return _transportationType; }
  void setTransportationType(TransportationType transportationType);

  InteractionClass* getParentInteractionClass();
  InteractionClassHandle getParentInteractionClassHandle() const;

  ChildList& getChildInteractionClassList() { return _childInteractionClassList; }
  const ChildList& getChildInteractionClassList() const { return _childInteractionClassList; }

  bool getIsReferencedByAnyModule() const;
  bool getAreParametersReferencedByAnyModule() const;

  void eraseParameterDefinitions();
  uint32_t getNumParameterDefinitions() const;
  ParameterHandle getFirstUnusedParameterHandle();

  void insert(ParameterDefinition& parameterDefinition);
  ParameterDefinition* getParameterDefinition(const std::string& name);
  ParameterDefinition* getParameterDefinition(const ParameterHandle& parameterHandle);
  ParameterDefinition::HandleMap& getParameterHandleParameterMap()
  { return _parameterHandleParameterMap; }
  ParameterDefinition* findParameterDefinition(const ParameterHandle& parameterHandle);

  void insertClassParameterFor(ParameterDefinition& parameterDefinition);
  ClassParameter* getClassParameter(const ParameterHandle& parameterHandle);
  ClassParameter::HandleMap& getParameterHandleClassParameterMap()
  { return _parameterHandleClassParameterMap; }

  void insert(InteractionClassModule& interactionClassModule)
  { _interactionClassModuleList.push_back(interactionClassModule); }
  InteractionClassModule::SecondList& getInteractionClassModuleList()
  { return _interactionClassModuleList; }

  void insert(ParameterDefinitionModule& parameterDefinitionModule)
  { _parameterDefinitionModuleList.push_back(parameterDefinitionModule); }
  ParameterDefinitionModule::SecondList& getParameterDefinitionModuleList()
  { return _parameterDefinitionModuleList; }

  // FIXME temporarily in this way
  DimensionHandleSet _dimensionHandleSet;

  void updateCumulativeSubscription(const ConnectHandle& connectHandle)
  {
    bool parentSubscribed = false;
    if (_parentInteractionClass &&
        0 != _parentInteractionClass->_cumulativeSubscribedConnectHandleSet.count(connectHandle))
      parentSubscribed = true;

    _updateCumulativeSubscription(connectHandle, parentSubscribed);
  }
  void _updateCumulativeSubscription(const ConnectHandle& connectHandle, bool subscribe /*Replace with regionset or something*/)
  {
    subscribe |= (Unsubscribed != getSubscriptionType(connectHandle));
    if (!updateCumulativeSubscribedConnectHandleSet(connectHandle, subscribe))
      return;
    // Update the receiving connect handle set
    for (ChildList::iterator i = _childInteractionClassList.begin(); i != _childInteractionClassList.end(); ++i) {
      i->_updateCumulativeSubscription(connectHandle, subscribe);
    }
  }

  // returns true if the filter changes for given connect
  bool updateParameterFilterValues(const ConnectHandle& connectHandle, const ParameterValueVector& parameterFilterValues, bool remove=false);
  bool clearParameterFilters(const ConnectHandle& connectHandle);
  bool hasFilterSubscriptions() const;
  bool hasFilterSubscriptions(const ConnectHandle& connectHandle) const;
  bool isMatching(const ConnectHandle& connectHandle, const ParameterValueVector& parameterFilterValues) const;
  std::list<ParameterValueVector> getParameterFilters(const ConnectHandle& connectHandle);

  std::string DumpInteractionFilters();

  void writeCurrentFDD(std::ostream& out, unsigned int level) const;
  const ParameterFilterMapByConnect& getParameterFiltersByConnect() const { return _parameterFiltersByConnect; }
private:
  InteractionClass(const InteractionClass&) = delete;
  InteractionClass& operator=(const InteractionClass&) = delete;

  ParameterValueVector getParameterFilterPrototype() const;
  void NormalizeFilterValues(const ParameterValueVector& parameterFilters, ParameterHandleVector& filterKeyVectorReturn, VariableLengthDataTuple& filterValueVectorReturn) const;
  bool AddParameterFilterValues(VariableLengthDataTupleSet& filterValueTuples, const ParameterValueVector& parameterFilters);
  bool RemoveParameterFilterValues(VariableLengthDataTupleSet& filterValueTuples, const ParameterValueVector& parameterFilters);

  Federation& _federation;

  InteractionClassModule::SecondList _interactionClassModuleList;
  ParameterDefinitionModule::SecondList _parameterDefinitionModuleList;

  InteractionClass* _parentInteractionClass;
  ChildList _childInteractionClassList;

  OrderType _orderType;
  TransportationType _transportationType;

  ParameterDefinition::HandleMap _parameterHandleParameterMap;
  ParameterDefinition::NameMap _parameterNameParameterMap;

  ClassParameter::HandleMap _parameterHandleClassParameterMap;
  ParameterHandleVector _parameterFilterKeyPrototype;
  ParameterFilterMapByConnect _parameterFiltersByConnect;
};

////////////////////////////////////////////////////////////

class Module;
class ObjectClass;

class OPENRTI_LOCAL ObjectClassModule : public ListPair<ObjectClassModule> {
public:
  ObjectClassModule(ObjectClass& objectClass, Module& module);
  ~ObjectClassModule();

  const ObjectClass& getObjectClass() const
  { return _objectClass; }
  ObjectClass& getObjectClass()
  { return _objectClass; }
  const Module& getModule() const
  { return _module; }
  Module& getModule()
  { return _module; }

private:
  ObjectClassModule(const ObjectClassModule&);
  ObjectClassModule& operator=(const ObjectClassModule&);

  ObjectClass& _objectClass;
  Module& _module;
};

////////////////////////////////////////////////////////////

class Module;
class ObjectClass;

class OPENRTI_LOCAL AttributeDefinitionModule : public ListPair<AttributeDefinitionModule> {
public:
  AttributeDefinitionModule(ObjectClass& objectClass, Module& module);
  ~AttributeDefinitionModule();

  const ObjectClass& getObjectClass() const
  { return _objectClass; }
  ObjectClass& getObjectClass()
  { return _objectClass; }
  const Module& getModule() const
  { return _module; }
  Module& getModule()
  { return _module; }

private:
  AttributeDefinitionModule(const AttributeDefinitionModule&);
  AttributeDefinitionModule& operator=(const AttributeDefinitionModule&);

  ObjectClass& _objectClass;
  Module& _module;
};

////////////////////////////////////////////////////////////

class AttributeDefinition;

class OPENRTI_LOCAL ClassAttribute : public HandleListEntity<ClassAttribute, AttributeHandle>,
                                     public PublishSubscribe /*FIXME*/ {
public:
  typedef HandleListEntity<ClassAttribute, AttributeHandle>::HandleMap HandleMap;
  typedef HandleListEntity<ClassAttribute, AttributeHandle>::FirstList FirstList;

  ClassAttribute(ObjectClass& objectClass, AttributeDefinition& attributeDefinition);
  ~ClassAttribute();

  const AttributeHandle& getAttributeHandle() const
  { return HandleListEntity<ClassAttribute, AttributeHandle>::_getHandle(); }
  void setAttributeHandle(const AttributeHandle& attributeHandle);

  const ObjectClass& getObjectClass() const
  { return _objectClass; }
  ObjectClass& getObjectClass()
  { return _objectClass; }

  const AttributeDefinition& getAttributeDefinition() const
  { return _attributeDefinition; }
  AttributeDefinition& getAttributeDefinition()
  { return _attributeDefinition; }

private:
  ClassAttribute(const ClassAttribute&);
  ClassAttribute& operator=(const ClassAttribute&);

  ObjectClass& _objectClass;
  AttributeDefinition& _attributeDefinition;

  // InstanceAttribute::FirstList _instanceAttributeList;
};

////////////////////////////////////////////////////////////

/// May be move the handle out here and move this into the object class attribute??
class OPENRTI_LOCAL AttributeDefinition : public HandleStringEntity<AttributeDefinition, AttributeHandle> {
public:
  typedef HandleStringEntity<AttributeDefinition, AttributeHandle>::HandleMap HandleMap;
  typedef HandleStringEntity<AttributeDefinition, AttributeHandle>::StringMap NameMap;

  AttributeDefinition(ObjectClass& objectClass);
  ~AttributeDefinition();

  const std::string& getName() const
  { return HandleStringEntity<AttributeDefinition, AttributeHandle>::_getString(); }
  void setName(const std::string& name);

  const std::string& getDataType() const { return _dataType; }
  void setDataType(const std::string& dataType) { _dataType = dataType; }

  const AttributeHandle& getAttributeHandle() const
  { return HandleStringEntity<AttributeDefinition, AttributeHandle>::_getHandle(); }
  void setAttributeHandle(const AttributeHandle& attributeHandle);

  const ObjectClass& getObjectClass() const
  { return _objectClass; }
  ObjectClass& getObjectClass()
  { return _objectClass; }

  OrderType getOrderType() const
  { return _orderType; }
  void setOrderType(OrderType orderType);

  TransportationType getTransportationType() const
  { return _transportationType; }
  void setTransportationType(TransportationType transportationType);

  void insert(ClassAttribute& classAttribute)
  { _classAttributeList.push_back(classAttribute); }
  ClassAttribute::FirstList& getClassAttributeList()
  { return _classAttributeList; }


  void writeCurrentFDD(std::ostream& out, unsigned int level) const;

  // FIXME temporarily in this way
  DimensionHandleSet _dimensionHandleSet;

private:
  AttributeDefinition(const AttributeDefinition&);
  AttributeDefinition& operator=(const AttributeDefinition&);

  ObjectClass& _objectClass;

  OrderType _orderType;
  TransportationType _transportationType;

  ClassAttribute::FirstList _classAttributeList;
  std::string _dataType;
};

////////////////////////////////////////////////////////////

class OPENRTI_LOCAL ObjectClass : public ModuleClassEntity<ObjectClass, ObjectClassHandle> {
public:
  typedef ModuleClassEntity<ObjectClass, ObjectClassHandle>::HandleMap HandleMap;
  typedef ModuleClassEntity<ObjectClass, ObjectClassHandle>::StringMap NameMap;

  ObjectClass(Federation& federation, ObjectClass* parentObjectClass = 0);
  ~ObjectClass();

  const Federation& getFederation() const
  { return _federation; }
  Federation& getFederation()
  { return _federation; }

  const StringVector& getName() const
  { return ModuleClassEntity<ObjectClass, ObjectClassHandle>::_getString(); }
  void setName(const StringVector& name);
  std::string getFQName() const;

  const ObjectClassHandle& getObjectClassHandle() const
  { return ModuleClassEntity<ObjectClass, ObjectClassHandle>::_getHandle(); }
  void setObjectClassHandle(const ObjectClassHandle& objectClassHandle);

  ObjectClass* getParentObjectClass();
  ObjectClassHandle getParentObjectClassHandle() const;

  ChildList& getChildObjectClassList() { return _childObjectClassList; }
  const ChildList& getChildObjectClassList() const { return _childObjectClassList; }

  bool getIsReferencedByAnyModule() const;
  bool getAreAttributesReferencedByAnyModule() const;

  void eraseAttributeDefinitions();
  uint32_t getNumAttributeDefinitions() const;
  AttributeHandle getFirstUnusedAttributeHandle();

  void insert(AttributeDefinition& attributeDefinition);
  AttributeDefinition* getAttributeDefinition(const std::string& name);
  AttributeDefinition* getAttributeDefinition(const AttributeHandle& attributeHandle);
  AttributeDefinition::HandleMap& getAttributeHandleAttributeDefinitionMap()
  { return _attributeHandleAttributeDefinitionMap; }

  void insertClassAttributeFor(AttributeDefinition& attributeDefinition);
  ClassAttribute* getClassAttribute(const AttributeHandle& attributeHandle);
  ClassAttribute* getPrivilegeToDeleteClassAttribute();
  ClassAttribute::HandleMap& getAttributeHandleClassAttributeMap()
  { return _attributeHandleClassAttributeMap; }

  void insert(ObjectClassModule& objectClassModule);
  ObjectClassModule::SecondList& getObjectClassModuleList()
  { return _objectClassModuleList; }

  void insert(AttributeDefinitionModule& attributeDefinitionModule);
  AttributeDefinitionModule::SecondList& getAttributeDefinitionModuleList()
  { return _attributeDefinitionModuleList; }

  void insert(ObjectInstance& objectInstance);
  ObjectInstance::FirstList& getObjectInstanceList()
  { return _objectInstanceList; }


  void removeConnect(const ConnectHandle& connectHandle);

  typedef std::list<ObjectInstance*> ObjectInstanceList;

  /// since we might end in different depths for different attributes, this is done per attribute
  void updateCumulativeSubscription(const ConnectHandle& connectHandle,
                                    const AttributeHandle& attributeHandle,
                                    ObjectInstanceList& objectInstanceList);

  void _updateCumulativeSubscription(const ConnectHandle& connectHandle,
                                     const AttributeHandle& attributeHandle,
                                     bool subscribe /*Replace with regionset or something*/,
                                     ObjectInstanceList& objectInstanceList);

  void accumulateAllPublications(ConnectHandleSet& connectHandleSet)
  {
    connectHandleSet.insert(getPrivilegeToDeleteClassAttribute()->getPublishingConnectHandleSet().begin(),
                            getPrivilegeToDeleteClassAttribute()->getPublishingConnectHandleSet().end());
    for (ChildList::iterator i = getChildObjectClassList().begin(); i != getChildObjectClassList().end(); ++i) {
      i->accumulateAllPublications(connectHandleSet);
    }
  }

  void writeCurrentFDD(std::ostream& out, unsigned int level = 2) const;

private:
  ObjectClass(const ObjectClass&);
  ObjectClass& operator=(const ObjectClass&);

  Federation& _federation;

  /// Carries the modules referencint this object class and the attributes
  ObjectClassModule::SecondList _objectClassModuleList;
  AttributeDefinitionModule::SecondList _attributeDefinitionModuleList;

  ObjectClass* _parentObjectClass;
  ChildList _childObjectClassList;

  /// Attributes defined in this object class. Does not contain the inherited attributes.
  AttributeDefinition::HandleMap _attributeHandleAttributeDefinitionMap;
  AttributeDefinition::NameMap _attributeNameAttributeDefinitionMap;

  /// Attributes available in this object class. Does contain also the inherited attributes.
  ClassAttribute::HandleMap _attributeHandleClassAttributeMap;

  /// All instances of this particular class
  ObjectInstance::FirstList _objectInstanceList;
};

////////////////////////////////////////////////////////////

// class OPENRTI_LOCAL ModuleInsertResult {
// public:
//   enum Type {
//     Success,
//     InconsistentFDD
//   };
//   ModuleInsertResult(const Type& type) : _type(type) {}
//   ModuleInsertResult(const Type& type, const std::string& errorMessage) : _type(type), _errorMessage(errorMessage) {}

//   const Type& getType() const
//   { return _type; }
//   const std::string& getErrorMessage() const
//   { return _errorMessage; }

//   operator bool() const { return _type == Success; }

// private:
//   Type _type;
//   std::string _errorMessage;
// };

////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// a Module is the root of a processed FOM file.
// Source is a 
////////////////////////////////////////////////////////////
class OPENRTI_LOCAL Module : public HandleEntity<Module, ModuleHandle> {
public:
  typedef HandleEntity<Module, ModuleHandle>::HandleMap HandleMap;

  Module(Federation& federation, const std::string& designator);
  ~Module();

  const Federation& getFederation() const
  { return _federation; }
  Federation& getFederation()
  { return _federation; }

  const ModuleHandle& getModuleHandle() const
  { return HandleEntity<Module, ModuleHandle>::_getHandle(); }
  void setModuleHandle(const ModuleHandle& moduleHandle);

  const std::string& getContent() const
  { return _content; }
  void setContent(const std::string& content);

  const std::string& getDesignator() const
  { return _designator; }

  bool getArtificialInteractionRoot() const
  { return _artificialInteractionRoot; }
  void setArtificialInteractionRoot(bool artificialInteractionRoot);

  bool getArtificialObjectRoot() const
  { return _artificialObjectRoot; }
  void setArtificialObjectRoot(bool artificialObjectRoot);

  /// Read back the FOMModule context to send this with a message
  void getModule(FOMModule& module);

  // insert into this module
  DimensionModule* insert(Dimension& dimension);
  UpdateRateModule* insert(UpdateRate& updateRate);

  BasicDataTypeModule* insert(BasicDataType& dataType);
  SimpleDataTypeModule* insert(SimpleDataType& dataType);
  ArrayDataTypeModule* insert(ArrayDataType& dataType);
  FixedRecordDataTypeModule* insert(FixedRecordDataType& dataType);
  VariantRecordDataTypeModule* insert(VariantRecordDataType& dataType);

  InteractionClassModule* insert(InteractionClass& interactionClass);
  ParameterDefinitionModule* insertParameters(InteractionClass& interactionClass);
  EnumeratedDataTypeModule* insert(EnumeratedDataType& dataType);
  ObjectClassModule* insert(ObjectClass& objectClass);
  AttributeDefinitionModule* insertAttributes(ObjectClass& objectClass);

  void insertModule(DimensionModule& dimensionModule)
  { _dimensionModuleList.push_back(dimensionModule); }
  DimensionModule::FirstList& getDimensionModuleList()
  { return _dimensionModuleList; }

  void insertModule(UpdateRateModule& updateRateModule)
  { _updateRateModuleList.push_back(updateRateModule); }
  UpdateRateModule::FirstList& getUpdateRateModuleList()
  { return _updateRateModuleList; }

  void insertModule(BasicDataTypeModule& module)
  { _basicDataTypeModuleList.push_back(module); }
  BasicDataTypeModule::FirstList& getBasicDataTypeModuleList()
  { return _basicDataTypeModuleList; }

  void insertModule(SimpleDataTypeModule& module)
  { _simpleDataTypeModuleList.push_back(module); }
  SimpleDataTypeModule::FirstList& getSimpleDataTypeModuleList()
  { return _simpleDataTypeModuleList; }

  void insertModule(EnumeratedDataTypeModule& module)
  { _enumeratedDataTypeModuleList.push_back(module); }
  EnumeratedDataTypeModule::FirstList& getEnumeratedDataTypeModuleList()
  { return _enumeratedDataTypeModuleList; }

  void insertModule(ArrayDataTypeModule& module)
  { _arrayDataTypeModuleList.push_back(module); }
  ArrayDataTypeModule::FirstList& getArrayDataTypeModuleList()
  { return _arrayDataTypeModuleList; }

  void insertModule(FixedRecordDataTypeModule& fixedRecordDataTypeModule)
  { _fixedRecordDataTypeModuleList.push_back(fixedRecordDataTypeModule); }
  FixedRecordDataTypeModule::FirstList& getFixedRecordDataTypeModuleList()
  { return _fixedRecordDataTypeModuleList; }

  void insertModule(VariantRecordDataTypeModule& module)
  { _variantRecordDataTypeModuleList.push_back(module); }
  VariantRecordDataTypeModule::FirstList& getVariantRecordDataTypeModuleList()
  { return _variantRecordDataTypeModuleList; }

  void insertModule(InteractionClassModule& interactionClassModule)
  { _interactionClassModuleList.push_back(interactionClassModule); }
  InteractionClassModule::FirstList& getInteractionClassModuleList()
  { return _interactionClassModuleList; }

  void insertModule(ParameterDefinitionModule& parameterDefinitionModule)
  { _parameterDefinitionModuleList.push_back(parameterDefinitionModule); }
  ParameterDefinitionModule::FirstList& getParameterDefinitionModuleList()
  { return _parameterDefinitionModuleList; }

  void insertModule(ObjectClassModule& objectClassModule)
  { _objectClassModuleList.push_back(objectClassModule); }
  ObjectClassModule::FirstList& getObjectClassModuleList()
  { return _objectClassModuleList; }

  void insertModule(AttributeDefinitionModule& attributeDefinitionModule)
  { _attributeDefinitionModuleList.push_back(attributeDefinitionModule); }
  AttributeDefinitionModule::FirstList& getAttributeDefinitionModuleList()
  { return _attributeDefinitionModuleList; }

private:
  Module(const Module&);
  Module& operator=(const Module&);

  Federation& _federation;

  std::string _content;
  std::string _designator;
  bool _artificialInteractionRoot;
  bool _artificialObjectRoot;

  // All dimensions that are referenced by this module
  DimensionModule::FirstList _dimensionModuleList;
  // All update rates that are referenced by this module
  UpdateRateModule::FirstList _updateRateModuleList;

  // All data types that are referenced by this module
  BasicDataTypeModule::FirstList _basicDataTypeModuleList;
  SimpleDataTypeModule::FirstList _simpleDataTypeModuleList;
  EnumeratedDataTypeModule::FirstList _enumeratedDataTypeModuleList;
  ArrayDataTypeModule::FirstList _arrayDataTypeModuleList;
  FixedRecordDataTypeModule::FirstList _fixedRecordDataTypeModuleList;
  VariantRecordDataTypeModule::FirstList _variantRecordDataTypeModuleList;

  // All interaction classes that are referenced by this module
  InteractionClassModule::FirstList _interactionClassModuleList;
  ParameterDefinitionModule::FirstList _parameterDefinitionModuleList;
  // All object classes that are referenced by this module
  ObjectClassModule::FirstList _objectClassModuleList;
  AttributeDefinitionModule::FirstList _attributeDefinitionModuleList;
};

////////////////////////////////////////////////////////////

class NodeConnect;
class Federation;

class OPENRTI_LOCAL FederationConnect :
    public IntrusiveUnorderedMap<ConnectHandle, FederationConnect>::Hook,
    public IntrusiveList<FederationConnect, 0>::Hook,
    public IntrusiveList<FederationConnect, 1>::Hook,
    public IntrusiveList<FederationConnect, 2>::Hook {
public:
  typedef IntrusiveUnorderedMap<ConnectHandle, FederationConnect> HandleMap;
  typedef IntrusiveList<FederationConnect, 0> FirstList; /// Used to access FederationConnects from a NodeConnect
  typedef IntrusiveList<FederationConnect, 1> TimeRegulatingList; /// Used to access time regulating connects from the federation
  typedef IntrusiveList<FederationConnect, 2> TimeConstrainedList; /// Used to access time constrained connects from the federation

  FederationConnect(Federation& federation, NodeConnect& nodeConnect);
  ~FederationConnect();

  /// Retrieve the handles to the referenced data structures
  const FederationHandle& getFederationHandle() const;
  const ConnectHandle& getConnectHandle() const;
  bool getIsParentConnect() const;
  bool getHasFederates() const;

  const Federation& getFederation() const
  { return _federation; }
  Federation& getFederation()
  { return _federation; }
  const NodeConnect& getNodeConnect() const
  { return _nodeConnect; }
  NodeConnect& getNodeConnect()
  { return _nodeConnect; }

  /// If we should really send messages there.
  /// Also an initial connect is not active to signal that we need to push the
  /// federations context into this connect. Once this is done, the connect is marked active.
  bool getActive() const;
  void setActive(bool active);

  /// The federates hidden behind this connect
  Federate::FirstList& getFederateList()
  { return _federateList; }
  void insert(Federate& federate)
  { OpenRTIAssert(!federate.getFederationConnect()); _federateList.push_back(federate); federate.setFederationConnect(this); }
  void erase(Federate& federate)
  {
    if (!federate.getFederationConnect())
      return;
    OpenRTIAssert(this == federate.getFederationConnect());
    _federateList.unlink(federate);
    federate.setFederationConnect(0);
    //DebugPrintf("%s(this=%p federate=%s): %d federates left\n", __FUNCTION__, this, federate.getName().c_str(), _federateList.size());
    //for (auto iter=_federateList.begin(); iter != _federateList.end(); iter++)
    //{
    //  DebugPrintf("\t%s %s\n",iter->getFederateHandle().toString().c_str(), iter->getName().c_str());
    //}
  }

  /// The time regulating federates hidden behind this connect
  Federate::TimeRegulatingList& getTimeRegulatingFederateList()
  { return _timeRegulatingFederateList; }
  bool getIsTimeRegulating() const;
  void insertTimeRegulating(Federate& federate);
  void eraseTimeRegulating(Federate& federate);

  Federate::TimeConstrainedList& getTimeConstrainedFederateList()
  { return _timeConstrainedFederateList; }
  bool getIsTimeConstrained() const;
  void insertTimeConstrained(Federate& federate);
  void eraseTimeConstrained(Federate& federate);

  /// Object instance handle name pairs referenced by this connect
  void insert(ObjectInstanceConnect& objectInstanceConnect)
  { _objectInstanceConnectList.push_back(objectInstanceConnect); }
  ObjectInstanceConnect::FirstList& getObjectInstanceConnectList()
  { return _objectInstanceConnectList; }

  /// We can actually send something there
  void send(const SharedPtr<const AbstractMessage>& message);

  void sendAndDeactivate(const SharedPtr<const AbstractMessage>& message);
private:
  FederationConnect(const FederationConnect&) = delete;
  FederationConnect& operator=(const FederationConnect&) = delete;

  // The federation backward reference
  Federation& _federation;
  // The NodeConnect backward reference
  NodeConnect& _nodeConnect;

  bool _active;

  /// Federates behind this connect
  Federate::FirstList _federateList;
  /// Time regulating federates behind this connect
  Federate::TimeRegulatingList _timeRegulatingFederateList;
  Federate::TimeConstrainedList _timeConstrainedFederateList;

  // List of object instance handle/name references at this connect.
  ObjectInstanceConnect::FirstList _objectInstanceConnectList;
};

////////////////////////////////////////////////////////////

class Node;

class OPENRTI_LOCAL Federation : public HandleStringEntity<Federation, FederationHandle> {
private:
public:
  typedef HandleStringEntity<Federation, FederationHandle>::HandleMap HandleMap;
  typedef HandleStringEntity<Federation, FederationHandle>::StringMap NameMap;

  Federation(Node& serverNode);
  virtual ~Federation(); // FIXME no virtual

  /// The parent server node this belongs to
  const Node& getServerNode() const
  { return _serverNode; }
  Node& getServerNode()
  { return _serverNode; }
  /// Get the rti servers name or path, from the parent server node
  const std::string& getServerName() const;
  const std::string& getServerPath() const;
  /// Returns true if this is a root server
  bool isRootServer() const;
  /// Returns true if this is the parents server connect
  bool isParentConnect(const ConnectHandle& connectHandle) const;
  bool hasChildConnects() const;
  bool hasChildConnect(const ConnectHandle& connectHandle);
  bool hasJoinedFederates() const;
  /// Returns true if there is any federate joined that belongs to a child connect
  bool hasJoinedChildren();

  const std::string& getName() const
  { return HandleStringEntity<Federation, FederationHandle>::_getString(); }
  void setName(const std::string& name);

  const FederationHandle& getFederationHandle() const
  { return HandleStringEntity<Federation, FederationHandle>::_getHandle(); }
  void setFederationHandle(const FederationHandle& federationHandle);

  // The logical time factorys name.
  const std::string& getLogicalTimeFactoryName() const
  { return _logicalTimeFactoryName; }
  void setLogicalTimeFactoryName(const std::string& logicalTimeFactoryName);

  void send(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message);
  void send(const FederateHandle& federateHandle, const SharedPtr<const AbstractMessage>& message);
  void sendToParent(const SharedPtr<const AbstractMessage>& message);
  void broadcast(const SharedPtr<const AbstractMessage>& message);
  void broadcast(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message);
  void broadcastToChildren(const SharedPtr<const AbstractMessage>& message);
  void broadcastToChildren(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message);

  void insert(FederationConnect& federationConnect)
  { _connectHandleFederationConnectMap.insert(federationConnect); }
  FederationConnect* getFederationConnect(const ConnectHandle& connectHandle);
  FederationConnect::HandleMap& getConnectHandleFederationConnectMap()
  { return _connectHandleFederationConnectMap; }

  void removeConnect(const ConnectHandle& connectHandle);


  /// Access and modify object model modules
  void insert(Dimension& dimension);
  void insert(UpdateRate& updateRate);
  void insert(BasicDataType& dataType);
  void insert(SimpleDataType& dataType);
  void insert(EnumeratedDataType& dataType);
  void insert(ArrayDataType& dataType);
  void insert(FixedRecordDataType& dataType);
  void insert(VariantRecordDataType& dataType);

  void insert(InteractionClass& interactionClass);
  void insert(ObjectClass& objectClass);
  void insert(Module& module);

  /// Either insert a new entity or checks if the provided entity is
  /// compatible with the existing one. Returns true if the entity was newly created.
  bool insertOrCheck(Module& module, const FOMStringDimension& stringDimension);
  bool insertOrCheck(Module& module, const FOMStringUpdateRate& stringUpdateRate);
  bool insertOrCheck(Module& module, const FOMStringBasicDataType& stringDataType);
  bool insertOrCheck(Module& module, const FOMStringSimpleDataType& stringDataType);
  bool insertOrCheck(Module& module, const FOMStringEnumeratedDataType& stringDataType);
  bool insertOrCheck(Module& module, const FOMStringArrayDataType2& stringDataType);
  bool insertOrCheck(Module& module, const FOMStringFixedRecordDataType2& stringDataType);
  bool insertOrCheck(Module& module, const FOMStringVariantRecordDataType2& stringDataType);

  bool insertOrCheck(Module& module, const FOMStringInteractionClass& stringInteractionClass);
  bool insertOrCheck(Module& module, const FOMStringObjectClass& stringObjectClass);

  /// This is for inserting the initial object model
  ModuleHandle insert(const FOMStringModule2& stringModule);
  void insert(const FOMStringModule2List& stringModuleList);

  void insert(ModuleHandleVector& moduleHandleVector, const FOMStringModule2List& stringModuleList);

  /// Either insert a new entity or creates a new one.
  /// Throws a message error if an existing one does not match the provided.
  void insert(Module& module, const FOMDimension& fomDimension);
  void insert(Module& module, const FOMUpdateRate& fomUpdateRate);
  void insert(Module& module, const FOMBasicDataType& fomDataType);
  void insert(Module& module, const FOMSimpleDataType& fomDataType);
  void insert(Module& module, const FOMEnumeratedDataType& fomDataType);
  void insert(Module& module, const FOMArrayDataType& fomDataType);
  void insert(Module& module, const FOMFixedRecordDataType& fomDataType);
  void insert(Module& module, const FOMVariantRecordDataType& fomDataType);
  void insert(Module& module, const FOMInteractionClass& fomInteractionClass);
  void insert(Module& module, const FOMObjectClass& fomObjectClass);
  void insert(Module& module, const FOMTransportationType& fomObjectClass);

  void insert(const FOMModule& fomModule);
  void insert(const FOMModuleList& fomModuleList);
  void insert(const FOMModule2& fomModule);
  void insert(const FOMModule2List& fomModuleList);
  void erase(const ModuleHandle& moduleHandle);
  void erase(Module& module);

  /// To push this to other server nodes, we need to collect the module data for the message.
  void getModuleList(FOMModuleList& moduleList) const;
  void getModuleList(FOMModuleList& moduleList, const ModuleHandleVector& moduleHandleVector) const;
  void writeCurrentFDD(std::ostream& out) const;
  Module* getModule(const ModuleHandle& moduleHandle);

  Dimension* getDimension(const DimensionHandle& dimensionHandle);
  UpdateRate* getUpdateRate(const UpdateRateHandle& updateRateHandle);
  InteractionClass* getInteractionClass(const InteractionClassHandle& interactionClassHandle);
  InteractionClass::HandleMap& getInteractionClassHandleInteractionClassMap()
  { return _interactionClassHandleInteractionClassMap; }
  InteractionClassHandle getInteractionClassHandle(const std::string& fqInteractionClassName);

  ObjectClass* getObjectClass(const ObjectClassHandle& objectClassHandle);
  ObjectClass::HandleMap& getObjectClassHandleObjectClassMap()
  { return _objectClassHandleObjectClassMap; }
  ObjectClassHandle getObjectClassHandle(const std::string& fqClassName);

  OrderType resolveOrderType(const std::string& orderType);
  TransportationType resolveTransportationType(const std::string& transportationType);

  Dimension* resolveDimension(const std::string& dimensionName);
  UpdateRate* resolveUpdateRate(const std::string& updateRateName);
  InteractionClass* resolveParentInteractionClass(const StringVector& interactionClassName);
  ObjectClass* resolveParentObjectClass(const StringVector& objectClassName);

  /// Access and modify Federates in this Federation
  bool isFederateNameInUse(const std::string& name) const;

  void insert(Federate& federate);
  void erase(const FederateHandle& federateHandle);
  void erase(Federate& federate);
  Federate::HandleMap& getFederateHandleFederateMap()
  { return _federateHandleFederateMap; }

  Federate* getFederate(const FederateHandle& federateHandle);

  void insertTimeRegulating(Federate& federate);
  void eraseTimeRegulating(Federate& federate);
  FederationConnect::TimeRegulatingList& getTimeRegulatingFederationConnectList()
  { return _timeRegulatingFederationConnectList; }

  void insertTimeConstrained(Federate& federate);
  void eraseTimeConstrained(Federate& federate);
  FederationConnect::TimeConstrainedList& getTimeConstrainedFederationConnectList()
  { return _timeConstrainedFederationConnectList; }

  /// FIXME simplify region communication, only commit and erase is needed.
  Region* getOrCreateRegion(const RegionHandle& regionHandle);
  Region* getRegion(const RegionHandle& regionHandle);

  void insert(ObjectInstance& objectInstance);
  const ObjectInstance* getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle) const;
  ObjectInstance* getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle);
  void erase(ObjectInstance& objectInstance);
  ObjectInstance::HandleMap& getObjectInstanceHandleObjectInstanceMap()
  { return _objectInstanceHandleObjectInstanceMap; }
  bool isObjectInstanceNameInUse(const std::string& name) const;
  /// FIXME
  ObjectInstance* insertObjectInstance(const ObjectInstanceHandle& objectInstanceHandle, const std::string& objectInstanceName);

  // when the federateHandle is valid, it's assumed to run in a root server
  // otherwise it's a leaf server
  void initializeMom(AbstractServer* server, FederateHandle federateHandle);
  void resignMomServer();
  SharedPtr<MomServer> getMomServer() const;

  // federate metrics stuff
  std::shared_ptr<AbstractFederateMetrics> getFederateMetrics(const ConnectHandle& connectHandle);
  void interactionSent(const ConnectHandle& connectHandle, const InteractionClass* interactionClass);
  void interactionReceived(const ConnectHandle& connectHandle, const InteractionClass* interactionClass);
  void interactionClassSubscribed(const ConnectHandle& connectHandle, const InteractionClass* interactionClass, bool active);
  void interactionClassUnsubscribed(const ConnectHandle& connectHandle, const InteractionClass* interactionClass);
  void interactionClassPublished(const ConnectHandle& connectHandle, const InteractionClass* interactionClass);
  void interactionClassUnpublished(const ConnectHandle& connectHandle, const InteractionClass* interactionClass);
  void objectClassSubscribed(const ConnectHandle& connectHandle, const ObjectClass* objectClass, const AttributeHandleVector& attributes, bool active);
  void objectClassUnsubscribed(const ConnectHandle& connectHandle, const ObjectClass* objectClass, const AttributeHandleVector& attributes);
  void objectClassPublished(const ConnectHandle& connectHandle, const ObjectClass* objectClass, const AttributeHandleVector& attributes);
  void objectClassUnpublished(const ConnectHandle& connectHandle, const ObjectClass* objectClass, const AttributeHandleVector& attributes);
  void objectInstanceReflectionReceived(const ConnectHandle& connectHandle, const ObjectInstance* objectInstance);
  void objectInstanceUpdateSent(const ConnectHandle& connectHandle, const ObjectInstance* objectInstance);
  /// Synchronization state FIXME
  Synchronization::NameMap _synchronizationNameSynchronizationMap;

private:
  Federation(const Federation&) = delete;
  Federation& operator=(const Federation&) = delete;

  /// The server node this Federation belongs to
  Node& _serverNode;
  /// The MOM server of this federation
  SharedPtr<MomServer> _momServer;

  /// The name of the logical time factory
  std::string _logicalTimeFactoryName;

  // For almost everything a list would have been sufficient.
  // But inserting a federate into a NodeConnect requires to know if the
  // federation is already known at this connect and we do this by asking
  // the federation for a valid connect for a given connect handle.
  // Here a fast index keyed by connect handle helps a lot, so we have a map here.
  FederationConnect::HandleMap _connectHandleFederationConnectMap;

  // Module/ObjectModel dependent
  Module::HandleMap _moduleHandleModuleMap;
  HandleAllocator<ModuleHandle> _moduleHandleAllocator;

  /// FIXME, take these to map any name type string combination to one of the two fixed backend types
  // OrderType::NameMap _orderTypeNameOrderTypeMap;
  // OrderType::HandleMap _orderTypeHandleOrderTypeMap;
  // HandleAllocator<OrderTypeHandle> _orderTypeHandleAllocator;

  /// FIXME, take these to map any name type string combination to one of the two fixed backend types
  // TransportationType::NameMap _transportationTypeNameTransportationTypeMap;
  // TransportationType::HandleMap _transportationTypeHandleTransportationTypeMap;
  // HandleAllocator<TransportationTypeHandle> _transportationTypeHandleAllocator;

  Dimension::NameMap _dimensionNameDimensionMap;
  Dimension::HandleMap _dimensionHandleDimensionMap;
  HandleAllocator<DimensionHandle> _dimensionHandleAllocator;

  UpdateRate::NameMap _updateRateNameUpdateRateMap;
  UpdateRate::HandleMap _updateRateHandleUpdateRateMap;
  HandleAllocator<UpdateRateHandle> _updateRateHandleAllocator;

  BasicDataType::NameMap               _basicDataTypeNameBasicDataTypeMap;
  BasicDataType::HandleMap             _basicDataTypeHandleBasicDataTypeMap;
  HandleAllocator<BasicDataTypeHandle> _basicDataTypeHandleAllocator;

  SimpleDataType::NameMap               _simpleDataTypeNameSimpleDataTypeMap;
  SimpleDataType::HandleMap             _simpleDataTypeHandleSimpleDataTypeMap;
  HandleAllocator<SimpleDataTypeHandle> _simpleDataTypeHandleAllocator;

  EnumeratedDataType::NameMap               _enumeratedDataTypeNameEnumeratedDataTypeMap;
  EnumeratedDataType::HandleMap             _enumeratedDataTypeHandleEnumeratedDataTypeMap;
  HandleAllocator<EnumeratedDataTypeHandle> _enumeratedDataTypeHandleAllocator;

  ArrayDataType::NameMap               _arrayDataTypeNameArrayDataTypeMap;
  ArrayDataType::HandleMap             _arrayDataTypeHandleArrayDataTypeMap;
  HandleAllocator<ArrayDataTypeHandle> _arrayDataTypeHandleAllocator;

  FixedRecordDataType::NameMap               _fixedRecordDataTypeNameFixedRecordDataTypeMap;
  FixedRecordDataType::HandleMap             _fixedRecordDataTypeHandleFixedRecordDataTypeMap;
  HandleAllocator<FixedRecordDataTypeHandle> _fixedRecordDataTypeHandleAllocator;

  VariantRecordDataType::NameMap               _variantRecordDataTypeNameVariantRecordDataTypeMap;
  VariantRecordDataType::HandleMap             _variantRecordDataTypeHandleVariantRecordDataTypeMap;
  HandleAllocator<VariantRecordDataTypeHandle> _variantRecordDataTypeHandleAllocator;

  InteractionClass::NameMap _interactionClassNameInteractionClassMap;
  InteractionClass::HandleMap _interactionClassHandleInteractionClassMap;
  HandleAllocator<InteractionClassHandle> _interactionClassHandleAllocator;

  HandleAllocator<ParameterHandle> _parameterHandleAllocator;

  ObjectClass::NameMap _objectClassNameObjectClassMap;
  ObjectClass::HandleMap _objectClassHandleObjectClassMap;
  HandleAllocator<ObjectClassHandle> _objectClassHandleAllocator;


  // More runtime dependent stuff
  Federate::NameMap _federateNameFederateMap;
  Federate::HandleMap _federateHandleFederateMap;
  HandleAllocator<FederateHandle> _federateHandleAllocator;

  // Links the FederationConnects that have time regulating federates
  FederationConnect::TimeRegulatingList _timeRegulatingFederationConnectList;
  FederationConnect::TimeConstrainedList _timeConstrainedFederationConnectList;

  ObjectInstance::HandleMap _objectInstanceHandleObjectInstanceMap;
  ObjectInstance::NameMap _objectInstanceNameObjectInstanceMap;
  HandleAllocator<ObjectInstanceHandle> _objectInstanceHandleAllocator;
};

////////////////////////////////////////////////////////////

class OPENRTI_LOCAL NodeConnect : public IntrusiveUnorderedMap<ConnectHandle, NodeConnect>::Hook {
public:
  typedef IntrusiveUnorderedMap<ConnectHandle, NodeConnect> HandleMap;

  NodeConnect();
  ~NodeConnect();

  /// The connect handle to identify this connect
  const ConnectHandle& getConnectHandle() const
  { return HandleMap::Hook::getKey(); }
  void setConnectHandle(const ConnectHandle& connectHandle);

  /// True if this is the parent connect
  bool getIsParentConnect() const
  { return _isParentConnect; }
  void setIsParentConnect(const bool& isParentConnect);

  /// The connects name - for debugging
  const std::string& getName() const
  { return _name; }
  void setName(const std::string& name);

  /// The message send callback
  const SharedPtr<AbstractMessageSender>& getMessageSender() const;
  void setMessageSender(const SharedPtr<AbstractMessageSender>& messageSender);

  /// The string map from the other side of the connect
  const StringStringListMap& getOptions() const;
  void setOptions(const StringStringListMap& options);

  uint32_t getVersion() const { return _version; }
  /// The federations using this connect
  FederationConnect::FirstList& getFederationConnectList()
  { return _federationConnectList; }
  // FederationConnect* insert(Federation& federation)
  // { return new FederationConnect(federation, *this); }
  void insert(FederationConnect& federationConnect)
  { _federationConnectList.push_back(federationConnect); }

  /// We can actually send messages through a connect
  void send(const SharedPtr<const AbstractMessage>& message);

private:
  NodeConnect(const NodeConnect&) = delete;
  NodeConnect& operator=(const NodeConnect&) = delete;

  bool _isParentConnect;

  std::string _name;
  uint32_t _version;
  SharedPtr<AbstractMessageSender> _messageSender;

  StringStringListMap _options;

  /// Federations using this connect
  FederationConnect::FirstList _federationConnectList;
};

////////////////////////////////////////////////////////////

class OPENRTI_LOCAL Node {
public:
  Node();
  ~Node();

  bool hasChildConnects() const;
  bool isRootServer() const;
  bool isIdle() const;
  bool isParentConnect(const ConnectHandle& connectHandle) const
  { OpenRTIAssert(connectHandle.valid()); return _parentConnectHandle == connectHandle; }
  const ConnectHandle& getParentConnectHandle() const
  { return _parentConnectHandle; }

  const NodeConnect* getNodeConnect(const ConnectHandle& connectHandle) const;
  NodeConnect* getNodeConnect(const ConnectHandle& connectHandle);
  NodeConnect* insertNodeConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& options);
  NodeConnect* insertParentNodeConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& options);
  void insert(NodeConnect& nodeConnect);
  void erase(const ConnectHandle& connectHandle);
  void erase(NodeConnect& nodeConnect);

  bool getFederationExecutionAlreadyExists(const std::string& federationName) const;
  void insert(Federation& federation);
  void erase(Federation& federation);
  void insertName(Federation& federation);
  void eraseName(Federation& federation);
  Federation::HandleMap& getFederationHandleFederationMap()
  { return _federationHandleFederationMap; }

  void send(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message);
  // send message to the parent node
  void sendToParent(const SharedPtr<const AbstractMessage>& message);
  // send message to all connected nodes
  void broadcast(const SharedPtr<const AbstractMessage>& message);
  // send message to all connected nodes *except* the given connectHandle
  void broadcast(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message);
  // send message to all connected nodes *except* the parent connect
  void broadcastToChildren(const SharedPtr<const AbstractMessage>& message);

  ServerOptions& getServerOptions()
  { return *_serverOptions; }
  const ServerOptions& getServerOptions() const
  { return *_serverOptions; }

  const std::string& getServerName() const
  { return _serverOptions->getServerName(); }
  const std::string& getServerPath() const
  { return _serverOptions->getServerPath(); }

protected:
  // Connects
  ConnectHandle _parentConnectHandle;
  NodeConnect::HandleMap _connectHandleNodeConnectMap;
  HandleAllocator<ConnectHandle> _connectHandleAllocator;

  // Federations and their accessors
  Federation::NameMap _federationNameFederationMap;
  Federation::HandleMap _federationHandleFederationMap;
  FederationHandleAllocator _federationHandleAllocator;

  // The ServerOptions for this server component
  SharedPtr<ServerOptions> _serverOptions;

private:
  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;
};

} // namespace ServerModel
} // namespace OpenRTI

#endif
