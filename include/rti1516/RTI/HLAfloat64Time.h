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

#ifndef RTI_HLAfloat64Time_h
#define RTI_HLAfloat64Time_h

#include "LogicalTime.h"

class HLAfloat64Interval;
class HLAfloat64TimeImpl;

class RTI_EXPORT_FEDTIME HLAfloat64Time : public rti1516::LogicalTime {
public:
  HLAfloat64Time();
  HLAfloat64Time(double value);
  HLAfloat64Time(const rti1516::LogicalTime& logicalTime);
  HLAfloat64Time(const HLAfloat64Time& logicalTime);
  virtual ~HLAfloat64Time() RTI_NOEXCEPT;

  virtual void setInitial();
  virtual bool isInitial() const;

  virtual void setFinal();
  virtual bool isFinal() const;

  virtual HLAfloat64Time& operator=(const rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::InvalidLogicalTime));

  virtual HLAfloat64Time& operator+=(const rti1516::LogicalTimeInterval& logicalTimeInterval)
    RTI_THROW ((rti1516::IllegalTimeArithmetic, rti1516::InvalidLogicalTimeInterval));
  virtual HLAfloat64Time& operator-=(const rti1516::LogicalTimeInterval& logicalTimeInterval)
    RTI_THROW ((rti1516::IllegalTimeArithmetic, rti1516::InvalidLogicalTimeInterval));

  virtual bool operator>(rti1516::LogicalTime const & value) const
    RTI_THROW ((rti1516::InvalidLogicalTime));
  virtual bool operator<(rti1516::LogicalTime const & value) const
    RTI_THROW ((rti1516::InvalidLogicalTime));
  virtual bool operator==(rti1516::LogicalTime const & value) const
    RTI_THROW ((rti1516::InvalidLogicalTime));
  virtual bool operator>=(rti1516::LogicalTime const & value) const
    RTI_THROW ((rti1516::InvalidLogicalTime));
  virtual bool operator<=(rti1516::LogicalTime const & value) const
    RTI_THROW ((rti1516::InvalidLogicalTime));

  virtual rti1516::VariableLengthData encode() const;
  virtual unsigned long encodedLength() const;
  virtual unsigned long encode(void* buffer, unsigned long bufferSize) const
    RTI_THROW ((rti1516::CouldNotEncode));

  virtual void decode(const rti1516::VariableLengthData& variableLengthData)
    RTI_THROW ((rti1516::InternalError, rti1516::CouldNotDecode));
  virtual void decode(void* buffer, unsigned long bufferSize)
    RTI_THROW ((rti1516::InternalError, rti1516::CouldNotDecode));

  virtual std::wstring toString() const;
  virtual std::wstring implementationName() const;

  double getTime() const;
  void setTime(double value);

  HLAfloat64Time& operator=(const HLAfloat64Time&);
  operator double() const;

private:
  friend class HLAfloat64Interval;
  HLAfloat64TimeImpl* _impl;
};

#endif
