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

#if defined __hpux
#include <sys/time.h>
#else
#include <sys/select.h>
#endif
#include <pthread.h>
#include <vector>
#include <map>
#include <cerrno>

#include "AbstractSocketEvent.h"
#include "ClockPosix.h"
#include "Exception.h"
#include "SocketPrivateDataPosix.h"

namespace OpenRTI {

/// FIXME from the linux man page!!!!
/// Rethink!!!!
// Under Linux, select() may report a socket file descriptor as "ready
// for reading", while nevertheless a subsequent read blocks. This
// could for example happen when data has arrived but upon examination
// has wrong checksum and is discarded. There may be other
// circumstances in which a file descriptor is spuriously reported as
// ready. Thus it may be safer to use O_NONBLOCK on sockets that
// should not block.

/// eventfd looks nice too

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
        timeout = std::min(timeout, socketEvent->getTimeout());
        if (!socketEvent->getSocket())
          continue;
        int fd = socketEvent->getSocket()->_privateData->_fd;
        if (fd == -1)
          continue;
        if (socketEvent->getEnableRead()) {
          FD_SET(fd, &readfds);
          nfds = std::max(nfds, fd);
        }
        if (socketEvent->getEnableWrite()) {
          FD_SET(fd, &writefds);
          nfds = std::max(nfds, fd);
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
        uint64_t now = ClockPosix::now();
        if (timeout.getNSec() < now) {
          count = 0;
          FD_ZERO(&readfds);
          FD_ZERO(&writefds);
          FD_ZERO(&exceptfds);
        } else {
          struct timeval timeval = ClockPosix::toTimeval(timeout.getNSec() - now);
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
      uint64_t now = ClockPosix::now();
      if (absclock && absclock->getNSec() <= now) {
        retv = 0;
        break;
      }

      for (SocketEventList::const_iterator i = dispatcher._socketEventList.begin(); i != dispatcher._socketEventList.end();) {
        SharedPtr<AbstractSocketEvent> socketEvent = *i;
        ++i;
        Socket* abstractSocket = socketEvent->getSocket();
        if (abstractSocket) {
          int fd = abstractSocket->_privateData->_fd;
          if (fd != -1) {
            if (FD_ISSET(fd, &readfds))
              dispatcher.read(socketEvent);
            if (FD_ISSET(fd, &writefds))
              dispatcher.write(socketEvent);
          }
        }
        if (socketEvent->getTimeout().getNSec() <= now)
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
