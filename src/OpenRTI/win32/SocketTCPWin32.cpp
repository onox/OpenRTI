/* -*-c++-*- OpenRTI - Copyright (C) 2004-2022 Mathias Froehlich
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

#include "ErrnoWin32.h"
#include "SocketPrivateDataWin32.h"
#include "SocketAddressPrivateDataWin32.h"

namespace OpenRTI {

SocketTCP::SocketTCP() :
  SocketStream(new PrivateData)
{
  _privateData->wsaStartup();
}

void
SocketTCP::connect(const SocketAddress& socketAddress)
{
  if (_privateData->_socket != INVALID_SOCKET)
    throw TransportError("Trying to connect an already open SocketTCP!");

  if (!socketAddress.valid())
    throw TransportError("Trying to connect to an invalid address!");

  const struct sockaddr* sockaddr = SocketAddress::PrivateData::sockaddr(socketAddress.constData());
  SOCKET fd = ::socket(sockaddr->sa_family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == INVALID_SOCKET)
    throw TransportError(errnoToUtf8(WSAGetLastError()));

  int nodelay = 1;
  int ret = ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));
  if (ret == -1) {
    int errorNumber = WSAGetLastError();
    ::closesocket(fd);
    throw TransportError(errnoToUtf8(errorNumber));
  }

  // Switch to nonblocking io
  u_long mode = 1;
  ret = ::ioctlsocket(fd, FIONBIO, &mode);
  if (ret == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    ::closesocket(fd);
    throw TransportError(errnoToUtf8(errorNumber));
  }

  socklen_t addrlen = SocketAddress::PrivateData::addrlen(socketAddress.constData());
  ret = ::connect(fd, sockaddr, addrlen);
  if (ret == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    if (errorNumber != WSAEINPROGRESS && errorNumber != WSAEWOULDBLOCK) {
      ::closesocket(fd);
      throw TransportError(errnoToUtf8(errorNumber));
    }
  }

  _privateData->_socket = fd;
}

void
SocketTCP::cork(bool enable)
{
  int nodelay;
#ifdef DEBUG_LATENCY
  nodelay = 1;
#else
  if (enable)
    nodelay = 0;
  else
    nodelay = 1;
#endif
  setsockopt(_privateData->_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));
}

void
SocketTCP::shutdown()
{
  SOCKET fd = _privateData->_socket;
  if (fd == INVALID_SOCKET)
    return;
  ::shutdown(fd, SD_SEND);
}

SocketTCP::SocketTCP(PrivateData* privateData) :
  SocketStream(privateData)
{
}

SocketTCP::~SocketTCP()
{
  close();
}

} // namespace OpenRTI
