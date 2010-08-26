/* -*-c++-*- OpenRTI - Copyright (C) 2009-2010 Mathias Froehlich
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

#ifndef OpenRTI_AbstractMessage_h
#define OpenRTI_AbstractMessage_h

#include "Export.h"
#include "Referenced.h"

namespace OpenRTI {

class AbstractMessageDispatcher;
class ConstAbstractMessageDispatcher;

// template<typename T>
// class FunctorMessageDispatcher;
// template<typename T>
// class FunctorConstMessageDispatcher;

// class FederationMessage;
// class ServiceMessage;
// class TimeStampedMessage;

class OPENRTI_API AbstractMessage : public Referenced {
public:
  virtual ~AbstractMessage() {}
  // virtual FederationMessage* toFederationMessage() { return 0; }
  // virtual ServiceMessage* toServiceMessage() { return 0; }
  // virtual TimeStampedMessage* toTimeStampedMessage() { return 0; }
  virtual const char* getTypeName() const = 0;
  virtual void dispatch(AbstractMessageDispatcher&) = 0;
  virtual void dispatch(ConstAbstractMessageDispatcher&) const = 0;

  // template<typename T>
  // void accept(const T& functor)
  // {
  //   FunctorMessageDispatcher<T> dispatcher(functor);
  //   dispatch(dispatcher);
  // }
  // template<typename T>
  // void accept(const T& functor) const
  // {
  //   FunctorConstMessageDispatcher<T> dispatcher(functor);
  //   dispatch(dispatcher);
  // }
};

// Federation message, that means its destionation federation is given in the handle
// class FederationMessage : public AbstractMessage {
// public:
//   virtual ~FederationMessage() {}
//   virtual FederationMessage* toFederationMessage() { return this; }
//
//   const FederationHandle& getFederationHandle() const { return _federationHandle; }
//   void setFederationHandle(const FederationHandle& federationHandle) { _federationHandle = federationHandle; }
//
// private:
//   FederationHandle _federationHandle;
// };

// Introduce that for those messages that are required as a syncronous response in the ambassador
// class ServiceMessage : public FederationMessage {
// public:
//   virtual ~ServiceMessage() {}
//   virtual ServiceMessage* toServiceMessage() { return this; }
// };

// Timestamped message
// class TimeStampedMessage : public FederationMessage {
// public:
//   virtual ~TimeStampedMessage() {}
//   virtual TimeStampedMessage* toTimeStampedMessage() { return this; }
//
//   const VariableLengthData& getTimeStamp() const { return _timeStamp; }
//   void setTimeStamp(const VariableLengthData& timeStamp) { _timeStamp = timeStamp; }
//
// private:
//   VariableLengthData _timeStamp;
// };

} // namespace OpenRTI

#endif
