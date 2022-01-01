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

#ifndef OpenRTI_VariableLengthData_h
#define OpenRTI_VariableLengthData_h

#include "Exception.h"
#include "Export.h"
#include "Referenced.h"
#include "SharedPtr.h"
#include "Types.h"
#include <algorithm>
#include <cstring>
#include <list>
#include <ostream>
#include <string>

namespace OpenRTI {

typedef void (*_VariableLengthDataDeleteFunction)(void*);

class VariableLengthData;
typedef std::list<VariableLengthData> VariableLengthDataList;

/// Class for an opaque byte array.
/// The class behaves similar to a std::vector as it does not reallocate when clearing or
/// resizing to a smaller size.
/// The class uses reference counting for the actual data and does copy on write.
/// Subranges of such a byte array can be handled by just referencing the same data with
/// a non zero base offset.
/// Also allocation is delayed to the point a non const data pointer is requested.
/// This is used for the network buffer handling where the decoder attaches
/// VariableLengthData just with the desired size. But the network reading code
/// assignes the structs in the most efficient way.
/// As a side effect of this, it is possible to get a zero pointer, even if the
/// the size of the array is non zero. This happens when a fresh array is resized
/// but never written. On a const access the pointer is still zero.
/// Because of this it is also possible to have capacity < size.

/// FIXME should be like follows ...
/// _data == _dummy -> nothing to delete
/// _capacity > 0 && _data != _dummy -> delete
/// _capacity == 0 && _data != _dummy -> nothing to delete

class OPENRTI_API VariableLengthData {
public:
  VariableLengthData() :
    _data(0),
    _size(0),
    _offset(0)
  { }
  VariableLengthData(size_t size) :
    _data(0),
    _size(size),
    _offset(0)
  { }
  VariableLengthData(const void* data, size_t size) :
    _data(createOwnData(size)),
    _size(size),
    _offset(0)
  { std::memcpy(_data->data(0), data, size); }
  // VariableLengthData(void* data, size_t size) :
  //   _data(createExternalData(data)),
  //   _size(size),
  //   _offset(0)
  // { }
  VariableLengthData(const char* string) :
    _data(0),
    _size(0),
    _offset(0)
  { setData(string, std::strlen(string)); }
  VariableLengthData(const std::string& s) :
    _data(0),
    _size(0),
    _offset(0)
  { setData(s.c_str(), s.size()); }
  VariableLengthData(const VariableLengthData& value) :
    _data(value._data),
    _size(value._size),
    _offset(value._offset)
  { }
#if 201103L <= __cplusplus || 200610L <= __cpp_rvalue_reference
  VariableLengthData(VariableLengthData&& value) :
    _data(std::move(value._data)),
    _size(value._size),
    _offset(value._offset)
  { }
#endif
  // Constructs a subrange of the given variable length data.
  // It references the same data than 'value', but it starts at offset in value and has the given size
  VariableLengthData(const VariableLengthData& value, size_t offset, size_t size) :
    _data(value._data),
    _size(size),
    _offset(value._offset + offset)
  { OpenRTIAssert(offset + size <= value.size()); }
  VariableLengthData(const VariableLengthDataList& variableLengthDataList) :
    _data(0),
    _size(0),
    _offset(0)
  {
    size_t size = 0;
    for (VariableLengthDataList::const_iterator i = variableLengthDataList.begin();
         i != variableLengthDataList.end(); ++i)
      size += i->size();
    reserve(size);
    for (VariableLengthDataList::const_iterator i = variableLengthDataList.begin();
         i != variableLengthDataList.end(); ++i)
      append(*i);
  }
  ~VariableLengthData()
  { }

  VariableLengthData&
  operator=(const VariableLengthData& value)
  {
    _data = value._data;
    _size = value._size;
    _offset = value._offset;
    return *this;
  }
#if 201103L <= __cplusplus || 200610L <= __cpp_rvalue_reference
  VariableLengthData&
  operator=(VariableLengthData&& value)
  {
    _data.swap(value._data);
    _size = value._size;
    _offset = value._offset;
    return *this;
  }
#endif

  const void* data(size_t offset = 0) const
  { return constData(offset); }
  const void* constData(size_t offset = 0) const
  {
    OpenRTIAssert(_data.valid() || _size == 0);
    if (!_data.valid())
      return 0;
    OpenRTIAssert(offset < _size);
    return _data->data(_offset + offset);
  }
  void* data(size_t offset = 0)
  {
    if (!_size)
      return 0;
    OpenRTIAssert(offset < _size);
    if (!_data.valid())
      reserve(_size);
    else
      ensurePrivate();
    return _data->data(_offset + offset);
  }

  const char* charData(size_t offset = 0) const
  { return static_cast<const char*>(constData(offset)); }
  char* charData(size_t offset = 0)
  { return static_cast<char*>(data(offset)); }

  const uint8_t* uint8Data(size_t offset) const
  { return static_cast<const uint8_t*>(constData(offset)); }
  uint8_t* uint8Data(size_t offset)
  { return static_cast<uint8_t*>(data(offset)); }

  size_t size() const
  { return _size; }

  size_t capacity() const
  {
    if (!_data.valid())
      return 0;
    return _data->capacity() - _offset;
  }

  bool empty() const
  { return 0 == _size; }

  void clear()
  {
    if (_data.valid())
      _data->clear();
    _size = 0;
  }

  void swap(VariableLengthData& variableLengthData)
  {
    _data.swap(variableLengthData._data);
    std::swap(_size, variableLengthData._size);
    std::swap(_offset, variableLengthData._offset);
  }

  void resize(size_t size)
  {
    if (size == _size)
      return;
    if (_data.valid()) {
      size_t cap = capacity();
      if (cap < size)
        reserve(std::max(size, 2*cap));
    }
    _size = size;
  }

  /// Allocate space for cap bytes data.
  /// This call triggers delayed allocation of the data area.
  void reserve(size_t cap)
  {
    OpenRTIAssert(_size <= cap);
    // Don't mess with too small allocations.
    cap = std::max(size_t(512), cap);
    if (!_data.valid()) {
      _data = createOwnData(cap);
    } else if (capacity() < cap) {
      SharedPtr<Data> data = createOwnData(cap);
      if (_size)
        std::memcpy(data->data(0), constData(), _size);
      _data.swap(data);
      _offset = 0;
    }
  }

  /// Make sure the data is not shared with other instances of VariableLengthData
  void ensurePrivate()
  {
    if (Data::count(_data.get()) <= 1)
      return;
    SharedPtr<Data> data = createOwnData(capacity());
    if (_size)
      std::memcpy(data->data(0), constData(), _size);
    _data.swap(data);
    _offset = 0;
  }

  /// Replace this with the data range starting from offset up to the end.
  /// This is kind of 'resize from the start'.
  void tail(size_t offset)
  {
    OpenRTIAssert(offset <= size());
    _size -= offset;
    if (_size) {
      _offset += offset;
    } else {
      // If possible detach from the data element
      _offset = 0;
      _data.clear();
    }
  }

  void
  setData(const void* p, size_t size)
  {
    // Setting size to 0 avoids needless copying of old data
    _size = 0;
    ensurePrivate();
    resize(size);
    std::memcpy(data(), p, _size);
  }

  // Caller is responsible for ensuring that the data that is
  // pointed to is valid for the lifetime of this object, or past
  // the next time this object is given new data.
  // void
  // setDataPointer(void* data, size_t size)
  // {
  //   _size = size;
  //   _offset = 0;
  //   _data = createExternalData(data);
  // }

  // Caller gives up ownership of inData to this object.
  // This object assumes the responsibility of deleting inData
  // when it is no longer needed.
  void
  takeDataPointer(void* data, size_t size, _VariableLengthDataDeleteFunction variableLengthDataDeleteFunction)
  {
    // Take over ownership of that memory area.
    // Past that, we need to delete that.
    _size = size;
    _offset = 0;
    _data = createExternalData(data, variableLengthDataDeleteFunction);
  }

  bool operator==(const VariableLengthData& variableLengthData) const
  {
    size_t s = variableLengthData.size();
    if (size() != s)
      return false;
    if (s == 0)
      return true;
    const void* d0 = data();
    const void* d1 = variableLengthData.data();
    if (d0 == d1)
      return true;
    return std::memcmp(d0, d1, s) == 0;
  }
  bool operator<(const VariableLengthData& variableLengthData) const
  {
    const char* b0 = charData();
    const char* e0 = charData() + size();
    const char* b1 = variableLengthData.charData();
    const char* e1 = variableLengthData.charData() + variableLengthData.size();
    if (b0 == b1 && e0 == e1)
      return false;
    return std::lexicographical_compare(b0, e0, b1, e1);
  }
  bool operator!=(const VariableLengthData& variableLengthData) const
  { return !operator==(variableLengthData); }
  bool operator>(const VariableLengthData& variableLengthData) const
  { return variableLengthData.operator<(*this); }
  bool operator>=(const VariableLengthData& variableLengthData) const
  { return !operator<(variableLengthData); }
  bool operator<=(const VariableLengthData& variableLengthData) const
  { return !operator>(variableLengthData); }

  /// convert the content of this into a std::string value
  std::string toString() const
  { return std::string(charData(0), size()); }

  // Set of value accessors with a common interface:
  // Each set/get pair does *unchecked* access. So make sure to have sufficient memory.
  // We always have accessors for little endian and big endian, aligned and unaligned,
  // even for byte access. This is to have more consistent encoding/decoding over all types.

  void setVariableLengthData(const VariableLengthData& variableLengthData, size_t offset)
  {
    OpenRTIAssert(offset + variableLengthData.size() <= size());
    if (offset == 0 && variableLengthData.size() == size()) {
      _data = variableLengthData._data;
      _offset = variableLengthData._offset;
    } else {
      std::memcpy(data(offset), variableLengthData.data(), variableLengthData.size());
    }
  }
  VariableLengthData getVariableLengthData(size_t offset) const
  {
    OpenRTIAssert(offset <= size());
    return VariableLengthData(*this, offset, size() - offset);
  }

  /// Append valiableLengthData to this.
  VariableLengthData& append(const VariableLengthData& variableLengthData)
  {
    size_t offset = size();
    resize(offset + variableLengthData.size());
    std::memcpy(data(offset), variableLengthData.data(), variableLengthData.size());
    return *this;
  }

  // 8 bit value accessors
  void setChar(char value, size_t offset)
  {
    OpenRTIAssert(offset + 1 <= size());
    char* data = charData(offset);
    data[0] = value;
  }
  char getChar(size_t offset) const
  {
    OpenRTIAssert(offset + 1 <= size());
    const char* data = charData(offset);
    return data[0];
  }

  void setUInt8(uint8_t value, size_t offset)
  {
    OpenRTIAssert(offset + 1 <= size());
    uint8_t* data = uint8Data(offset);
    data[0] = value;
  }
  void setUInt8LE(uint8_t value, size_t offset)
  { setUInt8(value, offset); }
  void setUInt8BE(uint8_t value, size_t offset)
  { setUInt8(value, offset); }
  void setAlignedUInt8LE(uint8_t value, size_t offset)
  { setUInt8(value, offset); }
  void setAlignedUInt8BE(uint8_t value, size_t offset)
  { setUInt8(value, offset); }

  uint8_t getUInt8(size_t offset) const
  {
    OpenRTIAssert(offset + 1 <= size());
    const uint8_t* data = uint8Data(offset);
    return data[0];
  }
  uint8_t getUInt8LE(size_t offset) const
  { return getUInt8(offset); }
  uint8_t getUInt8BE(size_t offset) const
  { return getUInt8(offset); }
  uint8_t getAlignedUInt8LE(size_t offset) const
  { return getUInt8(offset); }
  uint8_t getAlignedUInt8BE(size_t offset) const
  { return getUInt8(offset); }


  void setInt8(int8_t value, size_t offset)
  { setUInt8(value, offset); }
  void setInt8LE(int8_t value, size_t offset)
  { setUInt8LE(value, offset); }
  void setInt8BE(int8_t value, size_t offset)
  { setUInt8BE(value, offset); }
  void setAlignedInt8LE(int8_t value, size_t offset)
  { setUInt8LE(value, offset); }
  void setAlignedInt8BE(int8_t value, size_t offset)
  { setUInt8BE(value, offset); }

  int8_t getInt8(size_t offset) const
  { return getUInt8(offset); }
  int8_t getInt8LE(size_t offset) const
  { return getUInt8LE(offset); }
  int8_t getInt8BE(size_t offset) const
  { return getUInt8BE(offset); }
  int8_t getAlignedInt8LE(size_t offset) const
  { return getUInt8LE(offset); }
  int8_t getAlignedInt8BE(size_t offset) const
  { return getUInt8BE(offset); }


  // 16 bit value accessors
  void setUInt16LE(uint16_t value, size_t offset)
  {
    OpenRTIAssert(offset + 2 <= size());
    uint8_t* data = uint8Data(offset);
    data[0] = uint8_t(value);
    data[1] = uint8_t(value >> 8);
  }
  void setUInt16BE(uint16_t value, size_t offset)
  {
    OpenRTIAssert(offset + 2 <= size());
    uint8_t* data = uint8Data(offset);
    data[0] = uint8_t(value >> 8);
    data[1] = uint8_t(value);
  }
  void setAlignedUInt16LE(uint16_t value, size_t offset)
  {
    OpenRTIAssert((offset & 1) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    setUInt16LE(value, offset);
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    OpenRTIAssert(offset + 2 <= size());
    *static_cast<uint16_t*>(data(offset)) = value;
#else
#error "No endianess defined!"
#endif
  }
  void setAlignedUInt16BE(uint16_t value, size_t offset)
  {
    OpenRTIAssert((offset & 1) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    OpenRTIAssert(offset + 2 <= size());
    *static_cast<uint16_t*>(data(offset)) = value;
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    setUInt16BE(value, offset);
#else
#error "No endianess defined!"
#endif
  }

  uint16_t getUInt16LE(size_t offset) const
  {
    OpenRTIAssert(offset + 2 <= size());
    const uint8_t* data = uint8Data(offset);
    uint16_t value = uint16_t(data[0]);
    value |= uint16_t(data[1]) << 8;
    return value;
  }
  uint16_t getUInt16BE(size_t offset) const
  {
    OpenRTIAssert(offset + 2 <= size());
    const uint8_t* data = uint8Data(offset);
    uint16_t value = uint16_t(data[0]) << 8;
    value |= uint16_t(data[1]);
    return value;
  }
  uint16_t getAlignedUInt16LE(size_t offset) const
  {
    OpenRTIAssert((offset & 1) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    return getUInt16LE(offset);
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    OpenRTIAssert(offset + 2 <= size());
    return *static_cast<const uint16_t*>(data(offset));
#else
#error "No endianess defined!"
#endif
  }
  uint16_t getAlignedUInt16BE(size_t offset) const
  {
    OpenRTIAssert((offset & 1) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    OpenRTIAssert(offset + 2 <= size());
    return *static_cast<const uint16_t*>(data(offset));
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    return getUInt16BE(offset);
#else
#error "No endianess defined!"
#endif
  }


  void setInt16LE(int16_t value, size_t offset)
  { setUInt16LE(value, offset); }
  void setInt16BE(int16_t value, size_t offset)
  { setUInt16BE(value, offset); }
  void setAlignedInt16LE(int16_t value, size_t offset)
  { setAlignedUInt16LE(value, offset); }
  void setAlignedInt16BE(int16_t value, size_t offset)
  { setAlignedUInt16BE(value, offset); }

  int16_t getInt16LE(size_t offset) const
  { return getUInt16LE(offset); }
  int16_t getInt16BE(size_t offset) const
  { return getUInt16BE(offset); }
  int16_t getAlignedInt16LE(size_t offset) const
  { return getAlignedUInt16LE(offset); }
  int16_t getAlignedInt16BE(size_t offset) const
  { return getAlignedUInt16BE(offset); }


  // 32 bit value accessors
  void setUInt32LE(uint32_t value, size_t offset)
  {
    OpenRTIAssert(offset + 4 <= size());
    uint8_t* data = uint8Data(offset);
    data[0] = uint8_t(value);
    data[1] = uint8_t(value >> 8);
    data[2] = uint8_t(value >> 16);
    data[3] = uint8_t(value >> 24);
  }
  void setUInt32BE(uint32_t value, size_t offset)
  {
    OpenRTIAssert(offset + 4 <= size());
    uint8_t* data = uint8Data(offset);
    data[0] = uint8_t(value >> 24);
    data[1] = uint8_t(value >> 16);
    data[2] = uint8_t(value >> 8);
    data[3] = uint8_t(value);
  }
  void setAlignedUInt32LE(uint32_t value, size_t offset)
  {
    OpenRTIAssert((offset & 3) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    setUInt32LE(value, offset);
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    OpenRTIAssert(offset + 4 <= size());
    *static_cast<uint32_t*>(data(offset)) = value;
#else
#error "No endianess defined!"
#endif
  }
  void setAlignedUInt32BE(uint32_t value, size_t offset)
  {
    OpenRTIAssert((offset & 3) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    OpenRTIAssert(offset + 4 <= size());
    *static_cast<uint32_t*>(data(offset)) = value;
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    setUInt32BE(value, offset);
#else
#error "No endianess defined!"
#endif
  }

  uint32_t getUInt32LE(size_t offset) const
  {
    OpenRTIAssert(offset + 4 <= size());
    const uint8_t* data = uint8Data(offset);
    uint32_t value = uint32_t(data[0]);
    value |= uint32_t(data[1]) << 8;
    value |= uint32_t(data[2]) << 16;
    value |= uint32_t(data[3]) << 24;
    return value;
  }
  uint32_t getUInt32BE(size_t offset) const
  {
    OpenRTIAssert(offset + 4 <= size());
    const uint8_t* data = uint8Data(offset);
    uint32_t value = uint32_t(data[0]) << 24;
    value |= uint32_t(data[1]) << 16;
    value |= uint32_t(data[2]) << 8;
    value |= uint32_t(data[3]);
    return value;
  }
  uint32_t getAlignedUInt32LE(size_t offset) const
  {
    OpenRTIAssert((offset & 3) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    return getUInt32LE(offset);
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    OpenRTIAssert(offset + 4 <= size());
    return *static_cast<const uint32_t*>(data(offset));
#else
#error "No endianess defined!"
#endif
  }
  uint32_t getAlignedUInt32BE(size_t offset) const
  {
    OpenRTIAssert((offset & 3) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    OpenRTIAssert(offset + 4 <= size());
    return *static_cast<const uint32_t*>(data(offset));
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    return getUInt32BE(offset);
#else
#error "No endianess defined!"
#endif
  }


  void setInt32LE(int32_t value, size_t offset)
  { setUInt32LE(value, offset); }
  void setInt32BE(int32_t value, size_t offset)
  { setUInt32BE(value, offset); }
  void setAlignedInt32LE(int32_t value, size_t offset)
  { setAlignedUInt32LE(value, offset); }
  void setAlignedInt32BE(int32_t value, size_t offset)
  { setAlignedUInt32BE(value, offset); }

  int32_t getInt32LE(size_t offset) const
  { return getUInt32LE(offset); }
  int32_t getInt32BE(size_t offset) const
  { return getUInt32BE(offset); }
  int32_t getAlignedInt32LE(size_t offset) const
  { return getAlignedUInt32LE(offset); }
  int32_t getAlignedInt32BE(size_t offset) const
  { return getAlignedUInt32BE(offset); }


  // 64 bit value accessors
  void setUInt64LE(uint64_t value, size_t offset)
  {
    OpenRTIAssert(offset + 8 <= size());
    uint8_t* data = uint8Data(offset);
    data[0] = uint8_t(value);
    data[1] = uint8_t(value >> 8);
    data[2] = uint8_t(value >> 16);
    data[3] = uint8_t(value >> 24);
    data[4] = uint8_t(value >> 32);
    data[5] = uint8_t(value >> 40);
    data[6] = uint8_t(value >> 48);
    data[7] = uint8_t(value >> 56);
  }
  void setUInt64BE(uint64_t value, size_t offset)
  {
    OpenRTIAssert(offset + 8 <= size());
    uint8_t* data = uint8Data(offset);
    data[0] = uint8_t(value >> 56);
    data[1] = uint8_t(value >> 48);
    data[2] = uint8_t(value >> 40);
    data[3] = uint8_t(value >> 32);
    data[4] = uint8_t(value >> 24);
    data[5] = uint8_t(value >> 16);
    data[6] = uint8_t(value >> 8);
    data[7] = uint8_t(value);
  }
  void setAlignedUInt64LE(uint64_t value, size_t offset)
  {
    OpenRTIAssert((offset & 7) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    setUInt64LE(value, offset);
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    OpenRTIAssert(offset + 8 <= size());
    *static_cast<uint64_t*>(data(offset)) = value;
#else
#error "No endianess defined!"
#endif
  }
  void setAlignedUInt64BE(uint64_t value, size_t offset)
  {
    OpenRTIAssert((offset & 7) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    OpenRTIAssert(offset + 8 <= size());
    *static_cast<uint64_t*>(data(offset)) = value;
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    setUInt64BE(value, offset);
#else
#error "No endianess defined!"
#endif
  }

  uint64_t getUInt64LE(size_t offset) const
  {
    OpenRTIAssert(offset + 4 <= size());
    const uint8_t* data = uint8Data(offset);
    uint64_t value = uint64_t(data[0]);
    value |= uint64_t(data[1]) << 8;
    value |= uint64_t(data[2]) << 16;
    value |= uint64_t(data[3]) << 24;
    value |= uint64_t(data[4]) << 32;
    value |= uint64_t(data[5]) << 40;
    value |= uint64_t(data[6]) << 48;
    value |= uint64_t(data[7]) << 56;
    return value;
  }
  uint64_t getUInt64BE(size_t offset) const
  {
    OpenRTIAssert(offset + 4 <= size());
    const uint8_t* data = uint8Data(offset);
    uint64_t value = uint64_t(data[0]) << 56;
    value |= uint64_t(data[1]) << 48;
    value |= uint64_t(data[2]) << 40;
    value |= uint64_t(data[3]) << 32;
    value |= uint64_t(data[4]) << 24;
    value |= uint64_t(data[5]) << 16;
    value |= uint64_t(data[6]) << 8;
    value |= uint64_t(data[7]);
    return value;
  }
  uint64_t getAlignedUInt64LE(size_t offset) const
  {
    OpenRTIAssert((offset & 7) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    return getUInt64LE(offset);
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    OpenRTIAssert(offset + 8 <= size());
    return *static_cast<const uint64_t*>(data(offset));
#else
#error "No endianess defined!"
#endif
  }
  uint64_t getAlignedUInt64BE(size_t offset) const
  {
    OpenRTIAssert((offset & 7) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    OpenRTIAssert(offset + 8 <= size());
    return *static_cast<const uint64_t*>(data(offset));
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    return getUInt64BE(offset);
#else
#error "No endianess defined!"
#endif
  }


  void setInt64LE(int64_t value, size_t offset)
  { setUInt64LE(value, offset); }
  void setInt64BE(int64_t value, size_t offset)
  { setUInt64BE(value, offset); }
  void setAlignedInt64LE(int64_t value, size_t offset)
  { setAlignedUInt64LE(value, offset); }
  void setAlignedInt64BE(int64_t value, size_t offset)
  { setAlignedUInt64BE(value, offset); }

  int64_t getInt64LE(size_t offset) const
  { return getUInt64LE(offset); }
  int64_t getInt64BE(size_t offset) const
  { return getUInt64BE(offset); }
  int64_t getAlignedInt64LE(size_t offset) const
  { return getAlignedUInt64LE(offset); }
  int64_t getAlignedInt64BE(size_t offset) const
  { return getAlignedUInt64BE(offset); }


  void setFloat32LE(float value, size_t offset)
  {
    union {
      float d;
      uint32_t u;
    } u;
    u.d = value;
    setUInt32LE(u.u, offset);
  }
  void setFloat32BE(float value, size_t offset)
  {
    union {
      float d;
      uint32_t u;
    } u;
    u.d = value;
    setUInt32BE(u.u, offset);
  }
  void setAlignedFloat32LE(float value, size_t offset)
  {
    OpenRTIAssert((offset & 3) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    setFloat32LE(value, offset);
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    OpenRTIAssert(offset + 4 <= size());
    *static_cast<float*>(data(offset)) = value;
#else
#error "No endianess defined!"
#endif
  }
  void setAlignedFloat32BE(float value, size_t offset)
  {
    OpenRTIAssert((offset & 3) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    OpenRTIAssert(offset + 4 <= size());
    *static_cast<float*>(data(offset)) = value;
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    setFloat32BE(value, offset);
#else
#error "No endianess defined!"
#endif
  }

  float getFloat32LE(size_t offset) const
  {
    union {
      float d;
      uint32_t u;
    } u;
    u.u = getUInt32LE(offset);
    return u.d;
  }
  float getFloat32BE(size_t offset) const
  {
    union {
      float d;
      uint32_t u;
    } u;
    u.u = getUInt32BE(offset);
    return u.d;
  }
  float getAlignedFloat32LE(size_t offset) const
  {
    OpenRTIAssert((offset & 3) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    return getFloat32LE(offset);
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    OpenRTIAssert(offset + 4 <= size());
    return *static_cast<const float*>(data(offset));
#else
#error "No endianess defined!"
#endif
  }
  float getAlignedFloat32BE(size_t offset) const
  {
    OpenRTIAssert((offset & 3) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    OpenRTIAssert(offset + 4 <= size());
    return *static_cast<const float*>(data(offset));
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    return getFloat32BE(offset);
#else
#error "No endianess defined!"
#endif
  }

  void setFloat64LE(double value, size_t offset)
  {
    union {
      double d;
      uint64_t u;
    } u;
    u.d = value;
    setUInt64LE(u.u, offset);
  }
  void setFloat64BE(double value, size_t offset)
  {
    union {
      double d;
      uint64_t u;
    } u;
    u.d = value;
    setUInt64BE(u.u, offset);
  }
  void setAlignedFloat64LE(double value, size_t offset)
  {
    OpenRTIAssert((offset & 7) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    setFloat64LE(value, offset);
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    OpenRTIAssert(offset + 8 <= size());
    *static_cast<double*>(data(offset)) = value;
#else
#error "No endianess defined!"
#endif
  }
  void setAlignedFloat64BE(double value, size_t offset)
  {
    OpenRTIAssert((offset & 7) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    OpenRTIAssert(offset + 8 <= size());
    *static_cast<double*>(data(offset)) = value;
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    setFloat64BE(value, offset);
#else
#error "No endianess defined!"
#endif
  }

  double getFloat64LE(size_t offset) const
  {
    union {
      double d;
      uint64_t u;
    } u;
    u.u = getUInt64LE(offset);
    return u.d;
  }
  double getFloat64BE(size_t offset) const
  {
    union {
      double d;
      uint64_t u;
    } u;
    u.u = getUInt64BE(offset);
    return u.d;
  }
  double getAlignedFloat64LE(size_t offset) const
  {
    OpenRTIAssert((offset & 7) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    return getFloat64LE(offset);
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    OpenRTIAssert(offset + 8 <= size());
    return *static_cast<const double*>(data(offset));
#else
#error "No endianess defined!"
#endif
  }
  double getAlignedFloat64BE(size_t offset) const
  {
    OpenRTIAssert((offset & 7) == 0);
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
    OpenRTIAssert(offset + 8 <= size());
    return *static_cast<const double*>(data(offset));
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
    return getFloat64BE(offset);
#else
#error "No endianess defined!"
#endif
  }

private:
  class OPENRTI_API Data : public Referenced {
  public:
    Data(size_t size) :
      _variableLengthDataDeleteFunction(),
      _capacity(size),
      _data(_dummy)
    { }
    Data(void* data, _VariableLengthDataDeleteFunction variableLengthDataDeleteFunction) :
      _variableLengthDataDeleteFunction(variableLengthDataDeleteFunction),
      _capacity(0),
      _data(data)
    { }
    ~Data()
    {
      if (_data != _dummy) {
        if (_variableLengthDataDeleteFunction)
          _variableLengthDataDeleteFunction(_data);
        else
          ::operator delete(_data);
      }
    }
    void* data(size_t offset = 0)
    {
      return static_cast<char*>(_data) + offset;
    }
    void clear()
    {
      if (_data != _dummy) {
        if (_variableLengthDataDeleteFunction)
          _variableLengthDataDeleteFunction(_data);
        else
          ::operator delete(_data);
        _data = 0;
      }
    }
    size_t capacity() const
    { return _capacity; }

  private:
    _VariableLengthDataDeleteFunction _variableLengthDataDeleteFunction;
    const size_t _capacity;
    void* _data;
    // Due to the struct layout, this should be aligned to 16 bytes on 64 bits
    // and 8 bytes on 32 bits. That is, the data field should be about aligned enough?!
    char _dummy[1];
  };

  static Data* createOwnData(size_t capacity)
  {
    void* data = ::operator new(capacity + sizeof(Data));
    return new (data) Data(capacity);
  }
  static Data* createExternalData(void *data, _VariableLengthDataDeleteFunction variableLengthDataDeleteFunction)
  {
    return new Data(data, variableLengthDataDeleteFunction);
  }

  SharedPtr<Data> _data;
  size_t _size;
  size_t _offset;
};

template<typename char_type, typename traits_type>
std::basic_ostream<char_type, traits_type>&
operator<<(std::basic_ostream<char_type, traits_type>& os, const VariableLengthData& value)
{
  os << "{ size: " << value.size() << ", data: [...] }";
  return os;
}

} // namespace OpenRTI

#endif
