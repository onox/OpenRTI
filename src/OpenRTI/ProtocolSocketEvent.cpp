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

#include "ProtocolSocketEvent.h"

#include "AbstractProtocolLayer.h"
#include "SocketEventDispatcher.h"

namespace OpenRTI {

struct OPENRTI_LOCAL ProtocolSocketEvent::ProtocolSocket : public AbstractProtocolSocket {
  ProtocolSocket(const SharedPtr<SocketStream>& socketStream) :
    _socketStream(socketStream),
    _closed(false)
  { }
  virtual ~ProtocolSocket()
  { }

  /// Can be called from the consuming layer to receive ready to read data
  virtual ssize_t recv(const BufferRange& bufferRange, bool peek)
  { return _socketStream->recv(bufferRange, peek); }
  /// Can be called from the consuming layer to send something into this protocol layer
  virtual ssize_t send(const ConstBufferRange& bufferRange, bool more)
  { return _socketStream->send(bufferRange, more); }

  /// Call when the user wants to close this socket.
  virtual void close()
  {
    _socketStream->shutdown();
    _closed = true;
  }

  virtual void replaceProtocol(const SharedPtr<AbstractProtocolLayer>& protocolLayer)
  { _replacingProtocol = protocolLayer; }

  void read()
  {
    _protocolLayer->read(*this);
    activateReplacingProtocol();
  }

  void write()
  {
    _protocolLayer->write(*this);
    activateReplacingProtocol();
  }

  // FIXME: do with a callback, the same goes for close ...
  void activateReplacingProtocol()
  {
    if (!_replacingProtocol.valid())
      return;
    _protocolLayer.swap(_replacingProtocol);
    _replacingProtocol = 0;
  }

  SharedPtr<SocketStream> _socketStream;
  SharedPtr<AbstractProtocolLayer> _protocolLayer;
  SharedPtr<AbstractProtocolLayer> _replacingProtocol;
  bool _closed;
};

ProtocolSocketEvent::ProtocolSocketEvent(const SharedPtr<SocketStream>& socketStream) :
  _protocolSocket(new ProtocolSocket(socketStream))
{
}

ProtocolSocketEvent::~ProtocolSocketEvent()
{
  delete _protocolSocket;
  _protocolSocket = 0;
}

void
ProtocolSocketEvent::read(SocketEventDispatcher& dispatcher)
{
  if (!_protocolSocket->_closed) {
    _protocolSocket->read();
  } else {
    Buffer buffer;
    buffer.push_back(VariableLengthData(64*1024));
    ssize_t ret;
    do {
      ret = _protocolSocket->_socketStream->recv(BufferRange(buffer.byte_begin(), buffer.byte_end()), false);
    } while (0 < ret);
    if (ret == 0) {
      dispatcher.erase(this);
    }
  }
}

bool
ProtocolSocketEvent::getEnableRead() const
{
  return _protocolSocket->_protocolLayer->getEnableRead() || _protocolSocket->_closed;
}

void
ProtocolSocketEvent::write(SocketEventDispatcher& dispatcher)
{
  OpenRTIAssert(!_protocolSocket->_closed);
  _protocolSocket->write();
}

bool
ProtocolSocketEvent::getEnableWrite() const
{
  return !_protocolSocket->_closed && _protocolSocket->_protocolLayer->getEnableWrite();
}

void
ProtocolSocketEvent::error(const Exception& e)
{
  _protocolSocket->_protocolLayer->error(e);
}

SocketStream*
ProtocolSocketEvent::getSocket() const
{
  return _protocolSocket->_socketStream.get();
}

void
ProtocolSocketEvent::setProtocolLayer(const SharedPtr<AbstractProtocolLayer>& protocolLayer)
{
  _protocolSocket->_protocolLayer = protocolLayer;
}

const SharedPtr<AbstractProtocolLayer>&
ProtocolSocketEvent::getProtocolLayer() const
{
  return _protocolSocket->_protocolLayer;
}

} // namespace OpenRTI
