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

#include "SocketWakeupTrigger.h"

#include "Exception.h"
#include "ErrnoWin32.h"
#include "SocketPrivateDataWin32.h"

namespace OpenRTI {

SocketWakeupTrigger::SocketWakeupTrigger() :
  Socket(new PrivateData)
{
}

SocketWakeupEvent*
SocketWakeupTrigger::connect()
{
  if (_socketWakeupEvent.valid())
    return _socketWakeupEvent.get();

  if (_privateData->_socket != SOCKET_ERROR)
    throw TransportError(L"SocketEventTrigger already connected but _socketWakeupEvent is zero!");

  _privateData->wsaStartup();

  // Build up an ipv4 socket, and connect immediately.
  SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener == INVALID_SOCKET) {
    int errorNumber = WSAGetLastError();
    throw TransportError(errnoToUcs(errorNumber));
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
    throw TransportError(errnoToUcs(errorNumber));
  }
  ret = getsockname(listener, (struct sockaddr*)&addr, &addrlen);
  if (ret == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    closesocket(listener);
    throw TransportError(errnoToUcs(errorNumber));
  }

  // Note that this 'listen for exactly 1 connect' is the key to prevent a
  // hijacked socket. It will just fail in this case
  if (listen(listener, 1) == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    closesocket(listener);
    throw TransportError(errnoToUcs(errorNumber));
  }
  SOCKET pipeFd[2];
  pipeFd[0] = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
  if (pipeFd[0] == INVALID_SOCKET) {
    int errorNumber = WSAGetLastError();
    closesocket(listener);
    throw TransportError(errnoToUcs(errorNumber));
  }
  if (::connect(pipeFd[0], (const struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    closesocket(pipeFd[0]);
    closesocket(listener);
    throw TransportError(errnoToUcs(errorNumber));
  }
  pipeFd[1] = accept(listener, NULL, NULL);
  if (pipeFd[1] == INVALID_SOCKET) {
    int errorNumber = WSAGetLastError();
    closesocket(pipeFd[0]);
    closesocket(listener);
    throw TransportError(errnoToUcs(errorNumber));
  }
  closesocket(listener);

  _privateData->_socket = pipeFd[1];
  
  _socketWakeupEvent = new SocketWakeupEvent(new PrivateData(pipeFd[0]));
  return _socketWakeupEvent.get();
}

ssize_t
SocketWakeupTrigger::trigger()
{
  char data = 1;
  ssize_t ret = ::send(_privateData->_socket, &data, sizeof(data), 0);
  if (ret == SOCKET_ERROR) {
    int errorNumber = WSAGetLastError();
    if (errorNumber == WSAESHUTDOWN || errorNumber == WSAECONNRESET)
      return -1;
    if (errorNumber == WSAEINTR || errorNumber == WSAEINPROGRESS || errorNumber == WSAENOBUFS || errorNumber == WSAEWOULDBLOCK)
      return 0;
    throw TransportError(errnoToUcs(errorNumber));
  }
  return 1;
}

SocketWakeupTrigger::~SocketWakeupTrigger()
{
}

} // namespace OpenRTI
