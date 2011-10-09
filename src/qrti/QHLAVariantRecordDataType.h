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

#ifndef QHLAVariantRecordDataType_h
#define QHLAVariantRecordDataType_h

#include "QHLADataType.h"

class QHLAEnumeratedDataType;
class QHLAVariantRecordDataType;
class QHLAVariantRecordDataTypeList;

class QHLAAlternative : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString enumerator READ enumerator WRITE setEnumerator)
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(const QHLADataType* type READ type WRITE setType)
  Q_PROPERTY(QString semantics READ semantics WRITE setSemantics)
public:
  QHLAAlternative(QHLAVariantRecordDataType* parent);
  virtual ~QHLAAlternative();

  const QString& enumerator() const;
  void setEnumerator(const QString& enumerator);

  const QString& name() const;
  void setName(const QString& name);

  const QHLADataType* type() const;
  void setType(const QHLADataType* type);

  const QString& semantics() const;
  void setSemantics(const QString& semantics);

  int indexInDataType() const;

private:
  QString _enumerator;
  QString _name;
  const QHLADataType* _type;
  QString _semantics;
  int _indexInDataType;

  friend class QHLAVariantRecordDataType;
};

class QHLAVariantRecordDataType : public QHLADataType {
  Q_OBJECT
  Q_PROPERTY(const QHLAEnumeratedDataType* type READ type WRITE setType)
  Q_PROPERTY(QString encoding READ encoding WRITE setEncoding)
  Q_PROPERTY(QString semantics READ semantics WRITE setSemantics)
public:
  QHLAVariantRecordDataType(QHLAVariantRecordDataTypeList* parent);
  virtual ~QHLAVariantRecordDataType();

  virtual void accept(QHLADataTypeVisitor& visitor) const;

  virtual QHLADataElement* createDataElement(QObject* parent) const;

  const QHLAEnumeratedDataType* type() const;
  void setType(const QHLAEnumeratedDataType* type);

  QHLAAlternative* createAlternative();
  int getNumAlternatives() const;
  const QHLAAlternative* getAlternative(int index) const;
  QHLAAlternative* getAlternative(int index);

  const QString& encoding() const;
  void setEncoding(const QString& encoding);

  const QString& semantics() const;
  void setSemantics(const QString& semantics);

  int indexInDataTypeList() const;

private:
  const QHLAEnumeratedDataType* _type;
  typedef QList<QHLAAlternative*> AlternativeList;
  AlternativeList _alternativeList;
  QString _encoding;
  QString _semantics;
  int _indexInDataTypeList;

  friend class QHLAVariantRecordDataTypeList;
};

class QHLAVariantRecordDataTypeList : public QObject {
  Q_OBJECT
public:
  QHLAVariantRecordDataTypeList(QObject* parent);
  virtual ~QHLAVariantRecordDataTypeList();

  QHLAVariantRecordDataType* createDataType();
  void eraseDataType(QHLAVariantRecordDataType* dataType);
  QHLAVariantRecordDataType* getDataType(const QString& name);
  QHLAVariantRecordDataType* getDataType(int index);
  int getNumDataTypes() const
  { return _dataTypeList.size(); }

private:
  typedef QList<QHLAVariantRecordDataType*> DataTypeList;
  DataTypeList _dataTypeList;
};

#endif
