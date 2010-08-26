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

#ifndef OpenRTI_StreamSocketReadEvent_h
#define OpenRTI_StreamSocketReadEvent_h

#include "SharedPtr.h"
#include "SocketReadEvent.h"
#include "SocketStream.h"
#include "VariableLengthData.h"

namespace OpenRTI {

/// StreamSocketReadEvent
///
/// Reads and assembles a data packet on a non blocking socket. The packet total size
/// is must be read from a fixed size header.
/// This is used to set up a common encoding at connection startup.
/// See InitialSocketReadEvent, InitialSocketWriteEvent and derived classes.
///
class OPENRTI_LOCAL StreamSocketReadEvent : public SocketReadEvent {
public:
  StreamSocketReadEvent(const SharedPtr<SocketStream>& socketStream);
  virtual ~StreamSocketReadEvent();

  // When the whole packet has arrived, this is called with the header in the first buffer and the body in the second
  virtual void readPacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer) = 0;

  class OPENRTI_LOCAL ReceiveCallback : public Referenced {
  public:
    virtual ~ReceiveCallback();
    virtual ssize_t recv(SocketStream& socketStream, NetworkBuffer& networkBuffer) = 0;
  };
  void setReceiveCallback(ReceiveCallback* receiveCallback)
  { _receiveCallback = receiveCallback; }
  ReceiveCallback* getReceiveCallback()
  { return _receiveCallback.get(); }

  // The SocketReadEvent side interface implementation
  virtual void read(SocketEventDispatcher& dispatcher);
  virtual SocketStream* getSocket() const;

protected:
  ssize_t recv(NetworkBuffer& networkBuffer);

private:
  SharedPtr<SocketStream> _socketStream;
  SharedPtr<ReceiveCallback> _receiveCallback;
  NetworkBuffer _networkBuffer;
};

} // namespace OpenRTI

#endif
