/* -*-c++-*- OpenRTI - Copyright (C) 2009-2010 Mathias Froehlich
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

#include "Clock.h"
#include "Exception.h"
#include "MessageEncodingRegistry.h"
#include "MessageServer.h"
#include "MessageSocketReadEvent.h"
#include "MessageSocketWriteEvent.h"
#include "SocketEventDispatcher.h"
#include "SocketAddress.h"
#include "SocketPipe.h"
#include "SocketServerPipe.h"
#include "SocketServerTCP.h"
#include "SocketServerAcceptEvent.h"
#include "SocketTCP.h"
#include "StringUtils.h"
#include "ZLibCallbacks.h"

namespace OpenRTI {

Server::Server() :
  _messageServer(new MessageServer)
{
}

Server::~Server()
{
}

const std::wstring&
Server::getName() const
{
  return _messageServer->getName();
}

void
Server::setName(const std::wstring& name)
{
  _messageServer->setName(name);
}

void
Server::listenInet(const std::wstring& address, int backlog)
{
  std::pair<std::wstring, std::wstring> hostPortPair;
  hostPortPair = parseInetAddress(address);

  std::list<SocketAddress> addressList = SocketAddress::resolve(hostPortPair.first, hostPortPair.second);
  // Set up a stream socket for the server connect
  bool success = false;
  for (std::list<SocketAddress>::const_iterator i = addressList.begin(); i != addressList.end(); ++i) {
    try {
      SharedPtr<SocketServerTCP> socket = new SocketServerTCP;
      socket->bind(*i);
      socket->listen(backlog);
      _dispatcher.insert(new SocketServerAcceptEvent(socket, _messageServer));
      success = true;
    } catch (const OpenRTI::Exception& e) {
      addressList.pop_front();
      if (addressList.empty() && !success)
        throw e;
    }
  }
}

void
Server::listenPipe(const std::wstring& address, int backlog)
{
  SharedPtr<SocketServerPipe> socket = new SocketServerPipe();
  socket->bind(address);
  socket->listen(backlog);
  _dispatcher.insert(new SocketServerAcceptEvent(socket, _messageServer));
}

SharedPtr<SocketTCP>
Server::connectedTCPSocket(const std::wstring& name)
{
  std::pair<std::wstring, std::wstring> hostPortPair;
  hostPortPair = parseInetAddress(name);

  // Note that here the may be lenghty name lookup for the connection address happens
  std::list<SocketAddress> addressList = SocketAddress::resolve(hostPortPair.first, hostPortPair.second);
  while (!addressList.empty()) {
    try {
      SharedPtr<SocketTCP> socketStream = new SocketTCP;
      socketStream->connect(addressList.front());
      return socketStream;
    } catch (const OpenRTI::Exception& e) {
      addressList.pop_front();
      if (addressList.empty())
        throw e;
    }
  }
  throw RTIinternalError(std::wstring(L"Can not resolve address") + name);
}

void
Server::connectParentInetServer(const std::wstring& name, const Clock& abstime)
{
  connectParentStreamServer(connectedTCPSocket(name), abstime);
}

void
Server::connectParentPipeServer(const std::wstring& name, const Clock& abstime)
{
  std::wstring path = localeToUcs(OpenRTI_DEFAULT_PIPE_PATH);
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
  bool compress = true;
  // Negotiate with the server how to encode
  MessageEncoderPair encodingPair = MessageEncodingRegistry::instance().negotiateEncoding(socketStream, abstime, compress);

  // The socket side message encoder and output message queue
  SharedPtr<MessageSocketWriteEvent> writeMessageSocketEvent = new MessageSocketWriteEvent(socketStream, encodingPair.first);
  /// The connection that sends messages up to the parent
  SharedPtr<AbstractMessageSender> toParentSender = writeMessageSocketEvent->getMessageSender();

  /// returns a sender where incomming messages should be sent to
  SharedPtr<AbstractMessageSender> toServerSender = _messageServer->insertParentConnect(toParentSender);
  if (!toServerSender.valid())
    throw RTIinternalError(L"Could not set up parent server connect");

  // The socket side message parser and dispatcher that fires the above on completely
  // received messages
  SharedPtr<MessageSocketReadEvent> readMessageSocketEvent;
  readMessageSocketEvent = new MessageSocketReadEvent(socketStream, toServerSender.get(), encodingPair.second);

  if (compress) {
#ifdef OPENRTI_HAVE_ZLIB
    readMessageSocketEvent->setReceiveCallback(new ZLibReceiveCallback);
    writeMessageSocketEvent->setSendCallback(new ZLibSendCallback);
#else
    Log(MessageCoding, Warning) << "Parent server negotiated zlib compression, but client does not support that!" << std::endl;
    throw RTIinternalError(L"Parent server negotiated zlib compression, but client does not support that!");
#endif
  }

  _dispatcher.insert(readMessageSocketEvent.get());
  _dispatcher.insert(writeMessageSocketEvent.get());
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

int
Server::exec()
{
  return _dispatcher.exec();
}

int
Server::exec(const Clock& abstime)
{
  return _dispatcher.exec(abstime);
}

} // namespace OpenRTI
