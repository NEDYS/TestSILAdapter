/*
 * This file contains the actual application logic.
 */

#include <iomanip>
#include <iostream>

#include "IO.hpp"
#include "Logic.hpp"

static constexpr double gHeatingThreshold = 18.0;
static constexpr double gCoolingThreshold = 23.0;


State GetHeatingState(double temperature) { return (temperature < gHeatingThreshold) ? State::On : State::Off; }

State GetCoolingState(double temperature) { return (temperature > gCoolingThreshold) ? State::On : State::Off; }

void PrintStatusToConsole(double sensor1, double sensor2, double sensor3, State heatingState, State coolingState)
{
    std::cout << std::fixed << std::setprecision(2) << "Sensor1: " << sensor1 << ", Sensor2: " << sensor2
        << ", Sensor3: " << sensor3 << ", heating is " << (heatingState == State::On ? "ON" : "OFF") << ", cooling is "
        << (coolingState == State::On ? "ON" : "OFF") << std::endl;
}

double CalcTemperature(double value1, double value2, double value3) { return (value1 + value2 + value3) / 3; }

void ControlRoomTemperature(int writeToConsole)
{
  const auto sensor1 = GetSensorValue1();
  const auto sensor2 = GetSensorValue2();
  const auto sensor3 = GetSensorValue3();

  const auto temperature = CalcTemperature(sensor1, sensor2, sensor3);

  const auto heatingState = GetHeatingState(temperature);
  const auto coolingState = GetCoolingState(temperature);

  SetHeatingState(heatingState, coolingState);

  if (writeToConsole != 0)
  {
    PrintStatusToConsole(sensor1, sensor2, sensor3, heatingState, coolingState);
  }
}
