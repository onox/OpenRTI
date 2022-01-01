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

#include <netdb.h>
#include <cstring>

#include "Exception.h"
#include "LogStream.h"
#include "Referenced.h"

namespace OpenRTI {

struct SocketAddress::PrivateData : public Referenced {
  static socklen_t capacity(const PrivateData* privateData)
  {
    if (!privateData)
      return 0;
    return privateData->_capacity;
  }
  static socklen_t addrlen(const PrivateData* privateData)
  {
    if (!privateData)
      return 0;
    return privateData->_addrlen;
  }
  static void setAddrlen(PrivateData* privateData, socklen_t addrlen)
  {
    OpenRTIAssert(addrlen <= capacity(privateData));
    if (!privateData)
      return;
    privateData->_addrlen = addrlen;
  }

  static const struct sockaddr* sockaddr(const PrivateData* privateData)
  {
    if (!privateData)
      return 0;
    return &privateData->_sockaddr;
  }
  static struct sockaddr* sockaddr(PrivateData* privateData)
  {
    if (!privateData)
      return 0;
    return &privateData->_sockaddr;
  }

  static PrivateData* create(socklen_t addrlen)
  {
    socklen_t capacity = std::max(addrlen, socklen_t(sizeof(struct sockaddr_storage)));
    void* data = ::operator new(capacity + sizeof(PrivateData) - sizeof(struct sockaddr));
    return new (data) PrivateData(capacity);
  }
  static PrivateData* create(const struct sockaddr* addr, socklen_t addrlen)
  {
    PrivateData* privateData = create(addrlen);
    privateData->_addrlen = addrlen;
    std::memcpy(&privateData->_sockaddr, addr, addrlen);
    return privateData;
  }

private:
  PrivateData(socklen_t capacity) :
    _capacity(capacity),
    _addrlen(0)
  {
    std::memset(&_sockaddr, 0, capacity);
  }

  // We should not need them
  PrivateData(const PrivateData& privateData);
  PrivateData& operator=(const PrivateData& privateData);

  socklen_t _capacity;
  socklen_t _addrlen;
  struct sockaddr _sockaddr;
};

}

#endif
