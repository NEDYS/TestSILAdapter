#pragma once

#include <RTI/SpecificConfig.h>
#include <string>
#include <memory>
#include <RTI/Typedefs.h>
#include <RTI/Exception.h>
#include <memory>
#include "RTI/Handle.h"

namespace rti1516e
{
// these functions are for the test suite only and must not be used elsewhere.
RTI_EXPORT FederateHandle createFederateHandle(uint64_t handleValue);
RTI_EXPORT ObjectClassHandle createObjectClassHandle(uint64_t handleValue);
RTI_EXPORT ObjectInstanceHandle createObjectInstanceHandle(uint64_t handleValue);
RTI_EXPORT AttributeHandle createAttributeHandle(uint64_t handleValue);
RTI_EXPORT InteractionClassHandle createInteractionClassHandle(uint64_t handleValue);
RTI_EXPORT ParameterHandle createParameterHandle(uint64_t handleValue);

}