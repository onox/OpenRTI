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

#include "QHLAEnumeratedDataType.h"

#include "QHLABasicDataType.h"
#include "QHLADataTypeVisitor.h"
#include "QHLAEnumeratedDataElement.h"

QHLAEnumerator::QHLAEnumerator(QHLAEnumeratedDataType* parent) :
  QObject(parent),
  _indexInDataType(-1)
{
}

QHLAEnumerator::~QHLAEnumerator()
{
}

const QString&
QHLAEnumerator::name() const
{
  return _name;
}

void
QHLAEnumerator::setName(const QString& name)
{
  _name = name;
}

const QString&
QHLAEnumerator::values() const
{
  return _values;
}

void
QHLAEnumerator::setValues(const QString& values)
{
  _values = values;
}

int
QHLAEnumerator::indexInDataType() const
{
  return _indexInDataType;
}

QHLAEnumeratedDataType::QHLAEnumeratedDataType(QHLAEnumeratedDataTypeList* parent) :
  QHLADataType(parent),
  _representation(0),
  _indexInDataTypeList(-1)
{
}

QHLAEnumeratedDataType::~QHLAEnumeratedDataType()
{
}

void
QHLAEnumeratedDataType::accept(QHLADataTypeVisitor& visitor) const
{
  visitor.apply(*this);
}

QHLADataElement*
QHLAEnumeratedDataType::createDataElement(QObject* parent) const
{
  return createEnumeratedDataElement(parent);
}

QHLAEnumeratedDataElement*
QHLAEnumeratedDataType::createEnumeratedDataElement(QObject* parent) const
{
  return new QHLAEnumeratedDataElement(parent, this);
}

const QHLABasicDataType*
QHLAEnumeratedDataType::representation() const
{
  return _representation;
}

void
QHLAEnumeratedDataType::setRepresentation(const QHLABasicDataType* representation)
{
  _representation = representation;
}

QHLAEnumerator*
QHLAEnumeratedDataType::createEnumerator()
{
  QHLAEnumerator* enumerator = new QHLAEnumerator(this);
  enumerator->_indexInDataType = _enumeratorList.size();
  _enumeratorList.push_back(enumerator);
  return enumerator;
}

int
QHLAEnumeratedDataType::getNumEnumerators() const
{
  return _enumeratorList.size();
}

const QHLAEnumerator*
QHLAEnumeratedDataType::getEnumerator(int index) const
{
  return _enumeratorList.value(index, 0);
}

QHLAEnumerator*
QHLAEnumeratedDataType::getEnumerator(int index)
{
  return _enumeratorList.value(index, 0);
}

const QString&
QHLAEnumeratedDataType::semantics() const
{
  return _semantics;
}

void
QHLAEnumeratedDataType::setSemantics(const QString& semantics)
{
  _semantics = semantics;
}

int
QHLAEnumeratedDataType::indexInDataTypeList() const
{
  return _indexInDataTypeList;
}

QHLAEnumeratedDataTypeList::QHLAEnumeratedDataTypeList(QObject* parent) :
  QObject(parent)
{
}

QHLAEnumeratedDataTypeList::~QHLAEnumeratedDataTypeList()
{
}

QHLAEnumeratedDataType*
QHLAEnumeratedDataTypeList::createDataType()
{
  QHLAEnumeratedDataType* dataType = new QHLAEnumeratedDataType(this);
  dataType->_indexInDataTypeList = _dataTypeList.size();
  _dataTypeList.push_back(dataType);
  return dataType;
}

void
QHLAEnumeratedDataTypeList::eraseDataType(QHLAEnumeratedDataType* dataType)
{
  Q_ASSERT(dataType->parent() == this);
  int index = dataType->_indexInDataTypeList;
  dataType->_indexInDataTypeList = -1;
  _dataTypeList.removeAt(index);
  for (; index < _dataTypeList.size(); ++index)
    _dataTypeList[index]->_indexInDataTypeList = index;
}

QHLAEnumeratedDataType*
QHLAEnumeratedDataTypeList::getDataType(const QString& name)
{
  for (int i = 0; i < _dataTypeList.size(); ++i) {
    if (_dataTypeList[i]->name() == name)
      return _dataTypeList[i];
  }
  return 0;
}

QHLAEnumeratedDataType*
QHLAEnumeratedDataTypeList::getDataType(int index)
{
  return _dataTypeList.value(index, 0);
}
