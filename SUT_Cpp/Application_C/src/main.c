/*
 * This file contains the application's entry point.
 */

#include <stdio.h>

#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

#include "Logic.h"
#include "IO.h"

int main()
{

  printf("Room Temperature Control application, version 1.0\r\n\r\n");

  // Initializes the SiL adapter and the connection to CANoe
  InitializeIO();

  int i = 0;
  for(;;)
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
    Sleep(100);
  }

  // Shuts down the SiL adapter and the connection to CANoe
  ShutDownIO();

  return 0;
}
