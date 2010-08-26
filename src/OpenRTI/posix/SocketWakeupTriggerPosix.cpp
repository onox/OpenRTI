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

#include "SocketWakeupTrigger.h"

#include "Exception.h"
#include "ErrnoPosix.h"
#include "SocketPrivateDataPosix.h"

namespace OpenRTI {

SocketWakeupTrigger::SocketWakeupTrigger() :
  Socket(new PrivateData(-1))
{
}

SocketWakeupEvent*
SocketWakeupTrigger::connect()
{
  if (_socketWakeupEvent.valid())
    return _socketWakeupEvent.get();

  if (_privateData->_fd != -1)
    throw TransportError(L"SocketEventTrigger already connected but _socketWakeupEvent is zero!");

  int pipeFd[2];
  int ret = pipe(pipeFd);
  if (ret == -1) {
    int errorNumber = errno;
    throw TransportError(errnoToUcs(errorNumber));
  }
  _privateData->_fd = pipeFd[1];
  
  _socketWakeupEvent = new SocketWakeupEvent(new PrivateData(pipeFd[0]));
  return _socketWakeupEvent.get();
}

ssize_t
SocketWakeupTrigger::trigger()
{
  char data = 1;
  ssize_t ret = ::write(_privateData->_fd, &data, sizeof(data));
  if (ret == -1) {
    int errorNumber = errno;
    if (errorNumber == EPIPE)
      return -1;
    if (errorNumber == EAGAIN || errorNumber == EINTR)
      return 0;
    throw TransportError(errnoToUcs(errorNumber));
  }
  return 1;
}

SocketWakeupTrigger::~SocketWakeupTrigger()
{
}

} // namespace OpenRTI
