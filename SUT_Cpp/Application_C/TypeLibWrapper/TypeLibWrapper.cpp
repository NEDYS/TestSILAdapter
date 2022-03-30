/*
 * This file contains the C-C++ glue code to access the C++
 * SIL Adapter from the C application. Needs to be provided by the user.
 */

#include "TypeLibWrapper.h"
#include "TypeLib/TypeLib.hpp"

 // disable warnings about C-enums in C++-Code
#pragma warning (disable: 26812) 

extern "C"
{
  void InitTypelib() { Vector::CANoe::TypeLib::Init(); }

  void ShutDownTypelib() { Vector::CANoe::TypeLib::ShutDown(); }

  double RoomTemperatureControl_Sensor1_Temperature_Get() { return RoomTemperatureControl::Sensor1.Temperature; }

  double RoomTemperatureControl_Sensor2_Temperature_Get() { return RoomTemperatureControl::Sensor2.Temperature; }

  double RoomTemperatureControl_Sensor3_Temperature_Get() { return RoomTemperatureControl::Sensor3.Temperature; }

  void RoomTemperatureControl_Heating_CoolingState_Set(enum State coolingState)
  {
    RoomTemperatureControl::Heating.CoolingState = static_cast<RoomTemperatureControl::State>(coolingState);
  }

  void RoomTemperatureControl_Heating_HeatingState_Set(enum State heatingState)
  {
    RoomTemperatureControl::Heating.HeatingState = static_cast<RoomTemperatureControl::State>(heatingState);
  }
}