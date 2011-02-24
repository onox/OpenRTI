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

#include "Clock.h"
#include "SocketEventDispatcher.h"
#include "SocketTCP.h"

namespace OpenRTI {

class Clock;
class MessageServer;
class SocketWakeupTrigger;

class OPENRTI_API Server {
public:
  Server();
  ~Server();

  const std::wstring& getServerName() const;
  void setServerName(const std::wstring& name);

  void listenInet(const std::wstring& address, int backlog);
  void listenPipe(const std::wstring& address, int backlog);

  SharedPtr<SocketTCP> connectedTCPSocket(const std::wstring& name);

  void connectParentInetServer(const std::wstring& name, const Clock& abstime);
  void connectParentPipeServer(const std::wstring& name, const Clock& abstime);
  void connectParentStreamServer(const SharedPtr<SocketStream>& socketStream, const Clock& abstime);

  void setDone(bool done);
  bool getDone() const;

  int exec();
  int exec(const Clock& abstime);

private:
  class WakeupSocketEvent;

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
