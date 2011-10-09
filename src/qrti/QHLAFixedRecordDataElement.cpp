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

#include "QHLAFixedRecordDataElement.h"

QHLAFixedRecordDataElement::QHLAFixedRecordDataElement(QObject* parent, const QHLAFixedRecordDataType* dataType) :
  QHLADataElement(parent),
  _dataType(dataType)
{
  for (int i = 0; i < _dataType->getNumFields(); ++i) {
    const QHLAField* field = _dataType->getField(i);
    if (field && field->type()) {
      QHLADataElement* dataElement;
      dataElement = field->type()->createDataElement(this);
      dataElement->_indexInParent = _fieldList.size();
      _fieldList.push_back(dataElement);
    } else {
      _fieldList.push_back(0);
    }
  }
}

QHLAFixedRecordDataElement::~QHLAFixedRecordDataElement()
{
}

const QHLAFixedRecordDataType*
QHLAFixedRecordDataElement::dataType() const
{
  return _dataType;
}

int
QHLAFixedRecordDataElement::decode(const QByteArray& byteArray, int pos)
{
  if (!_dataType)
    return -1;

  int alignment = _dataType->alignment();
  pos = (pos + alignment - 1) & (~(alignment - 1));

  for (int i = 0; i < _dataType->getNumFields(); ++i) {
    QHLADataElement* field = _fieldList.value(i, 0);
    if (!field)
      return -1;
    pos = field->decode(byteArray, pos);
  }

  return pos;
}

int
QHLAFixedRecordDataElement::numFields() const
{
  return _fieldList.size();
}

const QHLADataElement*
QHLAFixedRecordDataElement::field(int index) const
{
  return _fieldList.value(index, 0);
}

QHLADataElement*
QHLAFixedRecordDataElement::field(int index)
{
  return _fieldList.value(index, 0);
}
