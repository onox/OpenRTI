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

#ifndef OpenRTI_Server_h
#define OpenRTI_Server_h

#include <iosfwd>

#include "AbstractNetworkServer.h"
#include "Clock.h"
#include "SocketEventDispatcher.h"
#include "SocketTCP.h"
#include "StringUtils.h"

namespace OpenRTI {

class AbstractMessageSender;
class Clock;
class MessageServer;
class SocketWakeupTrigger;

class OPENRTI_API Server : public AbstractNetworkServer {
public:
  Server();
  ~Server();

  const std::string& getServerName() const;
  void setServerName(const std::string& name);

  void setUpFromConfig(const std::string& config);
  void setUpFromConfig(std::istream& stream);

  void listen(const std::string& url, int backlog);
  void listenInet(const std::string& address, int backlog);
  void listenInet(const std::string& node, const std::string& service, int backlog);
  void listenPipe(const std::string& address, int backlog);

  SharedPtr<SocketTCP> connectedTCPSocket(const std::string& name);

  void connectParentServer(const std::string& url, const Clock& abstime);
  void connectParentInetServer(const std::string& name, const Clock& abstime);
  void connectParentPipeServer(const std::string& name, const Clock& abstime);
  void connectParentStreamServer(const SharedPtr<SocketStream>& socketStream, const Clock& abstime);

  SharedPtr<AbstractMessageSender> connectServer(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions);

  /// Gives access to the rti server node running in this NetworkServer.
  /// The method is guaranteed to return a valid ServerNode.
  virtual AbstractServerNode& getServerNode();

  /// Stops the NetworkServers exec loop.
  /// Must be thread safe as it might be called from a different thread
  /// than the one running the exec loop.
  virtual void setDone();
  void setDone(bool done);
  bool getDone() const;

  /// Run the server's main message loop.
  virtual int exec();

private:
  class WakeupSocketEvent;
  class TriggeredConnectSocketEvent;

  Server(const Server&);
  Server& operator=(const Server&);

  // The rti server itself
  SharedPtr<MessageServer> _messageServer;

  // The socket dispatcher
  SocketEventDispatcher _dispatcher;

  // This is used to wake up the dispatcher waiting in the select call.
  SharedPtr<SocketWakeupTrigger> _socketWakeupTrigger;
};

} // namespace OpenRTI

#endif
