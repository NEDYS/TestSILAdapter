#pragma once

#include <stdint.h>
#include <atomic>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Simple performance counter based stop watch
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class VPerformanceCounterStopWatch
{
public:
  typedef int64_t TTimeNs;

  //Constructor
  VPerformanceCounterStopWatch( bool bStartImmediately = false );

  //Sets the start point of the measurement, returns the absolute time span in ns since program start.
  TTimeNs Start();


  //Stop can be called several times after start, you will always get the time since last start.
  //Returns the time since last Start call in ns.
  TTimeNs Stop();

  //Time in ns to construct, start and stop a VPerformanceCounterStopWatch in ns.
  static const TTimeNs ConstructStartStopDuration;
  //Time in ns to start and stop (without construction) a VPerformanceCounterStopWatch in ns.
  static const TTimeNs StartStopDuration;

private:
  static TTimeNs PermformanceCounterValueToNs( TTimeNs nPerformanceCounterValue );
  static const TTimeNs msPerformanceCounterFrequency;
  static const TTimeNs msProgramStart;
  std::atomic<bool> _started;
  TTimeNs moPerformanceCounterValueStart;
};