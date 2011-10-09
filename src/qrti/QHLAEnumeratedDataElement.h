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

#ifndef QHLAEnumeratedDataElement_h
#define QHLAEnumeratedDataElement_h

#include <Qt/QtCore>
#include "QHLADataElement.h"
#include "QHLAEnumeratedDataType.h"

class QHLAEnumeratedDataElement : public QHLADataElement {
  Q_OBJECT
public:
  QHLAEnumeratedDataElement(QObject* parent, const QHLAEnumeratedDataType* dataType);
  virtual ~QHLAEnumeratedDataElement();

  virtual const QHLAEnumeratedDataType* dataType() const;

  virtual int decode(const QByteArray& byteArray, int pos);

  qint64 numericValue() const;
  void setNumericValue(qint64 numericValue);

private:
  struct DecodeVisitor;

  const QHLAEnumeratedDataType* _dataType;
  qint64 _numericValue;
};

#endif
