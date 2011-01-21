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

#ifndef OpenRTI_InitialClientSocketReadEvent_h
#define OpenRTI_InitialClientSocketReadEvent_h

#include "InitialSocketReadEvent.h"
#include "SocketStream.h"

namespace OpenRTI {

class OPENRTI_LOCAL InitialClientSocketReadEvent : public InitialSocketReadEvent {
public:
  InitialClientSocketReadEvent(const SharedPtr<SocketStream>& socketStream);
  virtual ~InitialClientSocketReadEvent();

  virtual void readPacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer);
};

} // namespace OpenRTI

#endif
