/* -*-c++-*- OpenRTI - Copyright (C) 2009-2022 Mathias Froehlich
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

#include <poll.h>
#include <pthread.h>
#include <vector>
#include <map>
#include <cerrno>
#include <cstring>

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
      _fdVector.resize(0);
      _socketEventVector.resize(0);
      _timerSocketEventVector.resize(0);

      Clock timeout = absclock;

      struct pollfd pfd;
      std::memset(&pfd, 0, sizeof(pfd));
      for (SocketEventList::const_iterator i = dispatcher._socketEventList.begin(); i != dispatcher._socketEventList.end(); ++i) {
        AbstractSocketEvent* socketEvent = i->get();
        bool included = false;
        Socket* abstractSocket = socketEvent->getSocket();
        if (abstractSocket) {
          int fd = abstractSocket->_privateData->_fd;
          if (fd != -1) {
            pfd.fd = fd;
            pfd.events = 0;
            if (socketEvent->getEnableRead())
              pfd.events |= POLLRDNORM;
            if (socketEvent->getEnableWrite())
              pfd.events |= POLLWRNORM;

            if (pfd.events) {
              included = true;
              _fdVector.push_back(pfd);
              _socketEventVector.push_back(socketEvent);
            }
          }
        }

        if (socketEvent->getTimeout() != Clock::max()) {
          timeout = std::min(timeout, socketEvent->getTimeout());
          if (!included)
            _timerSocketEventVector.push_back(socketEvent);
        }
      }
      // The wakeup event is always put at the end and does *not* have a
      // corresponding _socketEventVector entry!
      pfd.fd = _wakeupReadFd;
      pfd.events = POLLRDNORM;
      _fdVector.push_back(pfd);

      int count;
      if (timeout < Clock::max()) {
        uint64_t now = ClockPosix::now();
        if (timeout.getNSec() < now) {
          count = 0;
        } else {
          count = ::poll(&_fdVector[0], _fdVector.size(), ClockPosix::toIntMSec(timeout.getNSec() - now));
        }
      } else {
        count = ::poll(&_fdVector[0], _fdVector.size(), -1);
      }
      if (count == -1) {
        int errorNumber = errno;
        if (errorNumber != EINTR && errorNumber != EAGAIN) {
          retv = -1;
          break;
        } else {
          count = 0;
        }
      }
      // Timeout
      uint64_t now = ClockPosix::now();
      if (absclock.getNSec() <= now) {
        retv = 0;
        break;
      }

      // We know the last one is from _fdVector is the wakup fd. Hence the _socketEventVector size is the one to walk.
      for (SocketEventVector::size_type i = 0; i < _socketEventVector.size(); ++i) {
        SharedPtr<AbstractSocketEvent> socketEvent;
        socketEvent.swap(_socketEventVector[i]);
        Socket* abstractSocket = socketEvent->getSocket();
        if (abstractSocket) {
          int fd = abstractSocket->_privateData->_fd;
          if (fd != -1) {
            OpenRTIAssert(fd == _fdVector[i].fd);
            short revents = _fdVector[i].revents;
            if (revents & POLLRDNORM)
              dispatcher.read(socketEvent);
            if (revents & POLLWRNORM)
              dispatcher.write(socketEvent);
          }
        }
        if (socketEvent->getTimeout().getNSec() <= now)
          dispatcher.timeout(socketEvent);
      }

      for (SocketEventVector::size_type i = 0; i < _timerSocketEventVector.size(); ++i) {
        SharedPtr<AbstractSocketEvent> socketEvent;
        socketEvent.swap(_timerSocketEventVector[i]);
        if (socketEvent->getTimeout().getNSec() <= now)
          dispatcher.timeout(socketEvent);
      }

      OpenRTIAssert(!_fdVector.empty());
      if (_fdVector.back().revents & POLLRDNORM) {
        char dummy[64];
        while (0 < ::read(_wakeupReadFd, dummy, sizeof(dummy)));
        if (!_wokenUp.compareAndExchange(1, 0, Atomic::MemoryOrderAcqRel))
          Log(Network, Warning) << "Having something to read from the wakeup pipe, but the flag is not set?" << std::endl;
        retv = 0;
        break;
      }
    }

    _fdVector.resize(0);
    _socketEventVector.resize(0);
    _timerSocketEventVector.resize(0);

    return retv;
  }

  void wakeUp()
  {
    // Check if we already have a wakeup pending
    if (!_wokenUp.compareAndExchange(0, 1, Atomic::MemoryOrderAcqRel))
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

  typedef std::vector<SharedPtr<AbstractSocketEvent> > SocketEventVector;
  // Contains the fds for use with poll
  std::vector<struct pollfd> _fdVector;
  // Must be kept consistent in size with _fdVector, for each entry in _fdVector, contains the
  // socket event belonging to the above fd.
  SocketEventVector _socketEventVector;
  // For times that can expire, store the AbstractSocetEvents that are not already stored in _socketEventVector.
  SocketEventVector _timerSocketEventVector;
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
