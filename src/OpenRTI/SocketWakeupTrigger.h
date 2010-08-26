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

#ifndef OpenRTI_SocketWakeupTrigger_h
#define OpenRTI_SocketWakeupTrigger_h

#include "SharedPtr.h"
#include "Socket.h"
#include "SocketWakeupEvent.h"
#include "Types.h"

namespace OpenRTI {

class OPENRTI_LOCAL SocketWakeupTrigger : public Socket {
public:
  SocketWakeupTrigger();

  // Get a wakeup event that can read/is signaled when this trigger is called
  // There can only be one sich SocketWakeupEvent connected to a trigger
  SocketWakeupEvent* connect();

  // Signal the event one time.
  // returns -1 on a closed wakeup event
  // returns 1 on success and
  // 0 if the event signaling queue is full.
  ssize_t trigger();

protected:
  virtual ~SocketWakeupTrigger();

  /// The wakeup event socket that is returned by a connect call.
  SharedPtr<SocketWakeupEvent> _socketWakeupEvent;
};

} // namespace OpenRTI

#endif
