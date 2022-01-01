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

#include "SocketPacket.h"

#ifdef OpenRTI_HAVE_ALLOCA
#include <alloca.h>
#endif

#include "SocketAddressPrivateDataPosix.h"
#include "SocketPrivateDataPosix.h"
#include "ErrnoPosix.h"

namespace OpenRTI {

ssize_t
SocketPacket::send(const SocketAddress& socketAddress, const ConstBufferRange& bufferRange)
{
  size_t bytelen = 0;
#ifdef OpenRTI_HAVE_ALLOCA
#if defined(__sun)
  size_t numPengingBuffers = 100;
#else
  size_t numPengingBuffers = std::distance(bufferRange.first.iterator(), bufferRange.second.iterator());
#endif
  size_t maxIovlen = numPengingBuffers;
  struct iovec* iov = static_cast<struct iovec*>(alloca(maxIovlen*sizeof(struct iovec)));
  size_t iovlen = 0;
#else
  // Currently fixed to max of 100
  struct iovec iov[100];
  size_t iovlen = 0;
  size_t maxIovlen = sizeof(iov)/sizeof(iov[0]);
#endif
  Buffer::const_byte_iterator i = bufferRange.first;
  i.skip_empty_chunks(bufferRange.second);
  for (; i != bufferRange.second; i.skip_empty_chunks(bufferRange.second)) {
    size_t size = i.chunk_size(bufferRange.second);
    if (!size)
      continue;

    iov[iovlen].iov_base = (char*)i.data();
    iov[iovlen].iov_len = size;
    bytelen += size;
    i += size;
    ++iovlen;

    // Return error here if we run out of space in the iovec ...
    if (maxIovlen <= iovlen)
      return -1;
  }

  struct msghdr msg = { 0, };
  msg.msg_name = const_cast<struct sockaddr*>(SocketAddress::PrivateData::sockaddr(socketAddress.constData()));
  msg.msg_namelen = SocketAddress::PrivateData::addrlen(socketAddress.constData());
  msg.msg_iov = iov;
  msg.msg_iovlen = iovlen;

  int flags = 0;
  ssize_t ret = ::sendmsg(_privateData->_fd, &msg, flags);
  int errorNumber = errno;

  if (ret != -1)
    return ret;

  // errors that just mean 'please try again' which is mapped to 'return nothing written'
  if (errorNumber == EWOULDBLOCK || errorNumber == EAGAIN || errorNumber == EINTR /* || errorNumber == EINPROGRESS*/)
    return 0;

  // Hmm, not sure if we should do so - not yet message based sockets in use
  if (errorNumber == EMSGSIZE)
    return -1;

  // // Also not sure - currently this is an exception when the connection is just closed below us
  // // Note that this should not happen during any controlled shutdown of a client
  // if (errorNumber == ECONNRESET || errorNumber == EPIPE)
  //   return -1;

  // All other errors are considered serious and need to be handled somewhere where this is caught
  throw TransportError(errnoToUtf8(errorNumber));
}

ssize_t
SocketPacket::recv(SocketAddress& socketAddress, const BufferRange& bufferRange, bool peek)
{
  size_t bytelen = 0;
#ifdef OpenRTI_HAVE_ALLOCA
#if defined(__sun)
  size_t numPengingBuffers = 100;
#else
  size_t numPengingBuffers = std::distance(bufferRange.first.iterator(), bufferRange.second.iterator());
#endif
  size_t maxIovlen = numPengingBuffers;
  struct iovec* iov = static_cast<struct iovec*>(alloca(maxIovlen*sizeof(struct iovec)));
  size_t iovlen = 0;
#else
  // Currently fixed to max of 100
  // For stream sockets it does not matter: If we could not receive all in one chunk, the next chunk will receive the rest.
  struct iovec iov[100];
  size_t iovlen = 0;
  size_t maxIovlen = sizeof(iov)/sizeof(iov[0]);
#endif
  Buffer::byte_iterator i = bufferRange.first;
  i.skip_empty_chunks(bufferRange.second);
  for (;i != bufferRange.second; i.skip_empty_chunks(bufferRange.second)) {
    size_t size = i.chunk_size(bufferRange.second);
    if (!size)
      continue;

    iov[iovlen].iov_base = (char*)i.data();
    iov[iovlen].iov_len = size;
    bytelen += size;
    i += size;
    ++iovlen;

    // Return error here if we run out of space in the iovec ...
    if (maxIovlen <= iovlen)
      return -1;
  }

  struct msghdr msg = { 0, };
  SocketAddress::PrivateData* socketAddressPrivateData = socketAddress.data();
  msg.msg_name = SocketAddress::PrivateData::sockaddr(socketAddressPrivateData);
  msg.msg_namelen = SocketAddress::PrivateData::capacity(socketAddressPrivateData);
  msg.msg_iov = iov;
  msg.msg_iovlen = iovlen;

  int flags = 0;
  if (peek)
    flags |= MSG_PEEK;
  ssize_t ret = ::recvmsg(_privateData->_fd, &msg, flags);
  if (ret != -1)
    return ret;

  // errors that just mean 'please try again' which is mapped to the traditional return path for read.
  // note that return 0 traditionally means end of file for reads
  if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
    return -1;

  // Set the address size
  SocketAddress::PrivateData::setAddrlen(socketAddressPrivateData, msg.msg_namelen);

  // All other errors are considered serious and need to be handled somewhere where this is caught
  throw TransportError(errnoToUtf8(errno));
}

SocketPacket::SocketPacket(PrivateData* privateData) :
  SocketData(privateData)
{
}

SocketPacket::~SocketPacket()
{
}

} // namespace OpenRTI
