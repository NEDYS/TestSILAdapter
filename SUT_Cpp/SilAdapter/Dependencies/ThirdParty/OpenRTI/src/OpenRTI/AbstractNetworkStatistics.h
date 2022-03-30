#pragma once

#include "Export.h"
#include <cstring> // for size_t

//#undef ENABLE_NETWORKSTATISTICS
#define ENABLE_NETWORKSTATISTICS

namespace OpenRTI {

class INetworkStatistics
{
public:
#ifdef ENABLE_NETWORKSTATISTICS
  virtual void BytesReceived(size_t bytes) = 0;
  virtual void BytesSent(size_t bytes) = 0;
  virtual void MessageSent(const char* typeName) = 0;
  virtual void MessageReceived(const char* typeName) = 0;
  virtual void DumpStatistics() = 0;
  virtual void DumpTotals() = 0;
  virtual void Reset() = 0;
  virtual void Start(bool startTimer) = 0;
  virtual void Stop() = 0;
#endif
};

#ifdef ENABLE_NETWORKSTATISTICS
OPENRTI_API INetworkStatistics& GetNetworkStatistics();
#endif
}
