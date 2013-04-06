/* -*-c++-*- OpenRTI - Copyright (C) 2009-2013 Mathias Froehlich
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
#include "MessageList.h"
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

class OPENRTI_LOCAL NetworkServer::_ToServerMessageSender : public AbstractMessageSender {
public:
  _ToServerMessageSender(const SharedPtr<AbstractServerNode>& serverNode, const ConnectHandle& connectHandle) :
    _serverNode(serverNode),
    _connectHandle(connectHandle)
  { }
  virtual ~_ToServerMessageSender()
  { close(); }
  virtual void send(const SharedPtr<const AbstractMessage>& message)
  {
    if (!_serverNode.valid())
      throw RTIinternalError("Trying to send message to a closed MessageSender");
    if (!message.valid())
      return;
    _serverNode->_dispatchMessage(message.get(), _connectHandle);
  }
  virtual void close()
  {
    if (!_serverNode.valid())
      return;
    _serverNode->_eraseConnect(_connectHandle);
    _serverNode = 0;
    _connectHandle = ConnectHandle();
  }

private:
  SharedPtr<AbstractServerNode> _serverNode;
  ConnectHandle _connectHandle;
};

NetworkServer::NetworkServer() :
  AbstractServer(new ServerNode),
  _done(false),
  _wakeUp(false)
{
}

NetworkServer::NetworkServer(const SharedPtr<AbstractServerNode>& serverNode) :
  AbstractServer(serverNode),
  _done(false),
  _wakeUp(false)
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
  std::pair<std::string, std::string> protocolAddressPair = getProtocolRestPair(config);
  if (protocolAddressPair.first == "file" || protocolAddressPair.first.empty()) {

    std::ifstream stream(utf8ToLocale(protocolAddressPair.second).c_str());
    if (!stream.is_open())
      throw RTIinternalError("Could not open server config file: \"" + protocolAddressPair.second + "\"!");

    setUpFromConfig(stream);

  } else {

    std::stringstream stream(protocolAddressPair.second);
    setUpFromConfig(stream);

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

  reader->parse(stream);

  std::string errorMessage = errorHandler->getMessages();
  if (!errorMessage.empty())
    throw RTIinternalError(errorMessage);

  getServerNode().getServerOptions()._preferCompression = contentHandler->getEnableZLibCompression();
  getServerNode().getServerOptions()._permitTimeRegulation = contentHandler->getPermitTimeRegulation();

  if (!contentHandler->getParentServerUrl().empty())
    connectParentServer(contentHandler->getParentServerUrl(), Clock::now() + Clock::fromSeconds(90));

  for (unsigned i = 0; i < contentHandler->getNumListenConfig(); ++i)
    listen(contentHandler->getListenConfig(i).getUrl(), 20);
}

void
NetworkServer::listen(const std::string& url, int backlog)
{
  std::pair<std::string, std::string> protocolAddressPair = getProtocolRestPair(url);
  if (protocolAddressPair.first == "rti") {
    listenInet(protocolAddressPair.second, backlog);
  } else if (protocolAddressPair.first == "pipe" || protocolAddressPair.first == "file" || protocolAddressPair.first.empty()) {
    listenPipe(protocolAddressPair.second, backlog);
  } else {
    throw RTIinternalError(std::string("Trying to listen on \"") + url + "\": Unknown protocol type!");
  }
}

void
NetworkServer::listenInet(const std::string& address, int backlog)
{
  std::pair<std::string, std::string> hostPortPair;
  hostPortPair = parseInetAddress(address);
  listenInet(hostPortPair.first, hostPortPair.second, backlog);
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
    } catch (const OpenRTI::Exception& e) {
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

SharedPtr<SocketTCP>
NetworkServer::connectedTCPSocket(const SocketAddress& socketAddress)
{
  SharedPtr<SocketTCP> socketStream = new SocketTCP;
  socketStream->connect(socketAddress);
  return socketStream;
}

void
NetworkServer::connectParentServer(const std::string& url, const Clock& abstime)
{
  std::pair<std::string, std::string> protocolAddressPair = getProtocolRestPair(url);
  if (protocolAddressPair.first == "rti") {
    connectParentInetServer(parseInetAddress(protocolAddressPair.second), abstime);
  } else if (protocolAddressPair.first == "pipe" || protocolAddressPair.first == "file" || protocolAddressPair.first.empty()) {
    connectParentPipeServer(protocolAddressPair.second, abstime);
  } else {
    throw RTIinternalError(std::string("Trying to listen on \"") + url + "\": Unknown protocol type!");
  }
}

void
NetworkServer::connectParentInetServer(const std::string& name, const Clock& abstime)
{
  connectParentInetServer(parseInetAddress(name), abstime);
}

void
NetworkServer::connectParentInetServer(const std::pair<std::string, std::string>& hostPortPair, const Clock& abstime)
{
  // Note that here the may be lenghty name lookup for the connection address happens
  std::list<SocketAddress> addressList = SocketAddress::resolve(hostPortPair.first, hostPortPair.second, false);
  while (!addressList.empty()) {
    try {
      connectParentInetServer(addressList.front(), abstime);
      return;
    } catch (const OpenRTI::Exception& e) {
      addressList.pop_front();
      if (addressList.empty())
        throw;
    }
  }
  throw RTIinternalError(std::string("Can not resolve address: \"") + hostPortPair.first + std::string(":")
                         + hostPortPair.second + std::string("\""));
}

void
NetworkServer::connectParentInetServer(const SocketAddress& socketAddress, const Clock& abstime)
{
  connectParentStreamServer(connectedTCPSocket(socketAddress), abstime, socketAddress.isLocal());
}

void
NetworkServer::connectParentPipeServer(const std::string& name, const Clock& abstime)
{
  std::string path = OpenRTI_DEFAULT_PIPE_PATH;
  if (!name.empty())
    path = name;

  // Try to connect to a pipe socket
  SharedPtr<SocketPipe> socketStream = new SocketPipe;
  socketStream->connect(path);

  connectParentStreamServer(socketStream, abstime, true);
}

// Creates a new server thread that is connected to a parent server through the socket stream
void
NetworkServer::connectParentStreamServer(const SharedPtr<SocketStream>& socketStream, const Clock& abstime, bool local)
{
  // Set up the server configured option map
  StringStringListMap connectOptions;
  if (getServerNode().getServerOptions()._preferCompression && !local) {
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

  // Process messages until we have either recieved the servers response or the timeout expires
  do {
    _dispatcher.exec(abstime);
  } while (Clock::now() <= abstime && !_dispatcher.getDone());

  if (!clientStreamProtocol->getSuccessfulConnect()) {
    if (!clientStreamProtocol->getErrorMessage().empty())
      throw RTIinternalError(clientStreamProtocol->getErrorMessage());
    throw RTIinternalError("Timeout connecting to parent server!");
  }

  setDone(false);
}

SharedPtr<AbstractMessageSender>
NetworkServer::insertConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& optionMap)
{
  ConnectHandle connectHandle = getServerNode()._insertConnect(messageSender, optionMap);
  if (!connectHandle.valid())
    return SharedPtr<AbstractMessageSender>();
  return new _ToServerMessageSender(&getServerNode(), connectHandle);
}

SharedPtr<AbstractMessageSender>
NetworkServer::insertParentConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& optionMap)
{
  ConnectHandle connectHandle = getServerNode()._insertParentConnect(messageSender, optionMap);
  if (!connectHandle.valid())
    return SharedPtr<AbstractMessageSender>();
  return new _ToServerMessageSender(&getServerNode(), connectHandle);
}

int
NetworkServer::exec()
{
  ScopeLock scopeLock(_mutex);
  _wakeUp = false;
  while (!_done && !_wakeUp) {
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

bool
NetworkServer::getDone() const
{
  return _done;
}

void
NetworkServer::_sendDone(bool done)
{
  _done = done;
  _dispatcher.setDone(done);
}

void
NetworkServer::_sendWakeUp()
{
  _wakeUp = true;
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
