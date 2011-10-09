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

#ifndef QHLAStringDataElement_h
#define QHLAStringDataElement_h

#include <Qt/QtCore>
#include "QHLAArrayDataType.h"
#include "QHLADataElement.h"

class QHLAStringDataElement : public QHLADataElement {
  Q_OBJECT
  Q_PROPERTY(QString value READ value WRITE setValue)
public:
  QHLAStringDataElement(QObject* parent, const QHLAArrayDataType* dataType);
  virtual ~QHLAStringDataElement();

  virtual const QHLAArrayDataType* dataType() const;

  virtual int decode(const QByteArray& byteArray, int pos);

  const QString& value() const;
  void setValue(const QString& value);

private:
  struct DecodeVisitor;

  const QHLAArrayDataType* _dataType;
  QString _value;
};

#endif
