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

#include "SocketServerTCP.h"

#include "ErrnoWin32.h"
#include "SocketPrivateDataWin32.h"
#include "SocketAddressPrivateDataWin32.h"

namespace OpenRTI {

SocketServerTCP::SocketServerTCP() :
  SocketServer(new PrivateData)
{
  _privateData->wsaStartup();
}

void
SocketServerTCP::bind(const SocketAddress& socketAddress)
{
  SOCKET fd = socket(socketAddress._privateData->_addr->sa_family, SOCK_STREAM, 0);
  if (fd == INVALID_SOCKET)
    throw TransportError(errnoToUcs(WSAGetLastError()));

  // Hmm, on win32 the SO_REUSEADDR does not just allow binding on a TIME_WAIT socket but
  // also forcably allows two sockets to bind on the same address - which is not what what we want.
  // So, do not do this on win32 ...
  // unsigned reuseaddr = 1;
  // setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

  int ret = ::bind(fd, socketAddress._privateData->_addr, socketAddress._privateData->_addrlen);
  if (ret == -1) {
    int errorNumber = WSAGetLastError();
    closesocket(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  _privateData->_socket = fd;
}

void
SocketServerTCP::listen(int backlog)
{
  int ret = ::listen(_privateData->_socket, backlog);
  if (ret == -1) {
    throw TransportError(errnoToUcs(WSAGetLastError()));
  }
}

SocketTCP*
SocketServerTCP::accept()
{
  SOCKET fd = ::accept(_privateData->_socket, 0, 0);
  if (fd == -1) {
    int errorNumber = WSAGetLastError();
    throw TransportError(errnoToUcs(errorNumber));
  }

  // Past the connect use non blocking io, required.
  u_long mode = 1;
  int ret = ioctlsocket(fd, FIONBIO, &mode);
  if (ret == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    closesocket(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  int one = 1;
  ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&one, sizeof(one));
  if (ret == -1) {
    int errorNumber = WSAGetLastError();
    closesocket(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  PrivateData* pd = new PrivateData;
  pd->wsaStartup();
  pd->_socket = fd;
  return new SocketTCP(pd);
}

SocketServerTCP::~SocketServerTCP()
{
  close();
}

} // namespace OpenRTI
