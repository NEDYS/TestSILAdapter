// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "DataTypes.hpp"
#include "ISerializer.hpp"
#include "IDeserializer.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

class IValueEntity {
public:
  virtual ~IValueEntity() = default;

  //! \returns the Serializer for this ValueEntity. The returned pointer is valid throughout the lifetime of this ValueEntity
  virtual ISerializer* GetSerializer() const = 0;
  //! \returns the Deserializer for this ValueEntity. The returned pointer is valid throughout the lifetime of this ValueEntity
  virtual IDeserializer* GetDeserializer() const = 0;

  virtual bool IsValid() const = 0;
  virtual ValueState GetState() const = 0;
  virtual Time GetLastUpdateTime() const = 0;
  virtual Time GetLastChangeTime() const = 0;

  virtual CallbackHandle RegisterCallback(std::function<void(IValueEntity&)> callback, ValueUpdateMode mode) = 0;
  virtual void UnregisterCallback(CallbackHandle handle) = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
