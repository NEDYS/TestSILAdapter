/* -*-c++-*- OpenRTI - Copyright (C) 2009-2014 Mathias Froehlich
 *
 * This file is part of OpenRTI.
 *
 * OpenRTI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * OpenRTI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenRTI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "DebugNew.h"
#include "SocketEventDispatcher.h"

#include <vector>
#include <map>
#include <cerrno>

#include "AbstractSocketEvent.h"
#include "Clock.h"
#include "ErrnoWin32.h"
#include "Exception.h"
#include "LogStream.h"
#include "SocketPrivateDataWin32.h"
#include "AbstractNetworkStatistics.h"
#include "AbsTimeout.h"

#pragma comment(lib, "ws2_32.lib")

namespace OpenRTI {

struct SocketEventDispatcher::PrivateData
{
    PrivateData() noexcept
      : _wakeupEvent(WSA_INVALID_EVENT)
    {
      try
      {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData))
        {
          throw TransportError("Could not initialize windows sockets!");
        }
        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
        {
          WSACleanup();
          throw TransportError("Could not initialize windows sockets 2.2!");
        }
        _wakeupEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        _wokenUp = false;
      }
      catch (...)
      {
      }
    }

    ~PrivateData() noexcept
    {
      try
      {
        CloseHandle(_wakeupEvent);
        WSACleanup();
      }
      catch (...)
      {
      }
    }

    static void DebugPrintSockets(const std::vector<SharedPtr<AbstractSocketEvent>>& sockets, const std::vector<HANDLE>& notificationEvents)
    {
      DebugPrintf("%s: %d handles:\n", __FUNCTION__, static_cast<int>(notificationEvents.size()));
      for (size_t i = 0; i < notificationEvents.size(); i++)
      {
        DebugPrintf("%s: notificationEvents[%d] = %p\n", __FUNCTION__, static_cast<int>(i), notificationEvents[i]);
      }
      DebugPrintf("%s: %d sockets:\n", __FUNCTION__, static_cast<int>(sockets.size()));
      for (size_t i = 0; i < sockets.size(); i++)
      {
        DebugPrintf("%s: sockets[%d] = 0x%08x\n", __FUNCTION__, static_cast<int>(i), static_cast<uint32_t>(sockets[i]->getSocket()->_privateData->_socket));
      }
    }

    int exec(SocketEventDispatcher& dispatcher, const AbsTimeout& timeout)
    {
      int retv = 0;
      //HANDLE unlockedEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
      while (!dispatcher._done)
      {
        bool doWait = true;
        std::vector<SharedPtr<AbstractSocketEvent>> sockets;
        sockets.reserve(dispatcher._socketEventList.size());

        std::vector<SharedPtr<AbstractSocketEvent>> socketsToErase;

        std::vector<HANDLE> notificationEvents;
        notificationEvents.reserve(dispatcher._socketEventList.size() + 1);

        for (auto& socketEventSP : dispatcher._socketEventList)
        {
          const AbstractSocketEvent* socketEvent = socketEventSP.get();
          Socket* abstractSocket = socketEvent->getSocket();
          if (!abstractSocket)
          {
            continue;
          }
          SOCKET socket = abstractSocket->_privateData->_socket;
          if (socket == INVALID_SOCKET)
          {
            continue;
          }
          if (socket == (SOCKET)SOCKET_ERROR)
          {
            continue;
          }

          bool notificationInserted = false;
          if (socketEvent->getEnableWrite())
          {
            if (socketEvent->getSocket()->isWritable())
            {
              // The socket has data to write, and hasn't been marked as blocking by a previous call to write -
              // jump right into write() again and restart the outer loop after having processed all sockets
              dispatcher.write(socketEventSP);
              doWait = false;
            }
            else
            {
              // The socket has data to write, but either write has not been called before, or the previous
              // call to write() returned WSAEWOULDBLOCK or similar - first check queue limit
              if (socketEvent->getBytesQueued() > dispatcher.getQueueLimit())
              {
                // queue overflow - close connection
                socketsToErase.push_back(socketEventSP);
              }
              else
              {
                // if queue limit has not been reached yet, add the socket to the list of sockets
                // to survey
                sockets.push_back(socketEventSP);
                notificationEvents.push_back(abstractSocket->_privateData->_notificationEvent);
                notificationInserted = true;
              }
            }
          }
          
          if (socketEvent->getEnableRead() && !notificationInserted)
          {
            sockets.push_back(socketEventSP);
            notificationEvents.push_back(abstractSocket->_privateData->_notificationEvent);
          }
        }

        if (!doWait) continue;

        notificationEvents.push_back(_wakeupEvent);

        uint32_t timeoutMs = INFINITE;
        if (timeout.getTimeout() != Clock::max())
        {
          //Clock now = Clock::now();
          Clock remaining;
          if (timeout.isExpired(remaining))
          {
            retv = 0;
            break;
          }
          else
          {
            //timeoutMs = static_cast<uint32_t>((absTimeoutClock - now).getNanoSeconds() / 1000000ULL);
            timeoutMs = static_cast<uint32_t>(remaining.getMilliSeconds());
          }
        }

        DWORD waitResult = WSAWaitForMultipleEvents(static_cast<DWORD>(notificationEvents.size()), notificationEvents.data(), FALSE, timeoutMs, FALSE);
        if (waitResult == WAIT_FAILED)
        {
          DWORD lastError = GetLastError();
          LPVOID lpMsgBuf;

          FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            lastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0, NULL);
          DebugPrintf("%s: WSAWaitForMultipleEvents failed: %S\n", __FUNCTION__, static_cast<const wchar_t*>(lpMsgBuf));
          LocalFree(lpMsgBuf);
          DebugPrintSockets(sockets, notificationEvents);
          retv = -1;
          break;
        }

        // Timeout
        //Clock now = Clock::now();
        if (timeout.isExpired() /*absclock <= now*/ || waitResult == WAIT_TIMEOUT)
        {
          retv = 0;
          break;
        }

        if (waitResult >= WAIT_OBJECT_0 && waitResult < WAIT_OBJECT_0 + notificationEvents.size())
        {
          for (uint32_t index = waitResult - WAIT_OBJECT_0; index < notificationEvents.size() - 1; index++)
          {
            SharedPtr<AbstractSocketEvent> socketEvent = sockets[index];
            Socket* abstractSocket = socketEvent->getSocket();
            if (abstractSocket)
            {
              SOCKET socketHandle = abstractSocket->_privateData->_socket;
              HANDLE notificationEvent = abstractSocket->_privateData->_notificationEvent;

              WSANETWORKEVENTS networkEvents;
              memset(&networkEvents, 0, sizeof(networkEvents));
              int rc = WSAEnumNetworkEvents(socketHandle, notificationEvent, &networkEvents);
              if (rc == SOCKET_ERROR)
              {
                DebugPrintf("%s: wsaError=%d\n", __FUNCTION__, WSAGetLastError());
                return 0;
              }
              if (networkEvents.lNetworkEvents & FD_CLOSE)
              {
                DebugPrintf("close %p\n", socketEvent->getSocket());
                socketEvent->error(OpenRTI::ConnectionFailed("connection closed"));
                dispatcher.erase(socketEvent);
                continue;
              }
              if (networkEvents.lNetworkEvents & FD_READ)
              {
                dispatcher.read(socketEvent);
              }
              if (networkEvents.lNetworkEvents & FD_WRITE)
              {
                socketEvent->getSocket()->setWriteable();
              }
              if (socketEvent->getSocket()->isWritable())
              {
                dispatcher.write(socketEvent);
              }
              if (networkEvents.lNetworkEvents & FD_ACCEPT)
              {
                dispatcher.read(socketEvent);
              }
              // NOTE: dispatcher.timeout/AbstractSocketEvent::timeout is empty anyway
              //if (socketEvent->getTimeout() <= absTimeout.getTimeout())
              //{
              //  dispatcher.timeout(socketEvent);
              //}
            }
          }

          for (auto& socketEventSP : socketsToErase)
          {
            socketEventSP->error(OpenRTI::ConnectionFailed("closing connection due to queue overflow."));
            dispatcher.erase(socketEventSP);
          }

          bool expected = true;
          if (_wokenUp.compare_exchange_weak(expected, false, std::memory_order_acq_rel))
          {
            retv = 0;
            break;
          }
        }
      }

      //DebugPrintf("<<< %s: retv=%d\n", __FUNCTION__, retv);
      return retv;
    }

    void wakeUp()
    {
      // Check if we already have a wakeup pending
      bool expected = false;
      if (!_wokenUp.compare_exchange_weak(expected, true, std::memory_order_acq_rel))
      {
        return;
      }
      // No, the first one, actually wake up the networking thread
      SetEvent(_wakeupEvent);
    }

  private:
    std::atomic_bool _wokenUp;
    HANDLE _wakeupEvent;
};

SocketEventDispatcher::SocketEventDispatcher() :
  _privateData(new PrivateData),
  _done(false),
  _queueLimit(std::numeric_limits<size_t>::max())
{
}

SocketEventDispatcher::~SocketEventDispatcher()
{
  delete _privateData;
  _privateData = 0;
}

void
SocketEventDispatcher::setDone(bool done)
{
  _done = done;
}

bool
SocketEventDispatcher::getDone() const
{
  return _done;
}

void
SocketEventDispatcher::wakeUp()
{
  _privateData->wakeUp();
}

int
SocketEventDispatcher::exec(const AbsTimeout& timeout)
{
  return _privateData->exec(*this, timeout);
}

}
