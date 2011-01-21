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

#include "SocketPacket.h"
#include "SocketPrivateDataPosix.h"
#include "ErrnoPosix.h"

namespace OpenRTI {

ssize_t
SocketPacket::send(const SocketAddress& socketAddress, const NetworkBuffer& networkBuffer)
{
  // Currently fixed to max of 100, 
  // For packet sockets return with the equivalent of EMSGSIZE in that case.
  // Packet socket handlers must be able to handle that anyway.
  struct iovec iov[100];
  size_t iovlen = 0;
  size_t bytelen = 0;
  size_t numPengingBuffers = networkBuffer.getNumPendingBuffers();
  for (size_t i = 0; i < numPengingBuffers; ++i) {
    size_t size = networkBuffer.getPendingBufferSize(i);
    if (!size)
      continue;

    iov[iovlen].iov_base = const_cast<char*>(networkBuffer.getPendingBuffer(i));
    iov[iovlen].iov_len = size;
    bytelen += size;
    ++iovlen;

    // Return with the equivalent of EMSGSIZE here if we run out of space in the iovec.
    if (sizeof(iov)/sizeof(iov[0]) <= iovlen)
      return -1;
  }

  struct msghdr msg = { 0, };
  msg.msg_name = 0;
  msg.msg_namelen = 0;
  msg.msg_iov = iov;
  msg.msg_iovlen = iovlen;

  ssize_t ret = ::sendmsg(_privateData->_fd, &msg, 0);
  if (ret != -1)
    return ret;

  // errors that just mean 'please try again' which is mapped to 'return nothing written'
  if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR /* || errno == EINPROGRESS*/)
    return 0;

  // Hmm, not sure if we should do so - not yet message based sockets in use
  // FIXME here in a stream socket implementation !!!
  if (errno == EMSGSIZE)
    return -1;

  // // Also not sure - currently this is an exception when the connection is just closed below us
  // // Note that this should not happen during any controlled shutdown of a client
  // if (errno == ECONNRESET || errno == EPIPE)
  //   return -1;

  // All other errors are considered serious and need to be handled somewhere where this is caught
  throw TransportError(errnoToUcs(errno));
}

ssize_t
SocketPacket::recv(SocketAddress& socketAddress, NetworkBuffer& networkBuffer, bool peek)
{
  // Currently fixed to max of 100
  // For stream sockets it does not matter: If we could not receive all in one chunk, the next chunk will receive the rest.
  struct iovec iov[100];
  size_t iovlen = 0;
  size_t bytelen = 0;
  size_t numPengingBuffers = networkBuffer.getNumPendingBuffers();
  for (size_t i = 0; i < numPengingBuffers; ++i) {
    size_t size = networkBuffer.getPendingBufferSize(i);
    if (!size)
      continue;

    iov[iovlen].iov_base = networkBuffer.getPendingBuffer(i);
    iov[iovlen].iov_len = size;
    bytelen += size;
    ++iovlen;

    // Stop processing stuff here if we run out of space in the iovec ...
    if (sizeof(iov)/sizeof(iov[0]) <= iovlen)
      break;
  }

  struct msghdr msg = { 0, };
  msg.msg_name = 0;
  msg.msg_namelen = 0;
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

  // All other errors are considered serious and need to be handled somewhere where this is caught
  throw TransportError(errnoToUcs(errno));
}

SocketPacket::SocketPacket(PrivateData* privateData) :
  SocketData(privateData)
{
}

SocketPacket::~SocketPacket()
{
}

} // namespace OpenRTI
