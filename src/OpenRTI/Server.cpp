/* -*-c++-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich
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

#include "Server.h"

#include <fstream>
#include <sstream>

#include "Clock.h"
#include "DefaultErrorHandler.h"
#include "Exception.h"
#include "ExpatXMLReader.h"
#include "InitialClientStreamProtocol.h"
#include "MessageEncodingRegistry.h"
#include "MessageList.h"
#include "MessageServer.h"
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
#include "SocketWakeupEvent.h"
#include "SocketWakeupTrigger.h"
#include "StringUtils.h"

namespace OpenRTI {

// This one is to communicate from the ambassador to the server.
class OPENRTI_LOCAL Server::TriggeredConnectSocketEvent : public AbstractSocketEvent {
  class OPENRTI_LOCAL LockedMessageList : public Referenced {
  public:
    void push_back(const SharedPtr<AbstractMessage>& message)
    {
      ScopeLock scopeLock(_mutex);
      _messageList.push_back(message);
    }
    SharedPtr<AbstractMessage> pop_front()
    {
      ScopeLock scopeLock(_mutex);
      return _messageList.pop_front();
    }
  public:
    Mutex _mutex;
    MessageList _messageList;
  };

public:
  // Need to provide the server side message sender.
  TriggeredConnectSocketEvent(const SharedPtr<AbstractMessageSender>& serverMessageSender) :
    _socketWakeupTrigger(new SocketWakeupTrigger),
    _socketWakeupEvent(_socketWakeupTrigger->connect()),
    _lockedMessageList(new LockedMessageList),
    _serverMessageSender(serverMessageSender)
  {
  }

  virtual void read(SocketEventDispatcher& dispatcher)
  {
    ssize_t ret = _socketWakeupEvent->read();
    for (;;) {
      SharedPtr<AbstractMessage> message = _lockedMessageList->pop_front();
      if (!message.valid())
        break;
      if (!_serverMessageSender.valid())
        continue;
      _serverMessageSender->send(message);
    }

    if (ret == -1) {
      // Protocol errors in any sense lead to a closed connection
      dispatcher.erase(this);
      /// Send something to the originator FIXME
    }
  }
  virtual bool getEnableRead() const
  { return true; }

  virtual void write(SocketEventDispatcher& dispatcher)
  { }
  virtual bool getEnableWrite() const
  { return false; }

  virtual SocketWakeupEvent* getSocket() const
  { return _socketWakeupEvent.get(); }

  // The ambassador side message sender.
  AbstractMessageSender* getAmbassadorMessageSender()
  {
    return new MessageSender(_socketWakeupTrigger, _lockedMessageList);
  }

private:
  class OPENRTI_LOCAL MessageSender : public AbstractMessageSender {
  public:
    MessageSender(const SharedPtr<SocketWakeupTrigger>& socketWakeupTrigger,
                  const SharedPtr<LockedMessageList>& lockedMessageList) :
      _socketWakeupTrigger(socketWakeupTrigger),
      _lockedMessageList(lockedMessageList)
    {
    }
    virtual ~MessageSender()
    {
      close();
    }
    virtual void send(const SharedPtr<AbstractMessage>& message)
    {
      if (!_socketWakeupTrigger.valid())
        throw RTIinternalError("Trying to send message to a closed MessageSender");
      _lockedMessageList->push_back(message);
      _socketWakeupTrigger->trigger();
    }
    virtual void close()
    {
      if (!_socketWakeupTrigger.valid())
        return;
      _socketWakeupTrigger->close();
      _socketWakeupTrigger = 0;
      _lockedMessageList = 0;
    }

  private:
    SharedPtr<SocketWakeupTrigger> _socketWakeupTrigger;
    SharedPtr<LockedMessageList> _lockedMessageList;
  };

  // FIXME reverse the connect direction in the trigger/event pair
  // Or think about a different api
  SharedPtr<SocketWakeupTrigger> _socketWakeupTrigger;
  SharedPtr<SocketWakeupEvent> _socketWakeupEvent;
  SharedPtr<LockedMessageList> _lockedMessageList;
  // The server side message sender. Here we need to have a server hanging.
  SharedPtr<AbstractMessageSender> _serverMessageSender;
};

Server::Server() :
  _messageServer(new MessageServer)
{
}

Server::~Server()
{
}

const std::string&
Server::getServerName() const
{
  return _messageServer->getServerName();
}

void
Server::setServerName(const std::string& name)
{
  _messageServer->setServerName(name);
}

void
Server::setUpFromConfig(const std::string& config)
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
Server::setUpFromConfig(std::istream& stream)
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

  _messageServer->getServerOptions()._preferCompression = contentHandler->getEnableZLibCompression();
  _messageServer->getServerOptions()._permitTimeRegulation = contentHandler->getPermitTimeRegulation();

  if (!contentHandler->getParentServerUrl().empty())
    connectParentServer(contentHandler->getParentServerUrl(), Clock::now() + Clock::fromSeconds(90));

  for (unsigned i = 0; i < contentHandler->getNumListenConfig(); ++i)
    listen(contentHandler->getListenConfig(i).getUrl(), 20);
}

void
Server::listen(const std::string& url, int backlog)
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
Server::listenInet(const std::string& address, int backlog)
{
  std::pair<std::string, std::string> hostPortPair;
  hostPortPair = parseInetAddress(address);
  listenInet(hostPortPair.first, hostPortPair.second, backlog);
}

void
Server::listenInet(const std::string& node, const std::string& service, int backlog)
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
Server::listenInet(const SocketAddress& socketAddress, int backlog)
{
  SharedPtr<SocketServerTCP> socket = new SocketServerTCP;
  socket->bind(socketAddress);
  socket->listen(backlog);
  SocketAddress boundAddress = socket->getsockname();
  _dispatcher.insert(new SocketServerAcceptEvent(socket, *this));
  return boundAddress;
}

void
Server::listenPipe(const std::string& address, int backlog)
{
  SharedPtr<SocketServerPipe> socket = new SocketServerPipe();
  socket->bind(address);
  socket->listen(backlog);
  _dispatcher.insert(new SocketServerAcceptEvent(socket, *this));
}

SharedPtr<SocketTCP>
Server::connectedTCPSocket(const std::string& name)
{
  std::pair<std::string, std::string> hostPortPair;
  hostPortPair = parseInetAddress(name);

  // Note that here the may be lenghty name lookup for the connection address happens
  std::list<SocketAddress> addressList = SocketAddress::resolve(hostPortPair.first, hostPortPair.second, false);
  while (!addressList.empty()) {
    try {
      return connectedTCPSocket(addressList.front());
    } catch (const OpenRTI::Exception& e) {
      addressList.pop_front();
      if (addressList.empty())
        throw;
    }
  }
  throw RTIinternalError(std::string("Can not resolve address") + name);
}

SharedPtr<SocketTCP>
Server::connectedTCPSocket(const SocketAddress& socketAddress)
{
  SharedPtr<SocketTCP> socketStream = new SocketTCP;
  socketStream->connect(socketAddress);
  return socketStream;
}

void
Server::connectParentServer(const std::string& url, const Clock& abstime)
{
  std::pair<std::string, std::string> protocolAddressPair = getProtocolRestPair(url);
  if (protocolAddressPair.first == "rti") {
    connectParentInetServer(protocolAddressPair.second, abstime);
  } else if (protocolAddressPair.first == "pipe" || protocolAddressPair.first == "file" || protocolAddressPair.first.empty()) {
    connectParentPipeServer(protocolAddressPair.second, abstime);
  } else {
    throw RTIinternalError(std::string("Trying to listen on \"") + url + "\": Unknown protocol type!");
  }
}

void
Server::connectParentInetServer(const std::string& name, const Clock& abstime)
{
  connectParentStreamServer(connectedTCPSocket(name), abstime);
}

void
Server::connectParentInetServer(const SocketAddress& socketAddress, const Clock& abstime)
{
  connectParentStreamServer(connectedTCPSocket(socketAddress), abstime);
}

void
Server::connectParentPipeServer(const std::string& name, const Clock& abstime)
{
  std::string path = OpenRTI_DEFAULT_PIPE_PATH;
  if (!name.empty())
    path = name;

  // Try to connect to a pipe socket
  SharedPtr<SocketPipe> socketStream = new SocketPipe;
  socketStream->connect(path);

  connectParentStreamServer(socketStream, abstime);
}

// Creates a new server thread that is connected to a parent server through the socket stream
void
Server::connectParentStreamServer(const SharedPtr<SocketStream>& socketStream, const Clock& abstime)
{
  // Set up the server configured option map
  StringStringListMap connectOptions;
  if (_messageServer->getServerOptions()._preferCompression) {
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
Server::connectServer(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions)
{
  SharedPtr<AbstractMessageSender> serverMessageSender;
  serverMessageSender = _messageServer->insertConnect(messageSender, clientOptions);
  if (!serverMessageSender.valid())
    return 0;

  SharedPtr<TriggeredConnectSocketEvent> socketEvent;
  socketEvent = new TriggeredConnectSocketEvent(serverMessageSender);
  _dispatcher.insert(socketEvent.get());

  SharedPtr<AbstractMessageSender> toServerSender;
  toServerSender = socketEvent->getAmbassadorMessageSender();

  return toServerSender;
}

AbstractServerNode&
Server::getServerNode()
{
  return *_messageServer;
}

void
Server::setDone()
{
  _dispatcher.setDone(true);
  _dispatcher.wakeUp();
}

void
Server::setDone(bool done)
{
  _dispatcher.setDone(done);
}

bool
Server::getDone() const
{
  return _dispatcher.getDone();
}

void
Server::wakeUp()
{
  _dispatcher.wakeUp();
}

int
Server::exec()
{
  return _dispatcher.exec();
}

} // namespace OpenRTI
