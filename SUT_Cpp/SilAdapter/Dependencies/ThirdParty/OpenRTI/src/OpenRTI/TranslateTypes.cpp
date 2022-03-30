#include "TranslateTypes.h"

namespace OpenRTI {


TranslateTypes::TranslateTypes()
{
}


TranslateTypes::~TranslateTypes()
{
}

FOMStringModule TranslateTypes::translate(const FOMStringModule2& ref)
{
  FOMStringModule result;
  result.setDesignator(ref.getDesignator());
  result.setTransportationTypeList(ref.getTransportationTypeList());
  result.setDimensionList(ref.getDimensionList());
  result.setRoutingSpaceList(ref.getRoutingSpaceList());
  result.setInteractionClassList(ref.getInteractionClassList());
  result.setObjectClassList(ref.getObjectClassList());
  result.setUpdateRateList(ref.getUpdateRateList());
  result.setSwitchList(ref.getSwitchList());
  result.setSimpleDataTypeList(ref.getSimpleDataTypeList());
  result.setEnumeratedDataTypeList(ref.getEnumeratedDataTypeList());
  result.setEnumeratedDataTypeList(ref.getEnumeratedDataTypeList());
  result.setArrayDataTypeList(translate(ref.getArrayDataTypeList()));
  //result.setFixedRecordDataTypeList(ref.getFixedRecordDataTypeList());
  //result.setVariantRecordDataTypeList(ref.getVariantRecordDataTypeList());
  result.setArtificialInteractionRoot(ref.getArtificialInteractionRoot());
  result.setArtificialObjectRoot(ref.getArtificialObjectRoot());
  return result;
}


FOMStringModuleList TranslateTypes::translate(const FOMStringModule2List& ref)
{
  FOMStringModuleList result;
  for (auto& fomModule : ref)
  {
    result.push_back(translate(fomModule));
  }
  return result;
}

FOMStringArrayDataType TranslateTypes::translate(const FOMStringArrayDataType2& ref)
{
  FOMStringArrayDataType result;
  result.setName(ref.getName());
  result.setEncoding(to_string(ref.getEncoding()));
  result.setDataType(ref.getDataType());
  result.setCardinality(ref.getCardinality());
  return result;
}

FOMStringArrayDataTypeList TranslateTypes::translate(const FOMStringArrayDataType2List& ref)
{
  FOMStringArrayDataTypeList result;
  for (auto& dataType : ref)
  {
    result.push_back(translate(dataType));
  }
  return result;
}

OpenRTI::FOMStringModule2 TranslateTypes::translate(const FOMStringModule& ref)
{
  FOMStringModule2 result;
  result.setDesignator(ref.getDesignator());
  result.setTransportationTypeList(ref.getTransportationTypeList());
  result.setDimensionList(ref.getDimensionList());
  result.setRoutingSpaceList(ref.getRoutingSpaceList());
  result.setInteractionClassList(ref.getInteractionClassList());
  result.setObjectClassList(ref.getObjectClassList());
  result.setUpdateRateList(ref.getUpdateRateList());
  result.setSwitchList(ref.getSwitchList());
  result.setSimpleDataTypeList(ref.getSimpleDataTypeList());
  result.setEnumeratedDataTypeList(ref.getEnumeratedDataTypeList());
  result.setEnumeratedDataTypeList(ref.getEnumeratedDataTypeList());
  result.setArrayDataTypeList(translate(ref.getArrayDataTypeList()));
  //result.setFixedRecordDataTypeList(ref.getFixedRecordDataTypeList());
  //result.setVariantRecordDataTypeList(ref.getVariantRecordDataTypeList());
  result.setArtificialInteractionRoot(ref.getArtificialInteractionRoot());
  result.setArtificialObjectRoot(ref.getArtificialObjectRoot());
  return result;
}

OpenRTI::FOMStringModule2List TranslateTypes::translate(const FOMStringModuleList& ref)
{
  FOMStringModule2List result;
  for (auto& fomModule : ref)
  {
    result.push_back(translate(fomModule));
  }
  return result;
}

OpenRTI::FOMStringArrayDataType2 TranslateTypes::translate(const FOMStringArrayDataType& ref)
{
  FOMStringArrayDataType2 result;
  result.setName(ref.getName());
  result.setEncoding(from_string<ArrayDataTypeEncoding>(ref.getEncoding()));
  result.setDataType(ref.getDataType());
  result.setCardinality(ref.getCardinality());
  return result;
}

OpenRTI::FOMStringArrayDataType2List TranslateTypes::translate(const FOMStringArrayDataTypeList& ref)
{
  FOMStringArrayDataType2List result;
  for (auto& dataType : ref)
  {
    result.push_back(translate(dataType));
  }
  return result;
}

FOMStringFixedRecordField2 TranslateTypes::translate(const FOMStringFixedRecordField& ref)
{
  FOMStringFixedRecordField2 result;
  result.setName(ref.getName());
  return result;
}

FOMStringFixedRecordField2List TranslateTypes::translate(const FOMStringFixedRecordFieldList& ref)
{
  FOMStringFixedRecordField2List result;
  for (auto& dataType : ref)
  {
    result.push_back(translate(dataType));
  }
  return result;
}

FOMStringFixedRecordDataType2 TranslateTypes::translate(const FOMStringFixedRecordDataType& ref)
{
  FOMStringFixedRecordDataType2 result;
  result.setName(ref.getName());
  result.setEncoding(ref.getEncoding());
  result.setFields(translate(ref.getFields()));
  return result;
}

FOMStringFixedRecordDataType2List TranslateTypes::translate(const FOMStringFixedRecordDataTypeList& ref)
{
  FOMStringFixedRecordDataType2List result;
  for (auto& dataType : ref)
  {
    result.push_back(translate(dataType));
  }
  return result;
}


CreateFederationExecutionRequest2Message TranslateTypes::translate(const CreateFederationExecutionRequestMessage& message)
{
  CreateFederationExecutionRequest2Message result;
  result.setFederationExecution(message.getFederationExecution());
  result.setFOMStringModuleList(translate(message.getFOMStringModuleList()));
  result.setLogicalTimeFactoryName(message.getLogicalTimeFactoryName());
  return result;
}

JoinFederationExecutionRequest2Message TranslateTypes::translate(const JoinFederationExecutionRequestMessage& ref)
{
  JoinFederationExecutionRequest2Message result;
  result.setFederationExecution(ref.getFederationExecution());
  result.setFederateType(ref.getFederateType());
  result.setFederateName(ref.getFederateName());
  result.setFOMStringModuleList(translate(ref.getFOMStringModuleList()));
  result.setConfigurationParameterMap(ref.getConfigurationParameterMap());
  result.setIsInternal(ref.getIsInternal());
  return result;
}

}