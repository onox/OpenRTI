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

#ifndef QHLAFixedRecordDataElement_h
#define QHLAFixedRecordDataElement_h

#include <Qt/QtCore>
#include "QHLADataElement.h"
#include "QHLAFixedRecordDataType.h"

class QHLAFixedRecordDataElement : public QHLADataElement {
  Q_OBJECT
public:
  QHLAFixedRecordDataElement(QObject* parent, const QHLAFixedRecordDataType* dataType);
  virtual ~QHLAFixedRecordDataElement();

  virtual const QHLAFixedRecordDataType* dataType() const;

  virtual int decode(const QByteArray& byteArray, int pos);

  int numFields() const;
  const QHLADataElement* field(int index) const;
  QHLADataElement* field(int index);

private:
  const QHLAFixedRecordDataType* _dataType;
  QList<QHLADataElement*> _fieldList;
};

#endif
