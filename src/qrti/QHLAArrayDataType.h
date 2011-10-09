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

#ifndef QHLAArrayDataType_h
#define QHLAArrayDataType_h

#include "QHLADataType.h"

class QHLAArrayDataTypeList;

class QHLAArrayDataType : public QHLADataType {
  Q_OBJECT
  Q_PROPERTY(const QHLADataType* elementType READ elementType WRITE setElementType)
  Q_PROPERTY(QString cardinality READ cardinality WRITE setCardinality)
  Q_PROPERTY(QString encoding READ encoding WRITE setEncoding)
  Q_PROPERTY(QString semantics READ semantics WRITE setSemantics)
public:
  QHLAArrayDataType(QHLAArrayDataTypeList* parent);
  virtual ~QHLAArrayDataType();

  virtual void accept(QHLADataTypeVisitor& visitor) const;

  virtual QHLADataElement* createDataElement(QObject* parent) const;

  const QHLADataType* elementType() const;
  void setElementType(const QHLADataType* elementType);

  // Comma seperated list of dimensions, ranges or 'Dynamic'
  // Possible values are '3,3' 'Dynamic' '[4:10]'
  // Currently not all of them are supported FIXME
  const QString& cardinality() const;
  void setCardinality(const QString& cardinality);

  const QString& encoding() const;
  void setEncoding(const QString& encoding);

  const QString& semantics() const;
  void setSemantics(const QString& semantics);

  bool isString() const;
  void setIsString(bool isString);

  bool isOpaqueData() const;
  void setIsOpaqueData(bool isOpaque);

  int numArrayDimensions() const;
  bool arrayDimensionFixed(int index) const;
  int fixedArrayDimension(int index) const;
  int minArrayDimension(int index) const;
  int maxArrayDimension(int index) const;

  int indexInDataTypeList() const;

private:
  const QHLADataType* _elementType;
  QString _cardinality;
  QString _encoding;
  QString _semantics;
  bool _isString;
  bool _isOpaqueData;
  // FIXME support multidimensional arrays and ranges
  int _arrayDimension;
  int _indexInDataTypeList;

  friend class QHLAArrayDataTypeList;
};

class QHLAArrayDataTypeList : public QObject {
  Q_OBJECT
public:
  QHLAArrayDataTypeList(QObject* parent);
  virtual ~QHLAArrayDataTypeList();

  QHLAArrayDataType* createDataType();
  void eraseDataType(QHLAArrayDataType* dataType);
  QHLAArrayDataType* getDataType(const QString& name);
  QHLAArrayDataType* getDataType(int index);
  int getNumDataTypes() const;

private:
  typedef QList<QHLAArrayDataType*> DataTypeList;
  DataTypeList _dataTypeList;
};


#endif
