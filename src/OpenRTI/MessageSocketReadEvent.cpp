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

#include "MessageSocketReadEvent.h"

#include "AbstractMessageSender.h"
#include "Message.h"
#include "SocketStream.h"
#include "StreamSocketReadEvent.h"

namespace OpenRTI {

MessageSocketReadEvent::MessageSocketReadEvent(const SharedPtr<SocketStream>& socket,
                                               const SharedPtr<AbstractMessageSender>& messageSender,
                                               const SharedPtr<AbstractMessageDecoder>& decoder) :
  StreamSocketReadEvent(socket),
  _messageSender(messageSender),
  _decoder(decoder)
{
}

MessageSocketReadEvent::~MessageSocketReadEvent()
{
}

void
MessageSocketReadEvent::readPacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer)
{
  SharedPtr<AbstractMessage> message = _decoder->readMessage(networkBuffer);
  if (!message.valid())
    return;
  _messageSender->send(message);
}

void
MessageSocketReadEvent::readError(const std::wstring& reason)
{
  SharedPtr<ConnectionLostMessage> message = new ConnectionLostMessage;
  message->setFaultDescription(reason);
  _messageSender->send(message);
}

} // namespace OpenRTI
