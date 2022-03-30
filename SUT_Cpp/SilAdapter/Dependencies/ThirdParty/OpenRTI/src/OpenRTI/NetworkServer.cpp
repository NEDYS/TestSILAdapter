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

#include "DebugNew.h"
#include "NetworkServer.h"

#include <fstream>
#include <sstream>

#include "Clock.h"
#include "DefaultErrorHandler.h"
#include "Exception.h"
#include "ExpatXMLReader.h"
#include "InitialClientStreamProtocol.h"
#include "MessageEncodingRegistry.h"
#include "ScopeLock.h"
#include "ScopeUnlock.h"
#include "ServerNode.h"
#include "ServerOptions.h"
#include "Mutex.h"
#include "ProtocolSocketEvent.h"
#include "ScopeLock.h"
#include "ServerConfigContentHandler.h"
#include "SocketEventDispatcher.h"
#include "SocketAddress.h"
#include "SocketPipe.h"
#include "SocketServerPipe.h"
#include "SocketServerTCP.h"
#include "SocketServerAcceptEvent.h"
#include "SocketTCP.h"
#include "StringUtils.h"
#include "LogStream.h"
#include "ServerModel.h"
#include "AbsTimeout.h"

namespace OpenRTI {

NetworkServer::NetworkServer() :
  AbstractServer(MakeShared<ServerNode>())
{
}

NetworkServer::NetworkServer(const SharedPtr<AbstractServerNode>& serverNode) :
  AbstractServer(serverNode)
{
}

NetworkServer::~NetworkServer()
{
  _queue.send(*this);
}

void
NetworkServer::setServerName(const std::string& name)
{
  return getServerNode().getServerOptions().setServerName(name);
}

void
NetworkServer::setUpFromConfig(const std::string& config)
{
  // The server configutation file is an xml file which does start with a '<'.
  // Where a protocol string cannot start with this character.
  if (!config.empty() && config[0] == '<') {
    std::stringstream stream(config);
    setUpFromConfig(stream);
  } else {
    URL url = URL::fromUrl(config);
    if (url.getProtocol() == "file") {
      std::ifstream stream(utf8ToLocale(url.getPath()).c_str());
      if (!stream.is_open())
        throw RTIinternalError("Could not open server config file: \"" + url.str() + "\"!");
      setUpFromConfig(stream);
    } else if (!config.empty()) {
      std::ifstream stream(config.c_str());
      if (!stream.is_open())
        throw RTIinternalError("Could not open server config file: \"" + url.str() + "\"!");
      setUpFromConfig(stream);
    } else {
      throw RTIinternalError("Unknown config file url: \"" + url.str() + "\"!");
    }
  }
}

void
NetworkServer::setUpFromConfig(std::istream& stream)
{
  // Set up the config file parser
  SharedPtr<XML::XMLReader> reader;
  reader = MakeShared<XML::ExpatXMLReader>();

  SharedPtr<ServerConfigContentHandler> contentHandler = MakeShared<ServerConfigContentHandler>();
  reader->setContentHandler(contentHandler);
  SharedPtr<DefaultErrorHandler> errorHandler = MakeShared<DefaultErrorHandler>();
  reader->setErrorHandler(errorHandler);

  reader->parse(stream, "UTF-8");

  std::string errorMessage = errorHandler->getMessages();
  if (!errorMessage.empty())
    throw RTIinternalError(errorMessage);

  if (contentHandler->isLogPrioritySet()) LogStream::setPriority(contentHandler->getLogPriority());
  if (contentHandler->isLogCategorySet()) LogStream::setCategory(contentHandler->getLogCategory());
  if (contentHandler->isLogFileSet()) LogStream::AddLogFile(contentHandler->getLogFile());
  if (contentHandler->isLogToConsoleSet()) LogStream::EnableLogToConsole(contentHandler->getLogToConsole());

  _dispatcher.setQueueLimit(contentHandler->getQueueLimit());
  getServerNode().getServerOptions()._preferCompression = contentHandler->getEnableZLibCompression();

  if (!contentHandler->getParentServerUrl().empty()) {
    URL url = URL::fromUrl(contentHandler->getParentServerUrl());
    if (url.getProtocol().empty()) {
      if (!url.getPath().empty())
        url.setProtocol("pipe");
      else
        url.setProtocol("rti");
    }
    AbsTimeout timeout(Clock::now() + Clock::fromSeconds(90));
    connectParentServer(url, timeout);
  }

  for (unsigned i = 0; i < contentHandler->getNumListenConfig(); ++i) {
    URL url = URL::fromUrl(contentHandler->getListenConfig(i).getUrl());
    if (url.getProtocol().empty()) {
      if (!url.getService().empty())
        url.setProtocol("rti");
      else
        url.setProtocol("pipe");
    }
    listen(url, 20);
  }
}

void
NetworkServer::listen(const URL& url, int backlog)
{
  if (url.getProtocol() == "rti" || url.getProtocol() == "rtic") {
    std::string service = url.getService();
    if (service.empty())
      service = OpenRTI_DEFAULT_PORT_STRING;
    listenInet(url.getHost(), service, backlog);
  } else if (url.getProtocol() == "pipe" || url.getProtocol() == "file") {
    listenPipe(url.getPath(), backlog);
  } else {
    throw RTIinternalError(std::string("Trying to listen on \"") + url.str() + "\": Unknown protocol type!");
  }
}

void
NetworkServer::listenInet(const std::string& node, const std::string& service, int backlog)
{
  std::list<SocketAddress> addressList = SocketAddress::resolve(node, service, true);
  // Set up a stream socket for the server connect
  bool success = false;
  while (!addressList.empty()) {
    SocketAddress address = addressList.front();
    addressList.pop_front();
    try {
      listenInet(address, backlog);
      success = true;
    } catch (const OpenRTI::Exception&) {
      if (addressList.empty() && !success)
        throw;
    }
  }
}

SocketAddress
NetworkServer::listenInet(const SocketAddress& socketAddress, int backlog)
{
  SharedPtr<SocketServerTCP> socket = MakeShared<SocketServerTCP>();
  socket->bind(socketAddress);
  socket->listen(backlog);
  SocketAddress boundAddress = socket->getsockname();
  _dispatcher.insert(MakeShared<SocketServerAcceptEvent>(socket, *this));
  return boundAddress;
}

void
NetworkServer::listenPipe(const std::string& address, int backlog)
{
  SharedPtr<SocketServerPipe> socket = MakeShared<SocketServerPipe>();
  socket->bind(address);
  socket->listen(backlog);
  _dispatcher.insert(MakeShared<SocketServerAcceptEvent>(socket, *this));
}

void
NetworkServer::connectParentServer(const URL& url, const AbsTimeout& timeout)
{
  if (url.getProtocol().empty() || url.getProtocol() == "rti" || url.getProtocol() == "rtic") {
    std::string host = url.getHost();
    if (host.empty())
      host = OpenRTI_DEFAULT_HOST_STRING;
    std::string service = url.getService();
    if (service.empty())
      service = OpenRTI_DEFAULT_PORT_STRING;
    bool compress = false;
    if (url.getProtocol() == "rtic")
      compress = true;
    connectParentInetServer(host, service, compress, timeout);
  } else if (url.getProtocol() == "pipe" || url.getProtocol() == "file") {
    std::string path = url.getPath();
    if (path.empty())
      path = OpenRTI_DEFAULT_PIPE_PATH;
    connectParentPipeServer(path, timeout);
  } else {
    throw RTIinternalError(std::string("Trying to connect to \"") + url.str() + "\": Unknown protocol type!");
  }
}

void
NetworkServer::connectParentInetServer(const std::string& host, const std::string& service, bool compress, const AbsTimeout& timeout)
{
  // Note that here the may be lenghty name lookup for the connection address happens
  std::list<SocketAddress> addressList = SocketAddress::resolve(host, service, false);
  while (!addressList.empty()) {
    try {
      connectParentInetServer(addressList.front(), compress, timeout);
      return;
    } catch (const OpenRTI::Exception&) {
      addressList.pop_front();
      if (addressList.empty())
        throw;
    }
  }
  throw RTIinternalError(std::string("Can not resolve address: \"") + host + std::string(":")
                         + service + std::string("\""));
}

void
NetworkServer::connectParentInetServer(const SocketAddress& socketAddress, bool compress, const AbsTimeout& timeout)
{
  SharedPtr<SocketTCP> socketStream = MakeShared<SocketTCP>();
  socketStream->connect(socketAddress);
  connectParentStreamServer(socketStream, timeout, compress);
}

void
NetworkServer::connectParentPipeServer(const std::string& name, const AbsTimeout& timeout)
{
  // Try to connect to a pipe socket
  SharedPtr<SocketPipe> socketStream = MakeShared<SocketPipe>();
  socketStream->connect(name);

  connectParentStreamServer(socketStream, timeout, false);
}

// Creates a new server thread that is connected to a parent server through the socket stream
void
NetworkServer::connectParentStreamServer(const SharedPtr<SocketStream>& socketStream, const AbsTimeout& timeout, bool compress)
{
  // Set up the server configured option map
  StringStringListMap connectOptions;
  if (compress) {
    connectOptions["compression"].push_back("zlib");
    connectOptions["compression"].push_back("lzma");
  } else {
    connectOptions["compression"].push_back("no");
  }

  // Set up the protocol and socket events for connection startup
  SharedPtr<ProtocolSocketEvent> protocolSocketEvent = MakeShared<ProtocolSocketEvent>(socketStream);
  SharedPtr<InitialClientStreamProtocol> clientStreamProtocol = MakeShared<InitialClientStreamProtocol>(*this, connectOptions);
  protocolSocketEvent->setProtocolLayer(clientStreamProtocol);
  _dispatcher.insert(protocolSocketEvent);

  // Process messages until we have either received the servers response or the timeout expires
  do {
    _dispatcher.exec(timeout);
  } while (!timeout.isExpired() && !_dispatcher.getDone());

  setDone(false);

  if (!clientStreamProtocol->getSuccessfulConnect()) {
    if (!clientStreamProtocol->getErrorMessage().empty())
      throw RTIinternalError(clientStreamProtocol->getErrorMessage());
    throw RTIinternalError("Timeout connecting to parent server!");
  }
}

int
NetworkServer::exec()
{
  ScopeLock scopeLock(_mutex);
  while (!getDone()) {
    if (_queue.empty()) {
      ScopeUnlock scopeUnlock(_mutex);

      _dispatcher.exec();

    } else {

      // Get pending messages.
      _Queue queue;
      _queue.swap(queue);

      {
        // Unlock the ingoing message queue as long
        // as the server is processing.
        ScopeUnlock scopeUnlock(_mutex);

        // now send to the messages to the server
        queue.send(*this);
      }

      // For recycling the queue entries, we need the lock again
      queue.recycle(_pool);
    }
  }

  return EXIT_SUCCESS;
}


uint32_t NetworkServer::getProtocolVersion() const
{
  const ServerNode* serverNode = dynamic_cast<const ServerNode*>(&getServerNode());
  if (serverNode != nullptr)
  {
    return serverNode->getParentConnect()->getVersion();
  }
  return 0;
}

void
NetworkServer::_sendDone(bool done)
{
  AbstractServer::_sendDone(done);
  _dispatcher.setDone(done);
}

void
NetworkServer::_postMessage(const _MessageConnectHandlePair& messageConnectHandlePair)
{
  ScopeLock scopeLock(_mutex);
  bool empty = _queue.empty();
  //DebugPrintf("%s: message=%s\n", __FUNCTION__, messageConnectHandlePair.first.valid() ? messageConnectHandlePair.first->toString().c_str() : "null");
  _queue.push_back(messageConnectHandlePair, _pool);
  // If the list already contains some unsent messages,
  // the socket event dispatcher is already informed and will also catch this new message.
  if (empty)
    _dispatcher.wakeUp();
}

void
NetworkServer::_postOperation(const SharedPtr<_Operation>& operation)
{
  ScopeLock scopeLock(_mutex);
  bool empty = _queue.empty();
  _queue.push_back(operation, _pool);
  // If the list already contains some unsent messages,
  // the socket event dispatcher is already informed and will also catch this new message.
  if (empty)
    _dispatcher.wakeUp();
}

} // namespace OpenRTI
