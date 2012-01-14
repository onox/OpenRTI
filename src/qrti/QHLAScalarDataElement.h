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

#ifndef QHLAScalarDataElement_h
#define QHLAScalarDataElement_h

#include <Qt/QtCore>
#include "QHLADataElement.h"
#include "QHLADataType.h"

class QHLAScalarDataElement : public QHLADataElement {
  Q_OBJECT
  Q_PROPERTY(QVariant value READ value WRITE setValue)
public:
  QHLAScalarDataElement(QObject* parent, const QHLADataType* dataType);
  virtual ~QHLAScalarDataElement();

  virtual const QHLADataType* dataType() const;

  virtual int decode(const QByteArray& byteArray, int pos);

  const QVariant& value() const;
  void setValue(const QVariant& value);

private:
  struct DecodeVisitor;

  const QHLADataType* _dataType;
  QVariant _value;
};

#endif
