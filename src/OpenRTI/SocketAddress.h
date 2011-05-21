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

#ifndef OpenRTI_SocketAddress_h
#define OpenRTI_SocketAddress_h

#include <list>
#include <string>

namespace OpenRTI {

class SocketServerTCP;
class SocketTCP;

class SocketAddress {
public:
  SocketAddress();
  SocketAddress(const SocketAddress& socketAddress);
  ~SocketAddress();

  SocketAddress& operator=(const SocketAddress& socketAddress);

  bool operator<(const SocketAddress& socketAddress) const;
  bool operator>(const SocketAddress& socketAddress) const
  { return socketAddress.operator<(*this); }
  bool operator<=(const SocketAddress& socketAddress) const
  { return !socketAddress.operator<(*this); }
  bool operator>=(const SocketAddress& socketAddress) const
  { return !operator<(socketAddress); }
  bool operator==(const SocketAddress& socketAddress) const;
  bool operator!=(const SocketAddress& socketAddress) const
  { return !operator==(socketAddress); }

  bool valid() const;
  bool isInet4() const;
  bool isInet6() const;

  static std::list<SocketAddress>
  resolve(const std::string& address, const std::string& service, bool passive);

private:
  struct PrivateData;

  SocketAddress(PrivateData* privateData);

  PrivateData* _privateData;

  friend class SocketServerTCP;
  friend class SocketTCP;
};

}

#endif
