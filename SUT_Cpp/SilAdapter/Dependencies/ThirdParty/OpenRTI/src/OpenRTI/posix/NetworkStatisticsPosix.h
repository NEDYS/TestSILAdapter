#pragma once

#include "NetworkStatistics.h"

// ems: fake network statistics for posix.

namespace OpenRTI
{

class NetworkStatisticsPosix : public NetworkStatistics
{
  public:
    NetworkStatisticsPosix() = default;
    ~NetworkStatisticsPosix() = default;

    void Start(bool startTimer) override;
    void Stop() override;
    void DumpStatistics() override;
    void DumpTotals() override;

    static NetworkStatisticsPosix& GetInstance();
};

} // namespace OpenRTI
