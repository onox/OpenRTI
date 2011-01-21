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

#include "SocketAddress.h"
#include "SocketAddressPrivateDataPosix.h"

#include <algorithm>
#include "Exception.h"
#include "StringUtils.h"

namespace OpenRTI {

SocketAddress::SocketAddress() :
  _privateData(new PrivateData)
{
}

SocketAddress::SocketAddress(const SocketAddress& socketAddress) :
  _privateData(new PrivateData(*socketAddress._privateData))
{
}

SocketAddress::~SocketAddress()
{
  delete _privateData;
  _privateData = 0;
}

SocketAddress&
SocketAddress::operator=(const SocketAddress& socketAddress)
{
  if (socketAddress._privateData == _privateData)
    return *this;
  delete _privateData;
  _privateData = new PrivateData(*socketAddress._privateData);
  return *this;
}

bool
SocketAddress::operator<(const SocketAddress& socketAddress) const
{
  return std::lexicographical_compare(_privateData->begin(), _privateData->end(),
                                      socketAddress._privateData->begin(), socketAddress._privateData->end());
}

bool
SocketAddress::operator==(const SocketAddress& socketAddress) const
{
  if (_privateData->_addrlen != socketAddress._privateData->_addrlen)
    return false;
  return std::memcmp(_privateData->begin(), socketAddress._privateData->begin(), _privateData->_addrlen) == 0;
}

bool
SocketAddress::valid() const
{
  return _privateData->_addrlen != 0;
}

bool
SocketAddress::isInet4() const
{
  if (_privateData->_addrlen != sizeof(struct sockaddr_in))
    return false;
  return _privateData->_addr->sa_family == AF_INET;
}

bool
SocketAddress::isInet6() const
{
  if (_privateData->_addrlen != sizeof(struct sockaddr_in6))
    return false;
  return _privateData->_addr->sa_family == AF_INET6;
}

std::list<SocketAddress>
SocketAddress::resolve(const std::wstring& address, const std::wstring& service)
{
  // Due to problems with ipv6 on these platforms
#if defined __sun || defined __hpux
  int family = AF_INET;
#else
  int family = AF_UNSPEC;
#endif
  int socktype = SOCK_STREAM;

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = family;
  hints.ai_socktype = socktype;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  // If the address is empty, assume we are asking for a wildcard address
  if (address.empty())
    hints.ai_flags = AI_PASSIVE;

  std::string localeAddress = ucsToLocale(address);
  std::string localeService = ucsToLocale(service);
  struct addrinfo *ai = 0;
  while (int ret = ::getaddrinfo(localeAddress.c_str(), localeService.c_str(), &hints, &ai)) {
    if (ret == EAI_AGAIN)
      continue;
    ::freeaddrinfo(ai);
    throw TransportError(localeToUcs(gai_strerror(ret)));
  }

  std::list<SocketAddress> socketAddressList;
  struct addrinfo *res = ai;
  while (res) {
    socketAddressList.push_back(SocketAddress(new PrivateData(res->ai_addr, res->ai_addrlen)));
    res = res->ai_next;
  }
  ::freeaddrinfo(ai);

  return socketAddressList;
}

SocketAddress::SocketAddress(PrivateData* privateData) :
  _privateData(privateData)
{
}

}
