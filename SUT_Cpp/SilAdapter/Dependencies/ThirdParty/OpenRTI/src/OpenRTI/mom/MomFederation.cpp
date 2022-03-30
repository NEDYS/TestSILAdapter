
#include "DebugNew.h"
#include "MomFederation.h"
#include "MomFederate.h"
#include "MomManager.h"
#include "OpenRTIVersion.h"
#include "dprintf.h"

namespace OpenRTI {

#pragma region class MomFederation
MomFederation::MomFederation(std::shared_ptr<MomManager> mom, std::wstring federationName, ObjectInstanceHandle federationObjectInstance)
  : mFederatesInFederation(HLAhandle())
  , mFOMmoduleDesignatorList(HLAunicodeString())
{
  MomManager::ObjectClassAccessor federationObject = mom->objects["HLAobjectRoot"]["HLAmanager"]["HLAfederation"];
  mFederationClassHandle = federationObject;
  mFederationObjectInstance = federationObjectInstance;
  
  // plumb attributes with encoders
  AttributeHandle federationNameAttribute = federationObject.attributes["HLAfederationName"];
  mAttributeGetters[federationNameAttribute] = [this] { return mFederationName.encode(); };
  mAttributeSetters[federationNameAttribute] = [this](const VariableLengthData& data) { mFederationName.decode(data); };

  AttributeHandle federatesInFederationAttribute = federationObject.attributes["HLAfederatesInFederation"];
  mAttributeGetters[federatesInFederationAttribute] = [this] { return mFederatesInFederation.encode(); };
  mAttributeSetters[federatesInFederationAttribute] = [this](const VariableLengthData& data) { mFederatesInFederation.decode(data); };

  AttributeHandle rtiVersionAttribute = federationObject.attributes["HLARTIversion"];
  mAttributeGetters[rtiVersionAttribute] = [this] { return mRTIversion.encode(); };
  mAttributeSetters[rtiVersionAttribute] = [this](const VariableLengthData& data) { mRTIversion.decode(data); };

  AttributeHandle MIMdesignatorAttribute = federationObject.attributes["HLAMIMdesignator"];
  mAttributeGetters[MIMdesignatorAttribute] = [this] { return mMIMdesignator.encode(); };
  mAttributeSetters[MIMdesignatorAttribute] = [this](const VariableLengthData& data) { mMIMdesignator.decode(data); };

  AttributeHandle FOMmoduleDesignatorListAttribute = federationObject.attributes["HLAFOMmoduleDesignatorList"];
  mAttributeGetters[FOMmoduleDesignatorListAttribute] = [this] { return mFOMmoduleDesignatorList.encode(); };
  mAttributeSetters[FOMmoduleDesignatorListAttribute] = [this](const VariableLengthData& data) { mFOMmoduleDesignatorList.decode(data); };

  AttributeHandle currentFDDAttribute = federationObject.attributes["HLAcurrentFDD"];
  mAttributeGetters[currentFDDAttribute] = [this] { return mCurrentFDD.encode(); };
  mAttributeSetters[currentFDDAttribute] = [this](const VariableLengthData& data) { mCurrentFDD.decode(data); };

  SetFederationName(federationName);
  SetRTIversion(utf8ToUcs(OPENRTI_FILE_VERSION_STRING));
}


AttributeHandleValueMap MomFederation::GetAttributeValues(AttributeHandleSet requestedAttributes)
{
  AttributeHandleValueMap providedAttributes;
  for (auto attribute : requestedAttributes)
  {
    providedAttributes[attribute] = mAttributeGetters[attribute]();
  }
  return providedAttributes;
}


void MomFederation::SetAttributeValues(const AttributeHandleValueMap& attributes)
{
  for (auto it_attributeHandle_attributeValue : attributes)
  {
    if (mAttributeSetters.find(it_attributeHandle_attributeValue.first) != mAttributeSetters.end())
    {
      mAttributeSetters[it_attributeHandle_attributeValue.first](it_attributeHandle_attributeValue.second);
    }
  }
}


AttributeHandleSet MomFederation::GetKnownAttributes() const
{
  AttributeHandleSet result;
  for (auto& entry : mAttributeGetters)
  {
    result.insert(entry.first);
  }
  return result;
}

void MomFederation::AddFederate(std::shared_ptr<MomFederate> federate)
{
  mFederates.insert(federate);
  assert(federate->GetInternalFederateObjectInstance().valid());
  assert(federate->GetInternalFederateHandle().valid());
  mFederatesByObject.insert(std::make_pair(federate->GetInternalFederateObjectInstance(), federate));
  mFederatesByHandle.insert(std::make_pair(federate->GetInternalFederateHandle(), federate));
  std::vector<FederateHandle> federateHandles;
  for (auto fedEntry : mFederatesByHandle)
  {
    federateHandles.push_back(fedEntry.first);
  }
  SetFederatesInFederation(federateHandles);
}

void MomFederation::RemoveFederate(std::shared_ptr<MomFederate> federate)
{
  mFederates.erase(federate);
  mFederatesByObject.erase(federate->GetInternalFederateObjectInstance());
  mFederatesByHandle.erase(federate->GetInternalFederateHandle());
  std::vector<FederateHandle> federateHandles;
  for (auto fedEntry : mFederatesByHandle)
  {
    federateHandles.push_back(fedEntry.first);
  }
  SetFederatesInFederation(federateHandles);

  assert(mFederatesByObject.size() == mFederates.size());
  assert(mFederatesByHandle.size() == mFederates.size());
  assert(mFederatesInFederation.size() == mFederates.size());
}

std::shared_ptr<MomFederate> MomFederation::GetFederate(ObjectInstanceHandle federateObjectInstance) const
{
  auto iter = mFederatesByObject.find(federateObjectInstance);
  if (iter != mFederatesByObject.end())
    return iter->second;
  else
    return std::shared_ptr<MomFederate>();
}

std::shared_ptr<MomFederate> MomFederation::GetFederate(FederateHandle federateHandle) const
{
  auto iter = mFederatesByHandle.find(federateHandle);
  if (iter != mFederatesByHandle.end())
    return iter->second;
  else
    return std::shared_ptr<MomFederate>();
}

std::vector<FederateHandle> MomFederation::GetFederatesInFederation() const
{
  std::vector<FederateHandle> result;
  result.reserve(mFederatesInFederation.size());
  for (size_t i = 0; i < mFederatesInFederation.size(); i++)
  {
    const HLAhandle& handle = static_cast<const HLAhandle&>(mFederatesInFederation.get(i));
    assert(handle.getFederateHandle().valid());
    result.push_back(handle.getFederateHandle());
  }
  //DebugPrintf("%s: %d federates\n", __FUNCTION__, result.size());
  return result;
}

void MomFederation::SetFederatesInFederation(const std::vector<FederateHandle>& value)
{
  //DebugPrintf("%s: %d federates\n", __FUNCTION__, value.size());
  HLAvariableArray newValue = HLAvariableArray(HLAhandle(FederateHandle()));
  for (auto handle : value)
  {
    assert(handle.valid());
    newValue.addElement(HLAhandle(handle));
  }
  mFederatesInFederation = newValue;
}

std::vector<std::wstring> MomFederation::GetFOMmoduleDesignatorList() const
{
  std::vector<std::wstring> result(mFOMmoduleDesignatorList.size());

  for (size_t index = 0; index < mFOMmoduleDesignatorList.size(); index++)
  {
    HLAunicodeString element = static_cast<const HLAunicodeString&>(mFOMmoduleDesignatorList.get(index));
    result[index] = element.get();
  }
  return result;
}

void MomFederation::SetFOMmoduleDesignatorList(const std::vector<std::wstring>& fomDesignatorList)
{
  HLAvariableArray newValue = HLAvariableArray(HLAunicodeString());
  for (auto fomDesignator : fomDesignatorList)
  {
    newValue.addElement(HLAunicodeString(fomDesignator));
  }
  mFOMmoduleDesignatorList = newValue;
}


#pragma endregion
}