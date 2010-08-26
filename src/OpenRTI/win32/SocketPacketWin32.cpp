/* -*-c++-*- OpenRTI - Copyright (C) 2009-2010 Mathias Froehlich
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
#include "ErrnoWin32.h"
#include "SocketPrivateDataWin32.h"

namespace OpenRTI {

ssize_t
SocketPacket::send(const SocketAddress& socketAddress, const NetworkBuffer& networkBuffer)
{
  throw RTIinternalError("No packet sockets on win32 so far!");
  // // Currently fixed to max of 100, 
  // // For packet sockets return with the equivalent of EMSGSIZE in that case.
  // // Packet socket handlers must be able to handle that anyway.
  // WSABUF iov[100];
  // size_t iovlen = 0;
  // size_t bytelen = 0;
  // for (size_t i = networkBuffer._dataVectorIndex; i < networkBuffer.size(); ++i) {
  //   size_t size = networkBuffer[i].size();
  //   if (!size)
  //     continue;

  //   if (networkBuffer._dataVectorIndex == i) {
  //     iov[iovlen].buf = static_cast<char*>(const_cast<void*>(networkBuffer[i].data(networkBuffer._dataVectorOffset)));
  //     size -= networkBuffer._dataVectorOffset;
  //   } else {
  //     iov[iovlen].buf = static_cast<char*>(const_cast<void*>(networkBuffer[i].data()));
  //   }
  //   iov[iovlen].len = size;
  //   bytelen += size;
  //   ++iovlen;

  //   // Return with the equivalent of EMSGSIZE here if we run out of space in the iovec.
  //   if (sizeof(iov)/sizeof(iov[0]) <= iovlen)
  //     return -1;
  // }

  // WSAMSG msg;
  // msg.msg_name = 0;
  // msg.msg_namelen = 0;
  // msg.msg_iov = iov;
  // msg.msg_iovlen = iovlen;
  // msg.msg_control = 0;
  // msg.msg_controllen = 0;
  // msg.msg_flags = 0;

  // DWORD lpNumberOfBytesSent = 0;
  // int ret = WSASendMsg(_privateData->_socket, &msg, 0, &lpNumberOfBytesSent, 0, 0);
  // if (ret != SOCKET_ERROR)
  //   return lpNumberOfBytesSent;

  // int errorNumber = WSAGetLastError();
  // // errors that just mean 'please try again' which is mapped to 'return nothing written'
  // if (errorNumber == WSAEINTR || errorNumber == WSAEINPROGRESS || errorNumber == WSAENOBUFS || errorNumber == WSAEWOULDBLOCK)
  //   return 0;

  // // Hmm, not sure if we should do so - not yet message based sockets in use
  // if (errorNumber == WSAEMSGSIZE)
  //   return -1;

  // // // Also not sure - currently this is an exception when the connection is just closed below us
  // // // Note that this should not happen during any controlled shutdown of a client
  // // if (errorNumber == WSAECONNRESET || errorNumber == WSAEPIPE)
  // //   return -1;

  // // All other errors are considered serious and need to be handled somewhere where this is caught
  // throw TransportError(errnoToUcs(errno));
}

ssize_t
SocketPacket::recv(SocketAddress& socketAddress, NetworkBuffer& networkBuffer, bool peek)
{
  throw RTIinternalError("No packet sockets on win32 so far!");
  // // Currently fixed to max of 100
  // // For stream sockets it does not matter: If we could not receive all in one chunk, the next chunk will receive the rest.
  // WSABUF iov[100];
  // size_t iovlen = 0;
  // size_t bytelen = 0;
  // for (size_t i = networkBuffer._dataVectorIndex; i < networkBuffer.size(); ++i) {
  //   size_t size = networkBuffer[i].size();
  //   if (!size)
  //     continue;

  //   if (networkBuffer._dataVectorIndex == i) {
  //     iov[iovlen].buf = networkBuffer[i].data(networkBuffer._dataVectorOffset);
  //     size -= networkBuffer._dataVectorOffset;
  //   } else {
  //     iov[iovlen].buf = networkBuffer[i].data();
  //   }
  //   iov[iovlen].len = size;
  //   bytelen += size;
  //   ++iovlen;

  //   // Stop processing stuff here if we run out of space in the iovec ...
  //   if (sizeof(iov)/sizeof(iov[0]) <= iovlen)
  //     break;
  // }

  // WSAMSG msg;
  // msg.name = 0;
  // msg.namelen = 0;
  // msg.lpBuffers = iov;
  // msg.dwBufferCount = iovlen;
  // msg.Control.buf = 0;
  // msg.Control.len = 0;
  // msg.dwFlags = 0;

  // if (peek)
  //   msg.dwFlags |= MSG_PEEK;
  // DWORD lpdwNumberOfBytesRecvd = 0;
  // int ret = WSARecvMsg(_privateData->_socket, &msg, &lpdwNumberOfBytesRecvd, 0, 0);
  // if (ret != SOCKET_ERROR)
  //   return lpdwNumberOfBytesRecvd;

  // int errorNumber = WSAGetLastError();
  // // errors that just mean 'please try again' which is mapped to the traditional return path for read.
  // // note that return 0 traditionally means end of file for reads
  // if (errorNumber == WSAEWOULDBLOCK || errorNumber == WSAEINTR || errorNumber == WSAEINPROGRESS)
  //   return -1;

  // // All other errors are considered serious and need to be handled somewhere where this is caught
  // throw TransportError(errnoToUcs(errno));
}

SocketPacket::SocketPacket(PrivateData* privateData) :
  SocketData(privateData)
{
}

SocketPacket::~SocketPacket()
{
}

} // namespace OpenRTI
