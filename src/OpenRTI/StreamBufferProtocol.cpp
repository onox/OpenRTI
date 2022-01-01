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

#include "StreamBufferProtocol.h"

#include "LogStream.h"

namespace OpenRTI {

StreamBufferProtocol::StreamBufferProtocol()
{
  _inputIterator = _inputBuffer.byte_begin();
  _outputIterator = _outputBuffer.byte_begin();
}

StreamBufferProtocol::~StreamBufferProtocol()
{
}

void
StreamBufferProtocol::read(AbstractProtocolSocket& protocolSocket)
{
  while (getEnableRead()) {
    while (_inputIterator != _inputBuffer.byte_end()) {
      ssize_t ret = protocolSocket.recv(BufferRange(_inputIterator, _inputBuffer.byte_end()), false);
      if (ret == -1) {
        // EAGAIN or similar in this socket abstraction.
        // Serious error numbers are delivered as exceptions.
        return;
      }
      if (ret == 0) {
        // EOF in this socket abstraction.
        // Serious error numbers are delivered as exceptions
        // FIXME close does not help.
        protocolSocket.close();
        return;
      }
      _inputIterator += ret;
      _inputIterator.skip_empty_chunks(_inputBuffer.byte_end());
    }

    readPacket(_inputBuffer);
    if (_inputIterator != _inputBuffer.byte_end())
      continue;

    for (std::list<Buffer::iterator>::iterator i = _inputScratchBufferList.begin();
         i != _inputScratchBufferList.end(); ++i) {
      _scratchPool.splice(_scratchPool.end(), _inputBuffer, *i);
    }

    _iteratorPool.splice(_iteratorPool.end(), _inputScratchBufferList);
    _inputBuffer.clear();
    _inputIterator = _inputBuffer.byte_begin();
  }
}

bool
StreamBufferProtocol::getEnableRead() const
{
  return !getInputBufferComplete();
}

void
StreamBufferProtocol::write(AbstractProtocolSocket& protocolSocket)
{
  while (getEnableWrite()) {
    if (_outputBuffer.empty())
      writePacket();

    while (_outputIterator != _outputBuffer.byte_end()) {
      ssize_t ret = protocolSocket.send(ConstBufferRange(_outputIterator, _outputBuffer.byte_end()), getMoreToSend());
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
      _outputIterator += ret;
      _outputIterator.skip_empty_chunks(_outputBuffer.byte_end());
    }

    // We are ready with this packet, reset state
    for (std::list<Buffer::iterator>::iterator i = _outputScratchBufferList.begin();
         i != _outputScratchBufferList.end(); ++i) {
      _scratchPool.splice(_scratchPool.end(), _outputBuffer, *i);
    }

    _iteratorPool.splice(_iteratorPool.end(), _outputScratchBufferList);
    _outputBuffer.clear();

    _outputIterator = _outputBuffer.byte_begin();
  }
}

bool
StreamBufferProtocol::getEnableWrite() const
{
  return !getOutputBufferComplete() || getMoreToSend();
}

void
StreamBufferProtocol::addReadBuffer(size_t size)
{
  OpenRTIAssert(size);
  if (_inputIterator == _inputBuffer.byte_end())
    _inputIterator = _inputBuffer.insert(_inputBuffer.end(), VariableLengthData(size));
  else
    _inputBuffer.push_back(VariableLengthData(size));
}

void
StreamBufferProtocol::addScratchReadBuffer(size_t size)
{
  OpenRTIAssert(size);
  VariableLengthDataList scratchElement;
  if (!_scratchPool.empty())
    scratchElement.splice(scratchElement.end(), _scratchPool, _scratchPool.begin());
  else
    scratchElement.push_back(VariableLengthData());
  _inputBuffer.splice(_inputBuffer.end(), scratchElement, scratchElement.begin());
  VariableLengthDataList::iterator back_iterator = _inputBuffer.end();
  --back_iterator;
  if (_inputIterator.iterator() == _inputBuffer.end())
    _inputIterator = back_iterator;
  if (_iteratorPool.empty())
    _inputScratchBufferList.push_back(back_iterator);
  else {
    _inputScratchBufferList.splice(_inputScratchBufferList.end(), _iteratorPool, _iteratorPool.begin());
    _inputScratchBufferList.back() = back_iterator;
  }
  _inputBuffer.back().resize(size);
}

void
StreamBufferProtocol::addWriteBuffer(const VariableLengthData& value)
{
  if (value.empty())
    return;
  if (_outputIterator == _outputBuffer.byte_end())
    _outputIterator = _outputBuffer.insert(_outputBuffer.end(), value);
  else
    _outputBuffer.push_back(value);
}

VariableLengthData&
StreamBufferProtocol::addScratchWriteBuffer()
{
  VariableLengthDataList scratchElement;
  if (!_scratchPool.empty())
    scratchElement.splice(scratchElement.end(), _scratchPool, _scratchPool.begin());
  else
    scratchElement.push_back(VariableLengthData());
  _outputBuffer.splice(_outputBuffer.end(), scratchElement, scratchElement.begin());
  VariableLengthDataList::iterator back_iterator = _outputBuffer.end();
  --back_iterator;
  if (_outputIterator.iterator() == _outputBuffer.end())
    _outputIterator = back_iterator;
  if (_iteratorPool.empty())
    _outputScratchBufferList.push_back(back_iterator);
  else {
    _outputScratchBufferList.splice(_outputScratchBufferList.end(), _iteratorPool, _iteratorPool.begin());
    _outputScratchBufferList.back() = back_iterator;
  }
  _outputBuffer.back().resize(0);
  return _outputBuffer.back();
}

} // namespace OpenRTI
