/* -*-c++-*- OpenRTI - Copyright (C) 2010-2011 Mathias Froehlich
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

#include "QHLAStringDataElement.h"

#include "QHLADataTypeVisitor.h"

struct QHLAStringDataElement::DecodeVisitor : public QHLADecodeVisitor {
  DecodeVisitor(const QByteArray& byteArray, int pos, QString& value) :
    QHLADecodeVisitor(byteArray, pos), _value(value)
  { }
  virtual void apply(const QHLADataType& dataType)
  { _pos = -1; }
  virtual void apply(const QHLAArrayDataType& dataType)
  {
    if (_pos < 0)
      return;
    if (!dataType.isString()) {
      _pos = -1;
      return;
    }
    if (1 != dataType.numArrayDimensions()) {
      _pos = -1;
      return;
    }
    if (!dataType.elementType()) {
      _pos = -1;
      return;
    }
    _align(dataType);
    if (_pos < 0)
      return;
    int numElements;
    if (dataType.arrayDimensionFixed(0)) {
      numElements = dataType.fixedArrayDimension(0);
    } else {
      quint32 _uint = _readUInt32BE();
      if (INT_MAX < _uint) {
        _pos = -1;
        return;
      }
      numElements = _uint;
    }
    _value.resize(0);
    _value.reserve(numElements);
    for (int i = 0; i < numElements; ++i)
      dataType.elementType()->accept(*this);
  }
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
          _value.push_back(QChar(u._int));
          break;
        case QHLABasicDataType::UnsignedType:
          _value.push_back(QChar(u._uint));
          break;
        case QHLABasicDataType::FloatType:
          _pos = -1;
          break;
        }
      }
      break;
    case 16:
      {
        const unsigned char* data = (const unsigned char*)_byteArray.data() + _pos;
        _pos += 2;
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
          _value.push_back(QChar(u._int));
          break;
        case QHLABasicDataType::UnsignedType:
          _value.push_back(QChar(u._uint));
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
        } u;
        if (dataType.endian() == QHLABasicDataType::LittleEndian) {
          u._uint = _readUInt32LE();
        } else {
          u._uint = _readUInt32BE();
        }
        switch (dataType.type()) {
        case QHLABasicDataType::SignedType:
          _value.push_back(QChar(u._int));
          break;
        case QHLABasicDataType::UnsignedType:
          _value.push_back(QChar(u._uint));
          break;
        case QHLABasicDataType::FloatType:
          _pos = -1;
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

  QString& _value;
};

QHLAStringDataElement::QHLAStringDataElement(QObject* parent, const QHLAArrayDataType* dataType) :
  QHLADataElement(parent),
  _dataType(dataType)
{
}

QHLAStringDataElement::~QHLAStringDataElement()
{
}

const QHLAArrayDataType*
QHLAStringDataElement::dataType() const
{
  return _dataType;
}

int
QHLAStringDataElement::decode(const QByteArray& byteArray, int pos)
{
  if (!_dataType)
    return -1;
  DecodeVisitor decodeVisitor(byteArray, pos, _value);
  _dataType->accept(decodeVisitor);
  return decodeVisitor.pos();
}

const QString&
QHLAStringDataElement::value() const
{
  return _value;
}

void
QHLAStringDataElement::setValue(const QString& value)
{
  _value = value;
}
