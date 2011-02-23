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

#include "StreamProtocol.h"

#include "Clock.h"
#include "Message.h"
#include "MessageEncodingRegistry.h"
#include "MessageSocketReadEvent.h"
#include "MessageSocketWriteEvent.h"
#include "MessageQueue.h"
#include "MessageServer.h"
#include "SocketEventDispatcher.h"
#include "SocketPipe.h"
#include "SocketTCP.h"
#include "SocketWakeupEvent.h"
#include "SocketWakeupTrigger.h"
#include "StringUtils.h"
#include "ThreadRegistry.h"
#include "ZLibCallbacks.h"

namespace OpenRTI {

// This one is to communicate from the ambassador to the server.
class OPENRTI_LOCAL TriggeredConnectSocketEvent : public SocketReadEvent {
  class OPENRTI_LOCAL LockedMessageList : public Referenced {
  public:
    void push_back(const SharedPtr<AbstractMessage>& message)
    {
      ScopeLock scopeLock(_mutex);
      _messageList.push_back(message);
    }
    SharedPtr<AbstractMessage> pop_front()
    {
      ScopeLock scopeLock(_mutex);
      return _messageList.pop_front();
    }
  public:
    Mutex _mutex;
    MessageList _messageList;
  };

public:
  // Need to provide the server side message sender.
  TriggeredConnectSocketEvent(const SharedPtr<AbstractMessageSender>& serverMessageSender) :
    SocketReadEvent(true),
    _socketWakeupTrigger(new SocketWakeupTrigger),
    _socketWakeupEvent(_socketWakeupTrigger->connect()),
    _lockedMessageList(new LockedMessageList),
    _serverMessageSender(serverMessageSender)
  {
  }

  virtual void read(SocketEventDispatcher& dispatcher)
  {
    ssize_t ret = _socketWakeupEvent->read();
    for (;;) {
      SharedPtr<AbstractMessage> message = _lockedMessageList->pop_front();
      if (!message.valid())
        break;
      if (!_serverMessageSender.valid())
        continue;
      _serverMessageSender->send(message);
    }

    if (ret == -1) {
      // Protocol errors in any sense lead to a closed connection
      dispatcher.eraseSocket(this);
      /// Send something to the originator FIXME
    }
  }

  virtual SocketWakeupEvent* getSocket() const
  { return _socketWakeupEvent.get(); }

  // The ambassador side message sender.
  AbstractMessageSender* getAmbassadorMessageSender()
  {
    return new MessageSender(_socketWakeupTrigger, _lockedMessageList);
  }

private:
  class OPENRTI_LOCAL MessageSender : public AbstractMessageSender {
  public:
    MessageSender(const SharedPtr<SocketWakeupTrigger>& socketWakeupTrigger,
                  const SharedPtr<LockedMessageList>& lockedMessageList) :
      _socketWakeupTrigger(socketWakeupTrigger),
      _lockedMessageList(lockedMessageList)
    {
    }
    virtual ~MessageSender()
    {
      close();
    }
    virtual void send(const SharedPtr<AbstractMessage>& message)
    {
      if (!_socketWakeupTrigger.valid())
        throw RTIinternalError(L"Trying to send message to a closed MessageSender");
      _lockedMessageList->push_back(message);
      _socketWakeupTrigger->trigger();
    }
    virtual void close()
    {
      if (!_socketWakeupTrigger.valid())
        return;
      _socketWakeupTrigger->close();
      _socketWakeupTrigger = 0;
      _lockedMessageList = 0;
    }

  private:
    SharedPtr<SocketWakeupTrigger> _socketWakeupTrigger;
    SharedPtr<LockedMessageList> _lockedMessageList;
  };

  // FIXME reverse the connect direction in the trigger/event pair
  // Or think about a different api
  SharedPtr<SocketWakeupTrigger> _socketWakeupTrigger;
  SharedPtr<SocketWakeupEvent> _socketWakeupEvent;
  SharedPtr<LockedMessageList> _lockedMessageList;
  // The server side message sender. Here we need to have a server hanging.
  SharedPtr<AbstractMessageSender> _serverMessageSender;
};

class OPENRTI_LOCAL ServerThreadRegistry : public ThreadRegistry {
public:
  class OPENRTI_LOCAL ServerThread : public NamedThread {
  public:
    ServerThread(ThreadRegistry* registry, const std::wstring& name) :
      NamedThread(registry, name),
      _messageServer(new MessageServer)
    {
    }

    SharedPtr<SocketTCP> connectedTCPSocket(const std::wstring& name)
    {
      std::pair<std::wstring, std::wstring> hostPortPair;
      hostPortPair = parseInetAddress(name);

      // Note that here the may be lenghty name lookup for the connection address happens
      std::list<SocketAddress> addressList = SocketAddress::resolve(hostPortPair.first, hostPortPair.second);
      while (!addressList.empty()) {
        try {
          SharedPtr<SocketTCP> socketStream = new SocketTCP;
          socketStream->connect(addressList.front());
          return socketStream;
        } catch (const OpenRTI::Exception& e) {
          addressList.pop_front();
          if (addressList.empty())
            throw e;
        }
      }
      throw RTIinternalError(std::wstring(L"Can not resolve address: ") + name);
    }

    void connectParentInetServer(const std::wstring& name, const Clock& abstime)
    {
      connectParentStreamServer(connectedTCPSocket(name), abstime);
      _messageServer->setServerName(L"INET leaf server");
    }

    void connectParentPipeServer(const std::wstring& name, const Clock& abstime)
    {
      std::wstring path = localeToUcs(OpenRTI_DEFAULT_PIPE_PATH);
      if (!name.empty())
        path = name;

      // Try to connect to a pipe socket
      SharedPtr<SocketPipe> socketStream = new SocketPipe;
      socketStream->connect(path);

      connectParentStreamServer(socketStream, abstime);
      _messageServer->setServerName(L"PIPE leaf server");
    }

    // Creates a new server thread that is connected to a parent server through the socket stream
    void connectParentStreamServer(const SharedPtr<SocketStream>& socketStream, const Clock& abstime)
    {
      bool compress = true;
      StringStringListMap parentOptions;
      // Negotiate with the server how to encode
      MessageEncoderPair encodingPair = MessageEncodingRegistry::instance().negotiateEncoding(socketStream, abstime, compress, parentOptions);

      // The socket side message encoder and output message queue
      SharedPtr<MessageSocketWriteEvent> writeMessageSocketEvent = new MessageSocketWriteEvent(socketStream, encodingPair.first);
      /// The connection that sends messages up to the parent
      SharedPtr<AbstractMessageSender> toParentSender = writeMessageSocketEvent->getMessageSender();

      /// returns a sender where incomming messages should be sent to
      SharedPtr<AbstractMessageSender> toServerSender = _messageServer->insertParentConnect(toParentSender, parentOptions);
      if (!toServerSender.valid())
        throw RTIinternalError(L"Could not allocate connectHandle for parent server connect");

      // The socket side message parser and dispatcher that fires the above on completely
      // received messages
      SharedPtr<MessageSocketReadEvent> readMessageSocketEvent;
      readMessageSocketEvent = new MessageSocketReadEvent(socketStream, toServerSender.get(), encodingPair.second);

      if (compress) {
#ifdef OPENRTI_HAVE_ZLIB
        readMessageSocketEvent->setReceiveCallback(new ZLibReceiveCallback);
        writeMessageSocketEvent->setSendCallback(new ZLibSendCallback);
#else
        Log(MessageCoding, Warning) << "Parent server negotiated zlib compression, but client does not support that!" << std::endl;
        throw RTIinternalError(L"Parent server negotiated zlib compression, but client does not support that!");
#endif
      }

      _dispatcher.insert(readMessageSocketEvent.get());
      _dispatcher.insert(writeMessageSocketEvent.get());
    }

    SharedPtr<AbstractMessageSender> connectServer(const SharedPtr<AbstractMessageSender>& messageSender)
    {
      SharedPtr<AbstractMessageSender> serverMessageSender;
      serverMessageSender = _messageServer->insertConnect(messageSender);
      if (!serverMessageSender.valid())
        return 0;

      SharedPtr<TriggeredConnectSocketEvent> socketEvent;
      socketEvent = new TriggeredConnectSocketEvent(serverMessageSender);
      _dispatcher.insert(socketEvent.get());

      SharedPtr<AbstractMessageSender> toServerSender;
      toServerSender = socketEvent->getAmbassadorMessageSender();

      return toServerSender;
    }
    bool disconnectServer(const SharedPtr<AbstractMessageSender>& messageSender)
    {
      messageSender->close();
      // FIXME what about just an atomic count on the number of connects to this registry???
      // Once this drops to zero, shut down that thread ...
      // FIXME, need to have good semantics for that
      // return _messageServer->hasChildConnects();
      return _messageServer->isRunning();
    }

  private:
    SharedPtr<MessageServer> _messageServer;
  };

  // Connect stuff
  class OPENRTI_LOCAL ConnectServerCallback : public ThreadProcedureCallback {
  public:
    ConnectServerCallback(const SharedPtr<AbstractMessageSender>& messageSender) :
      _messageSender(messageSender)
    { }
    virtual void exec(NamedThread& thread)
    {
      _messageSender = static_cast<ServerThread&>(thread).connectServer(_messageSender);
    }
    SharedPtr<AbstractMessageSender> _messageSender;
  };

  // Sync call to connect to a server
  SharedPtr<AbstractMessageSender> connectServer(const std::wstring& name, const SharedPtr<AbstractMessageSender>& fromServerQueue,
                                                 const Clock& abstime)
  {
    SharedPtr<ConnectServerCallback> callback = new ConnectServerCallback(fromServerQueue);
    _abstime = abstime;
    if (!execThreadProcedure(name, callback, true))
      return 0;
    return callback->_messageSender;
  }

  // Disconnect stuff
  class OPENRTI_LOCAL DisconnectServerCallback : public ThreadProcedureCallback {
  public:
    DisconnectServerCallback(const SharedPtr<AbstractMessageSender>& messageSender) :
      _messageSender(messageSender)
    { }
    virtual void exec(NamedThread& thread)
    {
      bool hasChildConnects = static_cast<ServerThread&>(thread).disconnectServer(_messageSender);
      if (!hasChildConnects)
        thread.stopThread();
    }
    SharedPtr<AbstractMessageSender> _messageSender;
  };

  // Sync call to connect to a server
  void disconnectServer(const std::wstring& name, SharedPtr<AbstractMessageSender> toServerQueue)
  {
    execThreadProcedure(name, new DisconnectServerCallback(toServerQueue), false);
  }

protected:
  // The connect timeout
  Clock _abstime;
};


class OPENRTI_LOCAL InetServerThreadRegistry : public ServerThreadRegistry {
public:
  // Factory to create new threads
  virtual SharedPtr<NamedThread> createNewThread(const std::wstring& name)
  {
    SharedPtr<ServerThread> serverThread = new ServerThread(this, name);
    serverThread->connectParentInetServer(name, _abstime);
    return serverThread;
  }
};

class OPENRTI_LOCAL PipeServerThreadRegistry : public ServerThreadRegistry {
public:
  // Factory to create new threads
  virtual SharedPtr<NamedThread> createNewThread(const std::wstring& name)
  {
    SharedPtr<ServerThread> serverThread = new ServerThread(this, name);
    serverThread->connectParentPipeServer(name, _abstime);
    return serverThread;
  }
};

/// Protocol root factory for a socket stream based rti.
class OPENRTI_LOCAL StreamProtocol::Connect : public AbstractConnect {
 public:
  Connect(const SharedPtr<AbstractMessageSender>& messageSender,
          const SharedPtr<AbstractMessageReceiver>& messageReceiver,
          const SharedPtr<ServerThreadRegistry>& serverThreadRegistry,
          const std::wstring& serverThreadKey) :
    _messageSender(messageSender),
    _messageReceiver(messageReceiver),
    _serverThreadRegistry(serverThreadRegistry),
    _serverThreadKey(serverThreadKey)
    { }
  virtual ~Connect()
  {
    _serverThreadRegistry->disconnectServer(_serverThreadKey, _messageSender);
  }
  virtual AbstractMessageSender* getMessageSender()
  { return _messageSender.get(); }
  virtual AbstractMessageReceiver* getMessageReceiver()
  { return _messageReceiver.get(); }

 private:
  SharedPtr<AbstractMessageSender> _messageSender;
  SharedPtr<AbstractMessageReceiver> _messageReceiver;
  SharedPtr<ServerThreadRegistry> _serverThreadRegistry;
  std::wstring _serverThreadKey;
};

StreamProtocol::StreamProtocol(SharedPtr<ServerThreadRegistry> serverThreadRegistry) :
  _serverThreadRegistry(serverThreadRegistry)
{
}

StreamProtocol::~StreamProtocol()
{
  // FIXME May be collect here still running threads???
}

SharedPtr<AbstractConnect>
StreamProtocol::connect(const std::map<std::wstring,std::wstring>& parameterMap, const Clock& abstime) const
{
  std::wstring serverThreadKey;
  std::map<std::wstring,std::wstring>::const_iterator i = parameterMap.find(L"address");
  if (i != parameterMap.end())
    serverThreadKey = i->second;

  SharedPtr<ThreadMessageQueue> threadMessageQueue = new ThreadMessageQueue;
  SharedPtr<AbstractMessageSender> messageSender;
  messageSender = _serverThreadRegistry->connectServer(serverThreadKey, threadMessageQueue->getMessageSender(), abstime);
  return new Connect(messageSender, threadMessageQueue, _serverThreadRegistry, serverThreadKey);
}

PipeProtocol::PipeProtocol() :
  StreamProtocol(new PipeServerThreadRegistry)
{
}

PipeProtocol::~PipeProtocol()
{
}

InetProtocol::InetProtocol() :
  StreamProtocol(new InetServerThreadRegistry)
{
}

InetProtocol::~InetProtocol()
{
}

} // namespace OpenRTI
