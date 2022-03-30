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

#include "DebugNew.h"
#include "ServerModel.h"

#include "ServerOptions.h"
#include <sstream>
#include <iomanip>
#include "MessageQueue.h"
#include "ServerNode.h"
#include "mom/MomServer.h"
#include "mom/MomFederateMetrics.h"
#include "mom/AbstractFederateMetrics.h"
#include "MessageEncodingRegistry.h"

namespace OpenRTI {
namespace ServerModel {

static const unsigned int kIndentSpaces = 2;

static inline std::string normalizeTransportationType(const TransportationType& value)
{
  switch (value)
  {
    case RELIABLE: return "HLAreliable";
    case BEST_EFFORT: return "HLAbestEffort";
    default: return "";
  }
}

static inline std::string normalizeOrderType(const OrderType& value)
{
  switch (value)
  {
    case RECEIVE: return "Receive";
    case TIMESTAMP: return "TimeStamp";
    default: return "";
  }
}

////////////////////////////////////////////////////////////

Region::Region()
{
}

Region::~Region()
{
}

void
Region::setRegionHandle(const LocalRegionHandle& regionHandle)
{
  HandleEntity<Region, LocalRegionHandle>::_setHandle(regionHandle);
}

////////////////////////////////////////////////////////////

InstanceAttribute::InstanceAttribute(ObjectInstance& objectInstance, ClassAttribute& classAttribute) :
  _objectInstance(objectInstance),
  _classAttribute(classAttribute)
{
  setAttributeHandle(_classAttribute.getAttributeHandle());
  /// FIXME
  _receivingConnects = _classAttribute._cumulativeSubscribedConnectHandleSet;
}

InstanceAttribute::~InstanceAttribute()
{
}

void
InstanceAttribute::setAttributeHandle(const AttributeHandle& attributeHandle)
{
  HandleEntity<InstanceAttribute, AttributeHandle>::_setHandle(attributeHandle);
}


void InstanceAttribute::setOwnerFederate(const FederateHandle& federateHandle)
{
  //DebugPrintf("%s: name=%s.%s FederateHandle(%d) => FederateHandle(%d)\n", __FUNCTION__, 
  //            _classAttribute.getObjectClass().getFQName().c_str(),
  //            _classAttribute.getAttributeDefinition().getName().c_str(), 
  //            _ownerFederate.getHandle(), federateHandle.getHandle());
  _ownerFederate = federateHandle;
}

void InstanceAttribute::removeConnect(const ConnectHandle& connectHandle)
{
  _receivingConnects.erase(connectHandle);
  if (_ownerConnectHandle == connectHandle)
    _ownerConnectHandle = ConnectHandle();
}

////////////////////////////////////////////////////////////

ObjectInstanceConnect::ObjectInstanceConnect(ObjectInstance& objectInstance, FederationConnect& federationConnect) :
  _objectInstance(objectInstance),
  _federationConnect(federationConnect)
{
  setConnectHandle(federationConnect.getConnectHandle());
}

ObjectInstanceConnect::~ObjectInstanceConnect()
{
}

void
ObjectInstanceConnect::setConnectHandle(const ConnectHandle& connectHandle)
{
  HandleMap::Hook::setKey(connectHandle);
}

////////////////////////////////////////////////////////////

ObjectInstance::ObjectInstance(Federation& federation) :
  _federation(federation),
  _objectClass(0)
{
}

ObjectInstance::~ObjectInstance()
{
  _connectHandleObjectInstanceConnectMap.clear();
  _attributeHandleInstanceAttributeMap.clear();
}

void
ObjectInstance::setObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle)
{
  HandleStringEntity<ObjectInstance, ObjectInstanceHandle>::_setHandle(objectInstanceHandle);
}

void
ObjectInstance::setName(const std::string& name)
{
  HandleStringEntity<ObjectInstance, ObjectInstanceHandle>::_setString(name);
}

/*
void
ObjectInstance::insert(InstanceAttribute& instanceAttribute)
{
  _attributeHandleInstanceAttributeMap.insert(instanceAttribute);
}
*/

InstanceAttribute*
ObjectInstance::getInstanceAttribute(const AttributeHandle& attributeHandle)
{
  InstanceAttribute::HandleMap::iterator i = _attributeHandleInstanceAttributeMap.find(attributeHandle);
  if (i == _attributeHandleInstanceAttributeMap.end())
    return 0;
  return i.get();
}

InstanceAttribute*
ObjectInstance::getPrivilegeToDeleteInstanceAttribute()
{
  // InstanceAttribute::HandleMap::iterator i = _attributeHandleInstanceAttributeMap.begin();
  InstanceAttribute::HandleMap::iterator i = _attributeHandleInstanceAttributeMap.find(AttributeHandle(0));
  if (i == _attributeHandleInstanceAttributeMap.end())
    return 0;
  OpenRTIAssert(i->getAttributeHandle() == AttributeHandle(0));
  return i.get();
}

void
ObjectInstance::reference(FederationConnect& federationConnect)
{
  // OpenRTIAssert(_connectHandleObjectInstanceConnectMap.find(federationConnect.getConnectHandle()) == _connectHandleObjectInstanceConnectMap.end());
  if (_connectHandleObjectInstanceConnectMap.find(federationConnect.getConnectHandle()) != _connectHandleObjectInstanceConnectMap.end())
    return;
  ObjectInstanceConnect* objectInstanceConnect;
  objectInstanceConnect = new ObjectInstanceConnect(*this, federationConnect);
  _connectHandleObjectInstanceConnectMap.insert(*objectInstanceConnect);
  federationConnect.insert(*objectInstanceConnect);
}

bool
ObjectInstance::unreference(const ConnectHandle& connectHandle)
{
  ObjectInstanceConnect::HandleMap::iterator i = _connectHandleObjectInstanceConnectMap.find(connectHandle);
  OpenRTIAssert(i != _connectHandleObjectInstanceConnectMap.end());
  _connectHandleObjectInstanceConnectMap.erase(i);
  return _connectHandleObjectInstanceConnectMap.empty();
}

void
ObjectInstance::removeConnect(const ConnectHandle& connectHandle)
{
  // FIXME make that an assert at some time
  // OpenRTIAssert(_connectHandleSet.find(connectHandle) == _connectHandleSet.end());
  for (InstanceAttribute::HandleMap::iterator i = _attributeHandleInstanceAttributeMap.begin(); i != _attributeHandleInstanceAttributeMap.end(); ++i) {
    i->removeConnect(connectHandle);
  }
}

void
ObjectInstance::setObjectClass(ObjectClass* objectClass)
{
  if (objectClass == _objectClass)
    return;
  OpenRTIAssert(!_objectClass);
  _objectClass = objectClass;
  _objectClass->insert(*this);

  ClassAttribute::HandleMap& attributeHandleClassAttributeMap = objectClass->getAttributeHandleClassAttributeMap();
  //size_t numSubscribers = getSubscribedConnectHandleSet().size();
  for (ClassAttribute& classAttribute : attributeHandleClassAttributeMap) {
    ServerModel::InstanceAttribute* instanceAttribute = new ServerModel::InstanceAttribute(*this, classAttribute);
    //insert(*instanceAttribute);
    _attributeHandleInstanceAttributeMap.insert(*instanceAttribute);
    for (auto subscriber : classAttribute._cumulativeSubscribedConnectHandleSet)
    {
      setSubscriptionType(subscriber, SubscribedActive);
    }
  }
}

////////////////////////////////////////////////////////////

SynchronizationFederate::SynchronizationFederate(Synchronization& synchronization, Federate& federate) :
  _synchronization(synchronization),
  _federate(federate),
  _successful(false)
{
}

SynchronizationFederate::~SynchronizationFederate()
{
}

void
SynchronizationFederate::setFederateHandle(const FederateHandle& federateHandle)
{
  HandleListEntity<SynchronizationFederate, FederateHandle>::_setHandle(federateHandle);
}

void
SynchronizationFederate::setSuccessful(bool successful)
{
  _successful = successful;
}

////////////////////////////////////////////////////////////

Synchronization::Synchronization()
{
}

Synchronization::~Synchronization()
{
  _achievedFederateSyncronizationMap.clear();
  _waitingFederateSyncronizationMap.clear();
}

void
Synchronization::setLabel(const std::string& label)
{
  NameMap::Hook::setKey(label);
}

void
Synchronization::setTag(const VariableLengthData& tag)
{
  _tag = tag;
}

void
Synchronization::setAddJoiningFederates(bool addJoiningFederates)
{
  _addJoiningFederates = addJoiningFederates;
}

bool
Synchronization::getIsWaitingFor(const FederateHandle& federateHandle)
{
  return _waitingFederateSyncronizationMap.find(federateHandle) != _waitingFederateSyncronizationMap.end();
}

void
Synchronization::insert(Federate& federate)
{
  OpenRTIAssert(_waitingFederateSyncronizationMap.find(federate.getFederateHandle()) == _waitingFederateSyncronizationMap.end());
  OpenRTIAssert(_achievedFederateSyncronizationMap.find(federate.getFederateHandle()) == _achievedFederateSyncronizationMap.end());
  if (federate.getResignPending())
    return;
  if (federate.getIsInternal())
  {
    //DebugPrintf("%s(label=%s): federate %s is internal\n", __FUNCTION__, getLabel().c_str(), federate.getName().c_str());
    return;
  }
  //else
  //{
  //  DebugPrintf("%s(label=%s): inserted federate %s\n", __FUNCTION__, getLabel().c_str(), federate.getName().c_str());
  //}
  SynchronizationFederate* synchronizationFederate = new SynchronizationFederate(*this, federate);
  synchronizationFederate->setFederateHandle(federate.getFederateHandle());
  federate.insert(*synchronizationFederate);
  _waitingFederateSyncronizationMap.insert(*synchronizationFederate);
}

void
Synchronization::achieved(const FederateHandle& federateHandle, bool successful)
{
  ServerModel::SynchronizationFederate::HandleMap::iterator i;
  i = _waitingFederateSyncronizationMap.find(federateHandle);
  if (i == _waitingFederateSyncronizationMap.end())
    return;
  i->setSuccessful(successful);
  // OpenRTIAssert(_achievedFederateSyncronizationMap.find(i->getFederateHandle()) == _achievedFederateSyncronizationMap.end());
  // Note that no matter where we are currently linked,
  // this removes the entry from one of the maps
  SynchronizationFederate::HandleMap::unlink(*i);
  _achievedFederateSyncronizationMap.insert(*i);
}

////////////////////////////////////////////////////////////

Federate::Federate(Federation& federation)
  : _federation(federation)
  , _resignAction(CANCEL_THEN_DELETE_THEN_DIVEST)
  , _resignPending(false)
  , _federationConnect(0)
  , _commitId()
  , _isInternal(false)
{
}

Federate::~Federate()
{
  _synchronizationFederateList.clear();
  _federationConnect = 0;
}

void
Federate::setName(const std::string& name)
{
  HandleStringEntity<Federate, FederateHandle>::_setString(name);
}

void
Federate::setFederateHandle(const FederateHandle& federateHandle)
{
  HandleStringEntity<Federate, FederateHandle>::_setHandle(federateHandle);
}

void
Federate::setFederateType(const std::string& federateType)
{
  _federateType = federateType;
}

void
Federate::setResignAction(ResignAction resignAction)
{
  _resignAction = resignAction;
}

void
Federate::insert(SynchronizationFederate& synchronizationFederate)
{
  _synchronizationFederateList.push_back(synchronizationFederate);
}

void
Federate::setResignPending(bool resignPending)
{
  _resignPending = resignPending;
}

Region*
Federate::getRegion(const LocalRegionHandle& regionHandle)
{
  Region::HandleMap::iterator i = _regionHandleRegionMap.find(regionHandle);
  if (i == _regionHandleRegionMap.end())
    return 0;
  return i.get();
}

bool
Federate::getIsTimeRegulating() const
{
  return TimeRegulatingList::Hook::is_linked();
}

bool
Federate::getIsTimeConstrained() const
{
  return TimeConstrainedList::Hook::is_linked();
}

void
Federate::setTimeAdvanceTimeStamp(const VariableLengthData& timeAdvanceTimeStamp)
{
  _timeAdvanceTimeStamp = timeAdvanceTimeStamp;
}

void
Federate::setNextMessageTimeStamp(const VariableLengthData& nextMessageTimeStamp)
{
  _nextMessageTimeStamp = nextMessageTimeStamp;
}

void
Federate::setCommitId(Unsigned commitId)
{
  _commitId = commitId;
}

ConnectHandle
Federate::getConnectHandle() const
{
  if (!_federationConnect)
    return ConnectHandle();
  return _federationConnect->getConnectHandle();
}

void
Federate::send(const SharedPtr<const AbstractMessage>& message)
{
  if (!_federationConnect)
    return;
  _federationConnect->send(message);
}

////////////////////////////////////////////////////////////

DimensionModule::DimensionModule(Dimension& dimension, Module& module) :
  _dimension(dimension),
  _module(module)
{
}

DimensionModule::~DimensionModule()
{
}

////////////////////////////////////////////////////////////

Dimension::Dimension(Federation& federation) :
  _federation(federation),
  _upperBound(~Unsigned(0))
{
}

Dimension::~Dimension()
{
  OpenRTIAssert(_dimensionModuleList.empty());
}

void
Dimension::setName(const std::string& name)
{
  ModuleEntity<Dimension, DimensionHandle>::_setString(name);
}

void
Dimension::setDimensionHandle(const DimensionHandle& dimensionHandle)
{
  ModuleEntity<Dimension, DimensionHandle>::_setHandle(dimensionHandle);
}

void
Dimension::setUpperBound(const Unsigned& upperBound)
{
  _upperBound = upperBound;
}

bool
Dimension::getIsReferencedByAnyModule() const
{
  return !_dimensionModuleList.empty();
}

////////////////////////////////////////////////////////////

UpdateRate::UpdateRate(Federation& federation) :
  _federation(federation),
  _rate(0)
{
}

UpdateRate::~UpdateRate()
{
  OpenRTIAssert(_updateRateModuleList.empty());
}

void
UpdateRate::setRate(const double& rate)
{
  _rate = rate;
}

////////////////////////////////////////////////////////////

InteractionClassModule::InteractionClassModule(InteractionClass& interactionClass, Module& module) :
  _interactionClass(interactionClass), _module(module)
{
}

InteractionClassModule::~InteractionClassModule()
{
}

////////////////////////////////////////////////////////////

ParameterDefinitionModule::ParameterDefinitionModule(InteractionClass& interactionClass, Module& module) :
  _interactionClass(interactionClass), _module(module)
{
}

ParameterDefinitionModule::~ParameterDefinitionModule()
{
}

////////////////////////////////////////////////////////////

ClassParameter::ClassParameter(InteractionClass& interactionClass, ParameterDefinition& parameterDefinition) :
  _interactionClass(interactionClass),
  _parameterDefinition(parameterDefinition)
{
}

ClassParameter::~ClassParameter()
{
}

void
ClassParameter::setParameterHandle(const ParameterHandle& parameterHandle)
{
  HandleListEntity<ClassParameter, ParameterHandle>::_setHandle(parameterHandle);
}

////////////////////////////////////////////////////////////

ParameterDefinition::ParameterDefinition(InteractionClass& interactionClass) :
  _interactionClass(interactionClass)
{
}

ParameterDefinition::~ParameterDefinition()
{
  _classParameterList.clear();
}

void
ParameterDefinition::setName(const std::string& name)
{
  HandleStringEntity<ParameterDefinition, ParameterHandle>::_setString(name);
}

void
ParameterDefinition::setParameterHandle(const ParameterHandle& parameterHandle)
{
  HandleStringEntity<ParameterDefinition, ParameterHandle>::_setHandle(parameterHandle);
}


void ParameterDefinition::writeCurrentFDD(std::ostream& out, unsigned int level) const
{
  std::string indent(level * kIndentSpaces, ' ');
  std::string indent1((level+1) * kIndentSpaces, ' ');
  out << indent << R"XML(<parameter>)XML" << std::endl;
  out << indent1 << R"XML(<name>)XML" << getName() << R"XML(</name>)XML" << std::endl;
  out << indent1 << R"XML(<dataType>)XML" << getDataType() << R"XML(</dataType>)XML" << std::endl;
  out << indent << R"XML(</parameter>)XML" << std::endl;
}

////////////////////////////////////////////////////////////

InteractionClass::InteractionClass(Federation& federation, InteractionClass* parentInteractionClass) :
  _federation(federation),
  _parentInteractionClass(parentInteractionClass),
  _orderType(),
  _transportationType()
{
  if (_parentInteractionClass) {
    _parentInteractionClass->_childInteractionClassList.push_back(*this);

    for (ClassParameter::HandleMap::iterator i = _parentInteractionClass->_parameterHandleClassParameterMap.begin();
         i != _parentInteractionClass->_parameterHandleClassParameterMap.end(); ++i) {
      insertClassParameterFor(i->getParameterDefinition());
    }
  }
}

InteractionClass::~InteractionClass()
{
  _parameterHandleClassParameterMap.clear();
  eraseParameterDefinitions();
  OpenRTIAssert(_parameterHandleParameterMap.empty());
  OpenRTIAssert(_parameterNameParameterMap.empty());

  OpenRTIAssert(_childInteractionClassList.empty());
  OpenRTIAssert(_interactionClassModuleList.empty());
  OpenRTIAssert(_parameterDefinitionModuleList.empty());
  _parentInteractionClass = 0;
}

std::string InteractionClass::getFQName() const
{
  // alternatively, we could go recursively through getParentObjectClass
  StringVector name = getName();
  std::string result;
  StringVector::iterator iter = name.begin();
  while (iter != name.end())
  {
    result += *iter;
    if (++iter != name.end())
      result += ".";
  }
  return result;
}

void
InteractionClass::setName(const StringVector& name)
{
  ModuleClassEntity<InteractionClass, InteractionClassHandle>::_setString(name);
}

void
InteractionClass::setInteractionClassHandle(const InteractionClassHandle& interactionClassHandle)
{
  ModuleClassEntity<InteractionClass, InteractionClassHandle>::_setHandle(interactionClassHandle);
}

void
InteractionClass::setOrderType(OrderType orderType)
{
  _orderType = orderType;
}

void
InteractionClass::setTransportationType(TransportationType transportationType)
{
  _transportationType = transportationType;
}

InteractionClass*
InteractionClass::getParentInteractionClass()
{
  return _parentInteractionClass;
}

InteractionClassHandle
InteractionClass::getParentInteractionClassHandle() const
{
  if (!_parentInteractionClass)
    return InteractionClassHandle();
  return _parentInteractionClass->getInteractionClassHandle();
}

bool
InteractionClass::getIsReferencedByAnyModule() const
{
  return !_interactionClassModuleList.empty() || !_parameterDefinitionModuleList.empty();
}

bool
InteractionClass::getAreParametersReferencedByAnyModule() const
{
  return !_parameterDefinitionModuleList.empty();
}

void
InteractionClass::eraseParameterDefinitions()
{
  _parameterNameParameterMap.clear();
  OpenRTIAssert(_parameterNameParameterMap.empty());
}

uint32_t
InteractionClass::getNumParameterDefinitions() const
{
  // FIXME O(N)
  return static_cast<uint32_t>(_parameterHandleParameterMap.size());
}

ParameterHandle
InteractionClass::getFirstUnusedParameterHandle()
{
  // FIXME this is O(N)
  uint32_t numParameters = 0;
  if (_parentInteractionClass)
    numParameters += _parentInteractionClass->getFirstUnusedParameterHandle().getHandle();
  numParameters += getNumParameterDefinitions();
  return ParameterHandle(numParameters);
}

void
InteractionClass::insert(ParameterDefinition& parameterDefinition)
{
  _parameterHandleParameterMap.insert(parameterDefinition);
  _parameterNameParameterMap.insert(parameterDefinition);
  insertClassParameterFor(parameterDefinition);
}

// Normalize the given parameterFilters to match the this->_parameterFilterKeyPrototype. For unspecified values wildcards (empty VLD) will be inserted.
// The result is split up to filterKeyVector (the parameter handles) and filterValueTuple.
void InteractionClass::NormalizeFilterValues(const ParameterValueVector& parameterFilters, ParameterHandleVector& filterKeyVector, VariableLengthDataTuple& filterValueTuple) const
{
  filterKeyVector = ParameterHandleVector(_parameterFilterKeyPrototype);
  filterValueTuple = VariableLengthDataTuple(filterKeyVector.size());
  int valueIndex = 0;
  for (auto& filterKey : filterKeyVector)
  {
    auto where = std::find_if(parameterFilters.begin(), parameterFilters.end(),
      [filterKey](ParameterValue parameterValue) { return parameterValue.getParameterHandle() == filterKey; });
    if (where == parameterFilters.end())
    {
      filterKey = ParameterHandle();
      filterValueTuple[valueIndex] = VariableLengthData();
    }
    else
    {
      filterValueTuple[valueIndex] = where->getValue();
    }
    valueIndex++;
  }
}

// Add the specified parameterFilters to the given VariableLengthDataTupleSet
bool InteractionClass::AddParameterFilterValues(VariableLengthDataTupleSet& filterValueTuples, const ParameterValueVector& parameterFilters)
{
  ParameterHandleVector filterKeyVector;
  VariableLengthDataTuple filterValueTuple; // (filterKeyVector.size());
  // update the _parameterFilterKeyPrototype, if necessary (new parameter handle specified in filter for the first time)
  for (auto& item : parameterFilters)
  {
    if (std::find(_parameterFilterKeyPrototype.begin(), _parameterFilterKeyPrototype.end(), item.getParameterHandle()) == _parameterFilterKeyPrototype.end())
    {
      _parameterFilterKeyPrototype.push_back(item.getParameterHandle());
    }
  }
  // bring the parameterFilters into a form matching _parameterFilterKeyPrototype. 
  NormalizeFilterValues(parameterFilters, filterKeyVector, filterValueTuple);
  if (!filterValueTuples.Contains(filterValueTuple))
  {
    filterValueTuples.Insert(filterValueTuple);
    return true;
  }
  else
  {
    return false;
  }
}

// Add the specified parameterFilters to the given VariableLengthDataTupleSet
bool InteractionClass::RemoveParameterFilterValues(VariableLengthDataTupleSet& filterValueTuples, const ParameterValueVector& parameterFilters)
{
  ParameterHandleVector filterKeyVector;
  VariableLengthDataTuple filterValueTuple; // (filterKeyVector.size());
  // update the _parameterFilterKeyPrototype, if necessary (new parameter handle specified in filter for the first time)
  for (auto& item : parameterFilters)
  {
    if (std::find(_parameterFilterKeyPrototype.begin(), _parameterFilterKeyPrototype.end(), item.getParameterHandle()) == _parameterFilterKeyPrototype.end())
    {
      _parameterFilterKeyPrototype.push_back(item.getParameterHandle());
    }
  }
  // bring the parameterFilters into a form matching _parameterFilterKeyPrototype. 
  NormalizeFilterValues(parameterFilters, filterKeyVector, filterValueTuple);
  if (filterValueTuples.Contains(filterValueTuple))
  {
    filterValueTuples.Erase(filterValueTuple);
    return true;
  }
  else
  {
    return false;
  }
}

bool InteractionClass::updateParameterFilterValues(const ConnectHandle& connectHandle, const ParameterValueVector& parameterFilters, bool remove)
{
  bool result = false;
  if (!connectHandle.valid())
  {
    // given list is empty and any filter value list exists: request to remove filter
    //_parameterFiltersByFederate.erase(where);
    DebugPrintf("%s(%s): invalid connect handle\n", __FUNCTION__, getFQName().c_str());
    return false;
  }
  auto where = _parameterFiltersByConnect.find(connectHandle);
  if (where == _parameterFiltersByConnect.end() && !remove)
  {
    // connect handle not yet in filter (and we're about to add filter): add new filter tuple set
    VariableLengthDataTupleSet newFilterValueTupleSet;
    // convert vector <ParameterHandle, Value> to map<ParameterHandle : list of Values>
    AddParameterFilterValues(newFilterValueTupleSet, parameterFilters);
    // now associate connect handle with ParameterFilterMap constructed above
    _parameterFiltersByConnect.insert(ParameterFilterMapByConnect::value_type(connectHandle, newFilterValueTupleSet));
    //DebugPrintf("%s(%s): added connect %s\n", __FUNCTION__, getFQName().c_str(), connectHandle.toString().c_str());
    result = true;
  }
  else
  {
    // filter already exists for given connect ...
    auto& existingFilterValues = where->second;
    // merge given filters into existing parameter filter map
    if (remove)
    {
      result = RemoveParameterFilterValues(existingFilterValues, parameterFilters);
      if (parameterFilters.empty())
      {
        _parameterFiltersByConnect.erase(connectHandle);
      }
    }
    else
    {
      result = AddParameterFilterValues(existingFilterValues, parameterFilters);
      for (auto& entry : _parameterFiltersByConnect)
      {
        if (entry.first != where->first)
        {
          entry.second.SetTupleSize(_parameterFilterKeyPrototype.size());
        }
      }
    }
  }
  return result;
}

bool InteractionClass::clearParameterFilters(const ConnectHandle& connectHandle)
{
  if (!connectHandle.valid())
  {
    // given list is empty and any filter value list exists: request to remove filter
    //_parameterFiltersByFederate.erase(where);
    DebugPrintf("%s(%s): invalid connect handle\n", __FUNCTION__, getFQName().c_str());
    return false;
  }
  auto where = _parameterFiltersByConnect.find(connectHandle);
  if (where != _parameterFiltersByConnect.end())
  {
    _parameterFiltersByConnect.erase(where);
    return true;
  }
  return false;
}

bool InteractionClass::hasFilterSubscriptions() const
{
  return !_parameterFiltersByConnect.empty();
}

bool InteractionClass::hasFilterSubscriptions(const ConnectHandle& connectHandle) const
{
  if (!connectHandle.valid())
  {
    // given list is empty and any filter value list exists: request to remove filter
    //_parameterFiltersByFederate.erase(where);
    DebugPrintf("%s(%s): invalid connect handle\n", __FUNCTION__, getFQName().c_str());
    return false;
  }
  auto where = _parameterFiltersByConnect.find(connectHandle);
  if (where != _parameterFiltersByConnect.end())
  {
    return true;
  }
  return false;
}

template<typename TInputIter>
std::string make_hex_string(TInputIter first, TInputIter last, bool use_uppercase = true, bool insert_spaces = false)
{
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    if (use_uppercase)
        ss << std::uppercase;
    while (first != last)
    {
        ss << std::setw(2) << static_cast<int>(*first++);
        if (insert_spaces && first != last)
            ss << " ";
    }
    return ss.str();
}

// In the parameter filter set defined in _parameterFiltersByConnect, test if all parameters from parameterValues match a given value.
// if no parameter filter is defined for connectHandle, return true (all parameters 'match').
// If there is no parameter filter specified for any of the parameters in parameterValues, also return true.
bool InteractionClass::isMatching(const ConnectHandle& connectHandle, const ParameterValueVector& parameterValues) const
{
  auto where = _parameterFiltersByConnect.find(connectHandle);
  if (where == _parameterFiltersByConnect.end())
  {
    //DebugPrintf("%s(name=%s): %s not found in filter list\n", __FUNCTION__, getFQName().c_str(), connectHandle.toString().c_str());
    return true;
  }
  // filter exists for given connect ...
  const VariableLengthDataTupleSet& parameterFilterMap = where->second;
  
  // go through received parameter value vector ...
  if (parameterFilterMap.empty())
  {
    //DebugPrintf("%s(name=%s): connect %s has no filter parameters\n", __FUNCTION__, getFQName().c_str(), connectHandle.toString().c_str());
    //Dump(__FUNCTION__, _parameterFiltersByConnect);
    return true;
  }

  ParameterHandleVector filterKeyVector;
  VariableLengthDataTuple filterValueTuple;

  NormalizeFilterValues(parameterValues, filterKeyVector, filterValueTuple);
  return parameterFilterMap.FindNormalized(filterValueTuple);
}

ParameterValueVector InteractionClass::getParameterFilterPrototype() const
{
  ParameterValueVector result(_parameterFilterKeyPrototype.size());
  for (size_t index=0;index<result.size();index++)
  {
    result[index].setParameterHandle(_parameterFilterKeyPrototype[index]);
  }
  return result;
}

std::list<ParameterValueVector> InteractionClass::getParameterFilters(const ConnectHandle& connectHandle)
{
  std::list<ParameterValueVector> result;
  auto where = _parameterFiltersByConnect.find(connectHandle);
  if (where != _parameterFiltersByConnect.end())
  {
    // filter exists for given connect ...
    const VariableLengthDataTupleSet& parameterFilterValues = where->second;
    // iterate whole set of tuples ...
    for (auto& parameterTuple : parameterFilterValues)
    {
      ParameterValueVector resultElement(getParameterFilterPrototype());
      auto resultElementIter = resultElement.begin();
      // fill ParameterValueVector with values from tuple ...
      for (auto& parameterValue : parameterTuple)
      {
        resultElementIter->setValue(parameterValue);
        resultElementIter++;
      }
      result.push_back(resultElement);
    }
  }
  return result;
}

std::string InteractionClass::DumpInteractionFilters()
{
  std::ostringstream out;
  for (auto& parameterFiltersByConnectEntry : _parameterFiltersByConnect)
  {
    out << parameterFiltersByConnectEntry.first << " : " << std::endl;
    out << parameterFiltersByConnectEntry.second << std::endl;
  }
  return out.str();
}

void InteractionClass::writeCurrentFDD(std::ostream& out, unsigned int level) const
{
  std::string indent(level * kIndentSpaces, ' ');
  std::string indent1((level+1) * kIndentSpaces, ' ');
  out << indent << R"XML(<interactionClass>)XML" << std::endl;
  out << indent1 << R"XML(<name>)XML" << getName().back() << R"XML(</name>)XML" << std::endl;
  out << indent1 << R"XML(<order>)XML" << normalizeOrderType(getOrderType()) << R"XML(</order>)XML" << std::endl;
  for (const ParameterDefinition& parameter : _parameterHandleParameterMap)
  {
    parameter.writeCurrentFDD(out, level + 1);
  }
  for (const InteractionClass& childClass : getChildInteractionClassList())
  {
    childClass.writeCurrentFDD(out, level + 1);
  }
  out << indent << R"XML(</interactionClass>)XML" << std::endl;
}

ParameterDefinition*
InteractionClass::getParameterDefinition(const std::string& name)
{
  ParameterDefinition::NameMap::iterator i = _parameterNameParameterMap.find(name);
  if (i == _parameterNameParameterMap.end())
    return 0;
  return i.get();
}

ParameterDefinition*
InteractionClass::getParameterDefinition(const ParameterHandle& parameterHandle)
{
  ParameterDefinition::HandleMap::iterator i = _parameterHandleParameterMap.find(parameterHandle);
  if (i == _parameterHandleParameterMap.end())
    return 0;
  return i.get();
}

ParameterDefinition*
InteractionClass::findParameterDefinition(const ParameterHandle& parameterHandle)
{
  ParameterDefinition* definition = nullptr;
  InteractionClass* interactionClass = this;
  definition = interactionClass->getParameterDefinition(parameterHandle);
  while (definition == nullptr && interactionClass->getParentInteractionClass() != nullptr)
  {
    interactionClass = interactionClass->getParentInteractionClass();
    definition = interactionClass->getParameterDefinition(parameterHandle);
  }
  return definition;
}

void
InteractionClass::insertClassParameterFor(ParameterDefinition& parameterDefinition)
{
  ClassParameter* classParameter = new ClassParameter(*this, parameterDefinition);
  classParameter->setParameterHandle(parameterDefinition.getParameterHandle());
  _parameterHandleClassParameterMap.insert(*classParameter);
  parameterDefinition.insert(*classParameter);

  for (ChildList::iterator i = _childInteractionClassList.begin(); i != _childInteractionClassList.end(); ++i)
    i->insertClassParameterFor(parameterDefinition);
}

ClassParameter*
InteractionClass::getClassParameter(const ParameterHandle& parameterHandle)
{
  ClassParameter::HandleMap::iterator i = _parameterHandleClassParameterMap.find(parameterHandle);
  if (i == _parameterHandleClassParameterMap.end())
    return 0;
  return i.get();
}

////////////////////////////////////////////////////////////

ObjectClassModule::ObjectClassModule(ObjectClass& objectClass, Module& module) :
  _objectClass(objectClass), _module(module)
{
}

ObjectClassModule::~ObjectClassModule()
{
}

////////////////////////////////////////////////////////////

AttributeDefinitionModule::AttributeDefinitionModule(ObjectClass& objectClass, Module& module) :
  _objectClass(objectClass), _module(module)
{
}

AttributeDefinitionModule::~AttributeDefinitionModule()
{
}

////////////////////////////////////////////////////////////

ClassAttribute::ClassAttribute(ObjectClass& objectClass, AttributeDefinition& attributeDefinition) :
  _objectClass(objectClass),
  _attributeDefinition(attributeDefinition)
{
}

ClassAttribute::~ClassAttribute()
{
}

void
ClassAttribute::setAttributeHandle(const AttributeHandle& attributeHandle)
{
  HandleListEntity<ClassAttribute, AttributeHandle>::_setHandle(attributeHandle);
}

////////////////////////////////////////////////////////////

AttributeDefinition::AttributeDefinition(ObjectClass& objectClass) :
  _objectClass(objectClass),
  _orderType(),
  _transportationType()
{
}

AttributeDefinition::~AttributeDefinition()
{
  _classAttributeList.clear();
}

void
AttributeDefinition::setName(const std::string& name)
{
  HandleStringEntity<AttributeDefinition, AttributeHandle>::_setString(name);
}

void
AttributeDefinition::setAttributeHandle(const AttributeHandle& attributeHandle)
{
  HandleStringEntity<AttributeDefinition, AttributeHandle>::_setHandle(attributeHandle);
}

void
AttributeDefinition::setOrderType(OrderType orderType)
{
  _orderType = orderType;
}

void
AttributeDefinition::setTransportationType(TransportationType transportationType)
{
  _transportationType = transportationType;
}

void AttributeDefinition::writeCurrentFDD(std::ostream& out, unsigned int level) const
{
  std::string indent(level * kIndentSpaces, ' ');
  std::string indent1((level+1) * kIndentSpaces, ' ');
  out << indent << R"XML(<attribute>)XML" << std::endl;
  out << indent1 << R"XML(<name>)XML" << getName() << R"XML(</name>)XML" << std::endl;
  out << indent1 << R"XML(<dataType>)XML" << getDataType() << R"XML(</dataType>)XML" << std::endl;
  out << indent1 << R"XML(<order>)XML" << normalizeOrderType(getOrderType()) << R"XML(</order>)XML" << std::endl;
  out << indent1 << R"XML(<transportation>)XML" << normalizeTransportationType(getTransportationType()) << R"XML(</transportation>)XML" << std::endl;
            //<updateType>Static</updateType>
            //<updateCondition>NA</updateCondition>
            //<ownership>DivestAcquire</ownership>
            //<sharing>PublishSubscribe</sharing>
            //<transportation>HLAreliable</transportation>
            //<order>TimeStamp</order>
  out << indent << R"XML(</attribute>)XML" << std::endl;
}

////////////////////////////////////////////////////////////

ObjectClass::ObjectClass(Federation& federation, ObjectClass* parentObjectClass) :
  _federation(federation),
  _parentObjectClass(parentObjectClass)
{
  if (_parentObjectClass) {
    _parentObjectClass->_childObjectClassList.push_back(*this);

    for (ClassAttribute::HandleMap::iterator i = _parentObjectClass->_attributeHandleClassAttributeMap.begin();
         i != _parentObjectClass->_attributeHandleClassAttributeMap.end(); ++i) {
      insertClassAttributeFor(i->getAttributeDefinition());
    }
  }
}

ObjectClass::~ObjectClass()
{
  _objectInstanceList.unlink();
  _attributeHandleClassAttributeMap.clear();
  eraseAttributeDefinitions();
  OpenRTIAssert(_childObjectClassList.empty());
  OpenRTIAssert(_objectClassModuleList.empty());
  OpenRTIAssert(_attributeDefinitionModuleList.empty());
  _parentObjectClass = 0;
}

void
ObjectClass::setName(const StringVector& name)
{
  ModuleClassEntity<ObjectClass, ObjectClassHandle>::_setString(name);
}

std::string ObjectClass::getFQName() const
{
  // alternatively, we could go recursively through getParentObjectClass
  StringVector name = getName();
  std::string result;
  StringVector::iterator iter = name.begin();
  while (iter != name.end())
  {
    result += *iter;
    if (++iter != name.end())
      result += ".";
  }
  return result;
}

void
ObjectClass::setObjectClassHandle(const ObjectClassHandle& objectClassHandle)
{
  ModuleClassEntity<ObjectClass, ObjectClassHandle>::_setHandle(objectClassHandle);
}

ObjectClass*
ObjectClass::getParentObjectClass()
{
  return _parentObjectClass;
}

ObjectClassHandle
ObjectClass::getParentObjectClassHandle() const
{
  if (!_parentObjectClass)
    return ObjectClassHandle();
  return _parentObjectClass->getObjectClassHandle();
}

bool
ObjectClass::getIsReferencedByAnyModule() const
{
  return !_objectClassModuleList.empty() || !_attributeDefinitionModuleList.empty();
}

bool
ObjectClass::getAreAttributesReferencedByAnyModule() const
{
  return !_attributeDefinitionModuleList.empty();
}

void
ObjectClass::eraseAttributeDefinitions()
{
  _attributeHandleAttributeDefinitionMap.clear();
  OpenRTIAssert(_attributeNameAttributeDefinitionMap.empty());
}

uint32_t
ObjectClass::getNumAttributeDefinitions() const
{
  // FIXME O(N)
  return static_cast<uint32_t>(_attributeHandleAttributeDefinitionMap.size());
}

AttributeHandle
ObjectClass::getFirstUnusedAttributeHandle()
{
  // FIXME this is O(N)
  uint32_t numAttributes = 0;
  if (_parentObjectClass)
    numAttributes += _parentObjectClass->getFirstUnusedAttributeHandle().getHandle();
  numAttributes += getNumAttributeDefinitions();
  return AttributeHandle(numAttributes);
}

void
ObjectClass::insert(AttributeDefinition& attributeDefinition)
{
  _attributeHandleAttributeDefinitionMap.insert(attributeDefinition);
  _attributeNameAttributeDefinitionMap.insert(attributeDefinition);
  insertClassAttributeFor(attributeDefinition);
}

AttributeDefinition*
ObjectClass::getAttributeDefinition(const std::string& name)
{
  AttributeDefinition::NameMap::iterator i = _attributeNameAttributeDefinitionMap.find(name);
  if (i == _attributeNameAttributeDefinitionMap.end())
    return 0;
  return i.get();
}

AttributeDefinition*
ObjectClass::getAttributeDefinition(const AttributeHandle& attributeHandle)
{
  AttributeDefinition::HandleMap::iterator i = _attributeHandleAttributeDefinitionMap.find(attributeHandle);
  if (i == _attributeHandleAttributeDefinitionMap.end())
    return 0;
  return i.get();
}

void
ObjectClass::insertClassAttributeFor(AttributeDefinition& attributeDefinition)
{
  ClassAttribute* classAttribute = new ClassAttribute(*this, attributeDefinition);
  classAttribute->setAttributeHandle(attributeDefinition.getAttributeHandle());
  _attributeHandleClassAttributeMap.insert(*classAttribute);
  attributeDefinition.insert(*classAttribute);

  for (ChildList::iterator i = _childObjectClassList.begin(); i != _childObjectClassList.end(); ++i)
    i->insertClassAttributeFor(attributeDefinition);
}

ClassAttribute*
ObjectClass::getClassAttribute(const AttributeHandle& attributeHandle)
{
  ClassAttribute::HandleMap::iterator i = _attributeHandleClassAttributeMap.find(attributeHandle);
  if (i == _attributeHandleClassAttributeMap.end())
    return 0;
  return i.get();
}

ClassAttribute*
ObjectClass::getPrivilegeToDeleteClassAttribute()
{
  // ClassAttribute::HandleMap::iterator i = _attributeHandleClassAttributeMap.begin();
  ClassAttribute::HandleMap::iterator i = _attributeHandleClassAttributeMap.find(AttributeHandle(0));
  if (i == _attributeHandleClassAttributeMap.end())
    return 0;
  OpenRTIAssert(i->getAttributeHandle() == AttributeHandle(0));
  return i.get();
}

void
ObjectClass::insert(ObjectClassModule& objectClassModule)
{
  _objectClassModuleList.push_back(objectClassModule);
}

void
ObjectClass::insert(AttributeDefinitionModule& attributeDefinitionModule)
{
  _attributeDefinitionModuleList.push_back(attributeDefinitionModule);
}

void
ObjectClass::insert(ObjectInstance& objectInstance)
{
  _objectInstanceList.push_back(objectInstance);
}

void
ObjectClass::removeConnect(const ConnectHandle& connectHandle)
{
  for (ClassAttribute::HandleMap::iterator i = _attributeHandleClassAttributeMap.begin();
       i != _attributeHandleClassAttributeMap.end(); ++i)
    i->removeConnect(connectHandle);
}

// Update internal subscription list. Also considers parent classes.
// If a particular object instance's attributes have been subscribed for the first time, return this instance
// in objectInstanceList
void ObjectClass::updateCumulativeSubscription(const ConnectHandle& connectHandle,
                                               const AttributeHandle& attributeHandle,
                                               ObjectInstanceList& objectInstanceList)
{
  bool parentSubscribed = false;
  if (_parentObjectClass) {
    ClassAttribute* classAttribute = _parentObjectClass->getClassAttribute(attributeHandle);
    if (classAttribute)
      if (0 != classAttribute->_cumulativeSubscribedConnectHandleSet.count(connectHandle))
        parentSubscribed = true;
  }

  _updateCumulativeSubscription(connectHandle, attributeHandle, parentSubscribed, objectInstanceList);
}

void ObjectClass::_updateCumulativeSubscription(const ConnectHandle& connectHandle, const AttributeHandle& attributeHandle, bool subscribe /*Replace with regionset or something*/, ObjectInstanceList& objectInstanceList)
{
  // PRECOND: we already updated the subscription type in the class attribute
  ClassAttribute* classAttribute = getClassAttribute(attributeHandle);
  subscribe |= (Unsubscribed != classAttribute->getSubscriptionType(connectHandle));
  if (!classAttribute->updateCumulativeSubscribedConnectHandleSet(connectHandle, subscribe))
    return;
  // Update the receiving connect handle set
  for (ObjectClass& childObjectClass : _childObjectClassList) {
    childObjectClass._updateCumulativeSubscription(connectHandle, attributeHandle, subscribe, objectInstanceList);
  }
  /// FIXME: need to walk the objects and see how the routing for the object changes
  /// FIXME: store the object instances that are yet unknown to a connect and store these to propagate them into the connect
  /// Hmm, here is the first good use case for a visitor
  for (ObjectInstance::FirstList::iterator i = _objectInstanceList.begin(); i != _objectInstanceList.end(); ++i) {
    InstanceAttribute* instanceAttribute = i->getInstanceAttribute(attributeHandle);
    if (!instanceAttribute)
      continue;

    // Don't add the owner to the list of connect handles that receive this attribute
    if (instanceAttribute->getOwnerConnectHandle() == connectHandle)
      continue;

    if (subscribe) {

      // Insert the connect handle into the receiving connects
      if (!instanceAttribute->_receivingConnects.insert(connectHandle).second)
        continue;

      // Note that we need to insert this object instance into this connect
      if (attributeHandle == AttributeHandle(0))
        objectInstanceList.push_back(i.get());

    }
    else {
      // Never remove a attribute 0 subsciption as pushing the instance information may race
      if (attributeHandle == AttributeHandle(0))
        continue;

      // Erase the connect handle from the receiving connects
      if (instanceAttribute->_receivingConnects.erase(connectHandle) == 0)
        continue;
    }
  }
}


void ObjectClass::writeCurrentFDD(std::ostream& out, unsigned int level) const
{
  std::string indent(level * kIndentSpaces, ' ');
  std::string indent1((level+1) * kIndentSpaces, ' ');
  out << indent << R"XML(<objectClass>)XML" << std::endl;
  out << indent1 << R"XML(<name>)XML" << getName().back() << R"XML(</name>)XML" << std::endl;
  for (const AttributeDefinition& attribute : _attributeHandleAttributeDefinitionMap)
  {
    attribute.writeCurrentFDD(out, level + 1);
  }
  for (const ObjectClass& childClass : getChildObjectClassList())
  {
    childClass.writeCurrentFDD(out, level + 1);
  }
  out << indent << R"XML(</objectClass>)XML" << std::endl;
}

////////////////////////////////////////////////////////////

Module::Module(Federation& federation, const std::string& designator)
  : _federation(federation)
  , _designator(designator)
  , _artificialInteractionRoot(false)
  , _artificialObjectRoot(false)
{
}

Module::~Module()
{
  OpenRTIAssert(_dimensionModuleList.empty());
  OpenRTIAssert(_updateRateModuleList.empty());
  OpenRTIAssert(_interactionClassModuleList.empty());
  OpenRTIAssert(_parameterDefinitionModuleList.empty());
  OpenRTIAssert(_objectClassModuleList.empty());
  OpenRTIAssert(_attributeDefinitionModuleList.empty());
  OpenRTIAssert(_basicDataTypeModuleList.empty());
  OpenRTIAssert(_simpleDataTypeModuleList.empty());
  OpenRTIAssert(_enumeratedDataTypeModuleList.empty());
  OpenRTIAssert(_arrayDataTypeModuleList.empty());
  OpenRTIAssert(_fixedRecordDataTypeModuleList.empty());
  OpenRTIAssert(_variantRecordDataTypeModuleList.empty());
}

void
Module::setModuleHandle(const ModuleHandle& moduleHandle)
{
  HandleEntity<Module, ModuleHandle>::_setHandle(moduleHandle);
}

void
Module::setContent(const std::string& content)
{
  _content = content;
}

void
Module::setArtificialInteractionRoot(bool artificialInteractionRoot)
{
  _artificialInteractionRoot = artificialInteractionRoot;
}

void Module::setArtificialObjectRoot(bool artificialObjectRoot)
{
  _artificialObjectRoot = artificialObjectRoot;
}

void
Module::getModule(FOMModule& module)
{
  module.setModuleHandle(getModuleHandle());
  /// FIXME
  // FOMSwitchList _switchList;
  module.setArtificialInteractionRoot(getArtificialInteractionRoot());
  module.setArtificialObjectRoot(getArtificialObjectRoot());
  module.setDesignator(getDesignator());

  module.getDimensionList().reserve(_dimensionModuleList.size());
  for (DimensionModule::FirstList::iterator i = _dimensionModuleList.begin();
       i != _dimensionModuleList.end(); ++i) {
    Dimension& dimension = i->getDimension();
    module.getDimensionList().push_back(FOMDimension());
    FOMDimension& fomDimension = module.getDimensionList().back();
    fomDimension.setName(dimension.getName());
    fomDimension.setDimensionHandle(dimension.getDimensionHandle());
    fomDimension.setUpperBound(dimension.getUpperBound());
  }

  module.getUpdateRateList().reserve(_updateRateModuleList.size());
  for (UpdateRateModule::FirstList::iterator i = _updateRateModuleList.begin();
       i != _updateRateModuleList.end(); ++i) {
    UpdateRate& updateRate = i->getUpdateRate();
    module.getUpdateRateList().push_back(FOMUpdateRate());
    FOMUpdateRate& fomUpdateRate = module.getUpdateRateList().back();
    fomUpdateRate.setName(updateRate.getName());
    fomUpdateRate.setUpdateRateHandle(updateRate.getUpdateRateHandle());
    fomUpdateRate.setRate(updateRate.getRate());
  }

  /*
  module.getBasicDataTypeList().reserve(_basicDataTypeModuleList.size());
  for (auto& basicDataTypeModule : _basicDataTypeModuleList) {
    BasicDataType& basicDataType = basicDataTypeModule.getBasicDataType();
    module.getBasicDataTypeList().push_back(FOMBasicDataType());
    FOMBasicDataType& fomBasicDataType = module.getBasicDataTypeList().back();
    fomBasicDataType.setName(basicDataType.getName());
    fomBasicDataType.setHandle(basicDataType.getHandle());
  }
  */
  {
    module.getInteractionClassList().reserve(_interactionClassModuleList.size());
    ParameterDefinitionModule::FirstList::iterator j = _parameterDefinitionModuleList.begin();
    for (InteractionClassModule::FirstList::iterator i = _interactionClassModuleList.begin();
         i != _interactionClassModuleList.end(); ++i) {
      InteractionClass& interactionClass = i->getInteractionClass();
      module.getInteractionClassList().push_back(FOMInteractionClass());
      FOMInteractionClass& fomInteractionClass = module.getInteractionClassList().back();
      fomInteractionClass.setName(interactionClass.getName().back());
      fomInteractionClass.setInteractionClassHandle(interactionClass.getInteractionClassHandle());
      fomInteractionClass.setParentInteractionClassHandle(interactionClass.getParentInteractionClassHandle());
      fomInteractionClass.setOrderType(interactionClass.getOrderType());
      fomInteractionClass.setTransportationType(interactionClass.getTransportationType());
      fomInteractionClass.setDimensionHandleSet(interactionClass._dimensionHandleSet);

      // Check if we also reference the parameters with this module
      if (j == _parameterDefinitionModuleList.end())
        continue;
      if (interactionClass.getInteractionClassHandle() != j->getInteractionClass().getInteractionClassHandle())
        continue;
      // If so, add them too
      ++j;
      fomInteractionClass.getParameterList().reserve(interactionClass.getNumParameterDefinitions());
      for (const ParameterDefinition& parameter : interactionClass.getParameterHandleParameterMap()) {
        fomInteractionClass.getParameterList().push_back(FOMParameter());
        FOMParameter& fomParameter = fomInteractionClass.getParameterList().back();
        fomParameter.setName(parameter.getName());
        fomParameter.setDataType(parameter.getDataType());
        fomParameter.setParameterHandle(parameter.getParameterHandle());
      }
    }
  }

  {
    module.getObjectClassList().reserve(_objectClassModuleList.size());
    AttributeDefinitionModule::FirstList::iterator j = _attributeDefinitionModuleList.begin();
    for (ObjectClassModule::FirstList::iterator i = _objectClassModuleList.begin();
         i != _objectClassModuleList.end(); ++i) {
      ObjectClass& objectClass = i->getObjectClass();
      module.getObjectClassList().push_back(FOMObjectClass());
      FOMObjectClass& fomObjectClass = module.getObjectClassList().back();
      fomObjectClass.setName(objectClass.getName().back());
      fomObjectClass.setObjectClassHandle(objectClass.getObjectClassHandle());
      fomObjectClass.setParentObjectClassHandle(objectClass.getParentObjectClassHandle());

      // Check if we also reference the attributes with this module
      if (j == _attributeDefinitionModuleList.end())
        continue;
      if (objectClass.getObjectClassHandle() != j->getObjectClass().getObjectClassHandle())
        continue;
      // If so, add them too
      ++j;
      fomObjectClass.getAttributeList().reserve(objectClass.getNumAttributeDefinitions());
      for (AttributeDefinition::HandleMap::iterator k = objectClass.getAttributeHandleAttributeDefinitionMap().begin();
           k != objectClass.getAttributeHandleAttributeDefinitionMap().end(); ++k) {
        fomObjectClass.getAttributeList().push_back(FOMAttribute());
        FOMAttribute& fomAttribute = fomObjectClass.getAttributeList().back();
        fomAttribute.setName(k->getName());
        fomAttribute.setDataType(k->getDataType());
        fomAttribute.setAttributeHandle(k->getAttributeHandle());
        fomAttribute.setOrderType(k->getOrderType());
        fomAttribute.setTransportationType(k->getTransportationType());
        fomAttribute.setDimensionHandleSet(k->_dimensionHandleSet);
      }
    }
  }
}

DimensionModule*
Module::insert(Dimension& dimension)
{
  DimensionModule* dimensionModule;
  dimensionModule = new DimensionModule(dimension, *this);
  dimension.insert(*dimensionModule);
  insertModule(*dimensionModule);
  return dimensionModule;
}

UpdateRateModule*
Module::insert(UpdateRate& updateRate)
{
  UpdateRateModule* updateRateModule;
  updateRateModule = new UpdateRateModule(updateRate, *this);
  updateRate.insert(*updateRateModule);
  insertModule(*updateRateModule);
  return updateRateModule;
}

BasicDataTypeModule*
Module::insert(BasicDataType& dataType)
{
  BasicDataTypeModule* module = new BasicDataTypeModule(dataType, *this);
  dataType.insert(*module);
  insertModule(*module);
  return module;
}

SimpleDataTypeModule*
Module::insert(SimpleDataType& dataType)
{
  SimpleDataTypeModule* module = new SimpleDataTypeModule(dataType, *this);
  dataType.insert(*module);
  insertModule(*module);
  return module;
}

EnumeratedDataTypeModule*
Module::insert(EnumeratedDataType& dataType)
{
  EnumeratedDataTypeModule* module = new EnumeratedDataTypeModule(dataType, *this);
  dataType.insert(*module);
  insertModule(*module);
  return module;
}

ArrayDataTypeModule*
Module::insert(ArrayDataType& dataType)
{
  ArrayDataTypeModule* module = new ArrayDataTypeModule(dataType, *this);
  dataType.insert(*module);
  insertModule(*module);
  return module;
}

FixedRecordDataTypeModule*
Module::insert(FixedRecordDataType& dataType)
{
  FixedRecordDataTypeModule* module = new FixedRecordDataTypeModule(dataType, *this);
  dataType.insert(*module);
  insertModule(*module);
  return module;
}

VariantRecordDataTypeModule*
Module::insert(VariantRecordDataType& dataType)
{
  VariantRecordDataTypeModule* module = new VariantRecordDataTypeModule(dataType, *this);
  dataType.insert(*module);
  insertModule(*module);
  return module;
}

InteractionClassModule*
Module::insert(InteractionClass& interactionClass)
{
  InteractionClassModule* interactionClassModule;
  interactionClassModule = new InteractionClassModule(interactionClass, *this);
  interactionClass.insert(*interactionClassModule);
  insertModule(*interactionClassModule);
  return interactionClassModule;
}

ParameterDefinitionModule*
Module::insertParameters(InteractionClass& interactionClass)
{
  ParameterDefinitionModule* parameterDefinitionModule;
  parameterDefinitionModule = new ParameterDefinitionModule(interactionClass, *this);
  interactionClass.insert(*parameterDefinitionModule);
  insertModule(*parameterDefinitionModule);
  return parameterDefinitionModule;
}

ObjectClassModule*
Module::insert(ObjectClass& objectClass)
{
  ObjectClassModule* objectClassModule;
  objectClassModule = new ObjectClassModule(objectClass, *this);
  objectClass.insert(*objectClassModule);
  insertModule(*objectClassModule);
  return objectClassModule;
}

AttributeDefinitionModule*
Module::insertAttributes(ObjectClass& objectClass)
{
  AttributeDefinitionModule* attributeDefinitionModule;
  attributeDefinitionModule = new AttributeDefinitionModule(objectClass, *this);
  objectClass.insert(*attributeDefinitionModule);
  insertModule(*attributeDefinitionModule);
  return attributeDefinitionModule;
}

////////////////////////////////////////////////////////////

FederationConnect::FederationConnect(Federation& federation, NodeConnect& nodeConnect) :
  _federation(federation),
  _nodeConnect(nodeConnect),
  _active(false)
{
  HandleMap::Hook::setKey(nodeConnect.getConnectHandle());
}

FederationConnect::~FederationConnect()
{
  /// FIXME
  _objectInstanceConnectList.clear();
  _timeRegulatingFederateList.unlink();
  _timeConstrainedFederateList.unlink();
}

const FederationHandle&
FederationConnect::getFederationHandle() const
{
  return _federation.getFederationHandle();
}

const ConnectHandle&
FederationConnect::getConnectHandle() const
{
  // return _nodeConnect.getConnectHandle();
  return HandleMap::Hook::getKey();
}

bool
FederationConnect::getIsParentConnect() const
{
  return _nodeConnect.getIsParentConnect();
}

bool
FederationConnect::getHasFederates() const
{
  return !_federateList.empty();
}

bool
FederationConnect::getActive() const
{
  return _active;
}

void
FederationConnect::setActive(bool active)
{
  _active = active;
}

bool
FederationConnect::getIsTimeRegulating() const
{
  OpenRTIAssert(_timeRegulatingFederateList.empty() != TimeRegulatingList::Hook::is_linked());
  return TimeRegulatingList::Hook::is_linked();
}

void
FederationConnect::insertTimeRegulating(Federate& federate)
{
  _timeRegulatingFederateList.push_back(federate);
}

void
FederationConnect::eraseTimeRegulating(Federate& federate)
{
  _timeRegulatingFederateList.unlink(federate);
}

bool
FederationConnect::getIsTimeConstrained() const
{
  OpenRTIAssert(_timeConstrainedFederateList.empty() != TimeConstrainedList::Hook::is_linked());
  return TimeConstrainedList::Hook::is_linked();
}

void
FederationConnect::insertTimeConstrained(Federate& federate)
{
  _timeConstrainedFederateList.push_back(federate);
}

void
FederationConnect::eraseTimeConstrained(Federate& federate)
{
  _timeConstrainedFederateList.unlink(federate);
}

void
FederationConnect::send(const SharedPtr<const AbstractMessage>& message)
{
  if (!_active)
    return;
  _nodeConnect.send(message);
}

void
FederationConnect::sendAndDeactivate(const SharedPtr<const AbstractMessage>& message)
{
  if (!_active)
    return;
  _active = false;
  _nodeConnect.send(message);
}

////////////////////////////////////////////////////////////

Federation::Federation(Node& serverNode)
  : _serverNode(serverNode)
  , _momServer()
  , _objectInstanceHandleObjectInstanceMap(16384/*hash size*/)
{
}

Federation::~Federation()
{
  _momServer.reset();
  // In case of an exception tis can be non empty
  _objectInstanceHandleObjectInstanceMap.clear();

  // There is no need that this gets cleaned up. We can have open synchronizations ...
  // OpenRTIAssert(_synchronizationNameSynchronizationMap.empty());
  _synchronizationNameSynchronizationMap.clear();

  _federateHandleFederateMap.clear();
  OpenRTIAssert(_federateHandleFederateMap.empty());

  OpenRTIAssert(_timeRegulatingFederationConnectList.empty());
  OpenRTIAssert(_timeConstrainedFederationConnectList.empty());

  // Throw away the base modules
  while (!_moduleHandleModuleMap.empty()) {
    Federation::erase(_moduleHandleModuleMap.back());
  }

  OpenRTIAssert(!hasChildConnects());
  OpenRTIAssert(_connectHandleFederationConnectMap.size() <= 1);
  _connectHandleFederationConnectMap.clear();
  OpenRTIAssert(_moduleHandleModuleMap.empty());
  OpenRTIAssert(_dimensionNameDimensionMap.empty());
  OpenRTIAssert(_dimensionHandleDimensionMap.empty());
  OpenRTIAssert(_updateRateNameUpdateRateMap.empty());
  OpenRTIAssert(_updateRateHandleUpdateRateMap.empty());
  OpenRTIAssert(_interactionClassNameInteractionClassMap.empty());
  OpenRTIAssert(_interactionClassHandleInteractionClassMap.empty());
  OpenRTIAssert(_objectClassNameObjectClassMap.empty());
  OpenRTIAssert(_objectClassHandleObjectClassMap.empty());
}

const std::string&
Federation::getServerName() const
{
  return getServerNode().getServerName();
}

const std::string&
Federation::getServerPath() const
{
  return getServerNode().getServerPath();
}

bool
Federation::isRootServer() const
{
  return getServerNode().isRootServer();
}

bool
Federation::isParentConnect(const ConnectHandle& connectHandle) const
{
  return getServerNode().isParentConnect(connectHandle);
}

bool
Federation::hasChildConnects() const
{
  for (FederationConnect::HandleMap::const_reverse_iterator i = _connectHandleFederationConnectMap.rbegin();
       i != _connectHandleFederationConnectMap.rend(); ++i) {
    if (i->getIsParentConnect())
      continue;
    return true;
  }
  return false;
}

bool
Federation::hasChildConnect(const ConnectHandle& connectHandle)
{
  FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.find(connectHandle);
  if (i == _connectHandleFederationConnectMap.end())
    return false;
  return !i->getIsParentConnect();
}

bool
Federation::hasJoinedFederates() const
{
  OpenRTIAssert(_federateHandleFederateMap.empty() == (!_federateHandleAllocator.used()));
  return !_federateHandleFederateMap.empty();
}

bool
Federation::hasJoinedChildren()
{
  for (ServerModel::Federate::HandleMap::const_iterator i = getFederateHandleFederateMap().begin();
       i != getFederateHandleFederateMap().end(); ++i) {
    FederationConnect* federationConnect = i->getFederationConnect();
    if (!federationConnect)
      continue;
    if (federationConnect->getIsParentConnect())
      continue;
    // Ok, not even the still pending resign request for invalid connect handles is treated as valid child.
    if (i->getResignPending())
      continue;
    return true;
  }
  return false;
}

void
Federation::setName(const std::string& name)
{
  HandleStringEntity<Federation, FederationHandle>::_setString(name);
}

void
Federation::setFederationHandle(const FederationHandle& federationHandle)
{
  HandleStringEntity<Federation, FederationHandle>::_setHandle(federationHandle);
}

void
Federation::setLogicalTimeFactoryName(const std::string& logicalTimeFactoryName)
{
  _logicalTimeFactoryName = logicalTimeFactoryName;
}

FederationConnect*
Federation::getFederationConnect(const ConnectHandle& connectHandle)
{
  FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.find(connectHandle);
  if (i == _connectHandleFederationConnectMap.end())
    return 0;
  return i.get();
}

void
Federation::removeConnect(const ConnectHandle& connectHandle)
{
  for (ObjectInstance::HandleMap::iterator i = _objectInstanceHandleObjectInstanceMap.begin();
       i != _objectInstanceHandleObjectInstanceMap.end(); ++i) {
    i->removeConnect(connectHandle);
  }
  for (ObjectClass::HandleMap::iterator i = _objectClassHandleObjectClassMap.begin();
       i != _objectClassHandleObjectClassMap.end(); ++i) {
    i->removeConnect(connectHandle);
  }
  for (InteractionClass::HandleMap::iterator i = _interactionClassHandleInteractionClassMap.begin();
       i != _interactionClassHandleInteractionClassMap.end(); ++i) {
    i->removeConnect(connectHandle);
  }
}

void
Federation::send(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message)
{
  FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.find(connectHandle);
  if (i == _connectHandleFederationConnectMap.end())
    return;
  i->send(message);
}

void
Federation::send(const FederateHandle& federateHandle, const SharedPtr<const AbstractMessage>& message)
{
  Federate::HandleMap::iterator i = _federateHandleFederateMap.find(federateHandle);
  if (i == _federateHandleFederateMap.end())
    return;
  i->send(message);
}

void
Federation::sendToParent(const SharedPtr<const AbstractMessage>& message)
{
  _serverNode.sendToParent(message);
}

void
Federation::broadcast(const SharedPtr<const AbstractMessage>& message)
{
  for (FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.begin();
       i != _connectHandleFederationConnectMap.end(); ++i) {
    i->send(message);
  }
}

void
Federation::broadcast(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message)
{
  for (FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.begin();
       i != _connectHandleFederationConnectMap.end(); ++i) {
    if (i->getConnectHandle() == connectHandle)
      continue;
    i->send(message);
  }
}

void
Federation::broadcastToChildren(const SharedPtr<const AbstractMessage>& message)
{
  for (FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.begin();
       i != _connectHandleFederationConnectMap.end(); ++i) {
    if (i->getIsParentConnect())
      continue;
    i->send(message);
  }
}

void
Federation::broadcastToChildren(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message)
{
  for (FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.begin();
       i != _connectHandleFederationConnectMap.end(); ++i) {
    if (i->getIsParentConnect())
      continue;
    if (i->getConnectHandle() == connectHandle)
      continue;
    i->send(message);
  }
}

void
Federation::insert(Dimension& dimension)
{
  _dimensionNameDimensionMap.insert(dimension);
  _dimensionHandleDimensionMap.insert(dimension);
}

void
Federation::insert(UpdateRate& updateRate)
{
  _updateRateNameUpdateRateMap.insert(updateRate);
  _updateRateHandleUpdateRateMap.insert(updateRate);
}

void
Federation::insert(BasicDataType& dataType)
{
  _basicDataTypeNameBasicDataTypeMap.insert(dataType);
  _basicDataTypeHandleBasicDataTypeMap.insert(dataType);
}

void
Federation::insert(SimpleDataType& dataType)
{
  _simpleDataTypeNameSimpleDataTypeMap.insert(dataType);
  _simpleDataTypeHandleSimpleDataTypeMap.insert(dataType);
}

void
Federation::insert(EnumeratedDataType& dataType)
{
  _enumeratedDataTypeNameEnumeratedDataTypeMap.insert(dataType);
  _enumeratedDataTypeHandleEnumeratedDataTypeMap.insert(dataType);
}

void
Federation::insert(ArrayDataType& dataType)
{
  _arrayDataTypeNameArrayDataTypeMap.insert(dataType);
  _arrayDataTypeHandleArrayDataTypeMap.insert(dataType);
}

void
Federation::insert(FixedRecordDataType& dataType)
{
  _fixedRecordDataTypeNameFixedRecordDataTypeMap.insert(dataType);
  _fixedRecordDataTypeHandleFixedRecordDataTypeMap.insert(dataType);
}

void
Federation::insert(VariantRecordDataType& dataType)
{
  _variantRecordDataTypeNameVariantRecordDataTypeMap.insert(dataType);
  _variantRecordDataTypeHandleVariantRecordDataTypeMap.insert(dataType);
}

void
Federation::insert(InteractionClass& interactionClass)
{
  _interactionClassNameInteractionClassMap.insert(interactionClass);
  _interactionClassHandleInteractionClassMap.insert(interactionClass);
}

void
Federation::insert(ObjectClass& objectClass)
{
  _objectClassNameObjectClassMap.insert(objectClass);
  _objectClassHandleObjectClassMap.insert(objectClass);
}

void
Federation::insert(Module& module)
{
  OpenRTIAssert(_moduleHandleModuleMap.find(module.getModuleHandle()) == _moduleHandleModuleMap.end());
  _moduleHandleModuleMap.insert(module);
}


void Federation::insert(Module& /*module*/, const FOMTransportationType& /*fomObjectClass*/)
{

}

bool
Federation::insertOrCheck(Module& module, const FOMStringDimension& stringDimension)
{
  Dimension::NameMap::iterator i;
  i = _dimensionNameDimensionMap.find(stringDimension.getName());
  if (i != _dimensionNameDimensionMap.end()) {
    if (stringDimension.getUpperBound() != i->getUpperBound()) {
      std::stringstream ss;
      ss << "Inconsistent Dimension \"" << stringDimension.getName() << "\": Upper bound "
         << stringDimension.getUpperBound() << " does not match the already established value of "
         << i->getUpperBound() << "!";
      throw InconsistentFDD(ss.str());
    }
    module.insert(*i);
    return false;
  } else {
    Dimension* dimension = new Dimension(*this);
    dimension->setName(stringDimension.getName());
    dimension->setDimensionHandle(_dimensionHandleAllocator.get());
    dimension->setUpperBound(stringDimension.getUpperBound());
    insert(*dimension);
    module.insert(*dimension);
    return true;
  }
}

bool
Federation::insertOrCheck(Module& module, const FOMStringUpdateRate& stringUpdateRate)
{
  UpdateRate::NameMap::iterator i;
  i = _updateRateNameUpdateRateMap.find(stringUpdateRate.getName());
  if (i != _updateRateNameUpdateRateMap.end()) {
    if (stringUpdateRate.getRate() != i->getRate()) {
      std::stringstream ss;
      ss << "Inconsistent UpdateRate \"" << stringUpdateRate.getName() << "\": Rate "
         << stringUpdateRate.getRate() << " does not match the already established value of "
         << i->getRate() << "!";
      throw InconsistentFDD(ss.str());
    }
    module.insert(*i);
    return false;
  } else {
    UpdateRate* updateRate = new UpdateRate(*this);
    updateRate->setName(stringUpdateRate.getName());
    updateRate->setUpdateRateHandle(_updateRateHandleAllocator.get());
    updateRate->setRate(stringUpdateRate.getRate());
    insert(*updateRate);
    module.insert(*updateRate);
    return true;
  }
}

bool
Federation::insertOrCheck(Module& module, const FOMStringBasicDataType& stringDataType)
{
  BasicDataType::NameMap::iterator i;
  i = _basicDataTypeNameBasicDataTypeMap.find(stringDataType.getName());
  if (i != _basicDataTypeNameBasicDataTypeMap.end()) {
    module.insert(*i);
    return false;
  } else {
    BasicDataType* basicDataType = new BasicDataType(*this);
    basicDataType->setName(stringDataType.getName());
    basicDataType->setSize(stringDataType.getSize());
    basicDataType->setEndian(stringDataType.getEndian());
    basicDataType->setHandle(_basicDataTypeHandleAllocator.get());
    insert(*basicDataType);
    module.insert(*basicDataType);
    return true;
  }
}

bool
Federation::insertOrCheck(Module& module, const FOMStringSimpleDataType& stringDataType)
{
  SimpleDataType::NameMap::iterator i;
  i = _simpleDataTypeNameSimpleDataTypeMap.find(stringDataType.getName());
  if (i != _simpleDataTypeNameSimpleDataTypeMap.end()) {
    module.insert(*i);
    return false;
  } else {
    SimpleDataType* simpleDataType = new SimpleDataType(*this);
    simpleDataType->setName(stringDataType.getName());
    simpleDataType->setRepresentation(stringDataType.getRepresentation());
    simpleDataType->setHandle(_simpleDataTypeHandleAllocator.get());
    insert(*simpleDataType);
    module.insert(*simpleDataType);
    return true;
  }
}

bool Federation::insertOrCheck(Module& module, const FOMStringEnumeratedDataType& stringDataType)
{
  EnumeratedDataType::NameMap::iterator i;
  i = _enumeratedDataTypeNameEnumeratedDataTypeMap.find(stringDataType.getName());
  if (i != _enumeratedDataTypeNameEnumeratedDataTypeMap.end()) {
    module.insert(*i);
    return false;
  } else {
    EnumeratedDataType* enumeratedDataType = new EnumeratedDataType(*this);
    enumeratedDataType->setName(stringDataType.getName());
    enumeratedDataType->setRepresentation(stringDataType.getRepresentation());
    for (auto& enumerator : stringDataType.getEnumerators())
    {
      enumeratedDataType->addEnumerator(enumerator.getName(), enumerator.getValue());
    }
    enumeratedDataType->setHandle(_enumeratedDataTypeHandleAllocator.get());
    insert(*enumeratedDataType);
    module.insert(*enumeratedDataType);
    return true;
  }
}

//static inline ArrayDataTypeEncoding convert(const std::string& s)
//{
//  if (s == "HLAvariableArray") 
//    return VariableArrayDataTypeEncoding;
//  else
//    return FixedArrayDataTypeEncoding;
//}

bool Federation::insertOrCheck(Module& module, const FOMStringArrayDataType2& stringDataType)
{
  ArrayDataType::NameMap::iterator i;
  i = _arrayDataTypeNameArrayDataTypeMap.find(stringDataType.getName());
  if (i != _arrayDataTypeNameArrayDataTypeMap.end()) {
    module.insert(*i);
    return false;
  } else {
    ArrayDataType* arrayDataType = new ArrayDataType(*this);
    arrayDataType->setName(stringDataType.getName());
    arrayDataType->setEncoding(stringDataType.getEncoding());
    arrayDataType->setDataType(stringDataType.getDataType());
    arrayDataType->setCardinality(stringDataType.getCardinality());
    arrayDataType->setHandle(_arrayDataTypeHandleAllocator.get());
    insert(*arrayDataType);
    module.insert(*arrayDataType);
    return true;
  }
}

bool Federation::insertOrCheck(Module& module, const FOMStringFixedRecordDataType2& stringDataType)
{
  FixedRecordDataType::NameMap::iterator i;
  i = _fixedRecordDataTypeNameFixedRecordDataTypeMap.find(stringDataType.getName());
  if (i != _fixedRecordDataTypeNameFixedRecordDataTypeMap.end()) {
    module.insert(*i);
    return false;
  } else {
    FixedRecordDataType* fixedRecordDataType = new FixedRecordDataType(*this);
    fixedRecordDataType->setName(stringDataType.getName());
    for (auto& field : stringDataType.getFields())
    {
      fixedRecordDataType->addField(field.getName(), field.getDataType(), field.getVersion());
    }
    fixedRecordDataType->setEncoding(stringDataType.getEncoding());
    fixedRecordDataType->setInclude(stringDataType.getInclude());
    fixedRecordDataType->setVersion(stringDataType.getVersion());
    fixedRecordDataType->setHandle(_fixedRecordDataTypeHandleAllocator.get());
    insert(*fixedRecordDataType);
    module.insert(*fixedRecordDataType);
    return true;
  }
}

bool Federation::insertOrCheck(Module& module, const FOMStringVariantRecordDataType2& stringDataType)
{
  VariantRecordDataType::NameMap::iterator i;
  i = _variantRecordDataTypeNameVariantRecordDataTypeMap.find(stringDataType.getName());
  if (i != _variantRecordDataTypeNameVariantRecordDataTypeMap.end()) {
    module.insert(*i);
    return false;
  } else {
    VariantRecordDataType* variantRecordDataType = new VariantRecordDataType(*this);
    variantRecordDataType->setName(stringDataType.getName());
    variantRecordDataType->setHandle(_variantRecordDataTypeHandleAllocator.get());
    insert(*variantRecordDataType);
    module.insert(*variantRecordDataType);
    return true;
  }
}

bool
Federation::insertOrCheck(Module& module, const FOMStringInteractionClass& stringInteractionClass)
{

  InteractionClass::NameMap::iterator i;
  i = _interactionClassNameInteractionClassMap.find(stringInteractionClass.getName());
  if (i != _interactionClassNameInteractionClassMap.end()) {
    OpenRTIAssert(!i->getName().empty());
    bool created = false;

    if (i->getOrderType() != resolveOrderType(stringInteractionClass.getOrderType())) {
      std::stringstream ss;
      ss << "Inconsistent InteractionClass \"" << stringInteractionClass.getName() << "\": OrderType "
         << stringInteractionClass.getOrderType() << " does not match the already established value of "
         << i->getOrderType() << "!";
      throw InconsistentFDD(ss.str());
    }
    if (i->getTransportationType() != resolveTransportationType(stringInteractionClass.getTransportationType())) {
      std::stringstream ss;
      ss << "Inconsistent InteractionClass \"" << stringInteractionClass.getName() << "\": TransportationType "
         << stringInteractionClass.getTransportationType() << " does not match the already established value of "
         << i->getTransportationType() << "!";
      throw InconsistentFDD(ss.str());
    }

    DimensionHandleSet dimensionHandleSet;
    for (StringSet::const_iterator j = stringInteractionClass.getDimensionSet().begin();
         j != stringInteractionClass.getDimensionSet().end(); ++j) {
      Dimension* dimension = resolveDimension(*j);
      if (!dimension)
        throw InconsistentFDD("Cannot resolve dimension \"" + *j + "\"!");
      /// FIXME avoid using the handle sets ...
      dimensionHandleSet.insert(dimension->getDimensionHandle());
    }
    if (i->_dimensionHandleSet != dimensionHandleSet) {
      std::stringstream ss;
      ss << "Inconsistent InteractionClass \"" << stringInteractionClass.getName()
         << "\": Dimensions do not match the already established value!";
      throw InconsistentFDD(ss.str());
    }

    // In this case we need to check for the parameter list being the same on both ends.
    // i: Established IA Class
    // j: IA Class to check
    if (!stringInteractionClass.getParameterList().empty()) {
      std::vector<ParameterHandle> parameterHandles;
      parameterHandles.reserve(stringInteractionClass.getParameterList().size());

      for (FOMStringParameterList::const_iterator j = stringInteractionClass.getParameterList().begin();
            j != stringInteractionClass.getParameterList().end(); ++j) {

        ParameterDefinition* parameterDefinition;
        parameterDefinition = i->getParameterDefinition(j->getName());

        if (!parameterDefinition) 
        {
          ParameterDefinition* addParameterDefinition;
          InteractionClass* interactionClass = getInteractionClass(i->getInteractionClassHandle());
          addParameterDefinition = new ParameterDefinition(*interactionClass);
          addParameterDefinition->setName(j->getName());
          addParameterDefinition->setDataType(j->getDataType());
          addParameterDefinition->setParameterHandle(module.getFederation()._parameterHandleAllocator.get());
          i->insert(*addParameterDefinition);
          parameterHandles.push_back(addParameterDefinition->getParameterHandle());

          // This path so far only made consistency checks. We extended the IA class, so we have to return created = true.
          // Otherwise the changed module is discarded.
          created = true;

          DebugPrintf("%s:  Adding unknown parameter '%s' to established IA class '%s'\n", __FUNCTION__, j->getName().c_str(), interactionClass->getName().back().c_str());
        }
        else
          parameterHandles.push_back(parameterDefinition->getParameterHandle());
      }

      // Sort by handles...
      std::sort(parameterHandles.begin(), parameterHandles.end());
      // ... to check for duplicate handles
      if (std::unique(parameterHandles.begin(), parameterHandles.end()) != parameterHandles.end()) {
        std::stringstream ss;
        ss << "Inconsistent InteractionClass \"" << stringInteractionClass.getName()
           << "\": Parameters do not match the already established value!";
        throw InconsistentFDD(ss.str());
      }
    }

    module.insert(*i);
    if (!stringInteractionClass.getParameterList().empty())
      module.insertParameters(*i);

    return created;
  } else {
    InteractionClass* parentInteractionClass;
    parentInteractionClass = resolveParentInteractionClass(stringInteractionClass.getName());
    // This signals an error in message preparation
    if (!parentInteractionClass && 1 < stringInteractionClass.getName().size())
      throw MessageError("Cannot resolve parent interaction class name!");
    InteractionClass* interactionClass = new InteractionClass(*this, parentInteractionClass);
    interactionClass->setName(stringInteractionClass.getName());
    interactionClass->setInteractionClassHandle(_interactionClassHandleAllocator.get());
    insert(*interactionClass);
    module.insert(*interactionClass);

    interactionClass->setOrderType(resolveOrderType(stringInteractionClass.getOrderType()));
    interactionClass->setTransportationType(resolveTransportationType(stringInteractionClass.getTransportationType()));
    for (StringSet::const_iterator dimSetIter = stringInteractionClass.getDimensionSet().begin();
         dimSetIter != stringInteractionClass.getDimensionSet().end(); ++dimSetIter) {
      Dimension* dimension = resolveDimension(*dimSetIter);
      if (!dimension)
        throw InconsistentFDD("Cannot resolve dimension \"" + *dimSetIter + "\"!");
      /// FIXME avoid using the handle sets ...
      interactionClass->_dimensionHandleSet.insert(dimension->getDimensionHandle());
    }

    if (!stringInteractionClass.getParameterList().empty())
      module.insertParameters(*interactionClass);

    for (const FOMStringParameter& stringParameter : stringInteractionClass.getParameterList()) {
      if (interactionClass->getParameterDefinition(stringParameter.getName())) {
        std::stringstream ss;
        ss << "Duplicate parameter name \"" << stringParameter.getName() << "\" in InteractionClass \""
           << interactionClass->getName() << "\"!";
        throw InconsistentFDD(ss.str());
      }

      ParameterDefinition* parameterDefinition;
      parameterDefinition = new ParameterDefinition(*interactionClass);
      parameterDefinition->setName(stringParameter.getName());
      parameterDefinition->setDataType(stringParameter.getDataType());
      parameterDefinition->setParameterHandle(module.getFederation()._parameterHandleAllocator.get());
      interactionClass->insert(*parameterDefinition);
    }

    return true;
  }
}

bool
Federation::insertOrCheck(Module& module, const FOMStringObjectClass& stringObjectClass)
{
  ObjectClass::NameMap::iterator i;
  i = _objectClassNameObjectClassMap.find(stringObjectClass.getName());
  if (i != _objectClassNameObjectClassMap.end()) {
    OpenRTIAssert(!i->getName().empty());
    bool created = false;

    // In this case we need to check for the attribute list being the same on both ends.
    if (!stringObjectClass.getAttributeList().empty()) {
      std::vector<AttributeHandle> attributeHandles;
      attributeHandles.reserve(stringObjectClass.getAttributeList().size());
      for (const FOMStringAttribute& stringAttribute : stringObjectClass.getAttributeList()) {
        AttributeDefinition* attributeDefinition;
        attributeDefinition = i->getAttributeDefinition(stringAttribute.getName());

        if (!attributeDefinition) 
        {
          AttributeDefinition* addAttributeDefinition;
          ObjectClass* objectClass = getObjectClass(i->getObjectClassHandle());
          addAttributeDefinition = new AttributeDefinition(*objectClass);
          addAttributeDefinition->setName(stringAttribute.getName());
          addAttributeDefinition->setDataType(stringAttribute.getDataType());
          addAttributeDefinition->setAttributeHandle(objectClass->getFirstUnusedAttributeHandle());
          i->insert(*addAttributeDefinition);

          addAttributeDefinition->setOrderType(resolveOrderType(stringAttribute.getOrderType()));
          addAttributeDefinition->setTransportationType(resolveTransportationType(stringAttribute.getTransportationType()));
          for (StringSet::const_iterator j = stringAttribute.getDimensionSet().begin();
            j != stringAttribute.getDimensionSet().end(); ++j) {
            Dimension* dimension = resolveDimension(*j);
            if (!dimension)
              throw InconsistentFDD("Cannot resolve dimension \"" + *j + "\"!");
            /// FIXME avoid using the handle sets ...
            addAttributeDefinition->_dimensionHandleSet.insert(dimension->getDimensionHandle());
          }

          attributeHandles.push_back(addAttributeDefinition->getAttributeHandle());

          //objectClass->writeCurrentFDD(std::cout, 0);

          // This path so far only made consistency checks. We extended the object class, so we have to return created = true.
          // Otherwise the changed module is discarded.
          created = true;

          DebugPrintf("%s:  Adding unknown attribute '%s' to established object class '%s'\n", __FUNCTION__, stringAttribute.getName().c_str(), objectClass->getName().back().c_str());
        }
        else
        {
          if (attributeDefinition->getOrderType() != resolveOrderType(stringAttribute.getOrderType())) {
            std::stringstream ss;
            ss << "Inconsistent ObjectClass  \"" << stringObjectClass.getName() << "\": Attribute \""
              << stringAttribute.getName() << "\": OrderType " << stringAttribute.getOrderType()
              << " does not match the already established value of "
              << attributeDefinition->getOrderType() << "!";
            throw InconsistentFDD(ss.str());
          }
          if (attributeDefinition->getTransportationType() != resolveTransportationType(stringAttribute.getTransportationType())) {
            std::stringstream ss;
            ss << "Inconsistent ObjectClass  \"" << stringObjectClass.getName() << "\": Attribute \""
              << stringAttribute.getName() << "\": TransportationType " << stringAttribute.getTransportationType()
              << " does not match the already established value of "
              << attributeDefinition->getTransportationType() << "!";
            throw InconsistentFDD(ss.str());
          }

          DimensionHandleSet dimensionHandleSet;
          for (StringSet::const_iterator k = stringAttribute.getDimensionSet().begin();
            k != stringAttribute.getDimensionSet().end(); ++k) {
            Dimension* dimension = resolveDimension(*k);
            if (!dimension)
              throw InconsistentFDD("Cannot resolve dimension \"" + *k + "\"!");
            /// FIXME avoid using the handle sets ...
            dimensionHandleSet.insert(dimension->getDimensionHandle());
          }
          if (attributeDefinition->_dimensionHandleSet != dimensionHandleSet) {
            std::stringstream ss;
            ss << "Inconsistent ObjectClass  \"" << stringObjectClass.getName() << "\": Attribute \""
              << stringAttribute.getName() << "\": Dimensions do not match the already established value!";
            throw InconsistentFDD(ss.str());
          }
          attributeHandles.push_back(attributeDefinition->getAttributeHandle());
        }
      }
      std::sort(attributeHandles.begin(), attributeHandles.end());
      if (std::unique(attributeHandles.begin(), attributeHandles.end()) != attributeHandles.end()) {
        std::stringstream ss;
        ss << "Inconsistent ObjectClass \"" << stringObjectClass.getName()
           << "\": Attributes do not match the already established value!";
        throw InconsistentFDD(ss.str());
      }
    }

    module.insert(*i);
    if (!stringObjectClass.getAttributeList().empty())
      module.insertAttributes(*i);

    return created;
  } else {
    ObjectClass* parentObjectClass;
    parentObjectClass = resolveParentObjectClass(stringObjectClass.getName());
    // This signals an error in message preparation
    if (!parentObjectClass && 1 < stringObjectClass.getName().size())
      throw MessageError("Cannot resolve parent object class name!");
    ObjectClass* objectClass = new ObjectClass(*this, parentObjectClass);
    objectClass->setName(stringObjectClass.getName());
    objectClass->setObjectClassHandle(_objectClassHandleAllocator.get());
    insert(*objectClass);
    module.insert(*objectClass);

    if (!stringObjectClass.getAttributeList().empty())
      module.insertAttributes(*objectClass);

    AttributeHandle nextAttributeHandle = objectClass->getFirstUnusedAttributeHandle();
    for (FOMStringAttributeList::const_iterator attrListIter = stringObjectClass.getAttributeList().begin();
         attrListIter != stringObjectClass.getAttributeList().end(); ++attrListIter) {
      if (objectClass->getAttributeDefinition(attrListIter->getName())) {
        std::stringstream ss;
        ss << "Duplicate attribute name \"" << attrListIter->getName() << "\" in ObjectClass \""
           << objectClass->getName() << "\"!";
        throw InconsistentFDD(ss.str());
      }

      AttributeDefinition* attributeDefinition;
      attributeDefinition = new AttributeDefinition(*objectClass);
      attributeDefinition->setName(attrListIter->getName());
      attributeDefinition->setDataType(attrListIter->getDataType());
      attributeDefinition->setAttributeHandle(nextAttributeHandle);
      objectClass->insert(*attributeDefinition);

      attributeDefinition->setOrderType(resolveOrderType(attrListIter->getOrderType()));
      attributeDefinition->setTransportationType(resolveTransportationType(attrListIter->getTransportationType()));
      for (StringSet::const_iterator j = attrListIter->getDimensionSet().begin();
           j != attrListIter->getDimensionSet().end(); ++j) {
        Dimension* dimension = resolveDimension(*j);
        if (!dimension)
          throw InconsistentFDD("Cannot resolve dimension \"" + *j + "\"!");
        /// FIXME avoid using the handle sets ...
        attributeDefinition->_dimensionHandleSet.insert(dimension->getDimensionHandle());
      }

      nextAttributeHandle = AttributeHandle(nextAttributeHandle.getHandle() + 1);
    }

    return true;
  }
}

/*
void
Federation::insert(ModuleHandleVector& moduleHandleVector, const FOMStringModuleList& stringModuleList)
{
  moduleHandleVector.reserve(stringModuleList.size());
  try {
    for (auto& module : stringModuleList) {
      ModuleHandle moduleHandle = insert(module);
      if (moduleHandle.valid())
        moduleHandleVector.push_back(moduleHandle);
    }
  } catch (...) {
    for (ModuleHandleVector::iterator i = moduleHandleVector.begin(); i != moduleHandleVector.end(); ++i)
      erase(*i);
    moduleHandleVector.clear();
    throw;
  }
}
*/
ModuleHandle
Federation::insert(const FOMStringModule2& stringModule)
{
  Module* module = new Module(*this, stringModule.getDesignator());
  module->setModuleHandle(_moduleHandleAllocator.get());
  module->setArtificialInteractionRoot(stringModule.getArtificialInteractionRoot());
  module->setArtificialObjectRoot(stringModule.getArtificialObjectRoot());
  insert(*module);

  bool created = false;
  try {

    for (auto& stringDimension : stringModule.getDimensionList()) {
      if (insertOrCheck(*module, stringDimension))
        created = true;
    }

    for (auto& stringUpdateRate : stringModule.getUpdateRateList()) {
      if (insertOrCheck(*module, stringUpdateRate))
        created = true;
    }

    for (auto& stringInteractionClassList : stringModule.getInteractionClassList()) {
      if (insertOrCheck(*module, stringInteractionClassList))
        created = true;
    }

    for (auto& stringObjectClass : stringModule.getObjectClassList()) {
      if (insertOrCheck(*module, stringObjectClass))
        created = true;
    }

    for (auto& basicDataType : stringModule.getBasicDataTypeList()) {
      if (insertOrCheck(*module, basicDataType))
        created = true;
    }

    for (auto& simpleDataType : stringModule.getSimpleDataTypeList()) {
      if (insertOrCheck(*module, simpleDataType))
        created = true;
    }

    for (auto& enumeratedDataType : stringModule.getEnumeratedDataTypeList()) {
      if (insertOrCheck(*module, enumeratedDataType))
        created = true;
    }

    for (auto& arrayDataType : stringModule.getArrayDataTypeList()) {
      if (insertOrCheck(*module, arrayDataType))
        created = true;
    }

    for (auto& fixedRecordDataType : stringModule.getFixedRecordDataTypeList()) {
      if (insertOrCheck(*module, fixedRecordDataType))
        created = true;
    }

    for (auto& variantRecordDataType : stringModule.getVariantRecordDataTypeList()) {
      if (insertOrCheck(*module, variantRecordDataType))
        created = true;
    }

    if (!created) {
      erase(*module);
      return ModuleHandle();
    }

    return module->getModuleHandle();
  } catch (...) {
    erase(*module);
    throw;
  }

  //UNREACHABLE: return ModuleHandle();
}

void
Federation::insert(const FOMStringModule2List& stringModuleList)
{
  ModuleHandleVector moduleHandleVector;
  insert(moduleHandleVector, stringModuleList);
}

void
Federation::insert(ModuleHandleVector& moduleHandleVector, const FOMStringModule2List& stringModuleList)
{
  moduleHandleVector.reserve(stringModuleList.size());
  try {
    for (auto& module : stringModuleList) {
      ModuleHandle moduleHandle = insert(module);
      if (moduleHandle.valid())
        moduleHandleVector.push_back(moduleHandle);
    }
  } catch (...) {
    for (ModuleHandleVector::iterator i = moduleHandleVector.begin(); i != moduleHandleVector.end(); ++i)
      erase(*i);
    moduleHandleVector.clear();
    throw;
  }
}

void
Federation::insert(Module& module, const FOMDimension& fomDimension)
{
  Dimension::HandleMap::iterator i = _dimensionHandleDimensionMap.find(fomDimension.getDimensionHandle());
  if (i != _dimensionHandleDimensionMap.end()) {
    if (fomDimension.getName() != i->getName())
      throw MessageError("Dimension name does not match.");
    if (fomDimension.getUpperBound() != i->getUpperBound())
      throw MessageError("Dimension upper bound does not match.");
    module.insert(*i);
  } else {
    _dimensionHandleAllocator.take(fomDimension.getDimensionHandle());
    Dimension* dimension = new Dimension(*this);
    dimension->setName(fomDimension.getName());
    dimension->setDimensionHandle(fomDimension.getDimensionHandle());
    dimension->setUpperBound(fomDimension.getUpperBound());
    insert(*dimension);
    module.insert(*dimension);
  }
}

void
Federation::insert(Module& module, const FOMUpdateRate& fomUpdateRate)
{
  UpdateRate::HandleMap::iterator i = _updateRateHandleUpdateRateMap.find(fomUpdateRate.getUpdateRateHandle());
  if (i != _updateRateHandleUpdateRateMap.end()) {
    if (fomUpdateRate.getName() != i->getName())
      throw MessageError("UpdateRate name does not match.");
    if (fomUpdateRate.getRate() != i->getRate())
      throw MessageError("UpdateRate rate does not match.");
    module.insert(*i);
  } else {
    _updateRateHandleAllocator.take(fomUpdateRate.getUpdateRateHandle());
    UpdateRate* updateRate = new UpdateRate(*this);
    updateRate->setName(fomUpdateRate.getName());
    updateRate->setUpdateRateHandle(fomUpdateRate.getUpdateRateHandle());
    updateRate->setRate(fomUpdateRate.getRate());
    insert(*updateRate);
    module.insert(*updateRate);
  }
}

void
Federation::insert(Module& module, const FOMBasicDataType& fomBasicDataType)
{
  BasicDataType::HandleMap::iterator i = _basicDataTypeHandleBasicDataTypeMap.find(fomBasicDataType.getHandle());
  if (i != _basicDataTypeHandleBasicDataTypeMap.end()) {
    if (fomBasicDataType.getName() != i->getName())
      throw MessageError("BasicDataType name does not match.");

    module.insert(*i);
  } else {
    _basicDataTypeHandleAllocator.take(fomBasicDataType.getHandle());
    BasicDataType* basicDataType = new BasicDataType(*this);
    basicDataType->setName(fomBasicDataType.getName());
    basicDataType->setHandle(fomBasicDataType.getHandle());
    insert(*basicDataType);
    module.insert(*basicDataType);
  }
}

void
Federation::insert(Module& module, const FOMSimpleDataType& fomSimpleDataType)
{
  SimpleDataType::HandleMap::iterator i = _simpleDataTypeHandleSimpleDataTypeMap.find(fomSimpleDataType.getHandle());
  if (i != _simpleDataTypeHandleSimpleDataTypeMap.end()) {
    if (fomSimpleDataType.getName() != i->getName())
      throw MessageError("SimpleDataType name does not match.");

    module.insert(*i);
  } else {
    _simpleDataTypeHandleAllocator.take(fomSimpleDataType.getHandle());
    SimpleDataType* simpleDataType = new SimpleDataType(*this);
    simpleDataType->setName(fomSimpleDataType.getName());
    simpleDataType->setHandle(fomSimpleDataType.getHandle());
    insert(*simpleDataType);
    module.insert(*simpleDataType);
  }
}

void
Federation::insert(Module& module, const FOMEnumeratedDataType& fomEnumeratedDataType)
{
  EnumeratedDataType::HandleMap::iterator i = _enumeratedDataTypeHandleEnumeratedDataTypeMap.find(fomEnumeratedDataType.getHandle());
  if (i != _enumeratedDataTypeHandleEnumeratedDataTypeMap.end()) {
    if (fomEnumeratedDataType.getName() != i->getName())
      throw MessageError("EnumeratedDataType name does not match.");

    module.insert(*i);
  } else {
    _enumeratedDataTypeHandleAllocator.take(fomEnumeratedDataType.getHandle());
    EnumeratedDataType* enumeratedDataType = new EnumeratedDataType(*this);
    enumeratedDataType->setName(fomEnumeratedDataType.getName());
    enumeratedDataType->setHandle(fomEnumeratedDataType.getHandle());
    insert(*enumeratedDataType);
    module.insert(*enumeratedDataType);
  }
}

void
Federation::insert(Module& module, const FOMArrayDataType& fomArrayDataType)
{
  ArrayDataType::HandleMap::iterator i = _arrayDataTypeHandleArrayDataTypeMap.find(fomArrayDataType.getHandle());
  if (i != _arrayDataTypeHandleArrayDataTypeMap.end()) {
    if (fomArrayDataType.getName() != i->getName())
      throw MessageError("ArrayDataType name does not match.");

    module.insert(*i);
  } else {
    _arrayDataTypeHandleAllocator.take(fomArrayDataType.getHandle());
    ArrayDataType* arrayDataType = new ArrayDataType(*this);
    arrayDataType->setName(fomArrayDataType.getName());
    arrayDataType->setHandle(fomArrayDataType.getHandle());
    insert(*arrayDataType);
    module.insert(*arrayDataType);
  }
}

void
Federation::insert(Module& module, const FOMFixedRecordDataType& fomFixedRecordDataType)
{
  FixedRecordDataType::HandleMap::iterator i = _fixedRecordDataTypeHandleFixedRecordDataTypeMap.find(fomFixedRecordDataType.getHandle());
  if (i != _fixedRecordDataTypeHandleFixedRecordDataTypeMap.end()) {
    if (fomFixedRecordDataType.getName() != i->getName())
      throw MessageError("FixedRecordDataType name does not match.");

    module.insert(*i);
  } else {
    _fixedRecordDataTypeHandleAllocator.take(fomFixedRecordDataType.getHandle());
    FixedRecordDataType* fixedRecordDataType = new FixedRecordDataType(*this);
    fixedRecordDataType->setName(fomFixedRecordDataType.getName());
    fixedRecordDataType->setHandle(fomFixedRecordDataType.getHandle());
    insert(*fixedRecordDataType);
    module.insert(*fixedRecordDataType);
  }
}

void
Federation::insert(Module& module, const FOMVariantRecordDataType& fomVariantRecordDataType)
{
  VariantRecordDataType::HandleMap::iterator i = _variantRecordDataTypeHandleVariantRecordDataTypeMap.find(fomVariantRecordDataType.getHandle());
  if (i != _variantRecordDataTypeHandleVariantRecordDataTypeMap.end()) {
    if (fomVariantRecordDataType.getName() != i->getName())
      throw MessageError("VariantRecordDataType name does not match.");

    module.insert(*i);
  } else {
    _variantRecordDataTypeHandleAllocator.take(fomVariantRecordDataType.getHandle());
    VariantRecordDataType* variantRecordDataType = new VariantRecordDataType(*this);
    variantRecordDataType->setName(fomVariantRecordDataType.getName());
    variantRecordDataType->setHandle(fomVariantRecordDataType.getHandle());
    insert(*variantRecordDataType);
    module.insert(*variantRecordDataType);
  }
}

void
Federation::insert(Module& module, const FOMInteractionClass& fomInteractionClass)
{
  InteractionClass::HandleMap::iterator i;
  i = _interactionClassHandleInteractionClassMap.find(fomInteractionClass.getInteractionClassHandle());
  if (i != _interactionClassHandleInteractionClassMap.end()) {
    OpenRTIAssert(!i->getName().empty());
    if (fomInteractionClass.getName() != i->getName().back())
      throw MessageError("InteractionClass name does not match.");
    if (fomInteractionClass.getParentInteractionClassHandle() != i->getParentInteractionClassHandle())
      throw MessageError("Parent InteractionClass does not match.");
    if (fomInteractionClass.getOrderType() != i->getOrderType())
      throw MessageError("InteractionClass order type does not match.");
    if (fomInteractionClass.getTransportationType() != i->getTransportationType())
      throw MessageError("InteractionClass transportation type does not match.");
    if (fomInteractionClass.getDimensionHandleSet() != i->_dimensionHandleSet)
      throw MessageError("InteractionClass dimension handle set does not match.");
    // In this case we want to check for the parameter list being the same on both ends.
    if (!fomInteractionClass.getParameterList().empty()) {
      module.insertParameters(*i);
      if (i->getParameterHandleParameterMap().empty()) {
        for (FOMParameterList::const_iterator j = fomInteractionClass.getParameterList().begin();
             j != fomInteractionClass.getParameterList().end(); ++j) {
          if (i->getParameterDefinition(j->getName()))
            throw MessageError("Duplicate InteractionClass parameter names.");
          if (i->getParameterDefinition(j->getParameterHandle()))
            throw MessageError("Duplicate InteractionClass parameter handles.");
          ParameterDefinition* parameterDefinition;
          parameterDefinition = new ParameterDefinition(*i);
          parameterDefinition->setName(j->getName());
          parameterDefinition->setDataType(j->getDataType());
          parameterDefinition->setParameterHandle(j->getParameterHandle());
          i->insert(*parameterDefinition);
        }
      } else {
        std::vector<ParameterHandle> parameterHandles;
        parameterHandles.reserve(fomInteractionClass.getParameterList().size());
        for (FOMParameterList::const_iterator j = fomInteractionClass.getParameterList().begin();
             j != fomInteractionClass.getParameterList().end(); ++j) {
          ParameterDefinition* parameterDefinition;
          parameterDefinition = i->getParameterDefinition(j->getParameterHandle());
          
          //if (!parameterDefinition)
          //  throw MessageError("InteractionClass parameter lists do not match.");
          if (!parameterDefinition)
          {
            ParameterDefinition* addParameterDefinition;
            InteractionClass* interactionClass = getInteractionClass(i->getInteractionClassHandle());
            addParameterDefinition = new ParameterDefinition(*interactionClass);
            addParameterDefinition->setName(j->getName());
            addParameterDefinition->setDataType(j->getDataType());
            addParameterDefinition->setParameterHandle(j->getParameterHandle());   
            i->insert(*addParameterDefinition);
            DebugPrintf("%s:  Adding unknown parameter '%s' to established IA class '%s'\n", __FUNCTION__, j->getName().c_str(), interactionClass->getName().back().c_str());
          }
          else if (parameterDefinition->getName() != j->getName())
            throw MessageError("InteractionClass parameter lists do not match.");

          parameterHandles.push_back(j->getParameterHandle());
        }
        std::sort(parameterHandles.begin(), parameterHandles.end());
        if (std::unique(parameterHandles.begin(), parameterHandles.end()) != parameterHandles.end())
          throw MessageError("InteractionClass parameter lists do not match.");
      }
    }
    module.insert(*i);
  } else {
    InteractionClass* parentInteractionClass;
    parentInteractionClass = getInteractionClass(fomInteractionClass.getParentInteractionClassHandle());
   if (fomInteractionClass.getParentInteractionClassHandle().valid() && !parentInteractionClass)
      throw MessageError("Invalid parent InteractionClass!");
    StringVector name;
    if (parentInteractionClass)
      name = parentInteractionClass->getName();
    name.push_back(fomInteractionClass.getName());
    _interactionClassHandleAllocator.take(fomInteractionClass.getInteractionClassHandle());
    InteractionClass* interactionClass = new InteractionClass(*this, parentInteractionClass);
    interactionClass->setName(name);
    interactionClass->setInteractionClassHandle(fomInteractionClass.getInteractionClassHandle());
    insert(*interactionClass);
    module.insert(*interactionClass);
    interactionClass->setOrderType(fomInteractionClass.getOrderType());
    interactionClass->setTransportationType(fomInteractionClass.getTransportationType());
    interactionClass->_dimensionHandleSet = fomInteractionClass.getDimensionHandleSet();
    if (!fomInteractionClass.getParameterList().empty())
      module.insertParameters(*interactionClass);
    for (FOMParameterList::const_iterator j = fomInteractionClass.getParameterList().begin();
         j != fomInteractionClass.getParameterList().end(); ++j) {
      if (interactionClass->getParameterDefinition(j->getName()))
        throw MessageError("Duplicate InteractionClass parameter names.");
      if (interactionClass->getParameterDefinition(j->getParameterHandle()))
        throw MessageError("Duplicate InteractionClass parameter handles.");
      ParameterDefinition* parameterDefinition;
      parameterDefinition = new ParameterDefinition(*interactionClass);
      parameterDefinition->setName(j->getName());
      parameterDefinition->setParameterHandle(j->getParameterHandle());
      parameterDefinition->setDataType(j->getDataType());
      interactionClass->insert(*parameterDefinition);
    }
  }
}

void
Federation::insert(Module& module, const FOMObjectClass& fomObjectClass)
{
  ObjectClass::HandleMap::iterator i;
  i = _objectClassHandleObjectClassMap.find(fomObjectClass.getObjectClassHandle());
  if (i != _objectClassHandleObjectClassMap.end()) {
    OpenRTIAssert(!i->getName().empty());
    if (fomObjectClass.getName() != i->getName().back())
      throw MessageError("ObjectClass name does not match.");
    if (fomObjectClass.getParentObjectClassHandle() != i->getParentObjectClassHandle())
      throw MessageError("Parent ObjectClass does not match.");
    // In this case we want to check for the attribute list being the same on both ends.
    if (!fomObjectClass.getAttributeList().empty()) {
      module.insertAttributes(*i);
      if (i->getAttributeHandleAttributeDefinitionMap().empty()) {
        for (FOMAttributeList::const_iterator j = fomObjectClass.getAttributeList().begin();
             j != fomObjectClass.getAttributeList().end(); ++j) {
          if (i->getAttributeDefinition(j->getName()))
            throw MessageError("Duplicate ObjectClass attribute names.");
          if (i->getAttributeDefinition(j->getAttributeHandle()))
            throw MessageError("Duplicate ObjectClass attribute handles.");
          AttributeDefinition* attributeDefinition;
          attributeDefinition = new AttributeDefinition(*i);
          attributeDefinition->setName(j->getName());
          attributeDefinition->setDataType(j->getDataType());
          attributeDefinition->setAttributeHandle(j->getAttributeHandle());
          i->insert(*attributeDefinition);
          attributeDefinition->setOrderType(j->getOrderType());
          attributeDefinition->setTransportationType(j->getTransportationType());
          attributeDefinition->_dimensionHandleSet = j->getDimensionHandleSet();
        }
      } else {
        std::vector<AttributeHandle> attributeHandles;
        attributeHandles.reserve(fomObjectClass.getAttributeList().size());
        for (FOMAttributeList::const_iterator j = fomObjectClass.getAttributeList().begin();
             j != fomObjectClass.getAttributeList().end(); ++j) {
          AttributeDefinition* attributeDefinition;
          attributeDefinition = i->getAttributeDefinition(j->getAttributeHandle());
          //if (!attributeDefinition)
          //  throw MessageError("ObjectClass attribute lists do not match.");
          if (!attributeDefinition)
          {
            AttributeDefinition* addAttributeDefinition;
            ObjectClass* objectClass = getObjectClass(i->getObjectClassHandle());
            addAttributeDefinition = new AttributeDefinition(*objectClass);
            addAttributeDefinition->setName(j->getName());
            addAttributeDefinition->setDataType(j->getDataType());
            addAttributeDefinition->setAttributeHandle(objectClass->getFirstUnusedAttributeHandle());
            i->insert(*addAttributeDefinition);
            addAttributeDefinition->setOrderType(j->getOrderType());
            addAttributeDefinition->setTransportationType(j->getTransportationType());
            addAttributeDefinition->_dimensionHandleSet = j->getDimensionHandleSet();
            DebugPrintf("%s:  Adding unknown attribute '%s' to established object class '%s'\n", __FUNCTION__, j->getName().c_str(), objectClass->getName().back().c_str());
          }
          else
          {
            if (attributeDefinition->getName() != j->getName())
              throw MessageError("ObjectClass attribute lists do not match.");
            if (attributeDefinition->getOrderType() != j->getOrderType())
              throw MessageError("ObjectClass order type does not match.");
            if (attributeDefinition->getTransportationType() != j->getTransportationType())
              throw MessageError("ObjectClass transportation type does not match.");
            if (attributeDefinition->_dimensionHandleSet != j->getDimensionHandleSet())
              throw MessageError("ObjectClass dimension handle set does not match.");
          }
          attributeHandles.push_back(j->getAttributeHandle());
        }
        std::sort(attributeHandles.begin(), attributeHandles.end());
        if (std::unique(attributeHandles.begin(), attributeHandles.end()) != attributeHandles.end())
          throw MessageError("ObjectClass attribute lists do not match.");
      }
    }
    module.insert(*i);
  } else {
    ObjectClass* parentObjectClass;
    parentObjectClass = getObjectClass(fomObjectClass.getParentObjectClassHandle());
    if (fomObjectClass.getParentObjectClassHandle().valid() && !parentObjectClass)
      throw MessageError("Invalid parent ObjectClass!");
    StringVector name;
    if (parentObjectClass)
      name = parentObjectClass->getName();
    name.push_back(fomObjectClass.getName());
    _objectClassHandleAllocator.take(fomObjectClass.getObjectClassHandle());
    ObjectClass* objectClass = new ObjectClass(*this, parentObjectClass);
    objectClass->setName(name);
    objectClass->setObjectClassHandle(fomObjectClass.getObjectClassHandle());
    insert(*objectClass);
    module.insert(*objectClass);
    if (!fomObjectClass.getAttributeList().empty())
      module.insertAttributes(*objectClass);
    for (FOMAttributeList::const_iterator j = fomObjectClass.getAttributeList().begin();
         j != fomObjectClass.getAttributeList().end(); ++j) {
      if (objectClass->getAttributeDefinition(j->getName()))
        throw MessageError("Duplicate ObjectClass attribute names.");
      if (objectClass->getAttributeDefinition(j->getAttributeHandle()))
        throw MessageError("Duplicate ObjectClass attribute handles.");
      AttributeDefinition* attributeDefinition;
      attributeDefinition = new AttributeDefinition(*objectClass);
      attributeDefinition->setName(j->getName());
      attributeDefinition->setDataType(j->getDataType());
      attributeDefinition->setAttributeHandle(j->getAttributeHandle());
      objectClass->insert(*attributeDefinition);
      attributeDefinition->setOrderType(j->getOrderType());
      attributeDefinition->setTransportationType(j->getTransportationType());
      attributeDefinition->_dimensionHandleSet = j->getDimensionHandleSet();
    }
  }
}

void
Federation::insert(const FOMModule& fomModule)
{
  Module::HandleMap::iterator i = _moduleHandleModuleMap.find(fomModule.getModuleHandle());
  if (i == _moduleHandleModuleMap.end()) {
    _moduleHandleAllocator.take(fomModule.getModuleHandle());
    Module* module = new Module(*this, fomModule.getDesignator());
    module->setModuleHandle(fomModule.getModuleHandle());
    module->setArtificialInteractionRoot(fomModule.getArtificialInteractionRoot());
    module->setArtificialObjectRoot(fomModule.getArtificialObjectRoot());
    insert(*module);

    for (auto& dimension : fomModule.getDimensionList()) {
      insert(*module, dimension);
    }

    for (auto& updateRate : fomModule.getUpdateRateList()) {
      insert(*module, updateRate);
    }

    for (auto& interactionClass : fomModule.getInteractionClassList()) {
      insert(*module, interactionClass);
    }

    for (auto& objectClass : fomModule.getObjectClassList()) {
      insert(*module, objectClass);
    }
    for (auto& transportationType : fomModule.getTransportationTypeList()) {
      insert(*module, transportationType);
    }
  }
}

void
Federation::insert(const FOMModule2& fomModule)
{
  Module::HandleMap::iterator i = _moduleHandleModuleMap.find(fomModule.getModuleHandle());
  if (i == _moduleHandleModuleMap.end()) {
    _moduleHandleAllocator.take(fomModule.getModuleHandle());
    Module* module = new Module(*this, fomModule.getDesignator());
    module->setModuleHandle(fomModule.getModuleHandle());
    module->setArtificialInteractionRoot(fomModule.getArtificialInteractionRoot());
    module->setArtificialObjectRoot(fomModule.getArtificialObjectRoot());
    insert(*module);

    for (auto& dimension : fomModule.getDimensionList()) {
      insert(*module, dimension);
    }

    for (auto& updateRate : fomModule.getUpdateRateList()) {
      insert(*module, updateRate);
    }

    for (auto& interactionClass : fomModule.getInteractionClassList()) {
      insert(*module, interactionClass);
    }

    for (auto& objectClass : fomModule.getObjectClassList()) {
      insert(*module, objectClass);
    }
    for (auto& transportationType : fomModule.getTransportationTypeList()) {
      insert(*module, transportationType);
    }
    for (auto& basicDataType : fomModule.getBasicDataTypeList()) {
      insert(*module, basicDataType);
    }
    for (auto& simpleDataType : fomModule.getSimpleDataTypeList()) {
      insert(*module, simpleDataType);
    }
    for (auto& enumeratedDataType : fomModule.getEnumeratedDataTypeList()) {
      insert(*module, enumeratedDataType);
    }
    for (auto& arrayDataType : fomModule.getArrayDataTypeList()) {
      insert(*module, arrayDataType);
    }
    for (auto& fixedRecordDataType : fomModule.getFixedRecordDataTypeList()) {
      insert(*module, fixedRecordDataType);
    }
    for (auto& variantRecordDataType : fomModule.getVariantRecordDataTypeList()) {
      insert(*module, variantRecordDataType);
    }
  }
}

void
Federation::insert(const FOMModuleList& fomModuleList)
{
  for (auto& module : fomModuleList) {
    insert(module);
  }
}

void
Federation::insert(const FOMModule2List& fomModuleList)
{
  for (auto& module : fomModuleList) {
    insert(module);
  }
}

void
Federation::erase(const ModuleHandle& moduleHandle)
{
  Module::HandleMap::iterator i = _moduleHandleModuleMap.find(moduleHandle);
  OpenRTIAssert(i != _moduleHandleModuleMap.end());
  erase(*i);
}

void
Federation::erase(Module& module)
{
  while (!module.getParameterDefinitionModuleList().empty()) {
    InteractionClass& interactionClass = module.getParameterDefinitionModuleList().back().getInteractionClass();
    module.getParameterDefinitionModuleList().pop_back();
    if (interactionClass.getAreParametersReferencedByAnyModule())
      continue;
    interactionClass.eraseParameterDefinitions();
  }
  while (!module.getInteractionClassModuleList().empty()) {
    InteractionClass& interactionClass = module.getInteractionClassModuleList().back().getInteractionClass();
    module.getInteractionClassModuleList().pop_back();
    if (interactionClass.getIsReferencedByAnyModule())
      continue;
    _interactionClassHandleAllocator.put(interactionClass.getInteractionClassHandle());
    InteractionClass::HandleMap::erase(interactionClass);
  }

  while (!module.getAttributeDefinitionModuleList().empty()) {
    ObjectClass& objectClass = module.getAttributeDefinitionModuleList().back().getObjectClass();
    module.getAttributeDefinitionModuleList().pop_back();
    if (objectClass.getAreAttributesReferencedByAnyModule())
      continue;
    objectClass.eraseAttributeDefinitions();
  }
  while (!module.getObjectClassModuleList().empty()) {
    ObjectClass& objectClass = module.getObjectClassModuleList().back().getObjectClass();
    module.getObjectClassModuleList().pop_back();
    if (objectClass.getIsReferencedByAnyModule())
      continue;
    _objectClassHandleAllocator.put(objectClass.getObjectClassHandle());
    ObjectClass::HandleMap::erase(objectClass);
  }

  while (!module.getUpdateRateModuleList().empty()) {
    UpdateRate& updateRate = module.getUpdateRateModuleList().back().getUpdateRate();
    module.getUpdateRateModuleList().pop_back();
    if (updateRate.getIsReferencedByAnyModule())
      continue;
    _updateRateHandleAllocator.put(updateRate.getUpdateRateHandle());
    UpdateRate::HandleMap::erase(updateRate);
  }

  while (!module.getDimensionModuleList().empty()) {
    Dimension& dimension = module.getDimensionModuleList().back().getDimension();
    module.getDimensionModuleList().pop_back();
    if (dimension.getIsReferencedByAnyModule())
      continue;
    _dimensionHandleAllocator.put(dimension.getDimensionHandle());
    Dimension::HandleMap::erase(dimension);
  }

  while (!module.getBasicDataTypeModuleList().empty()) {
    BasicDataType& dataType = module.getBasicDataTypeModuleList().back().getBasicDataType();
    module.getBasicDataTypeModuleList().pop_back();
    if (dataType.getIsReferencedByAnyModule())
      continue;
    _basicDataTypeHandleAllocator.put(dataType.getHandle());
    BasicDataType::HandleMap::erase(dataType);
  }

  while (!module.getSimpleDataTypeModuleList().empty()) {
    SimpleDataType& dataType = module.getSimpleDataTypeModuleList().back().getSimpleDataType();
    module.getSimpleDataTypeModuleList().pop_back();
    if (dataType.getIsReferencedByAnyModule())
      continue;
    _simpleDataTypeHandleAllocator.put(dataType.getHandle());
    SimpleDataType::HandleMap::erase(dataType);
  }

  while (!module.getEnumeratedDataTypeModuleList().empty()) {
    EnumeratedDataType& dataType = module.getEnumeratedDataTypeModuleList().back().getEnumeratedDataType();
    module.getEnumeratedDataTypeModuleList().pop_back();
    if (dataType.getIsReferencedByAnyModule())
      continue;
    _enumeratedDataTypeHandleAllocator.put(dataType.getHandle());
    EnumeratedDataType::HandleMap::erase(dataType);
  }

  while (!module.getArrayDataTypeModuleList().empty()) {
    ArrayDataType& dataType = module.getArrayDataTypeModuleList().back().getArrayDataType();
    module.getArrayDataTypeModuleList().pop_back();
    if (dataType.getIsReferencedByAnyModule())
      continue;
    _arrayDataTypeHandleAllocator.put(dataType.getHandle());
    ArrayDataType::HandleMap::erase(dataType);
  }

  while (!module.getFixedRecordDataTypeModuleList().empty()) {
    FixedRecordDataType& dataType = module.getFixedRecordDataTypeModuleList().back().getFixedRecordDataType();
    module.getFixedRecordDataTypeModuleList().pop_back();
    if (dataType.getIsReferencedByAnyModule())
      continue;
    _fixedRecordDataTypeHandleAllocator.put(dataType.getHandle());
    FixedRecordDataType::HandleMap::erase(dataType);
  }

  while (!module.getVariantRecordDataTypeModuleList().empty()) {
    VariantRecordDataType& dataType = module.getVariantRecordDataTypeModuleList().back().getVariantRecordDataType();
    module.getVariantRecordDataTypeModuleList().pop_back();
    if (dataType.getIsReferencedByAnyModule())
      continue;
    _variantRecordDataTypeHandleAllocator.put(dataType.getHandle());
    VariantRecordDataType::HandleMap::erase(dataType);
  }

  _moduleHandleAllocator.put(module.getModuleHandle());
  Module::HandleMap::erase(module);
}

void
Federation::getModuleList(FOMModuleList& moduleList) const
{
  moduleList.reserve(_moduleHandleModuleMap.size());
  for (Module::HandleMap::iterator i = _moduleHandleModuleMap.begin();
       i != _moduleHandleModuleMap.end(); ++i) {
    moduleList.push_back(FOMModule());
    i->getModule(moduleList.back());
  }
}

void
Federation::getModuleList(FOMModuleList& moduleList, const ModuleHandleVector& moduleHandleVector) const
{
  for (ModuleHandleVector::const_iterator i = moduleHandleVector.begin(); i != moduleHandleVector.end(); ++i) {
    Module::HandleMap::iterator j = _moduleHandleModuleMap.find(*i);
    OpenRTIAssert(j != _moduleHandleModuleMap.end());
    moduleList.push_back(FOMModule());
    j->getModule(moduleList.back());
  }
}


void Federation::writeCurrentFDD(std::ostream& out) const
{
  out << R"XML(<?xml version="1.0" encoding="UTF-8"?>)XML" << std::endl;
  out << R"XML(<objectModel xmlns="http://standards.ieee.org/IEEE1516-2010" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://standards.ieee.org/IEEE1516-2010 http://standards.ieee.org/downloads/1516/1516.2-2010/IEEE1516-DIF-2010.xsd">)XML" << std::endl;
  std::string indent(kIndentSpaces, ' ');

  if (!_objectClassNameObjectClassMap.empty())
  {
    out << indent << R"XML(<objects>)XML" << std::endl;
    for (const ServerModel::ObjectClass& objectClass : _objectClassNameObjectClassMap)
    {
      if (!objectClass.getParentObjectClassHandle().valid())
      {
        objectClass.writeCurrentFDD(out, 2);
      }
    }
    out << indent << R"XML(</objects>)XML" << std::endl;
  }
  if (!_interactionClassNameInteractionClassMap.empty())
  {
    out << indent << R"XML(<interactions>)XML" << std::endl;
    for (const ServerModel::InteractionClass& interactionClass : _interactionClassNameInteractionClassMap)
    {
      if (!interactionClass.getParentInteractionClassHandle().valid())
      {
        interactionClass.writeCurrentFDD(out, 2);
      }
    }
    out << indent << R"XML(</interactions>)XML" << std::endl;
  }
  out << indent << R"XML(<dimensions>)XML" << std::endl;
  out << indent << R"XML(</dimensions>)XML" << std::endl;
  /*
  if (!_transpo.empty())
  {
    out << indent << R"XML(<interactions>)XML" << std::endl;
    for (const ServerModel::InteractionClass& interactionClass : _interactionClassNameInteractionClassMap)
    {
      if (!interactionClass.getParentInteractionClassHandle().valid())
      {
        interactionClass.writeCurrentFDD(out, 2);
      }
    }
    out << indent << R"XML(</interactions>)XML" << std::endl;
  }
  */
  out << indent << R"XML(<transportations>)XML" << std::endl;
  out << indent << R"XML(</transportations>)XML" << std::endl;

  out << indent << R"XML(<dataTypes>)XML" << std::endl;
  if (!_basicDataTypeNameBasicDataTypeMap.empty())
  {
    out << indent << indent << R"XML(<basicDataTypes>)XML" << std::endl;
    for (const ServerModel::BasicDataType& dataType : _basicDataTypeNameBasicDataTypeMap)
    {
      dataType.writeCurrentFDD(out, 3);
    }
    out << indent << indent << R"XML(</basicDataTypes>)XML" << std::endl;
  }
  if (!_simpleDataTypeNameSimpleDataTypeMap.empty())
  {
    out << indent << indent << R"XML(<simpleDataTypes>)XML" << std::endl;
    for (const ServerModel::SimpleDataType& dataType : _simpleDataTypeNameSimpleDataTypeMap)
    {
      dataType.writeCurrentFDD(out, 3);
    }
    out << indent << indent << R"XML(</simpleDataTypes>)XML" << std::endl;
  }
  if (!_enumeratedDataTypeNameEnumeratedDataTypeMap.empty())
  {
    out << indent << indent << R"XML(<enumeratedDataTypes>)XML" << std::endl;
    for (const ServerModel::EnumeratedDataType& dataType : _enumeratedDataTypeNameEnumeratedDataTypeMap)
    {
      dataType.writeCurrentFDD(out, 3);
    }
    out << indent << indent << R"XML(</enumeratedDataTypes>)XML" << std::endl;
  }
  if (!_arrayDataTypeNameArrayDataTypeMap.empty())
  {
    out << indent << indent << R"XML(<arrayDataTypes>)XML" << std::endl;
    for (const ServerModel::ArrayDataType& dataType : _arrayDataTypeNameArrayDataTypeMap)
    {
      dataType.writeCurrentFDD(out, 3);
    }
    out << indent << indent << R"XML(</arrayDataTypes>)XML" << std::endl;
  }
  if (!_fixedRecordDataTypeNameFixedRecordDataTypeMap.empty())
  {
    out << indent << indent << R"XML(<fixedRecordDataTypes>)XML" << std::endl;
    for (const ServerModel::FixedRecordDataType& dataType : _fixedRecordDataTypeNameFixedRecordDataTypeMap)
    {
      dataType.writeCurrentFDD(out, 3);
    }
    out << indent << indent << R"XML(</fixedRecordDataTypes>)XML" << std::endl;
  }
  if (!_variantRecordDataTypeNameVariantRecordDataTypeMap.empty())
  {
    out << indent << indent << R"XML(<variantRecordDataTypes>)XML" << std::endl;
    for (const ServerModel::VariantRecordDataType& dataType : _variantRecordDataTypeNameVariantRecordDataTypeMap)
    {
      dataType.writeCurrentFDD(out, 3);
    }
    out << indent << indent << R"XML(</variantRecordDataTypes>)XML" << std::endl;
  }
  out << indent << R"XML(</dataTypes>)XML" << std::endl;
  out << R"XML(</objectModel>)XML" << std::endl;
}

Module*
Federation::getModule(const ModuleHandle& moduleHandle)
{
  Module::HandleMap::iterator i = _moduleHandleModuleMap.find(moduleHandle);
  if (i == _moduleHandleModuleMap.end())
    return 0;
  return i.get();
}

Dimension*
Federation::getDimension(const DimensionHandle& dimensionHandle)
{
  Dimension::HandleMap::iterator i = _dimensionHandleDimensionMap.find(dimensionHandle);
  if (i == _dimensionHandleDimensionMap.end())
    return 0;
  return i.get();
}

UpdateRate*
Federation::getUpdateRate(const UpdateRateHandle& updateRateHandle)
{
  UpdateRate::HandleMap::iterator i = _updateRateHandleUpdateRateMap.find(updateRateHandle);
  if (i == _updateRateHandleUpdateRateMap.end())
    return 0;
  return i.get();
}

InteractionClass*
Federation::getInteractionClass(const InteractionClassHandle& interactionClassHandle)
{
  InteractionClass::HandleMap::iterator i = _interactionClassHandleInteractionClassMap.find(interactionClassHandle);
  if (i == _interactionClassHandleInteractionClassMap.end())
    return 0;
  return i.get();
}


InteractionClassHandle
Federation::getInteractionClassHandle(const std::string& fqInteractionClassName)
{
  StringVector interactionClassName = split(fqInteractionClassName, ".");
  InteractionClass::NameMap::iterator i;
  i = _interactionClassNameInteractionClassMap.find(interactionClassName);
  if (i != _interactionClassNameInteractionClassMap.end())
  {
    return i->getInteractionClassHandle();
  }
  return ObjectClassHandle::invalid();
}

ObjectClass*
Federation::getObjectClass(const ObjectClassHandle& objectClassHandle)
{
  ObjectClass::HandleMap::iterator i = _objectClassHandleObjectClassMap.find(objectClassHandle);
  if (i == _objectClassHandleObjectClassMap.end())
    return 0;
  return i.get();
}

ObjectClassHandle
Federation::getObjectClassHandle(const std::string& fqClassName)
{
  StringVector className = split(fqClassName, ".");
  ObjectClass::NameMap::iterator i;
  i = _objectClassNameObjectClassMap.find(className);
  if (i != _objectClassNameObjectClassMap.end())
  {
    return i->getObjectClassHandle();
  }
  return ObjectClassHandle::invalid();
}


OrderType
Federation::resolveOrderType(const std::string& orderType)
{
  if (caseCompare(orderType, "timestamp"))
    return TIMESTAMP;
  else if (caseCompare(orderType, "receive"))
    return RECEIVE;
  else if (orderType.empty())
    return TIMESTAMP;
  else
    throw InconsistentFDD(std::string("Unknown order type \"") + orderType + "\".");
}

TransportationType
Federation::resolveTransportationType(const std::string& transportationType)
{
  if (transportationType == "HLAreliable")
    return RELIABLE;
  else if (transportationType == "HLAbestEffort")
    return BEST_EFFORT;
  else if (transportationType.empty())
    return RELIABLE;
  else
    throw InconsistentFDD(std::string("Unknown transportation type \"") + transportationType + "\".");
}

Dimension*
Federation::resolveDimension(const std::string& dimensionName)
{
  Dimension::NameMap::iterator i = _dimensionNameDimensionMap.find(dimensionName);
  if (i == _dimensionNameDimensionMap.end())
    return 0;
  return i.get();
}

UpdateRate*
Federation::resolveUpdateRate(const std::string& updateRateName)
{
  UpdateRate::NameMap::iterator i = _updateRateNameUpdateRateMap.find(updateRateName);
  if (i == _updateRateNameUpdateRateMap.end())
    return 0;
  return i.get();
}

InteractionClass*
Federation::resolveParentInteractionClass(const StringVector& interactionClassName)
{
  StringVector parentInteractionClassName = interactionClassName;
  parentInteractionClassName.pop_back();
  InteractionClass::NameMap::iterator i = _interactionClassNameInteractionClassMap.find(parentInteractionClassName);
  if (i == _interactionClassNameInteractionClassMap.end())
    return 0;
  return i.get();
}

ObjectClass*
Federation::resolveParentObjectClass(const StringVector& objectClassName)
{
  StringVector parentObjectClassName = objectClassName;
  parentObjectClassName.pop_back();
  ObjectClass::NameMap::iterator i = _objectClassNameObjectClassMap.find(parentObjectClassName);
  if (i == _objectClassNameObjectClassMap.end())
    return 0;
  return i.get();
}

bool
Federation::isFederateNameInUse(const std::string& name) const
{
  return _federateNameFederateMap.find(name) != _federateNameFederateMap.end();
}

void
Federation::insert(Federate& federate)
{
  federate.setFederateHandle(_federateHandleAllocator.getOrTake(federate.getFederateHandle()));
  if (federate.getName().empty())
    federate.setName(federate.getFederateHandle().getReservedName("HLAfederate"));
  OpenRTIAssert(!isFederateNameInUse(federate.getName()));
  _federateNameFederateMap.insert(federate);
  _federateHandleFederateMap.insert(federate);
}

void
Federation::erase(const FederateHandle& federateHandle)
{
  Federate::HandleMap::iterator i = _federateHandleFederateMap.find(federateHandle);
  OpenRTIAssert(i != _federateHandleFederateMap.end());
  erase(*i);
}

void
Federation::erase(Federate& federate)
{
  _federateHandleAllocator.put(federate.getFederateHandle());
  Federate::HandleMap::erase(federate);
}

Federate*
Federation::getFederate(const FederateHandle& federateHandle)
{
  Federate::HandleMap::iterator i = _federateHandleFederateMap.find(federateHandle);
  if (i == _federateHandleFederateMap.end())
    return 0;
  return i.get();
}

void
Federation::insertTimeRegulating(Federate& federate)
{
  OpenRTIAssert(!federate.getIsTimeRegulating());
  FederationConnect* federationConnect = federate.getFederationConnect();
  OpenRTIAssert(federationConnect);
  if (!federationConnect->getIsTimeRegulating())
    _timeRegulatingFederationConnectList.push_back(*federationConnect);
  federationConnect->insertTimeRegulating(federate);
}

void
Federation::eraseTimeRegulating(Federate& federate)
{
  OpenRTIAssert(federate.getIsTimeRegulating());
  FederationConnect* federationConnect = federate.getFederationConnect();
  // We can only be time regulating if this exists and provides us with a link
  OpenRTIAssert(federationConnect);
  OpenRTIAssert(federationConnect->getIsTimeRegulating());
  federationConnect->eraseTimeRegulating(federate);
  if (!federationConnect->getTimeRegulatingFederateList().empty())
    return;
  _timeRegulatingFederationConnectList.unlink(*federationConnect);
}

void
Federation::insertTimeConstrained(Federate& federate)
{
  OpenRTIAssert(!federate.getIsTimeConstrained());
  FederationConnect* federationConnect = federate.getFederationConnect();
  if (federationConnect == nullptr || federate.getResignPending()) {
    return;
  }
  OpenRTIAssert(federationConnect);
  if (!federationConnect->getIsTimeConstrained())
    _timeConstrainedFederationConnectList.push_back(*federationConnect);
  federationConnect->insertTimeConstrained(federate);
}

void
Federation::eraseTimeConstrained(Federate& federate)
{
  OpenRTIAssert(federate.getIsTimeConstrained());
  FederationConnect* federationConnect = federate.getFederationConnect();
  // We can only be time Constrained if this exists and provides us with a link
  OpenRTIAssert(federationConnect);
  OpenRTIAssert(federationConnect->getIsTimeConstrained());
  federationConnect->eraseTimeConstrained(federate);
  if (!federationConnect->getTimeConstrainedFederateList().empty())
    return;
  _timeConstrainedFederationConnectList.unlink(*federationConnect);
}

Region*
Federation::getOrCreateRegion(const RegionHandle& regionHandle)
{
  Federate::HandleMap::iterator i = _federateHandleFederateMap.find(regionHandle.getFederateHandle());
  if (i == _federateHandleFederateMap.end())
    return 0;
  Region* region = i->getRegion(regionHandle.getLocalRegionHandle());
  if (region)
    return region;
  region = new Region;
  region->setRegionHandle(regionHandle.getLocalRegionHandle());
  i->insert(*region);
  return region;
}

Region*
Federation::getRegion(const RegionHandle& regionHandle)
{
  Federate::HandleMap::iterator i = _federateHandleFederateMap.find(regionHandle.getFederateHandle());
  if (i == _federateHandleFederateMap.end())
    return 0;
  return i->getRegion(regionHandle.getLocalRegionHandle());
}

void
Federation::insert(ObjectInstance& objectInstance)
{
  objectInstance.setObjectInstanceHandle(_objectInstanceHandleAllocator.getOrTake(objectInstance.getObjectInstanceHandle()));
  if (objectInstance.getName().empty())
    objectInstance.setName(objectInstance.getObjectInstanceHandle().getReservedName("HLAobjectInstance"));
  OpenRTIAssert(_objectInstanceHandleObjectInstanceMap.find(objectInstance.getObjectInstanceHandle()) == _objectInstanceHandleObjectInstanceMap.end());
  OpenRTIAssert(!isObjectInstanceNameInUse(objectInstance.getName()));
  _objectInstanceHandleObjectInstanceMap.insert(objectInstance);
  _objectInstanceNameObjectInstanceMap.insert(objectInstance);
}

const ObjectInstance*
Federation::getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle) const
{
  ObjectInstance::HandleMap::const_iterator i = _objectInstanceHandleObjectInstanceMap.find(objectInstanceHandle);
  if (i == _objectInstanceHandleObjectInstanceMap.end())
    return 0;
  return i.get();
}

ObjectInstance*
Federation::getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle)
{
  ObjectInstance::HandleMap::iterator i = _objectInstanceHandleObjectInstanceMap.find(objectInstanceHandle);
  if (i == _objectInstanceHandleObjectInstanceMap.end())
    return 0;
  return i.get();
}

void
Federation::erase(ObjectInstance& objectInstance)
{
#if !defined(NDEBUG) && !defined(_NDEBUG)
  auto handle = objectInstance.getObjectInstanceHandle();
  auto name = objectInstance.getName();
#endif
  _objectInstanceHandleAllocator.put(objectInstance.getObjectInstanceHandle());
  ObjectInstance::HandleMap::erase(objectInstance);

  OpenRTIAssert(_objectInstanceHandleObjectInstanceMap.find(handle) == _objectInstanceHandleObjectInstanceMap.end());
  OpenRTIAssert(!isObjectInstanceNameInUse(name));
}

bool
Federation::isObjectInstanceNameInUse(const std::string& name) const
{
  return _objectInstanceNameObjectInstanceMap.find(name) != _objectInstanceNameObjectInstanceMap.end();
}

ObjectInstance*
Federation::insertObjectInstance(const ObjectInstanceHandle& objectInstanceHandle, const std::string& objectInstanceName)
{
  ObjectInstance* objectInstance = new ObjectInstance(*this);
  objectInstance->setObjectInstanceHandle(objectInstanceHandle);
  objectInstance->setName(objectInstanceName);
  insert(*objectInstance);
  return objectInstance;
}

void Federation::initializeMom(AbstractServer* server, FederateHandle federateHandle)
{
#ifndef OPENRTI_STANDALONE_TESTS
  // be careful: we always get called twice: once for the RTI federate, a second time for the application federate
  if (_momServer == nullptr)
  {
    bool isRoot = !federateHandle.valid();
    _momServer = MakeShared<MomServer>(this, server, isRoot, federateHandle);
  }
#endif
}


void Federation::resignMomServer()
{
#ifndef OPENRTI_STANDALONE_TESTS
  if (_momServer != nullptr)
  {
    _momServer->resignFederationExecution();
  }
#endif
}

SharedPtr<MomServer> Federation::getMomServer() const
{
  return _momServer;
}

std::shared_ptr<AbstractFederateMetrics> Federation::getFederateMetrics(const ConnectHandle& connectHandle)
{
#ifndef OPENRTI_STANDALONE_TESTS
  if (_momServer != nullptr)
  {
    for (auto iter = _federateHandleFederateMap.begin(); iter != _federateHandleFederateMap.end(); iter++)
    {
      if (connectHandle == iter->getConnectHandle())
      {
        return _momServer->getFederateMetrics();
      }
    }
  }
#endif
  return std::shared_ptr<AbstractFederateMetrics>();
}

// conditionally increment the counters, if the connectHandle belongs to a ambassador which connects to this federation
// through the given connectHandle. In that case, there will be a local/client MomManager with a
// federate metrics data collector connected to this server.
void Federation::interactionSent(const ConnectHandle& connectHandle, const InteractionClass* interactionClass)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
    metrics->interactionSent(interactionClass->getInteractionClassHandle());
}

void Federation::interactionReceived(const ConnectHandle& connectHandle, const InteractionClass* interactionClass)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
    metrics->interactionReceived(interactionClass->getInteractionClassHandle());
}

void Federation::interactionClassSubscribed(const ConnectHandle& connectHandle, const InteractionClass* interactionClass, bool active)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
    metrics->interactionSubscribed(interactionClass->getInteractionClassHandle(), active);
}
void Federation::interactionClassUnsubscribed(const ConnectHandle& connectHandle, const InteractionClass* interactionClass)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
    metrics->interactionUnsubscribed(interactionClass->getInteractionClassHandle());
}
void Federation::interactionClassPublished(const ConnectHandle& connectHandle, const InteractionClass* interactionClass)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
    metrics->interactionPublished(interactionClass->getInteractionClassHandle());
}
void Federation::interactionClassUnpublished(const ConnectHandle& connectHandle, const InteractionClass* interactionClass)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
    metrics->interactionUnpublished(interactionClass->getInteractionClassHandle());
}

void Federation::objectClassSubscribed(const ConnectHandle& connectHandle, const ObjectClass* objectClass, const AttributeHandleVector& attributes, bool active)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
    metrics->objectClassSubscribed(objectClass->getObjectClassHandle(), attributes, active);
}
void Federation::objectClassUnsubscribed(const ConnectHandle& connectHandle, const ObjectClass* objectClass, const AttributeHandleVector& attributes)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
    metrics->objectClassUnsubscribed(objectClass->getObjectClassHandle(), attributes);
}
void Federation::objectClassPublished(const ConnectHandle& connectHandle, const ObjectClass* objectClass, const AttributeHandleVector& attributes)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
    metrics->objectClassPublished(objectClass->getObjectClassHandle(), attributes);
}
void Federation::objectClassUnpublished(const ConnectHandle& connectHandle, const ObjectClass* objectClass, const AttributeHandleVector& attributes)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
    metrics->objectClassUnpublished(objectClass->getObjectClassHandle(), attributes);
}

void Federation::objectInstanceReflectionReceived(const ConnectHandle& connectHandle, const ObjectInstance* objectInstance)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
  {
    ObjectInstanceHandle instanceHandle = objectInstance->getObjectInstanceHandle();
    ObjectClassHandle classHandle = objectInstance->getObjectClass()->getObjectClassHandle();
    metrics->reflectionReceived(classHandle, instanceHandle);
  }
}


void Federation::objectInstanceUpdateSent(const ConnectHandle& connectHandle, const ObjectInstance* objectInstance)
{
  auto metrics = getFederateMetrics(connectHandle);
  if (metrics != nullptr)
  {
    ObjectInstanceHandle instanceHandle = objectInstance->getObjectInstanceHandle();
    ObjectClassHandle classHandle = objectInstance->getObjectClass()->getObjectClassHandle();
    metrics->sentUpdate(classHandle, instanceHandle);
  }
}

////////////////////////////////////////////////////////////

NodeConnect::NodeConnect()
  : _isParentConnect(false)
  , _version(0)
{
}

NodeConnect::~NodeConnect()
{
  // We need to make sure that this list got processed before deletion.
  // FIXME
  // OpenRTIAssert(_federationConnectList.empty());
  _federationConnectList.clear();
}

void
NodeConnect::setConnectHandle(const ConnectHandle& connectHandle)
{
  HandleMap::Hook::setKey(connectHandle);
}

void
NodeConnect::setIsParentConnect(const bool& isParentConnect)
{
  _isParentConnect = isParentConnect;
}

void
NodeConnect::setName(const std::string& name)
{
  _name = name;
}

const SharedPtr<AbstractMessageSender>&
NodeConnect::getMessageSender() const
{
  return _messageSender;
}

void
NodeConnect::setMessageSender(const SharedPtr<AbstractMessageSender>& messageSender)
{
  _messageSender = messageSender;
}

const StringStringListMap&
NodeConnect::getOptions() const
{
  return _options;
}

void
NodeConnect::setOptions(const StringStringListMap& options)
{
  _options = options;

  StringStringListMap::const_iterator i = options.find("serverName");
  if (i != options.end() && !i->second.empty())
    _name = i->second.front();
  else
    _name.clear();
  i = options.find("version");
  assert(i != options.end() && !i->second.empty());
  if (i != options.end() && !i->second.empty())
  {
    const char* s = i->second.front().c_str();
    _version = strtoul(s, nullptr, 10);
  }
}

void
NodeConnect::send(const SharedPtr<const AbstractMessage>& message)
{
  if (!_messageSender.valid())
    return;
  //std::cout << __FUNCTION__ << ": connect=" << getConnectHandle() << " version=" << getVersion() << std::endl;
  _messageSender->send(message);
}

////////////////////////////////////////////////////////////

Node::Node() :
  _serverOptions(new ServerOptions)
{
}

Node::~Node()
{
  _federationHandleFederationMap.clear();
  OpenRTIAssert(_federationHandleFederationMap.empty());
  OpenRTIAssert(_federationNameFederationMap.empty());

  _connectHandleNodeConnectMap.clear();
  OpenRTIAssert(_connectHandleNodeConnectMap.empty());
}

bool
Node::hasChildConnects() const
{
  for (NodeConnect::HandleMap::const_reverse_iterator i = _connectHandleNodeConnectMap.rbegin();
       i != _connectHandleNodeConnectMap.rend(); ++i) {
    if (i->getIsParentConnect())
      continue;
    return true;
  }
  return false;
}

bool
Node::isRootServer() const
{
  return !_parentConnectHandle.valid();
}

bool
Node::isIdle() const
{
  return !isRootServer() && !hasChildConnects();
}

NodeConnect*
Node::getNodeConnect(const ConnectHandle& connectHandle)
{
  auto i = _connectHandleNodeConnectMap.find(connectHandle);
  if (i == _connectHandleNodeConnectMap.end())
    return 0;
  return i.get();
}

const NodeConnect*
Node::getNodeConnect(const ConnectHandle& connectHandle) const
{
  auto i = _connectHandleNodeConnectMap.find(connectHandle);
  if (i == _connectHandleNodeConnectMap.end())
    return 0;
  return i.get();
}

NodeConnect*
Node::insertNodeConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& options)
{
  NodeConnect* nodeConnect = new NodeConnect;
  // >>> ### THIS ALLOCATES THE CONNECT HANDLE ###
  insert(*nodeConnect);
  // <<< ### THIS ALLOCATES THE CONNECT HANDLE ###
  nodeConnect->setMessageSender(messageSender);
  nodeConnect->setOptions(options);
  return nodeConnect;
}

NodeConnect*
Node::insertParentNodeConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& options)
{
  OpenRTIAssert(!_parentConnectHandle.valid());
  NodeConnect* nodeConnect = insertNodeConnect(messageSender, options);
  nodeConnect->setIsParentConnect(true);
  _parentConnectHandle = nodeConnect->getConnectHandle();
  return nodeConnect;
}

void
Node::insert(NodeConnect& nodeConnect)
{
  // >>> ### THIS ALLOCATES THE CONNECT HANDLE ###
  nodeConnect.setConnectHandle(_connectHandleAllocator.getOrTake(nodeConnect.getConnectHandle()));
  // <<< ### THIS ALLOCATES THE CONNECT HANDLE ###
  _connectHandleNodeConnectMap.insert(nodeConnect);
}

void
Node::erase(const ConnectHandle& connectHandle)
{
  NodeConnect::HandleMap::iterator i = _connectHandleNodeConnectMap.find(connectHandle);
  OpenRTIAssert(i != _connectHandleNodeConnectMap.end());
  erase(*i);
}

void
Node::erase(NodeConnect& nodeConnect)
{
  if (_parentConnectHandle == nodeConnect.getConnectHandle())
    _parentConnectHandle = ConnectHandle();
  _connectHandleAllocator.put(nodeConnect.getConnectHandle());
  NodeConnect::HandleMap::erase(nodeConnect);
}

bool
Node::getFederationExecutionAlreadyExists(const std::string& federationName) const
{
  return _federationNameFederationMap.find(federationName) != _federationNameFederationMap.end();
}

void
Node::insert(Federation& federation)
{
  federation.setFederationHandle(_federationHandleAllocator.getOrTake(federation.getFederationHandle()));
  _federationNameFederationMap.insert(federation);
  _federationHandleFederationMap.insert(federation);
}

void
Node::erase(Federation& federation)
{
  OpenRTIAssert(!federation.hasJoinedChildren());
  Log(ServerFederation, Info) << getServerPath() << ": Released FederationHandle in child server for \""
                              << federation.getName() << "\"!" << std::endl;
  _federationHandleAllocator.put(federation.getFederationHandle());
  Federation::HandleMap::erase(federation);
}

void
Node::insertName(Federation& federation)
{
  _federationNameFederationMap.insert(federation);
}

void
Node::eraseName(Federation& federation)
{
  OpenRTIAssert(!federation.hasJoinedChildren());
  Log(ServerFederation, Info) << getServerPath() << ": Destroyed federation execution in child server for \""
                              << federation.getName() << "\"!" << std::endl;
  _federationNameFederationMap.unlink(federation);
}

void
Node::send(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message)
{
  NodeConnect::HandleMap::iterator i = _connectHandleNodeConnectMap.find(connectHandle);
  if (i == _connectHandleNodeConnectMap.end())
    return;
  i->send(message);
}

void
Node::sendToParent(const SharedPtr<const AbstractMessage>& message)
{
  send(_parentConnectHandle, message);
}

void
Node::broadcast(const SharedPtr<const AbstractMessage>& message)
{
  for (NodeConnect::HandleMap::iterator i = _connectHandleNodeConnectMap.begin();
       i != _connectHandleNodeConnectMap.end(); ++i) {
    i->send(message);
  }
}

void
Node::broadcast(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message)
{
  for (auto& connect : _connectHandleNodeConnectMap) {
    if (connect.getConnectHandle() == connectHandle)
      continue;
    connect.send(message);
  }
}

void
Node::broadcastToChildren(const SharedPtr<const AbstractMessage>& message)
{
  broadcast(_parentConnectHandle, message);
}


void BasicDataType::writeCurrentFDD(std::ostream& out, unsigned int level) const
{
  std::string indent(level * kIndentSpaces, ' ');
  std::string indent1((level + 1) * kIndentSpaces, ' ');
  out << indent << R"XML(<basicData>)XML" << std::endl;
  out << indent1 << R"XML(<name>)XML" << getName() << R"XML(</name>)XML" << std::endl;
  out << indent1 << R"XML(<size>)XML" << getSize() << R"XML(</size>)XML" << std::endl;
  out << indent1 << R"XML(<endian>)XML" << getEndian() << R"XML(</endian>)XML" << std::endl;
  out << indent << R"XML(</basicData>)XML" << std::endl;
}


void SimpleDataType::writeCurrentFDD(std::ostream& out, unsigned int level) const
{
  std::string indent(level * kIndentSpaces, ' ');
  std::string indent1((level + 1) * kIndentSpaces, ' ');
  out << indent << R"XML(<simpleData>)XML" << std::endl;
  out << indent1 << R"XML(<name>)XML" << getName() << R"XML(</name>)XML" << std::endl;
  out << indent1 << R"XML(<representation>)XML" << getRepresentation() << R"XML(</representation>)XML" << std::endl;
  out << indent << R"XML(</simpleData>)XML" << std::endl;
}


void EnumeratedDataType::writeCurrentFDD(std::ostream& out, unsigned int level) const
{
  std::string indent(level * kIndentSpaces, ' ');
  std::string indent1((level + 1) * kIndentSpaces, ' ');
  out << indent << R"XML(<enumeratedData>)XML" << std::endl;
  out << indent1 << R"XML(<name>)XML" << getName() << R"XML(</name>)XML" << std::endl;
  out << indent1 << R"XML(<representation>)XML" << getRepresentation() << R"XML(</representation>)XML" << std::endl;
  std::string indent2((level + 2) * kIndentSpaces, ' ');
  for (auto& enumerator : getEnumerators())
  {
    out << indent1 << R"XML(<enumerator>)XML" << std::endl;
    out << indent2 << R"XML(<name>)XML" << enumerator.getName() << R"XML(</name>)XML" << std::endl;
    out << indent2 << R"XML(<value>)XML" << enumerator.getValue() << R"XML(</value>)XML" << std::endl;
    out << indent1 << R"XML(</enumerator>)XML" << std::endl;
  }
  out << indent << R"XML(</enumeratedData>)XML" << std::endl;
}


void ArrayDataType::writeCurrentFDD(std::ostream& out, unsigned int level) const
{
  std::string indent(level * kIndentSpaces, ' ');
  std::string indent1((level + 1) * kIndentSpaces, ' ');
  out << indent << R"XML(<arrayData>)XML" << std::endl;
  out << indent1 << R"XML(<name>)XML" << getName() << R"XML(</name>)XML" << std::endl;
  out << indent1 << R"XML(<encoding>)XML" << getEncoding() << R"XML(</encoding>)XML" << std::endl;
  out << indent1 << R"XML(<dataType>)XML" << getDataType() << R"XML(</dataType>)XML" << std::endl;
  out << indent1 << R"XML(<cardinality>)XML" << getCardinality() << R"XML(</cardinality>)XML" << std::endl;
  out << indent << R"XML(</arrayData>)XML" << std::endl;
}


void FixedRecordDataType::writeCurrentFDD(std::ostream& out, unsigned int level) const
{
  std::string indent(level * kIndentSpaces, ' ');
  std::string indent1((level + 1) * kIndentSpaces, ' ');
  std::string indent2((level + 2) * kIndentSpaces, ' ');
  out << indent << R"XML(<fixedRecordData>)XML" << std::endl;
  out << indent1 << R"XML(<name>)XML" << getName() << R"XML(</name>)XML" << std::endl;
  out << indent1 << R"XML(<encoding>)XML" << getEncoding() << R"XML(</encoding>)XML" << std::endl;
  out << indent1 << R"XML(<version>)XML" << getVersion() << R"XML(</version>)XML" << std::endl;
  if (!getInclude().empty())
  {
    out << indent1 << R"XML(<include>)XML" << getInclude() << R"XML(</include>)XML" << std::endl;
  }
  for (auto& field : getFields())
  {
    out << indent1 << R"XML(<field>)XML" << std::endl;
    out << indent2 << R"XML(<name>)XML" << field.getName() << R"XML(</name>)XML" << std::endl;
    out << indent2 << R"XML(<dataType>)XML" << field.getDataType() << R"XML(</dataType>)XML" << std::endl;
    out << indent2 << R"XML(<version>)XML" << field.getVersion() << R"XML(</version>)XML" << std::endl;
    out << indent1 << R"XML(</field>)XML" << std::endl;
  }
  out << indent << R"XML(</fixedRecordData>)XML" << std::endl;
}

void VariantRecordDataType::writeCurrentFDD(std::ostream& out, unsigned int level) const
{
  std::string indent(level * kIndentSpaces, ' ');
  std::string indent1((level + 1) * kIndentSpaces, ' ');
  out << indent << R"XML(<variantRecordData>)XML" << std::endl;
  out << indent1 << R"XML(<name>)XML" << getName() << R"XML(</name>)XML" << std::endl;
  out << indent << R"XML(</variantRecordData>)XML" << std::endl;
}

} // namespace ServerModel
} // namespace OpenRTI
