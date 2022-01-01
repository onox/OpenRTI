/* -*-c++-*- OpenRTI - Copyright (C) 2009-2022 Mathias Froehlich
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

#include "LeafServerThread.h"

#include "MessageQueue.h"
#include "Mutex.h"
#include "NetworkServer.h"
#include "ScopeUnlock.h"
#include "ServerNode.h"
#include "ServerOptions.h"
#include "SingletonPtr.h"
#include "ThreadServer.h"

namespace OpenRTI {

class OPENRTI_LOCAL LeafServerThread::_Registry : public Referenced {
public:
  _Registry();
  ~_Registry();

  SharedPtr<AbstractConnect> connect(const URL& url, const StringStringListMap& clientOptions);

  void erase(LeafServerThread& serverThread);

  static SharedPtr<AbstractServer> createServer(const URL& url, const SharedPtr<AbstractServerNode>& serverNode);
  static SharedPtr<AbstractServerNode> createServerNode();

  static SingletonPtr<_Registry> _instance;

private:
  mutable Mutex _mutex;
  UrlServerMap _urlServerMap;
};

SingletonPtr<LeafServerThread::_Registry>
LeafServerThread::_Registry::_instance;

LeafServerThread::_Registry::_Registry()
{
}

LeafServerThread::_Registry::~_Registry()
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
}

SharedPtr<AbstractConnect>
LeafServerThread::_Registry::connect(const URL& url, const StringStringListMap& clientOptions)
{
  ScopeLock scopeLock(_mutex);
  UrlServerMap::iterator i = _urlServerMap.find(url);
  if (i != _urlServerMap.end()) {
    SharedPtr<AbstractConnect> connect = i->second->connect(clientOptions);
    /// Even if we have a server it might have already decided to stop working.
    /// If it is working it is guarenteed to get at least a connect to this server thread.
    if (connect.valid())
      return connect;

    /// If we get here, the server must be already stopped
    /// and below this server entry in the map is being replaced by a new one.
    /// Make sure the iterator does not point to anything important.
    i->second->_iterator = _urlServerMap.end();
  } else {
    i = _urlServerMap.insert(UrlServerMap::value_type(url, SharedPtr<LeafServerThread>())).first;
  }

  /// This is be default the rti server node.
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
    server = createServer(url, serverNode);
  } catch (...) {
  }
  if (!server.valid()) {
    _urlServerMap.erase(i);
    return SharedPtr<AbstractConnect>();
  }

  i->second = new LeafServerThread(server);
  i->second->_iterator = i;
  i->second->start();
  return i->second->connect(clientOptions);
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
LeafServerThread::_Registry::createServer(const URL& url, const SharedPtr<AbstractServerNode>& serverNode)
{
  // rti://localhost connect is the default.
  if (url.getProtocol().empty() || url.getProtocol() == "rti" || url.getProtocol() == "pipe") {
    SharedPtr<NetworkServer> server = new NetworkServer(serverNode);

    server->setServerName("Leaf server");
    server->connectParentServer(url, Clock::now() + Clock::fromSeconds(70));

    return server;
  } else if (url.getProtocol() == "thread") {
    return new ThreadServer(serverNode);
  } else if (url.getProtocol() == "rtinode") {
    SharedPtr<NetworkServer> server = new NetworkServer(serverNode);

    server->setServerName(url.str());
    for (std::size_t i = 0; i < url.getNumQueries(); ++i) {
      StringPair stringPair = url.getQuery(i);
      if (stringPair.first == "config") {
        server->setUpFromConfig(stringPair.second);
      } else if (stringPair.first == "listen") {
        server->listen(URL::fromUrl(stringPair.second), 20);
      } else if (stringPair.first == "parent") {
        server->connectParentServer(URL::fromUrl(stringPair.second), Clock::now() + Clock::fromSeconds(70));
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
  return new ServerNode;
}

LeafServerThread::LeafServerThread(const SharedPtr<AbstractServer>& server) :
  _server(server)
{
  OpenRTIAssert(_server.valid());
}

LeafServerThread::~LeafServerThread(void)
{
}

SharedPtr<AbstractConnect>
LeafServerThread::connect(const StringStringListMap& clientOptions)
{
  return _server->postConnect(clientOptions);
}

void
LeafServerThread::postShutdown()
{
  _server->postDone();
  wait();
}

SharedPtr<AbstractConnect>
LeafServerThread::connect(const URL& url, const StringStringListMap& clientOptions)
{
  SharedPtr<_Registry> registry = _Registry::_instance.get();
  if (!registry.valid())
    return SharedPtr<AbstractConnect>();

  return registry->connect(url, clientOptions);
}

void
LeafServerThread::run()
{
  _server->exec();

  SharedPtr<_Registry> registry = _Registry::_instance.get();
  if (registry.valid())
    registry->erase(*this);
}

} // namespace OpenRTI
