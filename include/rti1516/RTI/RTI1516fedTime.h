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

#ifndef RTI_RTI1516fedTime_h
#define RTI_RTI1516fedTime_h

#include <memory>
#include <string>

#include "HLAfloat64Time.h"
#include "HLAfloat64Interval.h"
#include "LogicalTimeFactory.h"

// more or less certi compatible double valued fedtime implementation

class RTI_EXPORT_FEDTIME RTI1516fedTime : public HLAfloat64Time {
public:
  RTI1516fedTime();
  RTI1516fedTime(double value);
  RTI1516fedTime(const rti1516::LogicalTime& logicalTime);
  RTI1516fedTime(const RTI1516fedTime& fedTime);
  virtual ~RTI1516fedTime() RTI_NOEXCEPT;

  virtual std::wstring implementationName() const;

  double getFedTime() const
  { return getTime(); }

  bool isInfinity() const
  { return isFinal(); }
};

class RTI_EXPORT_FEDTIME RTI1516fedTimeInterval : public HLAfloat64Interval {
public:
  RTI1516fedTimeInterval();
  RTI1516fedTimeInterval(double);
  RTI1516fedTimeInterval(const rti1516::LogicalTimeInterval&);
  RTI1516fedTimeInterval(const RTI1516fedTimeInterval&);
  virtual ~RTI1516fedTimeInterval() RTI_NOEXCEPT;

  virtual std::wstring implementationName() const;

  // double getEpsilon() const
  // { return _epsilon; }
};

class RTI_EXPORT_FEDTIME RTI1516fedTimeFactory : public rti1516::LogicalTimeFactory {
public:
  RTI1516fedTimeFactory();
  virtual ~RTI1516fedTimeFactory() RTI_NOEXCEPT;
  virtual RTI_UNIQUE_PTR<rti1516::LogicalTime> makeLogicalTime()
    RTI_THROW ((rti1516::InternalError));
  virtual RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval> makeLogicalTimeInterval()
    RTI_THROW ((rti1516::InternalError));
};

#endif
