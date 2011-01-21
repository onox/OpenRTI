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
    throw TransportError(L"Trying to connect an already open SocketTCP!");

  if (!socketAddress.valid())
    throw TransportError(L"Trying to connect an invalid address!");

  int fd = ::socket(socketAddress._privateData->_addr->sa_family, SOCK_STREAM, 0);
  if (fd == -1)
    throw TransportError(errnoToUcs(errno));

  // This is nice to have, so just try and don't bail out
  int flags = fcntl(fd, F_GETFD, 0);
  if (flags != -1)
    fcntl(fd, F_SETFD, flags | FD_CLOEXEC);

#ifdef DEBUG_LATENCY
  // When debugging latencies, just keep the messages longer in the send
  // queue until they are flushed by a timeout.
  int delay = 0;
#else
  int delay = 1;
#endif
  int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &delay, sizeof(delay));
  if (ret == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  ret = ::connect(fd, socketAddress._privateData->_addr, socketAddress._privateData->_addrlen);
  if (ret == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  // Past the connect use non blocking io, required.
  flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }
  ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (ret == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  _privateData->_fd = fd;
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
