/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
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

#ifndef OpenRTI_Server_h
#define OpenRTI_Server_h

#include <iosfwd>

#include "NetworkServer.h"
#include "Clock.h"
#include "SocketEventDispatcher.h"
#include "SocketTCP.h"
#include "StringUtils.h"

namespace OpenRTI {

class AbstractMessageSender;
class Clock;

class OPENRTI_API Server : public NetworkServer {
public:
  Server();
  Server(const SharedPtr<AbstractServerNode>& serverNode);
  virtual ~Server();

  void setServerName(const std::string& name);

  void setUpFromConfig(const std::string& config);
  void setUpFromConfig(std::istream& stream);

  void listen(const std::string& url, int backlog);
  void listenInet(const std::string& address, int backlog);
  void listenInet(const std::string& node, const std::string& service, int backlog);
  SocketAddress listenInet(const SocketAddress& socketAddress, int backlog);
  void listenPipe(const std::string& address, int backlog);

  SharedPtr<SocketTCP> connectedTCPSocket(const SocketAddress& socketAddress);

  void connectParentServer(const std::string& url, const Clock& abstime);
  void connectParentInetServer(const std::string& name, const Clock& abstime);
  void connectParentInetServer(const std::pair<std::string, std::string>& hostPortPair, const Clock& abstime);
  void connectParentInetServer(const SocketAddress& socketAddress, const Clock& abstime);
  void connectParentPipeServer(const std::string& name, const Clock& abstime);
  void connectParentStreamServer(const SharedPtr<SocketStream>& socketStream, const Clock& abstime, bool local);

  SharedPtr<AbstractMessageSender> insertConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& optionMap);
  SharedPtr<AbstractMessageSender> insertParentConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& optionMap);

private:
  class _ToServerMessageSender;

  Server(const Server&);
  Server& operator=(const Server&);
};

} // namespace OpenRTI

#endif
