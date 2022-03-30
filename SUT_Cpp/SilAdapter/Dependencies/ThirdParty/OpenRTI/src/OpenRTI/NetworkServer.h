/* -*-c++-*- OpenRTI - Copyright (C) 2009-2015 Mathias Froehlich
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

#ifndef OpenRTI_NetworkServer_h
#define OpenRTI_NetworkServer_h

#include "AbstractServer.h"
#include "Mutex.h"
#include "SocketAddress.h"
#include "SocketEventDispatcher.h"

namespace OpenRTI {

class URL;

class OPENRTI_API NetworkServer : public AbstractServer {
public:
  NetworkServer();
  NetworkServer(const SharedPtr<AbstractServerNode>& serverNode);
  virtual ~NetworkServer();

  void setServerName(const std::string& name);

  void setUpFromConfig(const std::string& config);
  void setUpFromConfig(std::istream& stream);

  void listen(const URL& url, int backlog);
  void listenInet(const std::string& node, const std::string& service, int backlog);
  SocketAddress listenInet(const SocketAddress& socketAddress, int backlog);
  void listenPipe(const std::string& address, int backlog);

  void connectParentServer(const URL& url, const AbsTimeout& timeout);
  void connectParentInetServer(const std::string& host, const std::string& service, bool compress, const AbsTimeout& timeout);
  void connectParentInetServer(const SocketAddress& socketAddress, bool compress, const AbsTimeout& timeout);
  void connectParentPipeServer(const std::string& name, const AbsTimeout& timeout);
  void connectParentStreamServer(const SharedPtr<SocketStream>& socketStream, const AbsTimeout& timeout, bool compress);

  int exec() override;
  uint32_t getProtocolVersion() const override;
protected:
  void _sendDone(bool done) override;

  void _postMessage(const _MessageConnectHandlePair& messageConnectHandlePair) override;
  void _postOperation(const SharedPtr<_Operation>& operation) override;

private:
  NetworkServer(const NetworkServer&) = delete;
  NetworkServer& operator=(const NetworkServer&) = delete;

  SocketEventDispatcher _dispatcher;

  Mutex _mutex;
  _Queue _queue;
  _MessageConnectHandlePairList _pool;
};

} // namespace OpenRTI

#endif
