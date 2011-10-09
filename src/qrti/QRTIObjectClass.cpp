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

#include "QRTIObjectClass.h"

#include "QRTIObjectClassAttribute.h"

QRTIObjectClass::QRTIObjectClass(QObject* parent, const QString& name) :
  QObject(parent),
  _name(name),
  _indexInFederate(-1),
  _indexInParentObjectClass(-1),
  _parentObjectClass(0)
{
}

QRTIObjectClass::~QRTIObjectClass()
{
  if (_parentObjectClass)
    _parentObjectClass->eraseDerivedObjectClass(this);
  for (int i = 0; i < _derivedObjectClassList.size(); ++i) {
    _derivedObjectClassList[i]->_parentObjectClass = 0;
    _derivedObjectClassList[i]->_indexInParentObjectClass = -1;
  }
  for (int i = 0; i < _attributeList.size(); ++i)
    delete _attributeList[i];
}

const QString&
QRTIObjectClass::name() const
{
  return _name;
}

void
QRTIObjectClass::insertDerivedObjectClass(QRTIObjectClass* objectClass)
{
  if (!objectClass)
    return;
  objectClass->_parentObjectClass = this;
  objectClass->_indexInParentObjectClass = _derivedObjectClassList.size();
  _derivedObjectClassList.push_back(objectClass);
  for (int i = 0; i < _attributeList.size(); ++i) {
    objectClass->insertObjectClassAttribute(_attributeList[i]->name());
  }
}

void
QRTIObjectClass::eraseDerivedObjectClass(QRTIObjectClass* objectClass)
{
  if (!objectClass)
    return;
  _derivedObjectClassList.removeAll(objectClass);
  objectClass->_indexInParentObjectClass = -1;
  objectClass->_parentObjectClass = 0;
}

QRTIObjectClassAttribute*
QRTIObjectClass::insertObjectClassAttribute(const QString& name)
{
  QRTIObjectClassAttribute* objectClassAttribute = _createObjectClassAttribute(name);
  for (int i = 0; i < _derivedObjectClassList.size(); ++i)
    _derivedObjectClassList[i]->insertObjectClassAttribute(name);
  return objectClassAttribute;
}

int
QRTIObjectClass::_insertObjectClassAttribute(QRTIObjectClassAttribute* objectClassAttribute)
{
  int index = _attributeList.size();
  _attributeList.push_back(objectClassAttribute);
  _nameAttributeIndexMap[objectClassAttribute->name()] = index;
  return index;
}

int
QRTIObjectClass::getNumObjectClassAttributes() const
{
  return _attributeList.size();
}

int
QRTIObjectClass::getObjectClassAttributeIndex(const QString& name)
{
  return _nameAttributeIndexMap.value(name, -1);
}

QRTIObjectClassAttribute*
QRTIObjectClass::getObjectClassAttribute(int index)
{
  return _attributeList.value(index, 0);
}

QRTIObjectClassAttribute*
QRTIObjectClass::getObjectClassAttribute(const QString& name)
{
  return getObjectClassAttribute(getObjectClassAttributeIndex(name));
}
