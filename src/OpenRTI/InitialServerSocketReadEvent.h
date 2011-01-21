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

#ifndef OpenRTI_InitialServerSocketReadEvent_h
#define OpenRTI_InitialServerSocketReadEvent_h

#include "InitialSocketReadEvent.h"
#include "MessageServer.h"
#include "SocketStream.h"

namespace OpenRTI {

// Have a common class peeking into a header and reading the whole packet then
class OPENRTI_LOCAL InitialServerSocketReadEvent : public InitialSocketReadEvent {
public:
  InitialServerSocketReadEvent(const SharedPtr<SocketStream>& socketStream,
                               const SharedPtr<MessageServer>& messageServer);
  virtual ~InitialServerSocketReadEvent();

  // When the whole packet has arrived, this is called with the whole data
  virtual void readPacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer);

private:
  SharedPtr<MessageServer> _messageServer;
};

} // namespace OpenRTI

#endif
