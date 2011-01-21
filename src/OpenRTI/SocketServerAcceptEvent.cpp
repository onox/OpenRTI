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

#include "SocketServerAcceptEvent.h"

#include "InitialServerSocketReadEvent.h"
#include "MessageServer.h"
#include "SocketEventDispatcher.h"
#include "SocketReadEvent.h"
#include "SocketServer.h"
#include "SocketStream.h"

namespace OpenRTI {

SocketServerAcceptEvent::SocketServerAcceptEvent(SharedPtr<SocketServer> socket, SharedPtr<MessageServer> messageServer) :
  SocketReadEvent(true),
  _socket(socket),
  _messageServer(messageServer)
{
}

SocketServerAcceptEvent::~SocketServerAcceptEvent()
{
}

void
SocketServerAcceptEvent::read(SocketEventDispatcher& dispatcher)
{
  SharedPtr<SocketStream> s = _socket->accept();
  if (!s.valid())
    return;

  // Set up a connection accepting a new client connection
  dispatcher.insert(new InitialServerSocketReadEvent(s.get(), _messageServer));
}

SocketServer*
SocketServerAcceptEvent::getSocket() const
{
  return _socket.get();
}

} // namespace OpenRTI
