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

#ifndef QHLAVariantRecordDataElement_h
#define QHLAVariantRecordDataElement_h

#include <Qt/QtCore>
#include "QHLADataElement.h"
#include "QHLAVariantRecordDataType.h"

class QHLAEnumeratedDataElement;

class QHLAVariantRecordDataElement : public QHLADataElement {
  Q_OBJECT
public:
  QHLAVariantRecordDataElement(QObject* parent, const QHLAVariantRecordDataType* dataType);
  virtual ~QHLAVariantRecordDataElement();

  virtual const QHLAVariantRecordDataType* dataType() const;

  virtual int decode(const QByteArray& byteArray, int pos);

private:
  const QHLAVariantRecordDataType* _dataType;

  QHLAEnumeratedDataElement* _enumeratedDataElement;
  QHLADataElement* _dataElement;
};

#endif
