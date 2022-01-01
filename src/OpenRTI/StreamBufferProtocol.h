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

#ifndef OpenRTI_StreamBufferProtocol_h
#define OpenRTI_StreamBufferProtocol_h

#include "AbstractProtocolLayer.h"
#include "Buffer.h"

namespace OpenRTI {

class OPENRTI_API StreamBufferProtocol : public AbstractProtocolLayer {
public:
  StreamBufferProtocol();
  virtual ~StreamBufferProtocol();

  virtual void readPacket(const Buffer& buffer) = 0;
  virtual void writePacket() = 0;
  virtual bool getMoreToSend() const = 0;

  virtual void read(AbstractProtocolSocket& protocolSocket);
  virtual bool getEnableRead() const;

  virtual void write(AbstractProtocolSocket& protocolSocket);
  virtual bool getEnableWrite() const;

  bool getBuffersComplete() const
  { return getInputBufferComplete() && getOutputBufferComplete(); }
  bool getInputBufferComplete() const
  { return _inputIterator == _inputBuffer.byte_end(); }
  bool getOutputBufferComplete() const
  { return _outputIterator == _outputBuffer.byte_end(); }

  void addReadBuffer(size_t size);
  void addScratchReadBuffer(size_t size);
  void addWriteBuffer(const VariableLengthData& value);
  VariableLengthData& addScratchWriteBuffer();

private:
  // Buffer for the incomming data
  Buffer _inputBuffer;
  Buffer::byte_iterator _inputIterator;

  // Buffer for the outgoing data
  Buffer _outputBuffer;
  Buffer::const_byte_iterator _outputIterator;

  // Pool of scratch data packets
  std::list<Buffer::iterator> _inputScratchBufferList;
  std::list<Buffer::iterator> _outputScratchBufferList;
  std::list<Buffer::iterator> _iteratorPool;
  VariableLengthDataList _scratchPool;
};

} // namespace OpenRTI

#endif
