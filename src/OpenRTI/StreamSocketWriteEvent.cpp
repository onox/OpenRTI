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

#include "StreamSocketWriteEvent.h"

#include "LogStream.h"
#include "SocketEventDispatcher.h"

namespace OpenRTI {

StreamSocketWriteEvent::SendCallback::~SendCallback()
{
}

// Have a common class peeking into a header and writeing the whole packet then
StreamSocketWriteEvent::StreamSocketWriteEvent(const SharedPtr<SocketStream>& socketStream) :
  SocketWriteEvent(false),
  _socketStream(socketStream)
{
}

StreamSocketWriteEvent::~StreamSocketWriteEvent()
{
}

void
StreamSocketWriteEvent::write(SocketEventDispatcher& dispatcher)
{
  try {
    while (getEnable()) {
      if (_networkBuffer.empty())
        writePacket(dispatcher, _networkBuffer);

      while (!_networkBuffer.complete()) {
        ssize_t ret = send(_networkBuffer, getMoreToSend());
        if (ret == -1) {
          // Signals a message to big error. Currently unhandled ...
          // Serious error numbers are delivered as exceptions.
          dispatcher.eraseSocket(this);
          Log(MessageCoding, Warning) << "Got an error code from sending to socket!" << std::endl;
          return;
        }
        if (ret == 0) {
          // EAGAIN or similar in this socket abstraction.
          // Serious error numbers are delivered as exceptions.
          return;
        }
        _networkBuffer.processed(ret);
      }

      // We are ready with this packet, reset state
      _networkBuffer.clear();

      written(dispatcher);
    }

    // Here is the place to flush/uncork

  } catch (const Exception& e) {
    dispatcher.eraseSocket(this);
    Log(MessageCoding, Warning) << "Caught exception while processing socket output: " << e.getReasonInLocale()
                                << "\nClosing connection!" << std::endl;
  } catch (...) {
    dispatcher.eraseSocket(this);
    Log(MessageCoding, Warning) << "Caught exception while processing socket output!\n"
                                << "Closing connection!" << std::endl;
  }
}

void
StreamSocketWriteEvent::written(SocketEventDispatcher& dispatcher)
{
}

bool
StreamSocketWriteEvent::getMoreToSend() const
{
  return false;
}

SocketStream*
StreamSocketWriteEvent::getSocket() const
{
  return _socketStream.get();
}

ssize_t
StreamSocketWriteEvent::send(const NetworkBuffer& networkBuffer, bool moreToSend)
{
  if (_sendCallback.valid())
    return _sendCallback->send(*_socketStream, networkBuffer, moreToSend);
  else
    return _socketStream->send(networkBuffer, moreToSend);
}

} // namespace OpenRTI
