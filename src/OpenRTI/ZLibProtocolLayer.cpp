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

#include "ZLibProtocolLayer.h"

#include <zlib.h>
#include "LogStream.h"

namespace OpenRTI {

class OPENRTI_LOCAL ZLibProtocolLayer::ProtocolSocket : public AbstractProtocolSocket {
public:
  ProtocolSocket(int level) :
    _closed(false),
    _more(false)
  {
    std::memset(&_inStream, 0, sizeof(_inStream));
    inflateInit(&_inStream);
    std::memset(&_outStream, 0, sizeof(_outStream));
    deflateInit(&_outStream, level);

    _readBuffer.push_back(VariableLengthData());
    _readBuffer.front().reserve(64*1024 + 1024);
    _writeBuffer.push_back(VariableLengthData());
    _writeBuffer.front().reserve(64*1024 + 1024);
    _outputIterator = _writeBuffer.byte_end();
  }
  virtual ~ProtocolSocket()
  {
    inflateEnd(&_inStream);
    std::memset(&_inStream, 0, sizeof(_inStream));
    deflateEnd(&_outStream);
    std::memset(&_outStream, 0, sizeof(_outStream));
  }

  // Can be called from the consuming layer to receive ready to read data
  virtual ssize_t recv(const BufferRange& bufferRange, bool peek)
  {
    OpenRTIAssert(!peek);

    // If closed return EOF
    if (_closed)
      return 0;

    // If we have nothing to read, return EAGAIN
    if (!_inStream.avail_in)
      return -1;

    ssize_t bytesRead = 0;
    Buffer::byte_iterator i = bufferRange.first;
    i.skip_empty_chunks(bufferRange.second);
    while (i != bufferRange.second && _inStream.avail_in) {
      size_t size = i.chunk_size(bufferRange.second);
      if (!size)
        continue;

      _inStream.next_out = (Bytef*)(i.data());
      _inStream.avail_out = size;

      Buffer::byte_iterator j = i;
      j += size;
      j.skip_empty_chunks(bufferRange.second);

      int flush;
      if (j == bufferRange.second)
        flush = Z_SYNC_FLUSH;
      else
        flush = Z_NO_FLUSH;
      int ret = inflate(&_inStream, flush);
      if (ret != Z_OK) {
        Log(MessageCoding, Warning) << "Compression error!" << std::endl;
        throw MessageError("Compression error!");
      }
      // if (ret == Z_DATA_ERROR) {
      //   Log(MessageCoding, Warning) << "Compression error!" << std::endl;
      //   throw MessageError("Compression error!");
      // }
      size -= _inStream.avail_out;
      i += size;
      i.skip_empty_chunks(bufferRange.second);
      bytesRead += size;
    }

    _inStream.next_out = 0;
    _inStream.avail_out = 0;

    if (bytesRead == 0)
      return -1;

    return bytesRead;
  }

  // Can be called from the consuming layer to send something into this protocol layer
  virtual ssize_t send(const ConstBufferRange& bufferRange, bool more)
  {
    if (_closed)
      throw MessageError("Connection already closed!");

    // return EAGAIN if there is no room left to compress
    if (!_outStream.avail_out)
      return 0;

    ssize_t bytesWritten = 0;
    Buffer::const_byte_iterator i = bufferRange.first;
    i.skip_empty_chunks(bufferRange.second);
    while (i != bufferRange.second && _outStream.avail_out) {
      size_t size = i.chunk_size(bufferRange.second);
      if (!size)
        continue;

      _outStream.next_in = (Bytef*)(i.data());
      _outStream.avail_in = size;

      int ret = deflate(&_outStream, Z_NO_FLUSH);
      OpenRTIAssert(ret != Z_STREAM_END);
      OpenRTIAssert(ret != Z_STREAM_ERROR);
      if (ret == Z_BUF_ERROR)
        break;
      OpenRTIAssert(ret == Z_OK);
      size -= _outStream.avail_in;
      i += size;
      i.skip_empty_chunks(bufferRange.second);
      bytesWritten += size;
    }

    _outStream.next_in = 0;
    _outStream.avail_in = 0;

    // FIXME may be move this into the write call below?
    _more = more || i != bufferRange.second;

    // Need to terminate the buffer when nothing else is pending
    if (!_more) {
      do {
        if (_outStream.avail_out == 0) {
          size_t size = _writeBuffer.front().size();
          _writeBuffer.front().resize(7 + size);
          _outStream.next_out = (Bytef*)_writeBuffer.front().data(size);
          _outStream.avail_out = 7;
        }
        int ret = deflate(&_outStream, Z_SYNC_FLUSH);
        OpenRTIAssert(ret != Z_STREAM_ERROR);
        // The zlib documentation reads to me that this check is not needed,
        // but it looks like it actually is?
        if (ret == Z_STREAM_END) break;
      } while (_outStream.avail_out == 0);
    }

    return bytesWritten;
  }

  virtual void close()
  {
    _closed = true;
  }

  virtual void replaceProtocol(const SharedPtr<AbstractProtocolLayer>&)
  {
    OpenRTIAssert(false);
  }

  z_stream _inStream;
  Buffer _readBuffer;

  z_stream _outStream;
  Buffer _writeBuffer;
  Buffer::const_byte_iterator _outputIterator;

  bool _closed;
  bool _more;
};

ZLibProtocolLayer::ZLibProtocolLayer() :
  _protocolSocket(new ProtocolSocket(Z_BEST_SPEED))
{
}

ZLibProtocolLayer::~ZLibProtocolLayer()
{
  delete _protocolSocket;
  _protocolSocket = 0;
}

void
ZLibProtocolLayer::read(AbstractProtocolSocket& protocolSocket)
{
  while (NestedProtocolLayer::getEnableRead()) {
    // FIXME move this into the protocolsocket
    if (_protocolSocket->_inStream.avail_in == 0) {
      _protocolSocket->_readBuffer.front().resize(64*1024);
      ssize_t ret = protocolSocket.recv(BufferRange(_protocolSocket->_readBuffer.byte_begin(),
                                                    _protocolSocket->_readBuffer.byte_end()), false);
      if (ret == -1) {
        // EAGAIN or similar in this socket abstraction.
        // Serious error numbers are delivered as exceptions.
        _protocolSocket->_readBuffer.front().resize(0);
        return;
      }
      if (ret == 0) {
        // EOF in this socket abstraction.
        // Serious error numbers are delivered as exceptions
        _protocolSocket->_readBuffer.front().resize(0);
        protocolSocket.close();
        return;
      }
      _protocolSocket->_readBuffer.front().resize(ret);
      _protocolSocket->_inStream.next_in = (Bytef*)_protocolSocket->_readBuffer.front().data();
      _protocolSocket->_inStream.avail_in = ret;
    }

    // Call the nested read
    NestedProtocolLayer::read(*_protocolSocket);

    if (_protocolSocket->_closed) {
      protocolSocket.close();
      return;
    }
  }
}

bool
ZLibProtocolLayer::getEnableRead() const
{
  return NestedProtocolLayer::getEnableRead();
}

void
ZLibProtocolLayer::write(AbstractProtocolSocket& protocolSocket)
{
  while (getEnableWrite()) {
    while (_protocolSocket->_outputIterator != _protocolSocket->_writeBuffer.byte_end()) {
      ssize_t ret = protocolSocket.send(ConstBufferRange(_protocolSocket->_outputIterator,
                                                         _protocolSocket->_writeBuffer.byte_end()), _protocolSocket->_more);
      if (ret == -1) {
        // Signals a message to big error. Currently unhandled ...
        // Serious error numbers are delivered as exceptions.
        protocolSocket.close();
        Log(MessageCoding, Warning) << "Got an error code from sending to socket!" << std::endl;
        return;
      }
      if (ret == 0) {
        // EAGAIN or similar in this socket abstraction.
        // Serious error numbers are delivered as exceptions.
        return;
      }
      _protocolSocket->_outputIterator += ret;
      _protocolSocket->_outputIterator.skip_empty_chunks(_protocolSocket->_writeBuffer.byte_end());
    }

    if (_protocolSocket->_closed) {
      protocolSocket.close();
      return;
    }

    if (!NestedProtocolLayer::getEnableWrite())
      return;

    // We are ready with this packet, reset state
    size_t size = 32*1024;
    _protocolSocket->_writeBuffer.front().resize(size);
    _protocolSocket->_outStream.next_out = (Bytef*)_protocolSocket->_writeBuffer.front().data();
    _protocolSocket->_outStream.avail_out = size;

    // Call the nested write, fill the write buffer to get food to compress
    do {
      NestedProtocolLayer::write(*_protocolSocket);
    } while (_protocolSocket->_outStream.avail_out && _protocolSocket->_more);

    size = _protocolSocket->_writeBuffer.front().size() - _protocolSocket->_outStream.avail_out;
    _protocolSocket->_writeBuffer.front().resize(size);

    _protocolSocket->_outStream.next_out = 0;
    _protocolSocket->_outStream.avail_out = 0;

    _protocolSocket->_outputIterator = _protocolSocket->_writeBuffer.byte_begin();
  }
}

bool
ZLibProtocolLayer::getEnableWrite() const
{
  return _protocolSocket->_outputIterator != _protocolSocket->_writeBuffer.byte_end() || NestedProtocolLayer::getEnableWrite();
}

void ZLibProtocolLayer::error(const Exception& e)
{
  NestedProtocolLayer::error(e);
}

} // namespace OpenRTI
