
#pragma once
#include <mutex>
#include <map>
#include <atomic>
#include "AbstractNetworkStatistics.h"

namespace OpenRTI {

class NetworkStatistics : public INetworkStatistics
{
public:
#ifdef ENABLE_NETWORKSTATISTICS
  void BytesReceived(size_t bytes) override;
  void BytesSent(size_t bytes) override;
  void MessageSent(const char* typeName) override;
  void MessageReceived(const char* typeName) override;
  void Reset() override;
#endif
protected:
  NetworkStatistics();
  ~NetworkStatistics();
#ifdef ENABLE_NETWORKSTATISTICS
  std::atomic<bool> _Collecting;
  size_t _BytesReceived;
  size_t _BytesReceivedTotal;
  size_t _BytesSent;
  size_t _BytesSentTotal;
  size_t _MessagesSent;
  size_t _MessagesSentTotal;
  size_t _MessagesReceived;
  size_t _MessagesReceivedTotal;
  std::map<std::string, size_t> _MessagesReceivedByType;
  std::map<std::string, size_t> _MessagesSentByType;
#endif
};

}
