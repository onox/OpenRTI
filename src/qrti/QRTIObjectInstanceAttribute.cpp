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

#include "QRTIObjectInstanceAttribute.h"

#include "QRTIObjectClassAttribute.h"
#include "QRTIObjectInstance.h"

QRTIObjectInstanceAttribute::QRTIObjectInstanceAttribute(QRTIObjectInstance* parent) :
  QObject(parent),
  _indexInObjectInstance(-1),
  _objectClassAttribute(0),
  _objectInstance(0),
  _dataElement(0)
{
}

QRTIObjectInstanceAttribute::~QRTIObjectInstanceAttribute()
{
}

const QString&
QRTIObjectInstanceAttribute::name() const
{
  return _objectClassAttribute->name();
}

QRTIObjectClassAttribute*
QRTIObjectInstanceAttribute::getObjectClassAttribute()
{
  return _objectClassAttribute;
}
