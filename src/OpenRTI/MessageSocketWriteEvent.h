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

#ifndef OpenRTI_MessageSocketWriteEvent_h
#define OpenRTI_MessageSocketWriteEvent_h

#include "AbstractMessageEncoder.h"
#include "AbstractMessageSender.h"
#include "Message.h"
#include "MessageList.h"
#include "StreamSocketWriteEvent.h"
#include "SocketStream.h"
#include "SocketEventDispatcher.h"

namespace OpenRTI {

class OPENRTI_LOCAL MessageSocketWriteEvent : public StreamSocketWriteEvent {
public:
  MessageSocketWriteEvent(const SharedPtr<SocketStream>& socket, const SharedPtr<AbstractMessageEncoder>& encoder);
  virtual ~MessageSocketWriteEvent();

  virtual void writePacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer);
  virtual void written(SocketEventDispatcher& dispatcher);
  virtual bool getMoreToSend() const;

  AbstractMessageSender* getMessageSender();
  void sendToSocket(const SharedPtr<AbstractMessage>& message);
  void shutdownSocket();

private:
  class MessageSender;

  SharedPtr<AbstractMessageEncoder> _encoder;
  MessageList _messageList;
  bool _shutdownSocket;
};

} // namespace OpenRTI

#endif
