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

#ifndef OpenRTI_SocketEventDispatcher_h
#define OpenRTI_SocketEventDispatcher_h

#include "Export.h"
#include "SharedPtr.h"

namespace OpenRTI {

class Clock;
class SocketEvent;
class SocketReadEvent;
class SocketWriteEvent;

class OPENRTI_API SocketEventDispatcher {
public:
  SocketEventDispatcher();
  ~SocketEventDispatcher();

  void setDone(bool done);
  bool getDone() const;

  void insert(const SharedPtr<SocketReadEvent>& socketEvent);
  void erase(const SharedPtr<SocketReadEvent>& socketEvent);

  void insert(const SharedPtr<SocketWriteEvent>& socketEvent);
  void erase(const SharedPtr<SocketWriteEvent>& socketEvent);

  // Erases both the read and the write part.
  void eraseSocket(const SharedPtr<SocketEvent>& socketEvent);

  /// FIXME colapse them all to what is needed
  int exec();
  int exec(const Clock& absclock);

  // only iterate one time
  int exec1(const Clock& absclock);

private:
  struct PrivateData;
  PrivateData* _privateData;
};

} // namespace OpenRTI

#endif
