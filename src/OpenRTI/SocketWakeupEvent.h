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

#ifndef OpenRTI_SocketWakeupEvent_h
#define OpenRTI_SocketWakeupEvent_h

#include "Socket.h"
#include "Types.h"

namespace OpenRTI {

class SocketWakeupTrigger;

class SocketWakeupEvent : public Socket {
public:
  // FIXME: unify win32/posix behaviour. win32 just tells if there is something, posix currently how many ...
  // Returns > 0 if there are events.
  // Returns 0 if there is nothing pending
  // Returns -1 on EOF
  // Throws like mad if there is a real error
  ssize_t read();

private:
  SocketWakeupEvent(PrivateData* privateData);
  virtual ~SocketWakeupEvent();

  friend class SocketWakeupTrigger;
};

} // namespace OpenRTI

#endif
