/* -*-c++-*- OpenRTI - Copyright (C) 2009-2010 Mathias Froehlich
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

#ifndef OpenRTI_MessageQueue_h
#define OpenRTI_MessageQueue_h

#include "AbstractMessageQueue.h"
#include "Condition.h"
#include "MessageList.h"
#include "Mutex.h"
#include "ScopeLock.h"

namespace OpenRTI {

// Unlocked queue implementation,
// For single threaded use.
// The timeout in the receive call is just ignored since
// single threaded use does not have any chance to fill the queue
// if that single thread is waiting for exactly that queue.
class OPENRTI_LOCAL LocalMessageQueue : public AbstractMessageQueue {
public:
  virtual SharedPtr<AbstractMessage> receive(const Clock&)
  { return _messageList.pop_front(); }

protected:
  virtual void append(const SharedPtr<AbstractMessage>& message)
  { _messageList.push_back(message); }

private:
  MessageList _messageList;
};

// Thread safe queue with condition/mutex based signaling of new messages
class OPENRTI_LOCAL ThreadMessageQueue : public AbstractMessageQueue {
public:
  virtual SharedPtr<AbstractMessage> receive(const Clock& timeout)
  {
    ScopeLock scopeLock(_mutex);
    while (_messageList.empty()) {
      // On timeout, the list must be empty, so return 0
      if (!_condition.wait(_mutex, timeout))
        return 0;
    }
    return _messageList.pop_front();
  }

protected:
  virtual void append(const SharedPtr<AbstractMessage>& message)
  {
    ScopeLock scopeLock(_mutex);
    bool needSignal = _messageList.empty();
    _messageList.push_back(message);
    if (needSignal)
      _condition.signal();
  }

private:
  Mutex _mutex;
  Condition _condition;
  MessageList _messageList;
};

} // namespace OpenRTI

#endif
