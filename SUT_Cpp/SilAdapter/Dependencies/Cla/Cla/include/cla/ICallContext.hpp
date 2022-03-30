// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ISerializer.hpp"
#include "IDeserializer.hpp"
#include "DataTypes.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {
  
class ICallContext 
{
public:
  virtual ~ICallContext() = default;

  //! \returns the Deserializer for the input parameters of the CallContext. The returned pointer is valid throughout the lifetime of this CallContext
  virtual IDeserializer* GetInputParametersDeserializer() = 0;
  //! \returns the Serializer for the input parameters of the CallContext. The returned pointer is valid throughout the lifetime of this CallContext.
  virtual ISerializer*   GetInputParametersSerializer() = 0;

  //! \returns the Deserializer for the output parameters of the CallContext. The returned pointer is valid throughout the lifetime of this CallContext.
  //! If the method has a return value (non-void) it shall be deserialized as the first value.
  virtual IDeserializer* GetOutputParametersDeserializer() = 0;
  //! \returns the Serializer for the output parameters of the CallContext. The returned pointer is valid throughout the lifetime of this CallContext.
  //! If the method has a return value (non-void) it shall be serialized as the first value.
  virtual ISerializer*   GetOutputParametersSerializer() = 0;

  virtual CallState      GetCallState() const = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
