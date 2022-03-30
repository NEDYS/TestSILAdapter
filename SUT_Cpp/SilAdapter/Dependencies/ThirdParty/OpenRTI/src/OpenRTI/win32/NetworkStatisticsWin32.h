#pragma once
#include <Windows.h>
#include <mmsystem.h>

#include "NetworkStatistics.h"
#include "PerformanceCounterStopWatch.h"

namespace OpenRTI
{

class NetworkStatisticsWin32 : public NetworkStatistics
{
  public:
    NetworkStatisticsWin32() noexcept;
    ~NetworkStatisticsWin32() noexcept;

#ifdef ENABLE_NETWORKSTATISTICS
    void Start(bool startTimer) override;
    void Stop() override;
    void DumpStatistics() override;
    void DumpTotals() override;
    static NetworkStatisticsWin32& GetInstance();
#endif
  private:

#ifdef ENABLE_NETWORKSTATISTICS
    static void WINAPI timerCallback(UINT /*uTimerID*/, UINT /*uMsg*/, DWORD_PTR /*dwUser*/, DWORD_PTR /*dw1*/, DWORD_PTR /*dw2*/);
    MMRESULT mTimer;
    VPerformanceCounterStopWatch _notificationStopWatch;
#endif
};

} // namespace OpenRTI
