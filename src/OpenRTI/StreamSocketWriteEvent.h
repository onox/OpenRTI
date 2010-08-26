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

#ifndef OpenRTI_StreamSocketWriteEvent_h
#define OpenRTI_StreamSocketWriteEvent_h

#include "SharedPtr.h"
#include "SocketWriteEvent.h"
#include "SocketStream.h"
#include "VariableLengthData.h"

namespace OpenRTI {

/// StreamSocketWriteEvent
///
/// Write and flushes a data packet on a non blocking socket.
/// This is used to set up a common encoding at connection startup.
/// See InitialSocketReadEvent, InitialSocketWriteEvent and derived classes.
///
class OPENRTI_LOCAL StreamSocketWriteEvent : public SocketWriteEvent {
public:
  StreamSocketWriteEvent(const SharedPtr<SocketStream>& socketStream);
  virtual ~StreamSocketWriteEvent();

  // Compose the whole packet
  virtual void writePacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer) = 0;
  // Called when the packet is written completely
  virtual void written(SocketEventDispatcher& dispatcher);
  // Returns if there is currently more data to send beyond the currently sent network buffer
  virtual bool getMoreToSend() const;

  class OPENRTI_LOCAL SendCallback : public Referenced {
  public:
    virtual ~SendCallback();
    virtual ssize_t send(SocketStream& socketStream, const NetworkBuffer& networkBuffer, bool moreToSend) = 0;
  };
  void setSendCallback(SendCallback* sendCallback)
  { _sendCallback = sendCallback; }
  SendCallback* getSendCallback()
  { return _sendCallback.get(); }

  // The SocketReadEvent side interface implementation
  virtual void write(SocketEventDispatcher& dispatcher);
  virtual SocketStream* getSocket() const;

protected:
  ssize_t send(const NetworkBuffer& networkBuffer, bool moreToSend);

private:
  SharedPtr<SocketStream> _socketStream;
  SharedPtr<SendCallback> _sendCallback;
  NetworkBuffer _networkBuffer;
};

} // namespace OpenRTI

#endif
