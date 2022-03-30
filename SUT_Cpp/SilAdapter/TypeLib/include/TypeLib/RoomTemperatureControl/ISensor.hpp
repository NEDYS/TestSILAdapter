// This code was generated automatically. Changes to this file may
// cause incorrect behavior and will be lost if the code is regenerated.

#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <string>

#include "TypeLib/DistributedObjectBase.hpp"

#include "RoomTemperatureControl_Types.hpp"


namespace RoomTemperatureControl {

class ISensor : virtual public Vector::CANoe::TypeLib::DistributedObjectBase
{
public:
  ISensor() = default;

  // -----------------------
  // Data member
  // -----------------------

  Vector::CANoe::TypeLib::DoConsumedData<double> Temperature;

protected:
  friend class Vector::CANoe::TypeLib::Accessor;
  void _Init(const std::string& path) override;
  void _WaitForFirstTransmission() override;
  void _Reset() override;
};

} // namespace RoomTemperatureControl
