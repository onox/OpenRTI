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

#ifndef OpenRTI_SocketAddressPrivateDataPosix_h
#define OpenRTI_SocketAddressPrivateDataPosix_h

#include "SocketAddress.h"

#include <netdb.h>
#include <cstring>

namespace OpenRTI {

struct SocketAddress::PrivateData {
  PrivateData() :
    _addr(0), _addrlen(0)
  { }
  PrivateData(const struct sockaddr* addr, const socklen_t addrlen) :
    _addr(0), _addrlen(addrlen)
  {
    if (addrlen) {
      _addr = static_cast<struct sockaddr*>(::operator new(addrlen));
      memcpy(_addr, addr, addrlen);
    }
  }
  PrivateData(const PrivateData& privateData) :
    _addr(0), _addrlen(privateData._addrlen)
  {
    if (_addrlen) {
      _addr = static_cast<struct sockaddr*>(::operator new(_addrlen));
      memcpy(_addr, privateData._addr, _addrlen);
    }
  }
  ~PrivateData()
  {
    ::operator delete(_addr);
    _addr = 0;
  }

  PrivateData& operator=(const PrivateData& privateData)
  {
    assignSocketAddress(privateData._addr, privateData._addrlen);
    return *this;
  }

  void assignSocketAddress(const struct sockaddr* addr, const socklen_t addrlen)
  {
    if (_addrlen) {
      ::operator delete(_addr);
      _addr = 0;
    }
    _addrlen = addrlen;
    if (_addrlen) {
      _addr = static_cast<struct sockaddr*>(::operator new(addrlen));
      memcpy(_addr, addr, addrlen);
    }
  }

  const char* begin() const
  { return reinterpret_cast<const char*>(_addr); }
  const char* end() const
  { return reinterpret_cast<const char*>(_addr) + _addrlen; }

  struct sockaddr* _addr;
  socklen_t _addrlen;
};

}

#endif
