/*
 * This file contains functions that access the hardware.
 * 
 * These functions represent the functional system interface of the software under test.
 * To establish a communiction with CANoe/CANoe4SW the original source code is substituted 
 * by calls to the SIL Adapter. Necessary adaptations are marked with "INSTRUMENTATION".
 */

#include "IO.h"

// ---------- INSTRUMENTATION ----------
#include "TypeLibWrapper.h"
// -------------------------------------

void InitializeIO() 
{
    // ---------- INSTRUMENTATION ----------
    // hw_init();   // original source code
    InitTypelib();  // call to SIL Adapter
    // -------------------------------------
}

void ShutDownIO() 
{
    // ---------- INSTRUMENTATION ----------
    // hw_shut_down();  // original source code
    ShutDownTypelib();  // call to SIL Adapter
    // -------------------------------------
}

double GetSensorValue1()
{ 
    // ---------- INSTRUMENTATION ----------
    // return hw_read_sensor_value1();                          // original source code
    return RoomTemperatureControl_Sensor1_Temperature_Get();    // call to SIL Adapter
    // -------------------------------------
}

double GetSensorValue2() 
{ 
    // ---------- INSTRUMENTATION ----------
    // return hw_read_sensor_value2();                          // original source code
    return RoomTemperatureControl_Sensor2_Temperature_Get();    // call to SIL Adapter
    // -------------------------------------
}

double GetSensorValue3()
{ 
    // ---------- INSTRUMENTATION ----------
    // return hw_read_sensor_value3();                          // original source code
    return RoomTemperatureControl_Sensor3_Temperature_Get();    // call to SIL Adapter
    // -------------------------------------
}

void SetHeatingState(enum State heatingState, enum State coolingState)
{
    // ---------- INSTRUMENTATION ----------
    // hw_write_switch_heating(heatingState);                       // original source code
    // hw_write_switch_cooling(coolingState);                       // original source code
    RoomTemperatureControl_Heating_HeatingState_Set(heatingState);  // call to SIL Adapter
    RoomTemperatureControl_Heating_CoolingState_Set(coolingState);  // call to SIL Adapter
    // -------------------------------------
}