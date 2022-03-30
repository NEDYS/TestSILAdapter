// This code was generated automatically. Changes to this file may
// cause incorrect behavior and will be lost if the code is regenerated.

#include "TypeLib/TypeLib.hpp"

#include "cla/ClaSil.hpp"

#include "RoomTemperatureControl/RoomTemperatureControl_Initialization.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

void Init()
{
  RoomTemperatureControl::Init();
  Vector::CANoe::Cla::Connect();
  RoomTemperatureControl::WaitForFirstTransmission();
}

void ShutDown()
{
  Vector::CANoe::Cla::Disconnect();
  RoomTemperatureControl::ShutDown();
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
