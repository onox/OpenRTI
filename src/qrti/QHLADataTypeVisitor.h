/* -*-c++-*- OpenRTI - Copyright (C) 2010-2012 Mathias Froehlich
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

#ifndef QHLADataTypeVisitor_h
#define QHLADataTypeVisitor_h

#include "QHLADataType.h"
#include "QHLAArrayDataType.h"
#include "QHLABasicDataType.h"
#include "QHLAEnumeratedDataType.h"
#include "QHLAFixedRecordDataType.h"
#include "QHLASimpleDataType.h"
#include "QHLAVariantRecordDataType.h"

class QHLADataTypeVisitor {
public:
  virtual ~QHLADataTypeVisitor()
  { }
  virtual void apply(const QHLADataType& dataType)
  { }
  virtual void apply(const QHLAArrayDataType& dataType)
  { apply(static_cast<const QHLADataType&>(dataType)); }
  virtual void apply(const QHLABasicDataType& dataType)
  { apply(static_cast<const QHLADataType&>(dataType)); }
  virtual void apply(const QHLAEnumeratedDataType& dataType)
  { apply(static_cast<const QHLADataType&>(dataType)); }
  virtual void apply(const QHLAFixedRecordDataType& dataType)
  { apply(static_cast<const QHLADataType&>(dataType)); }
  virtual void apply(const QHLASimpleDataType& dataType)
  { apply(static_cast<const QHLADataType&>(dataType)); }
  virtual void apply(const QHLAVariantRecordDataType& dataType)
  { apply(static_cast<const QHLADataType&>(dataType)); }
};

class QHLADecodeVisitor : public QHLADataTypeVisitor {
public:
  QHLADecodeVisitor(const QByteArray& byteArray, int pos) :
    _byteArray(byteArray), _pos(pos)
  { }

  int pos() const
  { return _pos; }

protected:
  void _align(const QHLADataType& dataType)
  {
    _align(dataType.alignment());
  }
  void _align(int alignment)
  {
    _pos = (_pos + alignment - 1) & (~(alignment - 1));
    if (_byteArray.size() <= _pos)
      _pos = -1;
  }

  quint8 _readUInt8()
  {
    if (_byteArray.size() <= _pos) {
      _pos = -1;
      return 0;
    }

    const unsigned char* data = (const unsigned char*)_byteArray.data() + _pos;
    _pos += 1;
    return data[0];
  }

  quint16 _readUInt16LE()
  {
    if (_byteArray.size() <= _pos + 1) {
      _pos = -1;
      return 0;
    }

    const unsigned char* data = (const unsigned char*)_byteArray.data() + _pos;
    _pos += 2;

    return quint16(data[0]) |
      (quint16(data[1]) << 8);
  }

  quint16 _readUInt16BE()
  {
    if (_byteArray.size() <= _pos + 1) {
      _pos = -1;
      return 0;
    }

    const unsigned char* data = (const unsigned char*)_byteArray.data() + _pos;
    _pos += 2;

    return quint16(data[1]) |
      (quint16(data[0]) << 8);
  }

  quint32 _readUInt32LE()
  {
    if (_byteArray.size() <= _pos + 3) {
      _pos = -1;
      return 0;
    }

    const unsigned char* data = (const unsigned char*)_byteArray.data() + _pos;
    _pos += 4;

    return quint32(data[0]) |
      (quint32(data[1]) << 8) |
      (quint32(data[2]) << 16) |
      (quint32(data[3]) << 24);
  }

  quint32 _readUInt32BE()
  {
    if (_byteArray.size() <= _pos + 3) {
      _pos = -1;
      return 0;
    }

    const unsigned char* data = (const unsigned char*)_byteArray.data() + _pos;
    _pos += 4;

    return quint32(data[3]) |
      (quint32(data[2]) << 8) |
      (quint32(data[1]) << 16) |
      (quint32(data[0]) << 24);
  }

  quint64 _readUInt64LE()
  {
    if (_byteArray.size() <= _pos + 7) {
      _pos = -1;
      return 0;
    }

    const unsigned char* data = (const unsigned char*)_byteArray.data() + _pos;
    _pos += 8;

    return quint64(data[0]) |
      (quint64(data[1]) << 8) |
      (quint64(data[2]) << 16) |
      (quint64(data[3]) << 24) |
      (quint64(data[4]) << 32) |
      (quint64(data[5]) << 40) |
      (quint64(data[6]) << 48) |
      (quint64(data[7]) << 56);
  }

  quint64 _readUInt64BE()
  {
    if (_byteArray.size() <= _pos + 7) {
      _pos = -1;
      return 0;
    }

    const unsigned char* data = (const unsigned char*)_byteArray.data() + _pos;
    _pos += 8;

    return quint64(data[7]) |
      (quint64(data[6]) << 8) |
      (quint64(data[5]) << 16) |
      (quint64(data[4]) << 24) |
      (quint64(data[3]) << 32) |
      (quint64(data[2]) << 40) |
      (quint64(data[1]) << 48) |
      (quint64(data[0]) << 56);
  }

  const QByteArray& _byteArray;
  int _pos;
};

#endif
