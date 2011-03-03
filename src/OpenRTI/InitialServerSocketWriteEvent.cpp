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

#include "InitialServerSocketWriteEvent.h"

#include "InitialSocketWriteEvent.h"
#include "MessageEncodingRegistry.h"
#include "MessageSocketReadEvent.h"
#include "MessageSocketWriteEvent.h"
#include "SocketEventDispatcher.h"
#include "SocketStream.h"
#include "ZLibCallbacks.h"

namespace OpenRTI {

// Just a chain writing socket event to push out the initial connect answer and
// Than replace that one with a next read event
InitialServerSocketWriteEvent::InitialServerSocketWriteEvent(const SharedPtr<SocketStream>& socketStream,
                                                             const SharedPtr<MessageServer>& messageServer,
                                                             const StringStringListMap& clientValueMap) :
  InitialSocketWriteEvent(socketStream),
  _messageServer(messageServer),
  _clientValueMap(clientValueMap)
{
}

InitialServerSocketWriteEvent::~InitialServerSocketWriteEvent()
{
}

void
InitialServerSocketWriteEvent::written(SocketEventDispatcher& dispatcher)
{
  // this is a one time event
  setEnable(false);
  dispatcher.erase(this);

  // Try to get an encoder pair.
  // That fails with and exception, which just makes the connect attempt abort
  MessageEncoderPair encoderPair;
  encoderPair = MessageEncodingRegistry::instance().getEncodingPair(_valueMap);

  // Set up a connection to the federation server of the given name

  // The socket side message encoder and output message queue
  SharedPtr<MessageSocketWriteEvent> writeMessageSocketEvent;
  writeMessageSocketEvent = new MessageSocketWriteEvent(getSocket(), encoderPair.first.get());
  // And finally register that one in the dispatcher.
  // Note that this RTI dispatcher level just has currently one counterpart on the single
  // other side of the socket. So this one does not talk to any other socket

  /// The connection that sends messages back to the child
  SharedPtr<AbstractMessageSender> toClientSender = writeMessageSocketEvent->getMessageSender();

  /// returns a sender wher incomming messages should be sent to
  SharedPtr<AbstractMessageSender> toServerSender = _messageServer->insertConnect(toClientSender, _clientValueMap);
  if (!toServerSender.valid()) {
    dispatcher.eraseSocket(this);
    Log(MessageCoding, Warning) << "Could not get server connect handle: Dropping connection!" << std::endl;
    return;
  }

  // The socket side message parser and dispatcher that fires the above on completely
  // received messages
  SharedPtr<MessageSocketReadEvent> readMessageSocketEvent;
  readMessageSocketEvent = new MessageSocketReadEvent(getSocket(), toServerSender.get(), encoderPair.second.get());

  if (MessageEncodingRegistry::getUseCompression(_valueMap)) {
#ifdef OPENRTI_HAVE_ZLIB
    readMessageSocketEvent->setReceiveCallback(new ZLibReceiveCallback);
    writeMessageSocketEvent->setSendCallback(new ZLibSendCallback);
#else
    dispatcher.eraseSocket(this);
    Log(MessageCoding, Warning) << "Server negotiated zlib compression, but does not support that: Dropping connection!" << std::endl;
    return;
#endif
  }

  dispatcher.insert(writeMessageSocketEvent.get());
  dispatcher.insert(readMessageSocketEvent.get());
}

} // namespace OpenRTI
