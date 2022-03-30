
#include "DebugNew.h"
#include "MomFederate.h"
#include "MomManager.h"
#include "MomFederateMetrics.h"
#include "OpenRTIVersion.h"

#include "dprintf.h"

namespace OpenRTI {

#pragma region class MomFederate
MomFederate::MomFederate(std::shared_ptr<MomManager> mom, FederateHandle federateHandle, ObjectInstanceHandle federateObjectInstance)
  : mFOMmoduleDesignatorList(HLAunicodeString())
{
  mFederateObjectInstance = federateObjectInstance;
  mInternalFederateHandle = federateHandle;
  //DebugPrintf("%s: federateHandle=%s\n", __FUNCTION__, mInternalFederateHandle.toString().c_str());

  // get handles
  MomManager::ObjectClassAccessor federateObject = mom->objects["HLAobjectRoot"]["HLAmanager"]["HLAfederate"];
  mFederateClassHandle = federateObject;

  // plumb attribute encodings with attributes and getter/setter functions
  AttributeHandle federateHandleAttribute = federateObject.attributes["HLAfederateHandle"];
  mAttributeGetters[federateHandleAttribute] = [this] { return mFederateHandle.encode(); };
  mAttributeSetters[federateHandleAttribute] = [this](const VariableLengthData& data) { mFederateHandle.decode(data); };

  AttributeHandle federateNameAttribute = federateObject.attributes["HLAfederateName"];
  mAttributeGetters[federateNameAttribute] = [this]{ return mFederateName.encode(); };
  mAttributeSetters[federateNameAttribute] = [this](const VariableLengthData& data) { mFederateName.decode(data); };

  AttributeHandle federateTypeAttribute = federateObject.attributes["HLAfederateType"];
  mAttributeGetters[federateTypeAttribute] = [this]{ return mFederateType.encode(); };
  mAttributeSetters[federateTypeAttribute] = [this](const VariableLengthData& data) { mFederateType.decode(data); };

  AttributeHandle federateHostAttribute = federateObject.attributes["HLAfederateHost"];
  mAttributeGetters[federateHostAttribute] = [this]{ return mFederateHost.encode(); };
  mAttributeSetters[federateHostAttribute] = [this](const VariableLengthData& data) { mFederateHost.decode(data); };

  AttributeHandle rtiVersionAttribute = federateObject.attributes["HLARTIversion"];
  mAttributeGetters[rtiVersionAttribute] = [this]{ return mFederateRTIversion.encode(); };
  mAttributeSetters[rtiVersionAttribute] = [this](const VariableLengthData& data) { mFederateRTIversion.decode(data); };

  AttributeHandle hlaFOMmoduleDesignatorListAttribute = federateObject.attributes["HLAFOMmoduleDesignatorList"];
  mAttributeGetters[hlaFOMmoduleDesignatorListAttribute] = [this]{ return mFOMmoduleDesignatorList.encode(); };
  mAttributeSetters[hlaFOMmoduleDesignatorListAttribute] = [this](const VariableLengthData& data) { mFOMmoduleDesignatorList.decode(data); };

  AttributeHandle timeConstrainedAttribute = federateObject.attributes["HLAtimeConstrained"];
  mAttributeGetters[timeConstrainedAttribute] = [this]{ return mTimeConstrained.encode(); };
  mAttributeSetters[timeConstrainedAttribute] = [this](const VariableLengthData& data) { mTimeConstrained.decode(data); };

  AttributeHandle timeRegulatingAttribute = federateObject.attributes["HLAtimeRegulating"];
  mAttributeGetters[timeRegulatingAttribute] = [this]{ return mTimeRegulating.encode(); };
  mAttributeSetters[timeRegulatingAttribute] = [this](const VariableLengthData& data) { mTimeRegulating.decode(data); };

  AttributeHandle asynchronousDeliveryAttribute = federateObject.attributes["HLAasynchronousDelivery"];
  mAttributeGetters[asynchronousDeliveryAttribute] = [this]{ return mAsynchronousDelivery.encode(); };
  mAttributeSetters[asynchronousDeliveryAttribute] = [this](const VariableLengthData& data) { mAsynchronousDelivery.decode(data); };

  AttributeHandle reflectionsReceivedAttribute = federateObject.attributes["HLAreflectionsReceived"];
  mAttributeGetters[reflectionsReceivedAttribute] = [this]{ return mReflectionsReceived.encode(); };
  mAttributeSetters[reflectionsReceivedAttribute] = [this](const VariableLengthData& data) { mReflectionsReceived.decode(data); };

  AttributeHandle updatesSentAttribute = federateObject.attributes["HLAupdatesSent"];
  mAttributeGetters[updatesSentAttribute] = [this]{ return mUpdatesSent.encode(); };
  mAttributeSetters[updatesSentAttribute] = [this](const VariableLengthData& data) { mUpdatesSent.decode(data); };

  AttributeHandle interactionsReceivedAttribute = federateObject.attributes["HLAinteractionsReceived"];
  mAttributeGetters[interactionsReceivedAttribute] = [this]{ return mInteractionsReceived.encode(); };
  mAttributeSetters[interactionsReceivedAttribute] = [this](const VariableLengthData& data) { mInteractionsReceived.decode(data); };

  AttributeHandle interactionsSentAttribute = federateObject.attributes["HLAinteractionsSent"];
  mAttributeGetters[interactionsSentAttribute] = [this]{ return mInteractionsSent.encode(); };
  mAttributeSetters[interactionsSentAttribute] = [this](const VariableLengthData& data) { mInteractionsSent.decode(data); };

  AttributeHandle objectInstancesThatCanBeDeletedAttribute = federateObject.attributes["HLAobjectInstancesThatCanBeDeleted"];
  mAttributeGetters[objectInstancesThatCanBeDeletedAttribute] = [this]{ return mObjectInstancesThatCanBeDeleted.encode(); };
  mAttributeSetters[objectInstancesThatCanBeDeletedAttribute] = [this](const VariableLengthData& data) { mObjectInstancesThatCanBeDeleted.decode(data); };

  AttributeHandle objectInstancesUpdatedAttribute = federateObject.attributes["HLAobjectInstancesUpdated"];
  mAttributeGetters[objectInstancesUpdatedAttribute] = [this]{ return mObjectInstancesUpdated.encode(); };
  mAttributeSetters[objectInstancesUpdatedAttribute] = [this](const VariableLengthData& data) { mObjectInstancesUpdated.decode(data); };

  AttributeHandle objectInstancesReflectedAttribute = federateObject.attributes["HLAobjectInstancesReflected"];
  mAttributeGetters[objectInstancesReflectedAttribute] = [this]{ return mObjectInstancesReflected.encode(); };
  mAttributeSetters[objectInstancesReflectedAttribute] = [this](const VariableLengthData& data) { mObjectInstancesReflected.decode(data); };

  AttributeHandle objectInstancesDeletedAttribute = federateObject.attributes["HLAobjectInstancesDeleted"];
  mAttributeGetters[objectInstancesDeletedAttribute] = [this]{ return mObjectInstancesDeleted.encode(); };
  mAttributeSetters[objectInstancesDeletedAttribute] = [this](const VariableLengthData& data) { mObjectInstancesDeleted.decode(data); };

  AttributeHandle objectInstancesRemovedAttribute = federateObject.attributes["HLAobjectInstancesRemoved"];
  mAttributeGetters[objectInstancesRemovedAttribute] = [this]{ return mObjectInstancesRemoved.encode(); };
  mAttributeSetters[objectInstancesRemovedAttribute] = [this](const VariableLengthData& data) { mObjectInstancesRemoved.decode(data); };

  AttributeHandle objectInstancesRegisteredAttribute = federateObject.attributes["HLAobjectInstancesRegistered"];
  mAttributeGetters[objectInstancesRegisteredAttribute] = [this]{ return mObjectInstancesRegistered.encode(); };
  mAttributeSetters[objectInstancesRegisteredAttribute] = [this](const VariableLengthData& data) { mObjectInstancesRegistered.decode(data); };

  AttributeHandle objectInstancesDiscoveredAttribute = federateObject.attributes["HLAobjectInstancesDiscovered"];
  mAttributeGetters[objectInstancesDiscoveredAttribute] = [this]{ return mObjectInstancesDiscovered.encode(); };
  mAttributeSetters[objectInstancesDiscoveredAttribute] = [this](const VariableLengthData& data) { mObjectInstancesDiscovered.decode(data); };

  AttributeHandle federateTimeGrantedTimeAttribute = federateObject.attributes["HLAtimeGrantedTime"];
  mAttributeGetters[federateTimeGrantedTimeAttribute] = [this]{ return mFederateTimeGrantedTime.encode(); };
  mAttributeSetters[federateTimeGrantedTimeAttribute] = [this](const VariableLengthData& data) { mFederateTimeGrantedTime.decode(data); };

  AttributeHandle federateTimeAdvancingTimeAttribute = federateObject.attributes["HLAtimeAdvancingTime"];
  mAttributeGetters[federateTimeAdvancingTimeAttribute] = [this]{ return mFederateTimeAdvancingTime.encode(); };
  mAttributeSetters[federateTimeAdvancingTimeAttribute] = [this](const VariableLengthData& data) { mFederateTimeAdvancingTime.decode(data); };

}


MomFederate::~MomFederate() noexcept
{
  //DebugPrintf("%s: federateHandle=%s\n", __FUNCTION__, mInternalFederateHandle.toString().c_str());
}

AttributeHandleValueMap MomFederate::GetAttributeValues(AttributeHandleSet requestedAttributes)
{

  AttributeHandleValueMap providedAttributes;
  for (auto attribute : requestedAttributes)
  {
    providedAttributes[attribute] = mAttributeGetters[attribute]();
  }
  return providedAttributes;
}


void MomFederate::SetAttributeValues(const AttributeHandleValueMap& attributes)
{
  for (auto it_attributeHandle_attributeValue : attributes)
  {
    if (mAttributeSetters.find(it_attributeHandle_attributeValue.first) != mAttributeSetters.end())
    {
      mAttributeSetters[it_attributeHandle_attributeValue.first](it_attributeHandle_attributeValue.second);
    }
  }
}

AttributeHandleSet MomFederate::GetKnownAttributes() const
{
  AttributeHandleSet result;
  for (auto& entry : mAttributeGetters)
  {
    result.insert(entry.first);
  }
  return result;
}


std::vector<std::wstring> MomFederate::GetFOMmoduleDesignatorList() const
{
  std::vector<std::wstring> result(mFOMmoduleDesignatorList.size());

  for (size_t index = 0; index < mFOMmoduleDesignatorList.size(); index++)
  {
    HLAunicodeString element = static_cast<const HLAunicodeString&>(mFOMmoduleDesignatorList.get(index));
    result[index] = element.get();
  }
  return result;
}

void MomFederate::SetFOMmoduleDesignatorList(const std::vector<std::wstring>& fomDesignatorList)
{
  for (auto fomDesignator : fomDesignatorList)
  {
    mFOMmoduleDesignatorList.addElement(HLAunicodeString(fomDesignator));
  }
}

OpenRTI::VariableLengthData MomFederate::GetEncodedFederateHandle() const
{
  return mFederateHandle.encode();
}

void MomFederate::insertFOMModule(const FOMModule& /*fomModule*/)
{

}

#pragma endregion
}