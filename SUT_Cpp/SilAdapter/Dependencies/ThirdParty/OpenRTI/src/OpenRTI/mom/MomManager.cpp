

#include "DebugNew.h"
#include "MomManager.h"
#include "StringUtils.h"

#include "MomFederation.h"
#include "MomFederate.h"
#include "MomFederateMetrics.h"
#include "MomInteractions.h"
#include "SocketAddress.h"
#include "OpenRTIVersion.h"
#include "Exception.h"
#include <cassert>
#include <assert.h>

namespace OpenRTI {

#pragma endregion

#pragma region class MomManager

MomManager::MomManager(MomServer* rtiAmb, FederateHandle ownerFederateHandle) 
  : _momServer(rtiAmb), _ownerFederateHandle(ownerFederateHandle)
{
}

MomManager::~MomManager() noexcept
{
  delete objects.mRoot;
  delete interactions.mRoot;
}


void MomManager::initialize()
{
  MomTreeNodeBuilder b(_momServer);

  //
  // Objects
  //
  b.object("HLAobjectRoot")
    .object("HLAmanager")
      .object("HLAfederate")
        .attribute("HLAfederateHandle", "HLAhandle")
        .attribute("HLAfederateName", "HLAunicodeString")
        .attribute("HLAfederateType", "HLAunicodeString")
        .attribute("HLAfederateHost", "HLAunicodeString")
        .attribute("HLARTIversion", "HLAunicodeString")
        .attribute("HLAFOMmoduleDesignatorList", "HLAmoduleDesignatorList")
        .attribute("HLAtimeConstrained", "HLAboolean")
        .attribute("HLAtimeRegulating", "HLAboolean")
        .attribute("HLAasynchronousDelivery", "HLAboolean")
        .attribute("HLAfederateState", "HLAfederateState")
        .attribute("HLAtimeManagerState", "HLAtimeState")
        .attribute("HLAlogicalTime", "HLAlogicalTime")
        .attribute("HLAlookahead", "HLAtimeInterval")
        .attribute("HLAGALT", "HLAlogicalTime")
        .attribute("HLALITS", "HLAlogicalTime")
        .attribute("HLAROlength", "HLAcount")
        .attribute("HLATSOlength", "HLAcount")
        .attribute("HLAreflectionsReceived", "HLAcount")
        .attribute("HLAupdatesSent", "HLAcount")
        .attribute("HLAinteractionsReceived", "HLAcount")
        .attribute("HLAinteractionsSent", "HLAcount")
        .attribute("HLAobjectInstancesThatCanBeDeleted", "HLAcount")
        .attribute("HLAobjectInstancesUpdated", "HLAcount")
        .attribute("HLAobjectInstancesReflected", "HLAcount")
        .attribute("HLAobjectInstancesDeleted", "HLAcount")
        .attribute("HLAobjectInstancesRemoved", "HLAcount")
        .attribute("HLAobjectInstancesRegistered", "HLAcount")
        .attribute("HLAobjectInstancesDiscovered", "HLAcount")
        .attribute("HLAtimeGrantedTime", "HLAmsec")
        .attribute("HLAtimeAdvancingTime", "HLAmsec")
        .attribute("HLAconveyRegionDesignatorSets", "HLAswitch")
        .attribute("HLAconveyProducingFederate", "HLAswitch")
      .end()
      .object("HLAfederation")
        .attribute("HLAfederationName", "HLAunicodeString")
        .attribute("HLAfederatesInFederation", "HLAhandleList")
        .attribute("HLARTIversion", "HLAunicodeString")
        .attribute("HLAMIMdesignator", "HLAunicodeString")
        .attribute("HLAFOMmoduleDesignatorList", "HLAmoduleDesignatorList")
        .attribute("HLAcurrentFDD", "HLAunicodeString")
        .attribute("HLAtimeImplementationName", "HLAunicodeString")
        .attribute("HLAlastSaveName", "HLAunicodeString")
        .attribute("HLAlastSaveTime", "HLAlogicalTime")
        .attribute("HLAnextSaveName", "HLAunicodeString")
        .attribute("HLAnextSaveTime", "HLAlogicalTime")
        //.attribute("HLAconveyRegionDesignatorSets", "HLAswitch")
        .attribute("HLAautoProvide", "HLAswitch")
      .end()
    .end();
  MomTreeNode* objectRoot = b.getContext();
  b.end();

  //
  // Interactions
  //
  b.interaction("HLAinteractionRoot")
    .interaction("HLAmanager")
      .interaction("HLAfederate")
        .parameter("HLAfederate", "HLAhandle")
        .interaction("HLAadjust")
          .interaction("HLAsetTiming")
            .parameter("HLAreportPeriod", "HLAseconds")
          .end()
          .interaction("HLAmodifyAttributeState")
            .parameter("HLAobjectInstance", "HLAhandle")
            .parameter("HLAattribute", "HLAhandle")
            .parameter("HLAattributeState", "HLAownership")
          .end()
          .interaction("HLAsetServiceReporting")
            .parameter("HLAreportingState", "HLAboolean")
          .end()
          .interaction("HLAsetExceptionReporting")
            .parameter("HLAreportingState", "HLAboolean")
          .end()
          .interaction("HLAsetSwitches")
            //.parameter("HLAconveyRegionDesignatorSets", "HLAswitch")
            //.parameter("HLAconveyProducingFederate", "HLAswitch")
          .end()
        .end()
        .interaction("HLArequest")
          .interaction("HLArequestPublications").end()
          .interaction("HLArequestSubscriptions").end()
          .interaction("HLArequestObjectInstancesThatCanBeDeleted").end()
          .interaction("HLArequestObjectInstancesUpdated").end()
          .interaction("HLArequestObjectInstancesReflected").end()
          .interaction("HLArequestUpdatesSent").end()
          .interaction("HLArequestInteractionsSent").end()
          .interaction("HLArequestReflectionsReceived").end()
          .interaction("HLArequestInteractionsReceived").end()
          .interaction("HLArequestObjectInstanceInformation")
            .parameter("HLAobjectInstance", "HLAhandle")
          .end()
          .interaction("HLArequestFOMmoduleData")
          .parameter("HLAFOMmoduleIndicator", "HLAindex")
          .end()
        .end()
        .interaction("HLAreport")
          .interaction("HLAreportObjectClassPublication")
            .parameter("HLAnumberOfClasses", "HLAcount")
            .parameter("HLAobjectClass", "HLAhandle")
            .parameter("HLAattributeList", "HLAhandleList")
          .end()
          .interaction("HLAreportInteractionPublication")
            .parameter("HLAinteractionClassList", "HLAhandleList")
          .end()
          .interaction("HLAreportObjectClassSubscription")
            .parameter("HLAnumberOfClasses", "HLAcount")
            .parameter("HLAobjectClass", "HLAhandle")
            .parameter("HLAactive", "HLAboolean")
            .parameter("HLAmaxUpdateRate", "HLAupdateRateName")
            .parameter("HLAattributeList", "HLAhandleList")
          .end()
          .interaction("HLAreportInteractionSubscription")
            .parameter("HLAinteractionClassList", "HLAinteractionSubList")
          .end()
          .interaction("HLAreportObjectInstancesThatCanBeDeleted")
            .parameter("HLAobjectInstanceCounts", "HLAobjectClassBasedCounts")
          .end()
          .interaction("HLAreportObjectInstancesUpdated")
            .parameter("HLAobjectInstanceCounts", "HLAobjectClassBasedCounts")
          .end()
          .interaction("HLAreportObjectInstancesReflected")
            .parameter("HLAobjectInstanceCounts", "HLAobjectClassBasedCounts")
          .end()
          .interaction("HLAreportUpdatesSent")
            .parameter("HLAtransportation", "HLAtransportationName")
            .parameter("HLAupdateCounts", "HLAobjectClassBasedCounts")
          .end()
          .interaction("HLAreportReflectionsReceived")
            .parameter("HLAtransportation", "HLAtransportationName")
            .parameter("HLAreflectCounts", "HLAobjectClassBasedCounts")
          .end()
          .interaction("HLAreportInteractionsSent")
            .parameter("HLAtransportation", "HLAtransportationName")
            .parameter("HLAinteractionCounts", "HLAinteractionCounts")
          .end()
          .interaction("HLAreportInteractionsReceived")
            .parameter("HLAtransportation", "HLAtransportationName")
            .parameter("HLAinteractionCounts", "HLAinteractionCounts")
          .end()
          .interaction("HLAreportObjectInstanceInformation")
            .parameter("HLAobjectInstance", "HLAhandle")
            .parameter("HLAownedInstanceAttributeList", "HLAhandleList")
            .parameter("HLAregisteredClass", "HLAhandle")
            .parameter("HLAknownClass", "HLAhandle")
          .end()
          //.interaction("Alert")
          //  .parameter("AlertSeverity", "HLAunicodeString")
          //  .parameter("AlertDescription", "HLAunicodeString")
          //  .parameter("AlertID", "HLAunicodeString")
          //.end()
          .interaction("HLAreportException")
            .parameter("HLAservice", "HLAunicodeString")
            .parameter("HLAexception", "HLAunicodeString")
          .end()
          .interaction("HLAreportServiceInvocation")
            .parameter("HLAservice", "HLAunicodeString")
            //.parameter("Initiator", "HLAunicodeString")
            .parameter("HLAsuccessIndicator", "HLAboolean")
            .parameter("HLAsuppliedArguments", "HLAargumentList")
            //.parameter("SuppliedArgument1", "HLAunicodeString")
            //.parameter("SuppliedArgument2", "HLAunicodeString")
            //.parameter("SuppliedArgument3", "HLAunicodeString")
            //.parameter("SuppliedArgument4", "HLAunicodeString")
            //.parameter("SuppliedArgument5", "HLAunicodeString")
            .parameter("HLAreturnedArguments", "HLAargumentList")
            .parameter("HLAexception", "HLAunicodeString")
            .parameter("HLAserialNumber", "HLAcount")
          .end()
          .interaction("HLAreportMOMexception")
            .parameter("HLAservice", "HLAunicodeString")
            .parameter("HLAexception", "HLAunicodeString")
            .parameter("HLAparameterError", "HLAboolean")
          .end()
          .interaction("HLAreportFederateLost")
            .parameter("HLAfederateName", "HLAunicodeString")
            .parameter("HLAtimeStamp", "HLAlogicalTime")
            .parameter("HLAfaultDescription", "HLAunicodeString")
          .end()
          .interaction("HLAreportFOMmoduleData")
            .parameter("HLAFOMmoduleIndicator", "HLAindex")
            .parameter("HLAFOMmoduleData", "HLAunicodeString")
          .end()
        .end()
        .interaction("HLAservice")
          .interaction("HLAresignFederationExecution")
            .parameter("HLAresignAction", "HLAresignAction")
          .end()
          .interaction("HLAsynchronizationPointAchieved")
            .parameter("HLAlabel", "HLAunicodeString")
          .end()
          .interaction("HLAfederateSaveBegun").end()
          .interaction("HLAfederateSaveComplete")
            .parameter("HLAsuccessIndicator", "HLAboolean")
          .end()
          .interaction("HLAfederateRestoreComplete")
            .parameter("HLAsuccessIndicator", "HLAboolean")
          .end()
          .interaction("HLApublishObjectClassAttributes")
            .parameter("HLAobjectClass", "HLAhandle")
            .parameter("HLAattributeList", "HLAhandleList")
          .end()
          .interaction("HLAunpublishObjectClassAttributes")
            .parameter("HLAobjectClass", "HLAhandle")
            .parameter("HLAattributeList", "HLAhandleList")
          .end()
          .interaction("HLApublishInteractionClass")
            .parameter("HLAinteractionClass", "HLAhandle")
          .end()
          .interaction("HLAunpublishInteractionClass")
            .parameter("HLAinteractionClass", "HLAhandle")
          .end()
          .interaction("HLAsubscribeObjectClassAttributes")
            .parameter("HLAobjectClass", "HLAhandle")
            .parameter("HLAattributeList", "HLAhandleList")
            .parameter("HLAactive", "HLAboolean")
          .end()
          .interaction("HLAunsubscribeObjectClassAttributes")
            .parameter("HLAobjectClass", "HLAhandle")
            .parameter("HLAattributeList", "HLAhandleList")
          .end()
          .interaction("HLAsubscribeInteractionClass")
            .parameter("HLAinteractionClass", "HLAhandle")
            .parameter("HLAactive", "HLAboolean")
          .end()
          .interaction("HLAunsubscribeInteractionClass")
            .parameter("HLAinteractionClass", "HLAhandle")
          .end()
          .interaction("HLAdeleteObjectInstance")
            .parameter("HLAobjectInstance", "HLAhandle")
            .parameter("HLAtag", "HLAopaqueData")
            .parameter("HLAtimeStamp", "HLAlogicalTime")
          .end()
          .interaction("HLAlocalDeleteObjectInstance")
            .parameter("HLAobjectInstance", "HLAhandle")
          .end()
          .interaction("HLArequestAttributeTransportationTypeChange")
            .parameter("HLAobjectInstance", "HLAhandle")
            .parameter("HLAattributeList", "HLAhandleList")
            .parameter("HLAtransportation", "HLAtransportationName")
          .end()
          .interaction("HLArequestInteractionTransportationTypeChange")
            .parameter("HLAinteractionClass", "HLAhandle")
            .parameter("HLAtransportation", "HLAtransportationName")
          .end()
          .interaction("HLAunconditionalAttributeOwnershipDivestiture")
            .parameter("HLAobjectInstance", "HLAhandle")
            .parameter("HLAattributeList", "HLAhandleList")
          .end()
          .interaction("HLAenableTimeRegulation")
            //.parameter("FederationTime", "HLAlogicalTime")
            .parameter("HLAlookahead", "HLAtimeInterval")
          .end()
          .interaction("HLAdisableTimeRegulation").end()
          .interaction("HLAenableTimeConstrained").end()
          .interaction("HLAdisableTimeConstrained").end()
          .interaction("HLAtimeAdvanceRequest")
            .parameter("HLAtimeStamp", "HLAlogicalTime")
          .end()
          .interaction("HLAtimeAdvanceRequestAvailable")
            .parameter("HLAtimeStamp", "HLAlogicalTime")
          .end()
          .interaction("HLAnextMessageRequest")
            .parameter("HLAtimeStamp", "HLAlogicalTime")
          .end()
          .interaction("HLAnextMessageRequestAvailable")
            .parameter("HLAtimeStamp", "HLAlogicalTime")
          .end()
          .interaction("HLAflushQueueRequest")
            .parameter("HLAtimeStamp", "HLAlogicalTime")
          .end()
          .interaction("HLAenableAsynchronousDelivery").end()
          .interaction("HLAdisableAsynchronousDelivery").end()
          .interaction("HLAmodifyLookahead")
            .parameter("HLAlookahead", "HLAtimeInterval")
          .end()
          .interaction("HLAchangeAttributeOrderType")
            .parameter("HLAobjectInstance", "HLAhandle")
            .parameter("HLAattributeList", "HLAhandleList")
            .parameter("HLAsendOrder", "HLAorderType")
          .end()
          .interaction("HLAchangeInteractionOrderType")
            .parameter("HLAinteractionClass", "HLAhandle")
            .parameter("HLAsendOrder", "HLAorderType")
          .end()
        .end()
      .end()
      .interaction("HLAfederation")
        .interaction("HLAadjust")
          .interaction("HLAsetSwitches")
          .parameter("HLAautoProvide", "HLAswitch")
          .end()
        .end()
        .interaction("HLArequest")
          .interaction("HLArequestSynchronizationPoints").end()
          .interaction("HLArequestSynchronizationPointStatus").end()
          .interaction("HLArequestFOMmoduleData")
            .parameter("HLAFOMmoduleIndicator", "HLAindex")
          .end()
          .interaction("HLArequestMIMdata").end()
        .end()
        .interaction("HLAreport")
          .interaction("HLAreportSynchronizationPoints")
            .parameter("HLAsyncPoints", "HLAsynchPointList")
          .end()
          .interaction("HLAreportSynchronizationPointStatus")
            .parameter("HLAsyncPointName", "HLAunicodeString")
            .parameter("HLAsyncPointFederates", "HLAsynchPointFederateList")
          .end()
          .interaction("HLAreportFOMmoduleData")
            .parameter("HLAFOMmoduleIndicator", "HLAindex")
            .parameter("HLAFOMmoduleData", "HLAunicodeString")
          .end()
          .interaction("HLAreportMIMdata")
            .parameter("HLAMIMdata", "HLAunicodeString")
          .end()
        .end()
      .end()
    .end();

  MomTreeNode* interactionRoot = b.getContext();
  b.end();

  objects = objectRoot;
  interactions = interactionRoot;
  mFederateMetrics = std::make_shared<MomFederateMetrics>(this);

  mFederationObjectClass = objects["HLAobjectRoot"]["HLAmanager"]["HLAfederation"];;
  assert(mFederationObjectClass.valid());
  mFederateObjectClass = objects["HLAobjectRoot"]["HLAmanager"]["HLAfederate"];;
  assert(mFederateObjectClass.valid());
}

void MomManager::publishSubscribeObjects()
{
  ObjectClassAccessor federationClass = objects["HLAobjectRoot"]["HLAmanager"]["HLAfederation"];
  assert(federationClass.mRoot->objectClassHandle.valid());
  ObjectClassAccessor federateClass = objects["HLAobjectRoot"]["HLAmanager"]["HLAfederate"];
  assert(federateClass.mRoot->objectClassHandle.valid());
  if (_momServer->isRoot())
  {
    _momServer->publishObjectClassAttributes(federationClass, federationClass.attributes);
    // a root server just wants to keep track of federate objects created elsewhere
    //_momServer->publishObjectClassAttributes(federateClass, federateClass.attributes);
    _momServer->subscribeObjectClassAttributes(federateClass, federateClass.attributes, true);
  }
  else
  {
    // a child server only publishes federate objects
    _momServer->subscribeObjectClassAttributes(federationClass, federationClass.attributes, true);
    _momServer->publishObjectClassAttributes(federateClass, federateClass.attributes);
    _momServer->subscribeObjectClassAttributes(federationClass, federationClass.attributes, true);
  }
}

void MomManager::publishSubscribeInteractions()
{
  ParameterHandleValueMap filter;
  if (_momServer->isRoot())
  {
    // the root server won't respond to request interactions
    return;
  }

  // we should have discovered our owner federate already
  assert(mFederate != nullptr);
  assert(mFederate->GetFederateHandle().valid());
  assert(mFederate->GetFederateHandle() == _ownerFederateHandle);

  ParameterHandle federateParameterHandle = interactions["HLAinteractionRoot"]["HLAmanager"]["HLAfederate"]["HLArequest"].parameters["HLAfederate"];
  assert(federateParameterHandle.valid());
  filter[federateParameterHandle] = HLAhandle(mFederate->GetFederateHandle()).encode();

  std::shared_ptr<MomInteractionHandler> handlers[]{
    std::make_shared<InteractionHandlerFederateRequestPublications>(shared_from_this()),
    std::make_shared<InteractionHandlerFederateRequestSubscriptions>(shared_from_this()),
    std::make_shared<InteractionHandlerFederateRequestObjectInstancesUpdated>(shared_from_this()),
    std::make_shared<InteractionHandlerFederateRequestObjectInstancesReflected>(shared_from_this()),
    std::make_shared<InteractionHandlerFederateRequestUpdatesSent>(shared_from_this()),
    std::make_shared<InteractionHandlerFederateRequestReflectionsReceived>(shared_from_this()),
    std::make_shared<InteractionHandlerFederateRequestInteractionsSent>(shared_from_this()),
    std::make_shared<InteractionHandlerFederateRequestInteractionsReceived>(shared_from_this()),
  };

  for (auto handler : handlers)
  {
    InteractionClassHandle request = handler->getRequestInteractionClass();
    //DebugPrintf("%s: subscribed to request %s, %d responses\n", __FUNCTION__, _momServer->getInteractionClassName(request).c_str(), handler->getResponseInteractionClasses().size());
    _momServer->subscribeInteractionClassWithFilter(request, filter, true);
    for (auto ic : handler->getResponseInteractionClasses())
    {
      //DebugPrintf("%s: publish response %s\n", __FUNCTION__, _momServer->getInteractionClassName(ic).c_str());
      _momServer->publishInteractionClass(ic);
    }
    mMomInteractionHandlers[request] = handler;
  }
}

// called by the root server after the federation has been created and the RTI federate joined the federation
void MomManager::notifyFederationCreated(const std::string& federationName)
{
  //DebugPrintf(">>> %s\n", __FUNCTION__);
  if (_momServer->isRoot())
  {
    try
    {
      _momServer->reserveObjectInstanceName(federationName, [this, federationName]
      {
        mFederationObject = _momServer->registerObjectInstance(mFederationObjectClass, federationName);
        mFederation = RegisterFederation(mFederationObject);
        provideAttributeValueUpdate(mFederationObject, mFederation->GetKnownAttributes());
        //DebugPrintf("%s: created federation object %s\n", __FUNCTION__, federationName.c_str());
      });
    }
    catch (const Exception& e)
    {
      DebugPrintf("%s (federation object): %s\n", __FUNCTION__, e.what());
      throw;
    }
  }
  //DebugPrintf("<<< %s\n", __FUNCTION__);
}


// called by all servers after the federation has been created and the RTI federate joined the federation
void MomManager::notifyFederationJoined(FederateHandle /*federateHandle*/, const std::string& /*federateName*/, const std::string& /*federateType*/)
{
  //DebugPrintf(">>> %s federateHandle=%s federateName=%s federateType=%s root=%d\n", __FUNCTION__,
  //            federateHandle.toString().c_str(), federateName.c_str(), federateType.c_str(), _momServer->isRoot());
  // nothing special to do here - we don't register the RTI MOM federate as an object
}

// called by child servers whenever a *different* federate (e.g. an application federate or another child RTI federate) joined the federation
void MomManager::notifyOwnerFederateJoined(FederateHandle federateHandle, const std::string& federateName, const std::string& federateType)
{
  //DebugPrintf(">>> %s: federateHandle=%s federateName=%s federatetype=%s\n", __FUNCTION__,
  //            federateHandle.toString().c_str(), federateName.c_str(), federateType.c_str());
  if (_momServer->isRoot())
  {
    //DebugPrintf("%s: root server must not create federate objects\n", __FUNCTION__);
    return;
  }
  try
  {
    _momServer->reserveObjectInstanceName(federateName,  [this, federateHandle, federateName, federateType]()
    {
      //DebugPrintf("creating federate object %s\n", federateName.c_str());
      if (mFederation == nullptr)
      {
        //DebugPrintf("federation object not present\n");
        return;
      }
      ObjectInstanceHandle federateObject = _momServer->registerObjectInstance(mFederateObjectClass, federateName);
      assert(mFederation != nullptr);
      auto federate = RegisterFederate(federateHandle, federateObject);
      federate->SetFederateHandle(federateHandle);
      federate->SetFederateName(utf8ToUcs(federateName));
      federate->SetFederateType(utf8ToUcs(federateType));
      federate->SetFederateHost(OpenRTI::SocketAddress::getHostName());
      federate->SetFederateRTIversion(utf8ToUcs(OPENRTI_FILE_VERSION_STRING));

      // remember:
      // 1. this only gets called in a leaf server
      // 2. in a leaf server the federation object is only more or less a dummy, because it's data is never accessed
      // 3. in case we want more than one federate in the same process/NetworkServer, it would be called multiple times,
      //    so we need to get rid of mFederate and mFederateMetrics. Then we actually require the list of federates 
      //    attached to the federation
      mFederation->AddFederate(federate);
      //DebugPrintf("%s: created federate object %s\n", __FUNCTION__, federateName.c_str());
      if (federateHandle == _ownerFederateHandle)
      {
        //DebugPrintf("%s: owner federate joined: name=%s type=%s\n", __FUNCTION__, federateName.c_str(), federateType.c_str());
        mFederate = federate;
        // While initializing, the messages indicating time constrained/time regulating usually have already passed,
        // and won't be received again.
        // Get the information directly from the ServerModel.
        mFederate->SetTimeConstrained(_momServer->getIsTimeConstrained(federateHandle));
        mFederate->SetTimeRegulating(_momServer->getIsTimeRegulating(federateHandle));
        mFederateMetrics->setFederate(mFederate);
        publishSubscribeInteractions();
        provideAttributeValueUpdate(federateObject, mFederate->GetKnownAttributes());
      }
    });
  }
  catch (const Exception& e)
  {
    DebugPrintf("%s: %s\n", __FUNCTION__, e.what());
    throw;
  }
}

void MomManager::notifyFederateResigned(FederateHandle federateHandle)
{
  //DebugPrintf("%s: federateHandle=%s\n", __FUNCTION__, federateHandle.toString().c_str());
  auto iterByHandle = mFederatesByHandle.find(federateHandle);
  ObjectInstanceHandle federateObject;
  if (iterByHandle != mFederatesByHandle.end())
  {
    auto federate = iterByHandle->second;
    federateObject = federate->GetInternalFederateObjectInstance();
    mFederatesByHandle.erase(iterByHandle);
    auto iterByObject = mFederatesByObject.find(federateObject);
    if (iterByObject != mFederatesByObject.end())
    {
      mFederatesByObject.erase(iterByObject);
    }
    else
    {
      //DebugPrintf("%s: federate object %s not found in FederatesByObject\n", __FUNCTION__, _momServer->getObjectInstanceName(federateObject).c_str());
    }
    assert(mFederation != nullptr);
    mFederation->RemoveFederate(federate);
  }
  // else: it's most probably a RTI MOM federate - no action required, because we don't register these
  // check if the federate just killed is the one who instantiated us
  if (federateHandle == _ownerFederateHandle)
  {
    // Upps! It's me ... suicide day.
    mFederate.reset();
    if (mFederateObject.valid())
    {
      _momServer->deleteObjectInstance(mFederateObject);
      mFederateObject = ObjectInstanceHandle();
    }
    _ownerFederateHandle = FederateHandle();
  }
  if (_momServer->isRoot() && mFederation != nullptr)
  {
    provideAttributeValueUpdate(mFederationObject, mFederation->GetKnownAttributes());
  }
}

void MomManager::discoverObjectInstance(ObjectInstanceHandle objectInstance, ObjectClassHandle objectClass, const std::string& name)
{
  if (objectClass == mFederationObjectClass)
  {
    //DebugPrintf("discovered federation object %s\n", name.c_str());
    if (mFederation != nullptr)
    {
      //DebugPrintf("federation object already present");
    }
    mFederationObject = objectInstance;
    mFederation = RegisterFederation(objectInstance);
  }
  else if (objectClass == mFederateObjectClass)
  {
    //DebugPrintf("discovered federate object %s\n", name.c_str());
    if (mFederation != nullptr)
    {
      FederateHandle federateHandle;

      try {
        federateHandle = _momServer->getFederateHandle(name);
        //DebugPrintf("discovered federate object %s, federateName=%s federateHandle=%s\n",
        //            name.c_str(), name.c_str(), federateHandle.toString().c_str());
      }
      catch (const NameNotFound&)
      {
      }
      if (!federateHandle.valid())
      {
        try {
          federateHandle = _momServer->getFederateHandle("HLA" + name);
          //DebugPrintf("discovered federate object %s, federateName=%s federateHandle=%s\n",
          //            name.c_str(), ("HLA" + name).c_str(), federateHandle.toString().c_str());
        }
        catch (const NameNotFound&)
        {
        }
      }
      if (federateHandle.valid())
      {
        auto federate = RegisterFederate(federateHandle, objectInstance);
      }
      else
      {
        //DebugPrintf("federate not found: %s\n", name.c_str());
      }
    }
  }
}

void MomManager::removeObjectInstance(ObjectInstanceHandle objectInstance)
{
  ObjectClassHandle objectClass = _momServer->getKnownObjectClassHandle(objectInstance);
  auto name = _momServer->getObjectInstanceName(objectInstance);
  if (objectClass == mFederationObjectClass)
  {
    //DebugPrintf("%s: destroyed federation object %s\n", __FUNCTION__, name.c_str());
  }
  else if (objectClass == mFederateObjectClass)
  {
    //DebugPrintf("%s: destroyed federate object %s\n", __FUNCTION__, name.c_str());
  }
}

std::shared_ptr<MomFederation> MomManager::RegisterFederation(ObjectInstanceHandle federationObject)
{
  auto federationName = _momServer->getObjectInstanceName(federationObject);
  std::shared_ptr<MomFederation> federation = std::make_shared<MomFederation>(shared_from_this(), utf8ToUcs(federationName), federationObject);
  return federation;
}

std::shared_ptr<MomFederate> MomManager::RegisterFederate(FederateHandle federateHandle, ObjectInstanceHandle federateObject)
{
  if (!federateHandle.valid())
  {
    //DebugPrintf("%s: invalid federate handle", __FUNCTION__);
  }

  std::shared_ptr<MomFederate> federate = std::make_shared<MomFederate>(shared_from_this(), federateHandle, federateObject);
  mFederatesByHandle.insert(std::make_pair(federateHandle, federate));
  mFederatesByObject.insert(std::make_pair(federateObject, federate));
  if (mFederation != nullptr)
  {
    mFederation->AddFederate(federate);
    if (_momServer->isRoot())
    {
      provideAttributeValueUpdate(mFederationObject, mFederation->GetKnownAttributes());
    }
  }
  return federate;
}

bool MomManager::provideAttributeValueUpdate(ObjectInstanceHandle theObject, AttributeHandleSet const& requestedAttributes)
{
  ObjectClassHandle objectClass;
  try
  {
    objectClass = _momServer->getKnownObjectClassHandle(theObject);
  }
  catch (const Exception& e)
  {
    DebugPrintf("%s: getKnownObjectClassHandle failed: %s\n", __FUNCTION__, e.what());
    return false;
  }
  //DebugPrintf("%s: class=%s instance=%s\n", __FUNCTION__, _momServer->getObjectClassName(objectClass).c_str(), _momServer->getObjectInstanceName(theObject).c_str());
  if (theObject == mFederationObject)
  {
    AttributeHandleValueMap responseAttributes = mFederation->GetAttributeValues(requestedAttributes);
    VariableLengthData tag;
    try
    {
      _momServer->updateAttributeValues(mFederationObject, responseAttributes);
      //DebugPrintf("%s: class=%s instance=%s: sent %d attributes for federation object\n", __FUNCTION__, _momServer->getObjectClassName(objectClass).c_str(), _momServer->getObjectInstanceName(theObject).c_str(), responseAttributes.size());
    }
    catch (const Exception& e)
    {
      DebugPrintf("%s: updateAttributeValues failed: %s\n", __FUNCTION__, e.what());
    }
    return true;
  }
  else if (objectClass == mFederateObjectClass)
  {
    auto iter = mFederatesByObject.find(theObject);
    if (iter != mFederatesByObject.end())
    {
      auto federate = iter->second;
      AttributeHandleValueMap responseAttributes = federate->GetAttributeValues(requestedAttributes);
      VariableLengthData tag;
      try
      {
        _momServer->updateAttributeValues(theObject, responseAttributes);
        //DebugPrintf("%s: class=%s instance=%s: sent %d attributes for federate object\n", __FUNCTION__, _momServer->getObjectClassName(objectClass).c_str(), _momServer->getObjectInstanceName(theObject).c_str(), responseAttributes.size());
        return true;
      }
      catch (const Exception& e)
      {
        DebugPrintf("%s: updateAttributeValues failed: %s\n", __FUNCTION__, e.what());
      }
    }
    else
    {
      //DebugPrintf("%s: federate object not found: %s %s\n", __FUNCTION__, theObject.toString().c_str(), _momServer->getObjectInstanceName(theObject).c_str());
    }
  }
  // otherwise it'S not an object handled by this federate (e.g. we've found the federation by discovery, or it's a federate found by discovery)
  return false;
}


void MomManager::reflectAttributeValues(ObjectInstanceHandle theObject,
                                        AttributeHandleValueMap const& theAttributeValues,
                                        VariableLengthData const& /*theUserSuppliedTag*/)
{
  //DebugPrintf("%s\n", __FUNCTION__);
  try
  {
    ObjectClassHandle theObjectClass = _momServer->getKnownObjectClassHandle(theObject);
    if (theObject == mFederation->GetObjectInstance())
    {
      auto federation = mFederation;
      federation->SetAttributeValues(theAttributeValues);
    }
    else if (theObjectClass == mFederateObjectClass)
    {
      auto federate = mFederatesByObject[theObject];
      federate->SetAttributeValues(theAttributeValues);
    }
  }
  catch (const Exception& e)
  {
    DebugPrintf("%s: SetAttributeValues failed: %s\n", __FUNCTION__, e.what());
  }
}

bool MomManager::receiveInteraction(InteractionClassHandle theInteraction, ParameterHandleValueMap const& requestParameters)
{
  auto handler = mMomInteractionHandlers.find(theInteraction);
  if (handler != mMomInteractionHandlers.end())
  {
    try {
      FederateHandle requestedFederate = handler->second->getFederateHandleFromParameters(requestParameters);
      if (requestedFederate != _ownerFederateHandle)
      {
        //DebugPrintf("%s: wrong federate in request: %s <=> %s\n", __FUNCTION__,
        //            _momServer->getFederateName(requestedFederate).c_str(),
        //            _momServer->getFederateName(_ownerFederateHandle).c_str());
        return false;
      }
      MomInteractionHandler::ResponseList responses = (*handler->second)(requestParameters);
      for (auto it_ic_pv : responses)
      {
#ifndef NDEBUG
        FederateHandle responseFederate = handler->second->getFederateHandleFromResponse(it_ic_pv.second);
#endif
        assert(requestedFederate.valid());
        assert(responseFederate.valid());
        assert(requestedFederate == responseFederate);
        _momServer->sendInteraction(it_ic_pv.first, it_ic_pv.second);
        //DebugPrintf("%s: sent response %s\n", __FUNCTION__, _momServer->getInteractionClassName(ic).c_str());
      }
      return true;
    }
    catch (const Exception& e)
    {
      DebugPrintf("%s: %s\n", __FUNCTION__, e.what());
    }
  }
  return false;
}

std::shared_ptr<MomFederate> MomManager::getFederate(FederateHandle federateHandle) const
{
  assert(federateHandle.valid());
  /*
  DebugPrintf("%s: federateHandle=%s(%s), known federates:\n", __FUNCTION__, federateHandle.toString().c_str(), _momServer->getFederateName(federateHandle).c_str());
  for (auto [handle, fed] : mFederatesByHandle)
  {
    DebugPrintf("%s:     federateHandle=%s(%s) %ls\n", __FUNCTION__, 
                handle.toString().c_str(), _momServer->getFederateName(handle).c_str(), fed->GetFederateName().c_str());
  }
  */
  auto iter = mFederatesByHandle.find(federateHandle);
  if (iter != mFederatesByHandle.end()) return iter->second;
  return std::shared_ptr<MomFederate>();
}

std::shared_ptr<MomFederateMetrics> MomManager::getFederateMetrics() const
{
  return mFederateMetrics;
}

VariableLengthData MomManager::getEncodedFederateHandle() const
{
  return HLAhandle(mFederate->GetFederateHandle()).encode();
}

void MomManager::setFOMModuleList(const FOMModuleList& moduleList, const std::string& currentFDD)
{
  if (mFederation != nullptr)
  {
    std::vector<std::wstring> moduleDesignatorList;
    for (auto& module : moduleList)
    {
      moduleDesignatorList.push_back(utf8ToUcs(module.getDesignator()));
    }
    mFederation->SetFOMmoduleDesignatorList(moduleDesignatorList);
    mFederation->SetCurrentFDD(utf8ToUcs(currentFDD));
  }
}

#pragma endregion

#pragma region class MomManager::MomTreeNode

MomManager::MomTreeNode::~MomTreeNode() noexcept
{
  for (auto child : children)
  {
    delete child;
  }
}

const std::string MomManager::MomTreeNode::getQualifiedName() const
{
  std::string qualifiedName = this->getName();
  if (this->mParent != nullptr)
    qualifiedName = this->mParent->getQualifiedName() + "." + qualifiedName;

  return qualifiedName;
}

void MomManager::MomTreeNode::addChild(MomTreeNode* node)
{
  if (node->mParent == nullptr)
  {
    node->mParent = this;
    this->children.push_back(node);
  }
  else
  {
    throw RTIinternalError(node->getName() + " already has a parent");
  }
}
#pragma endregion

#pragma region class MomManager::MomTreeNodeBuilder
void MomManager::MomTreeNodeBuilder::add(MomTreeNode* newNode)
{
  if (!this->mContextStack.empty())
  {
    MomTreeNode* currentContext = this->mContextStack.front();
    currentContext->addChild(newNode);
  }
  else
  {
    throw new RTIinternalError("no context to add to");
  }
}

void MomManager::MomTreeNodeBuilder::addAndPush(MomTreeNode* newNode)
{
  assert(newNode->getType() == Object || newNode->getType() == Attribute || newNode->getType() == Interaction|| newNode->getType() == Parameter);
  if (!this->mContextStack.empty())
  {
    MomTreeNode* currentContext = this->mContextStack.front();
    currentContext->addChild(newNode);
  }

  this->mContextStack.push_front(newNode);
}

MomManager::MomTreeNodeBuilder& MomManager::MomTreeNodeBuilder::object(std::string name)
{
  try
  {
    std::string fqName;
    if (!mContextStack.empty())
      fqName = mContextStack.front()->getQualifiedName() + "." + name;
    else
      fqName = name;
    ObjectClassHandle handle = mMomServer->getObjectClassHandle(fqName);
    if (!handle.valid())
      throw NameNotFound(name.c_str());
    MomTreeNode* node = new MomTreeNode(name, MomType::Object, handle);

    this->addAndPush(node);
    return *this;
  }
  catch (const NameNotFound& e)
  {
    DebugPrintf("%s: object class %s not found: %s", __FUNCTION__, name.c_str(), e.what());
    throw;
  }
  catch (const Exception& e)
  {
    DebugPrintf("%s: Exception: %s", __FUNCTION__, e.what());
    throw;
  }
}

MomManager::MomTreeNodeBuilder& MomManager::MomTreeNodeBuilder::attribute(std::string name, std::string datatype)
{
  if (mContextStack.size() == 0)
    throw RTIinternalError("attribute cannot be root node");

  if (mContextStack.front()->getType() != MomType::Object)
    throw RTIinternalError("attribute can only be added to an object");

  try
  {
    AttributeHandle handle = mMomServer->getAttributeHandle(mContextStack.front()->objectClassHandle, name);
    //DebugPrintf("%s: \"%s\" => %s\n", __FUNCTION__, name.c_str(), handle.toString().c_str());
    if (!handle.valid())
      throw NameNotFound(name.c_str());
    MomTreeNode* node = new MomTreeNode(name, MomType::Attribute, handle, datatype);
    this->add(node);
    return *this;
  }
  catch (const NameNotFound& e)
  {
    DebugPrintf("%s: attribute %s not found: %s", __FUNCTION__, name.c_str(), e.what());
    throw;
  }
  catch (const Exception& e)
  {
    DebugPrintf("%s: Exception: %s", __FUNCTION__, e.what());
    throw;
  }
}

MomManager::MomTreeNodeBuilder& MomManager::MomTreeNodeBuilder::interaction(std::string name)
{
  try
  {
    std::string fqName;
    if (!mContextStack.empty())
      fqName = mContextStack.front()->getQualifiedName() + "." + name;
    else
      fqName = name;
    InteractionClassHandle handle = mMomServer->getInteractionClassHandle(name);
    if (!handle.valid())
      throw NameNotFound(name.c_str());
    MomTreeNode* node = new MomTreeNode(name, MomType::Interaction, handle);

    this->addAndPush(node);
    return *this;
  }
  catch (const NameNotFound& e)
  {
    DebugPrintf("%s: interaction class %s not found: %s", __FUNCTION__, name.c_str(), e.what());
    throw;
  }
  catch (const Exception& e)
  {
    DebugPrintf("%s: Exception: %s", __FUNCTION__, e.what());
    throw;
  }
}

MomManager::MomTreeNodeBuilder& MomManager::MomTreeNodeBuilder::parameter(std::string name, std::string datatype)
{
  if (mContextStack.size() == 0)
    throw RTIinternalError("parameter cannot be root node");

  if (mContextStack.front()->getType() != MomType::Interaction)
    throw RTIinternalError("parameter can only be added to an interaction");

  try
  {
    ParameterHandle handle = mMomServer->getParameterHandle(mContextStack.front()->interactionClassHandle, name);
    assert(handle.valid());
    MomTreeNode* node = new MomTreeNode(name, MomType::Parameter, handle, datatype);
    this->add(node);
    return *this;
  }
  catch (const NameNotFound& e)
  {
    DebugPrintf("%s: parameter %s not found: %s", __FUNCTION__, name.c_str(), e.what());
    DebugPrintf("%s:  in interaction class %s\n", __FUNCTION__, mMomServer->getInteractionClassName(mContextStack.front()->interactionClassHandle).c_str());
    throw;
  }
  catch (const Exception& e)
  {
    DebugPrintf("%s: Exception: %s", __FUNCTION__, e.what());
    throw;
  }
}
#pragma endregion

}