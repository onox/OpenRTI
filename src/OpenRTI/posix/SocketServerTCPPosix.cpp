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

#include "SocketServerTCP.h"

#include "ErrnoPosix.h"
#include "Exception.h"
#include "SocketAddressPrivateDataPosix.h"
#include "SocketPrivateDataPosix.h"

namespace OpenRTI {

SocketServerTCP::SocketServerTCP() :
  SocketServer(new PrivateData(-1))
{
}

void
SocketServerTCP::bind(const SocketAddress& socketAddress)
{
  int fd = socket(socketAddress._privateData->_addr->sa_family, SOCK_STREAM, 0);
  if (fd == -1)
    throw TransportError(errnoToUcs(errno));

  // This is nice to have, so just try and don't bail out
  int flags = fcntl(fd, F_GETFD, 0);
  if (flags != -1)
    fcntl(fd, F_SETFD, flags | FD_CLOEXEC);

  // This is also nice to have, but not essential
  unsigned reuseaddr = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

  int ret = ::bind(fd, socketAddress._privateData->_addr, socketAddress._privateData->_addrlen);
  if (ret == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  _privateData->_fd = fd;
}

void
SocketServerTCP::listen(int backlog)
{
  int ret = ::listen(_privateData->_fd, backlog);
  if (ret == -1) {
    int errorNumber = errno;
    throw TransportError(errnoToUcs(errorNumber));
  }
}

SocketTCP*
SocketServerTCP::accept()
{
  int fd = ::accept(_privateData->_fd, 0, 0);
  if (fd == -1) {
    int errorNumber = errno;
    throw TransportError(errnoToUcs(errorNumber));
  }

  // Need to have non blocking IO
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }
  int ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (ret == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

#ifdef DEBUG_LATENCY
  // When debugging latencies, just keep the messages longer in the send
  // queue until they are flushed by a timeout.
  int delay = 0;
#else
  int delay = 1;
#endif
  ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &delay, sizeof(delay));
  if (ret == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  return new SocketTCP(new PrivateData(fd));
}

SocketServerTCP::~SocketServerTCP()
{
  close();
}

} // namespace OpenRTI
