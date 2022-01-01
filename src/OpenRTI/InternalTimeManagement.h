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

#ifndef OpenRTI_InternalTimeManagement_h
#define OpenRTI_InternalTimeManagement_h

#include "Export.h"
#include "Referenced.h"
#include "Message.h"

namespace OpenRTI {

class InternalAmbassador;

class OPENRTI_API InternalTimeManagement : public Referenced {
public:
  enum TimeRegulationMode {
    TimeRegulationDisabled,
    TimeRegulationEnablePending,
    TimeRegulationEnabled
  };
  enum TimeConstrainedMode {
    TimeConstrainedDisabled,
    TimeConstrainedEnablePending,
    TimeConstrainedEnabled
  };
  enum TimeAdvanceMode {
    TimeAdvanceGranted,
    TimeAdvanceRequest,
    TimeAdvanceRequestAvailable,
    NextMessageRequest,
    NextMessageRequestAvailable,
    FlushQueueRequest
  };

  InternalTimeManagement();
  virtual ~InternalTimeManagement();

  void setTimeRegulationMode(TimeRegulationMode timeRegulationMode)
  { _timeRegulationMode = timeRegulationMode; }
  TimeRegulationMode getTimeRegulationMode() const
  { return _timeRegulationMode; }
  bool getTimeRegulationDisabled() const
  { return _timeRegulationMode == TimeRegulationDisabled; }
  bool getTimeRegulationEnabled() const
  { return _timeRegulationMode == TimeRegulationEnabled; }
  bool getTimeRegulationEnablePending() const
  { return _timeRegulationMode == TimeRegulationEnablePending; }
  bool getTimeRegulationEnabledOrPending() const
  { return !getTimeRegulationDisabled(); }

  void setTimeConstrainedMode(TimeConstrainedMode timeConstrainedMode)
  { _timeConstrainedMode = timeConstrainedMode; }
  TimeConstrainedMode getTimeConstrainedMode() const
  { return _timeConstrainedMode; }
  bool getTimeConstrainedDisabled() const
  { return _timeConstrainedMode == TimeConstrainedDisabled; }
  bool getTimeConstrainedEnabled() const
  { return _timeConstrainedMode == TimeConstrainedEnabled; }
  bool getTimeConstrainedEnablePending() const
  { return _timeConstrainedMode == TimeConstrainedEnablePending; }
  bool getTimeConstrainedEnabledOrPending() const
  { return !getTimeConstrainedDisabled(); }

  void setTimeAdvanceMode(TimeAdvanceMode timeAdvanceMode)
  { _timeAdvanceMode = timeAdvanceMode; }
  TimeAdvanceMode getTimeAdvanceMode() const
  { return _timeAdvanceMode; }
  bool getTimeAdvancePending() const
  { return _timeAdvanceMode != TimeAdvanceGranted; }
  bool getFlushQueueMode() const
  { return _timeAdvanceMode == FlushQueueRequest; }
  bool getIsAnyAdvanceRequest() const
  { return _timeAdvanceMode == TimeAdvanceRequest || _timeAdvanceMode == TimeAdvanceRequestAvailable; }
  bool getIsAnyAvailableMode() const
  { return _timeAdvanceMode == TimeAdvanceRequestAvailable || _timeAdvanceMode == NextMessageRequestAvailable || _timeAdvanceMode == FlushQueueRequest; }
  bool getIsAnyNextMessageMode() const
  { return _timeAdvanceMode == NextMessageRequest || _timeAdvanceMode == NextMessageRequestAvailable; }

  bool getAsynchronousDeliveryEnabled() const
  { return _asynchronousDeliveryEnabled; }
  void setAsynchronousDeliveryEnabled(bool asynchronousDeliveryEnabled)
  { _asynchronousDeliveryEnabled = asynchronousDeliveryEnabled; }

  OrderType getTimeStampOrderDelivery(OrderType orderType) const
  {
    if (!getTimeConstrainedEnabled())
      return RECEIVE;
    return orderType;
  }

  uint32_t getNextMessageRetractionSerial()
  { return _messageRetractionSerial++; }

  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const JoinFederateNotifyMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const ResignFederateNotifyMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const EnableTimeRegulationRequestMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const EnableTimeRegulationResponseMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const DisableTimeRegulationRequestMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const CommitLowerBoundTimeStampMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const CommitLowerBoundTimeStampResponseMessage& message) = 0;
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const LockedByNextMessageRequestMessage& message) = 0;

  virtual void queueTimeStampedMessage(InternalAmbassador& ambassador, const VariableLengthData& timeStamp, const AbstractMessage& message) = 0;
  virtual void queueReceiveOrderMessage(InternalAmbassador& ambassador, const AbstractMessage& message) = 0;

  virtual bool dispatchCallback(const AbstractMessageDispatcher& dispatcher) = 0;
  virtual bool callbackMessageAvailable() = 0;

protected:

  // State values
  TimeRegulationMode _timeRegulationMode;
  TimeConstrainedMode _timeConstrainedMode;
  TimeAdvanceMode _timeAdvanceMode;
  bool _asynchronousDeliveryEnabled;
  // Serial number for message retraction
  uint32_t _messageRetractionSerial;
};

} // namespace OpenRTI

#endif
