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

#ifndef QHLADataElement_h
#define QHLADataElement_h

#include <Qt/QtCore>
#include "QHLADataType.h"

class QHLADataElement : public QObject {
  Q_OBJECT
  Q_PROPERTY(const QHLADataType* dataType READ dataType)
public:
  QHLADataElement(QObject* parent);
  virtual ~QHLADataElement();

  virtual const QHLADataType* dataType() const = 0;

  virtual int decode(const QByteArray& byteArray, int pos) = 0;

  int indexInParent() const
  { return _indexInParent; }

// private:
  int _indexInParent;
};

#endif
