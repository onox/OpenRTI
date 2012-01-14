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

#include "QHLAVariantRecordDataElement.h"

#include "QHLAEnumeratedDataElement.h"

QHLAVariantRecordDataElement::QHLAVariantRecordDataElement(QObject* parent, const QHLAVariantRecordDataType* dataType) :
  QHLADataElement(parent),
  _dataType(dataType),
  _enumeratedDataElement(0),
  _dataElement(0)
{
  if (_dataType && _dataType->type())
    _enumeratedDataElement = _dataType->type()->createEnumeratedDataElement(this);
}

QHLAVariantRecordDataElement::~QHLAVariantRecordDataElement()
{
}

const QHLAVariantRecordDataType*
QHLAVariantRecordDataElement::dataType() const
{
  return _dataType;
}

int
QHLAVariantRecordDataElement::decode(const QByteArray& byteArray, int pos)
{
  if (!_dataType)
    return -1;
  if (!_enumeratedDataElement)
    return -1;

  pos = _enumeratedDataElement->decode(byteArray, pos);
  // QHLAAlternative* alternative = _dataType->()

  // for (int i = 0; i < _dataType->getNumFields(); ++i) {
  //   const QHLAField* field = _dataType->getField(i);
  //   _fieldList.push_back(field->type()->createDataElement(this));
  // }

  /// FIXME
  return -1;

  return pos;
}
