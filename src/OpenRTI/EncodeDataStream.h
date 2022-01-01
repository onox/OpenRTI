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

#ifndef OpenRTI_EncodeDataStream_h
#define OpenRTI_EncodeDataStream_h

#include "VariableLengthData.h"
#include "Types.h"

namespace OpenRTI {

class OPENRTI_API EncodeDataStream {
public:
  EncodeDataStream(VariableLengthData& variableLengthData) :
    _variableLengthData(variableLengthData),
    _offset(0)
  {
    _variableLengthData.resize(0);
    _variableLengthData.ensurePrivate();
  }

  size_t size() const
  { return _variableLengthData.size(); }

  size_t offset() const
  { return _offset; }

  void trunc()
  {
    _offset = 0;
    _variableLengthData.resize(0);
  }

  void seek(size_t offset)
  {
    _offset = offset;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
  }

  void align(size_t alignSize)
  {
    if (!alignSize)
      return;
    size_t offset = ((_offset + alignSize - 1)/alignSize)*alignSize;
// #if defined(NDEBUG) || defined(_NDEBUG)
//     // Just seeking will make valgrind cry
//     seek(offset);
// #else
    while (_offset < offset)
      writeUInt8BE(0xff);
// #endif
  }

  void writeUInt8LE(uint8_t value)
  {
    size_t offset = _offset;
    _offset += 1;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setUInt8LE(value, offset);
  }
  void writeUInt8BE(uint8_t value)
  {
    size_t offset = _offset;
    _offset += 1;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setUInt8BE(value, offset);
  }
  void writeAlignedUInt8LE(uint8_t value)
  {
    size_t offset = _offset;
    _offset += 1;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedUInt8LE(value, offset);
  }
  void writeAlignedUInt8BE(uint8_t value)
  {
    size_t offset = _offset;
    _offset += 1;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedUInt8BE(value, offset);
  }

  void writeInt8LE(int8_t value)
  {
    size_t offset = _offset;
    _offset += 1;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setInt8LE(value, offset);
  }
  void writeInt8BE(int8_t value)
  {
    size_t offset = _offset;
    _offset += 1;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setInt8BE(value, offset);
  }
  void writeAlignedInt8LE(int8_t value)
  {
    size_t offset = _offset;
    _offset += 1;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedInt8LE(value, offset);
  }
  void writeAlignedInt8BE(int8_t value)
  {
    size_t offset = _offset;
    _offset += 1;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedInt8BE(value, offset);
  }


  void writeUInt16LE(uint16_t value)
  {
    size_t offset = _offset;
    _offset += 2;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setUInt16LE(value, offset);
  }
  void writeUInt16BE(uint16_t value)
  {
    size_t offset = _offset;
    _offset += 2;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setUInt16BE(value, offset);
  }
  void writeAlignedUInt16LE(uint16_t value)
  {
    size_t offset = _offset;
    _offset += 2;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedUInt16LE(value, offset);
  }
  void writeAlignedUInt16BE(uint16_t value)
  {
    size_t offset = _offset;
    _offset += 2;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedUInt16BE(value, offset);
  }

  void writeInt16LE(int16_t value)
  {
    size_t offset = _offset;
    _offset += 2;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setInt16LE(value, offset);
  }
  void writeInt16BE(int16_t value)
  {
    size_t offset = _offset;
    _offset += 2;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setInt16BE(value, offset);
  }
  void writeAlignedInt16LE(int16_t value)
  {
    size_t offset = _offset;
    _offset += 2;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedInt16LE(value, offset);
  }
  void writeAlignedInt16BE(int16_t value)
  {
    size_t offset = _offset;
    _offset += 2;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedInt16BE(value, offset);
  }


  void writeUInt32LE(uint32_t value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setUInt32LE(value, offset);
  }
  void writeUInt32BE(uint32_t value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setUInt32BE(value, offset);
  }
  void writeAlignedUInt32LE(uint32_t value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedUInt32LE(value, offset);
  }
  void writeAlignedUInt32BE(uint32_t value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedUInt32BE(value, offset);
  }

  void writeInt32LE(int32_t value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setInt32LE(value, offset);
  }
  void writeInt32BE(int32_t value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setInt32BE(value, offset);
  }
  void writeAlignedInt32LE(int32_t value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedInt32LE(value, offset);
  }
  void writeAlignedInt32BE(int32_t value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedInt32BE(value, offset);
  }


  void writeUInt64LE(uint64_t value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setUInt64LE(value, offset);
  }
  void writeUInt64BE(uint64_t value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setUInt64BE(value, offset);
  }
  void writeAlignedUInt64LE(uint64_t value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedUInt64LE(value, offset);
  }
  void writeAlignedUInt64BE(uint64_t value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedUInt64BE(value, offset);
  }

  void writeInt64LE(int64_t value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setInt64LE(value, offset);
  }
  void writeInt64BE(int64_t value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setInt64BE(value, offset);
  }
  void writeAlignedInt64LE(int64_t value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedInt64LE(value, offset);
  }
  void writeAlignedInt64BE(int64_t value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setInt64BE(value, offset);
  }

  void writeFloat32LE(float value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setFloat32LE(value, offset);
  }
  void writeFloat32BE(float value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setFloat32BE(value, offset);
  }
  void writeAlignedFloat32LE(float value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedFloat32LE(value, offset);
  }
  void writeAlignedFloat32BE(float value)
  {
    size_t offset = _offset;
    _offset += 4;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedFloat32BE(value, offset);
  }

  void writeFloat64LE(double value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setFloat64LE(value, offset);
  }
  void writeFloat64BE(double value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setFloat64BE(value, offset);
  }
  void writeAlignedFloat64LE(double value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setAlignedFloat64LE(value, offset);
  }
  void writeAlignedFloat64BE(double value)
  {
    size_t offset = _offset;
    _offset += 8;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    _variableLengthData.setFloat64BE(value, offset);
  }

  // Rethink the below FIXME

  void writeBool(bool value)
  { writeUInt8BE(value); }

  void writeUInt8Compressed(uint8_t value)
  { writeTemplateCompressed(value); }
  void writeInt8Compressed(int8_t value)
  { writeUInt8Compressed(value); }

  void writeUInt16Compressed(uint16_t value)
  { writeTemplateCompressed(value); }
  void writeInt16Compressed(int16_t value)
  { writeUInt16Compressed(value); }

  void writeUInt32Compressed(uint32_t value)
  { writeTemplateCompressed(value); }
  void writeInt32Compressed(int32_t value)
  { writeUInt32Compressed(value); }

  void writeUInt64Compressed(uint64_t value)
  { writeTemplateCompressed(value); }
  void writeInt64Compressed(int64_t value)
  { writeUInt64Compressed(value); }

  void writeFloat32Compressed(float value)
  {
    union {
      float f;
      uint32_t u;
    } u;
    u.f = value;
    writeUInt32Compressed(u.u);
  }
  void writeFloat64Compressed(double value)
  {
    union {
      double d;
      uint64_t u;
    } u;
    u.d = value;
    writeUInt64Compressed(u.u);
  }

  void writeSizeTCompressed(size_t value)
  { writeTemplateCompressed(value); }
  void writeCharCompressed(char value)
  { writeUInt8BE(value); }
  void writeChar(char value)
  { writeUInt8BE(value); }

  void writeBoolCompressed(bool value)
  { writeBool(value); }

  void writeData(const VariableLengthData& d)
  {
    size_t offset = _offset;
    _offset += d.size();
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    if (d.size())
      std::memcpy(_variableLengthData.data(offset), d.data(), d.size());
  }

  void writeData(const void* data, size_t size)
  {
    size_t offset = _offset;
    _offset += size;
    if (_variableLengthData.size() < _offset)
      _variableLengthData.resize(_offset);
    if (size)
      std::memcpy(_variableLengthData.data(offset), data, size);
  }

private:
  // compress away leading zeros by using 7 bits as the payload and 1 bit
  // to mark kind of continuation for the same amount of data
  template<typename T>
  void writeTemplateCompressed(T value)
  {
    while (value & (~T(0x7f))) {
      writeUInt8BE(0x80 | (0x7f & uint8_t(value)));
      value >>= 7;
    }
    writeUInt8BE(uint8_t(value));
  }

  VariableLengthData& _variableLengthData;
  size_t _offset;
};

} // namespace OpenRTI

#endif
