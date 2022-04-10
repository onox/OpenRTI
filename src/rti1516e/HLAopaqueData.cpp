/* -*-c++-*- OpenRTI - Copyright (C) 2011-2022 Mathias Froehlich
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

#include <RTI/encoding/HLAopaqueData.h>

#include <RTI/VariableLengthData.h>
#include <RTI/encoding/DataElement.h>

#include <cstring>
#include <vector>

#include "Export.h"
#include "Encoding.h"
#include "Types.h"

namespace rti1516e
{

class OPENRTI_LOCAL HLAopaqueDataImplementation {
public:
  HLAopaqueDataImplementation()
  {
  }
  HLAopaqueDataImplementation(const Octet* inData, size_t dataSize) :
    _buffer(inData, inData + dataSize)
  {
  }
  HLAopaqueDataImplementation(Octet** inData, size_t bufferSize, size_t dataSize)
  {
    // FIXME no clue if this is right!!
    if (!dataSize) {
      _buffer.clear();
    } else {
      set(*inData, dataSize);
    }
    if (*inData)
      delete *inData;
    inData = 0;
  }
  HLAopaqueDataImplementation(const HLAopaqueDataImplementation& rhs) :
    _buffer(rhs._buffer)
  {
  }
  ~HLAopaqueDataImplementation()
  {
  }

  void encodeInto(std::vector<Octet>& buffer) const
  {
    align(buffer, 4);
    size_t length = _buffer.size();
    if (0xffffffffu < length)
      throw EncoderException(L"HLAopaqueData::encodeInto(): array size is too big to encode!");
    buffer.push_back(uint8_t(length >> 24));
    buffer.push_back(uint8_t(length >> 16));
    buffer.push_back(uint8_t(length >> 8));
    buffer.push_back(uint8_t(length));

    buffer.insert(buffer.end(), _buffer.begin(), _buffer.end());
  }

  size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
  {
    index = align(index, 4);
    if (buffer.size() < index + 4)
      throw EncoderException(L"HLAopaqueData::decodeFrom(): Insufficient buffer size for decoding!");
    size_t length = size_t(uint8_t(buffer[index])) << 24;
    length |= size_t(uint8_t(buffer[index + 1])) << 16;
    length |= size_t(uint8_t(buffer[index + 2])) << 8;
    length |= size_t(uint8_t(buffer[index + 3]));
    index = index + 4;

    std::vector<Octet>::const_iterator b = buffer.begin();
    std::advance(b, index);
    std::vector<Octet>::const_iterator e = b;
    std::advance(e, length);
    _buffer.resize(0);
    _buffer.insert(_buffer.end(), b, e);

    return index + length;
  }

  void setDataPointer(Octet** inData, size_t bufferSize, size_t dataSize)
  {
    // FIXME no clue if this is right!!
    if (!dataSize) {
      _buffer.clear();
    } else {
      set(*inData, dataSize);
    }
    if (*inData)
      delete *inData;
    inData = 0;
  }

  void set(const Octet* inData, size_t dataSize)
  {
    _buffer.reserve(dataSize);
    if (!dataSize)
      return;
    _buffer.assign(inData, inData + dataSize);
  }

  const Octet* get() const
  {
    if (_buffer.empty())
      return 0;
    return &_buffer.front();
  }

  std::vector<Octet> _buffer;
};

HLAopaqueData::HLAopaqueData() :
  _impl(new HLAopaqueDataImplementation())
{
}

HLAopaqueData::HLAopaqueData(const Octet* inData, size_t dataSize) :
  _impl(new HLAopaqueDataImplementation(inData, dataSize))
{
}

HLAopaqueData::HLAopaqueData(Octet** inData, size_t bufferSize, size_t dataSize)
  RTI_THROW ((EncoderException)) :
  _impl(new HLAopaqueDataImplementation(inData, bufferSize, dataSize))
{
}

HLAopaqueData::HLAopaqueData(HLAopaqueData const & rhs) :
  _impl(new HLAopaqueDataImplementation(*rhs._impl))
{
}

HLAopaqueData::~HLAopaqueData()
{
  delete _impl;
  _impl = 0;
}

RTI_UNIQUE_PTR<DataElement>
HLAopaqueData::clone () const
{
  return RTI_UNIQUE_PTR<rti1516e::DataElement>(new HLAopaqueData(*this));
}

VariableLengthData
HLAopaqueData::encode () const
  RTI_THROW ((EncoderException))
{
  VariableLengthData variableLengthData;
  encode(variableLengthData);
  return variableLengthData;
}

void
HLAopaqueData::encode(VariableLengthData& inData) const
  RTI_THROW ((EncoderException))
{
  std::vector<Octet> buffer;
  buffer.reserve(getEncodedLength());
  encodeInto(buffer);
  inData.setData(&buffer.front(), buffer.size());
}

void
HLAopaqueData::encodeInto(std::vector<Octet>& buffer) const
  RTI_THROW ((EncoderException))
{
  _impl->encodeInto(buffer);
}

void HLAopaqueData::decode(VariableLengthData const & inData)
  RTI_THROW ((EncoderException))
{
  std::vector<Octet> buffer(inData.size());
  if (inData.size())
    std::memcpy(&buffer.front(), inData.data(), inData.size());
  decodeFrom(buffer, 0);
}

size_t
HLAopaqueData::decodeFrom(std::vector<Octet> const & buffer, size_t index)
  RTI_THROW ((EncoderException))
{
  return _impl->decodeFrom(buffer, index);
}

size_t
HLAopaqueData::getEncodedLength() const
  RTI_THROW ((EncoderException))
{
  return 4 + _impl->_buffer.size();
}

unsigned int
HLAopaqueData::getOctetBoundary() const
{
  return 4;
}

size_t
HLAopaqueData::bufferLength() const
{
  return _impl->_buffer.size();
}

size_t
HLAopaqueData::dataLength() const
{
  return _impl->_buffer.size();
}

void
HLAopaqueData::setDataPointer(Octet** inData, size_t bufferSize, size_t dataSize)
  RTI_THROW ((EncoderException))
{
  _impl->setDataPointer(inData, bufferSize, dataSize);
}

void
HLAopaqueData::set(const Octet* inData, size_t dataSize)
{
  _impl->set(inData, dataSize);
}

const Octet*
HLAopaqueData::get() const
{
  return _impl->get();
}

HLAopaqueData::operator const Octet*() const
{
  return _impl->get();
}

}
