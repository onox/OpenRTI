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

#ifndef OpenRTI_MessageServer_h
#define OpenRTI_MessageServer_h

#include "AbstractMessageSender.h"
#include "ServerNode.h"
#include "ServerOptions.h"
#include "SharedPtr.h"

namespace OpenRTI {

class OPENRTI_LOCAL MessageServer : public ServerNode {
public:
  MessageServer(const SharedPtr<ServerOptions>& serverOptions = new ServerOptions);
  virtual ~MessageServer();

  SharedPtr<AbstractMessageSender> insertConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& childOptions);
  SharedPtr<AbstractMessageSender> insertParentConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& parentOptions);

private:
  class MessageSender;
  friend class MessageSender;
};

} // namespace OpenRTI

#endif
