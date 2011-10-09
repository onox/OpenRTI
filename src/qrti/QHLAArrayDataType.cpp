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

#include "QHLAArrayDataType.h"

#include "QHLAArrayDataElement.h"
#include "QHLADataTypeVisitor.h"
#include "QHLAStringDataElement.h"

QHLAArrayDataType::QHLAArrayDataType(QHLAArrayDataTypeList* parent) :
  QHLADataType(parent),
  _elementType(0),
  _isString(false),
  _isOpaqueData(false),
  _arrayDimension(-1),
  _indexInDataTypeList(-1)
{
}

QHLAArrayDataType::~QHLAArrayDataType()
{
}

void
QHLAArrayDataType::accept(QHLADataTypeVisitor& visitor) const
{
  visitor.apply(*this);
}

QHLADataElement*
QHLAArrayDataType::createDataElement(QObject* parent) const
{
  if (isString()) {
    return new QHLAStringDataElement(parent, this);
  } else {
    return new QHLAArrayDataElement(parent, this);
  }
}

const QHLADataType*
QHLAArrayDataType::elementType() const
{
  return _elementType;
}

void
QHLAArrayDataType::setElementType(const QHLADataType* elementType)
{
  _elementType = elementType;
}

const QString&
QHLAArrayDataType::cardinality() const
{
  return _cardinality;
}

void
QHLAArrayDataType::setCardinality(const QString& cardinality)
{
  _cardinality = cardinality;
  if (cardinality == "Dynamic") {
    _arrayDimension = -1;
  } else {
    bool ok;
    int dim = cardinality.toInt(&ok);
    if (ok && 0 <= dim)
      _arrayDimension = dim;
    else
      _arrayDimension = -1;
  }
}

const QString&
QHLAArrayDataType::encoding() const
{
  return _encoding;
}

void
QHLAArrayDataType::setEncoding(const QString& encoding)
{
  _encoding = encoding;
}

const QString&
QHLAArrayDataType::semantics() const
{
  return _semantics;
}

void
QHLAArrayDataType::setSemantics(const QString& semantics)
{
  _semantics = semantics;
}

bool
QHLAArrayDataType::isString() const
{
  return _isString;
}

void
QHLAArrayDataType::setIsString(bool isString)
{
  _isString = isString;
}

bool
QHLAArrayDataType::isOpaqueData() const
{
  return _isOpaqueData;
}

void
QHLAArrayDataType::setIsOpaqueData(bool isOpaque)
{
  _isOpaqueData = isOpaque;
}

int
QHLAArrayDataType::numArrayDimensions() const
{
  return 1;
}

bool
QHLAArrayDataType::arrayDimensionFixed(int index) const
{
  return 0 <= _arrayDimension;
}

int
QHLAArrayDataType::fixedArrayDimension(int index) const
{
  return _arrayDimension;
}

int
QHLAArrayDataType::minArrayDimension(int index) const
{
  if (_arrayDimension <= 0)
    return 0;
  return _arrayDimension;
}

int
QHLAArrayDataType::maxArrayDimension(int index) const
{
  if (_arrayDimension <= 0)
    return INT_MAX;
  return _arrayDimension;
}

int
QHLAArrayDataType::indexInDataTypeList() const
{
  return _indexInDataTypeList;
}

QHLAArrayDataTypeList::QHLAArrayDataTypeList(QObject* parent) :
  QObject(parent)
{
}

QHLAArrayDataTypeList::~QHLAArrayDataTypeList()
{
}

QHLAArrayDataType*
QHLAArrayDataTypeList::createDataType()
{
  QHLAArrayDataType* dataType = new QHLAArrayDataType(this);
  dataType->_indexInDataTypeList = _dataTypeList.size();
  _dataTypeList.push_back(dataType);
  return dataType;
}

void
QHLAArrayDataTypeList::eraseDataType(QHLAArrayDataType* dataType)
{
  Q_ASSERT(dataType->parent() == this);
  int index = dataType->_indexInDataTypeList;
  dataType->_indexInDataTypeList = -1;
  _dataTypeList.removeAt(index);
  for (; index < _dataTypeList.size(); ++index)
    _dataTypeList[index]->_indexInDataTypeList = index;
}

QHLAArrayDataType*
QHLAArrayDataTypeList::getDataType(const QString& name)
{
  for (int i = 0; i < _dataTypeList.size(); ++i) {
    if (_dataTypeList[i]->name() == name)
      return _dataTypeList[i];
  }
  return 0;
}

QHLAArrayDataType*
QHLAArrayDataTypeList::getDataType(int index)
{
  return _dataTypeList.value(index, 0);
}

int
QHLAArrayDataTypeList::getNumDataTypes() const
{
  return _dataTypeList.size();
}
