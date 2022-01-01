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

#ifndef OpenRTI_NetworkServer_h
#define OpenRTI_NetworkServer_h

#include "AbstractServer.h"
#include "Mutex.h"
#include "SocketAddress.h"
#include "SocketEventDispatcher.h"

namespace OpenRTI {

class URL;

class OPENRTI_API NetworkServer : public AbstractServer {
public:
  NetworkServer();
  NetworkServer(const SharedPtr<AbstractServerNode>& serverNode);
  virtual ~NetworkServer();

  void setServerName(const std::string& name);

  void setUpFromConfig(const std::string& config);
  void setUpFromConfig(std::istream& stream);

  void listen(const URL& url, int backlog);
  void listenInet(const std::string& node, const std::string& service, int backlog);
  SocketAddress listenInet(const SocketAddress& socketAddress, int backlog);
  void listenPipe(const std::string& address, int backlog);

  void connectParentServer(const URL& url, const Clock& abstime);
  void connectParentInetServer(const std::string& host, const std::string& service, bool compress, const Clock& abstime);
  void connectParentInetServer(const SocketAddress& socketAddress, bool compress, const Clock& abstime);
  void connectParentPipeServer(const std::string& name, const Clock& abstime);
  void connectParentStreamServer(const SharedPtr<SocketStream>& socketStream, const Clock& abstime, bool compress);

  virtual int exec();

protected:
  virtual void _sendDone(bool done);

  virtual void _postMessage(const _MessageConnectHandlePair& messageConnectHandlePair);
  virtual void _postOperation(const SharedPtr<_Operation>& operation);

private:
  NetworkServer(const NetworkServer&);
  NetworkServer& operator=(const NetworkServer&);

  SocketEventDispatcher _dispatcher;

  Mutex _mutex;
  _Queue _queue;
  _MessageConnectHandlePairList _pool;
};

} // namespace OpenRTI

#endif
