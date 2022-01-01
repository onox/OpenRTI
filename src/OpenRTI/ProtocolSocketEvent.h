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

#ifndef OpenRTI_ProtocolSocketEvent_h
#define OpenRTI_ProtocolSocketEvent_h

#include "AbstractSocketEvent.h"
#include "SharedPtr.h"
#include "SocketStream.h"

namespace OpenRTI {

class AbstractProtocolLayer;

/// This is the top level protocol layer that just puts data into the actual operating system socket
/// In a next step this shouold just become the top level socket dispatcher event.
class OPENRTI_API ProtocolSocketEvent : public AbstractSocketEvent {
public:
  ProtocolSocketEvent(const SharedPtr<SocketStream>& socketStream);
  virtual ~ProtocolSocketEvent();

  // Is called from the parent protocol layer when there is data to read
  virtual void read(SocketEventDispatcher& dispatcher);
  virtual bool getEnableRead() const;

  // Is called from the parent protocol layer when there is space to write something
  virtual void write(SocketEventDispatcher& dispatcher);
  virtual bool getEnableWrite() const;

  virtual void error(const Exception& e);

  virtual SocketStream* getSocket() const;

  void setProtocolLayer(const SharedPtr<AbstractProtocolLayer>& protocolLayer);
  const SharedPtr<AbstractProtocolLayer>& getProtocolLayer() const;

private:
  ProtocolSocketEvent(const ProtocolSocketEvent&);
  ProtocolSocketEvent& operator=(ProtocolSocketEvent&);

  struct ProtocolSocket;
  ProtocolSocket* _protocolSocket;
};

} // namespace OpenRTI

#endif
