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

#include "QRTIFederate.h"

QRTIFederate::QRTIFederate(QObject* parent) :
  QObject(parent),
  _basicDataTypeList(new QHLABasicDataTypeList(this)),
  _simpleDataTypeList(new QHLASimpleDataTypeList(this)),
  _enumeratedDataTypeList(new QHLAEnumeratedDataTypeList(this)),
  _arrayDataTypeList(new QHLAArrayDataTypeList(this)),
  _fixedRecordDataTypeList(new QHLAFixedRecordDataTypeList(this)),
  _variantRecordDataTypeList(new QHLAVariantRecordDataTypeList(this)),
  _objectClassModel(new ObjectClassModel(this)),
  _objectInstanceModel(new ObjectInstanceModel(this))
{
}

QRTIFederate::~QRTIFederate()
{
}

