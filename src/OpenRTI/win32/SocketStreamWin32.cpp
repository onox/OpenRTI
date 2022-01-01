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

#include "SocketStream.h"

#include "ErrnoWin32.h"
#include "SocketAddressPrivateDataWin32.h"
#include "SocketPrivateDataWin32.h"

namespace OpenRTI {

ssize_t
SocketStream::send(const ConstBufferRange& bufferRange, bool more)
{
  size_t bytelen = 0;
  size_t sendBufferSize = 64*1024; /* FIXME Use the real send buffer size instead */
  // Currently fixed to max of 100
  // For stream sockets it does not matter: If we could not send all in one chunk, the next chunk will send the rest.
  WSABUF buffers[100];
  DWORD bufferCount = 0;
  DWORD maxBufferCount = sizeof(buffers)/sizeof(buffers[0]);
  Buffer::const_byte_iterator i = bufferRange.first;
  i.skip_empty_chunks(bufferRange.second);
  for (; i != bufferRange.second; i.skip_empty_chunks(bufferRange.second)) {
    size_t size = i.chunk_size(bufferRange.second);
    if (!size)
      continue;

#ifdef DEBUG2_ASSEMBLY
    size = 1;
#endif

    buffers[bufferCount].buf = (char*)i.data();
    buffers[bufferCount].len = ULONG(size);
    bytelen += size;
    i += size;
    ++bufferCount;

#if defined DEBUG_ASSEMBLY || defined DEBUG2_ASSEMBLY
    break;
#endif

    // Stop processing stuff here if we run out of space in the buffers ...
    if (maxBufferCount <= bufferCount)
      break;
    // ... and when we expect to fail at this amount of memory anyway.
    if (sendBufferSize < bytelen)
      break;
  }

  if (more)
    cork(true);

  DWORD flags = 0;
  DWORD numBytesSent = 0;
  int ret = WSASend(_privateData->_socket, buffers, bufferCount, &numBytesSent, flags, NULL, NULL);
  // get the error of the send call before trying setsocketopt
  int errorNumber = WSAGetLastError();

  // flush the buffer
  if (!more)
    cork(false);

  if (ret != SOCKET_ERROR)
    return numBytesSent;

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
  size_t bytelen = 0;
  size_t readBufferSize = 64*1024; /* FIXME Use the real read buffer size instead */
  // Currently fixed to max of 100
  // For stream sockets it does not matter: If we could not receive all in one chunk, the next chunk will receive the rest.
  WSABUF buffers[100];
  DWORD bufferCount = 0;
  DWORD maxBufferCount = sizeof(buffers)/sizeof(buffers[0]);
  Buffer::byte_iterator i = bufferRange.first;
  i.skip_empty_chunks(bufferRange.second);
  for (;i != bufferRange.second; i.skip_empty_chunks(bufferRange.second)) {
    size_t size = i.chunk_size(bufferRange.second);
    if (!size)
      continue;

#ifdef DEBUG2_ASSEMBLY
    size = 1;
#endif

    buffers[bufferCount].buf = (char*)i.data();
    buffers[bufferCount].len = ULONG(size);
    bytelen += size;
    i += size;
    ++bufferCount;

#if defined DEBUG_ASSEMBLY || defined DEBUG2_ASSEMBLY
    break;
#endif

    // Stop processing stuff here if we run out of space in the buffers ...
    if (maxBufferCount <= bufferCount)
      break;
    // ... and when we expect to fail at this amount of memory anyway.
    if (readBufferSize < bytelen)
      break;
  }

  DWORD flags = 0;
  if (peek)
    flags |= MSG_PEEK;

  DWORD numBytesRecvd = 0;
  int ret = WSARecv(_privateData->_socket, buffers, bufferCount, &numBytesRecvd, &flags, NULL, NULL);
  if (ret != SOCKET_ERROR)
    return numBytesRecvd;

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
