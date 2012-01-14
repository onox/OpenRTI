/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich 
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

#ifndef VariableLengthDataImplementation_h
#define VariableLengthDataImplementation_h

#include "VariableLengthData.h"
#include "SharedPtr.h"

namespace rti1516
{

class VariableLengthDataImplementation : public OpenRTI::Referenced {
public:
  VariableLengthDataImplementation()
  { }
  VariableLengthDataImplementation(const OpenRTI::VariableLengthData& variableLengthData) :
    _variableLengthData(variableLengthData)
  { }
  VariableLengthDataImplementation(const void* data, unsigned long size) :
    _variableLengthData(data, size)
  { }
  VariableLengthDataImplementation(const VariableLengthDataImplementation& v) :
    _variableLengthData(v._variableLengthData)
  {
  }
  static void putAndDelete(VariableLengthDataImplementation* data)
  {
    if (OpenRTI::Referenced::put(data))
      return;
    delete data;
  }

  OpenRTI::VariableLengthData _variableLengthData;
};

class VariableLengthDataFriend {
public:
  static VariableLengthData
  create(const OpenRTI::VariableLengthData& variableLengthData)
  {
    VariableLengthData rti1516VariableLengthData;
    rti1516VariableLengthData._impl->_variableLengthData = variableLengthData;
    return rti1516VariableLengthData;
  }
  static const OpenRTI::VariableLengthData&
  readPointer(const rti1516::VariableLengthData& variableLengthData)
  {
    return variableLengthData._impl->_variableLengthData;
  }
  static OpenRTI::VariableLengthData&
  writePointer(rti1516::VariableLengthData& variableLengthData)
  {
    return variableLengthData._impl->_variableLengthData;
  }
};

}

#endif // VariableLengthDataImplementation_h
