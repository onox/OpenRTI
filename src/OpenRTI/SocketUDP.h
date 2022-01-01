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

#ifndef OpenRTI_SocketUDP_h
#define OpenRTI_SocketUDP_h

#include "SocketPacket.h"

namespace OpenRTI {

class SocketUDP : public SocketPacket {
public:
  SocketUDP();
  // SocketUDP(unsigned short localPort);
  // SocketUDP(const std::string &localAddress, unsigned short localPort);
  // void disconnect();
  // void sendTo(const void *buffer, int bufferLen, const std::string &foreignAddress,
  //           unsigned short foreignPort);
  // int recvFrom(void *buffer, int bufferLen, std::string &sourceAddress, 
  //              unsigned short &sourcePort);
  // void setMulticastTTL(unsigned char multicastTTL);
  // void joinGroup(const std::string &multicastGroup);
  // void leaveGroup(const std::string &multicastGroup);

// private:
  // void setBroadcast();

protected:
  virtual ~SocketUDP();
};

} // namespace OpenRTI

#endif
