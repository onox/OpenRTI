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

#include "QHLASimpleDataType.h"

#include "QHLABasicDataType.h"
#include "QHLADataTypeVisitor.h"
#include "QHLAScalarDataElement.h"

QHLASimpleDataType::QHLASimpleDataType(QHLASimpleDataTypeList* parent) :
  QHLADataType(parent),
  _representation(0),
  _indexInDataTypeList(-1)
{
}

QHLASimpleDataType::~QHLASimpleDataType()
{
}

void
QHLASimpleDataType::accept(QHLADataTypeVisitor& visitor) const
{
  visitor.apply(*this);
}

QHLADataElement*
QHLASimpleDataType::createDataElement(QObject* parent) const
{
  return new QHLAScalarDataElement(parent, this);
}

const QHLABasicDataType*
QHLASimpleDataType::representation() const
{
  return _representation;
}

void
QHLASimpleDataType::setRepresentation(const QHLABasicDataType* representation)
{
  _representation = representation;
}

const QString&
QHLASimpleDataType::units() const
{
  return _units;
}

void
QHLASimpleDataType::setUnits(const QString& units)
{
  _units = units;
}

const QString&
QHLASimpleDataType::resolution() const
{
  return _resolution;
}

void
QHLASimpleDataType::setResolution(const QString& resolution)
{
  _resolution = resolution;
}

const QString&
QHLASimpleDataType::accuracy() const
{
  return _accuracy;
}

void
QHLASimpleDataType::setAccuracy(const QString& accuracy)
{
  _accuracy = accuracy;
}

const QString&
QHLASimpleDataType::semantics() const
{
  return _semantics;
}

void
QHLASimpleDataType::setSemantics(const QString& semantics)
{
  _semantics = semantics;
}

int
QHLASimpleDataType::indexInDataTypeList() const
{
  return _indexInDataTypeList;
}

QHLASimpleDataTypeList::QHLASimpleDataTypeList(QObject* parent) :
  QObject(parent)
{
}

QHLASimpleDataTypeList::~QHLASimpleDataTypeList()
{
}

QHLASimpleDataType*
QHLASimpleDataTypeList::createDataType()
{
  QHLASimpleDataType* dataType = new QHLASimpleDataType(this);
  dataType->_indexInDataTypeList = _dataTypeList.size();
  _dataTypeList.push_back(dataType);
  return dataType;
}

void
QHLASimpleDataTypeList::eraseDataType(QHLASimpleDataType* dataType)
{
  Q_ASSERT(dataType->parent() == this);
  int index = dataType->_indexInDataTypeList;
  dataType->_indexInDataTypeList = -1;
  _dataTypeList.removeAt(index);
  for (; index < _dataTypeList.size(); ++index)
    _dataTypeList[index]->_indexInDataTypeList = index;
}

QHLASimpleDataType*
QHLASimpleDataTypeList::getDataType(const QString& name)
{
  for (int i = 0; i < _dataTypeList.size(); ++i) {
    if (_dataTypeList[i]->name() == name)
      return _dataTypeList[i];
  }
  return 0;
}

QHLASimpleDataType*
QHLASimpleDataTypeList::getDataType(int index)
{
  return _dataTypeList.value(index, 0);
}
