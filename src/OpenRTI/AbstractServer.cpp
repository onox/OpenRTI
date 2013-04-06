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

#include "AbstractServer.h"

#include "Exception.h"
#include "Condition.h"
#include "MessageQueue.h"
#include "Mutex.h"
#include "ScopeLock.h"

namespace OpenRTI {

AbstractServer::_Operation::~_Operation()
{
}

void
AbstractServer::_Queue::swap(_Queue& queue)
{
  _messageConnectHandlePairList.swap(queue._messageConnectHandlePairList);
  _operationList.swap(queue._operationList);
}

bool
AbstractServer::_Queue::empty() const
{
  return _messageConnectHandlePairList.empty();
}

void
AbstractServer::_Queue::send(AbstractServer& serverLoop)
{
  for (_MessageConnectHandlePairList::iterator i = _messageConnectHandlePairList.begin();
       i != _messageConnectHandlePairList.end(); ++i) {
    if (i->first.valid()) {
      serverLoop._sendMessage(*i);
      i->first.clear();
    } else {
      serverLoop._sendOperation(*_operationList.front());
      _operationList.pop_front();
    }
  }
}

void
AbstractServer::_Queue::push_back(const _MessageConnectHandlePair& messageConnectHandlePair, _MessageConnectHandlePairList& pool)
{
  if (pool.empty()) {
    _messageConnectHandlePairList.push_back(messageConnectHandlePair);
  } else {
    _messageConnectHandlePairList.splice(_messageConnectHandlePairList.end(), pool, pool.begin());
    _messageConnectHandlePairList.back() = messageConnectHandlePair;
  }
}

void
AbstractServer::_Queue::push_back(const SharedPtr<_Operation>& operation, _MessageConnectHandlePairList& pool)
{
  push_back(_MessageConnectHandlePair(), pool);
  _operationList.push_back(operation);
}

void
AbstractServer::_Queue::recycle(_MessageConnectHandlePairList& pool)
{
  pool.splice(pool.begin(), _messageConnectHandlePairList);
}

class OPENRTI_LOCAL AbstractServer::_Connect : public AbstractConnect {
public:
  _Connect(const SharedPtr<AbstractMessageSender>& messageSender,
           const SharedPtr<AbstractMessageReceiver>& messageReceiver);
  virtual ~_Connect();

  virtual AbstractMessageSender* getMessageSender();
  virtual AbstractMessageReceiver* getMessageReceiver();

private:
  SharedPtr<AbstractMessageSender> _messageSender;
  SharedPtr<AbstractMessageReceiver> _messageReceiver;
};

AbstractServer::_Connect::_Connect(const SharedPtr<AbstractMessageSender>& messageSender,
                                   const SharedPtr<AbstractMessageReceiver>& messageReceiver) :
  _messageSender(messageSender),
  _messageReceiver(messageReceiver)
{
}

AbstractServer::_Connect::~_Connect()
{
  if (_messageSender.valid())
    _messageSender->close();
  _messageSender.clear();
  _messageReceiver.clear();
}

AbstractMessageSender*
AbstractServer::_Connect::getMessageSender()
{
  return _messageSender.get();
}

AbstractMessageReceiver*
AbstractServer::_Connect::getMessageReceiver()
{
  return _messageReceiver.get();
}

class OPENRTI_LOCAL AbstractServer::_ConnectOperation : public AbstractServer::_Operation {
public:
  _ConnectOperation(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions);
  virtual ~_ConnectOperation();

  virtual void operator()(AbstractServer& serverLoop);
  void wait();
  const ConnectHandle& getConnectHandle() const;

private:
  SharedPtr<AbstractMessageSender> _messageSender;
  StringStringListMap _clientOptions;

  Mutex _mutex;
  Condition _condition;
  bool _done;

  ConnectHandle _connectHandle;
};

AbstractServer::_ConnectOperation::_ConnectOperation(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions) :
  _messageSender(messageSender),
  _clientOptions(clientOptions),
  _done(false)
{
}

AbstractServer::_ConnectOperation::~_ConnectOperation()
{
}

void
AbstractServer::_ConnectOperation::operator()(AbstractServer& serverLoop)
{
  _connectHandle = serverLoop._sendConnect(_messageSender, _clientOptions);

  ScopeLock scopeLock(_mutex);
  _done = true;
  _condition.signal();
}

void
AbstractServer::_ConnectOperation::wait()
{
  ScopeLock scopeLock(_mutex);
  while (!_done)
    _condition.wait(_mutex);
}

const ConnectHandle&
AbstractServer::_ConnectOperation::getConnectHandle() const
{
  return _connectHandle;
}

class OPENRTI_LOCAL AbstractServer::_DisconnectOperation : public AbstractServer::_Operation {
public:
  _DisconnectOperation(const ConnectHandle& connectHandle);
  virtual ~_DisconnectOperation();
  virtual void operator()(AbstractServer& serverLoop);

private:
  ConnectHandle _connectHandle;
};

AbstractServer::_DisconnectOperation::_DisconnectOperation(const ConnectHandle& connectHandle) :
  _connectHandle(connectHandle)
{
}

AbstractServer::_DisconnectOperation::~_DisconnectOperation()
{
}

void
AbstractServer::_DisconnectOperation::operator()(AbstractServer& serverLoop)
{
  serverLoop._eraseConnect(_connectHandle);
  if (serverLoop.getServerNode().isIdle())
    serverLoop.setDone(true);
}

class OPENRTI_LOCAL AbstractServer::_DoneOperation : public AbstractServer::_Operation {
public:
  _DoneOperation();
  virtual ~_DoneOperation();
  virtual void operator()(AbstractServer& serverLoop);
};

AbstractServer::_DoneOperation::_DoneOperation()
{
}

AbstractServer::_DoneOperation::~_DoneOperation()
{
}

void
AbstractServer::_DoneOperation::operator()(AbstractServer& serverLoop)
{
  serverLoop._sendDone(true);
}

class OPENRTI_LOCAL AbstractServer::_MessageSender : public AbstractMessageSender {
public:
  _MessageSender(const ConnectHandle& connectHandle, const SharedPtr<AbstractServer>& serverLoop);
  virtual ~_MessageSender();

  virtual void send(const SharedPtr<const AbstractMessage>& message);
  virtual void close();

private:
  ConnectHandle _connectHandle;
  SharedPtr<AbstractServer> _serverLoop;
};

AbstractServer::_MessageSender::_MessageSender(const ConnectHandle& connectHandle, const SharedPtr<AbstractServer>& serverLoop) :
  _connectHandle(connectHandle),
  _serverLoop(serverLoop)
{
  OpenRTIAssert(serverLoop.valid());
}

AbstractServer::_MessageSender::~_MessageSender()
{
}

void
AbstractServer::_MessageSender::send(const SharedPtr<const AbstractMessage>& message)
{
  if (!_connectHandle.valid())
    return;
  _serverLoop->_postMessage(_MessageConnectHandlePair(message, _connectHandle));
}

void
AbstractServer::_MessageSender::close()
{
  if (!_connectHandle.valid())
    return;
  SharedPtr<_DisconnectOperation> disconnectOperation;
  disconnectOperation = new _DisconnectOperation(_connectHandle);
  _serverLoop->_postOperation(disconnectOperation);
  _connectHandle = ConnectHandle();
}

AbstractServer::AbstractServer(const SharedPtr<AbstractServerNode>& serverNode) :
  _serverNode(serverNode),
  _done(false)
{
  OpenRTIAssert(_serverNode.valid());
}

AbstractServer::~AbstractServer()
{
}

const AbstractServerNode&
AbstractServer::getServerNode() const
{
  return *_serverNode;
}

AbstractServerNode&
AbstractServer::getServerNode()
{
  return *_serverNode;
}

SharedPtr<AbstractMessageSender>
AbstractServer::connect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions)
{
  SharedPtr<_ConnectOperation> connectOperation;
  connectOperation = new _ConnectOperation(messageSender, clientOptions);
  _postOperation(connectOperation);
  connectOperation->wait();

  ConnectHandle connectHandle = connectOperation->getConnectHandle();
  if (!connectHandle.valid())
    return 0;
  return new _MessageSender(connectHandle, this);
}

SharedPtr<AbstractConnect>
AbstractServer::connect(const StringStringListMap& clientOptions)
{
  SharedPtr<ThreadMessageQueue> threadMessageQueue = new ThreadMessageQueue;
  SharedPtr<AbstractMessageSender> messageSender = connect(threadMessageQueue->getMessageSender(), clientOptions);
  if (!messageSender.valid())
    return 0;
  return new _Connect(messageSender, threadMessageQueue);
}

ConnectHandle
AbstractServer::_sendConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions)
{
  OpenRTIAssert(messageSender.valid());
  if (getDone()) {
    return ConnectHandle();
  } else {
    return getServerNode()._insertConnect(messageSender, clientOptions);
  }
}

void
AbstractServer::_eraseConnect(const ConnectHandle& connectHandle)
{
  OpenRTIAssert(connectHandle.valid());
  getServerNode()._eraseConnect(connectHandle);
}

void
AbstractServer::_sendMessage(const _MessageConnectHandlePair& messageConnectHandlePair)
{
  OpenRTIAssert(messageConnectHandlePair.first.valid());
  OpenRTIAssert(messageConnectHandlePair.second.valid());
  getServerNode()._dispatchMessage(messageConnectHandlePair.first.get(), messageConnectHandlePair.second);
}

void
AbstractServer::_sendOperation(_Operation& operation)
{
  operation(*this);
}

void
AbstractServer::_sendDone(bool done)
{
  _done = done;
}

void
AbstractServer::_postDone()
{
  _postOperation(new _DoneOperation());
}

} // namespace OpenRTI
