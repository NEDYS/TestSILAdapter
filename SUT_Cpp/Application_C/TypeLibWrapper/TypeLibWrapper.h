/*
 * This file contains the C-C++ glue code to access the C++
 * SiL-Adapter from the C application. Needs to be provided by the user.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

  enum State
  {
    STATE_OFF,
    STATE_ON
  };

  // Vector::CANoe::Typelib::Init()
  void InitTypelib();

  // Vector::CANoe::Typelib::ShutDown()
  void ShutDownTypelib();

  // RoomTemperatureControl::Sensor1::Temperature
  double RoomTemperatureControl_Sensor1_Temperature_Get();

  // RoomTemperatureControl::Sensor2::Temperature
  double RoomTemperatureControl_Sensor2_Temperature_Get();

  // RoomTemperatureControl::Sensor3::Temperature
  double RoomTemperatureControl_Sensor3_Temperature_Get();

  // RoomTemperatureControl::Heating::CoolingState
  void RoomTemperatureControl_Heating_CoolingState_Set(enum State coolingState);

  // RoomTemperatureControl::Heating::HeatingState
  void RoomTemperatureControl_Heating_HeatingState_Set(enum State heatingState);

#ifdef __cplusplus
}
#endif
