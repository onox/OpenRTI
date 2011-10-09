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

#include "QHLABasicDataType.h"

#include "QHLADataTypeVisitor.h"
#include "QHLAScalarDataElement.h"

QHLABasicDataType::QHLABasicDataType(QHLABasicDataTypeList* parent) :
  QHLADataType(parent),
  _size(0),
  _endian(BigEndian),
  _type(SignedType),
  _indexInDataTypeList(-1)
{
}

QHLABasicDataType::~QHLABasicDataType()
{
}

void
QHLABasicDataType::accept(QHLADataTypeVisitor& visitor) const
{
  visitor.apply(*this);
}

QHLADataElement*
QHLABasicDataType::createDataElement(QObject* parent) const
{
  return new QHLAScalarDataElement(parent, this);
}

unsigned
QHLABasicDataType::size() const
{
  return _size;
}

void
QHLABasicDataType::setSize(unsigned size)
{
  _size = size;
  setAlignment(_size/8);
}

const QString&
QHLABasicDataType::interpretation() const
{
  return _interpretation;
}

void
QHLABasicDataType::setInterpretation(const QString& interpretation)
{
  _interpretation = interpretation;
}

QHLABasicDataType::Endian
QHLABasicDataType::endian() const
{
  return _endian;
}

void
QHLABasicDataType::setEndian(Endian endian)
{
  _endian = endian;
}

QHLABasicDataType::Type
QHLABasicDataType::type() const
{
  return _type;
}

void
QHLABasicDataType::setType(Type type)
{
  _type = type;
}

const QString&
QHLABasicDataType::encoding() const
{
  return _encoding;
}

void
QHLABasicDataType::setEncoding(const QString& encoding)
{
  _encoding = encoding;
}

int
QHLABasicDataType::indexInDataTypeList() const
{
  return _indexInDataTypeList;
}

QHLABasicDataTypeList::QHLABasicDataTypeList(QObject* parent) :
  QObject(parent)
{
}

QHLABasicDataTypeList::~QHLABasicDataTypeList()
{
}

QHLABasicDataType*
QHLABasicDataTypeList::createDataType()
{
  QHLABasicDataType* dataType = new QHLABasicDataType(this);
  dataType->_indexInDataTypeList = _dataTypeList.size();
  _dataTypeList.push_back(dataType);
  return dataType;
}

void
QHLABasicDataTypeList::eraseDataType(QHLABasicDataType* dataType)
{
  Q_ASSERT(dataType->parent() == this);
  int index = dataType->_indexInDataTypeList;
  dataType->_indexInDataTypeList = -1;
  _dataTypeList.removeAt(index);
  for (; index < _dataTypeList.size(); ++index)
    _dataTypeList[index]->_indexInDataTypeList = index;
}

QHLABasicDataType*
QHLABasicDataTypeList::getDataType(const QString& name)
{
  for (int i = 0; i < _dataTypeList.size(); ++i) {
    if (_dataTypeList[i]->name() == name)
      return _dataTypeList[i];
  }
  return 0;
}

QHLABasicDataType*
QHLABasicDataTypeList::getDataType(int index)
{
  return _dataTypeList.value(index, 0);
}
