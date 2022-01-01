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

#ifndef OpenRTI_SocketPrivateDataWin32_h
#define OpenRTI_SocketPrivateDataWin32_h

#include "Socket.h"

#include "Exception.h"
#include "Export.h"

#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>

namespace OpenRTI {

struct OPENRTI_LOCAL Socket::PrivateData {
  // Note that the socket is always set to an ivalid socket for the reason that the
  // wsa startup code is only executed past the arguments to the constructor are created, consequently
  // we cannot reliably have any socket handle in our hands at that time
  PrivateData(SOCKET socket = SOCKET_ERROR) : _socket(socket), _wsaStartupCalled(false)
  {
    // Probably too late, but make sure we have a reference as long as we have a socket
    if (_socket != SOCKET_ERROR)
      wsaStartup();
  }
  ~PrivateData()
  {
    close();
    if (_wsaStartupCalled)
      WSACleanup();
  }

  void close()
  {
    if (_socket != INVALID_SOCKET) {
      closesocket(_socket);
      _socket = INVALID_SOCKET;
    }
  }

  // FIXME: may be have a setSocket in the private that also ensures wsastartup
  void wsaStartup()
  {
    if (_wsaStartupCalled)
      return;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
      throw TransportError("Could not initialize windows sockets!");
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
      WSACleanup();
      throw TransportError("Could not initialize windows sockets 2.2!");
    }
    _wsaStartupCalled = true;
  }

  SOCKET _socket;

private:
  bool _wsaStartupCalled;
};

} // namespace OpenRTI

#endif
