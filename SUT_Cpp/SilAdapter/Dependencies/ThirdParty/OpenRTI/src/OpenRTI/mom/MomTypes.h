#pragma once

#include <map>
#include "Handle.h"
#include "VariableLengthData.h"

namespace OpenRTI {

typedef std::map< AttributeHandle, VariableLengthData > AttributeHandleValueMap;
typedef std::map< ParameterHandle, VariableLengthData > ParameterHandleValueMap;

}
