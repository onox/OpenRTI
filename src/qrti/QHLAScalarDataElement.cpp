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

#include "QHLAScalarDataElement.h"

#include "QHLABasicDataType.h"
#include "QHLADataTypeVisitor.h"
#include "QHLASimpleDataType.h"

struct QHLAScalarDataElement::DecodeVisitor : public QHLADecodeVisitor {
  DecodeVisitor(const QByteArray& byteArray, int pos, QVariant& value) :
    QHLADecodeVisitor(byteArray, pos), _value(value)
  { }
  virtual void apply(const QHLADataType& dataType)
  { _pos = -1; }
  virtual void apply(const QHLABasicDataType& dataType)
  {
    if (_pos < 0)
      return;
    _align(dataType);
    if (_pos < 0)
      return;
    switch (dataType.size()) {
    case 8:
      {
        union {
          quint8 _uint;
          qint8 _int;
        } u;
        u._uint = _readUInt8();
        switch (dataType.type()) {
        case QHLABasicDataType::SignedType:
          _value = u._int;
          break;
        case QHLABasicDataType::UnsignedType:
          _value = u._uint;
          break;
        case QHLABasicDataType::FloatType:
          _pos = -1;
          break;
        }
      }
      break;
    case 16:
      {
        union {
          quint16 _uint;
          qint16 _int;
        } u;
        if (dataType.endian() == QHLABasicDataType::LittleEndian) {
          u._uint = _readUInt16LE();
        } else {
          u._uint = _readUInt16BE();
        }
        switch (dataType.type()) {
        case QHLABasicDataType::SignedType:
          _value = u._int;
          break;
        case QHLABasicDataType::UnsignedType:
          _value = u._uint;
          break;
        case QHLABasicDataType::FloatType:
          _pos = -1;
          break;
        }
      }
      break;
    case 32:
      {
        union {
          quint32 _uint;
          qint32 _int;
          float _float;
        } u;
        if (dataType.endian() == QHLABasicDataType::LittleEndian) {
          u._uint = _readUInt32LE();
        } else {
          u._uint = _readUInt32BE();
        }
        switch (dataType.type()) {
        case QHLABasicDataType::SignedType:
          _value = u._int;
          break;
        case QHLABasicDataType::UnsignedType:
          _value = u._uint;
          break;
        case QHLABasicDataType::FloatType:
          _value = u._float;
          break;
        }
      }
      break;
    case 64:
      {
        union {
          quint64 _uint;
          qint64 _int;
          double _float;
        } u;
        if (dataType.endian() == QHLABasicDataType::LittleEndian) {
          u._uint = _readUInt64LE();
        } else {
          u._uint = _readUInt64BE();
        }
        switch (dataType.type()) {
        case QHLABasicDataType::SignedType:
          _value = u._int;
          break;
        case QHLABasicDataType::UnsignedType:
          _value = u._uint;
          break;
        case QHLABasicDataType::FloatType:
          _value = u._float;
          break;
        }
      }
      break;
    default:
      _pos = -1;
      break;
    }

  }
  virtual void apply(const QHLASimpleDataType& dataType)
  {
    if (_pos < 0)
      return;
    const QHLABasicDataType* basicDataType = dataType.representation();
    if (!basicDataType) {
      _pos = -1;
      return;
    }
    basicDataType->accept(*this);
  }

  QVariant& _value;
};

QHLAScalarDataElement::QHLAScalarDataElement(QObject* parent, const QHLADataType* dataType) :
  QHLADataElement(parent),
  _dataType(dataType)
{
}

QHLAScalarDataElement::~QHLAScalarDataElement()
{
}

const QHLADataType*
QHLAScalarDataElement::dataType() const
{
  return _dataType;
}

int
QHLAScalarDataElement::decode(const QByteArray& byteArray, int pos)
{
  if (!_dataType)
    return -1;
  DecodeVisitor decodeVisitor(byteArray, pos, _value);
  _dataType->accept(decodeVisitor);
  return decodeVisitor.pos();
}

const QVariant&
QHLAScalarDataElement::value() const
{
  return _value;
}

void
QHLAScalarDataElement::setValue(const QVariant& value)
{
  _value = value;
}
