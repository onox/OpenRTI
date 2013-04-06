/* -*-c++-*- OpenRTI - Copyright (C) 2009-2013 Mathias Froehlich
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

#ifndef OpenRTI_NetworkServer_h
#define OpenRTI_NetworkServer_h

#include "AbstractServer.h"
#include "Mutex.h"
#include "SocketAddress.h"
#include "SocketEventDispatcher.h"

namespace OpenRTI {

class OPENRTI_API NetworkServer : public AbstractServer {
public:
  NetworkServer();
  NetworkServer(const SharedPtr<AbstractServerNode>& serverNode);
  virtual ~NetworkServer();

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

  virtual int exec();

  virtual bool getDone() const;

protected:
  virtual void _sendDone(bool done);
  virtual void _sendWakeUp();

  virtual void _postMessage(const _MessageConnectHandlePair& messageConnectHandlePair);
  virtual void _postOperation(const SharedPtr<_Operation>& operation);

private:
  class _ToServerMessageSender;

  NetworkServer(const NetworkServer&);
  NetworkServer& operator=(const NetworkServer&);

  SocketEventDispatcher _dispatcher;

  bool _done;
  bool _wakeUp;

  Mutex _mutex;
  _Queue _queue;
  _MessageConnectHandlePairList _pool;
};

} // namespace OpenRTI

#endif
