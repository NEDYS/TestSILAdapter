/*
 * This file contains the actual application logic.
 */

#include <stdio.h>

#include "IO.h"
#include "Logic.h"

#define HEATING_THRESHOLD 18.0
#define COOLING_THRESHOLD 23.0

enum State GetHeatingState(double temperature) { return (temperature < HEATING_THRESHOLD) ? STATE_ON : STATE_OFF; }

enum State GetCoolingState(double temperature) { return (temperature > COOLING_THRESHOLD) ? STATE_ON : STATE_OFF; }

void PrintStatusToConsole(double sensor1, double sensor2, double sensor3, enum State heatingState, enum State coolingState)
{
  printf("Sensor1: %.2f, Sensor2: %.2f, Sensor3: %.2f, heating is %s, cooling is %s\r\n",
      sensor1,
      sensor2,
      sensor3,
      (heatingState == STATE_ON) ? "ON" : "OFF",
      (coolingState == STATE_ON) ? "ON" : "OFF");
}

double CalcTemperature(double value1, double value2, double value3) { return (value1 + value2 + value3) / 3.0; }

void ControlRoomTemperature(int writeToConsole)
{
  const double sensor1 = GetSensorValue1();
  const double sensor2 = GetSensorValue2();
  const double sensor3 = GetSensorValue3();

  const double temperature = CalcTemperature(sensor1, sensor2, sensor3);

  const enum State heatingState = GetHeatingState(temperature);
  const enum State coolingState = GetCoolingState(temperature);

  SetHeatingState(heatingState, coolingState);

  if (writeToConsole != 0)
  {
    PrintStatusToConsole(sensor1, sensor2, sensor3, heatingState, coolingState);
  }
}
