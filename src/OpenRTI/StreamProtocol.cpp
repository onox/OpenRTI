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
#include "MessageQueue.h"
#include "Server.h"
#include "StringUtils.h"
#include "ThreadRegistry.h"

namespace OpenRTI {

class OPENRTI_LOCAL ServerThreadRegistry : public ThreadRegistry {
public:
  class OPENRTI_LOCAL ServerThread : public NamedThread {
  public:
    ServerThread(ThreadRegistry* registry, const std::string& name) :
      NamedThread(registry, name)
    { }

    virtual void wakeUp()
    {
      _server.setDone(true);
    }
    virtual void exec()
    {
      _server.setDone(false);
      _server.exec();
    }

    void connectParentInetServer(const std::string& name, const Clock& abstime)
    {
      _server.setServerName("INET leaf server");
      _server.connectParentInetServer(name, abstime);
    }

    void connectParentPipeServer(const std::string& name, const Clock& abstime)
    {
      _server.setServerName("PIPE leaf server");
      _server.connectParentPipeServer(name, abstime);
    }

    SharedPtr<AbstractMessageSender> connectServer(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions)
    { return _server.connectServer(messageSender, clientOptions); }
    bool disconnectServer(const SharedPtr<AbstractMessageSender>& messageSender)
    {
      messageSender->close();
      // FIXME what about just an atomic count on the number of connects to this registry???
      // Once this drops to zero, shut down that thread ...
      // FIXME, need to have good semantics for that
      return !_server.getServerNode().isIdle();
    }

  private:
    Server _server;
  };

  // Connect stuff
  class OPENRTI_LOCAL ConnectServerCallback : public ThreadProcedureCallback {
  public:
    ConnectServerCallback(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions) :
      _messageSender(messageSender),
      _clientOptions(clientOptions)
    { }
    virtual void exec(ThreadRegistry& threadRegistry, NamedThread& thread)
    {
      _messageSender = static_cast<ServerThread&>(thread).connectServer(_messageSender, _clientOptions);
    }
    SharedPtr<AbstractMessageSender> _messageSender;
    StringStringListMap _clientOptions;
  };

  // Sync call to connect to a server
  SharedPtr<AbstractMessageSender> connectServer(const std::string& name, const StringStringListMap& clientOptions,
                                                 const SharedPtr<AbstractMessageSender>& fromServerQueue, const Clock& abstime)
  {
    SharedPtr<ConnectServerCallback> callback = new ConnectServerCallback(fromServerQueue, clientOptions);
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
    virtual void exec(ThreadRegistry& threadRegistry, NamedThread& thread)
    {
      bool hasChildConnects = static_cast<ServerThread&>(thread).disconnectServer(_messageSender);
      if (!hasChildConnects)
        thread.stopThread(threadRegistry);
    }
    SharedPtr<AbstractMessageSender> _messageSender;
  };

  // Sync call to connect to a server
  void disconnectServer(const std::string& name, SharedPtr<AbstractMessageSender> toServerQueue)
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
  virtual SharedPtr<NamedThread> createNewThread(const std::string& name)
  {
    SharedPtr<ServerThread> serverThread = new ServerThread(this, name);
    serverThread->connectParentInetServer(name, _abstime);
    return serverThread;
  }
};

class OPENRTI_LOCAL PipeServerThreadRegistry : public ServerThreadRegistry {
public:
  // Factory to create new threads
  virtual SharedPtr<NamedThread> createNewThread(const std::string& name)
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
          const std::string& serverThreadKey) :
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
  std::string _serverThreadKey;
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
StreamProtocol::connect(const StringStringListMap& clientOptions, const Clock& abstime) const
{
  std::string serverThreadKey;
  StringStringListMap::const_iterator i = clientOptions.find("address");
  if (i != clientOptions.end() && !i->second.empty())
    serverThreadKey = i->second.front();

  SharedPtr<ThreadMessageQueue> threadMessageQueue = new ThreadMessageQueue;
  SharedPtr<AbstractMessageSender> messageSender;
  messageSender = _serverThreadRegistry->connectServer(serverThreadKey, clientOptions, threadMessageQueue->getMessageSender(), abstime);
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
