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

#ifndef OpenRTI_SocketAddressPrivateDataPosix_h
#define OpenRTI_SocketAddressPrivateDataPosix_h

#include "SocketAddress.h"

#include <cstring>

#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "LogStream.h"
#include "Referenced.h"

namespace OpenRTI {

struct SocketAddress::PrivateData : public Referenced {
  PrivateData() :
    _addrlen(0)
  {
  }
  PrivateData(const struct sockaddr* addr, socklen_t addrlen) :
    _addrlen(0)
  {
    if (sizeof(_sockaddr) < size_t(addrlen)) {
      std::memset(&_sockaddr + addrlen, 0, sizeof(_sockaddr));
      Log(Network, Warning) << "Socket address is too long!" << std::endl;
      return;
    }
    _addrlen = addrlen;
    if (addrlen)
      std::memcpy(&_sockaddr, addr, addrlen);
    size_t len = sizeof(_sockaddr) - addrlen;
    if (len)
      std::memset(reinterpret_cast<char*>(&_sockaddr) + addrlen, 0, len);
  }

  static socklen_t addrlen(const PrivateData* privateData)
  {
    if (!privateData)
      return 0;
    return privateData->_addrlen;
  }
  static const struct sockaddr* sockaddr(const PrivateData* privateData)
  {
    if (!privateData)
      return 0;
    return reinterpret_cast<const struct sockaddr*>(&privateData->_sockaddr);
  }
  static struct sockaddr* sockaddr(PrivateData* privateData)
  {
    if (!privateData)
      return 0;
    return reinterpret_cast<struct sockaddr*>(&privateData->_sockaddr);
  }

private:
  // We should not need them
  PrivateData(const PrivateData& privateData);
  PrivateData& operator=(const PrivateData& privateData);

  socklen_t _addrlen;
  struct sockaddr_storage _sockaddr;
};

}

#endif
