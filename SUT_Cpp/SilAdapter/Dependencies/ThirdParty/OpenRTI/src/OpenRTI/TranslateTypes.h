
#pragma once

#include <iosfwd>
#include <string>
#include "Export.h"
#include "Message.h"
#include "StringUtils.h"

namespace OpenRTI {

template<typename T>
inline T from_string(const std::string& str)
{
  std::istringstream ss(str);
  T ret;
  ss >> ret;
  return ret;
}

template<>
inline Endianness from_string<Endianness>(const std::string& str)
{
  if (caseInSensitiveStringEqual(str, "little"))
    return Endianness::LittleEndian;
  else
    return Endianness::BigEndian;
}

template<>
inline ArrayDataTypeEncoding from_string<ArrayDataTypeEncoding>(const std::string& str)
{
  if (caseInSensitiveStringEqual(str, "HLAfixedArray"))
    return ArrayDataTypeEncoding::FixedArrayDataTypeEncoding;
  else
    return ArrayDataTypeEncoding::VariableArrayDataTypeEncoding;
}

static inline std::string to_string(ArrayDataTypeEncoding value)
{
  if (value == ArrayDataTypeEncoding::FixedArrayDataTypeEncoding)
    return "HLAfixedArray";
  else
    return "HLAVariableArray";
}

class OPENRTI_API TranslateTypes
{
public:
  TranslateTypes();
  ~TranslateTypes();

  // these translate a new-style FOMStringModule2 (as read from the FDD) into an old-style
  // FOMStringModule (as needed by CreateFederationExecutionRequestMessage 1st version)
  static FOMStringModule translate(const FOMStringModule2& ref);
  static FOMStringModuleList translate(const FOMStringModule2List& ref);
  static FOMStringArrayDataType translate(const FOMStringArrayDataType2& ref);
  static FOMStringArrayDataTypeList translate(const FOMStringArrayDataType2List& ref);

  // these translate an old-style FOM into new-style FOM (as received with a 
  // CreateFederationExecutionRequestMessage 1st version)
  static FOMStringModule2 translate(const FOMStringModule& ref);
  static FOMStringModule2List translate(const FOMStringModuleList& ref);
  static FOMStringArrayDataType2 translate(const FOMStringArrayDataType& ref);
  static FOMStringArrayDataType2List translate(const FOMStringArrayDataTypeList& ref);

  static FOMStringFixedRecordField2 translate(const FOMStringFixedRecordField& ref);
  static FOMStringFixedRecordField2List translate(const FOMStringFixedRecordFieldList& ref);
  static FOMStringFixedRecordDataType2 translate(const FOMStringFixedRecordDataType& ref);
  static FOMStringFixedRecordDataType2List translate(const FOMStringFixedRecordDataTypeList& ref);
  
  // translate a CreateFederationExecutionRequestMessage 1st version
  static CreateFederationExecutionRequest2Message translate(const CreateFederationExecutionRequestMessage& message);
  static JoinFederationExecutionRequest2Message translate(const JoinFederationExecutionRequestMessage& message);
};

}