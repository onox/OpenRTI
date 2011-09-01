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
#include "ErrnoWin32.h"
#include "Exception.h"
#include "SocketPrivateDataWin32.h"

namespace OpenRTI {

struct SocketEventDispatcher::PrivateData {
  struct SocketEventSet {
    SharedPtr<AbstractSocketEvent> _socketEvent;
  };

  /// Map from the socket file descriptor to a SocketEvent
  typedef std::map<SOCKET,SocketEventSet> SocketEventMap;
  SocketEventMap _socketEventMap;

  bool _done;

  PrivateData() :
    _done(false)
  {
  }

  void insert(const SharedPtr<AbstractSocketEvent>& socketEvent)
  {
    if (!socketEvent.valid())
      return;
    if (!socketEvent->getSocket())
      return;
    SOCKET socket = socketEvent->getSocket()->_privateData->_socket;
    _socketEventMap[socket]._socketEvent = socketEvent;
  }

  void erase(const SharedPtr<AbstractSocketEvent>& socketEvent)
  {
    if (!socketEvent.valid())
      return;
    if (!socketEvent->getSocket())
      return;
    SOCKET socket = socketEvent->getSocket()->_privateData->_socket;
    SocketEventMap::iterator i = _socketEventMap.find(socket);
    if (i == _socketEventMap.end())
      return;

    _socketEventMap.erase(i);
  }

  static struct timeval toTimeval(const uint64_t& nsec)
  {
    struct timeval tv;
    uint64_t usec = (nsec + 500) / 1000;
    uint64_t frac = usec % 1000000;
    tv.tv_usec = (long)frac;
    tv.tv_sec = (long)((usec - frac) / 1000000);
    return tv;
  }

  int exec(SocketEventDispatcher& dispatcher, const Clock* absclock)
  {
    int retv = 0;

    while (!_done) {
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
      for (SocketEventMap::const_iterator i = _socketEventMap.begin(); i != _socketEventMap.end(); ++i) {
        AbstractSocketEvent* socketEvent = i->second._socketEvent.get();
        if (socketEvent->getTimeout() < timeout)
          timeout = socketEvent->getTimeout();
        if (!socketEvent->getSocket())
          continue;
        SOCKET socket = socketEvent->getSocket()->_privateData->_socket;
        OpenRTIAssert(i->first == socket);
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
      // HMM???
      if (nfds == -1) {
        _done = true;
        retv = 0;
        break;
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
          struct timeval timeval = toTimeval((*absclock - now).getNSec());
          count = ::select(nfds + 1, &readfds, &writefds, &exceptfds, &timeval);
        }
      } else {
        count = ::select(nfds + 1, &readfds, &writefds, &exceptfds, 0);
      }
      if (count == -1 && errno != EINTR) {
        retv = -1;
        break;
      }
      // Timeout
      Clock now = Clock::now();
      if (absclock && *absclock <= now) {
        retv = 0;
        break;
      }

      for (SocketEventMap::const_iterator i = _socketEventMap.begin(); i != _socketEventMap.end();) {
        SharedPtr<AbstractSocketEvent> socketEvent = i->second._socketEvent;
        ++i;
        Socket* abstractSocket = socketEvent->getSocket();
        if (abstractSocket) {
          SOCKET socket = abstractSocket->_privateData->_socket;
          if (FD_ISSET(socket, &readfds))
            dispatcher.read(socketEvent);
          if (FD_ISSET(socket, &writefds))
            dispatcher.write(socketEvent);
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

void
SocketEventDispatcher::insert(const SharedPtr<AbstractSocketEvent>& socketEvent)
{
  _privateData->insert(socketEvent);
}

void
SocketEventDispatcher::erase(const SharedPtr<AbstractSocketEvent>& socketEvent)
{
  _privateData->erase(socketEvent);
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
