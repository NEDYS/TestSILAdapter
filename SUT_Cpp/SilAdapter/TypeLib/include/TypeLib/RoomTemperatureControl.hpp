// This code was generated automatically. Changes to this file may
// cause incorrect behavior and will be lost if the code is regenerated.

#pragma once

#include "TypeLib/DistributedObject.hpp"

#include "RoomTemperatureControl/RoomTemperatureControl_Types.hpp"

#include "RoomTemperatureControl/ISensor.hpp"
#include "RoomTemperatureControl/IHeating.hpp"


namespace RoomTemperatureControl {

extern Vector::CANoe::TypeLib::DistributedObject<ISensor> Sensor1;
extern Vector::CANoe::TypeLib::DistributedObject<ISensor> Sensor2;
extern Vector::CANoe::TypeLib::DistributedObject<ISensor> Sensor3;
extern Vector::CANoe::TypeLib::DistributedObject<IHeating> Heating;

} // namespace RoomTemperatureControl
