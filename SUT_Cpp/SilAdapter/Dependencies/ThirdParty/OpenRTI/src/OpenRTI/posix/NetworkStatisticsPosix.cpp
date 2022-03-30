#include "NetworkStatisticsPosix.h"

#include <iostream>

namespace OpenRTI
{

void NetworkStatisticsPosix::Start(bool /*startTimer*/) {}

void NetworkStatisticsPosix::Stop() {}

NetworkStatisticsPosix& NetworkStatisticsPosix::GetInstance()
{
    static NetworkStatisticsPosix stats;
    return stats;
}

void NetworkStatisticsPosix::DumpStatistics()
{
    std::cerr << "NetworkStatisticsPosix::DumpStatistics() is not implemented" << std::endl;
}
void NetworkStatisticsPosix::DumpTotals()
{
    std::cerr << "NetworkStatisticsPosix::DumpTotals() is not implemented" << std::endl;
}

INetworkStatistics& GetNetworkStatistics()
{
  return NetworkStatisticsPosix::GetInstance();
}

} // namespace OpenRTI
