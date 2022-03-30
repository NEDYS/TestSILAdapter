/*
 * This file contains the application's entry point.
 */

#include <iostream>
#include <chrono>
#include <thread>

#include "Logic.hpp"
#include "IO.hpp"

int main()
{

  std::cout << "Room Temperature Control application, version 1.0" << std::endl;

  InitializeIO();

  int i = 0;
  while (true)
  {
    // Write to console only every second
    i++;
    int writeToConsole = 0;
    if (i >= 9)
    {
      writeToConsole = 1;
      i = 0;
    }
    ControlRoomTemperature(writeToConsole);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  ShutDownIO();

  return 0;
}
