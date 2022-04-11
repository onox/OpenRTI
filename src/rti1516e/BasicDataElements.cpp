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

#include <RTI/encoding/BasicDataElements.h>

#include <RTI/VariableLengthData.h>
#include <RTI/encoding/DataElement.h>

#include <cstring>
#include <limits>
#include <vector>

#include "Encoding.h"
#include "Export.h"
#include "Types.h"
#include "VariableLengthDataFriend.h"

namespace rti1516e
{

#define IMPLEMENT_ENCODING_HELPER_CLASS(EncodableDataType, SimpleDataType, encoder) \
                                                                        \
class OPENRTI_LOCAL EncodableDataType##Implementation {                 \
public:                                                                 \
  EncodableDataType##Implementation() :                                 \
    _value()                                                            \
  { }                                                                   \
  EncodableDataType##Implementation(const SimpleDataType& value) :      \
    _value(value)                                                       \
  { }                                                                   \
  EncodableDataType##Implementation(const EncodableDataType##Implementation& rhs) : \
    _value(rhs._value)                                                  \
  { }                                                                   \
  virtual ~EncodableDataType##Implementation()                          \
  { }                                                                   \
                                                                        \
  EncodableDataType##Implementation&                                    \
  operator=(const EncodableDataType##Implementation& rhs)               \
  { _value = rhs._value; return *this; }                                \
                                                                        \
  encoder                                                               \
                                                                        \
  void set(const SimpleDataType& inData)                                \
  {                                                                     \
    _value = inData;                                                    \
  }                                                                     \
                                                                        \
  const SimpleDataType& get() const                                     \
  {                                                                     \
    return _value;                                                      \
  }                                                                     \
                                                                        \
  size_t align(size_t offset, size_t alignment) const                   \
  { return (offset + alignment - 1) & ~(alignment - 1); }               \
                                                                        \
  void align(std::vector<Octet>& buffer, size_t alignment) const        \
  {                                                                     \
    while (buffer.size() % alignment)                                   \
      buffer.push_back(0);                                              \
  }                                                                     \
                                                                        \
  SimpleDataType _value;                                                \
};                                                                      \
                                                                        \
EncodableDataType::EncodableDataType() :                                \
  _impl(new EncodableDataType##Implementation)                          \
{                                                                       \
}                                                                       \
                                                                        \
EncodableDataType::EncodableDataType(SimpleDataType const& inData) :    \
  _impl(new EncodableDataType##Implementation(inData))                  \
{                                                                       \
}                                                                       \
                                                                        \
EncodableDataType::EncodableDataType(SimpleDataType* inData) :          \
  _impl(new EncodableDataType##Implementation(*inData))                 \
{                                                                       \
  delete inData;                                                        \
}                                                                       \
                                                                        \
EncodableDataType::EncodableDataType(EncodableDataType const& rhs) :    \
  _impl(new EncodableDataType##Implementation(*rhs._impl))              \
{                                                                       \
}                                                                       \
                                                                        \
EncodableDataType::~EncodableDataType()                                 \
{                                                                       \
  delete _impl;                                                         \
  _impl = 0;                                                            \
}                                                                       \
                                                                        \
EncodableDataType&                                                      \
EncodableDataType::operator=(EncodableDataType const& rhs)              \
{                                                                       \
  *_impl = *rhs._impl;                                                  \
  return *this;                                                         \
}                                                                       \
                                                                        \
RTI_UNIQUE_PTR<DataElement>                                             \
EncodableDataType::clone() const                                        \
{                                                                       \
  return RTI_UNIQUE_PTR<DataElement>(new EncodableDataType(*this));     \
}                                                                       \
                                                                        \
VariableLengthData                                                      \
EncodableDataType::encode() const                                       \
  RTI_THROW ((EncoderException))                                        \
{                                                                       \
  VariableLengthData variableLengthData;                                \
  encode(variableLengthData);                                           \
  return variableLengthData;                                            \
}                                                                       \
                                                                        \
void                                                                    \
EncodableDataType::encode(VariableLengthData& inData) const             \
  RTI_THROW ((EncoderException))                                        \
{                                                                       \
  _impl->encode(VariableLengthDataFriend::writePointer(inData));        \
}                                                                       \
                                                                        \
void                                                                    \
EncodableDataType::encodeInto(std::vector<Octet>& buffer) const         \
  RTI_THROW ((EncoderException))                                        \
{                                                                       \
  return _impl->encodeInto(buffer);                                     \
}                                                                       \
                                                                        \
void                                                                    \
EncodableDataType::decode(VariableLengthData const & inData)            \
  RTI_THROW ((EncoderException))                                        \
{                                                                       \
  _impl->decode(VariableLengthDataFriend::readPointer(inData));         \
}                                                                       \
                                                                        \
size_t                                                                  \
EncodableDataType::decodeFrom(std::vector<Octet> const & buffer, size_t index) \
  RTI_THROW ((EncoderException))                                        \
{                                                                       \
  return _impl->decodeFrom(buffer, index);                              \
}                                                                       \
                                                                        \
size_t                                                                  \
EncodableDataType::getEncodedLength() const                             \
  RTI_THROW ((EncoderException))                                        \
{                                                                       \
  return _impl->getEncodedLength();                                     \
}                                                                       \
                                                                        \
unsigned int                                                            \
EncodableDataType::getOctetBoundary () const                            \
{                                                                       \
  return _impl->getOctetBoundary();                                     \
}                                                                       \
                                                                        \
Integer64                                                               \
EncodableDataType::hash() const                                         \
{                                                                       \
  return _impl->hash();                                                 \
}                                                                       \
                                                                        \
void                                                                    \
EncodableDataType::setDataPointer(SimpleDataType* inData)               \
  RTI_THROW ((EncoderException))                                        \
{                                                                       \
}                                                                       \
                                                                        \
void                                                                    \
EncodableDataType::set(SimpleDataType inData)                           \
{                                                                       \
  _impl->set(inData);                                                   \
}                                                                       \
                                                                        \
SimpleDataType                                                          \
EncodableDataType::get() const                                          \
{                                                                       \
  return _impl->get();                                                  \
}                                                                       \
                                                                        \
EncodableDataType&                                                      \
EncodableDataType::operator=(SimpleDataType rhs)                        \
{                                                                       \
  _impl->set(rhs);                                                      \
  return *this;                                                         \
}                                                                       \
                                                                        \
EncodableDataType::operator SimpleDataType() const                      \
{                                                                       \
  return _impl->get();                                                  \
}

// 8 bit values
IMPLEMENT_ENCODING_HELPER_CLASS(HLAASCIIchar, char,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getChar(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setChar(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  if (buffer.size() < index + 1)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = buffer[index];
  return index + 1;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  buffer.push_back(_value);
}

size_t
getEncodedLength() const
{
  return 1;
}

unsigned int
getOctetBoundary() const
{
  return 1;
}

Integer64 hash() const
{
  return Integer64(_value);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAboolean, bool,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = bool(inData.getInt32BE(0));
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setInt32BE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 4);
  if (buffer.size() < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint32_t u;
  u = uint32_t(uint8_t(buffer[index])) << 24;
  u |= uint32_t(uint8_t(buffer[index + 1])) << 16;
  u |= uint32_t(uint8_t(buffer[index + 2])) << 8;
  u |= uint32_t(uint8_t(buffer[index + 3]));
  _value = bool(u);
  return index + 4;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 4);
  uint32_t u = uint32_t(_value);
  buffer.push_back(uint8_t(u >> 24));
  buffer.push_back(uint8_t(u >> 16));
  buffer.push_back(uint8_t(u >> 8));
  buffer.push_back(uint8_t(u));
}

size_t
getEncodedLength() const
{
  return 4;
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  return Integer64(_value);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAbyte, Octet,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getInt8(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setInt8(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  if (buffer.size() < index + 1)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = buffer[index];
  return index + 1;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  buffer.push_back(_value);
}

size_t
getEncodedLength() const
{
  return 1;
}

unsigned int
getOctetBoundary() const
{
  return 1;
}

Integer64 hash() const
{
  return Integer64(_value);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAoctet, Octet,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getInt8(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setInt8(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  if (buffer.size() < index + 1)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = buffer[index];
  return index + 1;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  buffer.push_back(_value);
}

size_t
getEncodedLength() const
{
  return 1;
}

unsigned int
getOctetBoundary() const
{
  return 1;
}

Integer64 hash() const
{
  return Integer64(_value);
}
)

// > 8 bit values, fixed endianess
IMPLEMENT_ENCODING_HELPER_CLASS(HLAunicodeChar, wchar_t,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getUInt16BE(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setUInt16BE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 2);
  if (buffer.size() < index + 2)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint16_t u;
  u = uint16_t(uint8_t(buffer[index])) << 8;
  u |= uint16_t(uint8_t(buffer[index + 1]));
  _value = wchar_t(u);
  return index + 2;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 2);
  uint16_t u = uint16_t(_value);
  buffer.push_back(uint8_t(u >> 8));
  buffer.push_back(uint8_t(u));
}

size_t
getEncodedLength() const
{
  return 2;
}

unsigned int
getOctetBoundary() const
{
  return 2;
}

Integer64 hash() const
{
  return Integer64(_value);
}
)

// > 8 bit values, both endianess
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger16BE, Integer16,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getInt16BE(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setInt16BE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 2);
  if (buffer.size() < index + 2)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint16_t u;
  u = uint16_t(uint8_t(buffer[index])) << 8;
  u |= uint16_t(uint8_t(buffer[index + 1]));
  _value = Integer16(u);
  return index + 2;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 2);
  uint16_t u = _value;
  buffer.push_back(uint8_t(u >> 8));
  buffer.push_back(uint8_t(u));
}

size_t
getEncodedLength() const
{
  return 2;
}

unsigned int
getOctetBoundary() const
{
  return 2;
}

Integer64 hash() const
{
  return Integer64(_value);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger16LE, Integer16,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getInt16LE(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setInt16LE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 2);
  if (buffer.size() < index + 2)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint16_t u;
  u = uint16_t(uint8_t(buffer[index]));
  u |= uint16_t(uint8_t(buffer[index + 1])) << 8;
  _value = Integer16(u);
  return index + 2;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 2);
  uint16_t u = uint16_t(_value);
  buffer.push_back(uint8_t(u));
  buffer.push_back(uint8_t(u >> 8));
}

size_t
getEncodedLength() const
{
  return 2;
}

unsigned int
getOctetBoundary() const
{
  return 2;
}

Integer64 hash() const
{
  return Integer64(_value);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger32BE, Integer32,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getInt32BE(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setInt32BE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 4);
  if (buffer.size() < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint32_t u;
  u = uint32_t(uint8_t(buffer[index])) << 24;
  u |= uint32_t(uint8_t(buffer[index + 1])) << 16;
  u |= uint32_t(uint8_t(buffer[index + 2])) << 8;
  u |= uint32_t(uint8_t(buffer[index + 3]));
  _value = Integer32(u);
  return index + 4;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 4);
  uint32_t u = uint32_t(_value);
  buffer.push_back(uint8_t(u >> 24));
  buffer.push_back(uint8_t(u >> 16));
  buffer.push_back(uint8_t(u >> 8));
  buffer.push_back(uint8_t(u));
}

size_t
getEncodedLength() const
{
  return 4;
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  return Integer64(_value);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger32LE, Integer32,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getInt32LE(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setInt32LE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 4);
  if (buffer.size() < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint32_t u;
  u = uint32_t(uint8_t(buffer[index]));
  u |= uint32_t(uint8_t(buffer[index + 1])) << 8;
  u |= uint32_t(uint8_t(buffer[index + 2])) << 16;
  u |= uint32_t(uint8_t(buffer[index + 3])) << 24;
  _value = Integer32(u);
  return index + 4;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 4);
  uint32_t u = uint32_t(_value);
  buffer.push_back(uint8_t(u));
  buffer.push_back(uint8_t(u >> 8));
  buffer.push_back(uint8_t(u >> 16));
  buffer.push_back(uint8_t(u >> 24));
}

size_t
getEncodedLength() const
{
  return 4;
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  return Integer64(_value);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger64BE, Integer64,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getInt64BE(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setInt64BE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 8);
  if (buffer.size() < index + 8)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint64_t u;
  u = uint64_t(uint8_t(buffer[index])) << 56;
  u |= uint64_t(uint8_t(buffer[index + 1])) << 48;
  u |= uint64_t(uint8_t(buffer[index + 2])) << 40;
  u |= uint64_t(uint8_t(buffer[index + 3])) << 32;
  u |= uint64_t(uint8_t(buffer[index + 4])) << 24;
  u |= uint64_t(uint8_t(buffer[index + 5])) << 16;
  u |= uint64_t(uint8_t(buffer[index + 6])) << 8;
  u |= uint64_t(uint8_t(buffer[index + 7]));
  _value = Integer64(u);
  return index + 8;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 8);
  uint64_t u = uint64_t(_value);
  buffer.push_back(uint8_t(u >> 56));
  buffer.push_back(uint8_t(u >> 48));
  buffer.push_back(uint8_t(u >> 40));
  buffer.push_back(uint8_t(u >> 32));
  buffer.push_back(uint8_t(u >> 24));
  buffer.push_back(uint8_t(u >> 16));
  buffer.push_back(uint8_t(u >> 8));
  buffer.push_back(uint8_t(u));
}

size_t
getEncodedLength() const
{
  return 8;
}

unsigned int
getOctetBoundary() const
{
  return 8;
}

Integer64 hash() const
{
  return Integer64(_value);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger64LE, Integer64,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getInt64LE(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setInt64LE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 8);
  if (buffer.size() < index + 8)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint64_t u;
  u = uint64_t(uint8_t(buffer[index]));
  u |= uint64_t(uint8_t(buffer[index + 1])) << 8;
  u |= uint64_t(uint8_t(buffer[index + 2])) << 16;
  u |= uint64_t(uint8_t(buffer[index + 3])) << 24;
  u |= uint64_t(uint8_t(buffer[index + 4])) << 32;
  u |= uint64_t(uint8_t(buffer[index + 5])) << 40;
  u |= uint64_t(uint8_t(buffer[index + 6])) << 48;
  u |= uint64_t(uint8_t(buffer[index + 7])) << 56;
  _value = Integer64(u);
  return index + 8;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 8);
  uint64_t u = uint64_t(_value);
  buffer.push_back(uint8_t(u));
  buffer.push_back(uint8_t(u >> 8));
  buffer.push_back(uint8_t(u >> 16));
  buffer.push_back(uint8_t(u >> 24));
  buffer.push_back(uint8_t(u >> 32));
  buffer.push_back(uint8_t(u >> 40));
  buffer.push_back(uint8_t(u >> 48));
  buffer.push_back(uint8_t(u >> 56));
}

size_t
getEncodedLength() const
{
  return 8;
}

unsigned int
getOctetBoundary() const
{
  return 8;
}

Integer64 hash() const
{
  return Integer64(_value);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAoctetPairBE, OctetPair,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value.first = inData.getChar(0);
  _value.second = inData.getChar(1);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setChar(_value.first, 0);
  inData.setChar(_value.second, 1);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 2);
  if (buffer.size() < index + 2)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value.first = buffer[index];
  _value.second = buffer[index + 1];
  return index + 2;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 2);
  buffer.push_back(_value.first);
  buffer.push_back(_value.second);
}

size_t
getEncodedLength() const
{
  return 2;
}

unsigned int
getOctetBoundary() const
{
  return 2;
}

Integer64 hash() const
{
  return (Integer64(_value.first) << 8) | Integer64(_value.second);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAoctetPairLE, OctetPair,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value.second = inData.getChar(0);
  _value.first = inData.getChar(1);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setChar(_value.second, 0);
  inData.setChar(_value.first, 1);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 2);
  if (buffer.size() < index + 2)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value.second = buffer[index];
  _value.first = buffer[index + 1];
  return index + 2;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 2);
  buffer.push_back(_value.second);
  buffer.push_back(_value.first);
}

size_t
getEncodedLength() const
{
  return 2;
}

unsigned int
getOctetBoundary() const
{
  return 2;
}

Integer64 hash() const
{
  return (Integer64(_value.first) << 8) | Integer64(_value.second);
}
)

// float, > 8 bit values, both endianess
IMPLEMENT_ENCODING_HELPER_CLASS(HLAfloat32BE, float,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getFloat32BE(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setFloat32BE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 4);
  if (buffer.size() < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  union {
    uint32_t u;
    float s;
  } u;
  u.u = uint32_t(uint8_t(buffer[index])) << 24;
  u.u |= uint32_t(uint8_t(buffer[index + 1])) << 16;
  u.u |= uint32_t(uint8_t(buffer[index + 2])) << 8;
  u.u |= uint32_t(uint8_t(buffer[index + 3]));
  _value = u.s;
  return index + 4;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 4);
  union {
    uint32_t u;
    float s;
  } u;
  u.s = _value;
  buffer.push_back(uint8_t(u.u >> 24));
  buffer.push_back(uint8_t(u.u >> 16));
  buffer.push_back(uint8_t(u.u >> 8));
  buffer.push_back(uint8_t(u.u));
}

size_t
getEncodedLength() const
{
  return 4;
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  union {
    uint32_t u;
    float s;
  } u;
  u.s = _value;
  return Integer64(u.u);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAfloat32LE, float,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getFloat32LE(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setFloat32LE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 4);
  if (buffer.size() < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  union {
    uint32_t u;
    float s;
  } u;
  u.u = uint32_t(uint8_t(buffer[index]));
  u.u |= uint32_t(uint8_t(buffer[index + 1])) << 8;
  u.u |= uint32_t(uint8_t(buffer[index + 2])) << 16;
  u.u |= uint32_t(uint8_t(buffer[index + 3])) << 24;
  _value = u.s;
  return index + 4;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 4);
  union {
    uint32_t u;
    float s;
  } u;
  u.s = _value;
  buffer.push_back(uint8_t(u.u));
  buffer.push_back(uint8_t(u.u >> 8));
  buffer.push_back(uint8_t(u.u >> 16));
  buffer.push_back(uint8_t(u.u >> 24));
}

size_t
getEncodedLength() const
{
  return 4;
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  union {
    uint32_t u;
    float s;
  } u;
  u.s = _value;
  return Integer64(u.u);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAfloat64BE, double,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getFloat64BE(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setFloat64BE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 8);
  if (buffer.size() < index + 8)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  union {
    uint64_t u;
    double s;
  } u;
  u.u = uint64_t(uint8_t(buffer[index])) << 56;
  u.u |= uint64_t(uint8_t(buffer[index + 1])) << 48;
  u.u |= uint64_t(uint8_t(buffer[index + 2])) << 40;
  u.u |= uint64_t(uint8_t(buffer[index + 3])) << 32;
  u.u |= uint64_t(uint8_t(buffer[index + 4])) << 24;
  u.u |= uint64_t(uint8_t(buffer[index + 5])) << 16;
  u.u |= uint64_t(uint8_t(buffer[index + 6])) << 8;
  u.u |= uint64_t(uint8_t(buffer[index + 7]));
  _value = u.s;
  return index + 8;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 8);
  union {
    uint64_t u;
    double s;
  } u;
  u.s = _value;
  buffer.push_back(uint8_t(u.u >> 56));
  buffer.push_back(uint8_t(u.u >> 48));
  buffer.push_back(uint8_t(u.u >> 40));
  buffer.push_back(uint8_t(u.u >> 32));
  buffer.push_back(uint8_t(u.u >> 24));
  buffer.push_back(uint8_t(u.u >> 16));
  buffer.push_back(uint8_t(u.u >> 8));
  buffer.push_back(uint8_t(u.u));
}

size_t
getEncodedLength() const
{
  return 8;
}

unsigned int
getOctetBoundary() const
{
  return 8;
}

Integer64 hash() const
{
  union {
    uint64_t u;
    double s;
  } u;
  u.s = _value;
  return Integer64(u.u);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAfloat64LE, double,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() != getEncodedLength())
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value = inData.getFloat64LE(0);
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  inData.resize(getEncodedLength());
  inData.setFloat64LE(_value, 0);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 8);
  if (buffer.size() < index + 8)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  union {
    uint64_t u;
    double s;
  } u;
  u.u = uint64_t(uint8_t(buffer[index]));
  u.u |= uint64_t(uint8_t(buffer[index + 1])) << 8;
  u.u |= uint64_t(uint8_t(buffer[index + 2])) << 16;
  u.u |= uint64_t(uint8_t(buffer[index + 3])) << 24;
  u.u |= uint64_t(uint8_t(buffer[index + 4])) << 32;
  u.u |= uint64_t(uint8_t(buffer[index + 5])) << 40;
  u.u |= uint64_t(uint8_t(buffer[index + 6])) << 48;
  u.u |= uint64_t(uint8_t(buffer[index + 7])) << 56;
  _value = u.s;
  return index + 8;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 8);
  union {
    uint64_t u;
    double s;
  } u;
  u.s = _value;
  buffer.push_back(uint8_t(u.u));
  buffer.push_back(uint8_t(u.u >> 8));
  buffer.push_back(uint8_t(u.u >> 16));
  buffer.push_back(uint8_t(u.u >> 24));
  buffer.push_back(uint8_t(u.u >> 32));
  buffer.push_back(uint8_t(u.u >> 40));
  buffer.push_back(uint8_t(u.u >> 48));
  buffer.push_back(uint8_t(u.u >> 56));
}

size_t
getEncodedLength() const
{
  return 8;
}

unsigned int
getOctetBoundary() const
{
  return 8;
}

Integer64 hash() const
{
  union {
    uint64_t u;
    double s;
  } u;
  u.s = _value;
  return Integer64(u.u);
}
)

// Array values
IMPLEMENT_ENCODING_HELPER_CLASS(HLAASCIIstring, std::string,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() < 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");

  Integer32 length = inData.getInt32BE(0);
  if (length < 0)
    length = 0;

  if (inData.size() < 4 + length)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value.clear();
  _value.reserve(length);
  for (Integer32 i = 0; i < length; ++i)
    _value.push_back(inData.getChar(4 + i));
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  size_t length = _value.size();
  if (std::numeric_limits<Integer32>::max() < length)
    length = std::numeric_limits<Integer32>::max();
  inData.resize(length + 4);
  inData.setInt32BE(int32_t(length), 0);

  for (size_t i = 0; i < length; ++i)
    inData.setChar(_value[i], 4 + i);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 4);
  if (buffer.size() < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint32_t u;
  u = uint32_t(uint8_t(buffer[index])) << 24;
  u |= uint32_t(uint8_t(buffer[index + 1])) << 16;
  u |= uint32_t(uint8_t(buffer[index + 2])) << 8;
  u |= uint32_t(uint8_t(buffer[index + 3]));
  Integer32 length = Integer32(u);
  if (length < 0)
    length = 0;
  index += 4;

  if (buffer.size() < index + length)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value.clear();
  _value.reserve(length);
  while (0 < length--)
    _value.push_back(buffer[index++]);

  return index;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 4);
  size_t length = _value.size();
  if (std::numeric_limits<Integer32>::max() < length)
    length = std::numeric_limits<Integer32>::max();
  uint32_t u = uint32_t(length);
  buffer.push_back(uint8_t(u >> 24));
  buffer.push_back(uint8_t(u >> 16));
  buffer.push_back(uint8_t(u >> 8));
  buffer.push_back(uint8_t(u));

  for (size_t i = 0; i < length; ++i)
    buffer.push_back(_value[i]);
}

size_t
getEncodedLength() const
{
  return 4 + _value.size();
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  Integer64 hash = 8191;
  for (std::string::const_iterator i = _value.begin(); i != _value.end(); ++i)
    hash = hash*Integer64(127) + Integer64(*i);
  return hash;
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAunicodeString, std::wstring,
void
decode(OpenRTI::VariableLengthData const & inData)
{
  if (inData.size() < 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");

  Integer32 length = inData.getInt32BE(0);
  if (length < 0)
    length = 0;

  if (inData.size() < 4 + 2*length)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value.clear();
  _value.reserve(length);
  for (Integer32 i = 0; i < length; ++i)
    _value.push_back(inData.getUInt16BE(4 + 2*i));
}

void
encode(OpenRTI::VariableLengthData& inData) const
{
  size_t length = _value.size();
  if (std::numeric_limits<Integer32>::max() < length)
    length = std::numeric_limits<Integer32>::max();
  inData.resize(2*length + 4);
  inData.setInt32BE(int32_t(length), 0);

  for (size_t i = 0; i < length; ++i)
    inData.setUInt16BE(_value[i], 4 + 2*i);
}

size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  index = align(index, 4);
  if (buffer.size() < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint32_t u;
  u = uint32_t(uint8_t(buffer[index])) << 24;
  u |= uint32_t(uint8_t(buffer[index + 1])) << 16;
  u |= uint32_t(uint8_t(buffer[index + 2])) << 8;
  u |= uint32_t(uint8_t(buffer[index + 3]));
  Integer32 length = Integer32(u);
  if (length < 0)
    length = 0;
  index += 4;

  if (buffer.size() < index + 2*length)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _value.clear();
  _value.reserve(length);
  while (0 < length--) {
    uint16_t u;
    u = uint16_t(uint8_t(buffer[index])) << 8;
    u |= uint16_t(uint8_t(buffer[index + 1]));
    _value.push_back(wchar_t(u));
    index += 2;
  }

  return index;
}

void encodeInto(std::vector<Octet>& buffer) const
{
  align(buffer, 4);
  size_t length = _value.size();
  if (std::numeric_limits<Integer32>::max() < length)
    length = std::numeric_limits<Integer32>::max();
  uint32_t u = uint32_t(length);
  buffer.push_back(uint8_t(u >> 24));
  buffer.push_back(uint8_t(u >> 16));
  buffer.push_back(uint8_t(u >> 8));
  buffer.push_back(uint8_t(u));

  for (size_t i = 0; i < length; ++i) {
    uint16_t u = uint16_t(_value[i]);
    buffer.push_back(uint8_t(u >> 8));
    buffer.push_back(uint8_t(u));
  }
}

size_t
getEncodedLength() const
{
  return 4 + 2*_value.size();
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  Integer64 hash = 8191;
  for (std::wstring::const_iterator i = _value.begin(); i != _value.end(); ++i)
    hash = hash*Integer64(127) + Integer64(*i);
  return hash;
}
)

}
