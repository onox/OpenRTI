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

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "SocketEventDispatcher.h"

#include <vector>
#include <map>
#include <cerrno>

#include "AbstractSocketEvent.h"
#include "Clock.h"
#include "ClockWin32.h"
#include "ErrnoWin32.h"
#include "Exception.h"
#include "LogStream.h"
#include "SocketPrivateDataWin32.h"

namespace OpenRTI {

static void
nonblocking_pipe(SOCKET pipeFd[2])
{
  // Build up an ipv4 socket, and connect immediately.
  SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener == INVALID_SOCKET) {
    int errorNumber = WSAGetLastError();
    throw TransportError(errnoToUtf8(errorNumber));
  }

  struct sockaddr_in addr;
  int addrlen = sizeof(addr);
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(0x7f000001);
  addr.sin_port = 0;

  int ret = bind(listener, (const struct sockaddr*)&addr, sizeof(addr));
  if (ret == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    closesocket(listener);
    throw TransportError(errnoToUtf8(errorNumber));
  }
  ret = getsockname(listener, (struct sockaddr*)&addr, &addrlen);
  if (ret == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    closesocket(listener);
    throw TransportError(errnoToUtf8(errorNumber));
  }

  // Note that this 'listen for exactly 1 connect' is the key to prevent a
  // hijacked socket. It will just fail in this case
  if (listen(listener, 1) == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    closesocket(listener);
    throw TransportError(errnoToUtf8(errorNumber));
  }
  pipeFd[0] = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
  if (pipeFd[0] == INVALID_SOCKET) {
    int errorNumber = WSAGetLastError();
    closesocket(listener);
    throw TransportError(errnoToUtf8(errorNumber));
  }
  if (::connect(pipeFd[0], (const struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    closesocket(pipeFd[0]);
    closesocket(listener);
    throw TransportError(errnoToUtf8(errorNumber));
  }
  pipeFd[1] = accept(listener, NULL, NULL);
  if (pipeFd[1] == INVALID_SOCKET) {
    int errorNumber = WSAGetLastError();
    closesocket(pipeFd[0]);
    closesocket(listener);
    throw TransportError(errnoToUtf8(errorNumber));
  }
  closesocket(listener);

  // Switch to nonblocking io
  u_long mode = 1;
  ret = ::ioctlsocket(pipeFd[0], FIONBIO, &mode);
  if (ret == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    ::closesocket(pipeFd[0]);
    ::closesocket(pipeFd[1]);
    throw TransportError(errnoToUtf8(errorNumber));
  }

  ret = ::ioctlsocket(pipeFd[1], FIONBIO, &mode);
  if (ret == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    ::closesocket(pipeFd[0]);
    ::closesocket(pipeFd[1]);
    throw TransportError(errnoToUtf8(errorNumber));
  }
}

struct SocketEventDispatcher::PrivateData {
  PrivateData() :
    _wakeupReadSocket(INVALID_SOCKET),
    _wakeupWriteSocket(INVALID_SOCKET)
  {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
      throw TransportError("Could not initialize windows sockets!");
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
      WSACleanup();
      throw TransportError("Could not initialize windows sockets 2.2!");
    }

    SOCKET pipeSocket[2] = {INVALID_SOCKET, INVALID_SOCKET};
    nonblocking_pipe(pipeSocket);
    _wakeupReadSocket = pipeSocket[0];
    _wakeupWriteSocket = pipeSocket[1];
  }
  ~PrivateData()
  {
    if (_wakeupReadSocket != INVALID_SOCKET) {
      closesocket(_wakeupReadSocket);
      _wakeupReadSocket = INVALID_SOCKET;
    }
    if (_wakeupWriteSocket != INVALID_SOCKET) {
      closesocket(_wakeupWriteSocket);
      _wakeupWriteSocket = INVALID_SOCKET;
    }
    WSACleanup();
  }

  int exec(SocketEventDispatcher& dispatcher, const Clock& absclock)
  {
    int retv = 0;

    while (!dispatcher._done) {
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
          // In case of a failed connect we get the exception
          FD_SET(socket, &exceptfds);
          if (nfds < int(socket))
            nfds = int(socket);
        }
      }

      FD_SET(_wakeupReadSocket, &readfds);
      if (nfds < int(_wakeupReadSocket))
        nfds = int(_wakeupReadSocket);

      int count;
      if (timeout < Clock::max()) {
        Clock now = Clock::now();
        if (timeout < now) {
          count = 0;
          FD_ZERO(&readfds);
          FD_ZERO(&writefds);
          FD_ZERO(&exceptfds);
        } else {
          struct timeval timeval = ClockWin32::toTimeval((timeout - now).getNSec());
          count = ::select(nfds + 1, &readfds, &writefds, &exceptfds, &timeval);
        }
      } else {
        count = ::select(nfds + 1, &readfds, &writefds, &exceptfds, 0);
      }
      if (count == -1) {
        int errorNumber = WSAGetLastError();
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
      Clock now = Clock::now();
      if (absclock <= now) {
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
            if (FD_ISSET(socket, &writefds) || FD_ISSET(socket, &exceptfds))
              dispatcher.write(socketEvent);
          }
        }
        if (socketEvent->getTimeout() <= now)
          dispatcher.timeout(socketEvent);
      }
      if (FD_ISSET(_wakeupReadSocket, &readfds)) {
        char dummy[64];
        while (0 < ::recv(_wakeupReadSocket, dummy, sizeof(dummy), 0));
        if (!_wokenUp.compareAndExchange(1, 0, Atomic::MemoryOrderAcqRel))
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
    if (!_wokenUp.compareAndExchange(0, 1, Atomic::MemoryOrderAcqRel))
      return;
    // No, the first one, write to the pipe
    char data = 1;
    for (;;) {
      ssize_t ret = ::send(_wakeupWriteSocket, &data, sizeof(data), 0);
      if (ret == 1)
        break;
      // We should not get EAGAIN here, since we only write the first time to wake up,
      // but be paranoid.
      if (ret == 0)
        continue;
      int errorNumber = WSAGetLastError();
      if (ret == -1 && (errorNumber == EAGAIN || errorNumber == EINTR))
        continue;
      throw TransportError(errnoToUtf8(errorNumber));
    }
  }

private:
  Atomic _wokenUp;
  SOCKET _wakeupReadSocket;
  SOCKET _wakeupWriteSocket;
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
