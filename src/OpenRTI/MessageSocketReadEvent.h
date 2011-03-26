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

#ifndef OpenRTI_MessageSocketReadEvent_h
#define OpenRTI_MessageSocketReadEvent_h

#include "AbstractMessageDecoder.h"
#include "AbstractMessageSender.h"
#include "SocketStream.h"
#include "StreamSocketReadEvent.h"

namespace OpenRTI {

class OPENRTI_LOCAL MessageSocketReadEvent : public StreamSocketReadEvent {
public:
  MessageSocketReadEvent(const SharedPtr<SocketStream>& socket, const SharedPtr<AbstractMessageSender>& messageSender,
                         const SharedPtr<AbstractMessageDecoder>& decoder);
  virtual ~MessageSocketReadEvent();

  virtual void readPacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer);
  virtual void readError(const std::string& reason);

private:
  // Messages that are received completely are sent down there
  SharedPtr<AbstractMessageSender> _messageSender;

  // This is used to decode the messages
  SharedPtr<AbstractMessageDecoder> _decoder;
};

} // namespace OpenRTI

#endif
