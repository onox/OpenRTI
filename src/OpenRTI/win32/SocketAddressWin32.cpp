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

#include "SocketAddress.h"

#include <algorithm>
#include <cstring>
#include <sstream>

#include "Exception.h"
#include "StringUtils.h"
#include "VariableLengthData.h"

#include "SocketAddressPrivateDataWin32.h"

namespace OpenRTI {

SocketAddress::SocketAddress()
{
}

SocketAddress::SocketAddress(const SocketAddress& socketAddress) :
  _privateData(socketAddress._privateData)
{
}

SocketAddress::~SocketAddress()
{
}

SocketAddress&
SocketAddress::operator=(const SocketAddress& socketAddress)
{
  _privateData = socketAddress._privateData;
  return *this;
}

bool
SocketAddress::valid() const
{
  return SocketAddress::PrivateData::addrlen(_privateData.get()) != 0;
}

bool
SocketAddress::isPipe() const
{
  return false;
}

bool
SocketAddress::isInet4() const
{
  if (SocketAddress::PrivateData::addrlen(_privateData.get()) != sizeof(struct sockaddr_in))
    return false;
  const struct sockaddr* addr = SocketAddress::PrivateData::sockaddr(_privateData.get());
  return addr->sa_family == AF_INET;
}

bool
SocketAddress::isInet6() const
{
  if (SocketAddress::PrivateData::addrlen(_privateData.get()) != sizeof(struct sockaddr_in6))
    return false;
  const struct sockaddr* addr = SocketAddress::PrivateData::sockaddr(_privateData.get());
  return addr->sa_family == AF_INET6;
}

std::string
SocketAddress::getNumericName() const
{
  if (!valid())
    return std::string();

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    throw TransportError("Could not initialize windows sockets!");
  if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
    WSACleanup();
    throw TransportError("Could not initialize windows sockets 2.2!");
  }

  socklen_t addrlen = PrivateData::addrlen(_privateData.get());
  const struct sockaddr* addr = PrivateData::sockaddr(_privateData.get());
  char host[256];
  host[sizeof(host)-1] = 0;
  char serv[32];
  serv[sizeof(serv)-1] = 0;
  while (int ret = ::getnameinfo(addr, addrlen, host, sizeof(host)-1, serv, sizeof(serv)-1, NI_NUMERICHOST|NI_NUMERICSERV)) {
    if (ret == EAI_AGAIN)
      continue;
    WSACleanup();
    throw TransportError(localeToUtf8(gai_strerror(ret)));
  }
  WSACleanup();

  std::stringstream ss;
  bool inet6 = isInet6();
  if (inet6)
    ss << "[";
  ss << host;
  if (inet6)
    ss << "]";
  ss << ":";
  ss << serv;
  return ss.str();
}

SocketAddressList
SocketAddress::resolve(const std::string& address, const std::string& service, bool passive)
{
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    throw TransportError("Could not initialize windows sockets!");
  if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
    WSACleanup();
    throw TransportError("Could not initialize windows sockets 2.2!");
  }

  int family = AF_UNSPEC;
  int socktype = SOCK_STREAM;

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = family;
  hints.ai_socktype = socktype;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  if (passive)
    hints.ai_flags = AI_PASSIVE;

  std::string localeAddress = utf8ToLocale(address);
  // getaddrinfo returnd the wildcard address when NULL is given as the node argument
  const char *localeAddressChar = NULL;
  if (!localeAddress.empty())
    localeAddressChar = localeAddress.c_str();
  std::string localeService = utf8ToLocale(service);
  struct addrinfo *ai = 0;
  while (int ret = ::getaddrinfo(localeAddressChar, localeService.c_str(), &hints, &ai)) {
    if (ret == EAI_AGAIN)
      continue;
    WSACleanup();
    throw TransportError(localeToUtf8(gai_strerror(ret)));
  }

  SocketAddressList socketAddressList;
  struct addrinfo *res = ai;
  while (res) {
    socketAddressList.push_back(SocketAddress(new PrivateData(res->ai_addr, socklen_t(res->ai_addrlen))));
    res = res->ai_next;
  }
  ::freeaddrinfo(ai);
  WSACleanup();

  return socketAddressList;
}

SocketAddressList
SocketAddress::resolve(const std::string& address, bool passive)
{
  std::pair<std::string, std::string> addressServicePair;
  addressServicePair = parseInetAddress(address);
  return resolve(addressServicePair.first, addressServicePair.second, passive);
}

VariableLengthData
SocketAddress::getNetworkAddressData() const
{
  if (isInet4()) {
    const struct sockaddr_in* addr = (const struct sockaddr_in*)SocketAddress::PrivateData::sockaddr(_privateData.get());
    return VariableLengthData(&addr->sin_addr, sizeof(addr->sin_addr));
  } else if (isInet6()) {
    const struct sockaddr_in6* addr = (const struct sockaddr_in6*)SocketAddress::PrivateData::sockaddr(_privateData.get());
    return VariableLengthData(&addr->sin6_addr, sizeof(addr->sin6_addr));
  } else {
    return VariableLengthData();
  }
}

VariableLengthData
SocketAddress::getNetworkPortData() const
{
  if (isInet4()) {
    const struct sockaddr_in* addr = (const struct sockaddr_in*)SocketAddress::PrivateData::sockaddr(_privateData.get());
    return VariableLengthData(&addr->sin_port, sizeof(addr->sin_port));
  } else if (isInet6()) {
    const struct sockaddr_in6* addr = (const struct sockaddr_in6*)SocketAddress::PrivateData::sockaddr(_privateData.get());
    return VariableLengthData(&addr->sin6_port, sizeof(addr->sin6_port));
  } else {
    return VariableLengthData();
  }
}

SocketAddress
SocketAddress::fromInet4Network(const SocketAddress& socketAddress, const VariableLengthData& networkAddressData, const VariableLengthData& networkPortData)
{
  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;

  if (networkAddressData.size() != sizeof(addr.sin_addr))
    throw TransportError("Invalid INET4 network address!");
  std::memcpy(&addr.sin_addr, networkAddressData.data(), sizeof(addr.sin_addr));

  if (networkPortData.size() != sizeof(addr.sin_port))
    throw TransportError("Invalid INET4 network port!");
  std::memcpy(&addr.sin_port, networkPortData.data(), sizeof(addr.sin_port));

  return SocketAddress(new PrivateData((const struct sockaddr*)&addr, sizeof(addr)));
}

SocketAddress
SocketAddress::fromInet6Network(const SocketAddress& socketAddress, const VariableLengthData& networkAddressData, const VariableLengthData& networkPortData)
{
  struct sockaddr_in6 addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  if (socketAddress.isInet6()) {
    const struct sockaddr_in6* addr1 = (const struct sockaddr_in6*)PrivateData::sockaddr(socketAddress._privateData.get());
    addr.sin6_scope_id = addr1->sin6_scope_id;
  }

  if (networkAddressData.size() != sizeof(addr.sin6_addr))
    throw TransportError("Invalid INET6 network address!");
  std::memcpy(&addr.sin6_addr, networkAddressData.data(), sizeof(addr.sin6_addr));

  if (networkPortData.size() != sizeof(addr.sin6_port))
    throw TransportError("Invalid INET6 network port!");
  std::memcpy(&addr.sin6_port, networkPortData.data(), sizeof(addr.sin6_port));

  return SocketAddress(new PrivateData((const struct sockaddr*)&addr, sizeof(addr)));
}

SocketAddress::SocketAddress(PrivateData* privateData) :
  _privateData(privateData)
{
}

int
SocketAddress::cmp(const SocketAddress& socketAddress) const
{
  // Both refer to the same data, must be equal
  if (_privateData == socketAddress._privateData)
    return 0;
  socklen_t len1 = PrivateData::addrlen(_privateData.get());
  socklen_t len2 = PrivateData::addrlen(socketAddress._privateData.get());
  const struct sockaddr* addr1 = PrivateData::sockaddr(_privateData.get());
  const struct sockaddr* addr2 = PrivateData::sockaddr(socketAddress._privateData.get());
  if (len1 == len2) {
    if (len1 == 0)
      return 0;
    return std::memcmp(addr1, addr2, len1);
  } else if (len1 < len2) {
    if (len1 == 0)
      return -1;
    int ret = std::memcmp(addr1, addr2, len1);
    if (ret)
      return ret;
    return -1;
  } else {
    if (len2 == 0)
      return 1;
    int ret = std::memcmp(addr1, addr2, len2);
    if (ret)
      return ret;
    return 1;
  }
}

SocketAddress::PrivateData*
SocketAddress::data()
{
  if (PrivateData::count(_privateData.get()) == 1)
    return _privateData.get();
  _privateData = new PrivateData;
  return _privateData.get();
}

const SocketAddress::PrivateData*
SocketAddress::constData() const
{
  return _privateData.get();
}

}
