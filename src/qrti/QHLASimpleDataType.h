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

#ifndef QHLASimpleDataType_h
#define QHLASimpleDataType_h

#include "QHLADataType.h"

class QHLABasicDataType;
class QHLASimpleDataTypeList;

class QHLASimpleDataType : public QHLADataType {
  Q_OBJECT
  Q_PROPERTY(const QHLABasicDataType* representation READ representation WRITE setRepresentation)
  Q_PROPERTY(QString units READ units WRITE setUnits)
  Q_PROPERTY(QString resolution READ resolution WRITE setResolution)
  Q_PROPERTY(QString accuracy READ accuracy WRITE setAccuracy)
  Q_PROPERTY(QString semantics READ semantics WRITE setSemantics)
public:
  QHLASimpleDataType(QHLASimpleDataTypeList* parent);
  virtual ~QHLASimpleDataType();

  virtual void accept(QHLADataTypeVisitor& visitor) const;

  virtual QHLADataElement* createDataElement(QObject* parent) const;

  const QHLABasicDataType* representation() const;
  void setRepresentation(const QHLABasicDataType* representation);

  const QString& units() const;
  void setUnits(const QString& units);

  const QString& resolution() const;
  void setResolution(const QString& resolution);

  const QString& accuracy() const;
  void setAccuracy(const QString& accuracy);

  const QString& semantics() const;
  void setSemantics(const QString& semantics);

  int indexInDataTypeList() const;

private:
  const QHLABasicDataType* _representation;
  QString _units;
  QString _resolution;
  QString _accuracy;
  QString _semantics;
  int _indexInDataTypeList;

  friend class QHLASimpleDataTypeList;
};

class QHLASimpleDataTypeList : public QObject {
  Q_OBJECT
public:
  QHLASimpleDataTypeList(QObject* parent);
  virtual ~QHLASimpleDataTypeList();

  QHLASimpleDataType* createDataType();
  void eraseDataType(QHLASimpleDataType* dataType);
  QHLASimpleDataType* getDataType(const QString& name);
  QHLASimpleDataType* getDataType(int index);
  int getNumDataTypes() const
  { return _dataTypeList.size(); }

private:
  typedef QList<QHLASimpleDataType*> DataTypeList;
  DataTypeList _dataTypeList;
};

#endif
