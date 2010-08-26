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

#ifndef OpenRTI_SocketServerAcceptEvent_h
#define OpenRTI_SocketServerAcceptEvent_h

#include "MessageServer.h"
#include "SocketReadEvent.h"
#include "SocketServer.h"

namespace OpenRTI {

class OPENRTI_LOCAL SocketServerAcceptEvent : public SocketReadEvent {
public:
  SocketServerAcceptEvent(SharedPtr<SocketServer> socket, SharedPtr<MessageServer> messageServer);
  virtual ~SocketServerAcceptEvent();

  virtual void read(SocketEventDispatcher& dispatcher);
  virtual SocketServer* getSocket() const;

private:
  SharedPtr<SocketServer> _socket;
  SharedPtr<MessageServer> _messageServer;
};

} // namespace OpenRTI

#endif
