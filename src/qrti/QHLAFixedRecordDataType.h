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

#ifndef QHLAFixedRecordDataType_h
#define QHLAFixedRecordDataType_h

#include "QHLADataType.h"

class QHLAFixedRecordDataType;
class QHLAFixedRecordDataTypeList;

class QHLAField : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(const QHLADataType* type READ type WRITE setType)
  Q_PROPERTY(QString semantics READ semantics WRITE setSemantics)
public:
  QHLAField(QHLAFixedRecordDataType* parent);
  virtual ~QHLAField();

  const QString& name() const;
  void setName(const QString& name);

  const QHLADataType* type() const;
  void setType(const QHLADataType* type);

  const QString& semantics() const;
  void setSemantics(const QString& semantics);

  int indexInDataType() const;

private:
  QString _name;
  const QHLADataType* _type;
  QString _semantics;
  int _indexInDataType;

  friend class QHLAFixedRecordDataType;
};

class QHLAFixedRecordDataType : public QHLADataType {
  Q_OBJECT
  Q_PROPERTY(QString encoding READ encoding WRITE setEncoding)
  Q_PROPERTY(QString semantics READ semantics WRITE setSemantics)
public:
  QHLAFixedRecordDataType(QHLAFixedRecordDataTypeList* parent);
  virtual ~QHLAFixedRecordDataType();

  virtual void accept(QHLADataTypeVisitor& visitor) const;

  virtual QHLADataElement* createDataElement(QObject* parent) const;

  QHLAField* createField();
  int getNumFields() const;
  const QHLAField* getField(int index) const;
  QHLAField* getField(int index);

  const QString& encoding() const;
  void setEncoding(const QString& encoding);

  const QString& semantics() const;
  void setSemantics(const QString& semantics);

  int indexInDataTypeList() const;

private:
  typedef QList<QHLAField*> FieldList;
  FieldList _fieldList;
  QString _encoding;
  QString _semantics;
  int _indexInDataTypeList;

  friend class QHLAFixedRecordDataTypeList;
};

class QHLAFixedRecordDataTypeList : public QObject {
  Q_OBJECT
public:
  QHLAFixedRecordDataTypeList(QObject* parent);
  virtual ~QHLAFixedRecordDataTypeList();

  QHLAFixedRecordDataType* createDataType();
  void eraseDataType(QHLAFixedRecordDataType* dataType);
  QHLAFixedRecordDataType* getDataType(const QString& name);
  QHLAFixedRecordDataType* getDataType(int index);
  int getNumDataTypes() const
  { return _dataTypeList.size(); }

private:
  typedef QList<QHLAFixedRecordDataType*> DataTypeList;
  DataTypeList _dataTypeList;
};

#endif
