/*
 * This file contains functions that access the hardware.
 *
 * These functions represent the functional system interface of the software under test.
 * To establish a communiction with CANoe/CANoe4SW the original source code is substituted
 * by calls to the SIL Adapter. Necessary adaptations are marked with "INSTRUMENTATION".
 */

#include "IO.hpp"

 // ---------- INSTRUMENTATION ----------
#include "TypeLib/TypeLib.hpp"
// -------------------------------------

void InitializeIO() 
{
    // ---------- INSTRUMENTATION ----------
    // hw_init();                   // original source code
    Vector::CANoe::TypeLib::Init(); // call to SIL Adapter
    // -------------------------------------
}

void ShutDownIO() 
{
    // ---------- INSTRUMENTATION ----------
    // hw_shut_down();                  // original source code
    Vector::CANoe::TypeLib::ShutDown(); // call to SIL Adapter
    // -------------------------------------
}

double GetSensorValue1()
{ 
    // ---------- INSTRUMENTATION ----------
    // return hw_read_sensor_value1();                  // original source code
    return RoomTemperatureControl::Sensor1.Temperature; // call to SIL Adapter
    // -------------------------------------
}

double GetSensorValue2()
{ 
    // ---------- INSTRUMENTATION ----------
    // return hw_read_sensor_value2();                  // original source code
    return RoomTemperatureControl::Sensor2.Temperature; // call to SIL Adapter
    // -------------------------------------
}

double GetSensorValue3()
{ 
    // ---------- INSTRUMENTATION ----------
    // return hw_read_sensor_value3();                  // original source code
    return RoomTemperatureControl::Sensor3.Temperature; // call to SIL Adapter
    // -------------------------------------
}

void SetHeatingState(State heatingState, State coolingState)
{
    // ---------- INSTRUMENTATION ----------
    // hw_write_switch_heating(heatingState);   // original source code
    // hw_write_switch_cooling(coolingState);   // original source code
    RoomTemperatureControl::Heating.HeatingState = static_cast<RoomTemperatureControl::State>(heatingState);    // call to SIL Adapter
    RoomTemperatureControl::Heating.CoolingState = static_cast<RoomTemperatureControl::State>(coolingState);    // call to SIL Adapter
    // -------------------------------------
}
