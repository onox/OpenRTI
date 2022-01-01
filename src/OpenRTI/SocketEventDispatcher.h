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

#ifndef OpenRTI_SocketEventDispatcher_h
#define OpenRTI_SocketEventDispatcher_h

#include "AbstractSocketEvent.h"
#include "Export.h"
#include "SharedPtr.h"

namespace OpenRTI {

class Clock;

class OPENRTI_API SocketEventDispatcher {
public:
  SocketEventDispatcher();
  ~SocketEventDispatcher();

  void setDone(bool done);
  bool getDone() const;

  void insert(const SharedPtr<AbstractSocketEvent>& socketEvent);
  void erase(const SharedPtr<AbstractSocketEvent>& socketEvent);
  bool empty() const
  { return _socketEventList.empty(); }

  // Make the exec loop exit one time ???
  void wakeUp();

  // Processes socket events when there is something to do.
  // The function returns on three conditions:
  // * The timeout expires.
  // * When empty(), that is there are no socket events left.
  // * The wakeUp method is called from some thread.
  int exec(const Clock& absclock = Clock::max());

private:
  SocketEventDispatcher(const SocketEventDispatcher&);
  SocketEventDispatcher& operator=(const SocketEventDispatcher&);

  void read(const SharedPtr<AbstractSocketEvent>& socketEvent);
  void write(const SharedPtr<AbstractSocketEvent>& socketEvent);
  void timeout(const SharedPtr<AbstractSocketEvent>& socketEvent);

  struct PrivateData;
  PrivateData* _privateData;

  SocketEventList _socketEventList;
  bool _done;
};

} // namespace OpenRTI

#endif
