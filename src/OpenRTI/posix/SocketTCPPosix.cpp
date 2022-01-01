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

#include "SocketTCP.h"

#include "ErrnoPosix.h"
#include "Exception.h"
#include "SocketPrivateDataPosix.h"
#include "SocketAddressPrivateDataPosix.h"

namespace OpenRTI {

SocketTCP::SocketTCP() :
  SocketStream(new PrivateData(-1))
{
}

void
SocketTCP::connect(const SocketAddress& socketAddress)
{
  if (0 <= _privateData->_fd)
    throw TransportError("Trying to connect an already open SocketTCP!");

  if (!socketAddress.valid())
    throw TransportError("Trying to connect to an invalid address!");

  const struct sockaddr* sockaddr = SocketAddress::PrivateData::sockaddr(socketAddress.constData());
  int fd = ::socket(sockaddr->sa_family, SOCK_STREAM, 0);
  if (fd == -1)
    throw TransportError(errnoToUtf8(errno));

  // This is nice to have, so just try and don't bail out
  int flags = fcntl(fd, F_GETFD, 0);
  if (flags != -1)
    fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
  // The error codes are set anyway, disable sending SIGPIPE
#ifdef SO_NOSIGPIPE
  int nosigpipe = 1;
  setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(nosigpipe));
#endif

#ifdef DEBUG_LATENCY
  // When debugging latencies, just keep the messages longer in the send
  // queue until they are flushed by a timeout.
  int nodelay = 0;
#else
  int nodelay = 1;
#endif
  int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
  if (ret == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUtf8(errorNumber));
  }

  // Switch to nonblocking io
  flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUtf8(errorNumber));
  }
  ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (ret == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUtf8(errorNumber));
  }

  socklen_t addrlen = SocketAddress::PrivateData::addrlen(socketAddress.constData());
  ret = ::connect(fd, sockaddr, addrlen);
  if (ret == -1) {
    int errorNumber = errno;
    if (errorNumber != EINPROGRESS && errorNumber != EWOULDBLOCK) {
      ::close(fd);
      throw TransportError(errnoToUtf8(errorNumber));
    }
  }

  _privateData->_fd = fd;
}

void
SocketTCP::cork(bool enable)
{
#if defined TCP_CORK
  int cork;
  if (enable)
    cork = 1;
  else
    cork = 0;
  // On succes, we are done here
  setsockopt(_privateData->_fd, IPPROTO_TCP, TCP_CORK, &cork, sizeof(cork));
#else
  int nodelay;
#ifdef DEBUG_LATENCY
  nodelay = 1;
#else
  if (enable)
    nodelay = 0;
  else
    nodelay = 1;
#endif
  setsockopt(_privateData->_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
#endif
}

void
SocketTCP::shutdown()
{
  int fd = _privateData->_fd;
  if (fd == -1)
    return;
  ::shutdown(fd, SHUT_WR);
}

SocketTCP::SocketTCP(PrivateData* privateData) :
  SocketStream(privateData)
{
}

SocketTCP::~SocketTCP()
{
  if (_privateData->_fd == -1)
    return;
  ::close(_privateData->_fd);
  _privateData->_fd = -1;
}

} // namespace OpenRTI
