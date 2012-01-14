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

#include "QHLAFixedRecordDataType.h"

#include "QHLADataTypeVisitor.h"
#include "QHLAFixedRecordDataElement.h"

QHLAField::QHLAField(QHLAFixedRecordDataType* parent) :
  QObject(parent),
  _type(0),
  _indexInDataType(-1)
{
}

QHLAField::~QHLAField()
{
}

const QString&
QHLAField::name() const
{
  return _name;
}

void
QHLAField::setName(const QString& name)
{
  _name = name;
}

const QHLADataType*
QHLAField::type() const
{
  return _type;
}

void
QHLAField::setType(const QHLADataType* type)
{
  _type = type;
}

const QString&
QHLAField::semantics() const
{
  return _semantics;
}

void
QHLAField::setSemantics(const QString& semantics)
{
  _semantics = semantics;
}

int
QHLAField::indexInDataType() const
{
  return _indexInDataType;
}

QHLAFixedRecordDataType::QHLAFixedRecordDataType(QHLAFixedRecordDataTypeList* parent) :
  QHLADataType(parent),
  _indexInDataTypeList(-1)
{
}

QHLAFixedRecordDataType::~QHLAFixedRecordDataType()
{
}

void
QHLAFixedRecordDataType::accept(QHLADataTypeVisitor& visitor) const
{
  visitor.apply(*this);
}

QHLADataElement*
QHLAFixedRecordDataType::createDataElement(QObject* parent) const
{
  return new QHLAFixedRecordDataElement(parent, this);
}

QHLAField*
QHLAFixedRecordDataType::createField()
{
  QHLAField* field = new QHLAField(this);
  field->_indexInDataType = _fieldList.size();
  _fieldList.push_back(field);
  return field;
}

int
QHLAFixedRecordDataType::getNumFields() const
{
  return _fieldList.size();
}

const QHLAField*
QHLAFixedRecordDataType::getField(int index) const
{
  return _fieldList.value(index, 0);
}

QHLAField*
QHLAFixedRecordDataType::getField(int index)
{
  return _fieldList.value(index, 0);
}

const QString&
QHLAFixedRecordDataType::encoding() const
{
  return _encoding;
}

void
QHLAFixedRecordDataType::setEncoding(const QString& encoding)
{
  _encoding = encoding;
}

const QString&
QHLAFixedRecordDataType::semantics() const
{
  return _semantics;
}

void
QHLAFixedRecordDataType::setSemantics(const QString& semantics)
{
  _semantics = semantics;
}

int
QHLAFixedRecordDataType::indexInDataTypeList() const
{
  return _indexInDataTypeList;
}

QHLAFixedRecordDataTypeList::QHLAFixedRecordDataTypeList(QObject* parent) :
  QObject(parent)
{
}

QHLAFixedRecordDataTypeList::~QHLAFixedRecordDataTypeList()
{
}

QHLAFixedRecordDataType*
QHLAFixedRecordDataTypeList::createDataType()
{
  QHLAFixedRecordDataType* dataType = new QHLAFixedRecordDataType(this);
  dataType->_indexInDataTypeList = _dataTypeList.size();
  _dataTypeList.push_back(dataType);
  return dataType;
}

void
QHLAFixedRecordDataTypeList::eraseDataType(QHLAFixedRecordDataType* dataType)
{
  Q_ASSERT(dataType->parent() == this);
  int index = dataType->_indexInDataTypeList;
  dataType->_indexInDataTypeList = -1;
  _dataTypeList.removeAt(index);
  for (; index < _dataTypeList.size(); ++index)
    _dataTypeList[index]->_indexInDataTypeList = index;
}

QHLAFixedRecordDataType*
QHLAFixedRecordDataTypeList::getDataType(const QString& name)
{
  for (int i = 0; i < _dataTypeList.size(); ++i) {
    if (_dataTypeList[i]->name() == name)
      return _dataTypeList[i];
  }
  return 0;
}

QHLAFixedRecordDataType*
QHLAFixedRecordDataTypeList::getDataType(int index)
{
  return _dataTypeList.value(index, 0);
}
