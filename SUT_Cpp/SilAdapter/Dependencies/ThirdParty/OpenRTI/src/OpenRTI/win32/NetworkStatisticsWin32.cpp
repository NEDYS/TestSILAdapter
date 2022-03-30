
#include "DebugNew.h"
#include "NetworkStatisticsWin32.h"
#include <sstream>

#pragma comment(lib, "winmm.lib")

namespace OpenRTI {


NetworkStatisticsWin32::NetworkStatisticsWin32() noexcept
#ifdef ENABLE_NETWORKSTATISTICS
  : mTimer(0)
#endif
{
}

NetworkStatisticsWin32::~NetworkStatisticsWin32() noexcept
{
}

#ifdef ENABLE_NETWORKSTATISTICS
void WINAPI NetworkStatisticsWin32::timerCallback(UINT /*uTimerID*/, UINT /*uMsg*/, DWORD_PTR /*dwUser*/, DWORD_PTR /*dw1*/, DWORD_PTR /*dw2*/)
{
  GetNetworkStatistics().DumpStatistics();
}

void NetworkStatisticsWin32::Start(bool startTimer)
{
  _Collecting = true;
  Reset();
  if (mTimer != 0)
  {
    ::timeKillEvent(mTimer);
    mTimer = 0;
  }
  if (startTimer)
  {
    mTimer = ::timeSetEvent(1000, 1, timerCallback, reinterpret_cast<DWORD_PTR>(this), TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
  }
}

void NetworkStatisticsWin32::Stop()
{
  _Collecting = false;
  if (mTimer != 0)
  {
    ::timeKillEvent(mTimer);
    mTimer = 0;
  }
  DumpTotals();
}

void NetworkStatisticsWin32::DumpStatistics()
{
  std::ostringstream to;
  to << "OpenRTI Network Statistics:" << std::endl;
  to << "messages received: " << _MessagesReceivedTotal << "(" << _MessagesReceived <<"/s)" << std::endl;
  to << "messages sent:     " << _MessagesSentTotal << "(" << _MessagesSent <<"/s)" << std::endl;
  to << "bytes received:    " << _BytesReceivedTotal << "(" << _BytesReceived <<"/s)" << std::endl;
  to << "bytes sent:        " << _BytesSentTotal << "(" << _BytesSent <<"/s)" << std::endl;
  OutputDebugStringA(to.str().c_str());
  _MessagesReceived = 0;
  _MessagesSent = 0;
  _BytesSent = 0;
  _BytesReceived = 0;
}

void NetworkStatisticsWin32::DumpTotals()
{
  DumpStatistics();
  std::ostringstream to;
  to << "Messages sent, by type:" << std::endl;
  for (auto it_name_count : _MessagesSentByType)
  {
    to << "  " << it_name_count.first << ":     " << it_name_count.second << std::endl;
  }
  to << "Messages received, by type:" << std::endl;
  for (auto it_name_count : _MessagesReceivedByType)
  {
    to << "  " << it_name_count.first << ":     " << it_name_count.second << std::endl;
  }
  OutputDebugStringA(to.str().c_str());
}


NetworkStatisticsWin32& NetworkStatisticsWin32::GetInstance()
{
  static NetworkStatisticsWin32 _statistics;
  return _statistics;
}

INetworkStatistics& GetNetworkStatistics()
{
  return NetworkStatisticsWin32::GetInstance();
}
#endif

}
