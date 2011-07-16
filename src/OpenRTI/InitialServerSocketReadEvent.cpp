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

#include "InitialServerSocketReadEvent.h"

#include "InitialSocketReadEvent.h"
#include "InitialServerSocketWriteEvent.h"
#include "MessageEncodingRegistry.h"
#include "SocketEventDispatcher.h"
#include "SocketStream.h"

namespace OpenRTI {

InitialServerSocketReadEvent::InitialServerSocketReadEvent(const SharedPtr<SocketStream>& socketStream,
                                                           const SharedPtr<AbstractServerNode>& serverNode) :
  InitialSocketReadEvent(socketStream),
  _serverNode(serverNode)
{
}

InitialServerSocketReadEvent::~InitialServerSocketReadEvent()
{
}

void
InitialServerSocketReadEvent::readPacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer)
{
  InitialSocketReadEvent::readPacket(dispatcher, networkBuffer);
  if (!networkBuffer.complete())
    return;

  // .. now we have the initial value map avaliable

  // This is a one time action ...
  dispatcher.erase(this);
  // Replace this message dispatcher with the one that handles the connection
  dispatcher.eraseSocket(this);

  // Ok, now it is the servers choice to decide which encoding to use

  // Helper to set up a server connect
  StringStringListMap responseValueMap;
  responseValueMap = MessageEncodingRegistry::instance().getBestServerEncoding(_valueMap, _serverNode->getServerOptions());

  StringStringListMap optionMap = _serverNode->getServerOptions()._optionMap;
  for (StringStringListMap::iterator i = optionMap.begin(); i != optionMap.end(); ++i) {
    if (responseValueMap.find(i->first) != responseValueMap.end())
      continue;
    responseValueMap.insert(*i);
  }

  // Ok, we have now set up all we need for the encoded communication.
  // But still we need to respond ...
  SharedPtr<InitialServerSocketWriteEvent> initialServerWriteEvent;
  initialServerWriteEvent = new InitialServerSocketWriteEvent(getSocket(), _serverNode, _valueMap);
  initialServerWriteEvent->setValueMap(responseValueMap);

  dispatcher.insert(initialServerWriteEvent.get());
}

} // namespace OpenRTI
