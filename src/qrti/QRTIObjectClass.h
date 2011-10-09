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

#ifndef QRTIObjectClass_h
#define QRTIObjectClass_h

#include <Qt/QtCore>

class QRTIObjectClassAttribute;

class QRTIObjectClass : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name)
public:
  QRTIObjectClass(QObject* parent, const QString& name);
  virtual ~QRTIObjectClass();

  const QString& name() const;

  void insertDerivedObjectClass(QRTIObjectClass* objectClass);
  void eraseDerivedObjectClass(QRTIObjectClass* objectClass);

  QRTIObjectClassAttribute* insertObjectClassAttribute(const QString& name);

  virtual QRTIObjectClassAttribute* _createObjectClassAttribute(const QString& name) = 0;
  int _insertObjectClassAttribute(QRTIObjectClassAttribute* objectClassAttribute);

  int getNumObjectClassAttributes() const;
  int getObjectClassAttributeIndex(const QString& name);
  QRTIObjectClassAttribute* getObjectClassAttribute(int index);
  QRTIObjectClassAttribute* getObjectClassAttribute(const QString& name);

  virtual void subscribe()
  {
    /// FIXME think about something that sets things dirty and schedules an idle timeout timer to actually execute what is changed
  }

  QString _name;
  int _indexInFederate;
  int _indexInParentObjectClass;

  QRTIObjectClass* _parentObjectClass;
  QList<QRTIObjectClass*> _derivedObjectClassList;

  QMap<QString, int> _nameAttributeIndexMap;
  QList<QRTIObjectClassAttribute*> _attributeList;
};

#endif
