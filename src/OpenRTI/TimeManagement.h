/* -*-c++-*- OpenRTI - Copyright (C) 2009-2013 Mathias Froehlich
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

#ifndef OpenRTI_TimeManagement_h
#define OpenRTI_TimeManagement_h

#include "InternalTimeManagement.h"

namespace OpenRTI {

template<typename T>
class Ambassador;
class InternalAmbassador;

template<typename T>
class TimeManagement : public InternalTimeManagement {
public:
  typedef T Traits;

  typedef typename Traits::NativeLogicalTime NativeLogicalTime;
  typedef typename Traits::NativeLogicalTimeInterval NativeLogicalTimeInterval;

  TimeManagement()
  { }
  virtual ~TimeManagement()
  { }

  virtual bool isLogicalTimeInThePast(const NativeLogicalTime& logicalTime) = 0;
  virtual bool isLogicalTimeStrictlyInThePast(const NativeLogicalTime& logicalTime) = 0;
  virtual bool logicalTimeAlreadyPassed(const NativeLogicalTime& logicalTime) = 0;

  virtual void enableTimeRegulation(Ambassador<T>& ambassador, const NativeLogicalTimeInterval& nativeLookahead) = 0;
  virtual void enableTimeRegulation(Ambassador<T>& ambassador, const NativeLogicalTime& nativeLogicalTime, const NativeLogicalTimeInterval& nativeLookahead) = 0;
  virtual void disableTimeRegulation(Ambassador<T>& ambassador) = 0;

  virtual void enableTimeConstrained(Ambassador<T>& ambassador) = 0;
  virtual void disableTimeConstrained(Ambassador<T>& ambassador) = 0;

  virtual void timeAdvanceRequest(InternalAmbassador& ambassador, const NativeLogicalTime& nativeLogicalTime, bool availableMode, bool nextMessageMode) = 0;
  virtual void flushQueueRequest(Ambassador<T>& ambassador, const NativeLogicalTime& nativeLogicalTime) = 0;

  virtual bool queryGALT(Ambassador<T>& ambassador, NativeLogicalTime& logicalTime) = 0;
  virtual void queryLogicalTime(Ambassador<T>& ambassador, NativeLogicalTime& logicalTime) = 0;
  virtual bool queryLITS(Ambassador<T>& ambassador, NativeLogicalTime& logicalTime) = 0;
  virtual void modifyLookahead(Ambassador<T>& ambassador, const NativeLogicalTimeInterval& nativeLookahead) = 0;
  virtual void queryLookahead(Ambassador<T>& ambassador, NativeLogicalTimeInterval& logicalTimeInterval) = 0;

  virtual std::string logicalTimeToString(const NativeLogicalTime& nativeLogicalTime) = 0;
  virtual std::string logicalTimeIntervalToString(const NativeLogicalTimeInterval& nativeLogicalTimeInterval) = 0;
  virtual bool isPositiveLogicalTimeInterval(const NativeLogicalTimeInterval& nativeLogicalTimeInterval) = 0;

  virtual VariableLengthData encodeLogicalTime(const NativeLogicalTime& nativeLogicalTime) = 0;

  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeConstrainedEnabledMessage& message) = 0;
  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeRegulationEnabledMessage& message) = 0;
  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeAdvanceGrantedMessage& message) = 0;
  virtual void reflectAttributeValues(Ambassador<T>& ambassador, const Federate::ObjectClass& objectClass,
                                      bool flushQueueMode, bool timeConstrainedEnabled,
                                      const TimeStampedAttributeUpdateMessage& message) = 0;
  virtual void removeObjectInstance(Ambassador<T>& ambassador, bool flushQueueMode, bool timeConstrainedEnabled,
                                    const TimeStampedDeleteObjectInstanceMessage& message) = 0;
  virtual void receiveInteraction(Ambassador<T>& ambassador, const InteractionClassHandle& interactionClassHandle,
                                  const Federate::InteractionClass& interactionClass, bool flushQueueMode,
                                  bool timeConstrainedEnabled, const TimeStampedInteractionMessage& message) = 0;
};

} // namespace OpenRTI

#endif
