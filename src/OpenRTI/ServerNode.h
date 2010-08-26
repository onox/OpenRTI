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

#ifndef OpenRTI_ServerNode_h
#define OpenRTI_ServerNode_h

#include <string>
#include "AbstractMessageSender.h"
#include "Referenced.h"
#include "SharedPtr.h"

namespace OpenRTI {

class ConnectHandle;
class ServerMessageDispatcher;
class ServerOptions;

class OPENRTI_LOCAL ServerNode : public Referenced {
public:
  ServerNode(const SharedPtr<ServerOptions>& serverOptions);
  virtual ~ServerNode();

  const std::wstring& getName() const;
  void setName(const std::wstring& name);

  bool isRunning() const;

  const ServerOptions& getServerOptions() const;

protected:
  ConnectHandle insertConnect(const SharedPtr<AbstractMessageSender>& messageSender);
  ConnectHandle insertParentConnect(const SharedPtr<AbstractMessageSender>& messageSender);
  void removeConnect(const ConnectHandle& connectHandle);

  void dispatchMessage(AbstractMessage& message, const ConnectHandle& connectHandle);

private:
  ServerNode(const ServerNode&);
  ServerNode& operator=(const ServerNode&);

  SharedPtr<ServerMessageDispatcher> _serverMessageDispatcher;
};

} // namespace OpenRTI

#endif
