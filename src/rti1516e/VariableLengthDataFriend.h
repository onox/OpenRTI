/* -*-c++-*- OpenRTI - Copyright (C) 2009-2022 Mathias Froehlich
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

#ifndef VariableLengthDataFriend_h
#define VariableLengthDataFriend_h

#include "VariableLengthData.h"

namespace rti1516e
{

class RTI_EXPORT VariableLengthDataFriend {
public:
  static VariableLengthData
  create(const OpenRTI::VariableLengthData& variableLengthData);
  static const OpenRTI::VariableLengthData&
  readPointer(const rti1516e::VariableLengthData& variableLengthData);
  static OpenRTI::VariableLengthData&
  writePointer(rti1516e::VariableLengthData& variableLengthData);
};

}

#endif // VariableLengthDataFriend_h
