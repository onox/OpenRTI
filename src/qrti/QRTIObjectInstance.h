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

#ifndef QRTIObjectInstance_h
#define QRTIObjectInstance_h

#include <Qt/QtCore>

#include "QRTIObjectClass.h"
#include "QRTIObjectClassAttribute.h"
#include "QRTIObjectInstanceAttribute.h"

class QRTIObjectClass;
class QRTIObjectClassAttribute;
class QRTIObjectInstanceAttribute;

class QRTIObjectInstance : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name)
public:
  QRTIObjectInstance(QObject* parent, QRTIObjectClass* objectClass, const QString& name);
  virtual ~QRTIObjectInstance();

  const QString& name() const;

  QRTIObjectClass* getObjectClass()
  { return _objectClass; }

  int getIndexInFederate() const
  { return _indexInFederate; }
  int getIndexInObjectClass() const
  { return _indexInObjectClass; }
  QRTIObjectInstanceAttribute* getObjectInstanceAttribute(int index)
  { return _attributeList.value(index, 0); }
  int getNumObjectInstanceAttributes() const
  { return _attributeList.size(); }
  int getObjectInstanceAttributeIndex(const QString& name)
  { return getObjectClass()->getObjectClassAttributeIndex(name); }

  QRTIObjectInstanceAttribute* insertObjectInstanceAttribute(QRTIObjectClassAttribute* objectClassAttribute)
  {
    QRTIObjectInstanceAttribute* objectInstanceAttribute;
    objectInstanceAttribute = new QRTIObjectInstanceAttribute(this);
    objectInstanceAttribute->_objectClassAttribute = objectClassAttribute;
    objectInstanceAttribute->_objectInstance = this;
    objectInstanceAttribute->_indexInObjectInstance = objectClassAttribute->_indexInObjectClass;
    _attributeList.insert(objectClassAttribute->_indexInObjectClass, objectInstanceAttribute);
    return objectInstanceAttribute;
  }

  QString _name;
  QRTIObjectClass* _objectClass;
  int _indexInFederate;
  int _indexInObjectClass;
  QList<QRTIObjectInstanceAttribute*> _attributeList;
};

#endif
