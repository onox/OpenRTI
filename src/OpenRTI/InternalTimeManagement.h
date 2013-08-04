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

#ifndef OpenRTI_InternalTimeManagement_h
#define OpenRTI_InternalTimeManagement_h

#include "Referenced.h"

namespace OpenRTI {

class InternalTimeManagement : public Referenced {
public:
  InternalTimeManagement() :
    _timeRegulationEnablePending(false),
    _timeRegulationEnabled(false),
    _timeConstrainedEnablePending(false),
    _timeConstrainedEnabled(false),
    _timeAdvancePending(false),
    _nextMessageMode(false),
    _flushQueueMode(false),
    _messageRetractionSerial(0)
  { }
  virtual ~InternalTimeManagement() {}

  bool getTimeRegulationEnabled() const
  { return _timeRegulationEnabled; }
  bool getTimeRegulationEnablePending() const
  { return _timeRegulationEnablePending; }
  bool getTimeRegulationEnabledOrPending() const
  { return _timeRegulationEnabled || _timeRegulationEnablePending; }

  bool getTimeConstrainedEnabled() const
  { return _timeConstrainedEnabled; }
  bool getTimeConstrainedEnablePending() const
  { return _timeConstrainedEnablePending; }
  bool getTimeConstrainedEnabledOrPending() const
  { return _timeConstrainedEnabled || _timeConstrainedEnablePending; }

  bool getTimeAdvancePending() const
  { return _timeAdvancePending; }

  bool getFlushQueueMode() const
  { return _flushQueueMode; }

  uint32_t getNextMessageRetractionSerial()
  { return _messageRetractionSerial++; }

  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const JoinFederateNotifyMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const ResignFederateNotifyMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const EnableTimeRegulationRequestMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const EnableTimeRegulationResponseMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const DisableTimeRegulationRequestMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const CommitLowerBoundTimeStampMessage& message) = 0;

  virtual void queueTimeStampedMessage(InternalAmbassador& ambassador, const VariableLengthData& timeStamp, const AbstractMessage& message) = 0;

protected:

  // State values
  bool _timeRegulationEnablePending;
  bool _timeRegulationEnabled;
  bool _timeConstrainedEnablePending;
  bool _timeConstrainedEnabled;
  bool _timeAdvancePending;
  bool _nextMessageMode;
  bool _flushQueueMode;
  // Serial number for message retraction
  uint32_t _messageRetractionSerial;
};

} // namespace OpenRTI

#endif
