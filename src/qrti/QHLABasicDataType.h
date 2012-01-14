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

#ifndef QHLABasicDataType_h
#define QHLABasicDataType_h

#include "QHLADataType.h"

class QHLABasicDataTypeList;

class QHLABasicDataType : public QHLADataType {
  Q_OBJECT
  Q_PROPERTY(unsigned size READ size WRITE setSize)
  Q_PROPERTY(QString interpretation READ interpretation WRITE setInterpretation)
  Q_PROPERTY(Endian endian READ endian WRITE setEndian)
  Q_PROPERTY(QString encoding READ encoding WRITE setEncoding)
  Q_ENUMS(Endian)
public:
  QHLABasicDataType(QHLABasicDataTypeList* parent);
  virtual ~QHLABasicDataType();

  virtual void accept(QHLADataTypeVisitor& visitor) const;

  virtual QHLADataElement* createDataElement(QObject* parent) const;

  unsigned size() const;
  void setSize(unsigned size);

  const QString& interpretation() const;
  void setInterpretation(const QString& interpretation);

  enum Endian {
    LittleEndian,
    BigEndian
  };

  Endian endian() const;
  void setEndian(Endian endian);

  enum Type {
    SignedType,
    UnsignedType,
    FloatType
  };

  Type type() const;
  void setType(Type type);

  const QString& encoding() const;
  void setEncoding(const QString& encoding);

  int indexInDataTypeList() const;

private:
  unsigned _size;
  Endian _endian;
  QString _interpretation;
  Type _type;
  QString _encoding;
  int _indexInDataTypeList;

  friend class QHLABasicDataTypeList;
};

class QHLABasicDataTypeList : public QObject {
  Q_OBJECT
public:
  QHLABasicDataTypeList(QObject* parent);
  virtual ~QHLABasicDataTypeList();

  QHLABasicDataType* createDataType();
  void eraseDataType(QHLABasicDataType* dataType);
  QHLABasicDataType* getDataType(const QString& name);
  QHLABasicDataType* getDataType(int index);
  int getNumDataTypes() const
  { return _dataTypeList.size(); }

private:
  typedef QList<QHLABasicDataType*> DataTypeList;
  DataTypeList _dataTypeList;
};

#endif
