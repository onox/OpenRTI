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

#ifndef OpenRTI_NetworkServerConnect_h
#define OpenRTI_NetworkServerConnect_h

#include "AbstractConnect.h"
#include "AbstractServerNode.h"
#include "MessageQueue.h"

namespace OpenRTI {

/// This is the class that is persistent over reconnects of clients
class OPENRTI_API NetworkServerConnect : public AbstractConnect {
public:
  NetworkServerConnect() :
    _messageQueue(new LocalMessageQueue)
  { }
  virtual ~NetworkServerConnect()
  { }

  virtual AbstractMessageSender* getMessageSender()
  { return _messageSender.get(); }
  virtual AbstractMessageReceiver* getMessageReceiver()
  { return _messageQueue.get(); }

  void connect(AbstractServerNode& serverNode, const StringStringListMap& optionMap)
  {
    OpenRTIAssert(!_messageSender.valid());
    _messageSender = serverNode.insertConnect(_messageQueue->getMessageSender(), optionMap);
  }

  void connectParent(AbstractServerNode& serverNode, const StringStringListMap& optionMap)
  {
    OpenRTIAssert(!_messageSender.valid());
    _messageSender = serverNode.insertParentConnect(_messageQueue->getMessageSender(), optionMap);
  }

private:
  /// The message connection to the server
  SharedPtr<AbstractMessageSender> _messageSender;
  /// The messages that need to go out through this connection
  SharedPtr<LocalMessageQueue> _messageQueue;
};

} // namespace OpenRTI

#endif
