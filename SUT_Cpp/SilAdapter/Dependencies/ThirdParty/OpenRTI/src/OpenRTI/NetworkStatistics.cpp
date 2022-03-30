
#include "DebugNew.h"
#include "NetworkStatistics.h"
#include "dprintf.h"

#include <limits.h>

namespace OpenRTI {

NetworkStatistics::NetworkStatistics() : _Collecting(false)
{
}


NetworkStatistics::~NetworkStatistics()
{
}

#ifdef ENABLE_NETWORKSTATISTICS
void NetworkStatistics::Reset()
{
  _BytesReceived = 0;
  _BytesReceivedTotal = 0;
  _BytesSent = 0;
  _BytesSentTotal = 0;
  _MessagesSent = 0;
  _MessagesSentTotal = 0;
  _MessagesReceived = 0;
  _MessagesReceivedTotal = 0;
  _MessagesSentByType.clear();
  _MessagesReceivedByType.clear();
}


void NetworkStatistics::BytesReceived(size_t bytes)
{
  if (_Collecting)
  {
    _BytesReceived += bytes;
    _BytesReceivedTotal += bytes;
  }
}

void NetworkStatistics::BytesSent(size_t bytes)
{
  if (_Collecting)
  {
    _BytesSent += bytes;
    _BytesSentTotal += bytes;
  }
}

void NetworkStatistics::MessageReceived(const char* typeName)
{
  if (_Collecting)
  {
    _MessagesReceived++;
    _MessagesReceivedTotal++;
    _MessagesReceivedByType[typeName]++;
  }
}

void NetworkStatistics::MessageSent(const char* typeName)
{
  if (_Collecting)
  {
    _MessagesSent++;
    _MessagesSentTotal++;
    _MessagesSentByType[typeName]++;
  }
}

#endif
}