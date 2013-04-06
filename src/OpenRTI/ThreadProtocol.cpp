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

#include "ThreadProtocol.h"

#include "Handle.h"
#include "MessageQueue.h"
#include "Mutex.h"
#include "ServerNode.h"
#include "ServerOptions.h"
#include "ScopeLock.h"

namespace OpenRTI {

// Basically a wrapper around a ServerNode with a mutex to protect against
// concurrent access. Note that only little communication should go through the server itself.
// The fast path of the uptates and interactions should go through direct ambassador to ambassador connects.
class OPENRTI_LOCAL ThreadProtocol::ThreadProtocolServer : public ServerNode {
public:
  ThreadProtocolServer() :
    ServerNode(new ServerOptions)
  { }

  SharedPtr<AbstractMessageSender> insertConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions)
  {
    ScopeLock scopeLock(_mutex);
    ConnectHandle connectHandle = _insertConnect(messageSender, clientOptions);
    if (!connectHandle.valid())
      return 0;
    return new MessageSender(this, connectHandle);
  }

private:
  void removeConnectLocked(const ConnectHandle& connectHandle)
  {
    ScopeLock scopeLock(_mutex);
    _eraseConnect(connectHandle);
  }
  void dispatchMessageLocked(const AbstractMessage* message, const ConnectHandle& connectHandle)
  {
    ScopeLock scopeLock(_mutex);
    _dispatchMessage(message, connectHandle);
  }

  /// MessageSender implementation that calls a MessageDispatcher
  /// implementation below. Message dispatching is done syncronously.
  class MessageSender : public AbstractMessageSender {
  public:
    MessageSender(const SharedPtr<ThreadProtocolServer>& threadProtocolServer, const ConnectHandle& connectHandle) :
      _threadProtocolServer(threadProtocolServer),
      _connectHandle(connectHandle)
    { }
    virtual ~MessageSender()
    { close(); }
    virtual void send(const SharedPtr<const AbstractMessage>& message)
    {
      if (!_threadProtocolServer.valid())
        throw RTIinternalError("Trying to send message to a closed MessageSender");
      if (!message.valid())
        return;
      _threadProtocolServer->dispatchMessageLocked(message.get(), _connectHandle);
    }
    virtual void close()
    {
      if (!_threadProtocolServer.valid())
        return;
      _threadProtocolServer->removeConnectLocked(_connectHandle);
      _threadProtocolServer = 0;
      _connectHandle = ConnectHandle();
    }

  private:
    SharedPtr<ThreadProtocolServer> _threadProtocolServer;
    ConnectHandle _connectHandle;
  };

  // Mutex to protect the server.
  // Note that this server class runs otherwise in the server binary where it runs in its own
  // thread and does not need locking. So consequently provide that locks in this abstraction level.
  // The mutex is held for every message that passes the server. That appears to be heavyweight for the first cut.
  // But thinking about it, most of the messages are directly passed to the other federates queues, so this lock is only
  // held for a few messages that really need to pass into the server.
  Mutex _mutex;
};

class OPENRTI_LOCAL ThreadProtocol::Connect : public AbstractConnect {
public:
  Connect(const SharedPtr<AbstractMessageSender>& messageSender,
          const SharedPtr<AbstractMessageReceiver>& messageReceiver) :
    _messageSender(messageSender),
    _messageReceiver(messageReceiver)
  { }
  virtual AbstractMessageSender* getMessageSender()
  { return _messageSender.get(); }
  virtual AbstractMessageReceiver* getMessageReceiver()
  { return _messageReceiver.get(); }

private:
  SharedPtr<AbstractMessageSender> _messageSender;
  SharedPtr<AbstractMessageReceiver> _messageReceiver;
};

ThreadProtocol::ThreadProtocol() :
  _threadProtocolServer(new ThreadProtocolServer)
{
}

ThreadProtocol::~ThreadProtocol()
{
}

SharedPtr<AbstractConnect>
ThreadProtocol::connect(const StringStringListMap& clientOptions, const Clock&) const
{
  SharedPtr<ThreadMessageQueue> outgoingQueue = new ThreadMessageQueue;
  SharedPtr<AbstractMessageSender> toAmbassador = outgoingQueue->getMessageSender();
  SharedPtr<AbstractMessageSender> messageSender = _threadProtocolServer->insertConnect(toAmbassador, clientOptions);
  return new Connect(messageSender, outgoingQueue);
}

} // namespace OpenRTI
