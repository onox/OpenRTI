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
  NetworkServer(const SharedPtr<AbstractServerNode>& serverNode);
  virtual ~NetworkServer();

  virtual int exec();

  virtual bool getDone() const;

protected:
  virtual void _sendDone(bool done);
  virtual void _sendWakeUp();

  virtual void _postMessage(const _MessageConnectHandlePair& messageConnectHandlePair);
  virtual void _postOperation(const SharedPtr<_Operation>& operation);

// private: FIXME
  SocketEventDispatcher _dispatcher;

private:
  NetworkServer(const NetworkServer&);
  NetworkServer& operator=(const NetworkServer&);

  bool _done;
  bool _wakeUp;

  Mutex _mutex;
  _Queue _queue;
  _MessageConnectHandlePairList _pool;
};

} // namespace OpenRTI

#endif
