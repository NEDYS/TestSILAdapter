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
#include "LeafServerThread.h"

#include "MessageQueue.h"
#include "Mutex.h"
#include "NetworkServer.h"
#include "ScopeUnlock.h"
#include "ServerNode.h"
#include "ServerOptions.h"
#include "ThreadServer.h"

namespace OpenRTI {
  
static const uint32_t kInfinite = static_cast<uint32_t>(-1);

class OPENRTI_LOCAL LeafServerThread::_Registry final : public Referenced {
public:
  _Registry();
  _Registry(const _Registry&) = delete;
  ~_Registry() noexcept;
  void Finalize();
  static _Registry& GetInstance();

  SharedPtr<AbstractConnect> connect(const URL& url, const StringStringListMap& clientOptions, uint32_t timeoutMilliSeconds);

  void erase(LeafServerThread& serverThread);

  static SharedPtr<AbstractServer> createServer(const URL& url, const SharedPtr<AbstractServerNode>& serverNode, uint32_t timeoutMilliSeconds);
  static SharedPtr<AbstractServerNode> createServerNode();

private:
  mutable Mutex _mutex;
  UrlServerMap _urlServerMap;
};

LeafServerThread::_Registry::_Registry()
{
}

LeafServerThread::_Registry::~_Registry() noexcept
{
  Finalize();
}

void LeafServerThread::_Registry::Finalize()
{
  ScopeLock scopeLock(_mutex);
  for (UrlServerMap::iterator i = _urlServerMap.begin(); i != _urlServerMap.end();) {
    /// Need to be safe for map erase during shutdown
    SharedPtr<LeafServerThread> leafServerThread;
    leafServerThread.swap((i++)->second);
    if (!leafServerThread.valid())
      continue;
    ScopeUnlock scopeUnlock(_mutex);
    leafServerThread->postShutdown();
  }
  _urlServerMap.clear();
}

SharedPtr<AbstractConnect>
LeafServerThread::_Registry::connect(const URL& url, const StringStringListMap& clientOptions, uint32_t timeoutMilliSeconds)
{
  ScopeLock scopeLock(_mutex);
  StringStringListMap serverThreadOptions = clientOptions;
  UrlServerMap::iterator i = _urlServerMap.find(url);
  if (i != _urlServerMap.end()) {
    SharedPtr<LeafServerThread> leafServerThread = i->second;
    if (serverThreadOptions.find("version") == serverThreadOptions.end())
    {
      auto version = leafServerThread->_server->getProtocolVersion();
      serverThreadOptions["version"] = std::list<std::string>{std::to_string(version)};
    }
    SharedPtr<AbstractConnect> connect = leafServerThread->connect(serverThreadOptions);
    /// Even if we have a server it might have already decided to stop working.
    /// If it is working it is guaranteed to get at least a connect to this server thread.
    if (connect.valid())
      return connect;

    /// If we get here, the server must be already stopped
    /// and below this server entry in the map is being replaced by a new one.
    /// Make sure the iterator does not point to anything important.
    i->second->_iterator = _urlServerMap.end();
  } else {
    i = _urlServerMap.insert(UrlServerMap::value_type(url, SharedPtr<LeafServerThread>())).first;
  }

  /// This is by default the rti server node.
  /// For testing we can plug something different
  SharedPtr<AbstractServerNode> serverNode;
  try {
    serverNode = createServerNode();
  } catch (...) {
  }
  if (!serverNode.valid()) {
    _urlServerMap.erase(i);
    return SharedPtr<AbstractConnect>();
  }
  serverNode->getServerOptions().setServerName("ambassadorConnect");

  /// Depending on the url create a server
  SharedPtr<AbstractServer> server;
  try {
    server = createServer(url, serverNode, timeoutMilliSeconds);
  } catch (...) {
  }
  if (!server.valid()) {
    _urlServerMap.erase(i);
    return SharedPtr<AbstractConnect>();
  }
  server->getServerNode();
  i->second = MakeShared<LeafServerThread>(server);
  i->second->_iterator = i;
  i->second->start();
  serverThreadOptions["version"] = std::list<std::string>{std::to_string(server->getProtocolVersion())};
  return i->second->connect(serverThreadOptions);
}

void
LeafServerThread::_Registry::erase(LeafServerThread& serverThread)
{
  /// Only remove the map entry if the map entry still points to the thread in question
  ScopeLock scopeLock(_mutex);
  if (serverThread._iterator != _urlServerMap.end() && serverThread._iterator->second.get() == &serverThread)
    _urlServerMap.erase(serverThread._iterator);
  serverThread._iterator = _urlServerMap.end();
}

SharedPtr<AbstractServer>
LeafServerThread::_Registry::createServer(const URL& url, const SharedPtr<AbstractServerNode>& serverNode, uint32_t timeoutMilliSeconds)
{
  // rti://localhost connect is the default.
  if (url.getProtocol().empty() || url.getProtocol() == "rti" || url.getProtocol() == "pipe") {
    SharedPtr<NetworkServer> server = MakeShared<NetworkServer>(serverNode);

    server->setServerName("Leaf server");
    AbsTimeout timeout((timeoutMilliSeconds == kInfinite) ? Clock::max() : Clock::now() + Clock::fromMilliSeconds(timeoutMilliSeconds));
    server->connectParentServer(url, timeout);
    //DebugPrintf("parent protocol version=%d\n", server->getProtocolVersion());

    return server;
  } else if (url.getProtocol() == "thread") {
    return MakeShared<ThreadServer>(serverNode);
  } else if (url.getProtocol() == "rtinode") {
    SharedPtr<NetworkServer> server = MakeShared<NetworkServer>(serverNode);

    server->setServerName(url.str());
    for (std::size_t i = 0; i < url.getNumQueries(); ++i) {
      StringPair stringPair = url.getQuery(i);
      if (stringPair.first == "config") {
        server->setUpFromConfig(stringPair.second);
      } else if (stringPair.first == "listen") {
        server->listen(URL::fromUrl(stringPair.second), 20);
      } else if (stringPair.first == "parent") {
        AbsTimeout timeout((timeoutMilliSeconds == kInfinite) ? Clock::max() : Clock::now() + Clock::fromMilliSeconds(timeoutMilliSeconds));
        server->connectParentServer(URL::fromUrl(stringPair.second), timeout);
      }
    }

    return server;
  } else {
    return SharedPtr<AbstractServer>();
  }
}

SharedPtr<AbstractServerNode>
LeafServerThread::_Registry::createServerNode()
{
  return MakeShared<ServerNode>();
}

LeafServerThread::_Registry& LeafServerThread::_Registry::GetInstance()
{
  static LeafServerThread::_Registry _instance;
  return _instance;
}

LeafServerThread::LeafServerThread(const SharedPtr<AbstractServer>& server) :
  _server(server)
{
  OpenRTIAssert(_server.valid());
}

LeafServerThread::~LeafServerThread() noexcept
{
}

SharedPtr<AbstractConnect>
LeafServerThread::connect(const StringStringListMap& clientOptions)
{
  return _server->postConnect(clientOptions);
}


void LeafServerThread::shutdown()
{
  _Registry::GetInstance().Finalize();
}

void
LeafServerThread::postShutdown()
{
  _server->postDone();
  wait();
}

SharedPtr<AbstractConnect>
LeafServerThread::connect(const URL& url, const StringStringListMap& clientOptions, uint32_t timeoutMilliSeconds)
{
  _Registry& registry = _Registry::GetInstance();
  return registry.connect(url, clientOptions, timeoutMilliSeconds);
}

void
LeafServerThread::run()
{
  _server->exec();

  _Registry& registry = _Registry::GetInstance();
  registry.erase(*this);
}

} // namespace OpenRTI
