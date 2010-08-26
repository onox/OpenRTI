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

#ifndef OpenRTI_AbstractMessageQueue_h
#define OpenRTI_AbstractMessageQueue_h

#include "AbstractMessage.h"
#include "AbstractMessageReceiver.h"
#include "AbstractMessageSender.h"
#include "Atomic.h"
#include "SharedPtr.h"
// FIXME make implementation file
#include "Exception.h"

namespace OpenRTI {

/// FIXME may be here the double derived one???
/// Ok, no double derived stuff here. It is just not clear which in/out pair to use.
/// In this case it would be the send receive ends of the same message directions which is
/// Not the same that one would expect when thinking at iostream.
/// So, no - no double derived stuff!
class OPENRTI_LOCAL AbstractMessageQueue : public AbstractMessageReceiver {
public:
  virtual ~AbstractMessageQueue() {}
  virtual SharedPtr<AbstractMessage> receive(const Clock& timeout) = 0;

  SharedPtr<AbstractMessageSender> getMessageSender()
  { return new MessageSender(this); }

protected:
  // FIXME may be only have const messages in delivery???
  virtual void append(const SharedPtr<AbstractMessage>& message) = 0;

  // bool canRecieve() const
  // { return 0 < _numMessageSenders; }

private:
  class OPENRTI_LOCAL MessageSender : public AbstractMessageSender {
  public:
    MessageSender(AbstractMessageQueue* messageQueue) : _messageQueue(messageQueue)
    {
      // ++messageQueue->_numMessageSenders;
    }
    virtual ~MessageSender()
    {
      close();
    }
    virtual void send(const SharedPtr<AbstractMessage>& message)
    {
      if (!_messageQueue.valid())
        throw RTIinternalError(L"Trying to send message to a closed MessageSender");
      _messageQueue->append(message);
    }
    virtual void close()
    {
      if (!_messageQueue.valid())
        return;
      // --messageQueue->_numMessageSenders;
      _messageQueue = 0;
    }
  private:
    SharedPtr<AbstractMessageQueue> _messageQueue;
  };

  // Atomic _numMessageSenders;
};

} // namespace OpenRTI

#endif
