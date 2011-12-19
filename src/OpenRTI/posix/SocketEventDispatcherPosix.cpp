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
#include "ErrnoPosix.h"
#include "Exception.h"
#include "LogStream.h"
#include "SocketPrivateDataPosix.h"

namespace OpenRTI {

static int
nonblocking_pipe(int pipeFd[2])
{
  int ret = pipe(pipeFd);
  if (-1 == ret)
    return ret;

  for (unsigned i = 0; i < 2; ++i) {
    // This is nice to have, so just try and don't bail out
    int flags = fcntl(pipeFd[i], F_GETFD, 0);
    if (flags != -1)
      fcntl(pipeFd[i], F_SETFD, flags | FD_CLOEXEC);

    // Switch to nonblocking io, this is required
    flags = fcntl(pipeFd[i], F_GETFL, 0);
    if (flags == -1)
      return flags;
    ret = fcntl(pipeFd[i], F_SETFL, flags | O_NONBLOCK);
    if (ret == -1)
      return ret;
  }
  return 0;
}

struct OPENRTI_LOCAL SocketEventDispatcher::PrivateData {
  PrivateData() :
    _wakeupReadFd(-1),
    _wakeupWriteFd(-1)
  {
    int pipeFd[2] = {-1, -1};
    if (-1 == nonblocking_pipe(pipeFd)) {
      int errorNumber = errno;
      for (unsigned i = 0; i < 2; ++i) {
        if (pipeFd[i] == -1)
          continue;
        close(pipeFd[i]);
      }
      throw TransportError(errnoToUtf8(errorNumber));
    }
    _wakeupReadFd = pipeFd[0];
    _wakeupWriteFd = pipeFd[1];
  }
  ~PrivateData()
  {
    if (_wakeupReadFd != -1) {
      close(_wakeupReadFd);
      _wakeupReadFd = -1;
    }
    if (_wakeupWriteFd != -1) {
      close(_wakeupWriteFd);
      _wakeupWriteFd = -1;
    }
  }

  int exec(SocketEventDispatcher& dispatcher, const Clock& absclock)
  {
    int retv = 0;

    while (!dispatcher._done) {
      if (dispatcher.empty()) {
        dispatcher._done = true;
        retv = 0;
        break;
      }

      fd_set readfds;
      fd_set writefds;
      fd_set exceptfds;
      FD_ZERO(&readfds);
      FD_ZERO(&writefds);
      FD_ZERO(&exceptfds);

      Clock timeout = absclock;

      int nfds = -1;
      for (SocketEventList::const_iterator i = dispatcher._socketEventList.begin(); i != dispatcher._socketEventList.end(); ++i) {
        AbstractSocketEvent* socketEvent = i->get();
        timeout = std::min(timeout, socketEvent->getTimeout());
        Socket* abstractSocket = socketEvent->getSocket();
        if (!abstractSocket)
          continue;
        int fd = abstractSocket->_privateData->_fd;
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

      FD_SET(_wakeupReadFd, &readfds);
      nfds = std::max(nfds, _wakeupReadFd);

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
      if (count == -1) {
        int errorNumber = errno;
        if (errorNumber != EINTR && errorNumber != EAGAIN) {
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
      uint64_t now = ClockPosix::now();
      if (absclock.getNSec() <= now) {
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

      if (FD_ISSET(_wakeupReadFd, &readfds)) {
        char dummy[64];
        while (0 < ::read(_wakeupReadFd, dummy, sizeof(dummy)));
        if (!_wokenUp.compareAndExchange(1, 0))
          Log(Network, Warning) << "Having something to read from the wakeup pipe, but the flag is not set?" << std::endl;
        retv = 0;
        break;
      }
    }

    return retv;
  }

  void wakeUp()
  {
    // Check if we already have a wakeup pending
    if (!_wokenUp.compareAndExchange(0, 1))
      return;
    // No, the first one, write to the pipe
    char data = 1;
    for (;;) {
      ssize_t ret = ::write(_wakeupWriteFd, &data, sizeof(data));
      if (ret == 1)
        break;
      // We should not get EAGAIN here, since we only write the first time to wake up,
      // but be paranoid.
      if (ret == 0)
        continue;
      int errorNumber = errno;
      if (ret == -1 && (errorNumber == EAGAIN || errorNumber == EINTR))
        continue;
      throw TransportError(errnoToUtf8(errorNumber));
    }
  }

private:
  Atomic _wokenUp;
  int _wakeupReadFd;
  int _wakeupWriteFd;
};

SocketEventDispatcher::SocketEventDispatcher() :
  _privateData(new PrivateData),
  _done(false)
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
  _done = done;
}

bool
SocketEventDispatcher::getDone() const
{
  return _done;
}

void
SocketEventDispatcher::wakeUp()
{
  _privateData->wakeUp();
}

int
SocketEventDispatcher::exec(const Clock& absclock)
{
  return _privateData->exec(*this, absclock);
}

}
