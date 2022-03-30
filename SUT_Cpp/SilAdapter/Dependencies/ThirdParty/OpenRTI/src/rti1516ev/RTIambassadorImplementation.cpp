/* -*-c++-*- OpenRTI - Copyright (C) 2009-2015 Mathias Froehlich
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

// This time, the first include is above the api include.
// because of unique_ptr in the ambassador header
#include <memory>
#include "DebugNew.h"

#include "RTIambassadorImplementation.h"

#include <algorithm>
#include <fstream>

#include <RTI/RTIambassador.h>
#include <RTI/FederateAmbassador.h>
#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/RangeBounds.h>

#include "Ambassador.h"
#include "DynamicModule.h"
#include "FDD1516EFileReader.h"
#include "LogStream.h"
#include "TemplateTimeManagement.h"

#include "HandleImplementation.h"
#include "RTI1516ELogicalTimeFactory.h"
#include "RTI1516Einteger64TimeFactory.h"
#include "RTI1516Efloat64TimeFactory.h"
#include "VariableLengthDataImplementation.h"

// Embed the HLAstandardMIM hard into the library as a last resort

// 'cast truncates constant value'
#pragma warning(disable: 4310)

#include "HLAstandardMIM.inc"
#include "AbstractNetworkStatistics.h"

namespace OpenRTI {

static std::list<std::string> findHLAstandardMIMFileCandidates()
{
  std::list<std::string> candidates;

  std::string moduleName = DynamicModule::getFileNameForAddress((const void*)findHLAstandardMIMFileCandidates);
  if (!moduleName.empty()) {
    moduleName = getBasePart(moduleName);
    // This gets us to the parent directory
    moduleName = getBasePart(moduleName);
    // FIXME: moduleName depends on the build path, the shared MIM not -> CTests can't find the file
    candidates.push_back(moduleName + std::string("/share/OpenRTI/rti1516ev/HLAstandardMIM.xml"));
  }

  // Puh, don't know the encoding of this definition originating from cmake.
  // The user of this result assumes this to be utf8
  candidates.push_back(OPENRTI_DATAROOTDIR "/rti1516ev/HLAstandardMIM.xml");

  return candidates;
}

static void loadModuleFromStream(OpenRTI::FOMStringModule2List& fomModuleList, std::istream& stream, const std::string& designator, const std::string& encoding)
{
  try {
    FOMStringModule2 module = OpenRTI::FDD1516EFileReader::read(stream, encoding);
    module.setDesignator(designator);
    fomModuleList.push_back(std::move(module));
  } catch (const OpenRTI::Exception& e) {
    throw rti1516ev::ErrorReadingFDD(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown error while reading fdd file");
  }
}

static void loadHLAstandardMIM(OpenRTI::FOMStringModule2List& fomModuleList)
{
  std::list<std::string> fileCandidates = findHLAstandardMIMFileCandidates();
  for (auto designator : fileCandidates) {
    std::ifstream stream;
    stream.open(utf8ToLocale(designator).c_str());
    if (!stream.is_open())
      continue;

    loadModuleFromStream(fomModuleList, stream, designator, std::string());
    break;
  }
  if (fomModuleList.empty()) {
    std::string s(HLAstandardMIM_xml, sizeof(HLAstandardMIM_xml));
    std::istringstream stream(s);
    loadModuleFromStream(fomModuleList, stream, "HLAStandardMIM.xml", "UTF-8");
  }
}

static void loadModuleFromFile(OpenRTI::FOMStringModule2List& fomModuleList, const std::string& fileName)
{
  std::string designator = std::string("file:///") + fileName;
  std::ifstream stream(fileName.c_str());
  if (stream.is_open()) {
    loadModuleFromStream(fomModuleList, stream, designator, std::string());
  } else {
    throw rti1516ev::CouldNotOpenFDD(utf8ToUcs(designator));
  }
}

static void loadModule(OpenRTI::FOMStringModule2List& fomModuleList, const std::wstring& fomModuleDesignator)
{
  if (fomModuleDesignator.empty())
    throw rti1516ev::CouldNotOpenFDD(L"Empty module.");
  FOMModule module;
  if (fomModuleDesignator.compare(0, 8, L"file:///") == 0) {
    loadModuleFromFile(fomModuleList, ucsToUtf8(fomModuleDesignator.substr(8)));
  } else if (fomModuleDesignator.compare(0, 16, L"data:text/plain,") == 0) {
    std::stringstream stream(ucsToUtf8(fomModuleDesignator.substr(16)));
    std::string designator = string_printf("data:text/plain,<%d bytes>", fomModuleDesignator.size()-16);
    loadModuleFromStream(fomModuleList, stream, ucsToUtf8(fomModuleDesignator), "UTF-8");
  } else if (fomModuleDesignator.compare(0, 6, L"data:,") == 0) {
    std::stringstream stream(ucsToUtf8(fomModuleDesignator.substr(6)));
    std::string designator = string_printf("data:,<%d bytes>", fomModuleDesignator.size()-6);
    loadModuleFromStream(fomModuleList, stream, designator, "UTF-8");
  } else {
    loadModuleFromFile(fomModuleList, ucsToUtf8(fomModuleDesignator));
  }
}

static void loadModules(OpenRTI::FOMStringModule2List& fomModuleList, const std::vector<std::wstring>& fomModules)
{
  for (auto& fomModule : fomModules)
    loadModule(fomModuleList, fomModule);
}

// unreferenced local function has been removed
#pragma warning(disable: 4505)

static OpenRTI::CallbackModel translate(rti1516ev::CallbackModel callbackModel)
{
  switch (callbackModel) {
  case rti1516ev::HLA_IMMEDIATE:
    return OpenRTI::HLA_IMMEDIATE;
  case rti1516ev::HLA_EVOKED:
  default:
    return OpenRTI::HLA_EVOKED;
  }
}

static rti1516ev::OrderType translate(OpenRTI::OrderType orderType)
{
  switch (orderType) {
  case OpenRTI::TIMESTAMP:
    return rti1516ev::TIMESTAMP;
  case OpenRTI::RECEIVE:
  default:
    return rti1516ev::RECEIVE;
  }
}

static OpenRTI::OrderType translate(rti1516ev::OrderType orderType)
{
  switch (orderType) {
  case rti1516ev::TIMESTAMP:
    return OpenRTI::TIMESTAMP;
  case rti1516ev::RECEIVE:
  default:
    return OpenRTI::RECEIVE;
  }
}

static rti1516ev::TransportationType translate(OpenRTI::TransportationType transportationType)
{
  switch (transportationType) {
  case OpenRTI::BEST_EFFORT:
    return rti1516ev::BEST_EFFORT;
  case OpenRTI::RELIABLE:
  default:
    return rti1516ev::RELIABLE;
  }
}

static OpenRTI::TransportationType translate(rti1516ev::TransportationType transportationType)
{
  switch (transportationType) {
  case rti1516ev::BEST_EFFORT:
    return OpenRTI::BEST_EFFORT;
  case rti1516ev::RELIABLE:
  default:
    return OpenRTI::RELIABLE;
  }
}

static OpenRTI::ResignAction translate(rti1516ev::ResignAction resignAction)
{
  switch (resignAction) {
  case rti1516ev::UNCONDITIONALLY_DIVEST_ATTRIBUTES:
    return OpenRTI::UNCONDITIONALLY_DIVEST_ATTRIBUTES;
  case rti1516ev::DELETE_OBJECTS:
    return OpenRTI::DELETE_OBJECTS;
  case rti1516ev::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS:
    return OpenRTI::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS;
  case rti1516ev::DELETE_OBJECTS_THEN_DIVEST:
    return OpenRTI::DELETE_OBJECTS_THEN_DIVEST;
  case rti1516ev::CANCEL_THEN_DELETE_THEN_DIVEST:
    return OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST;
  case rti1516ev::NO_ACTION:
    return OpenRTI::NO_ACTION;
  default:
    return OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST;
  }
}

static rti1516ev::ResignAction translate(OpenRTI::ResignAction resignAction)
{
  switch (resignAction) {
  case OpenRTI::UNCONDITIONALLY_DIVEST_ATTRIBUTES:
    return rti1516ev::UNCONDITIONALLY_DIVEST_ATTRIBUTES;
  case OpenRTI::DELETE_OBJECTS:
    return rti1516ev::DELETE_OBJECTS;
  case OpenRTI::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS:
    return rti1516ev::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS;
  case OpenRTI::DELETE_OBJECTS_THEN_DIVEST:
    return rti1516ev::DELETE_OBJECTS_THEN_DIVEST;
  case OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST:
    return rti1516ev::CANCEL_THEN_DELETE_THEN_DIVEST;
  case OpenRTI::NO_ACTION:
    return rti1516ev::NO_ACTION;
  default:
    return rti1516ev::CANCEL_THEN_DELETE_THEN_DIVEST;
  }
}

static OpenRTI::ServiceGroupIndicator translate(rti1516ev::ServiceGroup serviceGroup)
{
  switch (serviceGroup) {
  case rti1516ev::FEDERATION_MANAGEMENT:
    return OpenRTI::FEDERATION_MANAGEMENT;
  case rti1516ev::DECLARATION_MANAGEMENT:
    return OpenRTI::DECLARATION_MANAGEMENT;
  case rti1516ev::OBJECT_MANAGEMENT:
    return OpenRTI::OBJECT_MANAGEMENT;
  case rti1516ev::OWNERSHIP_MANAGEMENT:
    return OpenRTI::OWNERSHIP_MANAGEMENT;
  case rti1516ev::TIME_MANAGEMENT:
    return OpenRTI::TIME_MANAGEMENT;
  case rti1516ev::DATA_DISTRIBUTION_MANAGEMENT:
    return OpenRTI::DATA_DISTRIBUTION_MANAGEMENT;
  case rti1516ev::SUPPORT_SERVICES:
    return OpenRTI::SUPPORT_SERVICES;
  default:
    throw OpenRTI::InvalidServiceGroup();
  }
}

class OPENRTI_LOCAL _I1516EFederateHandle : public OpenRTI::FederateHandle {
public:
  _I1516EFederateHandle(const rti1516ev::FederateHandle& federateHandle)
  { rti1516ev::FederateHandleFriend::copy(*this, federateHandle); }
};
class OPENRTI_LOCAL _I1516EObjectClassHandle : public OpenRTI::ObjectClassHandle {
public:
  _I1516EObjectClassHandle(const rti1516ev::ObjectClassHandle& objectClassHandle)
  { rti1516ev::ObjectClassHandleFriend::copy(*this, objectClassHandle); }
};
class OPENRTI_LOCAL _I1516EInteractionClassHandle : public OpenRTI::InteractionClassHandle {
public:
  _I1516EInteractionClassHandle(const rti1516ev::InteractionClassHandle& interactionClassHandle)
  { rti1516ev::InteractionClassHandleFriend::copy(*this, interactionClassHandle); }
};
class OPENRTI_LOCAL _I1516EObjectInstanceHandle : public OpenRTI::ObjectInstanceHandle {
public:
  _I1516EObjectInstanceHandle(const rti1516ev::ObjectInstanceHandle& objectInstanceHandle)
  { rti1516ev::ObjectInstanceHandleFriend::copy(*this, objectInstanceHandle); }
};
class OPENRTI_LOCAL _I1516EAttributeHandle : public OpenRTI::AttributeHandle {
public:
  _I1516EAttributeHandle(const rti1516ev::AttributeHandle& attributeHandle)
  { rti1516ev::AttributeHandleFriend::copy(*this, attributeHandle); }
};
class OPENRTI_LOCAL _I1516EParameterHandle : public OpenRTI::ParameterHandle {
public:
  _I1516EParameterHandle(const rti1516ev::ParameterHandle& parameterHandle)
  { rti1516ev::ParameterHandleFriend::copy(*this, parameterHandle); }
};
class OPENRTI_LOCAL _I1516EDimensionHandle : public OpenRTI::DimensionHandle {
public:
  _I1516EDimensionHandle(const rti1516ev::DimensionHandle& dimensionHandle)
  { rti1516ev::DimensionHandleFriend::copy(*this, dimensionHandle); }
};
class OPENRTI_LOCAL _I1516ERegionHandle : public OpenRTI::RegionHandle {
public:
  _I1516ERegionHandle(const rti1516ev::RegionHandle& regionHandle)
  { rti1516ev::RegionHandleFriend::copy(*this, regionHandle); }
};
class OPENRTI_LOCAL _I1516EMessageRetractionHandle : public OpenRTI::MessageRetractionHandle {
public:
  _I1516EMessageRetractionHandle(const rti1516ev::MessageRetractionHandle& messageRetractionHandle)
  { rti1516ev::MessageRetractionHandleFriend::copy(*this, messageRetractionHandle); }
};
class OPENRTI_LOCAL _I1516EVariableLengthData : public OpenRTI::VariableLengthData {
public:
  _I1516EVariableLengthData(const rti1516ev::VariableLengthData& variableLengthData) :
    VariableLengthData(rti1516ev::VariableLengthDataFriend::readPointer(variableLengthData))
  { }
};
class OPENRTI_LOCAL _I1516ERangeBounds : public OpenRTI::RangeBounds {
public:
  _I1516ERangeBounds(const rti1516ev::RangeBounds& rangeBounds) :
    RangeBounds(rangeBounds.getLowerBound(), rangeBounds.getUpperBound())
  { }
};

class OPENRTI_LOCAL _I1516EFederateHandleVector : public OpenRTI::FederateHandleVector {
public:
  _I1516EFederateHandleVector(const rti1516ev::FederateHandleSet& federateHandleSet)
  {
    reserve(federateHandleSet.size() + 1);
    for (rti1516ev::FederateHandleSet::const_iterator i = federateHandleSet.begin(); i != federateHandleSet.end(); ++i)
      push_back(OpenRTI::_I1516EFederateHandle(*i));
  }
};
class OPENRTI_LOCAL _I1516EAttributeHandleVector : public OpenRTI::AttributeHandleVector {
public:
  _I1516EAttributeHandleVector(const rti1516ev::AttributeHandleSet& attributeHandleSet)
  {
    reserve(attributeHandleSet.size() + 1);
    for (rti1516ev::AttributeHandleSet::const_iterator i = attributeHandleSet.begin(); i != attributeHandleSet.end(); ++i)
      push_back(OpenRTI::_I1516EAttributeHandle(*i));
  }
};
class OPENRTI_LOCAL _I1516EDimensionHandleSet : public OpenRTI::DimensionHandleSet {
public:
  _I1516EDimensionHandleSet(const rti1516ev::DimensionHandleSet& dimensionHandleSet)
  {
    for (rti1516ev::DimensionHandleSet::const_iterator i = dimensionHandleSet.begin(); i != dimensionHandleSet.end(); ++i)
      insert(end(), OpenRTI::_I1516EDimensionHandle(*i));
  }
};
class OPENRTI_LOCAL _I1516ERegionHandleVector : public OpenRTI::RegionHandleVector {
public:
  _I1516ERegionHandleVector(const rti1516ev::RegionHandleSet& regionHandleSet)
  {
    reserve(regionHandleSet.size());
    for (rti1516ev::RegionHandleSet::const_iterator i = regionHandleSet.begin(); i != regionHandleSet.end(); ++i)
      push_back(OpenRTI::_I1516ERegionHandle(*i));
  }
};

class OPENRTI_LOCAL _I1516EAttributeHandleVectorRegionHandleVectorPairVector : public OpenRTI::AttributeHandleVectorRegionHandleVectorPairVector {
public:
  _I1516EAttributeHandleVectorRegionHandleVectorPairVector(const rti1516ev::AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector)
  {
    reserve(attributeHandleSetRegionHandleSetPairVector.size());
    for (rti1516ev::AttributeHandleSetRegionHandleSetPairVector::const_iterator i = attributeHandleSetRegionHandleSetPairVector.begin();
         i != attributeHandleSetRegionHandleSetPairVector.end(); ++i) {
      push_back(OpenRTI::AttributeHandleVectorRegionHandleVectorPair());
      _I1516EAttributeHandleVector(i->first).swap(back().first);
      _I1516ERegionHandleVector(i->second).swap(back().second);
    }
  }
};

class OPENRTI_LOCAL _I1516EAttributeValueVector : public OpenRTI::AttributeValueVector {
public:
  _I1516EAttributeValueVector(const rti1516ev::AttributeHandleValueMap& attributeHandleValueMap)
  {
    reserve(attributeHandleValueMap.size());
    for (rti1516ev::AttributeHandleValueMap::const_iterator i = attributeHandleValueMap.begin();
         i != attributeHandleValueMap.end(); ++i) {
      push_back(OpenRTI::AttributeValue());
      back().setAttributeHandle(OpenRTI::_I1516EAttributeHandle(i->first));
      back().setValue(rti1516ev::VariableLengthDataFriend::readPointer(i->second));
    }
  }
};
class OPENRTI_LOCAL _I1516EParameterValueVector : public OpenRTI::ParameterValueVector {
public:
  _I1516EParameterValueVector(const rti1516ev::ParameterHandleValueMap& parameterHandleValueMap)
  {
    reserve(parameterHandleValueMap.size());
    for (rti1516ev::ParameterHandleValueMap::const_iterator i = parameterHandleValueMap.begin();
         i != parameterHandleValueMap.end(); ++i) {
      push_back(OpenRTI::ParameterValue());
      back().setParameterHandle(OpenRTI::_I1516EParameterHandle(i->first));
      back().setValue(rti1516ev::VariableLengthDataFriend::readPointer(i->second));
    }
  }
};

class OPENRTI_LOCAL _O1516EFederateHandle : public rti1516ev::FederateHandle {
public:
  _O1516EFederateHandle(const OpenRTI::FederateHandle& federateHandle)
  { rti1516ev::FederateHandleFriend::copy(*this, federateHandle); }
};
class OPENRTI_LOCAL _O1516EObjectClassHandle : public rti1516ev::ObjectClassHandle {
public:
  _O1516EObjectClassHandle(const OpenRTI::ObjectClassHandle& objectClassHandle)
  { rti1516ev::ObjectClassHandleFriend::copy(*this, objectClassHandle); }
};
class OPENRTI_LOCAL _O1516EInteractionClassHandle : public rti1516ev::InteractionClassHandle {
public:
  _O1516EInteractionClassHandle(const OpenRTI::InteractionClassHandle& interactionClassHandle)
  { rti1516ev::InteractionClassHandleFriend::copy(*this, interactionClassHandle); }
};
class OPENRTI_LOCAL _O1516EObjectInstanceHandle : public rti1516ev::ObjectInstanceHandle {
public:
  _O1516EObjectInstanceHandle(const OpenRTI::ObjectInstanceHandle& objectInstanceHandle)
  { rti1516ev::ObjectInstanceHandleFriend::copy(*this, objectInstanceHandle); }
};
class OPENRTI_LOCAL _O1516EAttributeHandle : public rti1516ev::AttributeHandle {
public:
  _O1516EAttributeHandle(const OpenRTI::AttributeHandle& attributeHandle)
  { rti1516ev::AttributeHandleFriend::copy(*this, attributeHandle); }
};
class OPENRTI_LOCAL _O1516EParameterHandle : public rti1516ev::ParameterHandle {
public:
  _O1516EParameterHandle(const OpenRTI::ParameterHandle& parameterHandle)
  { rti1516ev::ParameterHandleFriend::copy(*this, parameterHandle); }
};
class OPENRTI_LOCAL _O1516EDimensionHandle : public rti1516ev::DimensionHandle {
public:
  _O1516EDimensionHandle(const OpenRTI::DimensionHandle& dimensionHandle)
  { rti1516ev::DimensionHandleFriend::copy(*this, dimensionHandle); }
};
class OPENRTI_LOCAL _O1516ERegionHandle : public rti1516ev::RegionHandle {
public:
  _O1516ERegionHandle(const OpenRTI::RegionHandle& regionHandle)
  { rti1516ev::RegionHandleFriend::copy(*this, regionHandle); }
};
class OPENRTI_LOCAL _O1516EMessageRetractionHandle : public rti1516ev::MessageRetractionHandle {
public:
  _O1516EMessageRetractionHandle(const OpenRTI::MessageRetractionHandle& messageRetractionHandle)
  { rti1516ev::MessageRetractionHandleFriend::copy(*this, messageRetractionHandle); }
};
class OPENRTI_LOCAL _O1516EVariableLengthData : public rti1516ev::VariableLengthData {
public:
  _O1516EVariableLengthData(const OpenRTI::VariableLengthData& variableLengthData)
  { rti1516ev::VariableLengthDataFriend::writePointer(*this) = variableLengthData; }
};
class OPENRTI_LOCAL _O1516ERangeBounds : public rti1516ev::RangeBounds {
public:
  _O1516ERangeBounds(const OpenRTI::RangeBounds& rangeBounds) :
    rti1516ev::RangeBounds(rangeBounds.getLowerBound(), rangeBounds.getUpperBound())
  { }
};
class OPENRTI_LOCAL _O1516EString : public std::wstring {
public:
  _O1516EString(const std::string& s)
  { utf8ToUcs(s).swap(*this); }
};

class OPENRTI_LOCAL _O1516EAttributeHandleSet : public rti1516ev::AttributeHandleSet {
public:
  _O1516EAttributeHandleSet(const OpenRTI::AttributeHandleVector& attributeHandleVector)
  {
    for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin();
         i != attributeHandleVector.end(); ++i)
      insert(end(), OpenRTI::_O1516EAttributeHandle(*i));
  }
};
class OPENRTI_LOCAL _O1516EDimensionHandleSet : public rti1516ev::DimensionHandleSet {
public:
  _O1516EDimensionHandleSet(const OpenRTI::DimensionHandleSet& dimensionHandleVector)
  {
    for (OpenRTI::DimensionHandleSet::const_iterator i = dimensionHandleVector.begin();
         i != dimensionHandleVector.end(); ++i)
      insert(end(), OpenRTI::_O1516EDimensionHandle(*i));
  }
};
class OPENRTI_LOCAL _O1516EFederateHandleSet : public rti1516ev::FederateHandleSet {
public:
  _O1516EFederateHandleSet(const OpenRTI::FederateHandleBoolPairVector& federateHandleBoolPairVector)
  {
    for (OpenRTI::FederateHandleBoolPairVector::const_iterator i = federateHandleBoolPairVector.begin();
         i != federateHandleBoolPairVector.end(); ++i) {
      if (i->second)
        continue;
      insert(end(), OpenRTI::_O1516EFederateHandle(i->first));
    }
  }
};

class OPENRTI_LOCAL _O1516EAttributeHandleValueMap : public rti1516ev::AttributeHandleValueMap {
public:
  _O1516EAttributeHandleValueMap(const OpenRTI::Federate::ObjectClass& objectClass,
                                 const OpenRTI::AttributeValueVector& attributeValueVector)
  {
    for (OpenRTI::AttributeValueVector::const_iterator i = attributeValueVector.begin();
         i != attributeValueVector.end(); ++i) {
        if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == Unsubscribed)
          continue;
        rti1516ev::VariableLengthData& variableLengthData = (*this)[OpenRTI::_O1516EAttributeHandle(i->getAttributeHandle())];
        rti1516ev::VariableLengthDataFriend::writePointer(variableLengthData) = i->getValue();
    }
  }
};
class OPENRTI_LOCAL _O1516EParameterHandleValueMap : public rti1516ev::ParameterHandleValueMap {
public:
  _O1516EParameterHandleValueMap(const OpenRTI::Federate::InteractionClass& interactionClass,
                                 const OpenRTI::ParameterValueVector& parameterValueVector)
  {
    for (OpenRTI::ParameterValueVector::const_iterator i = parameterValueVector.begin();
         i != parameterValueVector.end(); ++i) {
        if (!interactionClass.getParameter(i->getParameterHandle()))
          continue;
        rti1516ev::VariableLengthData& variableLengthData = (*this)[OpenRTI::_O1516EParameterHandle(i->getParameterHandle())];
        rti1516ev::VariableLengthDataFriend::writePointer(variableLengthData) = i->getValue();
    }
  }
};

class OPENRTI_LOCAL _O1516EStringSet : public std::set<std::wstring> {
public:
  _O1516EStringSet(const OpenRTI::StringVector& stringVector)
  {
    for (OpenRTI::StringVector::const_iterator i = stringVector.begin();
           i != stringVector.end(); ++i)
      insert(utf8ToUcs(*i));
  }
};

class OPENRTI_LOCAL _O1516EFederationExecutionInformationVector : public rti1516ev::FederationExecutionInformationVector {
public:
  _O1516EFederationExecutionInformationVector(const OpenRTI::FederationExecutionInformationVector& federationExecutionInformationVector)
  {
    reserve(federationExecutionInformationVector.size());
    for (OpenRTI::FederationExecutionInformationVector::const_iterator i = federationExecutionInformationVector.begin();
         i != federationExecutionInformationVector.end(); ++i) {
      push_back(rti1516ev::FederationExecutionInformation(utf8ToUcs(i->getFederationExecutionName()),
                                                         utf8ToUcs(i->getLogicalTimeFactoryName())));
    }
  }
};

class OPENRTI_LOCAL _O1516ESupplementalReflectInfo : public rti1516ev::SupplementalReflectInfo {
public:
  _O1516ESupplementalReflectInfo(const OpenRTI::FederateHandle& federateHandle) :
    rti1516ev::SupplementalReflectInfo(_O1516EFederateHandle(federateHandle))
  { hasProducingFederate = federateHandle.valid(); }
};
class OPENRTI_LOCAL _O1516ESupplementalRemoveInfo : public rti1516ev::SupplementalRemoveInfo {
public:
  _O1516ESupplementalRemoveInfo(const OpenRTI::FederateHandle& federateHandle) :
    rti1516ev::SupplementalRemoveInfo(_O1516EFederateHandle(federateHandle))
  { hasProducingFederate = federateHandle.valid(); }
};
class OPENRTI_LOCAL _O1516ESupplementalReceiveInfo : public rti1516ev::SupplementalReceiveInfo {
public:
  _O1516ESupplementalReceiveInfo(const OpenRTI::FederateHandle& federateHandle) :
    rti1516ev::SupplementalReceiveInfo(_O1516EFederateHandle(federateHandle))
  { hasProducingFederate = federateHandle.valid(); }
};

class OPENRTI_LOCAL RTI1516ETraits {
public:
  // The bindings have different logical times
  typedef rti1516ev::LogicalTime NativeLogicalTime;
  typedef rti1516ev::LogicalTimeInterval NativeLogicalTimeInterval;
};

// This class subclasses the OpenRTI::Ambassador template and acts as a bridge between the RTIambassador and 
// the application's FederateAmbassador. It holds a reference to the FederateAmbassador and 
// translates the (OpenRTI-) internal callback messages into callback invocations of the FederateAmbassador.
// As a subclass of OpenRTI::Ambassador, this class also has access to the callback queues (inherited from InternalAmbassador).
// The MomManager must live here, because this is the place to decide whether to route callbacks into the MomManager only
// or into the FederateAmbassador also.
class OPENRTI_LOCAL RTIambassadorImplementation::RTI1516EAmbassadorInterface final : public OpenRTI::Ambassador<RTI1516ETraits> {
public:
  RTI1516EAmbassadorInterface()
    : Ambassador<RTI1516ETraits>()
    , _federateAmbassador(0)
    , _inCallback(false)
  { }

  class OPENRTI_LOCAL CallbackScope {
  public:
    CallbackScope(RTI1516EAmbassadorInterface& ambassadorInterface) noexcept :
      _ambassadorInterface(ambassadorInterface)
    { _ambassadorInterface._inCallback = true; }
    ~CallbackScope() noexcept
    { _ambassadorInterface._inCallback = false; }
  private:
    RTI1516EAmbassadorInterface& _ambassadorInterface;
  };

  std::unique_ptr<rti1516ev::LogicalTimeFactory> getTimeFactory()
  {
    // FIXME ask the time management about that
    OpenRTI::Federate* federate = getFederate();
    if (!federate)
	std::unique_ptr<rti1516ev::LogicalTimeFactory>();
    return rti1516ev::LogicalTimeFactoryFactory::makeLogicalTimeFactory(utf8ToUcs(federate->getLogicalTimeFactoryName()));
  }

  SharedPtr<TimeManagement<RTI1516ETraits>> createTimeManagement(Federate& federate) override
  {
    std::string logicalTimeFactoryName = federate.getLogicalTimeFactoryName();
    std::unique_ptr<rti1516ev::LogicalTimeFactory> logicalTimeFactory;
    logicalTimeFactory = rti1516ev::LogicalTimeFactoryFactory::makeLogicalTimeFactory(utf8ToUcs(logicalTimeFactoryName));
    if (!logicalTimeFactory.get())
      return SharedPtr<TimeManagement<RTI1516ETraits>>();

    // Get logical time and logical time interval. If they are the well known ones,
    // try to use the optimized implementation using the native time data types directly.
    // An implementation is considered equal if the implementation name is the same and they are assignable in each direction,
    // Also add a flag that forces the to use the opaque factory

    // FIXME: make that again configurable
    bool forceOpaqueTime = false;
    if (!forceOpaqueTime) {
      std::unique_ptr<rti1516ev::LogicalTime> logicalTime = logicalTimeFactory->makeInitial();
      std::unique_ptr<rti1516ev::LogicalTimeInterval> logicalTimeInterval = logicalTimeFactory->makeZero();
      try {
        rti1516ev::HLAinteger64Time time;
        rti1516ev::HLAinteger64Interval interval;
        if (time.implementationName() == logicalTime->implementationName() && 
            interval.implementationName() == logicalTimeInterval->implementationName()) {
          time = *logicalTime;
          interval = *logicalTimeInterval;
          *logicalTime = time;
          *logicalTimeInterval = interval;
          if (*logicalTime == time && *logicalTimeInterval == interval) {
            return MakeShared<TemplateTimeManagement<RTI1516ETraits, RTI1516Einteger64TimeFactory>>(RTI1516Einteger64TimeFactory(ucsToUtf8(time.implementationName())));
          }
        }
      } catch (...) {
      }

      try {
        rti1516ev::HLAfloat64Time time;
        rti1516ev::HLAfloat64Interval interval;
        if (time.implementationName() == logicalTime->implementationName() &&
            interval.implementationName() == logicalTimeInterval->implementationName()) {
          time = *logicalTime;
          interval = *logicalTimeInterval;
          *logicalTime = time;
          *logicalTimeInterval = interval;
          if (*logicalTime == time && *logicalTimeInterval == interval) {
            return MakeShared<TemplateTimeManagement<RTI1516ETraits, RTI1516Efloat64TimeFactory>>(RTI1516Efloat64TimeFactory(ucsToUtf8(time.implementationName())));
          }
        }
      } catch (...) {
      }
    }

    // Ok, we will just need to use the opaque logical time factory
    return MakeShared<TemplateTimeManagement<RTI1516ETraits, RTI1516ELogicalTimeFactory>>(RTI1516ELogicalTimeFactory(std::move(logicalTimeFactory)));
  }

  void connectionLost(const std::string& faultDescription) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EString rti1516FaultDescription(faultDescription);
      _federateAmbassador->connectionLost(rti1516FaultDescription);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void reportFederationExecutions(const OpenRTI::FederationExecutionInformationVector& federationExecutionInformationVector) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _O1516EFederationExecutionInformationVector federationInformations(federationExecutionInformationVector);
      _federateAmbassador->reportFederationExecutions(federationInformations);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void synchronizationPointRegistrationResponse(const std::string& label, RegisterFederationSynchronizationPointResponseType reason) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _O1516EString rti1516Label(label);
      switch (reason) {
      case OpenRTI::RegisterFederationSynchronizationPointResponseSuccess:
        _federateAmbassador->synchronizationPointRegistrationSucceeded(rti1516Label);
        break;
      case OpenRTI::RegisterFederationSynchronizationPointResponseLabelNotUnique:
        _federateAmbassador->synchronizationPointRegistrationFailed(rti1516Label, rti1516ev::SYNCHRONIZATION_POINT_LABEL_NOT_UNIQUE);
        break;
      case OpenRTI::RegisterFederationSynchronizationPointResponseMemberNotJoined:
        _federateAmbassador->synchronizationPointRegistrationFailed(rti1516Label, rti1516ev::SYNCHRONIZATION_SET_MEMBER_NOT_JOINED);
        break;
      }
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void announceSynchronizationPoint(const std::string& label, const OpenRTI::VariableLengthData& tag) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EString rti1516Label(label);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      _federateAmbassador->announceSynchronizationPoint(rti1516Label, rti1516Tag);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void federationSynchronized(const std::string& label, const OpenRTI::FederateHandleBoolPairVector& federateHandleBoolPairVector) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EString rti1516Label(label);
      OpenRTI::_O1516EFederateHandleSet federateHandleSet(federateHandleBoolPairVector);
      _federateAmbassador->federationSynchronized(rti1516Label, federateHandleSet);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // // 4.12
  // void initiateFederateSave(std::string const& label) override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(label);
  //   } catch (const rti1516ev::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(e.what());
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // void initiateFederateSave(const std::string& label,
  //                      const LogicalTime& logicalTime) override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(label, RTI1516ELogicalTimeImplementation::getLogicalTime(logicalTime));
  //   } catch (const rti1516ev::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(e.what());
  //   } catch (const rti1516ev::InvalidLogicalTime& e) {
  //     throw OpenRTI::InvalidLogicalTime(e.what());
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.15
  // void federationSaved() override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationSaved();
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // void federationNotSaved(OpenRTI::SaveFailureReason reason) override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();

  //   rti1516ev::SaveFailureReason rti1516Reason;
  //   switch (reason) {
  //   case OpenRTI::RTI_UNABLE_TO_SAVE:
  //     rti1516Reason = rti1516ev::RTI_UNABLE_TO_SAVE;
  //     break;
  //   case OpenRTI::FEDERATE_REPORTED_FAILURE_DURING_SAVE:
  //     rti1516Reason = rti1516ev::FEDERATE_REPORTED_FAILURE_DURING_SAVE;
  //     break;
  //   case OpenRTI::FEDERATE_RESIGNED_DURING_SAVE:
  //     rti1516Reason = rti1516ev::FEDERATE_RESIGNED_DURING_SAVE;
  //     break;
  //   case OpenRTI::RTI_DETECTED_FAILURE_DURING_SAVE:
  //     rti1516Reason = rti1516ev::RTI_DETECTED_FAILURE_DURING_SAVE;
  //     break;
  //   case OpenRTI::SAVE_TIME_CANNOT_BE_HONORED:
  //     rti1516Reason = rti1516ev::SAVE_TIME_CANNOT_BE_HONORED;
  //     break;
  //   default:
  //     throw OpenRTI::FederateInternalError();
  //   }

  //   try {
  //     _federateAmbassador->federationNotSaved(rti1516Reason);
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }


  // // 4.17
  // void federationSaveStatusResponse(OpenRTI::FederateHandleSaveStatusPairVector const& federateStatusVector) override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //  /* enum SaveStatus */
  //  /* { */
  //  /*    NO_SAVE_IN_PROGRESS, */
  //  /*    FEDERATE_INSTRUCTED_TO_SAVE, */
  //  /*    FEDERATE_SAVING, */
  //  /*    FEDERATE_WAITING_FOR_FEDERATION_TO_SAVE */
  //  /* }; */

  //   try {
  //     throw rti1516ev::FederateInternalError(L"Not implemented");
  //     // _federateAmbassador
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.19
  // void requestFederationRestoreSucceeded(std::string const& label) override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreSucceeded(label);
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // void requestFederationRestoreFailed(std::string const& label) override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreFailed(label);
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }


  // // 4.20
  // void federationRestoreBegun() override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationRestoreBegun();
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.21
  // void initiateFederateRestore(std::string const & label,
  //                         FederateHandle handle) override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     OpenRTI::_O1516EFederateHandle rti1516Handle(handle);
  //     _federateAmbassador->initiateFederateRestore(label, rti1516Handle);
  //   } catch (const rti1516ev::SpecifiedSaveLabelDoesNotExist& e) {
  //     throw OpenRTI::SpecifiedSaveLabelDoesNotExist(e.what());
  //   } catch (const rti1516ev::CouldNotInitiateRestore& e) {
  //     throw OpenRTI::CouldNotInitiateRestore(e.what());
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.23
  // void federationRestored() override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationRestored();
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // void federationNotRestored(RestoreFailureReason reason) override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();

  //   rti1516ev::RestoreFailureReason rti1516Reason;
  //   switch (reason) {
  //   case OpenRTI::RTI_UNABLE_TO_RESTORE:
  //     rti1516Reason = rti1516ev::RTI_UNABLE_TO_RESTORE;
  //     break;
  //   case OpenRTI::FEDERATE_REPORTED_FAILURE_DURING_RESTORE:
  //     rti1516Reason = rti1516ev::FEDERATE_REPORTED_FAILURE_DURING_RESTORE;
  //     break;
  //   case OpenRTI::FEDERATE_RESIGNED_DURING_RESTORE:
  //     rti1516Reason = rti1516ev::FEDERATE_RESIGNED_DURING_RESTORE;
  //     break;
  //   case OpenRTI::RTI_DETECTED_FAILURE_DURING_RESTORE:
  //     rti1516Reason = rti1516ev::RTI_DETECTED_FAILURE_DURING_RESTORE;
  //     break;
  //   default:
  //     throw OpenRTI::FederateInternalError();
  //   }

  //   try {
  //     _federateAmbassador->federationNotRestored(rti1516Reason);
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.25
  // void federationRestoreStatusResponse(FederateHandleRestoreStatusPairVector const& theFederateStatusVector) override
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();


  //  /* enum RestoreStatus */
  //  /* { */
  //  /*    NO_RESTORE_IN_PROGRESS, */
  //  /*    FEDERATE_RESTORE_REQUEST_PENDING, */
  //  /*    FEDERATE_WAITING_FOR_RESTORE_TO_BEGIN, */
  //  /*    FEDERATE_PREPARED_TO_RESTORE, */
  //  /*    FEDERATE_RESTORING, */
  //  /*    FEDERATE_WAITING_FOR_FEDERATION_TO_RESTORE */
  //  /* }; */


  //   try {
  //     throw rti1516ev::FederateInternalError(L"Not implemented");
  //     // _federateAmbassador
  //   } catch (const rti1516ev::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  /////////////////////////////////////
  // Declaration Management Services //
  /////////////////////////////////////

  void registrationForObjectClass(OpenRTI::ObjectClassHandle objectClassHandle, bool start) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectClassHandle rti1516Handle(objectClassHandle);
      if (start)
        _federateAmbassador->startRegistrationForObjectClass(rti1516Handle);
      else
        _federateAmbassador->stopRegistrationForObjectClass(rti1516Handle);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void turnInteractionsOn(InteractionClassHandle interactionClassHandle, bool on) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EInteractionClassHandle rti1516Handle(interactionClassHandle);
      if (on)
        _federateAmbassador->turnInteractionsOn(rti1516Handle);
      else
        _federateAmbassador->turnInteractionsOff(rti1516Handle);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  ////////////////////////////////
  // Object Management Services //
  ////////////////////////////////

  void objectInstanceNameReservationSucceeded(const std::string& objectInstanceName) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EString rti1516ObjectInstanceName(objectInstanceName);
      _federateAmbassador->objectInstanceNameReservationSucceeded(rti1516ObjectInstanceName);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void objectInstanceNameReservationFailed(const std::string& objectInstanceName) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EString rti1516ObjectInstanceName(objectInstanceName);
      _federateAmbassador->objectInstanceNameReservationFailed(rti1516ObjectInstanceName);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void multipleObjectInstanceNameReservationSucceeded(const std::vector<std::string>& objectInstanceNames) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EStringSet rti1516ObjectInstanceName(objectInstanceNames);
      _federateAmbassador->multipleObjectInstanceNameReservationSucceeded(rti1516ObjectInstanceName);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }
  void multipleObjectInstanceNameReservationFailed(const std::vector<std::string>& objectInstanceNames) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EStringSet rti1516ObjectInstanceName(objectInstanceNames);
      _federateAmbassador->multipleObjectInstanceNameReservationFailed(rti1516ObjectInstanceName);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.5

  void discoverObjectInstance(ObjectInstanceHandle objectInstanceHandle,
                              ObjectClassHandle objectClassHandle,
                              std::string const& name) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EObjectClassHandle rti1516ObjectClassHandle(objectClassHandle);
      OpenRTI::_O1516EString rti1516Name(name);
      _federateAmbassador->discoverObjectInstance(rti1516ObjectInstanceHandle, rti1516ObjectClassHandle, rti1516Name);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void reflectAttributeValues(const Federate::ObjectClass& objectClass,
                              ObjectInstanceHandle objectInstanceHandle,
                              const AttributeValueVector& attributeValueVector, const VariableLengthData& tag,
                              OrderType sentOrder, TransportationType transportationType, FederateHandle federateHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EAttributeHandleValueMap rti1516AttributeValues(objectClass, attributeValueVector);
      if (!rti1516AttributeValues.empty()) {
        OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
        OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
        rti1516ev::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516ev::TransportationType rti1516TransportationType = translate(transportationType);
        OpenRTI::_O1516ESupplementalReflectInfo rti1516SupplementalReflectInfo(federateHandle);
        _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag,
                                                    rti1516SentOrderType, rti1516TransportationType, rti1516SupplementalReflectInfo);
      }
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }
  void reflectAttributeValues(const Federate::ObjectClass& objectClass, ObjectInstanceHandle objectInstanceHandle,
                              const AttributeValueVector& attributeValueVector, const VariableLengthData& tag,
                              OrderType sentOrder, TransportationType transportationType,
                              const NativeLogicalTime& logicalTime, OrderType receivedOrder, FederateHandle federateHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EAttributeHandleValueMap rti1516AttributeValues(objectClass, attributeValueVector);
      if (!rti1516AttributeValues.empty()) {
        OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
        OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
        rti1516ev::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516ev::TransportationType rti1516TransportationType = translate(transportationType);
        rti1516ev::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
        OpenRTI::_O1516ESupplementalReflectInfo rti1516SupplementalReflectInfo(federateHandle);
        _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag,
                                                    rti1516SentOrderType, rti1516TransportationType, logicalTime,
                                                    rti1516ReceivedOrderType, rti1516SupplementalReflectInfo);
      }
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }
  void reflectAttributeValues(const Federate::ObjectClass& objectClass, ObjectInstanceHandle objectInstanceHandle,
                              const AttributeValueVector& attributeValueVector, const VariableLengthData& tag,
                              OrderType sentOrder, TransportationType transportationType,
                              const NativeLogicalTime& logicalTime, OrderType receivedOrder, FederateHandle federateHandle,
                              MessageRetractionHandle messageRetractionHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EAttributeHandleValueMap rti1516AttributeValues(objectClass, attributeValueVector);
      if (!rti1516AttributeValues.empty()) {
        OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
        OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
        rti1516ev::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516ev::TransportationType rti1516TransportationType = translate(transportationType);
        rti1516ev::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
        OpenRTI::_O1516EMessageRetractionHandle rti1516MessageRetractionHandle(messageRetractionHandle);
        OpenRTI::_O1516ESupplementalReflectInfo rti1516SupplementalReflectInfo(federateHandle);
        _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag,
                                                    rti1516SentOrderType, rti1516TransportationType, logicalTime,
                                                    rti1516ReceivedOrderType, rti1516MessageRetractionHandle,
                                                    rti1516SupplementalReflectInfo);
      }
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void removeObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag, OrderType sentOrder, FederateHandle federateHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      rti1516ev::OrderType rti1516SentOrderType = translate(sentOrder);
      OpenRTI::_O1516ESupplementalRemoveInfo rti1516SupplementalRemoveInfo(federateHandle);
      _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516SentOrderType,
                                                rti1516SupplementalRemoveInfo);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void removeObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag, OrderType sentOrder,
                            const NativeLogicalTime& logicalTime, OrderType receivedOrder, FederateHandle federateHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      rti1516ev::OrderType rti1516SentOrderType = translate(sentOrder);
      rti1516ev::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
      OpenRTI::_O1516ESupplementalRemoveInfo rti1516SupplementalRemoveInfo(federateHandle);
      _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516SentOrderType,
                                                logicalTime, rti1516ReceivedOrderType, rti1516SupplementalRemoveInfo);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void removeObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag, OrderType sentOrder,
                            const NativeLogicalTime& logicalTime, OrderType receivedOrder, FederateHandle federateHandle,
                            MessageRetractionHandle messageRetractionHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      rti1516ev::OrderType rti1516SentOrderType = translate(sentOrder);
      rti1516ev::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
      OpenRTI::_O1516EMessageRetractionHandle rti1516MessageRetractionHandle(messageRetractionHandle);
      OpenRTI::_O1516ESupplementalRemoveInfo rti1516SupplementalRemoveInfo(federateHandle);
      _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516SentOrderType,
                                                logicalTime, rti1516ReceivedOrderType, rti1516MessageRetractionHandle,
                                                rti1516SupplementalRemoveInfo);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void receiveInteraction(const Federate::InteractionClass& interactionClass, InteractionClassHandle interactionClassHandle,
                          const ParameterValueVector& parameterValueVector, const VariableLengthData& tag,
                          OrderType sentOrder, TransportationType transportationType, FederateHandle federateHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EParameterHandleValueMap rti1516ParameterValues(interactionClass, parameterValueVector);
      if (!rti1516ParameterValues.empty()) {
        OpenRTI::_O1516EInteractionClassHandle rti1516InteractionClassHandle(interactionClassHandle);
        OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
        rti1516ev::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516ev::TransportationType rti1516TransportationType = translate(transportationType);
        OpenRTI::_O1516ESupplementalReceiveInfo rti1516SupplementalReceiveInfo(federateHandle);
        _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag,
                                                rti1516SentOrderType, rti1516TransportationType, rti1516SupplementalReceiveInfo);
      }
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void receiveInteraction(const Federate::InteractionClass& interactionClass, InteractionClassHandle interactionClassHandle,
                          const ParameterValueVector& parameterValueVector, const VariableLengthData& tag,
                          OrderType sentOrder, TransportationType transportationType, const NativeLogicalTime& logicalTime,
                          OrderType receivedOrder, FederateHandle federateHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EParameterHandleValueMap rti1516ParameterValues(interactionClass, parameterValueVector);
      if (!rti1516ParameterValues.empty()) {
        OpenRTI::_O1516EInteractionClassHandle rti1516InteractionClassHandle(interactionClassHandle);
        OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
        rti1516ev::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516ev::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
        rti1516ev::TransportationType rti1516TransportationType = translate(transportationType);
        OpenRTI::_O1516ESupplementalReceiveInfo rti1516SupplementalReceiveInfo(federateHandle);
        _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516SentOrderType,
                                                rti1516TransportationType, logicalTime, rti1516ReceivedOrderType,
                                                rti1516SupplementalReceiveInfo);
      }
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void receiveInteraction(const Federate::InteractionClass& interactionClass, InteractionClassHandle interactionClassHandle,
                          const ParameterValueVector& parameterValueVector, const VariableLengthData& tag,
                          OrderType sentOrder, TransportationType transportationType, const NativeLogicalTime& logicalTime,
                          OrderType receivedOrder, FederateHandle federateHandle, MessageRetractionHandle messageRetractionHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EParameterHandleValueMap rti1516ParameterValues(interactionClass, parameterValueVector);
      if (!rti1516ParameterValues.empty()) {
        OpenRTI::_O1516EInteractionClassHandle rti1516InteractionClassHandle(interactionClassHandle);
        OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
        rti1516ev::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516ev::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
        rti1516ev::TransportationType rti1516TransportationType = translate(transportationType);
        OpenRTI::_O1516EMessageRetractionHandle rti1516MessageRetractionHandle(messageRetractionHandle);
        OpenRTI::_O1516ESupplementalReceiveInfo rti1516SupplementalReceiveInfo(federateHandle);
        _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516SentOrderType,
                                                rti1516TransportationType, logicalTime, rti1516ReceivedOrderType,
                                                rti1516MessageRetractionHandle, rti1516SupplementalReceiveInfo);
      }
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.15
  void attributesInScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->attributesInScope(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.16
  void attributesOutOfScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->attributesOutOfScope(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.18
  void provideAttributeValueUpdate(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                              const VariableLengthData& tag) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      _federateAmbassador->provideAttributeValueUpdate(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.19
  void turnUpdatesOnForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector, const std::string& updateRate) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      if (updateRate.empty()) {
        _federateAmbassador->turnUpdatesOnForObjectInstance(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
      } else {
        OpenRTI::_O1516EString rti1516UpdateRate(updateRate);
        _federateAmbassador->turnUpdatesOnForObjectInstance(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516UpdateRate);
      }
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.20
  void turnUpdatesOffForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->turnUpdatesOffForObjectInstance(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  ///////////////////////////////////
  // Ownership Management Services //
  ///////////////////////////////////

  // 7.4
  void requestAttributeOwnershipAssumption(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                      const VariableLengthData& tag) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      _federateAmbassador->requestAttributeOwnershipAssumption(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.5
  void requestDivestitureConfirmation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->requestDivestitureConfirmation(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.7
  void attributeOwnershipAcquisitionNotification(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                            const VariableLengthData& tag) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      _federateAmbassador->attributeOwnershipAcquisitionNotification(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.10
  void attributeOwnershipUnavailable(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->attributeOwnershipUnavailable(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.11
  void requestAttributeOwnershipRelease(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                   const VariableLengthData& tag) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      _federateAmbassador->requestAttributeOwnershipRelease(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.15
  void confirmAttributeOwnershipAcquisitionCancellation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->confirmAttributeOwnershipAcquisitionCancellation(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.17
  void 
  informAttributeOwnership(ObjectInstanceHandle objectInstanceHandle,
                           AttributeHandle attributeHandle,
                           FederateHandle federateHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandle rti1516AttributeHandle(attributeHandle);
      OpenRTI::_O1516EFederateHandle rti1516FederateHandle(federateHandle);
      _federateAmbassador->informAttributeOwnership(rti1516ObjectInstanceHandle,
                                                    rti1516AttributeHandle,
                                                    rti1516FederateHandle);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void attributeIsNotOwned(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandle rti1516AttributeHandle(attributeHandle);
      _federateAmbassador->attributeIsNotOwned(rti1516ObjectInstanceHandle, rti1516AttributeHandle);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  void attributeIsOwnedByRTI(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandle rti1516AttributeHandle(attributeHandle);
      _federateAmbassador->attributeIsOwnedByRTI(rti1516ObjectInstanceHandle, rti1516AttributeHandle);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  //////////////////////////////
  // Time Management Services //
  //////////////////////////////

  // 8.3
  void timeRegulationEnabled(const rti1516ev::LogicalTime& logicalTime) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeRegulationEnabled(logicalTime);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.6
  void timeConstrainedEnabled(const rti1516ev::LogicalTime& logicalTime) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeConstrainedEnabled(logicalTime);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.13
  void timeAdvanceGrant(const rti1516ev::LogicalTime& logicalTime) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeAdvanceGrant(logicalTime);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.22
  void requestRetraction(MessageRetractionHandle messageRetractionHandle) override
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EMessageRetractionHandle rti1516MessageRetractionHandle(messageRetractionHandle);
      _federateAmbassador->requestRetraction(rti1516MessageRetractionHandle);
    } catch (const rti1516ev::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516ev exception in callback: " << e.what() << std::endl;
    }
  }

  rti1516ev::FederateAmbassador* _federateAmbassador;
  bool _inCallback;
};

RTIambassadorImplementation::RTIambassadorImplementation()
  : _ambassadorInterface(new RTI1516EAmbassadorInterface)
{
}

RTIambassadorImplementation::~RTIambassadorImplementation() noexcept
{
  delete _ambassadorInterface;
  _ambassadorInterface = 0;
}

void
RTIambassadorImplementation::connect(rti1516ev::FederateAmbassador & federateAmbassador, rti1516ev::CallbackModel rti1516CallbackModel,
                                     std::wstring const & localSettingsDesignator)
{
  try {
    if (_ambassadorInterface->_inCallback)
      throw OpenRTI::CallNotAllowedFromWithinCallback();
    URL url = URL::fromUrl(ucsToUtf8(localSettingsDesignator));
    StringStringListMap clientoptions;
    _ambassadorInterface->connect(url, clientoptions, _ambassadorInterface->getConnectWaitTimeoutMilliSeconds());
    _ambassadorInterface->_federateAmbassador = &federateAmbassador;
    OpenRTI::CallbackModel callbackModel = translate(rti1516CallbackModel);
    if (callbackModel == HLA_IMMEDIATE)
    {
      _ambassadorInterface->enableImmediateProcessing();
    }
  } catch (const OpenRTI::ConnectionFailed& e) {
    throw rti1516ev::ConnectionFailed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLocalSettingsDesignator& e) {
    throw rti1516ev::InvalidLocalSettingsDesignator(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AlreadyConnected& e) {
    throw rti1516ev::AlreadyConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CallNotAllowedFromWithinCallback& e) {
    throw rti1516ev::CallNotAllowedFromWithinCallback(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disconnect()
{
  try {
    if (_ambassadorInterface->_inCallback)
      throw OpenRTI::CallNotAllowedFromWithinCallback();
    _ambassadorInterface->disconnect();
    if (_ambassadorInterface->getCallbackModel() == HLA_IMMEDIATE)
    {
      _ambassadorInterface->disableImmediateProcessing();
    }
    _ambassadorInterface->_federateAmbassador = 0;
  } catch (const OpenRTI::FederateIsExecutionMember& e) {
    throw rti1516ev::FederateIsExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CallNotAllowedFromWithinCallback& e) {
    throw rti1516ev::CallNotAllowedFromWithinCallback(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::shutdown()
{
  _ambassadorInterface->shutdown();
  delete _ambassadorInterface;
  _ambassadorInterface = nullptr;
}

void
RTIambassadorImplementation::createFederationExecution(std::wstring const & federationExecutionName,
                                                       std::wstring const & fomModule,
                                                       std::wstring const & logicalTimeImplementationName)
{
  // Make sure we can read the fdd file
  OpenRTI::FOMStringModule2List fomModuleList;

  // Preload the HLAstandardMIM module
  loadHLAstandardMIM(fomModuleList);
  // And load the one given in the argument
  loadModule(fomModuleList, fomModule);

  try {
    std::string utf8FederationExecutionName = OpenRTI::ucsToUtf8(federationExecutionName);
    _ambassadorInterface->createFederationExecution(ucsToUtf8(federationExecutionName), fomModuleList, ucsToUtf8(logicalTimeImplementationName));
  } catch (const OpenRTI::InconsistentFDD& e) {
    throw rti1516ev::InconsistentFDD(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederationExecutionAlreadyExists& e) {
    throw rti1516ev::FederationExecutionAlreadyExists(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516ev::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::OperationTimeout & e) {
    throw rti1516ev::OperationTimeout(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::createFederationExecution(std::wstring const & federationExecutionName,
                                                       std::vector<std::wstring> const & fomModules,
                                                       std::wstring const & logicalTimeImplementationName)
{
  OpenRTI::FOMStringModule2List fomModuleList;

  // Preload the HLAstandardMIM module
  loadHLAstandardMIM(fomModuleList);
  // Load the ones given in the argument
  loadModules(fomModuleList, fomModules);

  try {
    _ambassadorInterface->createFederationExecution(ucsToUtf8(federationExecutionName), fomModuleList, ucsToUtf8(logicalTimeImplementationName));
  }
  catch (const OpenRTI::InconsistentFDD& e) {
    throw rti1516ev::InconsistentFDD(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::FederationExecutionAlreadyExists& e) {
    throw rti1516ev::FederationExecutionAlreadyExists(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516ev::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::OperationTimeout& e) {
    throw rti1516ev::OperationTimeout(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::createFederationExecutionWithMIM (std::wstring const & federationExecutionName,
                                                        std::vector<std::wstring> const & fomModules,
                                                        std::wstring const & mimModule,
                                                        std::wstring const & logicalTimeImplementationName)
{
  OpenRTI::FOMStringModule2List fomModuleList;

  try {
    loadModule(fomModuleList, mimModule);
  }
  catch (const rti1516ev::CouldNotOpenFDD& e) {
    throw rti1516ev::CouldNotOpenMIM(e.what());
  }
  catch (const rti1516ev::ErrorReadingFDD& e) {
    throw rti1516ev::ErrorReadingMIM(e.what());
  }
  catch (const rti1516ev::Exception& e) {
    throw rti1516ev::RTIinternalError(e.what());
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown error while reading mim file");
  }

  // Load the ones given in the argument
  loadModules(fomModuleList, fomModules);

  try {
    _ambassadorInterface->createFederationExecution(ucsToUtf8(federationExecutionName), fomModuleList, ucsToUtf8(logicalTimeImplementationName));
  }
  catch (const OpenRTI::InconsistentFDD& e) {
    throw rti1516ev::InconsistentFDD(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::FederationExecutionAlreadyExists& e) {
    throw rti1516ev::FederationExecutionAlreadyExists(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516ev::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::OperationTimeout& e) {
      throw rti1516ev::OperationTimeout(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::destroyFederationExecution(std::wstring const & federationExecutionName)
{
  try {
    _ambassadorInterface->destroyFederationExecution(ucsToUtf8(federationExecutionName));
  } catch (const OpenRTI::FederatesCurrentlyJoined& e) {
    throw rti1516ev::FederatesCurrentlyJoined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederationExecutionDoesNotExist& e) {
    throw rti1516ev::FederationExecutionDoesNotExist(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::listFederationExecutions()
{
  try {
    _ambassadorInterface->listFederationExecutions();
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::FederateHandle
RTIambassadorImplementation::joinFederationExecution(std::wstring const & federateType,
                                                     std::wstring const & federationExecutionName,
                                                     std::vector<std::wstring> const & additionalFomModules)
{
  OpenRTI::FOMStringModule2List fomModuleList;
  // Load the ones given in the argument
  loadModules(fomModuleList, additionalFomModules);

  try {
    if (_ambassadorInterface->_inCallback)
      throw OpenRTI::CallNotAllowedFromWithinCallback();
    return OpenRTI::_O1516EFederateHandle(_ambassadorInterface->joinFederationExecution(std::string(), ucsToUtf8(federateType),
                                                                                        ucsToUtf8(federationExecutionName), fomModuleList));
  }
  catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516ev::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::FederationExecutionDoesNotExist& e) {
    throw rti1516ev::FederationExecutionDoesNotExist(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::InconsistentFDD& e) {
    throw rti1516ev::InconsistentFDD(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::FederateAlreadyExecutionMember& e) {
    throw rti1516ev::FederateAlreadyExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::CallNotAllowedFromWithinCallback& e) {
    throw rti1516ev::CallNotAllowedFromWithinCallback(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const OpenRTI::OperationTimeout& e) {
    throw rti1516ev::OperationTimeout(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  }
  catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::FederateHandle
RTIambassadorImplementation::joinFederationExecution(std::wstring const & federateName,
                                                     std::wstring const & federateType,
                                                     std::wstring const & federationExecutionName,
                                                     std::vector<std::wstring> const & additionalFomModules)
{
  OpenRTI::FOMStringModule2List fomModuleList;
  // Load the ones given in the argument
  loadModules(fomModuleList, additionalFomModules);

  try {
    if (_ambassadorInterface->_inCallback)
      throw OpenRTI::CallNotAllowedFromWithinCallback();
    return OpenRTI::_O1516EFederateHandle(_ambassadorInterface->joinFederationExecution(ucsToUtf8(federateName), ucsToUtf8(federateType), ucsToUtf8(federationExecutionName), fomModuleList));
  } catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516ev::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederationExecutionDoesNotExist& e) {
    throw rti1516ev::FederationExecutionDoesNotExist(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNameAlreadyInUse& e) {
    throw rti1516ev::FederateNameAlreadyInUse(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InconsistentFDD& e) {
    throw rti1516ev::InconsistentFDD(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateAlreadyExecutionMember& e) {
    throw rti1516ev::FederateAlreadyExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CallNotAllowedFromWithinCallback& e) {
    throw rti1516ev::CallNotAllowedFromWithinCallback(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::OperationTimeout & e) {
    throw rti1516ev::OperationTimeout(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::resignFederationExecution(rti1516ev::ResignAction rti1516ResignAction)
{
  try {
    _ambassadorInterface->resignFederationExecution(translate(rti1516ResignAction));
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516ev::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw rti1516ev::FederateOwnsAttributes(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::registerFederationSynchronizationPoint(std::wstring const & label,
                                                                    rti1516ev::VariableLengthData const & rti1516Tag)
{
  try {
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    // According to the standard, an empty set also means all federates currently joined.
    OpenRTI::FederateHandleVector federateHandleVector;
    _ambassadorInterface->registerFederationSynchronizationPoint(ucsToUtf8(label), tag, federateHandleVector);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::registerFederationSynchronizationPoint(std::wstring const & label,
                                                                    rti1516ev::VariableLengthData const & rti1516Tag,
                                                                    rti1516ev::FederateHandleSet const & rti1516FederateHandleSet)
{
  try {
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    OpenRTI::_I1516EFederateHandleVector federateHandleVector(rti1516FederateHandleSet);
    _ambassadorInterface->registerFederationSynchronizationPoint(ucsToUtf8(label), tag, federateHandleVector);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::synchronizationPointAchieved(std::wstring const & label, bool successfully)
{
  try {
    _ambassadorInterface->synchronizationPointAchieved(ucsToUtf8(label), successfully);
  } catch (const OpenRTI::SynchronizationPointLabelNotAnnounced& e) {
    throw rti1516ev::SynchronizationPointLabelNotAnnounced(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestFederationSave(std::wstring const & label)
{
  try {
    _ambassadorInterface->requestFederationSave(ucsToUtf8(label));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestFederationSave(const std::wstring& label,
                                                   const rti1516ev::LogicalTime& rti1516LogicalTime)
{
  try {
    _ambassadorInterface->requestFederationSave(ucsToUtf8(label), rti1516LogicalTime);
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516ev::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516ev::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateUnableToUseTime& e) {
    throw rti1516ev::FederateUnableToUseTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateSaveBegun()
{
  try {
    _ambassadorInterface->federateSaveBegun();
  } catch (const OpenRTI::SaveNotInitiated& e) {
    throw rti1516ev::SaveNotInitiated(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateSaveComplete()
{
  try {
    _ambassadorInterface->federateSaveComplete();
  } catch (const OpenRTI::FederateHasNotBegunSave& e) {
    throw rti1516ev::FederateHasNotBegunSave(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateSaveNotComplete()
{
  try {
    _ambassadorInterface->federateSaveNotComplete();
  } catch (const OpenRTI::FederateHasNotBegunSave& e) {
    throw rti1516ev::FederateHasNotBegunSave(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::abortFederationSave()
{
  try {
    _ambassadorInterface->abortFederationSave();
  } catch (const OpenRTI::SaveNotInProgress& e) {
    throw rti1516ev::SaveNotInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryFederationSaveStatus()
{
  try {
    _ambassadorInterface->queryFederationSaveStatus();
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestFederationRestore(std::wstring const & label)
{
  try {
    _ambassadorInterface->requestFederationRestore(ucsToUtf8(label));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateRestoreComplete()
{
  try {
    _ambassadorInterface->federateRestoreComplete();
  } catch (const OpenRTI::RestoreNotRequested& e) {
    throw rti1516ev::RestoreNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateRestoreNotComplete()
{
  try {
    _ambassadorInterface->federateRestoreNotComplete();
  } catch (const OpenRTI::RestoreNotRequested& e) {
    throw rti1516ev::RestoreNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::abortFederationRestore()
{
  try {
    _ambassadorInterface->abortFederationRestore();
  } catch (const OpenRTI::RestoreNotInProgress& e) {
    throw rti1516ev::RestoreNotInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryFederationRestoreStatus()
{
  try {
    _ambassadorInterface->queryFederationRestoreStatus();
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::publishObjectClassAttributes(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                          rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->publishObjectClassAttributes(objectClassHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unpublishObjectClass(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    _ambassadorInterface->unpublishObjectClass(objectClassHandle);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516ev::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unpublishObjectClassAttributes(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                            rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->unpublishObjectClassAttributes(objectClassHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516ev::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::publishInteractionClass(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->publishInteractionClass(interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unpublishInteractionClass(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->unpublishInteractionClass(interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeObjectClassAttributes(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                            rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                            bool active, std::wstring const & updateRateDesignator)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->subscribeObjectClassAttributes(objectClassHandle, attributeHandleVector, active, ucsToUtf8(updateRateDesignator));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidUpdateRateDesignator& e) {
    throw rti1516ev::InvalidUpdateRateDesignator(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeObjectClass(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    _ambassadorInterface->unsubscribeObjectClass(objectClassHandle);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeObjectClassAttributes(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                              rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->unsubscribeObjectClassAttributes(objectClassHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeObjectInstance(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    _ambassadorInterface->unsubscribeObjectInstance(objectInstanceHandle);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeInteractionClass(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                       bool active)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->subscribeInteractionClass(interactionClassHandle, active);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateServiceInvocationsAreBeingReportedViaMOM& e) {
    throw rti1516ev::FederateServiceInvocationsAreBeingReportedViaMOM(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}


void RTIambassadorImplementation::subscribeInteractionClassWithFilter(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                                      const rti1516ev::ParameterHandleValueMap& filterValues,
                                                                      bool active)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterValueVector filterParameterValueVector(filterValues);
    _ambassadorInterface->subscribeInteractionClassWithFilter(interactionClassHandle, filterParameterValueVector, active);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateServiceInvocationsAreBeingReportedViaMOM& e) {
    throw rti1516ev::FederateServiceInvocationsAreBeingReportedViaMOM(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeInteractionClass(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->unsubscribeInteractionClass(interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeInteractionClassWithFilter(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                                   const rti1516ev::ParameterHandleValueMap& filterValues)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterValueVector filterParameterValueVector(filterValues);
    _ambassadorInterface->unsubscribeInteractionClassWithFilter(interactionClassHandle, filterParameterValueVector);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::reserveObjectInstanceName(std::wstring const & objectInstanceName)
{
  try {
    _ambassadorInterface->reserveObjectInstanceName(ucsToUtf8(objectInstanceName));
  } catch (const OpenRTI::IllegalName& e) {
    throw rti1516ev::IllegalName(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::releaseObjectInstanceName(std::wstring const & objectInstanceName)
{
  try {
    _ambassadorInterface->releaseObjectInstanceName(ucsToUtf8(objectInstanceName));
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw rti1516ev::ObjectInstanceNameNotReserved(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::reserveMultipleObjectInstanceName(std::set<std::wstring> const & objectInstanceName)
{
  try {
    std::set<std::string> utf8ObjectInstanceName;
    for (std::set<std::wstring>::const_iterator i = objectInstanceName.begin(); i != objectInstanceName.end(); ++i)
      utf8ObjectInstanceName.insert(ucsToUtf8(*i));
    _ambassadorInterface->reserveMultipleObjectInstanceName(utf8ObjectInstanceName);
  } catch (const OpenRTI::IllegalName& e) {
    throw rti1516ev::IllegalName(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NameSetWasEmpty& e) {
    throw rti1516ev::NameSetWasEmpty(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::releaseMultipleObjectInstanceName(std::set<std::wstring> const & objectInstanceNameSet)
{
  try {
    std::set<std::string> utf8ObjectInstanceName;
    for (std::set<std::wstring>::const_iterator i = objectInstanceNameSet.begin(); i != objectInstanceNameSet.end(); ++i)
      utf8ObjectInstanceName.insert(ucsToUtf8(*i));
    _ambassadorInterface->releaseMultipleObjectInstanceName(utf8ObjectInstanceName);
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw rti1516ev::ObjectInstanceNameNotReserved(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstance(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return OpenRTI::_O1516EObjectInstanceHandle(_ambassadorInterface->registerObjectInstance(objectClassHandle));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516ev::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstance(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                    std::wstring const & objectInstanceName)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return OpenRTI::_O1516EObjectInstanceHandle(_ambassadorInterface->registerObjectInstance(objectClassHandle, ucsToUtf8(objectInstanceName), false));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516ev::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw rti1516ev::ObjectInstanceNameNotReserved(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameInUse& e) {
    throw rti1516ev::ObjectInstanceNameInUse(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstance(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                    std::wstring const & objectInstanceName, bool allowUnreservedObjectNames)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return OpenRTI::_O1516EObjectInstanceHandle(_ambassadorInterface->registerObjectInstance(objectClassHandle, ucsToUtf8(objectInstanceName), allowUnreservedObjectNames));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516ev::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw rti1516ev::ObjectInstanceNameNotReserved(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameInUse& e) {
    throw rti1516ev::ObjectInstanceNameInUse(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::updateAttributeValues(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                   const rti1516ev::AttributeHandleValueMap& rti1516AttributeHandleValueMap,
                                                   const rti1516ev::VariableLengthData& rti1516Tag)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeValueVector attributeValueVector(rti1516AttributeHandleValueMap);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->updateAttributeValues(objectInstanceHandle, attributeValueVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516ev::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::MessageRetractionHandle
RTIambassadorImplementation::updateAttributeValues(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                   const rti1516ev::AttributeHandleValueMap& rti1516AttributeHandleValueMap,
                                                   const rti1516ev::VariableLengthData& rti1516Tag,
                                                   const rti1516ev::LogicalTime& rti1516LogicalTime)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeValueVector attributeValueVector(rti1516AttributeHandleValueMap);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516EMessageRetractionHandle(_ambassadorInterface->updateAttributeValues(objectInstanceHandle, attributeValueVector, tag, rti1516LogicalTime));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516ev::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516ev::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::sendInteraction(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                             const rti1516ev::ParameterHandleValueMap& rti1516ParameterHandleValueMap,
                                             const rti1516ev::VariableLengthData& rti1516Tag)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->sendInteraction(interactionClassHandle, parameterValueVector, tag);
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516ev::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516ev::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::MessageRetractionHandle
RTIambassadorImplementation::sendInteraction(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                             const rti1516ev::ParameterHandleValueMap& rti1516ParameterHandleValueMap,
                                             const rti1516ev::VariableLengthData& rti1516Tag,
                                             const rti1516ev::LogicalTime& rti1516LogicalTime)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516EMessageRetractionHandle(_ambassadorInterface->sendInteraction(interactionClassHandle, parameterValueVector, tag, rti1516LogicalTime));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516ev::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516ev::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516ev::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::deleteObjectInstance(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                  const rti1516ev::VariableLengthData& rti1516Tag)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->deleteObjectInstance(objectInstanceHandle, tag);
  } catch (const OpenRTI::DeletePrivilegeNotHeld& e) {
    throw rti1516ev::DeletePrivilegeNotHeld(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::MessageRetractionHandle
RTIambassadorImplementation::deleteObjectInstance(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                  const rti1516ev::VariableLengthData& rti1516Tag,
                                                  const rti1516ev::LogicalTime& rti1516LogicalTime)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516EMessageRetractionHandle(_ambassadorInterface->deleteObjectInstance(objectInstanceHandle, tag, rti1516LogicalTime));
  } catch (const OpenRTI::DeletePrivilegeNotHeld& e) {
    throw rti1516ev::DeletePrivilegeNotHeld(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516ev::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::localDeleteObjectInstance(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    _ambassadorInterface->localDeleteObjectInstance(objectInstanceHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw rti1516ev::FederateOwnsAttributes(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516ev::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

/*
void
RTIambassadorImplementation::changeAttributeTransportationType(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                               rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                               rti1516ev::TransportationType rti1516TransportationType)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::TransportationType transportationType = translate(rti1516TransportationType);
    _ambassadorInterface->changeAttributeTransportationType(objectInstanceHandle, attributeHandleVector, transportationType);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516ev::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::changeInteractionTransportationType(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                                 rti1516ev::TransportationType rti1516TransportationType)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::TransportationType transportationType = translate(rti1516TransportationType);
    _ambassadorInterface->changeInteractionTransportationType(interactionClassHandle, transportationType);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516ev::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}
*/

void
RTIambassadorImplementation::requestAttributeValueUpdate(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                         rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                         rti1516ev::VariableLengthData const & rti1516Tag)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->requestAttributeValueUpdate(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestAttributeValueUpdate(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                         rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                         rti1516ev::VariableLengthData const & rti1516Tag)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->requestAttributeValueUpdate(objectClassHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestAttributeTransportationTypeChange(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                      rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                      rti1516ev::TransportationType transportationType)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->requestAttributeTransportationTypeChange(objectInstanceHandle, attributeHandleVector,
                                                                   translate(transportationType));
  } catch (const OpenRTI::AttributeAlreadyBeingChanged& e) {
    throw rti1516ev::AttributeAlreadyBeingChanged(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516ev::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidTransportationType& e) {
    throw rti1516ev::InvalidTransportationType(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryAttributeTransportationType(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                              rti1516ev::AttributeHandle rti1516AttributeHandle)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    _ambassadorInterface->queryAttributeTransportationType(objectInstanceHandle, attributeHandle);
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestInteractionTransportationTypeChange(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                                        rti1516ev::TransportationType transportationType)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->requestInteractionTransportationTypeChange(interactionClassHandle, translate(transportationType));
  } catch (const OpenRTI::InteractionClassAlreadyBeingChanged& e) {
    throw rti1516ev::InteractionClassAlreadyBeingChanged(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516ev::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidTransportationType& e) {
    throw rti1516ev::InvalidTransportationType(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryInteractionTransportationType(rti1516ev::FederateHandle rti1516FederateHandle, rti1516ev::InteractionClassHandle rti1516InteractionClassHandle)
{
  try {
    OpenRTI::_I1516EFederateHandle federateHandle(rti1516FederateHandle);
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->queryInteractionTransportationType(federateHandle, interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unconditionalAttributeOwnershipDivestiture(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                        rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->unconditionalAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516ev::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::negotiatedAttributeOwnershipDivestiture(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                     rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                     rti1516ev::VariableLengthData const & rti1516Tag)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->negotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516ev::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAlreadyBeingDivested& e) {
    throw rti1516ev::AttributeAlreadyBeingDivested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::confirmDivestiture(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                rti1516ev::AttributeHandleSet const& rti1516AttributeHandleSet,
                                                rti1516ev::VariableLengthData const& rti1516Tag)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->confirmDivestiture(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516ev::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeDivestitureWasNotRequested& e) {
    throw rti1516ev::AttributeDivestitureWasNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NoAcquisitionPending& e) {
    throw rti1516ev::NoAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::attributeOwnershipAcquisition(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                           rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                           rti1516ev::VariableLengthData const & rti1516Tag)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->attributeOwnershipAcquisition(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516ev::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516ev::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::attributeOwnershipAcquisitionIfAvailable(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                      rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->attributeOwnershipAcquisitionIfAvailable(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516ev::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516ev::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw rti1516ev::FederateOwnsAttributes(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAlreadyBeingAcquired& e) {
    throw rti1516ev::AttributeAlreadyBeingAcquired(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::attributeOwnershipReleaseDenied(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                             rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->attributeOwnershipReleaseDenied(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516ev::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::attributeOwnershipDivestitureIfWanted(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                   rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                   rti1516ev::AttributeHandleSet & rti1516DivestedAttributeSet)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::AttributeHandleVector divestedAttributeHandleVector;
    _ambassadorInterface->attributeOwnershipDivestitureIfWanted(objectInstanceHandle, attributeHandleVector, divestedAttributeHandleVector);
    rti1516DivestedAttributeSet.clear();
    for (OpenRTI::AttributeHandleVector::const_iterator i = divestedAttributeHandleVector.begin(); i != divestedAttributeHandleVector.end(); ++i)
      rti1516DivestedAttributeSet.insert(OpenRTI::_O1516EAttributeHandle(*i));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516ev::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::cancelNegotiatedAttributeOwnershipDivestiture(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                           rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->cancelNegotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516ev::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeDivestitureWasNotRequested& e) {
    throw rti1516ev::AttributeDivestitureWasNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::cancelAttributeOwnershipAcquisition(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                 rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->cancelAttributeOwnershipAcquisition(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAlreadyOwned& e) {
    throw rti1516ev::AttributeAlreadyOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAcquisitionWasNotRequested& e) {
    throw rti1516ev::AttributeAcquisitionWasNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryAttributeOwnership(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                     rti1516ev::AttributeHandle rti1516AttributeHandle)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    _ambassadorInterface->queryAttributeOwnership(objectInstanceHandle, attributeHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::isAttributeOwnedByFederate(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                        rti1516ev::AttributeHandle rti1516AttributeHandle)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    return _ambassadorInterface->isAttributeOwnedByFederate(objectInstanceHandle, attributeHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableTimeRegulation(const rti1516ev::LogicalTimeInterval& rti1516Lookahead)
{
  try {
    _ambassadorInterface->enableTimeRegulation(rti1516Lookahead);
  } catch (const OpenRTI::TimeRegulationAlreadyEnabled& e) {
    throw rti1516ev::TimeRegulationAlreadyEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLookahead& e) {
    throw rti1516ev::InvalidLookahead(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516ev::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516ev::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableTimeRegulation()
{
  try {
    _ambassadorInterface->disableTimeRegulation();
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516ev::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableTimeConstrained()
{
  try {
    _ambassadorInterface->enableTimeConstrained();
  } catch (const OpenRTI::TimeConstrainedAlreadyEnabled& e) {
    throw rti1516ev::TimeConstrainedAlreadyEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516ev::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516ev::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableTimeConstrained()
{
  try {
    _ambassadorInterface->disableTimeConstrained();
  } catch (const OpenRTI::TimeConstrainedIsNotEnabled& e) {
    throw rti1516ev::TimeConstrainedIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::timeAdvanceRequest(const rti1516ev::LogicalTime& logicalTime)
{
  try {
    _ambassadorInterface->timeAdvanceRequest(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516ev::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516ev::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516ev::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516ev::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516ev::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::timeAdvanceRequestAvailable(const rti1516ev::LogicalTime& logicalTime)
{
  try {
    _ambassadorInterface->timeAdvanceRequestAvailable(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516ev::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516ev::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516ev::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516ev::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516ev::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::nextMessageRequest(const rti1516ev::LogicalTime& logicalTime)
{
  try {
    _ambassadorInterface->nextMessageRequest(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516ev::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516ev::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516ev::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516ev::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516ev::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::nextMessageRequestAvailable(const rti1516ev::LogicalTime& logicalTime)
{
  try {
    _ambassadorInterface->nextMessageRequestAvailable(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516ev::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516ev::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516ev::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516ev::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516ev::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::flushQueueRequest(const rti1516ev::LogicalTime& logicalTime)
{
  try {
    _ambassadorInterface->flushQueueRequest(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516ev::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516ev::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516ev::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516ev::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516ev::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableAsynchronousDelivery()
{
  try {
    _ambassadorInterface->enableAsynchronousDelivery();
  } catch (const OpenRTI::AsynchronousDeliveryAlreadyEnabled& e) {
    throw rti1516ev::AsynchronousDeliveryAlreadyEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableAsynchronousDelivery()
{
  try {
    _ambassadorInterface->disableAsynchronousDelivery();
  } catch (const OpenRTI::AsynchronousDeliveryAlreadyDisabled& e) {
    throw rti1516ev::AsynchronousDeliveryAlreadyDisabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::queryGALT(rti1516ev::LogicalTime& logicalTime)
{
  try {
    return _ambassadorInterface->queryGALT(logicalTime);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryLogicalTime(rti1516ev::LogicalTime& logicalTime)
{
  try {
    _ambassadorInterface->queryLogicalTime(logicalTime);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::queryLITS(rti1516ev::LogicalTime& logicalTime)
{
  try {
    return _ambassadorInterface->queryLITS(logicalTime);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::modifyLookahead(const rti1516ev::LogicalTimeInterval& lookahead)
{
  try {
    _ambassadorInterface->modifyLookahead(lookahead, true/*checkForTimeRegulation*/);
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516ev::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLookahead& e) {
    throw rti1516ev::InvalidLookahead(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516ev::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryLookahead(rti1516ev::LogicalTimeInterval& lookahead)
{
  try {
    _ambassadorInterface->queryLookahead(lookahead, true/*checkForTimeRegulation*/);
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516ev::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::retract(rti1516ev::MessageRetractionHandle rti1516MessageRetractionHandle)
{
  try {
    OpenRTI::_I1516EMessageRetractionHandle messageRetractionHandle(rti1516MessageRetractionHandle);
    _ambassadorInterface->retract(messageRetractionHandle);
  } catch (const OpenRTI::InvalidMessageRetractionHandle& e) {
    throw rti1516ev::InvalidMessageRetractionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516ev::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::MessageCanNoLongerBeRetracted& e) {
    throw rti1516ev::MessageCanNoLongerBeRetracted(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::changeAttributeOrderType(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                      rti1516ev::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                      rti1516ev::OrderType rti1516OrderType)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::OrderType orderType = translate(rti1516OrderType);
    _ambassadorInterface->changeAttributeOrderType(objectInstanceHandle, attributeHandleVector, orderType);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516ev::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::changeInteractionOrderType(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516ev::OrderType rti1516OrderType)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::OrderType orderType = translate(rti1516OrderType);
    _ambassadorInterface->changeInteractionOrderType(interactionClassHandle, orderType);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516ev::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::RegionHandle
RTIambassadorImplementation::createRegion(rti1516ev::DimensionHandleSet const & rti1516DimensionHandleSet)
{
  try {
    OpenRTI::_I1516EDimensionHandleSet dimensionHandleSet(rti1516DimensionHandleSet);
    return OpenRTI::_O1516ERegionHandle(_ambassadorInterface->createRegion(dimensionHandleSet));
  } catch (const OpenRTI::InvalidDimensionHandle& e) {
    throw rti1516ev::InvalidDimensionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::commitRegionModifications(rti1516ev::RegionHandleSet const & rti1516RegionHandleSet)
{
  try {
    OpenRTI::_I1516ERegionHandleVector regionHandleVector(rti1516RegionHandleSet);
    _ambassadorInterface->commitRegionModifications(regionHandleVector);
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::deleteRegion(const rti1516ev::RegionHandle& rti1516RegionHandle)
{
  try {
    OpenRTI::_I1516ERegionHandle regionHandle(rti1516RegionHandle);
    _ambassadorInterface->deleteRegion(regionHandle);
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionInUseForUpdateOrSubscription& e) {
    throw rti1516ev::RegionInUseForUpdateOrSubscription(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstanceWithRegions(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                               rti1516ev::AttributeHandleSetRegionHandleSetPairVector const &
                                                               rti1516AttributeHandleSetRegionHandleSetPairVector)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    return OpenRTI::_O1516EObjectInstanceHandle(_ambassadorInterface->registerObjectInstanceWithRegions(objectClassHandle,
                                                                                                        attributeHandleVectorRegionHandleVectorPairVector));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516ev::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516ev::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516ev::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstanceWithRegions(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                               rti1516ev::AttributeHandleSetRegionHandleSetPairVector const &
                                                               rti1516AttributeHandleSetRegionHandleSetPairVector,
                                                               std::wstring const & objectInstanceName)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    return OpenRTI::_O1516EObjectInstanceHandle(_ambassadorInterface->registerObjectInstanceWithRegions(objectClassHandle,
                                                                                                        attributeHandleVectorRegionHandleVectorPairVector,
                                                                                                        ucsToUtf8(objectInstanceName)));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516ev::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516ev::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516ev::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw rti1516ev::ObjectInstanceNameNotReserved(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameInUse& e) {
    throw rti1516ev::ObjectInstanceNameInUse(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::associateRegionsForUpdates(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                        rti1516ev::AttributeHandleSetRegionHandleSetPairVector const &
                                                        rti1516AttributeHandleSetRegionHandleSetPairVector)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->associateRegionsForUpdates(objectInstanceHandle, attributeHandleVectorRegionHandleVectorPairVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516ev::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unassociateRegionsForUpdates(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                          rti1516ev::AttributeHandleSetRegionHandleSetPairVector const &
                                                          rti1516AttributeHandleSetRegionHandleSetPairVector)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->unassociateRegionsForUpdates(objectInstanceHandle, attributeHandleVectorRegionHandleVectorPairVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeObjectClassAttributesWithRegions(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                                       rti1516ev::AttributeHandleSetRegionHandleSetPairVector const &
                                                                       rti1516AttributeHandleSetRegionHandleSetPairVector,
                                                                       bool active, std::wstring const & updateRateDesignator)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->subscribeObjectClassAttributesWithRegions(objectClassHandle,
                                                                    attributeHandleVectorRegionHandleVectorPairVector,
                                                                    active, ucsToUtf8(updateRateDesignator));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516ev::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidUpdateRateDesignator& e) {
    throw rti1516ev::InvalidUpdateRateDesignator(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeObjectClassAttributesWithRegions(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                                         rti1516ev::AttributeHandleSetRegionHandleSetPairVector const &
                                                                         rti1516AttributeHandleSetRegionHandleSetPairVector)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->unsubscribeObjectClassAttributesWithRegions(objectClassHandle,
                                                                      attributeHandleVectorRegionHandleVectorPairVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeInteractionClassWithRegions(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                                  rti1516ev::RegionHandleSet const & rti1516RegionHandleSet,
                                                                  bool active)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516ERegionHandleVector regionHandleVector(rti1516RegionHandleSet);
    _ambassadorInterface->subscribeInteractionClassWithRegions(interactionClassHandle, regionHandleVector, active);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516ev::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateServiceInvocationsAreBeingReportedViaMOM& e) {
    throw rti1516ev::FederateServiceInvocationsAreBeingReportedViaMOM(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeInteractionClassWithRegions(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                                    rti1516ev::RegionHandleSet const & rti1516RegionHandleSet)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516ERegionHandleVector regionHandleVector(rti1516RegionHandleSet);
    _ambassadorInterface->unsubscribeInteractionClassWithRegions(interactionClassHandle, regionHandleVector);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::sendInteractionWithRegions(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516ev::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                                                        rti1516ev::RegionHandleSet const & rti1516RegionHandleSet,
                                                        rti1516ev::VariableLengthData const & rti1516Tag)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516ERegionHandleVector regionHandleVector(rti1516RegionHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->sendInteractionWithRegions(interactionClassHandle, parameterValueVector, regionHandleVector, tag);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516ev::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516ev::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516ev::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::MessageRetractionHandle
RTIambassadorImplementation::sendInteractionWithRegions(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516ev::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                                                        rti1516ev::RegionHandleSet const & rti1516RegionHandleSet,
                                                        rti1516ev::VariableLengthData const & rti1516Tag,
                                                        rti1516ev::LogicalTime const & rti1516LogicalTime)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516ERegionHandleVector regionHandleVector(rti1516RegionHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516EMessageRetractionHandle(_ambassadorInterface->sendInteractionWithRegions(interactionClassHandle, parameterValueVector, regionHandleVector, tag, rti1516LogicalTime));
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516ev::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516ev::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516ev::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516ev::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516ev::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestAttributeValueUpdateWithRegions(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                                    rti1516ev::AttributeHandleSetRegionHandleSetPairVector const&
                                                                    rti1516AttributeHandleSetRegionHandleSetPairVector,
                                                                    rti1516ev::VariableLengthData const & rti1516Tag)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->requestAttributeValueUpdateWithRegions(objectClassHandle, attributeHandleVectorRegionHandleVectorPairVector, tag);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516ev::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516ev::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ResignAction
RTIambassadorImplementation::getAutomaticResignDirective()
{
  try {
    return translate(_ambassadorInterface->getAutomaticResignDirective());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::setAutomaticResignDirective(rti1516ev::ResignAction resignAction)
{
  try {
    _ambassadorInterface->setAutomaticResignDirective(translate(resignAction));
  } catch (const OpenRTI::InvalidResignAction& e) {
    throw rti1516ev::InvalidResignAction(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::FederateHandle
RTIambassadorImplementation::getFederateHandle(std::wstring const & name)
{
  try {
    return OpenRTI::_O1516EFederateHandle(_ambassadorInterface->getFederateHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516ev::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getFederateName(rti1516ev::FederateHandle rti1516FederateHandle)
{
  try {
    OpenRTI::_I1516EFederateHandle federateHandle(rti1516FederateHandle);
    return utf8ToUcs(_ambassadorInterface->getFederateName(federateHandle));
  } catch (const OpenRTI::InvalidFederateHandle& e) {
    throw rti1516ev::InvalidFederateHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateHandleNotKnown& e) {
    throw rti1516ev::FederateHandleNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ObjectClassHandle
RTIambassadorImplementation::getObjectClassHandle(std::wstring const & name)
{
  try {
    return OpenRTI::_O1516EObjectClassHandle(_ambassadorInterface->getObjectClassHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516ev::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getObjectClassName(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return utf8ToUcs(_ambassadorInterface->getObjectClassName(objectClassHandle));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516ev::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::AttributeHandle
RTIambassadorImplementation::getAttributeHandle(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                std::wstring const & attributeName)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return OpenRTI::_O1516EAttributeHandle(_ambassadorInterface->getAttributeHandle(objectClassHandle, ucsToUtf8(attributeName)));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516ev::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516ev::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getAttributeName(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                              rti1516ev::AttributeHandle rti1516AttributeHandle)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    return utf8ToUcs(_ambassadorInterface->getAttributeName(objectClassHandle, attributeHandle));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516ev::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidAttributeHandle& e) {
    throw rti1516ev::InvalidAttributeHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getAttributeDataType(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                  rti1516ev::AttributeHandle rti1516AttributeHandle)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    return utf8ToUcs(_ambassadorInterface->getAttributeDataType(objectClassHandle, attributeHandle));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516ev::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidAttributeHandle& e) {
    throw rti1516ev::InvalidAttributeHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

double
RTIambassadorImplementation::getUpdateRateValue(std::wstring const & updateRateDesignator)
{
  try {
    return _ambassadorInterface->getUpdateRateValue(ucsToUtf8(updateRateDesignator));
  } catch (const OpenRTI::InvalidUpdateRateDesignator& e) {
    throw rti1516ev::InvalidUpdateRateDesignator(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

double
RTIambassadorImplementation::getUpdateRateValueForAttribute(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                            rti1516ev::AttributeHandle rti1516AttributeHandle)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    return _ambassadorInterface->getUpdateRateValueForAttribute(objectInstanceHandle, attributeHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::InteractionClassHandle
RTIambassadorImplementation::getInteractionClassHandle(std::wstring const & name)
{
  try {
    return OpenRTI::_O1516EInteractionClassHandle(_ambassadorInterface->getInteractionClassHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516ev::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getInteractionClassName(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    return utf8ToUcs(_ambassadorInterface->getInteractionClassName(interactionClassHandle));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516ev::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ParameterHandle
RTIambassadorImplementation::getParameterHandle(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                std::wstring const & parameterName)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    return OpenRTI::_O1516EParameterHandle(_ambassadorInterface->getParameterHandle(interactionClassHandle, ucsToUtf8(parameterName)));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516ev::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516ev::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getParameterName(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                              rti1516ev::ParameterHandle rti1516ParameterHandle)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterHandle parameterHandle(rti1516ParameterHandle);
    return utf8ToUcs(_ambassadorInterface->getParameterName(interactionClassHandle, parameterHandle));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516ev::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidParameterHandle& e) {
    throw rti1516ev::InvalidParameterHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516ev::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getParameterDataType(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle,
                                                  rti1516ev::ParameterHandle rti1516ParameterHandle)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterHandle parameterHandle(rti1516ParameterHandle);
    return utf8ToUcs(_ambassadorInterface->getParameterDataType(interactionClassHandle, parameterHandle));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516ev::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidParameterHandle& e) {
    throw rti1516ev::InvalidParameterHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516ev::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ObjectInstanceHandle
RTIambassadorImplementation::getObjectInstanceHandle(std::wstring const & name)
{
  try {
    return OpenRTI::_O1516EObjectInstanceHandle(_ambassadorInterface->getObjectInstanceHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getObjectInstanceName(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    return utf8ToUcs(_ambassadorInterface->getObjectInstanceName(objectInstanceHandle));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::DimensionHandle
RTIambassadorImplementation::getDimensionHandle(std::wstring const & name)
{
  try {
    return OpenRTI::_O1516EDimensionHandle(_ambassadorInterface->getDimensionHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516ev::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getDimensionName(rti1516ev::DimensionHandle rti1516DimensionHandle)
{
  try {
    OpenRTI::_I1516EDimensionHandle dimensionHandle(rti1516DimensionHandle);
    return utf8ToUcs(_ambassadorInterface->getDimensionName(dimensionHandle));
  } catch (const OpenRTI::InvalidDimensionHandle& e) {
    throw rti1516ev::InvalidDimensionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

unsigned long
RTIambassadorImplementation::getDimensionUpperBound(rti1516ev::DimensionHandle rti1516DimensionHandle)
{
  try {
    OpenRTI::_I1516EDimensionHandle dimensionHandle(rti1516DimensionHandle);
    return _ambassadorInterface->getDimensionUpperBound(dimensionHandle);
  } catch (const OpenRTI::InvalidDimensionHandle& e) {
    throw rti1516ev::InvalidDimensionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::DimensionHandleSet
RTIambassadorImplementation::getAvailableDimensionsForClassAttribute(rti1516ev::ObjectClassHandle rti1516ObjectClassHandle,
                                                                     rti1516ev::AttributeHandle rti1516AttributeHandle)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    return OpenRTI::_O1516EDimensionHandleSet(_ambassadorInterface->getAvailableDimensionsForClassAttribute(objectClassHandle, attributeHandle));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516ev::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidAttributeHandle& e) {
    throw rti1516ev::InvalidAttributeHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516ev::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ObjectClassHandle
RTIambassadorImplementation::getKnownObjectClassHandle(rti1516ev::ObjectInstanceHandle rti1516ObjectInstanceHandle)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    return OpenRTI::_O1516EObjectClassHandle(_ambassadorInterface->getKnownObjectClassHandle(objectInstanceHandle));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516ev::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::DimensionHandleSet
RTIambassadorImplementation::getAvailableDimensionsForInteractionClass(rti1516ev::InteractionClassHandle rti1516InteractionClassHandle)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    return OpenRTI::_O1516EDimensionHandleSet(_ambassadorInterface->getAvailableDimensionsForInteractionClass(interactionClassHandle));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516ev::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::TransportationType
RTIambassadorImplementation::getTransportationType(std::wstring const & transportationName)
{
  try {
    return translate(_ambassadorInterface->getTransportationType(ucsToUtf8(transportationName)));
  } catch (const OpenRTI::InvalidTransportationName& e) {
    throw rti1516ev::InvalidTransportationName(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getTransportationName(rti1516ev::TransportationType transportationType)
{
  try {
    return utf8ToUcs(_ambassadorInterface->getTransportationName(translate(transportationType)));
  } catch (const OpenRTI::InvalidTransportationType& e) {
    throw rti1516ev::InvalidTransportationType(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::OrderType
RTIambassadorImplementation::getOrderType(std::wstring const & orderName)
{
  try {
    return translate(_ambassadorInterface->getOrderType(ucsToUtf8(orderName)));
  } catch (const OpenRTI::InvalidOrderName& e) {
    throw rti1516ev::InvalidOrderName(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getOrderName(rti1516ev::OrderType orderType)
{
  try {
    return utf8ToUcs(_ambassadorInterface->getOrderName(translate(orderType)));
  } catch (const OpenRTI::InvalidOrderType& e) {
    throw rti1516ev::InvalidOrderType(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableObjectClassRelevanceAdvisorySwitch()
{
  try {
    _ambassadorInterface->enableObjectClassRelevanceAdvisorySwitch();
  } catch (const OpenRTI::ObjectClassRelevanceAdvisorySwitchIsOn& e) {
    throw rti1516ev::ObjectClassRelevanceAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableObjectClassRelevanceAdvisorySwitch()
{
  try {
    _ambassadorInterface->disableObjectClassRelevanceAdvisorySwitch();
  } catch (const OpenRTI::ObjectClassRelevanceAdvisorySwitchIsOff& e) {
    throw rti1516ev::ObjectClassRelevanceAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableAttributeRelevanceAdvisorySwitch()
{
  try {
    _ambassadorInterface->enableAttributeRelevanceAdvisorySwitch();
  } catch (const OpenRTI::AttributeRelevanceAdvisorySwitchIsOn& e) {
    throw rti1516ev::AttributeRelevanceAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableAttributeRelevanceAdvisorySwitch()
{
  try {
    _ambassadorInterface->disableAttributeRelevanceAdvisorySwitch();
  } catch (const OpenRTI::AttributeRelevanceAdvisorySwitchIsOff& e) {
    throw rti1516ev::AttributeRelevanceAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableAttributeScopeAdvisorySwitch()
{
  try {
    _ambassadorInterface->enableAttributeScopeAdvisorySwitch();
  } catch (const OpenRTI::AttributeScopeAdvisorySwitchIsOn& e) {
    throw rti1516ev::AttributeScopeAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableAttributeScopeAdvisorySwitch()
{
  try {
    _ambassadorInterface->disableAttributeScopeAdvisorySwitch();
  } catch (const OpenRTI::AttributeScopeAdvisorySwitchIsOff& e) {
    throw rti1516ev::AttributeScopeAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableInteractionRelevanceAdvisorySwitch()
{
  try {
    _ambassadorInterface->enableInteractionRelevanceAdvisorySwitch();
  } catch (const OpenRTI::InteractionRelevanceAdvisorySwitchIsOn& e) {
    throw rti1516ev::InteractionRelevanceAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableInteractionRelevanceAdvisorySwitch()
{
  try {
    _ambassadorInterface->disableInteractionRelevanceAdvisorySwitch();
  } catch (const OpenRTI::InteractionRelevanceAdvisorySwitchIsOff& e) {
    throw rti1516ev::InteractionRelevanceAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::DimensionHandleSet
RTIambassadorImplementation::getDimensionHandleSet(rti1516ev::RegionHandle rti1516RegionHandle)
{
  try {
    OpenRTI::_I1516ERegionHandle regionHandle(rti1516RegionHandle);
    return OpenRTI::_O1516EDimensionHandleSet(_ambassadorInterface->getDimensionHandleSet(regionHandle));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::RangeBounds
RTIambassadorImplementation::getRangeBounds(rti1516ev::RegionHandle rti1516RegionHandle,
                                            rti1516ev::DimensionHandle rti1516DimensionHandle)
{
  try {
    OpenRTI::_I1516ERegionHandle regionHandle(rti1516RegionHandle);
    OpenRTI::_I1516EDimensionHandle dimensionHandle(rti1516DimensionHandle);
    return OpenRTI::_O1516ERangeBounds(_ambassadorInterface->getRangeBounds(regionHandle, dimensionHandle));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionDoesNotContainSpecifiedDimension& e) {
    throw rti1516ev::RegionDoesNotContainSpecifiedDimension(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::setRangeBounds(rti1516ev::RegionHandle rti1516RegionHandle,
                                            rti1516ev::DimensionHandle rti1516DimensionHandle,
                                            rti1516ev::RangeBounds const & rti1516RangeBounds)
{
  try {
    OpenRTI::_I1516ERegionHandle regionHandle(rti1516RegionHandle);
    OpenRTI::_I1516EDimensionHandle dimensionHandle(rti1516DimensionHandle);
    OpenRTI::_I1516ERangeBounds rangeBounds(rti1516RangeBounds);
    _ambassadorInterface->setRangeBounds(regionHandle, dimensionHandle, rangeBounds);
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516ev::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516ev::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionDoesNotContainSpecifiedDimension& e) {
    throw rti1516ev::RegionDoesNotContainSpecifiedDimension(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRangeBound& e) {
    throw rti1516ev::InvalidRangeBound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

unsigned long
RTIambassadorImplementation::normalizeFederateHandle(rti1516ev::FederateHandle rti1516FederateHandle)
{
  try {
    OpenRTI::_I1516EFederateHandle federateHandle(rti1516FederateHandle);
    return _ambassadorInterface->normalizeFederateHandle(federateHandle);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidFederateHandle& e) {
    throw rti1516ev::InvalidFederateHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

unsigned long
RTIambassadorImplementation::normalizeServiceGroup(rti1516ev::ServiceGroup rti1516ServiceGroup)
{
  try {
    return _ambassadorInterface->normalizeServiceGroup(translate(rti1516ServiceGroup));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516ev::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidServiceGroup& e) {
    throw rti1516ev::InvalidServiceGroup(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516ev::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::evokeCallback(double approximateMinimumTimeInSeconds)
{
  try {
    if (_ambassadorInterface->_inCallback)
      throw OpenRTI::CallNotAllowedFromWithinCallback();
    RTI1516EAmbassadorInterface::CallbackScope callbackScope(*_ambassadorInterface);
    return _ambassadorInterface->evokeCallback(approximateMinimumTimeInSeconds);
  } catch (const OpenRTI::CallNotAllowedFromWithinCallback& e) {
    throw rti1516ev::CallNotAllowedFromWithinCallback(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                                                    double approximateMaximumTimeInSeconds)
{
  try {
    if (_ambassadorInterface->_inCallback)
      throw OpenRTI::CallNotAllowedFromWithinCallback();
    RTI1516EAmbassadorInterface::CallbackScope callbackScope(*_ambassadorInterface);
    return _ambassadorInterface->evokeMultipleCallbacks(approximateMinimumTimeInSeconds, approximateMaximumTimeInSeconds);
  } catch (const OpenRTI::CallNotAllowedFromWithinCallback& e) {
    throw rti1516ev::CallNotAllowedFromWithinCallback(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableCallbacks()
{
  try {
    _ambassadorInterface->enableCallbacks();
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableCallbacks()
{
  try {
    _ambassadorInterface->disableCallbacks();
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516ev::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516ev::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

std::unique_ptr<rti1516ev::LogicalTimeFactory>
RTIambassadorImplementation::getTimeFactory() const
{
  if (!_ambassadorInterface->isConnected())
    throw rti1516ev::NotConnected(L"");
  if (!_ambassadorInterface->getFederate())
    throw rti1516ev::FederateNotExecutionMember(L"");
  try {
    return _ambassadorInterface->getTimeFactory();
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::FederateHandle
RTIambassadorImplementation::decodeFederateHandle(rti1516ev::VariableLengthData const & encodedValue) const
{
  if (!_ambassadorInterface->isConnected())
    throw rti1516ev::NotConnected(L"decodeFederateHandle()");
  if (!_ambassadorInterface->getFederate())
    throw rti1516ev::FederateNotExecutionMember(L"decodeFederateHandle()");
  try {
    return rti1516ev::FederateHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516ev::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ObjectClassHandle
RTIambassadorImplementation::decodeObjectClassHandle(rti1516ev::VariableLengthData const & encodedValue) const
{
  if (!_ambassadorInterface->isConnected())
    throw rti1516ev::NotConnected(L"decodeObjectClassHandle()");
  if (!_ambassadorInterface->getFederate())
    throw rti1516ev::FederateNotExecutionMember(L"decodeObjectClassHandle()");
  try {
    return rti1516ev::ObjectClassHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516ev::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::InteractionClassHandle
RTIambassadorImplementation::decodeInteractionClassHandle(rti1516ev::VariableLengthData const & encodedValue) const
{
  if (!_ambassadorInterface->isConnected())
    throw rti1516ev::NotConnected(L"decodeInteractionClassHandle()");
  if (!_ambassadorInterface->getFederate())
    throw rti1516ev::FederateNotExecutionMember(L"decodeInteractionClassHandle()");
  try {
    return rti1516ev::InteractionClassHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516ev::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ObjectInstanceHandle
RTIambassadorImplementation::decodeObjectInstanceHandle(rti1516ev::VariableLengthData const & encodedValue) const
{
  if (!_ambassadorInterface->isConnected())
    throw rti1516ev::NotConnected(L"decodeObjectInstanceHandle()");
  if (!_ambassadorInterface->getFederate())
    throw rti1516ev::FederateNotExecutionMember(L"decodeObjectInstanceHandle()");
  try {
    return rti1516ev::ObjectInstanceHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516ev::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::AttributeHandle
RTIambassadorImplementation::decodeAttributeHandle(rti1516ev::VariableLengthData const & encodedValue) const
{
  if (!_ambassadorInterface->isConnected())
    throw rti1516ev::NotConnected(L"decodeAttributeHandle()");
  if (!_ambassadorInterface->getFederate())
    throw rti1516ev::FederateNotExecutionMember(L"decodeAttributeHandle()");
  try {
    return rti1516ev::AttributeHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516ev::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::ParameterHandle
RTIambassadorImplementation::decodeParameterHandle(rti1516ev::VariableLengthData const & encodedValue) const
{
  if (!_ambassadorInterface->isConnected())
    throw rti1516ev::NotConnected(L"decodeParameterHandle()");
  if (!_ambassadorInterface->getFederate())
    throw rti1516ev::FederateNotExecutionMember(L"decodeParameterHandle()");
  try {
    return rti1516ev::ParameterHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516ev::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::DimensionHandle
RTIambassadorImplementation::decodeDimensionHandle(rti1516ev::VariableLengthData const & encodedValue) const
{
  if (!_ambassadorInterface->isConnected())
    throw rti1516ev::NotConnected(L"decodeDimensionHandle()");
  if (!_ambassadorInterface->getFederate())
    throw rti1516ev::FederateNotExecutionMember(L"decodeDimensionHandle()");
  try {
    return rti1516ev::DimensionHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516ev::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::MessageRetractionHandle
RTIambassadorImplementation::decodeMessageRetractionHandle(rti1516ev::VariableLengthData const & encodedValue) const
{
  if (!_ambassadorInterface->isConnected())
    throw rti1516ev::NotConnected(L"decodeMessageRetractionHandle()");
  if (!_ambassadorInterface->getFederate())
    throw rti1516ev::FederateNotExecutionMember(L"decodeMessageRetractionHandle()");
  try {
    return rti1516ev::MessageRetractionHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516ev::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516ev::RegionHandle
RTIambassadorImplementation::decodeRegionHandle(rti1516ev::VariableLengthData const & encodedValue) const
{
  if (!_ambassadorInterface->isConnected())
    throw rti1516ev::NotConnected(L"decodeRegionHandle()");
  if (!_ambassadorInterface->getFederate())
    throw rti1516ev::FederateNotExecutionMember(L"decodeRegionHandle()");
  try {
    return rti1516ev::RegionHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516ev::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516ev::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516ev::RTIinternalError(L"Unknown internal error!");
  }
}

void RTIambassadorImplementation::setNotificationHandle(rti1516ev::RTInotificationHandle* h)
{
  _ambassadorInterface->setNotificationHandle(std::make_shared<NotificationHandleWrapper<rti1516ev::RTInotificationHandle> >(h));
}


void RTIambassadorImplementation::StartNetworkStatistics(bool startTimer)
{
#ifdef ENABLE_NETWORKSTATISTICS
  GetNetworkStatistics().Start(startTimer);
#endif
}


void RTIambassadorImplementation::StopNetworkStatistics()
{
#ifdef ENABLE_NETWORKSTATISTICS
  GetNetworkStatistics().Stop();
#endif
}


void RTIambassadorImplementation::ResetNetworkStatistics()
{
#ifdef ENABLE_NETWORKSTATISTICS
  GetNetworkStatistics().Reset();
#endif
}


void RTIambassadorImplementation::setInteractionClassDeliverToSelf(rti1516ev::InteractionClassHandle rti1516eInteractionClassHandle, bool enable)
{
  OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516eInteractionClassHandle);
_ambassadorInterface->setInteractionClassDeliverToSelf(interactionClassHandle, enable);
}


void RTIambassadorImplementation::setObjectClassDeliverToSelf(rti1516ev::ObjectClassHandle rti1516eobjectClassHandle, bool enable)
{
  OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516eobjectClassHandle);
_ambassadorInterface->setObjectClassDeliverToSelf(objectClassHandle, enable);
}


void RTIambassadorImplementation::setConnectWaitTimeout(uint32_t timeoutMilliSeconds)
{
  _ambassadorInterface->setConnectWaitTimeoutMilliSeconds(timeoutMilliSeconds);
}

void RTIambassadorImplementation::setOperationWaitTimeout(uint32_t timeoutMilliSeconds)
{
  _ambassadorInterface->setOperationWaitTimeoutMilliSeconds(timeoutMilliSeconds);
}

}
