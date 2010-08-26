/* -*-c++-*- OpenRTI - Copyright (C) 2004-2010 Mathias Froehlich
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
    throw TransportError(L"Trying to connect an already open SocketTCP!");

  if (!socketAddress.valid())
    throw TransportError(L"Trying to connect an invalid address!");

  SOCKET fd = ::socket(socketAddress._privateData->_addr->sa_family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == INVALID_SOCKET)
    throw TransportError(errnoToUcs(WSAGetLastError()));

  int one = 1;
  int ret = ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&one, sizeof(one));
  if (ret == -1) {
    int errorNumber = WSAGetLastError();
    ::closesocket(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  ret = ::connect(fd, socketAddress._privateData->_addr, socketAddress._privateData->_addrlen);
  if (ret == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    ::closesocket(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  // Past the connect use non blocking io, required.
  u_long mode = 1;
  ret = ::ioctlsocket(fd, FIONBIO, &mode);
  if (ret == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    ::closesocket(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  _privateData->_socket = fd;
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
