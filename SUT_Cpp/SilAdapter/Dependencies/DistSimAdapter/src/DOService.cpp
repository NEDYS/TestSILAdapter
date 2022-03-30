/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#include "DOService.h"
#include "TimingService.h"

namespace NDistSimIB {

// =========================================
// Creates an instance of the Service
std::unique_ptr<RemoteAmbassador::AbstractDOService> CreateDOService(RemoteAmbassador& remoteAmb)
{
  return std::unique_ptr<RemoteAmbassador::AbstractDOService>(new DOService(remoteAmb));
}

DOService::DOService(RemoteAmbassador& remoteAmb)
  : mRemoteAmbassador(remoteAmb)
{
}

DOService::~DOService()
{
  DebugPrintf(SDebug,"~DOService\n");
}

//=========================================
// Methods called by RemoteAbmassador
void DOService::OnJoined()
{
  InitializeHandles();
  PublishSubscribe();
  //RegisterBufferedDOSources();
  ProcessPersistentRegistrations();
}

void DOService::Reset()
{
  DebugPrintf(SDebug, "%s\n", __FUNCTION__);

  // LastData should survive any reset (measurementStop, connectionLost)
  // Copy LastDataVec from local sources to persistent source container
  for (auto& peristentSource : mPersistentRegisterDOSources)
  {
    auto where_ls = mLocalSourceByLocalHandle.find(peristentSource.LocalHandle);
    if (where_ls != mLocalSourceByLocalHandle.end())
    {
      auto localSource = (*where_ls).second;
      if (!localSource->mLastDataVec.empty())
      {
        peristentSource.LastDataVec = std::move(localSource->mLastDataVec);
      }
    }
  }

  mLocalSources.clear();
  mLocalTargets.clear();
  mRemoteSources.clear();
  mRemoteTargets.clear();
  
  mRemoteSourceByObjectInstanceHandle.clear();
  mRemoteTargetByObjectInstanceHandle.clear();
  mLocalTargetCallbacks.clear();
  mLocalSourceByLocalHandle.clear();

  {
    const std::lock_guard<std::mutex> lock(mMutRegisterDOSource);
    mBufferedRegisterDOSources.clear();
  }
  {
    const std::lock_guard<std::mutex> lock(mMutRegisterDOTarget);
    mBufferedRegisterDOTargets.clear();
  }
  {
    const std::lock_guard<std::mutex> lock(mMutTransmitDOMember);
    mBufferedTransmit.clear();
  }

}

// =========================================
// ObjectClassSubscriber implementation
void DOService::discoverObjectInstance(rti::ObjectInstanceHandle theObject, rti::ObjectClassHandle theObjectClass, std::wstring const& theObjectInstanceName)
{
  if (theObjectClass == mDOSourceObjClassHandle)
  {
    // Late joiner has not received the (name) update of this object sent on creation -> request it
    DebugPrintf(SDebug, "%s: Unknown Source Object discovered: objHandle = %S  objName = %S\n", __FUNCTION__, theObject.toString().c_str(), theObjectInstanceName.c_str());

    rti::AttributeHandleSet attributesOfDOSourceClass;
    attributesOfDOSourceClass.insert(mDOSourceObjectDOMemberNameAttributeHandle);
    if (mDOSourceObjectConnectionTypeAttributeHandle.isValid())
      attributesOfDOSourceClass.insert(mDOSourceObjectConnectionTypeAttributeHandle);
    attributesOfDOSourceClass.insert(mDOSourceObjectDataBytesAttributeHandle);

    try
    {
      DebugPrintf(SDebug, "%s: Source: requestAttributeValueUpdate\n", __FUNCTION__);
      mRemoteAmbassador.mRTIAmb->requestAttributeValueUpdate(theObject, attributesOfDOSourceClass, rti::HLAASCIIstring("Name").encode());
    }
    catch (const rti::ObjectInstanceNotKnown & e)
    {
      // Federate holding the object left
      DebugPrintf(SDebug, "%s: Source: requestAttributeValueUpdate failed: %S\n", __FUNCTION__, e.what().c_str());
      (void)e;
    }
  }
  else if (theObjectClass == mDOTargetObjClassHandle)
  {
    // Late joiner has not received the (name) update of this object sent on creation -> request it
    DebugPrintf(SDebug, "%s: Unknown Target Object discovered: objHandle = %S  objName = %S\n", __FUNCTION__, theObject.toString().c_str(), theObjectInstanceName.c_str());

    rti::AttributeHandleSet attributesOfDOTargetClass;
    attributesOfDOTargetClass.insert(mDOTargetObjectDOMemberNameAttributeHandle);
    if (mDOTargetObjectConnectionTypeAttributeHandle.isValid())
      attributesOfDOTargetClass.insert(mDOTargetObjectConnectionTypeAttributeHandle);

    try
    {
      DebugPrintf(SDebug, "%s: Target: requestAttributeValueUpdate\n", __FUNCTION__);
      mRemoteAmbassador.mRTIAmb->requestAttributeValueUpdate(theObject, attributesOfDOTargetClass, rti::HLAASCIIstring("Name").encode());
    }
    catch (const rti::ObjectInstanceNotKnown & e)
    {
      // Federate holding the object left
      DebugPrintf(SDebug, "%s: Target: requestAttributeValueUpdate failed: %S\n", __FUNCTION__, e.what().c_str());
      (void)e;
    }
  }
}

void DOService::removeObjectInstance(rti::ObjectInstanceHandle theObject, rti::VariableLengthData const& theUserSuppliedTag, rti::LogicalTime const& theTime, rti::SupplementalRemoveInfo theRemoveInfo)
{
  // For remote Targets/Sources
  auto theObjectClass = mRemoteAmbassador.mRTIAmb->getKnownObjectClassHandle(theObject);
  if (theObjectClass.isValid())
  {
    if (theObjectClass == mDOSourceObjClassHandle)
    {
      // Find remote source in storage vector
      auto where_source = std::find_if(mRemoteSources.begin(), mRemoteSources.end(),
        [theObject](std::shared_ptr<DOSource> const& s) {return s->mObjectInstanceHandle == theObject; });
      if (where_source != mRemoteSources.end())
      {
        auto s = (*where_source);
        // OnLost for local target counterparts
        ExecuteLocalTargetsDiscoveryCallbacks(DODiscoveryCallbackType::LostSource, s->mName, s->mUUID, s->mConnectionType, theRemoveInfo.producingFederate);
        // OnLost for global discovery
        ExecuteGlobalDiscoveryCallbacks(DODiscoveryCallbackType::LostSource, s->mName, s->mUUID, s->mConnectionType, theRemoveInfo.producingFederate);
        
        // Remove from maps
        mRemoteSourceByObjectInstanceHandle.erase(theObject);

        // Remove from storage vector
        mRemoteSources.erase(where_source);
      }
    }
    else if (theObjectClass == mDOTargetObjClassHandle)
    {
      // Find in storage vector
      auto where_target = std::find_if(mRemoteTargets.begin(), mRemoteTargets.end(),
        [theObject](std::shared_ptr<DOTarget> const& s) {return s->mObjectInstanceHandle == theObject; });
      //auto where_target = std::find_if(mRemoteTargets.begin(), mRemoteTargets.end(),
      //  [theObject](DOTarget const& s) {return s.mObjectInstanceHandle == theObject; });
      if (where_target != mRemoteTargets.end())
      {
        auto t = (*where_target);
        // OnLost for local source counterparts
        ExecuteLocalSourcesDiscoveryCallbacks(DODiscoveryCallbackType::LostTarget, t->mName, t->mUUID, t->mConnectionType, theRemoveInfo.producingFederate);
        // OnLost for global discovery
        ExecuteGlobalDiscoveryCallbacks(DODiscoveryCallbackType::LostTarget, t->mName, t->mUUID, t->mConnectionType, theRemoveInfo.producingFederate);

        // Remove from maps
        mRemoteTargetByObjectInstanceHandle.erase(theObject);

        // Remove from storage vector
        mRemoteTargets.erase(where_target);

      }

    }
  }
}

void DOService::reflectAttributeValues(rti::ObjectInstanceHandle& theObject, rti::ObjectClassHandle theObjectClass, const rti::AttributeHandleValueMap& theAttributeValues, const rti::VariableLengthData& theUserSuppliedTag, rti::LogicalTime const& theTime, rti::FederateHandle producingFederateHandle)
{
  if (theObjectClass == mDOSourceObjClassHandle)
  {
    HandleDOSourceObjectUpdate(theObject, theAttributeValues, theTime, producingFederateHandle);
  }
  else if (theObjectClass == mDOTargetObjClassHandle)
  {
    HandleDOTargetObjectUpdate(theObject, theAttributeValues, theTime, producingFederateHandle);
  }
}

// =========================================
// ObjectAttributeRequestProvider implementation
void DOService::provideAttributeValueUpdate(rti::ObjectClassHandle theObjectClass, rti::ObjectInstanceHandle theObject, rti::AttributeHandleSet const& theAttributes, rti::VariableLengthData const& theUserSuppliedTag)
{
  if (theObjectClass == mDOSourceObjClassHandle)
  {
    auto localSource = std::find_if(mLocalSources.begin(), mLocalSources.end(), [theObject](std::shared_ptr<DOSource> const& s) {return s->mObjectInstanceHandle == theObject; });
    if (localSource != mLocalSources.end())
    {
      auto theSource = (*localSource);
      DebugPrintf(SDebug, "%s: for Source %s\n", __FUNCTION__, theSource->mName.c_str());
      bool nameUpdate = theAttributes.find(mDOSourceObjectDOMemberNameAttributeHandle) != theAttributes.end();
      if (nameUpdate)
      {
        UpdateDOSourceObjectName(*theSource);
      }
      else
      {
        if (!theSource->mLastDataVec.empty())
        {
          //DebugPrintf(SDebug, "---> TX DO Data (last data) for %s: %d\n", theSource->mName.c_str(), theSource->mLastDataVec[2] );
          UpdateDOSourceObjectData(theSource->mObjectInstanceHandle, theSource->mLastDataVec);
        }
        else
        {
          DebugPrintf(SWarn, "provideAttributeValueUpdate (value-update) for object '%s' that has no data yet -> ignore\n", theSource->mName.c_str());
        }
      }
    }
    else
    {
      std::string objName = wstring2string(mRemoteAmbassador.mRTIAmb->getObjectInstanceName(theObject));
      DebugPrintf(SDebug, "ERROR: provideAttributeValueUpdate for unknown object %s\n", objName.c_str());
    }
  }
  else if (theObjectClass == mDOTargetObjClassHandle)
  {
    auto localTarget = std::find_if(mLocalTargets.begin(), mLocalTargets.end(), [theObject](DOTarget const& s) {return s.mObjectInstanceHandle == theObject; });
    if (localTarget != mLocalTargets.end())
    {
      DebugPrintf(SDebug, "%s: for Target %s\n", __FUNCTION__, localTarget->mName.c_str());
      UpdateDOTargetObjectName(*localTarget);
    }
    else
    {
      std::string objName = wstring2string(mRemoteAmbassador.mRTIAmb->getObjectInstanceName(theObject));
      DebugPrintf(SDebug, "ERROR: provideAttributeValueUpdate for unknown object %s\n", objName.c_str());
    }
  }
}

// =========================================
// DO internal methods

void DOService::InitializeHandles()
{
  // DO Source Object
  mDOSourceObjClassHandle = mRemoteAmbassador.mRTIAmb->getObjectClassHandle(FOMObjects::kDOMemberSource);
  mDOSourceObjectDOMemberNameAttributeHandle = mRemoteAmbassador.getAttributeHandle(mDOSourceObjClassHandle, FOMObjects::kDOMemberSource_Attr_DOMemberName);
  mDOSourceObjectConnectionTypeAttributeHandle = mRemoteAmbassador.getAttributeHandle(mDOSourceObjClassHandle, FOMObjects::kDOMemberSource_Attr_DOMemberConnectionType);
  mDOSourceObjectDataBytesAttributeHandle = mRemoteAmbassador.getAttributeHandle(mDOSourceObjClassHandle, FOMObjects::kDOMemberSource_Attr_DOMemberLastDataBytes);

  // DO Target Object
  mDOTargetObjClassHandle = mRemoteAmbassador.mRTIAmb->getObjectClassHandle(FOMObjects::kDOMemberTarget);
  mDOTargetObjectDOMemberNameAttributeHandle = mRemoteAmbassador.getAttributeHandle(mDOTargetObjClassHandle, FOMObjects::kDOMemberTarget_Attr_DOMemberName);
  mDOTargetObjectConnectionTypeAttributeHandle = mRemoteAmbassador.getAttributeHandle(mDOTargetObjClassHandle, FOMObjects::kDOMemberTarget_Attr_DOMemberConnectionType);
}

void DOService::PublishSubscribe()
{
  // Objects publish/subscribe
  // Sources 
  rti::AttributeHandleSet attributesOfDOSourceClass;
  attributesOfDOSourceClass.insert(mDOSourceObjectDOMemberNameAttributeHandle);
  if (mDOSourceObjectConnectionTypeAttributeHandle.isValid())
    attributesOfDOSourceClass.insert(mDOSourceObjectConnectionTypeAttributeHandle);
  attributesOfDOSourceClass.insert(mDOSourceObjectDataBytesAttributeHandle);
  mRemoteAmbassador.mRTIAmb->publishObjectClassAttributes(mDOSourceObjClassHandle, attributesOfDOSourceClass);
  mRemoteAmbassador.subscribeObjectClassAttributes(mDOSourceObjClassHandle, attributesOfDOSourceClass, this);

  // Targets
  rti::AttributeHandleSet attributesOfDOTargetClass;
  attributesOfDOTargetClass.insert(mDOTargetObjectDOMemberNameAttributeHandle);
  if (mDOTargetObjectConnectionTypeAttributeHandle.isValid())
    attributesOfDOTargetClass.insert(mDOTargetObjectConnectionTypeAttributeHandle);
  mRemoteAmbassador.mRTIAmb->publishObjectClassAttributes(mDOTargetObjClassHandle, attributesOfDOTargetClass);
  mRemoteAmbassador.subscribeObjectClassAttributes(mDOTargetObjClassHandle, attributesOfDOTargetClass, this);

  // No self delivery 
  mRemoteAmbassador.mRTIAmb->setObjectClassDeliverToSelf(mDOSourceObjClassHandle, false);
  mRemoteAmbassador.mRTIAmb->setObjectClassDeliverToSelf(mDOTargetObjClassHandle, false);

  // Objects providers
  mRemoteAmbassador.registerAttributeRequestProvider(mDOSourceObjClassHandle, this);
  mRemoteAmbassador.registerAttributeRequestProvider(mDOTargetObjClassHandle, this);
}


void DOService::ProcessPersistentRegistrations()
{
  DebugPrintf(SDebug, "%s\n", __FUNCTION__);

  const std::lock_guard<std::mutex> lockSource(mMutRegisterDOSource);
  const std::lock_guard<std::mutex> lockTarget(mMutRegisterDOTarget);

  mBufferedRegisterDOSources.insert(std::end(mBufferedRegisterDOSources), std::begin(mPersistentRegisterDOSources), std::end(mPersistentRegisterDOSources));
  mBufferedRegisterDOTargets.insert(std::end(mBufferedRegisterDOTargets), std::begin(mPersistentRegisterDOTargets), std::end(mPersistentRegisterDOTargets));
}

bool DOService::ProcessBuffers()
{
  assert(mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::Online || mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::InitFederate);
  //DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  bool triggerEventNotification = false;
  {
    const std::lock_guard<std::mutex> lock(mMutRegisterDOTarget);
    if (!mBufferedRegisterDOTargets.empty())
    {
      for (auto& buf : mBufferedRegisterDOTargets)
      {
        RegisterBufferedDOTarget(buf);
      }
      mBufferedRegisterDOTargets.clear();
      triggerEventNotification = true;
    }
  }

  {
    const std::lock_guard<std::mutex> lock(mMutRegisterDOSource);
    if (!mBufferedRegisterDOSources.empty())
    {
      for (auto& buf : mBufferedRegisterDOSources)
      {
        RegisterBufferedDOSource(buf);
      }
      mBufferedRegisterDOSources.clear();
      triggerEventNotification = true;
    }
  }

  {
    const std::lock_guard<std::mutex> lock(mMutTransmitDOMember);
    if (!mBufferedTransmit.empty())
    {
      mBufferedTransmit.erase(std::remove_if(mBufferedTransmit.begin(), mBufferedTransmit.end(),
        [this](const TransmitDOMemberBufferData& transmitData) {
          return BufferedTransmitDOMember(transmitData) == BufferedTransmitResult::OK;
        }), mBufferedTransmit.end());
    }
  }
  return triggerEventNotification;
}

void DOService::OnDOTargetObjectReservationCompleted(const std::string& DOmemberUUID, bool wasInstanceReservationSuccessful)
{
  assert(wasInstanceReservationSuccessful);

  // Find the local target by UUID
  auto localTarget = std::find_if(mLocalTargets.begin(), mLocalTargets.end(),
    [DOmemberUUID](DOTarget const& s) {return s.mUUID == DOmemberUUID; });

  if (localTarget != mLocalTargets.end())
  {
    DebugPrintf(SDebug, "%s: %s\n", __FUNCTION__, localTarget->mName.c_str());

    // Create one HLA object per target, name is UUID
    std::wstring DOmemberUUID_w = string2wstring(DOmemberUUID);
    rti::ObjectInstanceHandle objInstanceHandle = mRemoteAmbassador.mRTIAmb->registerObjectInstance(mDOTargetObjClassHandle, DOmemberUUID_w);

    // Now we can save the objectInstancehandle
    localTarget->mObjectInstanceHandle = objInstanceHandle;

    //mLocalTargetCallbacks[localTarget->mConnectionType][localTarget->mName] = std::move(localTarget->mCallback);

    UpdateDOTargetObjectName(*localTarget);
  }
  else
  {
    Error("%s: No local target for UUID %s\n", __FUNCTION__, DOmemberUUID.c_str());
  }
}

void DOService::OnDOSourceObjectReservationCompleted(const std::string& DOmemberUUID, bool wasInstanceReservationSuccessful)
{
  assert(wasInstanceReservationSuccessful);
  auto where_source = std::find_if(mLocalSources.begin(), mLocalSources.end(),
    [DOmemberUUID](std::shared_ptr<DOSource> const& s) {return s->mUUID == DOmemberUUID; });

  if (where_source != mLocalSources.end())
  {
    auto localSource = *where_source;
    DebugPrintf(SDebug, "%s: %s\n", __FUNCTION__, localSource->mName.c_str());

    // Create one HLA object per source, name is UUID
    std::wstring DOmemberUUID_w = string2wstring(DOmemberUUID);
    rti::ObjectInstanceHandle objInstanceHandle = mRemoteAmbassador.mRTIAmb->registerObjectInstance(mDOSourceObjClassHandle, DOmemberUUID_w);

    // Now we can save the objectInstancehandle
    localSource->mObjectInstanceHandle = objInstanceHandle;

    UpdateDOSourceObjectName(*localSource);

    // Save the source by the local handle after the name update. From now on, Transmits are possible.
    mLocalSourceByLocalHandle[localSource->mLocalHandle] = localSource;

  }
  else
  {
    Error("%s: No local source for UUID %s\n", __FUNCTION__, DOmemberUUID.c_str());
  }
}

void DOService::UpdateDOSourceObjectData(const rti::ObjectInstanceHandle& theObject, const std::vector<uint8_t>& dataVec)
{
  //DebugPrintf(SDebug, "%s\n", __FUNCTION__);

  rti::AttributeHandleValueMap attributeValues;
  attributeValues[mDOSourceObjectDataBytesAttributeHandle].setData(dataVec.data(), dataVec.size());

  rti::HLAinteger64Time sendTime = mRemoteAmbassador.mInternalTimingService->GetEarliestSendTime();
  mRemoteAmbassador.mRTIAmb->updateAttributeValues(theObject, attributeValues, rti::VariableLengthData(), sendTime);
}


void DOService::UpdateDOSourceObjectName(const DOSource& source) {
  DebugPrintf(SDebug, "%s: type = NameUpdate name = %s objInstanceHandle = %S\n", __FUNCTION__, source.mName.c_str(), source.mObjectInstanceHandle.toString().c_str());

  rti::AttributeHandleValueMap attributeValues;
  attributeValues[mDOSourceObjectDOMemberNameAttributeHandle] = rti::HLAASCIIstring(source.mName).encode();
  if (mDOSourceObjectConnectionTypeAttributeHandle.isValid())
    attributeValues[mDOSourceObjectConnectionTypeAttributeHandle] = rti::HLAASCIIstring(source.mConnectionType).encode();

  // Also send the last data with the name update
  attributeValues[mDOSourceObjectDataBytesAttributeHandle].setData(source.mLastDataVec.data(), source.mLastDataVec.size());

  rti::HLAinteger64Time sendTime = mRemoteAmbassador.mInternalTimingService->GetEarliestSendTime();
  mRemoteAmbassador.mRTIAmb->updateAttributeValues(source.mObjectInstanceHandle, attributeValues, rti::VariableLengthData(), sendTime);
}

void DOService::UpdateDOTargetObjectName(const DOTarget& target)
{
  DebugPrintf(SDebug, "%s: type = NameUpdate name = %s objInstanceHandle = %S\n", __FUNCTION__, target.mName.c_str(), target.mObjectInstanceHandle.toString().c_str());

  rti::AttributeHandleValueMap attributeValues;
  attributeValues[mDOTargetObjectDOMemberNameAttributeHandle] = rti::HLAASCIIstring(target.mName).encode();
  if (mDOTargetObjectConnectionTypeAttributeHandle.isValid())
    attributeValues[mDOTargetObjectConnectionTypeAttributeHandle] = rti::HLAASCIIstring(target.mConnectionType).encode();

  rti::HLAinteger64Time sendTime = mRemoteAmbassador.mInternalTimingService->GetEarliestSendTime();
  mRemoteAmbassador.mRTIAmb->updateAttributeValues(target.mObjectInstanceHandle, attributeValues, rti::VariableLengthData(), sendTime);
}

DOService::DOMemberType DOService::IdentifyDOMemberType(const std::string& DOMemberName)
{
  if (endsWith(DOMemberName, "_Value"))
    return DOMemberType::Value;
  else if (endsWith(DOMemberName, "_Call"))
    return DOMemberType::Call;
  else if (endsWith(DOMemberName, "_CallReturn"))
    return DOMemberType::CallReturn;
  else
    return DOMemberType::Unknown;
}

void DOService::HandleDOSourceObjectUpdate(const rti::ObjectInstanceHandle& objInstanceHandle, const rti::AttributeHandleValueMap& theAttributeValues, rti::LogicalTime const& theTime, rti::FederateHandle producingFederateHandle)
{
  // If there is a DOName in the attribute map, this update contains DOSource information.
  // Otherwise, this update contains DO data
  bool nameUpdate = theAttributeValues.find(mDOSourceObjectDOMemberNameAttributeHandle) != theAttributeValues.end();
  // Check if the source is already known (i.e. can be identified by the objectInstanceHandle)  
  bool newSource = mRemoteSourceByObjectInstanceHandle.find(objInstanceHandle) == mRemoteSourceByObjectInstanceHandle.end();

  if (nameUpdate)
  {
    if (newSource)
    {
      // Decode data
      // DO Name
      rti::VariableLengthData DOMemberName_vld;
      if (!mRemoteAmbassador.ReadAttribute(/*out*/ DOMemberName_vld, mDOSourceObjClassHandle, theAttributeValues, mDOSourceObjectDOMemberNameAttributeHandle))
      {
        return; // Abort if not available
      }
      rti::HLAASCIIstring DOMemberName_HLAstring;
      DOMemberName_HLAstring.decode(DOMemberName_vld);
      std::string DOMemberName = DOMemberName_HLAstring.get();

      // Connection type
      std::string DOMemberConnectionType;
      rti::VariableLengthData DOMemberConnectionType_vld;
      if (mDOSourceObjectConnectionTypeAttributeHandle.isValid() && // isValid for FOM merge CTest
          mRemoteAmbassador.ReadAttribute(/*out*/ DOMemberConnectionType_vld, mDOSourceObjClassHandle, theAttributeValues, mDOSourceObjectConnectionTypeAttributeHandle))
      {
        rti::HLAASCIIstring DOMemberConnectionType_HLAstring;
        DOMemberConnectionType_HLAstring.decode(DOMemberConnectionType_vld);
        DOMemberConnectionType = DOMemberConnectionType_HLAstring.get();
      }
      else
      {
        DOMemberConnectionType = IBDefaults::DOs::ConnectionType;
      }

      std::string uuid = wstring2string(mRemoteAmbassador.mRTIAmb->getObjectInstanceName(objInstanceHandle));

      // Create source that lives remotely
      mRemoteSources.emplace_back(std::make_shared<DOSource>(DOMemberName, uuid, DOMemberConnectionType, 0, objInstanceHandle, nullptr));
      // Used in this method to check for newSource
      mRemoteSourceByObjectInstanceHandle[objInstanceHandle] = mRemoteSources.back();

      DebugPrintf(SDebug, "%s: Name-Update: Identify %S as %s\n", __FUNCTION__, objInstanceHandle.toString().c_str(), DOMemberName.c_str());

      // Global Discovery Callbacks
      ExecuteGlobalDiscoveryCallbacks(DODiscoveryCallbackType::DiscoverSource, DOMemberName, uuid, DOMemberConnectionType, producingFederateHandle);

      // Check for local target counterparts, execute object specific discovery
      bool hasLocalTarget = ExecuteLocalTargetsDiscoveryCallbacks(DODiscoveryCallbackType::DiscoverSource, DOMemberName, uuid, DOMemberConnectionType, producingFederateHandle);;
      if (hasLocalTarget)
      {
        // If we have a local target, we unpack the data and execute the callback
        rti::VariableLengthData DOMemberData_vld;
        if (mRemoteAmbassador.ReadAttribute(DOMemberData_vld, mDOSourceObjClassHandle, theAttributeValues, mDOSourceObjectDataBytesAttributeHandle))
        {
          auto data = static_cast<const uint8_t*>(DOMemberData_vld.data());
          auto dataVec = std::vector<uint8_t>(data, data + DOMemberData_vld.size());
          if (dataVec.size() > 0)
          {
            //DebugPrintf(SDebug, "<--- RX DO Data (initial) for %s: %d\n", DOMemberName.c_str(), dataVec[2]);

            ExecuteTargetCallbacks(DOMemberName, DOMemberConnectionType, dataVec, theTime);
          }
          else
          {
            DebugPrintf(SWarn, "%s: Name-Update: No data provided\n", __FUNCTION__);
          }
        }
      }
      else
      {
        // Unsubscribe if no target on this federate. Resubscribe happens on target registration via mRemoteDOSourceByObjectInstanceHandle
        DebugPrintf(SDebug, "%s: Name-Update for %s: Received Name Update from known Source that has no Callback -> unsubscribe\n", __FUNCTION__, DOMemberName.c_str());
        mRemoteAmbassador.mRTIAmb->unsubscribeObjectInstance(objInstanceHandle);
      }
    }
    else
    {
      DebugPrintf(SDebug, "%s: Name-Update for %s: Received another Name update from known source -> Ignore\n", __FUNCTION__, mRemoteSourceByObjectInstanceHandle[objInstanceHandle]->mName.c_str());
    }
  }
  else // Value Update
  {
    if (newSource)
    {
      DebugPrintf(SDebug, "%s: Value-Update: Received Value update from unknown source -> Ignore\n", __FUNCTION__);
    }
    else
    {
      auto remoteSource = mRemoteSourceByObjectInstanceHandle[objInstanceHandle];

      if (hasLocalTargetCallback(remoteSource->mName, remoteSource->mConnectionType))
      {
        rti::VariableLengthData DOMemberData_vld;
        if (mRemoteAmbassador.ReadAttribute(DOMemberData_vld, mDOSourceObjClassHandle, theAttributeValues, mDOSourceObjectDataBytesAttributeHandle))
        {
          auto data = static_cast<const uint8_t*>(DOMemberData_vld.data());
          auto dataVec = std::vector<uint8_t>(data, data + DOMemberData_vld.size());
          //DebugPrintf(SDebug, "<--- RX DO Data (transmit) for %s: %d\n", remoteSource->mName.c_str(), dataVec[2]);
          ExecuteTargetCallbacks(remoteSource->mName, remoteSource->mConnectionType, dataVec, theTime);
        }
      }
      else
      {
        DebugPrintf(SDebug, "%s: Value-Update for %s: Received Value Update from known Source that has no Callback -> ignore\n", __FUNCTION__, remoteSource->mName.c_str());
      }
    }
  }
}

bool DOService::ExecuteLocalTargetsDiscoveryCallbacks(DODiscoveryCallbackType discoveryType, const std::string& DOMemberName, const std::string& uuid, const std::string& connectionType, rti::FederateHandle producingFederateHandle)
{
  bool hasLocalCounterpart = false;
  for (auto lt : mLocalTargets)
  {
    if (lt.mName == DOMemberName && lt.mConnectionType == connectionType)
    {
      hasLocalCounterpart = true;
      std::string producingFederateName = "UnknownFederate";
      try
      {
        producingFederateName = wstring2string(mRemoteAmbassador.mRTIAmb->getFederateName(producingFederateHandle));
      }
      catch (const rti::InvalidFederateHandle & e)
      {
        // Federate no longer available
        DebugPrintf(SDebug, "%s: getFederateName failed: %S\n", __FUNCTION__, e.what().c_str());
        (void)e;
      }
      if (lt.mDiscoverer != nullptr)
      {
        if (discoveryType == DODiscoveryCallbackType::DiscoverSource)
          lt.mDiscoverer->OnDiscoverCounterpart(DOMemberName, uuid, lt.mConnectionType, producingFederateName);
        else if (discoveryType == DODiscoveryCallbackType::LostSource)
          lt.mDiscoverer->OnLostCounterpart(DOMemberName, uuid, lt.mConnectionType, producingFederateName);
      }
    }
  }
  return hasLocalCounterpart;
}

bool DOService::ExecuteLocalSourcesDiscoveryCallbacks(DODiscoveryCallbackType discoveryType, const std::string& DOMemberName, const std::string& uuid, const std::string& connectionType, rti::FederateHandle producingFederateHandle)
{
  bool hasLocalCounterpart = false;
  for (auto lt : mLocalSources)
  {
    if (lt->mName == DOMemberName && lt->mConnectionType == connectionType)
    {
      hasLocalCounterpart = true;
      std::string producingFederateName = "UnknownFederate";
      try
      {
        producingFederateName = wstring2string(mRemoteAmbassador.mRTIAmb->getFederateName(producingFederateHandle));
      }
      catch (const rti::InvalidFederateHandle & e)
      {
        // Federate no longer available
        DebugPrintf(SDebug, "%s: getFederateName failed: %S\n", __FUNCTION__, e.what().c_str());
        (void)e;
      }
      if (lt->mDiscoverer != nullptr)
      {
        if (discoveryType == DODiscoveryCallbackType::DiscoverTarget)
          lt->mDiscoverer->OnDiscoverCounterpart(DOMemberName, uuid, lt->mConnectionType, producingFederateName);
        else if (discoveryType == DODiscoveryCallbackType::LostTarget)
          lt->mDiscoverer->OnLostCounterpart(DOMemberName, uuid, lt->mConnectionType, producingFederateName);
      }
    }
  }
  return hasLocalCounterpart;
}

bool DOService::hasLocalTargetCallback(const std::string& DOMemberName, const std::string& DOMemberConnectionType) {
  return mLocalTargetCallbacks.find(DOMemberConnectionType) != mLocalTargetCallbacks.end() &&
         mLocalTargetCallbacks[DOMemberConnectionType].find(DOMemberName) != mLocalTargetCallbacks[DOMemberConnectionType].end();
}

void DOService::HandleDOTargetObjectUpdate(const rti::ObjectInstanceHandle& objInstanceHandle, const rti::AttributeHandleValueMap& theAttributeValues, rti::LogicalTime const& theTime, rti::FederateHandle producingFederateHandle)
{
  bool newTarget = mRemoteTargetByObjectInstanceHandle.find(objInstanceHandle) == mRemoteTargetByObjectInstanceHandle.end();

  if (newTarget)
  {
    // DO Name
    rti::VariableLengthData DOMemberName_vld;
    if (!mRemoteAmbassador.ReadAttribute(/*out*/ DOMemberName_vld, mDOTargetObjClassHandle, theAttributeValues, mDOTargetObjectDOMemberNameAttributeHandle))
    {
      return; // Abort if not available
    }
    rti::HLAASCIIstring DOMemberName_HLAstring;
    DOMemberName_HLAstring.decode(DOMemberName_vld);
    std::string DOMemberName = DOMemberName_HLAstring.get();

    // Connection type
    std::string DOMemberConnectionType;
    rti::VariableLengthData DOMemberConnectionType_vld;
    if (mDOTargetObjectConnectionTypeAttributeHandle.isValid() && // isValid for FOM merge CTest
      mRemoteAmbassador.ReadAttribute(/*out*/ DOMemberConnectionType_vld, mDOTargetObjClassHandle, theAttributeValues, mDOTargetObjectConnectionTypeAttributeHandle))
    {
      rti::HLAASCIIstring DOMemberConnectionType_HLAstring;
      DOMemberConnectionType_HLAstring.decode(DOMemberConnectionType_vld);
      DOMemberConnectionType = DOMemberConnectionType_HLAstring.get();
    }
    else
    {
      DOMemberConnectionType = IBDefaults::DOs::ConnectionType;
    }
    std::string uuid = wstring2string(mRemoteAmbassador.mRTIAmb->getObjectInstanceName(objInstanceHandle));

    // Create target that lives remotely
    mRemoteTargets.emplace_back(std::make_shared<DOTarget>(DOMemberName, uuid, DOMemberConnectionType, 0, objInstanceHandle, DOMemberReceptionCallback(), nullptr));
    mRemoteTargetByObjectInstanceHandle[objInstanceHandle] = mRemoteTargets.back();

    DebugPrintf(SDebug, "%s: Name-Update: Identify %S as %s\n", __FUNCTION__, objInstanceHandle.toString().c_str(), DOMemberName.c_str());

    ExecuteGlobalDiscoveryCallbacks(DODiscoveryCallbackType::DiscoverTarget, DOMemberName, uuid, DOMemberConnectionType, producingFederateHandle);
    // Possibly execute discovery for local source counterpart, 
    ExecuteLocalSourcesDiscoveryCallbacks(DODiscoveryCallbackType::DiscoverTarget, DOMemberName, uuid, DOMemberConnectionType, producingFederateHandle);
  }
  else
  {
    DebugPrintf(SDebug, "%s: Name-Update: Received another Name update for known target -> Ignore\n", __FUNCTION__);
  }
}

void DOService::ExecuteGlobalDiscoveryCallbacks(DODiscoveryCallbackType discoveryType, const std::string& DOMemberName, const std::string& DOuuid, const std::string& DOMemberConnectionType, rti::FederateHandle producingFederateHandle)
{
  if (mDODiscoverers.find(DOMemberConnectionType) != mDODiscoverers.end())
  {

    DebugPrintf(SDebug, "%s: %s\n", __FUNCTION__, DOMemberName.c_str());

    std::string producingFederateName = "UnknownFederate";
    try
    {
      producingFederateName = wstring2string(mRemoteAmbassador.mRTIAmb->getFederateName(producingFederateHandle));
    }
    catch (const rti::InvalidFederateHandle & e)
    {
      // Federate no longer available
      DebugPrintf(SDebug, "%s: getFederateName failed: %S\n", __FUNCTION__, e.what().c_str());
      (void)e;
    }

    for (auto& discoCallback : mDODiscoverers[DOMemberConnectionType])
    {
      switch (discoveryType)
      {
      case DODiscoveryCallbackType::DiscoverSource:
        discoCallback->OnDiscoverDOSource(DOMemberName, DOuuid, DOMemberConnectionType, producingFederateName);
        break;
      case DODiscoveryCallbackType::DiscoverTarget:
        discoCallback->OnDiscoverDOTarget(DOMemberName, DOuuid, DOMemberConnectionType, producingFederateName);
        break;
      case DODiscoveryCallbackType::LostSource:
        discoCallback->OnLostDOSource(DOMemberName, DOuuid, DOMemberConnectionType, producingFederateName);
        break;
      case DODiscoveryCallbackType::LostTarget:
        discoCallback->OnLostDOTarget(DOMemberName, DOuuid, DOMemberConnectionType, producingFederateName);
        break;
      }
    }
  }
}

void DOService::ExecuteTargetCallbacks(const std::string& DOMemberName, const std::string& DOMemberConnectionType, const std::vector<uint8_t>& dataVec, const rti::LogicalTime& theTime)
{
  // TODO: Execute all callbacks if there are several targets for one source

  auto timestamp = convertTime(theTime);
  auto result = mLocalTargetCallbacks[DOMemberConnectionType][DOMemberName](dataVec, timestamp);
  if (result)
  {
    Error("%s: could not set DO member %S: %d\n", __FUNCTION__, DOMemberName.c_str(), result);
  }
  
}

// =========================================
// DO public API methods

// Targets
IDOService::DOHandle DOService::RegisterDOMemberTarget(const std::string& DOMemberName, DOMemberReceptionCallback receptionCallback) {
  return RegisterDOMemberTarget(DOMemberName, receptionCallback, nullptr);
};
IDOService::DOHandle DOService::RegisterDOMemberTarget(const std::string& DOMemberName, DOMemberReceptionCallback receptionCallback, IDODiscovererCounterpart* discoverer) {
  return RegisterDOMemberTargetCommon(DOMemberName, receptionCallback, discoverer, IBDefaults::DOs::ConnectionType);
};
IDOService::DOHandle DOService::RegisterDOMemberTarget(const std::string& DOMemberName, DOMemberReceptionCallback receptionCallback, IDODiscovererCounterpart* discoverer, const std::string& connectionType)
{
  DebugPrintf(SDebug,"%s: %s\n", __FUNCTION__, DOMemberName.c_str());

  if (connectionType == IBDefaults::DOs::ConnectionType)
  {
    Error("%s: Invalid connection type: '%s'\n", __FUNCTION__, connectionType.c_str());
    return IDOService::kInvalidRegistrationHandle;
  }
  return RegisterDOMemberTargetCommon(DOMemberName, receptionCallback, discoverer, connectionType);
}
IDOService::DOHandle DOService::RegisterDOMemberTargetCommon(const std::string& DOMemberName, DOMemberReceptionCallback receptionCallback, IDODiscovererCounterpart* discoverer, const std::string& connectionType)
{
  const std::lock_guard<std::mutex> lock(mMutRegisterDOTarget);
  DebugPrintf(SDebug, "%s: %s\n", __FUNCTION__, DOMemberName.c_str());

  // Processed when federation is joined
  mPersistentRegisterDOTargets.emplace_back(DOMemberName, connectionType, mNextLocalDOTargetHandle, receptionCallback, discoverer);

  // If Online, also save to short term buffer. Processed in event loop
  if (mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::Online || mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::InitFederate)
  {
    mBufferedRegisterDOTargets.emplace_back(DOMemberName, connectionType, mNextLocalDOTargetHandle, receptionCallback, discoverer);
  }
  return mNextLocalDOTargetHandle++;
}


void DOService::RegisterBufferedDOTarget(const RegisterBufferDOTargetData& DOTargetData)
{
  assert(mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::Online || mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::InitFederate);

  CreateDOTarget(DOTargetData);

  // If the name of the target is found in mRemoteSources, this federate has discovered remote sources already,
  // but unsubscribed because this federate had no target yet.
  // -> call requestAttributeUpdate (as value update), which implicitly causes a resubscribe in OpenRTI.
  for (auto it : mRemoteSources)
  {
    if (it->mName == DOTargetData.DOMemberName)
    {
      DebugPrintf(SDebug, "%s: Found Sources for %s -> requestAttributeValueUpdate (Value-Update) for objInstanceHandle %S\n", __FUNCTION__, it->mName.c_str(), it->mObjectInstanceHandle.toString().c_str());
      rti::AttributeHandleSet attributesOfDOSourceClass;
      attributesOfDOSourceClass.insert(mDOSourceObjectDataBytesAttributeHandle);
      mRemoteAmbassador.mRTIAmb->requestAttributeValueUpdate(it->mObjectInstanceHandle, attributesOfDOSourceClass, rti::VariableLengthData());
    }
  }
}

IDOService::DOHandle DOService::RegisterDOMemberTargetBySourceUUID(const std::string& sourceUUID, DOMemberReceptionCallback receptionCallback)
{
  return RegisterDOMemberTargetBySourceUUIDCommon(sourceUUID, receptionCallback, IBDefaults::DOs::ConnectionType);
}
IDOService::DOHandle DOService::RegisterDOMemberTargetBySourceUUID(const std::string& sourceUUID, DOMemberReceptionCallback receptionCallback, const std::string& connectionType)
{
  if (connectionType == IBDefaults::DOs::ConnectionType)
  {
    Error("%s: Invalid connection type: '%s'\n", __FUNCTION__, connectionType.c_str());
    return IDOService::kInvalidRegistrationHandle;
  }
  return RegisterDOMemberTargetBySourceUUIDCommon(sourceUUID, receptionCallback, connectionType);
}

IDOService::DOHandle DOService::RegisterDOMemberTargetBySourceUUIDCommon(const std::string& sourceUUID, DOMemberReceptionCallback receptionCallback, const std::string& connectionType)
{
  const std::lock_guard<std::mutex> lock(mMutRegisterDOTarget);
  // We only can know about a sourceUUID by global discovery of the source object, so Counterpart/GlobalDiscovery makes no sense here.
  // Find the source by UUID:
  std::string DOSourceName;
  bool foundSource = false;
  for (auto it : mRemoteSources)
  {
    if (it->mUUID == sourceUUID)
    {
      DebugPrintf(SDebug, "%s: Found Sources for %s -> requestAttributeValueUpdate (Value-Update) for objInstanceHandle %S\n", __FUNCTION__, it->mName.c_str(), it->mObjectInstanceHandle.toString().c_str());
      DOSourceName = it->mName;
      rti::AttributeHandleSet attributesOfDOSourceClass;
      attributesOfDOSourceClass.insert(mDOSourceObjectDataBytesAttributeHandle);
      mRemoteAmbassador.mRTIAmb->requestAttributeValueUpdate(it->mObjectInstanceHandle, attributesOfDOSourceClass, rti::VariableLengthData());
      foundSource = true;
      break;
    }
  }

  // No source found? Can happen if wrong uuid was given.
  if (!foundSource)
  {
    Error("%s: No DOSource with UUID '%s' was found.\n", __FUNCTION__, sourceUUID.c_str());
    return IDOService::kInvalidRegistrationHandle;
  }

  // Now we know the DOName, create the target object.
  CreateDOTarget(RegisterBufferDOTargetData(
    DOSourceName,
    connectionType,
    mNextLocalDOTargetHandle,
    receptionCallback,
    nullptr));

  return mNextLocalDOTargetHandle++;
}

void DOService::CreateDOTarget(const RegisterBufferDOTargetData& DOTargetData)
{
  DebugPrintf(SDebug, "%s: %s\n", __FUNCTION__, DOTargetData.DOMemberName.c_str());
  // Always assign a UUID per target 
  UUIDT4::UUID DOmemberUUID = UUIDT4::generate();
  const std::string DOmemberUUID_s = UUIDT4::toString(DOmemberUUID);

  // Create the local target object
  mLocalTargets.emplace_back(DOTargetData.DOMemberName, DOmemberUUID_s, DOTargetData.ConnectionType, DOTargetData.LocalHandle, rti::ObjectInstanceHandle(), DOTargetData.ReceptionCallback, DOTargetData.Discoverer);
  mLocalTargetCallbacks[DOTargetData.ConnectionType][DOTargetData.DOMemberName] = std::move(DOTargetData.ReceptionCallback);

  mRemoteAmbassador.reserveObjectInstanceName(string2wstring(DOmemberUUID_s), [this](const std::wstring& theObjectInstanceName, bool succeeded) {
    OnDOTargetObjectReservationCompleted(wstring2string(theObjectInstanceName), succeeded);
    });
}

void DOService::UnregisterDOMemberTarget(DOHandle DOMemberHandle)
{
  const std::lock_guard<std::mutex> lockEC(mRemoteAmbassador.mMutEvokeCallbacks);
  const std::lock_guard<std::mutex> lockBuf(mMutRegisterDOTarget);

  DebugPrintf(SDebug, "%s: %d\n", __FUNCTION__, DOMemberHandle);

  auto where_pers = std::find_if(mPersistentRegisterDOTargets.begin(), mPersistentRegisterDOTargets.end(),
    [DOMemberHandle](RegisterBufferDOTargetData const& s) { return s.LocalHandle == DOMemberHandle; });
    
  if (where_pers == mPersistentRegisterDOTargets.end())
  {
    DebugPrintf(SDebug, "%s: Unknown handle %d\n", __FUNCTION__, DOMemberHandle);
  }
  else
  {
    // Always erase from persistent registry by handle
    mPersistentRegisterDOTargets.erase(where_pers);

    // Find from short term buffer registry by handle
    auto where_buff = std::find_if(mBufferedRegisterDOTargets.begin(), mBufferedRegisterDOTargets.end(),
      [DOMemberHandle](RegisterBufferDOTargetData const& s) { return s.LocalHandle == DOMemberHandle; });
    // If found in short term buffer, it's sufficient to only erase there
    if (where_buff != mBufferedRegisterDOTargets.end())
    {
      mBufferedRegisterDOTargets.erase(where_buff);
    }
    else
    {
      // Find in storage vector by LocalHandle
      auto where_target = std::find_if(mLocalTargets.begin(), mLocalTargets.end(), 
        [DOMemberHandle](DOTarget const& s) {return s.mLocalHandle == DOMemberHandle; });
      if (where_target != mLocalTargets.end())
      {
        if (mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::Online)
        {
          try
          {
            // Delete HLA Object
            mRemoteAmbassador.mRTIAmb->deleteObjectInstance(where_target->mObjectInstanceHandle, rti::VariableLengthData());
          }
          catch (const rti::Exception & e)
          {
            Error("%s: %S\n", __FUNCTION__, e.what().c_str());
            (void)e;
            throw;
          }
        }
        // Remove from maps
        mLocalTargetCallbacks.erase(where_target->mName);

        // Remove from storage vector
        mLocalTargets.erase(where_target);
        
      }
      else 
      {
        Error("%s: Unknown handle %d\n", __FUNCTION__, DOMemberHandle);
      }
    }
  }
}

// Sources
IDOService::DOHandle DOService::RegisterDOMemberSource(const std::string& DOMemberName) {
  return RegisterDOMemberSource(DOMemberName, nullptr);
};
IDOService::DOHandle DOService::RegisterDOMemberSource(const std::string& DOMemberName, IDODiscovererCounterpart* discoverer)
{
  return RegisterDOMemberSourceCommon(DOMemberName, discoverer, IBDefaults::DOs::ConnectionType);
}
IDOService::DOHandle DOService::RegisterDOMemberSource(const std::string& DOMemberName, IDODiscovererCounterpart* discoverer, const std::string& connectionType)
{
  DebugPrintf(SDebug, "%s: %s\n", __FUNCTION__, DOMemberName.c_str());
  if (connectionType == IBDefaults::DOs::ConnectionType)
  {
    Error("%s: Invalid connection type: '%s'\n", __FUNCTION__, connectionType.c_str());
    return IDOService::kInvalidRegistrationHandle;
  }
  return RegisterDOMemberSourceCommon(DOMemberName, discoverer, connectionType);
}
IDOService::DOHandle DOService::RegisterDOMemberSourceCommon(const std::string& DOMemberName, IDODiscovererCounterpart* discoverer, const std::string& connectionType)
{
  const std::lock_guard<std::mutex> lock(mMutRegisterDOSource);

  // Processed when federation is joined
  mPersistentRegisterDOSources.emplace_back(DOMemberName, connectionType, mNextLocalDOSourceHandle, discoverer);
  // If Online, also save to short term buffer. Processed in event loop
  if (mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::Online || mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::InitFederate)
  {
    mBufferedRegisterDOSources.emplace_back(DOMemberName, connectionType, mNextLocalDOSourceHandle, discoverer);
  }
  return mNextLocalDOSourceHandle++;
}

void DOService::RegisterBufferedDOSource(const RegisterBufferDOSourceData& DOSourceData)
{
  assert(mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::Online || mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::InitFederate);

  DebugPrintf(SDebug, "%s: %s\n", __FUNCTION__, DOSourceData.DOMemberName.c_str());
  // Always assign a UUID per source 
  UUIDT4::UUID DOmemberUUID = UUIDT4::generate();
  const std::string DOmemberUUID_s = UUIDT4::toString(DOmemberUUID);

  // Create the local source object
  mLocalSources.emplace_back(std::make_shared<DOSource>(DOSourceData.DOMemberName, DOmemberUUID_s, DOSourceData.ConnectionType, DOSourceData.LocalHandle, rti::ObjectInstanceHandle(), DOSourceData.Discoverer));
  // Eventually use the last data from previous measurement
  if (!DOSourceData.LastDataVec.empty())
  {
    mLocalSources.back()->mLastDataVec = DOSourceData.LastDataVec;
  }

  mRemoteAmbassador.reserveObjectInstanceName(string2wstring(DOmemberUUID_s), [this](const std::wstring& theObjectInstanceName, bool succeeded) {
    OnDOSourceObjectReservationCompleted(wstring2string(theObjectInstanceName), succeeded);
    });
}

void DOService::UnregisterDOMemberSource(IDOService::DOHandle DOMemberHandle)
{
  const std::lock_guard<std::mutex> lockEC(mRemoteAmbassador.mMutEvokeCallbacks);
  const std::lock_guard<std::mutex> lockBuf(mMutRegisterDOSource);

  DebugPrintf(SDebug, "%s: %d\n", __FUNCTION__, DOMemberHandle);
  auto where_pers = std::find_if(mPersistentRegisterDOSources.begin(), mPersistentRegisterDOSources.end(),
    [DOMemberHandle](RegisterBufferDOSourceData const& s) { return s.LocalHandle == DOMemberHandle; });

  if (where_pers == mPersistentRegisterDOSources.end())
  {
    Error("%s: Unknown handle %d\n", __FUNCTION__, DOMemberHandle);
  }
  else
  {
    // Always erase from persistent registry 
    mPersistentRegisterDOSources.erase(where_pers);

    // Find from short term buffer registry by handle
    auto where_buff = std::find_if(mBufferedRegisterDOSources.begin(), mBufferedRegisterDOSources.end(),
      [DOMemberHandle](RegisterBufferDOSourceData const& s) { return s.LocalHandle == DOMemberHandle; });
    // If found in short term buffer, it's sufficient to only erase there
    if (where_buff != mBufferedRegisterDOSources.end())
    {
      mBufferedRegisterDOSources.erase(where_buff);
    }
    else
    {
      // Find in storage vector by LocalHandle
      auto where_source = std::find_if(mLocalSources.begin(), mLocalSources.end(),
        [DOMemberHandle](std::shared_ptr<DOSource> const& s) {return s->mLocalHandle == DOMemberHandle; });
      if (where_source != mLocalSources.end())
      {
        auto localSource = *where_source;

        if (mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::Online)
        {
          try
          {
            // Delete HLA Object
            mRemoteAmbassador.mRTIAmb->deleteObjectInstance(localSource->mObjectInstanceHandle, rti::VariableLengthData());
          }
          catch (const rti::Exception & e)
          {
            Error("%s: %S\n", __FUNCTION__, e.what().c_str());
            (void)e;
            throw;
          }
        }
        // Remove from maps
        mLocalSourceByLocalHandle.erase(DOMemberHandle);

        // Remove from storage vector
        mLocalSources.erase(where_source);

      }
      else
      {
        Error("%s: Unknown handle %d\n", __FUNCTION__, DOMemberHandle);
      }
      
    }
  }
}

// Transmission
// Simple API
IDOService::TransmitResult DOService::TransmitDOMember(DOHandle localHandle, const std::vector<uint8_t>& dataVec)
{
  {
    // Lock writing in the buffer
    const std::lock_guard<std::mutex> lock(mMutTransmitDOMember);
    mBufferedTransmit.emplace_back(localHandle, dataVec);
  }

  if (mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::Online && mRemoteAmbassador.mEventNotification != nullptr)
  {
    mRemoteAmbassador.SendManualSignal();
  }
  return TransmitResult::OK;
}

DOService::BufferedTransmitResult DOService::BufferedTransmitDOMember(const TransmitDOMemberBufferData& transmitDOData)
{
  assert(mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::Online || mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::InitFederate);

  auto where_source = mLocalSourceByLocalHandle.find(transmitDOData.LocalHandle);
  if (where_source != mLocalSourceByLocalHandle.end())
  {
    auto localSource = (*where_source).second;
    // Copy the data in the local source container
    localSource->mLastDataVec = transmitDOData.DataVec;

    //DebugPrintf(SDebug, "---> TX DO Data (transmit) for %s: %d\n", localSource->mName.c_str(), transmitDOData.DataVec[2]);
    UpdateDOSourceObjectData(localSource->mObjectInstanceHandle, transmitDOData.DataVec);
    return BufferedTransmitResult::OK;
  }
  else
  {
    DebugPrintf(SDebug, "%s: DO Source handle %i not found. Wrong handle or registration not completed yet\n", __FUNCTION__, transmitDOData.LocalHandle);
    return BufferedTransmitResult::NotRegistered;
  }
}

void DOService::RegisterGlobalDODiscovery(IDODiscovererGlobal* discoverer)
{
  RegisterGlobalDODiscovery(discoverer, IBDefaults::DOs::ConnectionType);
}
void DOService::RegisterGlobalDODiscovery(IDODiscovererGlobal* discoverer, const std::string& connectionType)
{
  mDODiscoverers[connectionType].push_back(discoverer);
}

void DOService::UnregisterGlobalDODiscovery()
{
  UnregisterGlobalDODiscovery(IBDefaults::DOs::ConnectionType);
}
void DOService::UnregisterGlobalDODiscovery(const std::string& connectionType)
{
  mDODiscoverers.erase(connectionType);
}

} // namespace NDistSimIB