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

#ifndef RTI_HLAinteger64TimeFactory_h
#define RTI_HLAinteger64TimeFactory_h

#include "LogicalTimeFactory.h"

class HLAinteger64Interval;
class HLAinteger64Time;

class RTI_EXPORT_FEDTIME HLAinteger64TimeFactory : public rti1516::LogicalTimeFactory {
public:
  HLAinteger64TimeFactory();
  virtual ~HLAinteger64TimeFactory()
    RTI_NOEXCEPT;
  virtual RTI_UNIQUE_PTR<rti1516::LogicalTime> makeLogicalTime()
    RTI_THROW ((rti1516::InternalError));
  virtual RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval> makeLogicalTimeInterval()
    RTI_THROW ((rti1516::InternalError));
};

#endif
