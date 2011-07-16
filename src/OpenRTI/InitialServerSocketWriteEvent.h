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

#ifndef OpenRTI_InitialServerSocketWriteEvent_h
#define OpenRTI_InitialServerSocketWriteEvent_h

#include "InitialSocketWriteEvent.h"
#include "MessageServer.h"
#include "SocketStream.h"

namespace OpenRTI {

// Just a chain writing socket event to push out the initial connect answer and
// Than replace that one with a next read event
class OPENRTI_LOCAL InitialServerSocketWriteEvent : public InitialSocketWriteEvent {
public:
  InitialServerSocketWriteEvent(const SharedPtr<SocketStream>& socketStream,
                                const SharedPtr<AbstractServerNode>& serverNode,
                                const StringStringListMap& clientValueMap);
  virtual ~InitialServerSocketWriteEvent();

  virtual void written(SocketEventDispatcher& dispatcher);

private:
  SharedPtr<AbstractServerNode> _serverNode;
  StringStringListMap _clientValueMap;
};

} // namespace OpenRTI

#endif
