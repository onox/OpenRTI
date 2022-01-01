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

#include "SocketServerAcceptEvent.h"

#include "InitialServerStreamProtocol.h"
#include "ProtocolSocketEvent.h"
#include "SocketEventDispatcher.h"
#include "SocketServer.h"
#include "SocketStream.h"

namespace OpenRTI {

SocketServerAcceptEvent::SocketServerAcceptEvent(const SharedPtr<SocketServer>& socketServer,
                                                 AbstractServer& abstractServer) :
  _socketServer(socketServer),
  _abstractServer(abstractServer)
{
}

SocketServerAcceptEvent::~SocketServerAcceptEvent()
{
}

void
SocketServerAcceptEvent::read(SocketEventDispatcher& dispatcher)
{
  SharedPtr<SocketStream> s = _socketServer->accept();
  if (!s.valid())
    return;

  /// IDEA: peek into the read data to see if this is an OpenRTI or a HTTP request

  SharedPtr<ProtocolSocketEvent> protocolSocketEvent = new ProtocolSocketEvent(s);
  protocolSocketEvent->setProtocolLayer(new InitialServerStreamProtocol(_abstractServer));
  dispatcher.insert(protocolSocketEvent);
}

bool
SocketServerAcceptEvent::getEnableRead() const
{
  return true;
}

void
SocketServerAcceptEvent::write(SocketEventDispatcher&)
{
}

bool
SocketServerAcceptEvent::getEnableWrite() const
{
  return false;
}

SocketServer*
SocketServerAcceptEvent::getSocket() const
{
  return _socketServer.get();
}

} // namespace OpenRTI
