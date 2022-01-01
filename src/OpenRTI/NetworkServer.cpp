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

namespace OpenRTI {

NetworkServer::NetworkServer() :
  AbstractServer(new ServerNode)
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
    if (url.getProtocol() == "file" || url.getProtocol().empty()) {
      std::ifstream stream(utf8ToLocale(url.getPath()).c_str());
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
  reader = new XML::ExpatXMLReader;

  SharedPtr<ServerConfigContentHandler> contentHandler = new ServerConfigContentHandler;
  reader->setContentHandler(contentHandler.get());
  SharedPtr<DefaultErrorHandler> errorHandler = new DefaultErrorHandler;
  reader->setErrorHandler(errorHandler.get());

  reader->parse(stream, "UTF-8");

  std::string errorMessage = errorHandler->getMessages();
  if (!errorMessage.empty())
    throw RTIinternalError(errorMessage);

  getServerNode().getServerOptions()._preferCompression = contentHandler->getEnableZLibCompression();
  getServerNode().getServerOptions()._permitTimeRegulation = contentHandler->getPermitTimeRegulation();

  if (!contentHandler->getParentServerUrl().empty()) {
    URL url = URL::fromUrl(contentHandler->getParentServerUrl());
    if (url.getProtocol().empty()) {
      if (!url.getPath().empty())
        url.setProtocol("pipe");
      else
        url.setProtocol("rti");
    }
    connectParentServer(url, Clock::now() + Clock::fromSeconds(90));
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
  SharedPtr<SocketServerTCP> socket = new SocketServerTCP;
  socket->bind(socketAddress);
  socket->listen(backlog);
  SocketAddress boundAddress = socket->getsockname();
  _dispatcher.insert(new SocketServerAcceptEvent(socket, *this));
  return boundAddress;
}

void
NetworkServer::listenPipe(const std::string& address, int backlog)
{
  SharedPtr<SocketServerPipe> socket = new SocketServerPipe();
  socket->bind(address);
  socket->listen(backlog);
  _dispatcher.insert(new SocketServerAcceptEvent(socket, *this));
}

void
NetworkServer::connectParentServer(const URL& url, const Clock& abstime)
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
    connectParentInetServer(host, service, compress, abstime);
  } else if (url.getProtocol() == "pipe" || url.getProtocol() == "file") {
    std::string path = url.getPath();
    if (path.empty())
      path = OpenRTI_DEFAULT_PIPE_PATH;
    connectParentPipeServer(path, abstime);
  } else {
    throw RTIinternalError(std::string("Trying to connect to \"") + url.str() + "\": Unknown protocol type!");
  }
}

void
NetworkServer::connectParentInetServer(const std::string& host, const std::string& service, bool compress, const Clock& abstime)
{
  // Note that here the may be lenghty name lookup for the connection address happens
  std::list<SocketAddress> addressList = SocketAddress::resolve(host, service, false);
  while (!addressList.empty()) {
    try {
      connectParentInetServer(addressList.front(), compress, abstime);
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
NetworkServer::connectParentInetServer(const SocketAddress& socketAddress, bool compress, const Clock& abstime)
{
  SharedPtr<SocketTCP> socketStream = new SocketTCP;
  socketStream->connect(socketAddress);
  connectParentStreamServer(socketStream, abstime, compress);
}

void
NetworkServer::connectParentPipeServer(const std::string& name, const Clock& abstime)
{
  // Try to connect to a pipe socket
  SharedPtr<SocketPipe> socketStream = new SocketPipe;
  socketStream->connect(name);

  connectParentStreamServer(socketStream, abstime, false);
}

// Creates a new server thread that is connected to a parent server through the socket stream
void
NetworkServer::connectParentStreamServer(const SharedPtr<SocketStream>& socketStream, const Clock& abstime, bool compress)
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
  SharedPtr<ProtocolSocketEvent> protocolSocketEvent = new ProtocolSocketEvent(socketStream);
  SharedPtr<InitialClientStreamProtocol> clientStreamProtocol = new InitialClientStreamProtocol(*this, connectOptions);
  protocolSocketEvent->setProtocolLayer(clientStreamProtocol);
  _dispatcher.insert(protocolSocketEvent);

  // Process messages until we have either received the servers response or the timeout expires
  do {
    _dispatcher.exec(abstime);
  } while (Clock::now() <= abstime && !_dispatcher.getDone());

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
