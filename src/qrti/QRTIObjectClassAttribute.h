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

#ifndef QRTIObjectClassAttribute_h
#define QRTIObjectClassAttribute_h

#include <Qt/QtCore>

class QHLADataType;
class QRTIObjectClass;

class QRTIObjectClassAttribute : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name)
public:
  QRTIObjectClassAttribute(QRTIObjectClass* parentObjectClass, const QString& name);
  virtual ~QRTIObjectClassAttribute();

  const QString& name() const
  { return _name; }

  virtual QRTIObjectClass* getObjectClass()
  // { return static_cast<QRTIObjectClass*>(parent()); }
  { return 0; }

  QHLADataType* dataType() const
  { return _dataType; }
  void setDataType(QHLADataType* dataType)
  { _dataType = dataType; }

  QString _name;
  int _indexInObjectClass;
  QHLADataType* _dataType;
};

#endif
