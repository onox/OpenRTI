/* -*-c++-*- OpenRTI - Copyright (C) 2004-2011 Mathias Froehlich 
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

#include "SocketStream.h"

#include "ErrnoWin32.h"
#include "SocketPrivateDataWin32.h"

namespace OpenRTI {

ssize_t
SocketStream::send(const NetworkBuffer& networkBuffer, bool)
{
  const char* data = networkBuffer.getPendingBuffer(0);
  size_t len = networkBuffer.getPendingBufferSize(0);
  ssize_t ret = ::send(_privateData->_socket, data, len, 0);
  if (ret != SOCKET_ERROR)
    return ret;

  int errorNumber = WSAGetLastError();
  // errors that just mean 'please try again' which is mapped to 'return nothing written'
  if (errorNumber == WSAEINTR || errorNumber == WSAEINPROGRESS || errorNumber == WSAENOBUFS || errorNumber == WSAEWOULDBLOCK)
    return 0;

  // Hmm, not sure if we should do so - not yet message based sockets in use
  if (errorNumber == WSAEMSGSIZE)
    return -1;

  // // Also not sure - currently this is an exception when the connection is just closed below us
  // // Note that this should not happen during any controlled shutdown of a client
  // if (errorNumber == WSAECONNRESET || errorNumber == WSAEPIPE)
  //   return -1;

  // All other errors are considered serious and need to be handled somewhere where this is caught
  throw TransportError(errnoToUcs(errorNumber));
}

ssize_t
SocketStream::recv(NetworkBuffer& networkBuffer)
{
  char* data = networkBuffer.getPendingBuffer(0);
  size_t len = networkBuffer.getPendingBufferSize(0);
  ssize_t ret = ::recv(_privateData->_socket, data, len, 0);
  if (ret != SOCKET_ERROR)
    return ret;

  int errorNumber = WSAGetLastError();
  // errors that just mean 'please try again' which is mapped to the traditional return path for read.
  // note that return 0 traditionally means end of file for reads
  if (errorNumber == WSAEWOULDBLOCK || errorNumber == WSAEINTR || errorNumber == WSAEINPROGRESS)
    return -1;

  // All other errors are considered serious and need to be handled somewhere where this is caught
  throw TransportError(errnoToUcs(errorNumber));
}

SocketStream::SocketStream(PrivateData* privateData) :
  SocketData(privateData)
{
}

SocketStream::~SocketStream()
{
}

} // namespace OpenRTI
