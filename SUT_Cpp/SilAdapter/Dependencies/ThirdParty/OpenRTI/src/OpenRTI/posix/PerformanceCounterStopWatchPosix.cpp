
#include "OpenRTIConfig.h"
#include "PerformanceCounterStopWatchPosix.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper function
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int64_t GetPerformanceCounterFrequency()
{
#if defined (_WIN32)
  LARGE_INTEGER nActualPerformanceFrequency;
  QueryPerformanceFrequency( &nActualPerformanceFrequency );
  return nActualPerformanceFrequency.QuadPart;
#elif defined (__linux__)
//#pragma message __FILE__LINE__ "#System TODO: Implement me"
  return 0;
#endif // _WIN32
}

int64_t GetPerformanceCounterValue()
{
#if defined (_WIN32)
  LARGE_INTEGER nActualPerformanceCounter;
  QueryPerformanceCounter( &nActualPerformanceCounter );
  return nActualPerformanceCounter.QuadPart;
#elif defined (__linux__)
//#pragma message __FILE__LINE__ "#System TODO: Implement me"
  return 0;
#endif // _WIN32
}

int64_t GetPerformanceCounterStopWatchConstructStartStopDuration()
{
  VPerformanceCounterStopWatch oConstructStartStopDurationMeasurement;
  oConstructStartStopDurationMeasurement.Start();
  for( int i = 0; i < 1024; i++ )
  {
    VPerformanceCounterStopWatch oPerformanceCounterTimer;
    oPerformanceCounterTimer.Start();
    oPerformanceCounterTimer.Stop();
  }
  return (oConstructStartStopDurationMeasurement.Stop() >> 10);//Divide by 1024
}

int64_t GetPerformanceCounterStopWatchStartStopDuration()
{
  VPerformanceCounterStopWatch oStartStopDurationMeasurement;
  oStartStopDurationMeasurement.Start();
  VPerformanceCounterStopWatch oPerformanceCounterTimer;
  for( int i = 0; i < 1024; i++ )
  {
    oPerformanceCounterTimer.Start();
    oPerformanceCounterTimer.Stop();
  }
  return (oStartStopDurationMeasurement.Stop() >> 10);//Divide by 1024
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// class VPerformanceCounterStopWatch
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*static*/ const VPerformanceCounterStopWatch::TTimeNs VPerformanceCounterStopWatch::msPerformanceCounterFrequency  = GetPerformanceCounterFrequency();
/*static*/ const VPerformanceCounterStopWatch::TTimeNs VPerformanceCounterStopWatch::msProgramStart                 = GetPerformanceCounterValue();
/*static*/ const VPerformanceCounterStopWatch::TTimeNs VPerformanceCounterStopWatch::ConstructStartStopDuration     = GetPerformanceCounterStopWatchConstructStartStopDuration();
/*static*/ const VPerformanceCounterStopWatch::TTimeNs VPerformanceCounterStopWatch::StartStopDuration              = GetPerformanceCounterStopWatchStartStopDuration();

VPerformanceCounterStopWatch::VPerformanceCounterStopWatch( bool bStartImmediately /*= false*/ )
  : _started(false), moPerformanceCounterValueStart(0)
{
  if( bStartImmediately )
  {
    this->Start();
  }
}

VPerformanceCounterStopWatch::TTimeNs VPerformanceCounterStopWatch::Start()
{
  if (_started) return this->moPerformanceCounterValueStart;
  _started=true;
  return VPerformanceCounterStopWatch::PermformanceCounterValueToNs( (this->moPerformanceCounterValueStart = GetPerformanceCounterValue()) - VPerformanceCounterStopWatch::msProgramStart );
}

VPerformanceCounterStopWatch::TTimeNs VPerformanceCounterStopWatch::Stop()
{
  if (!_started) return 0;
  _started=false;
  return VPerformanceCounterStopWatch::PermformanceCounterValueToNs( GetPerformanceCounterValue() - this->moPerformanceCounterValueStart );
}

/*static*/ VPerformanceCounterStopWatch::TTimeNs VPerformanceCounterStopWatch::PermformanceCounterValueToNs( TTimeNs nPerformanceCounterValue )
{
  return static_cast<TTimeNs>( (static_cast<double>( nPerformanceCounterValue ) * 1000000000.0) / static_cast<double>( VPerformanceCounterStopWatch::msPerformanceCounterFrequency ));
}