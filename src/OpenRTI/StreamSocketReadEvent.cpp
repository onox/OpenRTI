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

#include "StreamSocketReadEvent.h"

#include "LogStream.h"
#include "SocketEventDispatcher.h"

namespace OpenRTI {

StreamSocketReadEvent::ReceiveCallback::~ReceiveCallback()
{
}

StreamSocketReadEvent::StreamSocketReadEvent(const SharedPtr<SocketStream>& socketStream) :
  SocketReadEvent(true),
  _socketStream(socketStream)
{
}

StreamSocketReadEvent::~StreamSocketReadEvent()
{
}

void
StreamSocketReadEvent::read(SocketEventDispatcher& dispatcher)
{
  try {
    while (getEnable()) {
      while (!_networkBuffer.complete()) {
        ssize_t ret = recv(_networkBuffer);
        if (ret == -1) {
          // EAGAIN or similar in this socket abstraction.
          // Serious error numbers are delivered as exceptions.
          return;
        }
        if (ret == 0) {
          // EOF in this socket abstraction.
          // Serious error numbers are delivered as exceptions
          dispatcher.erase(this);
          // FIXME be a litte less crude???
          dispatcher.eraseSocket(this);
          return;
        }
        _networkBuffer.processed(ret);
      }

      readPacket(dispatcher, _networkBuffer);
      if (!_networkBuffer.complete())
        continue;

      _networkBuffer.clear();
    }
  } catch (const Exception& e) {
    dispatcher.eraseSocket(this);
    Log(MessageCoding, Warning) << "Caught exception processing socket input: " << e.getReasonInLocale()
                                << "\nDropping connection!" << std::endl;
  } catch (...) {
    dispatcher.eraseSocket(this);
    Log(MessageCoding, Warning) << "Caught exception processing socket input!\n"
                                << "Dropping connection!" << std::endl;
  }
}

SocketStream*
StreamSocketReadEvent::getSocket() const
{
  return _socketStream.get();
}

ssize_t
StreamSocketReadEvent::recv(NetworkBuffer& networkBuffer)
{
  if (_receiveCallback.valid())
    return _receiveCallback->recv(*_socketStream, networkBuffer);
  else
    return _socketStream->recv(networkBuffer);
}

} // namespace OpenRTI
