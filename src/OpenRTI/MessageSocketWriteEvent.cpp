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

#include "MessageSocketWriteEvent.h"

#include "AbstractMessageSender.h"

namespace OpenRTI {

class OPENRTI_LOCAL MessageSocketWriteEvent::MessageSender : public AbstractMessageSender {
public:
  MessageSender(const SharedPtr<MessageSocketWriteEvent>& socketEvent) :
    _socketEvent(socketEvent)
  { }
  virtual ~MessageSender()
  { }

  virtual void send(const SharedPtr<AbstractMessage>& message)
  {
    _socketEvent->sendToSocket(message);
  }
  virtual void close()
  {
    _socketEvent->shutdownSocket();
  }

private:
  SharedPtr<MessageSocketWriteEvent> _socketEvent;
};

MessageSocketWriteEvent::MessageSocketWriteEvent(const SharedPtr<SocketStream>& socket, const SharedPtr<AbstractMessageEncoder>& encoder) :
  StreamSocketWriteEvent(socket),
  _encoder(encoder),
  _shutdownSocket(false),
  _enable(false)
{
}

MessageSocketWriteEvent::~MessageSocketWriteEvent()
{
}

bool
MessageSocketWriteEvent::getEnable() const
{
  return _enable;
}

void
MessageSocketWriteEvent::writePacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer)
{
  OpenRTIAssert(!_messageList.empty());
  // Process next message
  _encoder->encodeMessage(networkBuffer, *_messageList.pop_front());
}

void
MessageSocketWriteEvent::written(SocketEventDispatcher& dispatcher)
{
  if (!_messageList.empty())
    return;
  // disable writing for now when the next message arrives we get woken up again
  _enable = false;
  // May be shutdown???
  if (_shutdownSocket)
    getSocket()->shutdown();
}

bool
MessageSocketWriteEvent::getMoreToSend() const
{
  return !_messageList.empty();
}

AbstractMessageSender*
MessageSocketWriteEvent::getMessageSender()
{
  return new MessageSender(this);
}

void
MessageSocketWriteEvent::sendToSocket(const SharedPtr<AbstractMessage>& message)
{
  if (_shutdownSocket || !getSocket()->isOpen())
    throw RTIinternalError("Trying to send message to a closed MessageSender");
  _messageList.push_back(message);
  _enable = true;
}

void
MessageSocketWriteEvent::shutdownSocket()
{
  _shutdownSocket = true;
}

} // namespace OpenRTI
