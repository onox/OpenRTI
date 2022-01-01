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
  if (!socketAddress.valid())
    throw TransportError("Trying to bind server socket to an invalid address!");

  const struct sockaddr* sockaddr = SocketAddress::PrivateData::sockaddr(socketAddress.constData());
  int fd = socket(sockaddr->sa_family, SOCK_STREAM, 0);
  if (fd == -1)
    throw TransportError(errnoToUtf8(errno));

  // This is nice to have, so just try and don't bail out
  int flags = fcntl(fd, F_GETFD, 0);
  if (flags != -1)
    fcntl(fd, F_SETFD, flags | FD_CLOEXEC);

  // This is also nice to have, but not essential
  unsigned reuseaddr = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

  // Looks like we need this to get ipv6 up correctly,
  // don't care if there is an error, just try.
#ifdef IPV6_V6ONLY
  if (sockaddr->sa_family == AF_INET6) {
    unsigned yes = 1;
    setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes));
  }
#endif

  socklen_t addrlen = SocketAddress::PrivateData::addrlen(socketAddress.constData());
  int ret = ::bind(fd, sockaddr, addrlen);
  if (ret == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUtf8(errorNumber));
  }

  _privateData->_fd = fd;
}

void
SocketServerTCP::listen(int backlog)
{
  int ret = ::listen(_privateData->_fd, backlog);
  if (ret == -1) {
    int errorNumber = errno;
    throw TransportError(errnoToUtf8(errorNumber));
  }
}

SocketTCP*
SocketServerTCP::accept()
{
  int fd = ::accept(_privateData->_fd, 0, 0);
  if (fd == -1) {
    int errorNumber = errno;
    throw TransportError(errnoToUtf8(errorNumber));
  }

  // Need to have non blocking IO
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUtf8(errorNumber));
  }
  int ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (ret == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUtf8(errorNumber));
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
    throw TransportError(errnoToUtf8(errorNumber));
  }

  // The error codes are set anyway, disable sending SIGPIPE, nice to have
#ifdef SO_NOSIGPIPE
  int nosigpipe = 1;
  setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(nosigpipe));
#endif

  return new SocketTCP(new PrivateData(fd));
}

SocketServerTCP::~SocketServerTCP()
{
  close();
}

} // namespace OpenRTI
