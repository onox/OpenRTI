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

#ifndef QRTIObjectInstanceAttribute_h
#define QRTIObjectInstanceAttribute_h

#include <Qt/QtCore>

class QHLADataElement;
class QRTIObjectClassAttribute;
class QRTIObjectInstance;

class QRTIObjectInstanceAttribute : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name)
public:
  QRTIObjectInstanceAttribute(QRTIObjectInstance* parent);
  virtual ~QRTIObjectInstanceAttribute();

  const QString& name() const;
  QRTIObjectClassAttribute* getObjectClassAttribute();

  int getIndexInObjectInstance() const
  { return _indexInObjectInstance; }
  QRTIObjectInstance* getObjectInstance()
  { return _objectInstance; }

  QHLADataElement* getDataElement()
  { return _dataElement; }
  void setDataElement(QHLADataElement* dataElement)
  {
    delete _dataElement;
    _dataElement = dataElement;
  }

  int _indexInObjectInstance;
  QRTIObjectClassAttribute* _objectClassAttribute;
  QRTIObjectInstance* _objectInstance;
  QByteArray _rawData;
  QHLADataElement* _dataElement;
};

#endif
