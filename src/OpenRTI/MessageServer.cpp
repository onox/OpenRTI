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

#include "MessageServer.h"

#include "Exception.h"
#include "Handle.h"

namespace OpenRTI {

class OPENRTI_LOCAL MessageServer::MessageSender : public AbstractMessageSender {
public:
  MessageSender(const SharedPtr<MessageServer>& messageServer, const ConnectHandle& connectHandle) :
    _messageServer(messageServer),
    _connectHandle(connectHandle)
    { }
  virtual ~MessageSender()
  { close(); }
  virtual void send(const SharedPtr<AbstractMessage>& message)
  {
    if (!_messageServer.valid())
      throw RTIinternalError(L"Trying to send message to a closed MessageSender");
    if (!message.valid())
      return;
    _messageServer->dispatchMessage(*message, _connectHandle);
  }
  virtual void close()
  {
    if (!_messageServer.valid())
      return;
    _messageServer->removeConnect(_connectHandle);
    _messageServer = 0;
    _connectHandle = ConnectHandle();
  }

private:
  SharedPtr<MessageServer> _messageServer;
  ConnectHandle _connectHandle;
};

MessageServer::MessageServer(const SharedPtr<ServerOptions>& serverOptions) :
  ServerNode(serverOptions)
{
}

MessageServer::~MessageServer()
{
}

SharedPtr<AbstractMessageSender>
MessageServer::insertConnect(const SharedPtr<AbstractMessageSender>& messageSender)
{
  ConnectHandle connectHandle = ServerNode::insertConnect(messageSender);
  if (!connectHandle.valid())
    return 0;
  return new MessageSender(this, connectHandle);
}

SharedPtr<AbstractMessageSender>
MessageServer::insertParentConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& parentOptions)
{
  ConnectHandle connectHandle = ServerNode::insertParentConnect(messageSender, parentOptions);
  if (!connectHandle.valid())
    return 0;
  return new MessageSender(this, connectHandle);
}

} // namespace OpenRTI
