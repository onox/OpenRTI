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
#include "ErrnoWin32.h"
#include "SocketPrivateDataWin32.h"

namespace OpenRTI {

ssize_t
SocketPacket::send(const SocketAddress& socketAddress, const ConstBufferRange& bufferRange)
{
  throw RTIinternalError("No packet sockets on win32 so far!");
}

ssize_t
SocketPacket::recv(SocketAddress& socketAddress, const BufferRange& bufferRange, bool peek)
{
  throw RTIinternalError("No packet sockets on win32 so far!");
}

SocketPacket::SocketPacket(PrivateData* privateData) :
  SocketData(privateData)
{
}

SocketPacket::~SocketPacket()
{
}

} // namespace OpenRTI
