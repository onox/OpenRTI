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

#include "QRTIObjectInstance.h"

#include "QHLADataElement.h"
#include "QHLADataType.h"
#include "QRTIObjectClass.h"
#include "QRTIObjectClassAttribute.h"
#include "QRTIObjectInstanceAttribute.h"

QRTIObjectInstance::QRTIObjectInstance(QObject* parent, QRTIObjectClass* objectClass, const QString& name) :
  QObject(parent),
  _name(name),
  _objectClass(objectClass),
  _indexInFederate(-1),
  _indexInObjectClass(-1)
{
  for (int i = 0; i < objectClass->getNumObjectClassAttributes(); ++i) {
    QRTIObjectClassAttribute* classAttribute;
    classAttribute = objectClass->getObjectClassAttribute(i);
    QRTIObjectInstanceAttribute* instanceAttribute;
    instanceAttribute = insertObjectInstanceAttribute(classAttribute);
    if (classAttribute->_dataType) {
      instanceAttribute->_dataElement = classAttribute->_dataType->createDataElement(instanceAttribute);
      instanceAttribute->_dataElement->_indexInParent = 0;
    }
  }
}

QRTIObjectInstance::~QRTIObjectInstance()
{
}

const QString&
QRTIObjectInstance::name() const
{
  return _name;
}
