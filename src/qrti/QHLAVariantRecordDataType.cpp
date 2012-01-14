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

#include "QHLAVariantRecordDataType.h"

#include "QHLADataTypeVisitor.h"
#include "QHLAEnumeratedDataType.h"
#include "QHLAVariantRecordDataElement.h"

QHLAAlternative::QHLAAlternative(QHLAVariantRecordDataType* parent) :
  QObject(parent),
  _type(0),
  _indexInDataType(-1)
{
}

QHLAAlternative::~QHLAAlternative()
{
}

const QString&
QHLAAlternative::enumerator() const
{
  return _enumerator;
}

void
QHLAAlternative::setEnumerator(const QString& enumerator)
{
  _enumerator = enumerator;
}

const QString&
QHLAAlternative::name() const
{
  return _name;
}

void
QHLAAlternative::setName(const QString& name)
{
  _name = name;
}

const QHLADataType*
QHLAAlternative::type() const
{
  return _type;
}

void
QHLAAlternative::setType(const QHLADataType* type)
{
  _type = type;
}

const QString&
QHLAAlternative::semantics() const
{
  return _semantics;
}

void
QHLAAlternative::setSemantics(const QString& semantics)
{
  _semantics = semantics;
}

int
QHLAAlternative::indexInDataType() const
{
  return _indexInDataType;
}

QHLAVariantRecordDataType::QHLAVariantRecordDataType(QHLAVariantRecordDataTypeList* parent) :
  QHLADataType(parent),
  _type(0),
  _indexInDataTypeList(-1)
{
}

QHLAVariantRecordDataType::~QHLAVariantRecordDataType()
{
}

void
QHLAVariantRecordDataType::accept(QHLADataTypeVisitor& visitor) const
{
  visitor.apply(*this);
}

QHLADataElement*
QHLAVariantRecordDataType::createDataElement(QObject* parent) const
{
  return new QHLAVariantRecordDataElement(parent, this);
}

const QHLAEnumeratedDataType*
QHLAVariantRecordDataType::type() const
{
  return _type;
}

void
QHLAVariantRecordDataType::setType(const QHLAEnumeratedDataType* type)
{
  _type = type;
}

QHLAAlternative*
QHLAVariantRecordDataType::createAlternative()
{
  QHLAAlternative* alternative = new QHLAAlternative(this);
  alternative->_indexInDataType = _alternativeList.size();
  _alternativeList.push_back(alternative);
  return alternative;
}

int
QHLAVariantRecordDataType::getNumAlternatives() const
{
  return _alternativeList.size();
}

const QHLAAlternative*
QHLAVariantRecordDataType::getAlternative(int index) const
{
  return _alternativeList.value(index, 0);
}

QHLAAlternative*
QHLAVariantRecordDataType::getAlternative(int index)
{
  return _alternativeList.value(index, 0);
}

const QString&
QHLAVariantRecordDataType::encoding() const
{
  return _encoding;
}

void
QHLAVariantRecordDataType::setEncoding(const QString& encoding)
{
  _encoding = encoding;
}

const QString&
QHLAVariantRecordDataType::semantics() const
{
  return _semantics;
}

void
QHLAVariantRecordDataType::setSemantics(const QString& semantics)
{
  _semantics = semantics;
}

int
QHLAVariantRecordDataType::indexInDataTypeList() const
{
  return _indexInDataTypeList;
}

QHLAVariantRecordDataTypeList::QHLAVariantRecordDataTypeList(QObject* parent) :
  QObject(parent)
{
}

QHLAVariantRecordDataTypeList::~QHLAVariantRecordDataTypeList()
{
}

QHLAVariantRecordDataType*
QHLAVariantRecordDataTypeList::createDataType()
{
  QHLAVariantRecordDataType* dataType = new QHLAVariantRecordDataType(this);
  dataType->_indexInDataTypeList = _dataTypeList.size();
  _dataTypeList.push_back(dataType);
  return dataType;
}

void
QHLAVariantRecordDataTypeList::eraseDataType(QHLAVariantRecordDataType* dataType)
{
  Q_ASSERT(dataType->parent() == this);
  int index = dataType->_indexInDataTypeList;
  dataType->_indexInDataTypeList = -1;
  _dataTypeList.removeAt(index);
  for (; index < _dataTypeList.size(); ++index)
    _dataTypeList[index]->_indexInDataTypeList = index;
}

QHLAVariantRecordDataType*
QHLAVariantRecordDataTypeList::getDataType(const QString& name)
{
  for (int i = 0; i < _dataTypeList.size(); ++i) {
    if (_dataTypeList[i]->name() == name)
      return _dataTypeList[i];
  }
  return 0;
}

QHLAVariantRecordDataType*
QHLAVariantRecordDataTypeList::getDataType(int index)
{
  return _dataTypeList.value(index, 0);
}
