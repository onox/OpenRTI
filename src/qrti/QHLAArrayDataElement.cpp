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

#include "QHLAArrayDataElement.h"

#include "QHLADataTypeVisitor.h"

struct QHLAArrayDataElement::DecodeVisitor : public QHLADecodeVisitor {
  DecodeVisitor(const QByteArray& byteArray, int pos, QHLAArrayDataElement& value) :
    QHLADecodeVisitor(byteArray, pos), _value(value)
  { }
  virtual void apply(const QHLAArrayDataType& dataType)
  {
    if (_pos < 0)
      return;
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
    _value.resize(numElements);
    for (int i = 0; i < numElements; ++i)
      _pos = _value.dataElement(i)->decode(_byteArray, _pos);
  }

  QHLAArrayDataElement& _value;
};

QHLAArrayDataElement::QHLAArrayDataElement(QObject* parent, const QHLAArrayDataType* dataType) :
  QHLADataElement(parent),
  _dataType(dataType)
{
}

QHLAArrayDataElement::~QHLAArrayDataElement()
{
}

const QHLAArrayDataType*
QHLAArrayDataElement::dataType() const
{
  return _dataType;
}

int
QHLAArrayDataElement::decode(const QByteArray& byteArray, int pos)
{
  if (!_dataType)
    return -1;
  DecodeVisitor decodeVisitor(byteArray, pos, *this);
  _dataType->accept(decodeVisitor);
  return decodeVisitor.pos();
}

int
QHLAArrayDataElement::size() const
{
  return _dataElements.size();
}

void
QHLAArrayDataElement::resize(int size)
{
  int index = _dataElements.size();
  while (index < size) {
    QHLADataElement* dataElement;
    dataElement = _dataType->elementType()->createDataElement(this);
    dataElement->_indexInParent = _dataElements.size();
    _dataElements.push_back(dataElement);
    ++index;
  }
  while (size <= --index) {
    QHLADataElement* dataElement;
    dataElement = _dataElements.back();
    _dataElements.pop_back();
    dataElement->_indexInParent = -1;
    delete dataElement;
  }
}

QHLADataElement*
QHLAArrayDataElement::dataElement(int index)
{
  return _dataElements.value(index, 0);
}

const QHLADataElement*
QHLAArrayDataElement::dataElement(int index) const
{
  return _dataElements.value(index, 0);
}
