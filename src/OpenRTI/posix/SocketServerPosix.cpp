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

#include "SocketServer.h"

#include "ErrnoPosix.h"
#include "SocketPrivateDataPosix.h"
#include "SocketAddressPrivateDataPosix.h"

namespace OpenRTI {

SocketAddress
SocketServer::getsockname() const
{
  socklen_t addrlen = 0;
  int ret = ::getsockname(_privateData->_fd, 0, &addrlen);
  if (ret == -1) {
    int errorNumber = errno;
    throw TransportError(errnoToUtf8(errorNumber));
  }

  SharedPtr<SocketAddress::PrivateData> privateData = SocketAddress::PrivateData::create(addrlen);
  struct sockaddr* sockaddr = SocketAddress::PrivateData::sockaddr(privateData.get());
  addrlen = SocketAddress::PrivateData::capacity(privateData.get());
  ret = ::getsockname(_privateData->_fd, sockaddr, &addrlen);
  if (ret == -1) {
    int errorNumber = errno;
    throw TransportError(errnoToUtf8(errorNumber));
  }
  SocketAddress::PrivateData::setAddrlen(privateData.get(), addrlen);

  return SocketAddress(privateData.get());
}

SocketServer::SocketServer(PrivateData* privateData) :
  Socket(privateData)
{
}

SocketServer::~SocketServer()
{
}

} // namespace OpenRTI
