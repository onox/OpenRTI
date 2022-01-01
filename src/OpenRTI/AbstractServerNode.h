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

#ifndef OpenRTI_AbstractServerNode_h
#define OpenRTI_AbstractServerNode_h

#include "AbstractMessage.h"
#include "Handle.h"
#include "Referenced.h"
#include "SharedPtr.h"
#include "StringUtils.h"

namespace OpenRTI {

class AbstractMessageSender;
class ServerOptions;

/// Provides a rti ServerNode in the tree hierarchy.
/// This class already operates on AbstractMessage typed data structures.
/// This is purely for the message logic independent of any network details.
/// Must stay single threaded. Must not care for locks!
/// That means, it could run efficient even in single threaded servers/root hubs to the network etc
/// A really mutithreaded server can then provide message senders that care for locking.
/// These suffer from the locking overhead but do not disturb implementation that do not need.
class OPENRTI_API AbstractServerNode : public Referenced {
public:
  AbstractServerNode();
  virtual ~AbstractServerNode();

  /// Hmm, make that more callback based during connection setup, but for now ...
  virtual ServerOptions& getServerOptions() = 0;
  virtual const ServerOptions& getServerOptions() const = 0;

  /// Returns true if the server is idle.
  /// Whare idle means that it is save to shut down the server completely.
  /// This is false for any root server or, for a child server that has child connects.
  virtual bool isIdle() const = 0;

  virtual ConnectHandle _insertConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions) = 0;
  virtual ConnectHandle _insertParentConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& parentOptions) = 0;
  virtual void _eraseConnect(const ConnectHandle& connectHandle) = 0;
  virtual void _dispatchMessage(const AbstractMessage* message, const ConnectHandle& connectHandle) = 0;

private:
  AbstractServerNode(const AbstractServerNode&);
  AbstractServerNode& operator=(const AbstractServerNode&);
};

} // namespace OpenRTI

#endif
