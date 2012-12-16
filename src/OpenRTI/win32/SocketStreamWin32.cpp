/* -*-c++-*- OpenRTI - Copyright (C) 2004-2012 Mathias Froehlich
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
#include "SocketAddressPrivateDataWin32.h"
#include "SocketPrivateDataWin32.h"

namespace OpenRTI {

ssize_t
SocketStream::send(const ConstBufferRange& bufferRange, bool more)
{
  const char* data = (const char*)bufferRange.first.data();
  size_t len = bufferRange.first.chunk_size(bufferRange.second);

  // If not already set, check if we have more then one chunk pending
  if (!more && 0 < len) {
    Buffer::const_iterator i = bufferRange.first.iterator();
    more = ++i != bufferRange.second.iterator();
  }
  if (more)
    cork(true);

  ssize_t ret = ::send(_privateData->_socket, data, len, 0);
  // get the error of the send call before trying setsocketopt
  int errorNumber = WSAGetLastError();

  // flush the buffer
  if (!more)
    cork(false);

  if (ret != SOCKET_ERROR)
    return ret;

  // errors that just mean 'please try again' which is mapped to 'return nothing written'
  if (errorNumber == WSAEINTR || errorNumber == WSAEINPROGRESS || errorNumber == WSAENOBUFS || errorNumber == WSAEWOULDBLOCK)
    return 0;

  // Hmm, not sure if we should do so - not yet message based sockets in use
  if (errorNumber == WSAEMSGSIZE)
    return -1;

  // Also not sure - currently this is an exception when the connection is just closed below us
  // Note that this should not happen during any controlled shutdown of a client
  if (errorNumber == WSAECONNRESET || errorNumber == WSAECONNABORTED || errorNumber == WSAESHUTDOWN)
    return -1;

  // All other errors are considered serious and need to be handled somewhere where this is caught
  throw TransportError(errnoToUtf8(errorNumber));
}

ssize_t
SocketStream::recv(const BufferRange& bufferRange, bool peek)
{
  char* data = (char*)bufferRange.first.data();
  size_t len = bufferRange.first.chunk_size(bufferRange.second);
  int flags = 0;
  if (peek)
    flags |= MSG_PEEK;
  ssize_t ret = ::recv(_privateData->_socket, data, len, flags);
  if (ret != SOCKET_ERROR)
    return ret;

  int errorNumber = WSAGetLastError();
  // errors that just mean 'please try again' which is mapped to the traditional return path for read.
  // note that return 0 traditionally means end of file for reads
  if (errorNumber == WSAEWOULDBLOCK || errorNumber == WSAEINTR || errorNumber == WSAEINPROGRESS)
    return -1;

  // All other errors are considered serious and need to be handled somewhere where this is caught
  throw TransportError(errnoToUtf8(errorNumber));
}

void
SocketStream::cork(bool enable)
{
}

void
SocketStream::shutdown()
{
}

SocketAddress
SocketStream::getpeername() const
{
  struct sockaddr_storage sockaddr;
  socklen_t addrlen = sizeof(sockaddr);
  int ret = ::getpeername(_privateData->_socket, (struct sockaddr*)&sockaddr, &addrlen);
  if (ret == -1)
    throw TransportError(errnoToUtf8(WSAGetLastError()));

  return SocketAddress(new SocketAddress::PrivateData((struct sockaddr*)&sockaddr, addrlen));
}

SocketStream::SocketStream(PrivateData* privateData) :
  SocketData(privateData)
{
}

SocketStream::~SocketStream()
{
}

} // namespace OpenRTI
