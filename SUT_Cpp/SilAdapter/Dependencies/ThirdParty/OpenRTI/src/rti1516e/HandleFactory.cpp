
#include "DebugNew.h"
#include "HandleFactory.h"
#include "HandleImplementation.h"
#include "Handle.h"

namespace rti1516e
{

FederateHandle
createFederateHandle(uint64_t handleValue)
{
  OpenRTI::FederateHandle openRtiHandle(static_cast<OpenRTI::FederateHandle::value_type>(handleValue));
  FederateHandle rti1516eHandle;
  FederateHandleFriend::copy(rti1516eHandle, openRtiHandle);
  return rti1516eHandle;
}

ObjectClassHandle
createObjectClassHandle(uint64_t handleValue)
{
  OpenRTI::ObjectClassHandle openRtiHandle(static_cast<OpenRTI::ObjectClassHandle::value_type>(handleValue));
  ObjectClassHandle rti1516eHandle;
  ObjectClassHandleFriend::copy(rti1516eHandle, openRtiHandle);
  return rti1516eHandle;
}


RTI_EXPORT ObjectInstanceHandle createObjectInstanceHandle(uint64_t handleValue)
{
  OpenRTI::ObjectInstanceHandle openRtiHandle(static_cast<OpenRTI::ObjectInstanceHandle::value_type>(handleValue));
  ObjectInstanceHandle rti1516eHandle;
  ObjectInstanceHandleFriend::copy(rti1516eHandle, openRtiHandle);
  return rti1516eHandle;
}

RTI_EXPORT AttributeHandle createAttributeHandle(uint64_t handleValue)
{
  OpenRTI::AttributeHandle openRtiHandle(static_cast<OpenRTI::AttributeHandle::value_type>(handleValue));
  AttributeHandle rti1516eHandle;
  AttributeHandleFriend::copy(rti1516eHandle, openRtiHandle);
  return rti1516eHandle;
}

RTI_EXPORT InteractionClassHandle createInteractionClassHandle(uint64_t handleValue)
{
  OpenRTI::InteractionClassHandle openRtiHandle(static_cast<OpenRTI::InteractionClassHandle::value_type>(handleValue));
  InteractionClassHandle rti1516eHandle;
  InteractionClassHandleFriend::copy(rti1516eHandle, openRtiHandle);
  return rti1516eHandle;
}

RTI_EXPORT ParameterHandle createParameterHandle(uint64_t handleValue)
{
  OpenRTI::ParameterHandle openRtiHandle(static_cast<OpenRTI::ParameterHandle::value_type>(handleValue));
  ParameterHandle rti1516eHandle;
  ParameterHandleFriend::copy(rti1516eHandle, openRtiHandle);
  return rti1516eHandle;
}

}

