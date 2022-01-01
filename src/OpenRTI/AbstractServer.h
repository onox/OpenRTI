/* -*-c++-*- OpenRTI - Copyright (C) 2009-2022 Mathias Froehlich
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

#ifndef OpenRTI_AbstractServer_h
#define OpenRTI_AbstractServer_h

#include "AbstractConnect.h"
#include "AbstractMessage.h"
#include "AbstractServerNode.h"
#include "Mutex.h"
#include "Referenced.h"
#include "URL.h"

namespace OpenRTI {

// The input is implemented by the actual network side server.
// This one must make sure that it fits the threading model of the network server side.
// The output is implemented by this current class, probably just someting that dispatches into
// this servers accept methods. But still, this is assumed to be driven single threaded.
class OPENRTI_API AbstractServer : public Referenced {
public:
  AbstractServer(const SharedPtr<AbstractServerNode>& serverNode);
  virtual ~AbstractServer();

  /// The server node that processes the servers requests
  /// For debugging purpose this one is exchangable
  const AbstractServerNode& getServerNode() const;
  AbstractServerNode& getServerNode();

  virtual int exec() = 0;

  /// Supposed to be called from the current thread
  void setDone(bool done)
  { _sendDone(done); }
  bool getDone() const
  { return _done; }

  /// Supposed to be called from a differrent thread
  void postDone()
  { _postDone(); }

  /// Connect to the server - independent of the actual implementation
  SharedPtr<AbstractConnect> postConnect(const StringStringListMap& clientOptions);
  SharedPtr<AbstractConnect> sendConnect(const StringStringListMap& clientOptions, bool parent);

protected:
  typedef std::pair<SharedPtr<const AbstractMessage>, ConnectHandle> _MessageConnectHandlePair;
  typedef std::list<_MessageConnectHandlePair> _MessageConnectHandlePairList;

  class OPENRTI_LOCAL _Operation : public Referenced {
  public:
    virtual ~_Operation();
    virtual void operator()(AbstractServer& serverLoop) = 0;
  };
  typedef std::list<SharedPtr<_Operation> > _OperationList;

  // Connect to the server from the local thread
  ConnectHandle _sendConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions, bool parent);
  void _sendEraseConnect(const ConnectHandle& connectHandle);
  void _sendDisconnect(const ConnectHandle& connectHandle);

  // Use this to send something from within this current servers thread to this server.
  void _sendMessage(const _MessageConnectHandlePair& messageConnectHandlePair);
  void _sendOperation(_Operation& operation);
  virtual void _sendDone(bool done);

  // Use this to send something from a different thread to this server.
  virtual void _postMessage(const _MessageConnectHandlePair& messageConnectHandlePair) = 0;
  virtual void _postOperation(const SharedPtr<_Operation>& operation) = 0;

  // From a different thread, connect/disconnect to the server
  ConnectHandle _postConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions);
  void _postDisconnect(const ConnectHandle& connectHandle);
  // From a different thread, shut down the server
  void _postDone();

  class OPENRTI_API _Queue {
  public:
    void swap(_Queue& queue);
    bool empty() const;

    void send(AbstractServer& serverLoop);

    void push_back(const _MessageConnectHandlePair& messageConnectHandlePair, _MessageConnectHandlePairList& pool);
    void push_back(const SharedPtr<_Operation>& operation, _MessageConnectHandlePairList& pool);
    void recycle(_MessageConnectHandlePairList& pool);

  private:
    _MessageConnectHandlePairList _messageConnectHandlePairList;
    _OperationList _operationList;
  };

private:
  AbstractServer(const AbstractServer&);
  AbstractServer& operator=(const AbstractServer&);

  class _Connect;
  class _ConnectOperation;
  class _DisconnectOperation;
  class _DoneOperation;
  class _PostingMessageSender;
  class _SendingMessageSender;

  SharedPtr<AbstractServerNode> _serverNode;

  bool _done;
};

} // namespace OpenRTI

#endif
