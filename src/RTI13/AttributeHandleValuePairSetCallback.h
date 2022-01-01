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

#ifndef AttributeHandleValuePairSetCallback_h
#define AttributeHandleValuePairSetCallback_h

/// The original RTI13 include
#include <RTI.hh>

#include "Message.h"

class OPENRTI_LOCAL AttributeHandleValuePairSetCallback : public RTI::AttributeHandleValuePairSet {
public:
  AttributeHandleValuePairSetCallback(RTI::TransportType transportType, RTI::OrderType orderType);
  virtual ~AttributeHandleValuePairSetCallback();

  virtual RTI::ULong size() const;
  virtual RTI::Handle getHandle(RTI::ULong index) const
    RTI_THROW ((RTI::ArrayIndexOutOfBounds));
  virtual RTI::ULong getValueLength(RTI::ULong index) const
    RTI_THROW ((RTI::ArrayIndexOutOfBounds));
  virtual void getValue(RTI::ULong index, char* data, RTI::ULong& length) const
    RTI_THROW ((RTI::ArrayIndexOutOfBounds));
  virtual char* getValuePointer(RTI::ULong index, RTI::ULong& length) const
    RTI_THROW ((RTI::ArrayIndexOutOfBounds));
  virtual RTI::TransportType getTransportType(RTI::ULong index) const
    RTI_THROW ((RTI::InvalidHandleValuePairSetContext));
  virtual RTI::OrderType getOrderType(RTI::ULong index) const
    RTI_THROW ((RTI::ArrayIndexOutOfBounds,
           RTI::InvalidHandleValuePairSetContext));
  virtual RTI::Region* getRegion(RTI::ULong index) const
    RTI_THROW ((RTI::ArrayIndexOutOfBounds,
           RTI::InvalidHandleValuePairSetContext));
  virtual void add(RTI::Handle handle, const char* data, RTI::ULong length)
    RTI_THROW ((RTI::ValueLengthExceeded,
           RTI::ValueCountExceeded));
  virtual void remove(RTI::Handle handle)
    RTI_THROW ((RTI::ArrayIndexOutOfBounds));
  virtual void moveFrom(const RTI::AttributeHandleValuePairSet&, RTI::ULong&)
    RTI_THROW ((RTI::ValueCountExceeded,
           RTI::ArrayIndexOutOfBounds));
  /// Sigh, should have been called clear???
  virtual void empty();
  /// Do not understand that ...
  virtual RTI::ULong start() const;
  virtual RTI::ULong valid(RTI::ULong i) const;
  virtual RTI::ULong next(RTI::ULong i) const;

  // Accessor used to build up the value
  std::vector<OpenRTI::AttributeValue>& getAttributeValues()
  { return _attributeValues; }

private:
  mutable std::vector<OpenRTI::AttributeValue> _attributeValues;
  RTI::TransportType _transportType;
  RTI::OrderType _orderType;
};

#endif
