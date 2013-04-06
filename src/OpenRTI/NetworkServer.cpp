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

#include "NetworkServer.h"

#include "ScopeLock.h"
#include "ScopeUnlock.h"

namespace OpenRTI {

NetworkServer::NetworkServer(const SharedPtr<AbstractServerNode>& serverNode) :
  AbstractServer(serverNode),
  _done(false),
  _wakeUp(false)
{
}

NetworkServer::~NetworkServer()
{
  _queue.send(*this);
}

int
NetworkServer::exec()
{
  ScopeLock scopeLock(_mutex);
  _wakeUp = false;
  while (!_done && !_wakeUp) {
    if (_queue.empty()) {
      ScopeUnlock scopeUnlock(_mutex);

      _dispatcher.exec();

    } else {

      // Get pending messages.
      _Queue queue;
      _queue.swap(queue);

      {
        // Unlock the ingoing message queue as long
        // as the server is processing.
        ScopeUnlock scopeUnlock(_mutex);

        // now send to the messages to the server
        queue.send(*this);
      }

      // For recycling the queue entries, we need the lock again
      queue.recycle(_pool);
    }
  }

  return EXIT_SUCCESS;
}

bool
NetworkServer::getDone() const
{
  return _done;
}

void
NetworkServer::_sendDone(bool done)
{
  _done = done;
  _dispatcher.setDone(done);
}

void
NetworkServer::_sendWakeUp()
{
  _wakeUp = true;
}

void
NetworkServer::_postMessage(const _MessageConnectHandlePair& messageConnectHandlePair)
{
  ScopeLock scopeLock(_mutex);
  bool empty = _queue.empty();
  _queue.push_back(messageConnectHandlePair, _pool);
  // If the list already contains some unsent messages,
  // the socket event dispatcher is already informed and will also catch this new message.
  if (empty)
    _dispatcher.wakeUp();
}

void
NetworkServer::_postOperation(const SharedPtr<_Operation>& operation)
{
  ScopeLock scopeLock(_mutex);
  bool empty = _queue.empty();
  _queue.push_back(operation, _pool);
  // If the list already contains some unsent messages,
  // the socket event dispatcher is already informed and will also catch this new message.
  if (empty)
    _dispatcher.wakeUp();
}

} // namespace OpenRTI
