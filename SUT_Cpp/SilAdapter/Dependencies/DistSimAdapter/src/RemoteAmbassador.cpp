/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/
#pragma warning(push)
#if defined(_WIN32)
#pragma warning(disable : 26812)
#endif

#include "RemoteAmbassador.h"
#include "RTFederateFOM.inc"
#include "UUIDT4.h"

#include <iostream>
#include <chrono>


namespace NDistSimIB {

// =========================================
// Constructor: Creates the subscriber services
  RemoteAmbassador::RemoteAmbassador(const IBConfig& config) :
    mConfig(config)
{
  // Persistent members
  mEventNotificationSet = false;
  mSignaledWithMultipleCallback = 0;
  mEvokeLoopCounter = 0;
  mTotalCallbacks = 0;
  mSignaledWithSingleCallback = 0;
  mManualSignalCounter = 0;
  mWaitForMeasurementInit = config.WaitForMeasurementStart;

  if (config.InjectFOM.empty())
    mFomModule = string2wstring(std::string("data:text/plain,") + RTFederateFOM);
  else
    mFomModule = string2wstring(std::string("data:text/plain,") + config.InjectFOM);

  
  ResetRemoteAmbassador();
  ResetFederationMemory();
  
  mInternalTimingService = std::shared_ptr<TimingService>(new TimingService(*this));

  //mRegisteredServices[Service::Timing] = mInternalTimingService.get();
}



RemoteAmbassador::~RemoteAmbassador()
{
  DebugPrintf(SDebug, "%s\n", __FUNCTION__);

  UnSetNotificationHandle();
}

// =========================================
// General service registration
void RemoteAmbassador::RegisterService(Service service)
{
  if (mState != AmbassadorState::Offline)
  {
    Error("Services have to be registered offline.\n");
    throw;
  }

  if (service == Service::DistributedObjects)
  {
    auto where = mRegisteredServices.find(service);
    if (where == mRegisteredServices.end())
    {
      DebugPrintf(SDebug,"Registered Distributed Objects Service.\n");
      mInternalDOService = CreateDOService(*this);
      mRegisteredServices[service] = mInternalDOService.get();
    }
    else
    {
      Error("Distributed Objects Service already registered.\n");
      throw;
    }
  } 
  else if (service == Service::BusMessages)
  {
    auto where = mRegisteredServices.find(service);
    if (where == mRegisteredServices.end())
    {
      DebugPrintf(SDebug, "Registered Bus Messages Service.\n");
      mInternalBusService = CreateBusService(*this);
      mRegisteredServices[service] = mInternalBusService.get();
    }
    else
    {
      Error("Bus Message Service already registered.\n");
      throw;
    }
  }
  else if (service == Service::TextLog)
  {
    auto where = mRegisteredServices.find(service);
    if (where == mRegisteredServices.end())
    {
      DebugPrintf(SDebug, "Registered Text Log Service.\n");
      mInternalTextLogService = CreateTextLogService(*this);
      mRegisteredServices[service] = mInternalTextLogService.get();
    }
    else
    {
      Error("Text Log Service already registered.\n");
      throw;
    }
  }
  //bkd: Always register timing service
  //else if (service == Service::Timing)
  //{
  //  auto where = mRegisteredServices.find(service);
  //  if (where == mRegisteredServices.end())
  //  {
  //    DebugPrintf(SDebug, "Registered Timing Service.\n");
  //    mInternalTimingService = CreateTimingService(*this);
  //    mRegisteredServices[service] = mInternalTimingService.get();
  //  }
  //  else
  //  {
  //    Error("Timing Service already registered.\n");
  //    throw;
  //  }
  //}
  // NEW SERVICE REQUIRES IMPLEMENTATION HERE
}

void RemoteAmbassador::UnregisterService(Service service)
{
  if (service == Service::DistributedObjects)
  {
    auto where = mRegisteredServices.find(service);
    if (where != mRegisteredServices.end())
    {
      DebugPrintf(SDebug,"Unregistered Distributed Objects Service.\n");
      mInternalDOService.reset();
      mRegisteredServices.erase(service);
    }
    else
    {
      Error("Distributed Objects Service not registered yet.\n");
    }
  } 
  else if (service == Service::BusMessages)
  {
    auto where = mRegisteredServices.find(service);
    if (where != mRegisteredServices.end())
    {
      DebugPrintf(SDebug,"Unregistered Bus Messages Service.\n");
      mInternalBusService.reset();
      mRegisteredServices.erase(service);
    }
    else
    {
      Error("Bus Messages Service not registered yet.\n");
    }
  }
  else if (service == Service::TextLog)
  {
    auto where = mRegisteredServices.find(service);
    if (where != mRegisteredServices.end())
    {
      DebugPrintf(SDebug, "Unregistered Text Log Service.\n");
      mInternalTextLogService.reset();
      mRegisteredServices.erase(service);
    }
    else
    {
      Error("Text Log Service not registered yet.\n");
    }
  }   /*else if (service == Service::Timing)
  {
    auto where = mRegisteredServices.find(service);
    if (where != mRegisteredServices.end())
    {
      DebugPrintf(SDebug, "Unregistered Timing Service.\n");
      mInternalTimingService.reset();
      mRegisteredServices.erase(service);
    }
    else
    {
      Error("Timing Service not registered yet.\n");
    }
  }*/
  // NEW SERVICE REQUIRES IMPLEMENTATION HERE
}

// =========================================
// Service registration InteractionsClasses
void RemoteAmbassador::subscribeInteractionClass(rti::InteractionClassHandle theInteraction, AbstractInteractionClassSubscriber* subscriber)
{
  mRTIAmb->subscribeInteractionClass(theInteraction);
  mInteractionClassSubscriber[theInteraction] = subscriber;
}
void RemoteAmbassador::subscribeInteractionClassWithFilter(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& filterValues, AbstractInteractionClassSubscriber* subscriber)
{
  mRTIAmb->subscribeInteractionClassWithFilter(theInteraction, filterValues);
  mInteractionClassSubscriber[theInteraction] = subscriber;
  SendManualSignal();
}
void RemoteAmbassador::unsubscribeInteractionClass(rti::InteractionClassHandle theInteraction, AbstractInteractionClassSubscriber* subscriber)
{
  mRTIAmb->unsubscribeInteractionClass(theInteraction);
  mInteractionClassSubscriber.erase(theInteraction);
}

void RemoteAmbassador::unsubscribeInteractionClassWithFilter(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& filterValues, AbstractInteractionClassSubscriber* subscriber)
{
  mRTIAmb->unsubscribeInteractionClassWithFilter(theInteraction, filterValues);
  // NOT: mInteractionClassSubscriber.erase(theInteraction);
}

// =========================================
// Service registration ObjectClasses
void RemoteAmbassador::subscribeObjectClassAttributes(rti::ObjectClassHandle theClass, rti::AttributeHandleSet const& attributeList, AbstractObjectClassSubscriber* subscriber)
{
  mRTIAmb->subscribeObjectClassAttributes(theClass, attributeList);
  mObjectClassSubscriber[theClass] = subscriber;
}
void RemoteAmbassador::unsubscribeObjectClass(rti::ObjectClassHandle theClass, AbstractObjectClassSubscriber* subscriber)
{
  mRTIAmb->unsubscribeObjectClass(theClass);
  mObjectClassSubscriber.erase(theClass);
}

// =========================================
// Service registration Object attribute request providers
void RemoteAmbassador::registerAttributeRequestProvider(rti::ObjectClassHandle theClass, AbstractAttributeRequestProvider* provider)
{
  mAttributeRequestProvider[theClass] = provider;
}
void RemoteAmbassador::unregisterAttributeRequestProvider(rti::ObjectClassHandle theClass, AbstractAttributeRequestProvider* provider)
{
  mAttributeRequestProvider.erase(theClass);
}

// =========================================
// State handling
void RemoteAmbassador::ChangeAmbassadorState(AmbassadorState newState)
{
  DebugPrintf(SDebug,"Ambassador state flow: %s -> %s\n", to_string(mState).c_str(), to_string(newState).c_str());

  //// State change triggers
  //if (mState != newState)
  //{
  //  if (newState == AmbassadorState::) 
  //  {
  //  } 
  //}

  mState = newState;
}

std::string to_string(RemoteAmbassador::AmbassadorState state)
{
  static const char* AmbassadorStateNames[] =
  {
      "Offline",
      "AmbassadorAvailable",
      "ScanForFederationPrefix",
      "CreateFederation",
      "JoinFederation",
      "FoundFederation",
      "JoinedFederation",
      "InitFederate",
      "WaitForMeasurementInit",
      "Online",
      "ReceivedMeasurementStop",
      "ReceivedMeasurementInit",
      "FederationSynchronized",
      "ConnectionLost",
  };
  return AmbassadorStateNames[static_cast<uint8_t>(state)];
}

// =========================================
// Should be called multiple times until connection is complete
void RemoteAmbassador::Connect()
{
  // AmbassadorState changes:
  //   Connect()
  //   Disconnect()                      -> Offline
  //   ReceivedMeasurementStop()         -> ReceivedMeasurementStop
  //   ReceivedMeasurementInit()         -> ReceivedMeasurementInit
  //   connectionLost()                  -> ConnectionLost
  //   DisconnectRTINode()               -> Offline;

  // Lost connection to rtinode  
  if (mState == AmbassadorState::ConnectionLost)
  {
    HandleConnectionLost();
    ChangeAmbassadorState(AmbassadorState::Offline);
  }

  // Creates the RTIAmbassador
  if (mState == AmbassadorState::Offline)
  {
    if (CreateRTIAmbassador())
    {
      ChangeAmbassadorState(AmbassadorState::AmbassadorAvailable);
    }
  }

  if (mState == AmbassadorState::AmbassadorAvailable)
  {
    if (ConnectToRTINode())
    {
      ChangeAmbassadorState(AmbassadorState::ScanForFederationPrefix);
    }
  }

  if (mState == AmbassadorState::ReceivedMeasurementStop)
  {
    ResetOnMeasurementStop();
    ChangeAmbassadorState(AmbassadorState::ScanForFederationPrefix);
  }

  if (mState == AmbassadorState::ScanForFederationPrefix)
  {
    std::string latestFederationName;
    bool foundFederation = GetLatestFederation(/*out*/ latestFederationName);
    DebugPrintf(SDebug, "GetLatestFederation: foundFederation = %d, latestFederationName = %s ...\n", foundFederation, latestFederationName.c_str());

    UpdateFederationNameAndIndex(foundFederation, latestFederationName);
    // The result of UpdateFederationNameAndIndex could be that we only found the last federation.
    // In this case, the federation index got increased and we want to create the federation (join wouldn't be possible).

    ChangeAmbassadorState(AmbassadorState::CreateFederation);
  }

  if (mState == AmbassadorState::CreateFederation)
  {
    mCreateFederationResult = CreateFederation();
    if (mCreateFederationResult != CreateFederationResult::Error)
    {
      ChangeAmbassadorState(AmbassadorState::JoinFederation);
    }
  }

  if (mState == AmbassadorState::JoinFederation)
  {
    // Now join it
    if (JoinFederation())
    {
      ChangeAmbassadorState(AmbassadorState::JoinedFederation);
    }
  }

  if (mState == AmbassadorState::JoinedFederation)
  {
    if (mCreateFederationResult == CreateFederationResult::CreatedFederation)
    {
      RegisterSyncPoints();
      ChangeAmbassadorState(AmbassadorState::InitFederate);
    }
    else
    {
      // Retrieve all announced sync points
      while (mRTIAmb->evokeCallback(RTIConstants::kDefaultWaitForSyncPointsTimeout)) {}

      // if we got here, then no measurement init was discovered (~ init cannot be reached)
      if (!mIsAnnouncedInitializedSyncPoint)
      {
        if (mIsAnnouncedSynchronizedSyncPoint || mIsAnnouncedStoppedSyncPoint)
        {
          // We didn't see the init sync point, but any other -> Late joining 
          HandleLateJoining();
          ChangeAmbassadorState(AmbassadorState::InitFederate);
        }
        else
        {
          // We didn't see any sync point -> Sanitize dead federation
          HandleDeadFederation();
          // Got new name from HandleDeadFederation, try to create/join again
          ChangeAmbassadorState(AmbassadorState::CreateFederation);
        }
      }
      else
      {
        // Joining fresh federation
        ChangeAmbassadorState(AmbassadorState::InitFederate);
      }
    }
  }

  if (mState == AmbassadorState::InitFederate)
  {
    // Set enableAsynchronousDelivery and enableAttributeRelevanceAdvisorySwitch
    SetupRTIAmbassador();

    // Define interaction handles of the RemoteAmbassador (Init/Stop IA)
    InitializeHandles();

    // Subscribe to IAs of the RemoteAmbassador (Init/Stop IA)
    PublishSubscribe();

    // Call OnJoined() on all services.
    InitializeServices();

    ProcessServiceBuffers();

    if (mWaitForMeasurementInit)
    {
      ChangeAmbassadorState(AmbassadorState::WaitForMeasurementInit);
    }
    else
    {
      ChangeAmbassadorState(AmbassadorState::Online);
    }
  }

  // 'Waiting' federates wait for the measurement init IA
  if (mWaitForMeasurementInit && mState == AmbassadorState::WaitForMeasurementInit)
  {

    while (mState != AmbassadorState::ReceivedMeasurementInit)
    {
      mRTIAmb->evokeCallback(RTIConstants::kDefaultEvokeTimeout);
    }
    // 'Waiting' federates also wait for the 'initialized' sync point for a synchronized start
    try
    {
      mRTIAmb->synchronizationPointAchieved(RTISyncPointNames::kInitializedSyncPointName);
      mRTIAmb->synchronizationPointAchieved(RTISyncPointNames::kSynchronizeSysvarsSyncPointName);
    }
    catch (const rti::Exception & e)
    {
      Error("%s: %ls\n", __FUNCTION__, e.what().c_str());
      return;
    }
    while (mState != AmbassadorState::FederationSynchronized)
    {
      mRTIAmb->evokeCallback(RTIConstants::kDefaultEvokeTimeout);
    }

    ChangeAmbassadorState(AmbassadorState::Online);
  }

}

bool RemoteAmbassador::CreateRTIAmbassador()
{
  try
  {
    rti::RTIambassadorFactory rtiAmbassadorfactory;
    mRTIAmb = rtiAmbassadorfactory.createRTIambassador();
    mRTIAmb->setConnectWaitTimeout(mConfig.ConnectionTimeoutMS);
    mRTIAmb->setOperationWaitTimeout(mConfig.ConnectionTimeoutMS);
    DebugPrintf(SInfo,"Created the RTI ambassador, waiting for rtinode...\n");
    return true;
  }
  catch (const rti::RTIinternalError & e)
  {
    //TODO bkd: When does createRTIambassador fail?
    Error("%s: could not create RTI ambassador; rti exception: '%ls'\n", __FUNCTION__, e.what().c_str());
    (void)e;
    throw;
  }

  return false;
}

bool RemoteAmbassador::ConnectToRTINode()
{
  try
  {
    DebugPrintf(SInfo, "Calling mRTIAmb->connect ...\n");
    mRTIAmb->connect(*this, rti::HLA_EVOKED, string2wstring(mConfig.RtinodeAddress));
    SettleResolvableWarning(ResolvableWarning::NoRtinodeConnection, mAlreadyNotified, "Connected to rtinode at '%s'\n", mConfig.RtinodeAddress.c_str());
    DebugPrintf(SInfo, "Connected to rtinode, waiting for federation...\n");
    return true;
  }
  catch (const rti::Exception & e)
  {
    //DebugPrintf(SInfo, "Could not connect to rtinode; Exception: %S\n", e.what().c_str());
    RaiseResolvableWarning(ResolvableWarning::NoRtinodeConnection, mAlreadyNotified,  "Could not connect to rtinode at '%s', retrying...\n", mConfig.RtinodeAddress.c_str());

    (void)e;
  }
  return false;
}

bool RemoteAmbassador::GetLatestFederation(std::string& federationName)
{
  // if none is available, request all federation executions from the RTI node and check for any candidates
  // a candidate federation starts with the same federation name as the one provided in this call
  mIsListOfFederationExecutionsAvailable = false;
  mRTIAmb->listFederationExecutions();
  while (!mIsListOfFederationExecutionsAvailable)
  {
    mRTIAmb->evokeCallback(0.1);
  }

  DebugPrintf(SDebug, "Found federations:\n");
  for (auto item : mFederationExecutionInformationVector)
  {
    DebugPrintf(SDebug, "   %S\n", item.federationExecutionName.c_str());
  }

  std::reverse(mFederationExecutionInformationVector.begin(), mFederationExecutionInformationVector.end());
  auto federationBaseName_w = string2wstring(mConfig.FederationNamePrefix);
  for (rti::FederationExecutionInformation fedExInformation : mFederationExecutionInformationVector)
  {
    auto findPos = fedExInformation.federationExecutionName.rfind(federationBaseName_w, 0);
    auto latestMatchingFederation = wstring2string(fedExInformation.federationExecutionName);
    if (findPos != std::wstring::npos)
    {
      // found a federation - store current name
      federationName = latestMatchingFederation; // e.g., canoe-1
      return true;
    }
  }
  federationName = "";
  return false;
}

void RemoteAmbassador::UpdateFederationNameAndIndex(bool foundFederation, std::string latestFederationName)
{
  // check, if the current federation equals the most recent federation
  if (!foundFederation)
  {
    if (!mCurrentFederationName.empty()) // Do nothing if initial run
    {
      // if it is, simply increase the measurement index and continue
      if (mFederationIndex == -1)
      {
        mFederationIndex = 1;
      }
      else
      {
        ++mFederationIndex;
      }
    }
  }
  else
  {
    if (latestFederationName == mCurrentFederationName)
    {
      // we found our own federation (e.g. was not destroyed after measurement stop) -> increase index
      if (mFederationIndex == -1)
      {
        mFederationIndex = 1;
      }
      else
      {
        ++mFederationIndex;
      }
    }
    else
    {
      // otherwise use the most recently created federation
      auto measurementIndex = GetMeasurementIndexOfFederation(latestFederationName);
      try
      {

        if (measurementIndex.empty())
        {
          mFederationIndex = -1;
        }
        else
        {
          mFederationIndex = std::stoi(measurementIndex);
        }
      }
      catch (...)
      {
        // invalid index detected
        mFederationIndex = -1;
      }
    }
  }

  mCurrentFederationName = mConfig.FederationNamePrefix;
  if (mFederationIndex != -1)
  {
    mCurrentFederationName += "-" + std::to_string(mFederationIndex);
  }

  DebugPrintf(SDebug, "%s: mCurrentFederationName = %s, mFederationIndex = %d\n", __FUNCTION__, mCurrentFederationName.c_str(), mFederationIndex);
}

std::string RemoteAmbassador::GetMeasurementIndexOfFederation(const std::string federationName)
{
  if (mConfig.FederationNamePrefix == federationName)
  {
    return "";
  }
  else
  {
    return federationName.substr(mConfig.FederationNamePrefix.length() + 1);
  }
}

bool RemoteAmbassador::JoinFederation()
{
  std::string trialFederateName = mConfig.FederateName;
  
  std::vector<std::wstring> additionalFom;
  if (mCreateFederationResult == CreateFederationResult::FederationAlreadyExists) {
    // Federation already existed: Add our FOM on join
    additionalFom.emplace_back(mFomModule);
  }

  try
  {
    DebugPrintf(SInfo, "Joining federation '%s' as '%s'\n", mCurrentFederationName.c_str(), trialFederateName.c_str());
    mFederateHandle = mRTIAmb->joinFederationExecution(string2wstring(trialFederateName), string2wstring(mConfig.FederateType), string2wstring(mCurrentFederationName), additionalFom);
  }
  catch (const rti::OperationTimeout & e) // Timeout might be non-severe, can be caused while debugging -> Retry join
  {
    Error("Could not join federation '%s': %S\n", mCurrentFederationName.c_str(), e.what().c_str());
    (void)e;
    return false;
  }
  catch (const rti::InconsistentFDD & e) // Inconsistent FOM is severe
  {
    Error("Could not join federation '%s': %S\n", mCurrentFederationName.c_str(), e.what().c_str());
    (void)e;
    throw;
    return false;
  }
  catch (const rti::FederateNameAlreadyInUse & e) // Federate name in use is severe
  {
    (void)e;
    Error("Could not join federation '%s': %S\n", mCurrentFederationName.c_str(), e.what().c_str());
    throw;
    return false;
  }
  catch (const rti::Exception & e) // Other exceptions are severe
  {
    Error("Could not join federation '%s': %S\n", mCurrentFederationName.c_str(), e.what().c_str());
    (void)e;
    throw;
    return false;
  }

  mCurrentFederateName = trialFederateName;

  DebugPrintf(SInfo, "Joined federation '%s' as '%s'; FederationHandle = %S\n", mCurrentFederationName.c_str(), mCurrentFederateName.c_str(), mFederateHandle.toString().c_str());
  ProcessFederationControlCallbacks(IDistSimIB::FederationControlEvent::Join, mCurrentFederationName);
  return true;
}

RemoteAmbassador::CreateFederationResult RemoteAmbassador::CreateFederation()
{
  // Try creating the federation; may except non-severe with FederationExecutionAlreadyExists
  try
  {
    DebugPrintf(SInfo, "Trying to create federation '%s'...\n", mCurrentFederationName.c_str());
    mRTIAmb->createFederationExecution(string2wstring(mCurrentFederationName), mFomModule, L"HLAinteger64Time");
  }
  catch (const rti::FederationExecutionAlreadyExists&) // Federation name in use is non-severe -> Able to join
  {
    DebugPrintf(SInfo,"...didn't create federation, it already existed\n");
    return CreateFederationResult::FederationAlreadyExists;
  }
  catch (const rti::OperationTimeout & e) // Timeout might be non-severe, can be caused while debugging -> Retry create
  {
    Error("Could not create federation '%s': %S\n", mCurrentFederationName.c_str(), e.what().c_str());
    (void)e;
    return CreateFederationResult::Error;
  }
  catch (const rti::Exception & e) // Other exceptions are severe
  {
    Error("Could not create federation '%s': %S\n", mCurrentFederationName.c_str(), e.what().c_str());
    (void)e;
    throw;
    return CreateFederationResult::Error;
  }

  DebugPrintf(SInfo, "...created successfully\n");
  ProcessFederationControlCallbacks(IDistSimIB::FederationControlEvent::Create, mCurrentFederationName);
  return CreateFederationResult::CreatedFederation;
}

void RemoteAmbassador::SetupRTIAmbassador()
{
  assert(mState == AmbassadorState::InitFederate);

  try
  {
    mRTIAmb->enableAsynchronousDelivery();
  }
  catch (const rti::Exception & e)
  {
    Error("%s: Could not enable asynchronous delivery: %S\n", __FUNCTION__, e.what().c_str());
    (void)e;
    throw;
  }

  try
  {
    mRTIAmb->enableAttributeRelevanceAdvisorySwitch();
  }
  catch (const rti::Exception & e)
  {
    Error("%s: Could not enable attribute relevance advisory switch: %S\n", __FUNCTION__, e.what().c_str());
    (void)e;
    throw;
  }
}
void RemoteAmbassador::RegisterSyncPoints()
{
  DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  mIsAnnouncedInitializedSyncPoint = false;
  mIsAnnouncedSynchronizedSyncPoint = false;
  mIsAnnouncedStoppedSyncPoint = false;
  try
  {
    mRTIAmb->registerFederationSynchronizationPoint(RTISyncPointNames::kInitializedSyncPointName, rti::VariableLengthData());
    mRTIAmb->registerFederationSynchronizationPoint(RTISyncPointNames::kSynchronizeSysvarsSyncPointName, rti::VariableLengthData());
    mRTIAmb->registerFederationSynchronizationPoint(RTISyncPointNames::kStoppedSyncPointName, rti::VariableLengthData());

    while (!mIsAnnouncedInitializedSyncPoint && !mIsAnnouncedSynchronizedSyncPoint && !mIsAnnouncedStoppedSyncPoint)
    {
      mRTIAmb->evokeCallback(RTIConstants::kDefaultWaitForSyncPointsTimeout);
    }
  }
  catch (const rti::Exception & e)
  {
    (void)e;
    Error("%s: could not register synchronization point: %ls\n", __FUNCTION__, e.what().c_str());
    throw;
  }
}

// ========================================
// RemoteAbmassador RTI handles
void RemoteAmbassador::InitializeHandles()
{
  mIAHandleMeasurementInit = mRTIAmb->getInteractionClassHandle(FOMInteractions::kMeasurementInit);
  mMeasurementInitParameter = mRTIAmb->getParameterHandle(mIAHandleMeasurementInit, FOMInteractions::kMeasurementInit_Param);
  mIAHandleMeasurementStop = mRTIAmb->getInteractionClassHandle(FOMInteractions::kMeasurementStop);
  mMeasurementStopParameter = mRTIAmb->getParameterHandle(mIAHandleMeasurementStop, FOMInteractions::kMeasurementStop_Param_NextFederationSuffix);
}

// ========================================
// RemoteAbmassador subscriptions
void RemoteAmbassador::PublishSubscribe()
{
  try
  {
    mRTIAmb->subscribeInteractionClass(mIAHandleMeasurementInit);
    mRTIAmb->publishInteractionClass(mIAHandleMeasurementInit);
    mRTIAmb->subscribeInteractionClass(mIAHandleMeasurementStop);
    mRTIAmb->publishInteractionClass(mIAHandleMeasurementStop);
    mRTIAmb->setInteractionClassDeliverToSelf(mIAHandleMeasurementStop, true);
    mRTIAmb->setInteractionClassDeliverToSelf(mIAHandleMeasurementInit, true);
  }
  catch (const rti::NotConnected & e)
  {
    Error("%s: not connected: %S\n", __FUNCTION__, e.what().c_str());
    (void)e;
    throw;
  }
  catch (const rti::NameNotFound & e)
  {
    Error("%s: name not found: %S\n", __FUNCTION__, e.what().c_str());
    (void)e;
    throw;
  }
  catch (const rti::Exception & e)
  {
    Error("%s: %S\n", __FUNCTION__, e.what().c_str());
    (void)e;
    throw;
  }
}

// ========================================
// Service RTI handles and subscriptions
void RemoteAmbassador::InitializeServices()
{
  //DebugPrintf(SDebug, "%s\n", __FUNCTION__);

  // Timing Service first
  mInternalTimingService->OnJoined();

  for (auto const& iter : mRegisteredServices)
  {
    iter.second->OnJoined();
  }
}

// ========================================
// Service buffers
void RemoteAmbassador::ProcessServiceBuffers()
{
  //assert(mState == AmbassadorState::Online);
  assert(mState == RemoteAmbassador::AmbassadorState::Online || mState == RemoteAmbassador::AmbassadorState::InitFederate);

  //DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  bool triggerEventNotification = false;

  // Timing Service first
  triggerEventNotification |= mInternalTimingService->ProcessBuffers();

  for (auto const& iter : mRegisteredServices)
  {
    triggerEventNotification |= iter.second->ProcessBuffers();
  }

  if (triggerEventNotification)
  {
    SendManualSignal();
  }
}

// =========================================
// Event notification
void RemoteAmbassador::SetNotificationHandle() {
  //DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  assert(mState == AmbassadorState::Online);

  if (!mEventNotificationSet)
  {
    mEventNotificationSet = true;
    mEventNotification = std::unique_ptr<EventNotification>(new EventNotification());
    mRTIAmb->setNotificationHandle(mEventNotification.get());
    // Send an initial signal
    mEventNotification.get()->Signal();
  }
}

void RemoteAmbassador::UnSetNotificationHandle() {
  if (mEventNotificationSet)
  {
    DebugPrintf(SDebug, "%s\n", __FUNCTION__);
    mEventNotificationSet = false;
    mEventNotification->CloseEvent();
  }
}


void RemoteAmbassador::SendManualSignal()
{
  //DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  mManualSignalCounter++;
  if (mEventNotificationSet)
  {
    mEventNotification.get()->Signal();
  }
}

// =========================================
// Buffers and polling
void RemoteAmbassador::EvokeCallbacksPolled(double seconds)
{
  assert(mState == AmbassadorState::Online);
  EvokeCallbacks(seconds);
}

void RemoteAmbassador::EvokeCallbacksOnEvent()
{
  assert(mState == AmbassadorState::Online);
  int32_t timeout = -1;
  auto start = std::chrono::system_clock::now();

  if (!mInternalTimingService->IsTimeConstrained() && !mInternalTimingService->IsTimeRegulating() && mConfig.TimingInterpolationEnabled)
  {

    auto nextTimerDueTime = mInternalTimingService->GetNextTimerDueTime();
    if (nextTimerDueTime <= mInternalTimingService->GetSimTime())
    {
        mInternalTimingService->ProcessUserTimers();
    }

    auto duration = nextTimerDueTime - mInternalTimingService->GetSimTime();
    timeout = (int32_t)(mInternalTimingService->GetAnimationFactor() * duration.count() / 1000000);

    DebugPrintf(SDebug, "%s WAIT for %f * (%f - %f) = %d ...\n", __FUNCTION__, 
      mInternalTimingService->GetAnimationFactor(), 
      nextTimerDueTime.count() * 1e-9,
      mInternalTimingService->GetSimTime().count() * 1e-9,
      timeout);

    //std::cout << "WAIT for  "<< mInternalTimingService->GetAnimationFactor() << "*(" 
    //<< nextTimerDueTime.count() * 1e-9 << "-" << mInternalTimingService->GetSimTime().count() * 1e-9 << ")=" << timeout << "...";

  }
  //mInternalTimingService->GetAnimatio
  //// FIXME Mkb: Hier muss wie in TimingService::RunTo folgendes ergänzt werden
  // - Im Falle !TC:
  //   - Nächsten fälligen Timer bestimmen
  //   - Wartezeit mit Schätzung des Animationsfaktor multiplizieren
  //     - Nur so lange warten 
  //   - Zeit messen, Zeit hochdrehen, Timer abarbeiten
  auto result = mEventNotification->WaitForSingleEvent(timeout);
  if (result == EventNotification::WaitResult::Success)
  {
    EvokeCallbacks(0.0);
  }
  else if (result == EventNotification::WaitResult::Timeout && !mInternalTimingService->IsTimeConstrained() && !mInternalTimingService->IsTimeRegulating())
  {
    if (!mInternalTimingService->IsFederateTooFast())
    {
      std::chrono::nanoseconds spent = std::chrono::system_clock::now() - start;
      spent = std::chrono::nanoseconds((long long)(spent.count() / mInternalTimingService->GetAnimationFactor()));
      DebugPrintf(SDebug, "  .... spent= %lld\n", spent.count() * 1e-9);
      // std::cout << "  .... spent=" << spent.count() * 1e-9 << std::endl;
      // FIXME Mkb: increase time only after measurement started
      
      // Bkd: Shouldn't A TR/NTC only advance the time VIA TAR? This only sets the internal logicalTime
      mInternalTimingService->SetNewTime((mInternalTimingService->GetSimTime() + spent).count());
    }
  }
  else
  {
    Error("WaitForSingleEvent != Success\n");
  }
}

void RemoteAmbassador::EvokeCallbacks(double seconds)
{
  const std::lock_guard<std::mutex> lock(mMutEvokeCallbacks);
  mEvokeLoopCounter++;
  bool firstRun = true;
  bool callbackAvail = true;

  // 1.0 means that we have 1s for the first run of the evokeCallback Loop (until mStartTimeUserCode is available)
  double elapsedSecUserCode = 1.0;
  if (mStartTimeUserCodeAvailable)
  {
    elapsedSecUserCode = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - mStartTimeUserCode).count();
  }

  auto start = std::chrono::steady_clock::now();

  while (callbackAvail)
  {
    callbackAvail = mRTIAmb->evokeCallback(seconds);

    mTotalCallbacks++;

    if (!callbackAvail)
    {
      if (firstRun)
        mSignaledWithSingleCallback++;
    }
    else
    {
      double elapsedSecEvokeCallbackLoop = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count();
      if (elapsedSecEvokeCallbackLoop > 1.0 && elapsedSecEvokeCallbackLoop > elapsedSecUserCode)
      {
        DebugPrintf(SDebug, "%s: Processing incoming HLA Events takes too much time -> Bailout\n", __FUNCTION__);
        break;
      }

      if (firstRun)
        mSignaledWithMultipleCallback++;

      firstRun = false;
    }
  }

  mStartTimeUserCode = std::chrono::steady_clock::now();
  mStartTimeUserCodeAvailable = true;

  // Simulation control callbacks
  for (auto simEventIt = mSimulationEventToProcess.begin(); simEventIt != mSimulationEventToProcess.end();)
  {
    ProcessSimulationControlCallbacks(*simEventIt);
    simEventIt = mSimulationEventToProcess.erase(simEventIt);
  }

  if (mState == AmbassadorState::Online)
  {
    ProcessServiceBuffers();
  }
  
  mInternalTimingService->TimeRequest();

}

RemoteAmbassador::AmbassadorState RemoteAmbassador::GetState() const
{
  return mState;
}

// ========================================
// Special simulation state handling
bool RemoteAmbassador::HandleDeadFederation()
{

  DebugPrintf(SWarn, "%s: Detected dead simulation upon join. Sanitizing....\n", __FUNCTION__);
  bool success = ResignAndDestroyFederation();

  if (mFederationIndex == -1)
  {
    mFederationIndex = 1;
  }
  else
  {
    ++mFederationIndex;
  }
  mCurrentFederationName = mConfig.FederationNamePrefix + "-" + std::to_string(mFederationIndex);

  return success;
}

bool RemoteAmbassador::HandleLateJoining()
{
  DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  return true;
}

// ========================================
// Resign, destroy, reset and disconnect 
void RemoteAmbassador::Disconnect()
{
  //DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  if (mState != RemoteAmbassador::AmbassadorState::Offline)
  {
    ResignAndDestroyFederation();
  }

  ResetServices();
  ResetRemoteAmbassador();
  ResetFederationMemory();
  DisconnectRTINode();
}

void RemoteAmbassador::DisconnectRTINode()
{
  mRTIAmb->disconnect();
  ChangeAmbassadorState(AmbassadorState::Offline);
}

// ========================================
// Simulation control

void RemoteAmbassador::ProcessSimulationControlCallbacks(IDistSimIB::SimulationControlEvent SimulationEvent)
{
  auto callbacksByHandle = mSimulationControlCallbacks.find(SimulationEvent);
  if (callbacksByHandle != mSimulationControlCallbacks.end())
  {
    for (const auto& pair : callbacksByHandle->second)
    {
      pair.second();
    }
  }
}

// ========================================
// Federation control

void RemoteAmbassador::ProcessFederationControlCallbacks(IDistSimIB::FederationControlEvent federationEvent, const std::string& federationName)
{
  auto callbacksByHandle = mFederationControlCallbacks.find(federationEvent);
  if (callbacksByHandle != mFederationControlCallbacks.end())
  {
    for (const auto& pair : callbacksByHandle->second)
    {
      pair.second(federationName);
    }
  }
}

void RemoteAmbassador::HandleConnectionLost()
{
  mRTIAmb->shutdown();
  mRTIAmb.reset();

  // Invalidate eventNotification
  if (mEventNotificationSet)
  {
    mEventNotification->CloseEvent();
  }
  mEventNotificationSet = false;
  
  ResetServices();
  ResetRemoteAmbassador();
  ResetFederationMemory();

}

void RemoteAmbassador::ResetServices()
{
  
  // Timing Service first
  mInternalTimingService->Reset();

  // Services
  for (auto const& iter : mRegisteredServices)
  {
    iter.second->Reset();
  }
}

void RemoteAmbassador::ResetRemoteAmbassador()
{
  mIsListOfFederationExecutionsAvailable = false;
  mIsAnnouncedInitializedSyncPoint = mIsAnnouncedSynchronizedSyncPoint = mIsAnnouncedStoppedSyncPoint = false;
  mStartTimeUserCodeAvailable = false;
  mMeasurementInitSent = mMeasurementStopSent = false;

  mSimulationEventToProcess.clear();
  mInstanceNameReservations.clear();
  mCreateFederationResult = CreateFederationResult::Error;
}

void RemoteAmbassador::ResetFederationMemory()
{
  mFederationIndex = -1;
  mCurrentFederationName = "";
}

void RemoteAmbassador::ResetOnMeasurementStop()
{
  DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  ResignAndDestroyFederation();
  ResetServices();
  ResetRemoteAmbassador();
  // No ResetFederationMemory on a measurement stop

  uint32_t signalCounter = 0;
  if (mEventNotification != nullptr)
  {
    signalCounter = mEventNotification->mSignalCounter;
    mEventNotification->mSignalCounter = 0;
  }
  //DebugPrintf(SDebug, "EventNotifcation:\n");
  //DebugPrintf(SDebug, "   Signaled: %d\n", signalCounter);
  //DebugPrintf(SDebug, "   EvokeCallbackLoop: %d\n", mEvokeLoopCounter);
  //DebugPrintf(SDebug, "   TotalCallbacks: %d\n", mTotalCallbacks);
  //DebugPrintf(SDebug, "   SingleCallback: %d\n", mSignaledWithSingleCallback);
  //DebugPrintf(SDebug, "   MultipleCallbacks: %d\n", mSignaledWithMultipleCallback);
  //DebugPrintf(SDebug, "   ManualSignal: %d\n", mManualSignalCounter);
  //DebugPrintf(SDebug, "   Callbacks/EvokeLoop: %f\n", mTotalCallbacks * 1.0 / mEvokeLoopCounter);
  mEvokeLoopCounter = 0;
  mSignaledWithMultipleCallback = 0;
  mSignaledWithSingleCallback = 0;
  mManualSignalCounter = 0;
  mTotalCallbacks = 0;

}


bool RemoteAmbassador::ResignAndDestroyFederation()
{
  // Leaves the federation
  bool successResign = false;
  try
  {
    mRTIAmb->resignFederationExecution(rti::NO_ACTION);
    DebugPrintf(SInfo, "Resigned from Federation\n");
    successResign = true;
    ProcessFederationControlCallbacks(IDistSimIB::FederationControlEvent::Resign, mCurrentFederationName);
  }
  catch (const rti::Exception & e)
  {
    Error("%s: %S\n", __FUNCTION__, e.what().c_str());
    (void)e;
  }

  // Destroys the federation if this is the last federate
  bool successDestroy = false;
  try
  {
    mRTIAmb->destroyFederationExecution(string2wstring(mCurrentFederationName));
    DebugPrintf(SInfo, "Destroyed Federation %s\n", mCurrentFederationName.c_str());
    successDestroy = true;
    ProcessFederationControlCallbacks(IDistSimIB::FederationControlEvent::Destroy, mCurrentFederationName);
  }
  catch (const rti::Exception & e)
  {
    DebugPrintf(SInfo, "Did not destroy federation: %S\n", e.what().c_str());
    (void)e;
  }

  return successResign && successDestroy;
}

// ========================================
// CANoe measurement flow 
void RemoteAmbassador::SendMeasurementInit()
{
  DebugPrintf(SDebug, "%s\n", __FUNCTION__);

  // Allow sending once per measurement
  if (mMeasurementInitSent)
  {
    DebugPrintf(SWarn, "%s: Measurement init should only be sent once per measurement.\n", __FUNCTION__);
    return;
  }
  else
  {
    mMeasurementInitSent = true;
    rti::ParameterHandleValueMap parameters;
    parameters[mMeasurementInitParameter] = rti::VariableLengthData();
    mRTIAmb->sendInteraction(mIAHandleMeasurementInit, parameters, rti::HLAunicodeString(L"init").encode());
  }
}
void RemoteAmbassador::SendMeasurementStop()
{
  DebugPrintf(SDebug, "%s\n", __FUNCTION__);

  // Allow sending once per measurement
  if (mMeasurementStopSent)
  {
    DebugPrintf(SWarn, "%s: Measurement stop should only be sent once per measurement.\n", __FUNCTION__);
    return;
  }
  else
  {
    mMeasurementStopSent = true;
    rti::ParameterHandleValueMap parameters;
    std::wstring nextFedExSuffix = std::to_wstring(mFederationIndex);
    parameters[mMeasurementStopParameter] = rti::HLAunicodeString(nextFedExSuffix.c_str()).encode();
    mRTIAmb->sendInteraction(mIAHandleMeasurementStop, parameters, rti::HLAunicodeString(L"stop").encode());
  }
}

void RemoteAmbassador::OnReceiveIAMeasurementInit()
{

  DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  mSimulationEventToProcess.emplace(IDistSimIB::SimulationControlEvent::Start);

  if (mWaitForMeasurementInit)
  {
    // Only 'waiting' federates need to handle the ambassador state in the measurementInit IA
    if (mState == AmbassadorState::WaitForMeasurementInit)
      ChangeAmbassadorState(AmbassadorState::ReceivedMeasurementInit);
  }
  else
  {
    // we are not waiting for any sync points, but we need to confirm the initialization and synchronization sync points
    // once any simulation participant wants to start a measurement
    // achieve sync point without waiting for a confirmation (as we are not interested in the synchronization ourselves)
    mRTIAmb->synchronizationPointAchieved(RTISyncPointNames::kInitializedSyncPointName);
    mRTIAmb->synchronizationPointAchieved(RTISyncPointNames::kSynchronizeSysvarsSyncPointName);
  }
}

void RemoteAmbassador::OnReceiveIAMeasurementStop(const rti::ParameterHandleValueMap& theParameterValues)
{
  DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  mSimulationEventToProcess.emplace(IDistSimIB::SimulationControlEvent::Stop);

  mRTIAmb->synchronizationPointAchieved(RTISyncPointNames::kStoppedSyncPointName);

  if (mConfig.RejoinOnMeasurementStop)
  {
    ChangeAmbassadorState(AmbassadorState::ReceivedMeasurementStop);
  }
}

// =========================================
// Guarded getters for attributes/parameters

rti::ParameterHandle RemoteAmbassador::getParameterHandle(rti::InteractionClassHandle interactionClassHandle, std::wstring const& parameterName)
{
  rti::ParameterHandle p; // Default construction with invalid handle
  try
  {
    p = mRTIAmb->getParameterHandle(interactionClassHandle, parameterName);
  }
  catch (const rti::NameNotFound & e)
  {
    // This means that FOM and code basis using the parameter have diverged. 
    // Should only happen while testing the FOM merge (CTest-FOM-mismatch-interactions)
    Error("%s: %S\n", __FUNCTION__, e.what().c_str());
    (void)e;
  }
  catch (const rti::Exception & e)
  {
    Error("%s: %S\n", __FUNCTION__, e.what().c_str());
    (void)e;
    throw;
  }
  return p;
}

rti::AttributeHandle RemoteAmbassador::getAttributeHandle(rti::ObjectClassHandle objectClassHandle, std::wstring const& attributeName)
{
  rti::AttributeHandle h; // Default construction with invalid handle
  try
  {
    h = mRTIAmb->getAttributeHandle(objectClassHandle, attributeName);
  }
  catch (const rti::NameNotFound & e)
  {
    // This means that FOM and code basis using the attribute have diverged. 
    // Should only happen while testing the FOM merge (CTest-FOM-mismatch-objects)
    Error("%s: %S\n", __FUNCTION__, e.what().c_str()); 
    (void)e;
  }
  catch (const rti::Exception & e)
  {
    Error("%s: %S\n", __FUNCTION__, e.what().c_str());
    (void)e;
    throw;
  }
  return h;
}

// =========================================
// Guarded attribute/parameter map access

bool RemoteAmbassador::ReadAttribute(rti::VariableLengthData& dataReference, const rti::ObjectClassHandle& objectClass, const rti::AttributeHandleValueMap& attributeValues, const rti::AttributeHandle& handle)
{
  assert(objectClass.isValid());
  assert(handle.isValid());

  auto where_handle = attributeValues.find(handle);
  if (where_handle != attributeValues.end())
  {
    dataReference = (*where_handle).second;
    return true;
  }
  else
  {
    DebugPrintf(SWarn, "%s: Attribute '%S' for object class '%S' not found.\n", __FUNCTION__, mRTIAmb->getAttributeName(objectClass, handle).c_str(), mRTIAmb->getObjectClassName(objectClass).c_str());
    return false;
  }
}

bool RemoteAmbassador::ReadParameter(rti::VariableLengthData& dataReference, const rti::InteractionClassHandle& interactionClass, const rti::ParameterHandleValueMap& parameterValues, const rti::ParameterHandle& handle)
{
  assert(interactionClass.isValid());
  assert(handle.isValid());

  auto where_handle = parameterValues.find(handle);
  if (where_handle != parameterValues.end())
  {
    dataReference = (*where_handle).second;
    return true;
  }
  else
  {
    DebugPrintf(SWarn, "%s: Parameter '%S' for interaction class '%S' not found.\n", __FUNCTION__, mRTIAmb->getParameterName(interactionClass, handle).c_str(), mRTIAmb->getInteractionClassName(interactionClass).c_str());
    return false;
  }
}

// ========================================
// Object name reservation
void RemoteAmbassador::reserveObjectInstanceName(const std::wstring& name, InstanceNameReservationCallback callback)
{
  if (mInstanceNameReservations.insert(std::make_pair(name, callback)).second == false)
  {
    DebugPrintf(SDebug,"%s: name reservation for %ls already active\n", __FUNCTION__, name.c_str());
    callback(name, false);
    return;
  }
  mRTIAmb->reserveObjectInstanceName(name);
}

// ========================================
// OpenRTI callbacks

// ========================================
// Poll federations
void RemoteAmbassador::reportFederationExecutions(const rti::FederationExecutionInformationVector& federationExecutionInformationVector)
{
  mIsListOfFederationExecutionsAvailable = true;
  mFederationExecutionInformationVector = federationExecutionInformationVector;

  //mIsListOfFederationExecutionsAvailable = true;

  //bool foundFederation = false;
  //if (!federationExecutionInformationVector.empty())
  //{
  //  DebugPrintf(SDebug, "Found federations:\n");
  //  for (auto item : federationExecutionInformationVector)
  //  {
  //    DebugPrintf(SDebug, "   %S\n", item.federationExecutionName.c_str());
  //  }

  //  std::string latestFedName = wstring2string(federationExecutionInformationVector.back().federationExecutionName);

  //  // Iterate backwards to find the latest federation with the given prefix
  //  for (auto fedExInformation = federationExecutionInformationVector.rbegin(); fedExInformation != federationExecutionInformationVector.rend(); ++fedExInformation)
  //  {
  //    if (fedExInformation->federationExecutionName.rfind(string2wstring(mConfig.FederationNamePrefix), 0) != std::string::npos)
  //    {
  //      mNextFederationName = wstring2string(fedExInformation->federationExecutionName);
  //      DebugPrintf(SDebug, "Found federation %s\n", mNextFederationName.c_str());
  //      foundFederation = true;
  //      break;
  //    }
  //  }
  //}
  //if (!foundFederation)
  //{
  //  // If no federation is found, use the prefix to create a new one
  //  mNextFederationName = mConfig.FederationNamePrefix;
  //  DebugPrintf(SDebug, "No federation name found with prefix %s\n", mConfig.FederationNamePrefix.c_str());
  //}
}





void RemoteAmbassador::connectionLost(std::wstring const& faultDescription)
{
  ChangeAmbassadorState(AmbassadorState::ConnectionLost);
}

// ========================================
// Time management
void RemoteAmbassador::timeRegulationEnabled(rti::LogicalTime const& theFederateTime)
{
  mInternalTimingService->timeRegulationEnabled(theFederateTime);
}

void RemoteAmbassador::timeConstrainedEnabled(rti::LogicalTime const& theFederateTime)
{
  mInternalTimingService->timeConstrainedEnabled(theFederateTime);
}

void RemoteAmbassador::timeAdvanceGrant(rti::LogicalTime const& theTime)
{
  mInternalTimingService->timeAdvanceGrant(theTime);
  //DeliverDueEvents();
}

// ========================================
// Syncpoints
void RemoteAmbassador::synchronizationPointRegistrationSucceeded(std::wstring const& label)
{
}

void RemoteAmbassador::synchronizationPointRegistrationFailed(std::wstring const& label, rti::SynchronizationPointFailureReason reason)
{
}

void RemoteAmbassador::announceSynchronizationPoint(std::wstring const& label, rti::VariableLengthData const& theUserSuppliedTag)
{
  DebugPrintf(SDebug, "%s: %S\n", __FUNCTION__, label.c_str());
  // set sync point flags for known sync points
  if (label == RTISyncPointNames::kInitializedSyncPointName)
  {
    mIsAnnouncedInitializedSyncPoint = true;
  }
  else if (label == RTISyncPointNames::kSynchronizeSysvarsSyncPointName)
  {
    mIsAnnouncedSynchronizedSyncPoint = true;
  }
  else if (label == RTISyncPointNames::kStoppedSyncPointName)
  {
    mIsAnnouncedStoppedSyncPoint = true;
  }
  else 
  {
    // Immediately affirm all other sync points
    mRTIAmb->synchronizationPointAchieved(label);
  }
}

void RemoteAmbassador::federationSynchronized(std::wstring const& label, rti::FederateHandleSet const& failedToSyncSet)
{
  DebugPrintf(SDebug, "%s: %S\n", __FUNCTION__, label.c_str());
  if (mWaitForMeasurementInit && label == RTISyncPointNames::kInitializedSyncPointName)
  {
    ChangeAmbassadorState(AmbassadorState::FederationSynchronized);
  }

}

// ========================================
// Object name reservation
void RemoteAmbassador::objectInstanceNameReservationSucceeded(std::wstring const& theObjectInstanceName)
{
  auto iter = mInstanceNameReservations.find(theObjectInstanceName);
  assert(iter != mInstanceNameReservations.end());
  iter->second(iter->first, true);
  mInstanceNameReservations.erase(iter);
}

void RemoteAmbassador::objectInstanceNameReservationFailed(std::wstring const& theObjectInstanceName)
{
  auto iter = mInstanceNameReservations.find(theObjectInstanceName);
  assert(iter != mInstanceNameReservations.end());
  iter->second(iter->first, false);
  mInstanceNameReservations.erase(iter);
}

// ========================================
// Object discovery

void RemoteAmbassador::discoverObjectInstance(
  rti::ObjectInstanceHandle theObject,
  rti::ObjectClassHandle theObjectClass,
  std::wstring const& theObjectInstanceName,
  rti::FederateHandle producingFederate)
{
  discoverObjectInstance(theObject, theObjectClass, theObjectInstanceName);
}

void RemoteAmbassador::discoverObjectInstance(
  rti::ObjectInstanceHandle theObject,
  rti::ObjectClassHandle theObjectClass,
  std::wstring const& theObjectInstanceName)
{
  //DebugPrintf(SDebug,"%s: Object class = %ls, instance name = %ls\n", __FUNCTION__, mRTIAmb->getObjectClassName(theObjectClass).c_str(), mRTIAmb->getObjectInstanceName(theObject).c_str());

  auto iter = mObjectClassSubscriber.find(theObjectClass);
  if (iter != mObjectClassSubscriber.end())
  {
    iter->second->discoverObjectInstance(theObject, theObjectClass, theObjectInstanceName);
  }
  else
  {
  }
}


// ========================================
// Object attribute updates

void RemoteAmbassador::turnUpdatesOnForObjectInstance(rti::ObjectInstanceHandle theObject, rti::AttributeHandleSet const& theAttributes)
{
  //rti::ObjectClassHandle theObjectClass = mRTIAmb->getKnownObjectClassHandle(theObject);
  //DebugPrintf(SDebug,"%s: handle=%ls class=%ls instance=%ls\n", __FUNCTION__,
  //  theObject.toString().c_str(),
  //  mRTIAmb->getObjectClassName(theObjectClass).c_str(),
  //  mRTIAmb->getObjectInstanceName(theObject).c_str()
  //);
}

void RemoteAmbassador::turnUpdatesOnForObjectInstance(rti::ObjectInstanceHandle theObject, rti::AttributeHandleSet const& theAttributes, std::wstring const& updateRateDesignator)
{
}

void RemoteAmbassador::turnUpdatesOffForObjectInstance(rti::ObjectInstanceHandle theObject, rti::AttributeHandleSet const& theAttributes)
{
  //rti::ObjectClassHandle theObjectClass = mRTIAmb->getKnownObjectClassHandle(theObject);
  //DebugPrintf(SDebug,"%s: handle=%ls class=%ls instance=%ls\n", __FUNCTION__,
  //  theObject.toString().c_str(),
  //  mRTIAmb->getObjectClassName(theObjectClass).c_str(),
  //  mRTIAmb->getObjectInstanceName(theObject).c_str()
  //);
}

void RemoteAmbassador::reflectAttributeValues(
  rti::ObjectInstanceHandle theObject,
  rti::AttributeHandleValueMap const& theAttributeValues,
  rti::VariableLengthData const& theUserSuppliedTag,
  rti::OrderType sentOrder,
  rti::TransportationType theType,
  rti::LogicalTime const& theTime,
  rti::OrderType receivedOrder,
  rti::SupplementalReflectInfo theReflectInfo)
{ 
  reflectAttributeValues(
    theObject,
    theAttributeValues,
    theUserSuppliedTag,
    sentOrder,
    theType,
    theTime,
    rti::OrderType(),
    rti::MessageRetractionHandle(),
    theReflectInfo
  );
}

void RemoteAmbassador::reflectAttributeValues(
  rti::ObjectInstanceHandle theObject,
  rti::AttributeHandleValueMap const& theAttributeValues,
  rti::VariableLengthData const& theUserSuppliedTag,
  rti::OrderType sentOrder,
  rti::TransportationType theType,
  rti::SupplementalReflectInfo theReflectInfo) 
{
  auto timeNS = mInternalTimingService->GetSimTime();
  rti::HLAinteger64Time theTime(timeNS.count());

  reflectAttributeValues(
    theObject,
    theAttributeValues,
    theUserSuppliedTag,
    sentOrder,
    theType,
    theTime,
    rti::OrderType(),
    rti::MessageRetractionHandle(),
    theReflectInfo
  );
}

void RemoteAmbassador::reflectAttributeValues(
  rti::ObjectInstanceHandle theObject,
  rti::AttributeHandleValueMap const& theAttributeValues,
  rti::VariableLengthData const& theUserSuppliedTag,
  rti::OrderType sentOrder,
  rti::TransportationType theType,
  rti::LogicalTime const& theTime,
  rti::OrderType receivedOrder,
  rti::MessageRetractionHandle theHandle,
  rti::SupplementalReflectInfo theReflectInfo) 
{
  mInternalTimingService->SetNewTimeFromEvent(convertTime(theTime));
  
  auto theObjectClass = mRTIAmb->getKnownObjectClassHandle(theObject);

  //DebugPrintf(SDebug, "%s: Object class = %ls, instance name = %ls\n", __FUNCTION__, mRTIAmb->getObjectClassName(theObjectClass).c_str(), mRTIAmb->getObjectInstanceName(theObject).c_str());

  auto iter = mObjectClassSubscriber.find(theObjectClass);
  if (iter != mObjectClassSubscriber.end())
  {
    iter->second->reflectAttributeValues(theObject, theObjectClass, theAttributeValues, theUserSuppliedTag, theTime, theReflectInfo.producingFederate);
  }
  else
  {
    Error("%s: received unsupported attribute update from class %ls\n", __FUNCTION__, theObjectClass.toString().c_str());
  }
}

void RemoteAmbassador::provideAttributeValueUpdate(
  rti::ObjectInstanceHandle theObject,
  rti::AttributeHandleSet const& theAttributes,
  rti::VariableLengthData const& theUserSuppliedTag) 
{
  auto theObjectClass = mRTIAmb->getKnownObjectClassHandle(theObject);
  auto iter = mAttributeRequestProvider.find(theObjectClass);
  if (iter != mAttributeRequestProvider.end())
  {
    iter->second->provideAttributeValueUpdate(theObjectClass, theObject, theAttributes, theUserSuppliedTag);
  }
  else
  {
    Error("%s: received unsupported attribute request from class %ls\n", __FUNCTION__, theObjectClass.toString().c_str());
  }
}

// ========================================
// Remove object callbacks
void RemoteAmbassador::removeObjectInstance(rti::ObjectInstanceHandle theObject, rti::VariableLengthData const& theUserSuppliedTag, rti::OrderType sentOrder, rti::SupplementalRemoveInfo theRemoveInfo)
{
  removeObjectInstance(theObject, theUserSuppliedTag, sentOrder, rti::HLAinteger64Time(0), rti::OrderType::RECEIVE, rti::MessageRetractionHandle(), theRemoveInfo);
}

void RemoteAmbassador::removeObjectInstance(rti::ObjectInstanceHandle theObject, rti::VariableLengthData const& theUserSuppliedTag, rti::OrderType sentOrder, rti::LogicalTime const& theTime, rti::OrderType receivedOrder, rti::SupplementalRemoveInfo theRemoveInfo)
{
  removeObjectInstance(theObject, theUserSuppliedTag, sentOrder, theTime, receivedOrder, rti::MessageRetractionHandle(), theRemoveInfo);
}

void RemoteAmbassador::removeObjectInstance(rti::ObjectInstanceHandle theObject, rti::VariableLengthData const& theUserSuppliedTag, rti::OrderType sentOrder, rti::LogicalTime const& theTime, rti::OrderType receivedOrder, rti::MessageRetractionHandle theHandle, rti::SupplementalRemoveInfo theRemoveInfo)
{
  auto theObjectClass = mRTIAmb->getKnownObjectClassHandle(theObject);
  auto iter = mObjectClassSubscriber.find(theObjectClass);
  if (iter != mObjectClassSubscriber.end())
  {
    iter->second->removeObjectInstance(theObject, theUserSuppliedTag, theTime, theRemoveInfo);
  }
  else
  {
    Error("%s: received unsupported attribute update from class %ls\n", __FUNCTION__, theObjectClass.toString().c_str());
  }
}


// ========================================
// Interactions
void RemoteAmbassador::receiveInteraction(
  rti::InteractionClassHandle theInteraction,
  rti::ParameterHandleValueMap const& theParameterValues,
  rti::VariableLengthData const& theUserSuppliedTag,
  rti::OrderType sentOrder,
  rti::TransportationType theType,
  rti::SupplementalReceiveInfo theReceiveInfo)
{
  rti::HLAinteger64Time theTime(0); // Dummy time
  rti::OrderType receivedOrder = rti::OrderType::RECEIVE; // No time-> Use Receive OrderType

  receiveInteraction(
    theInteraction,
    theParameterValues,
    theUserSuppliedTag,
    sentOrder,
    theType,
    theTime,
    receivedOrder,
    theReceiveInfo
  );
}

void RemoteAmbassador::receiveInteraction(
  rti::InteractionClassHandle theInteraction,
  rti::ParameterHandleValueMap const& theParameterValues,
  rti::VariableLengthData const& theUserSuppliedTag,
  rti::OrderType sentOrder,
  rti::TransportationType theType,
  rti::LogicalTime const& theTime,
  rti::OrderType receivedOrder,
  rti::SupplementalReceiveInfo theReceiveInfo)
{
  mInternalTimingService->SetNewTimeFromEvent(convertTime(theTime));
  
  try
  {
    if (theInteraction == mIAHandleMeasurementInit)
    {
      OnReceiveIAMeasurementInit();
    }
    else if (theInteraction == mIAHandleMeasurementStop)
    {
      OnReceiveIAMeasurementStop(theParameterValues);
    }
    else
    {
      auto iter = mInteractionClassSubscriber.find(theInteraction);
      if (iter != mInteractionClassSubscriber.end())
      {
        iter->second->receiveInteraction(theInteraction, theParameterValues, theUserSuppliedTag, theReceiveInfo);
      }
      else
      {
        Error("%s: unknown interaction: %ls\n", __FUNCTION__, mRTIAmb->getInteractionClassName(theInteraction).c_str());
      }
    }
  }
  catch (const rti::Exception & e)
  {
    Error("%s: rti exception while processing callback for interaction %ls: %ls\n",
      __FUNCTION__,
      theInteraction.toString().c_str(),
      e.what().c_str()
    );
    (void)e;
  }
}

void RemoteAmbassador::receiveInteraction(
  rti::InteractionClassHandle theInteraction,
  rti::ParameterHandleValueMap const& theParameterValues,
  rti::VariableLengthData const& theUserSuppliedTag,
  rti::OrderType sentOrder,
  rti::TransportationType theType,
  rti::LogicalTime const& theTime,
  rti::OrderType receivedOrder,
  rti::MessageRetractionHandle theHandle,
  rti::SupplementalReceiveInfo theReceiveInfo)
{
  receiveInteraction(
    theInteraction,
    theParameterValues,
    theUserSuppliedTag,
    sentOrder,
    theType,
    theTime,
    receivedOrder,
    theReceiveInfo
  );
}

} // namespace NDistSimIB

#pragma warning(pop) 
