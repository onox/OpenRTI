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

#include "SocketEventDispatcher.h"

#include <vector>
#include <map>
#include <cerrno>

#include "AbstractSocketEvent.h"
#include "Clock.h"
#include "ClockWin32.h"
#include "ErrnoWin32.h"
#include "Exception.h"
#include "SocketPrivateDataWin32.h"

namespace OpenRTI {

struct SocketEventDispatcher::PrivateData {
  bool _done;

  PrivateData() :
    _done(false)
  {
  }

  int exec(SocketEventDispatcher& dispatcher, const Clock* absclock)
  {
    int retv = 0;

    while (!_done) {
      if (dispatcher.empty()) {
        _done = true;
        retv = 0;
        break;
      }

      fd_set readfds;
      fd_set writefds;
      fd_set exceptfds;
      FD_ZERO(&readfds);
      FD_ZERO(&writefds);
      FD_ZERO(&exceptfds);

      Clock timeout = Clock::final();
      if (absclock)
        timeout = *absclock;

      int nfds = -1;
      for (SocketEventList::const_iterator i = dispatcher._socketEventList.begin(); i != dispatcher._socketEventList.end(); ++i) {
        AbstractSocketEvent* socketEvent = i->get();
        if (socketEvent->getTimeout() < timeout)
          timeout = socketEvent->getTimeout();
        Socket* abstractSocket = socketEvent->getSocket();
        if (!abstractSocket)
           continue;
        SOCKET socket = abstractSocket->_privateData->_socket;
        if (socket == INVALID_SOCKET)
          continue;
        if (socket == SOCKET_ERROR)
          continue;
        if (socketEvent->getEnableRead()) {
          FD_SET(socket, &readfds);
          if (nfds < int(socket))
            nfds = int(socket);
        }
        if (socketEvent->getEnableWrite()) {
          FD_SET(socket, &writefds);
          if (nfds < int(socket))
            nfds = int(socket);
        }
      }

      int count;
      if (timeout < Clock::final()) {
        Clock now = Clock::now();
        if (timeout < now) {
          count = 0;
          FD_ZERO(&readfds);
          FD_ZERO(&writefds);
          FD_ZERO(&exceptfds);
        } else {
          struct timeval timeval = ClockWin32::toTimeval((*absclock - now).getNSec());
          count = ::select(nfds + 1, &readfds, &writefds, &exceptfds, &timeval);
        }
      } else {
        count = ::select(nfds + 1, &readfds, &writefds, &exceptfds, 0);
      }
      if (count == -1) {
        if (errno != EINTR) {
          retv = -1;
          break;
        } else {
          count = 0;
          FD_ZERO(&readfds);
          FD_ZERO(&writefds);
          FD_ZERO(&exceptfds);
        }
      }
      // Timeout
      Clock now = Clock::now();
      if (absclock && *absclock <= now) {
        retv = 0;
        break;
      }

      for (SocketEventList::const_iterator i = dispatcher._socketEventList.begin(); i != dispatcher._socketEventList.end();) {
        SharedPtr<AbstractSocketEvent> socketEvent = *i;
        ++i;
        Socket* abstractSocket = socketEvent->getSocket();
        if (abstractSocket) {
          SOCKET socket = abstractSocket->_privateData->_socket;
          if (socket != INVALID_SOCKET && socket != SOCKET_ERROR) {
            if (FD_ISSET(socket, &readfds))
              dispatcher.read(socketEvent);
            if (FD_ISSET(socket, &writefds))
              dispatcher.write(socketEvent);
          }
        }
        if (socketEvent->getTimeout() <= now)
          dispatcher.timeout(socketEvent);
      }
    }

    return retv;
  }
};

SocketEventDispatcher::SocketEventDispatcher() :
  _privateData(new PrivateData)
{
}

SocketEventDispatcher::~SocketEventDispatcher()
{
  delete _privateData;
  _privateData = 0;
}

void
SocketEventDispatcher::setDone(bool done)
{
  _privateData->_done = done;
}

bool
SocketEventDispatcher::getDone() const
{
  return _privateData->_done;
}

int
SocketEventDispatcher::exec()
{
  return _privateData->exec(*this, 0);
}

int
SocketEventDispatcher::exec(const Clock& absclock)
{
  return _privateData->exec(*this, &absclock);
}

}
