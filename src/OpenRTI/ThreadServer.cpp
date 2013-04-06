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

#include "ThreadServer.h"

#include "ScopeLock.h"
#include "ScopeUnlock.h"

namespace OpenRTI {

ThreadServer::ThreadServer(const SharedPtr<AbstractServerNode>& serverNode) :
  AbstractServer(serverNode),
  _done(false),
  _wakeUp(false)
{
}

ThreadServer::~ThreadServer()
{
  _queue.send(*this);
}

int
ThreadServer::exec()
{
  ScopeLock scopeLock(_mutex);
  _wakeUp = false;
  while (!_wakeUp && !_done) {
    if (_queue.empty()) {
      _condition.wait(_mutex);
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
ThreadServer::getDone() const
{
  return _done;
}

void
ThreadServer::_sendDone(bool done)
{
  _done = done;
}

void
ThreadServer::_sendWakeUp()
{
  _wakeUp = true;
}

void
ThreadServer::_postMessage(const _MessageConnectHandlePair& messageConnectHandlePair)
{
  ScopeLock scopeLock(_mutex);
  bool empty = _queue.empty();
  _queue.push_back(messageConnectHandlePair, _pool);
  if (empty)
    _condition.signal();
}

void
ThreadServer::_postOperation(const SharedPtr<_Operation>& operation)
{
  ScopeLock scopeLock(_mutex);
  bool empty = _queue.empty();
  _queue.push_back(operation, _pool);
  if (empty)
    _condition.signal();
}

} // namespace OpenRTI
