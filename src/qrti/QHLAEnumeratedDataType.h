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

#ifndef QHLAEnumeratedDataType_h
#define QHLAEnumeratedDataType_h

#include "QHLADataType.h"

class QHLABasicDataType;
class QHLAEnumeratedDataElement;
class QHLAEnumeratedDataType;
class QHLAEnumeratedDataTypeList;

class QHLAEnumerator : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(QString values READ values WRITE setValues)
public:
  QHLAEnumerator(QHLAEnumeratedDataType* parent);
  virtual ~QHLAEnumerator();

  const QString& name() const;
  void setName(const QString& name);

  const QString& values() const;
  void setValues(const QString& values);

  int indexInDataType() const;

private:
  QString _name;
  QString _values;
  int _indexInDataType;

  friend class QHLAEnumeratedDataType;
};

class QHLAEnumeratedDataType : public QHLADataType {
  Q_OBJECT
  Q_PROPERTY(const QHLABasicDataType* representation READ representation WRITE setRepresentation)
  Q_PROPERTY(QString semantics READ semantics WRITE setSemantics)
public:
  QHLAEnumeratedDataType(QHLAEnumeratedDataTypeList* parent);
  virtual ~QHLAEnumeratedDataType();

  virtual void accept(QHLADataTypeVisitor& visitor) const;

  virtual QHLADataElement* createDataElement(QObject* parent) const;
  QHLAEnumeratedDataElement* createEnumeratedDataElement(QObject* parent) const;

  const QHLABasicDataType* representation() const;
  void setRepresentation(const QHLABasicDataType* representation);

  QHLAEnumerator* createEnumerator();
  int getNumEnumerators() const;
  const QHLAEnumerator* getEnumerator(int index) const;
  QHLAEnumerator* getEnumerator(int index);

  const QString& semantics() const;
  void setSemantics(const QString& semantics);

  int indexInDataTypeList() const;

private:
  const QHLABasicDataType* _representation;
  typedef QList<QHLAEnumerator*> EnumeratorList;
  EnumeratorList _enumeratorList;
  QString _semantics;
  int _indexInDataTypeList;

  friend class QHLAEnumeratedDataTypeList;
};

class QHLAEnumeratedDataTypeList : public QObject {
  Q_OBJECT
public:
  QHLAEnumeratedDataTypeList(QObject* parent);
  virtual ~QHLAEnumeratedDataTypeList();

  QHLAEnumeratedDataType* createDataType();
  void eraseDataType(QHLAEnumeratedDataType* dataType);
  QHLAEnumeratedDataType* getDataType(const QString& name);
  QHLAEnumeratedDataType* getDataType(int index);
  int getNumDataTypes() const
  { return _dataTypeList.size(); }

private:
  typedef QList<QHLAEnumeratedDataType*> DataTypeList;
  DataTypeList _dataTypeList;
};

#endif
