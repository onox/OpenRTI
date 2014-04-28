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

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>
#include <memory>

#include "RTIambassadorImplementation.h"

#include <algorithm>
#include <fstream>

#include <RTI/RTIambassador.h>
#include <RTI/FederateAmbassador.h>
#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/RangeBounds.h>

#include "Ambassador.h"
#include "DynamicModule.h"
#include "FDD1516EFileReader.h"
#include "LogStream.h"
#include "TemplateTimeManagement.h"

#include "HandleImplementation.h"
#include "RTI1516ELogicalTimeFactory.h"
#include "RTI1516Einteger64TimeFactory.h"
#include "RTI1516Efloat64TimeFactory.h"
#include "VariableLengthDataImplementation.h"

namespace OpenRTI {

static std::list<std::string> findHLAstandardMIMCandidates()
{
  std::list<std::string> candidates;

  std::string moduleName = DynamicModule::getFileNameForAddress((const void*)findHLAstandardMIMCandidates);
  if (!moduleName.empty()) {
    moduleName = getBasePart(moduleName);
    // This gets us to the parent directory
    moduleName = getBasePart(moduleName);
    candidates.push_back(moduleName + std::string("/share/OpenRTI/rti1516e/HLAstandardMIM.xml"));
  }

  // Puh, don't know the encoding of this definition originating from cmake.
  // The user of this result assumes this to be utf8
  candidates.push_back(OPENRTI_DATAROOTDIR "/rti1516e/HLAstandardMIM.xml");

  return candidates;
}

static OpenRTI::CallbackModel translate(rti1516e::CallbackModel callbackModel)
{
  switch (callbackModel) {
  case rti1516e::HLA_IMMEDIATE:
    return OpenRTI::HLA_IMMEDIATE;
  case rti1516e::HLA_EVOKED:
  default:
    return OpenRTI::HLA_EVOKED;
  }
}

static rti1516e::OrderType translate(OpenRTI::OrderType orderType)
{
  switch (orderType) {
  case OpenRTI::TIMESTAMP:
    return rti1516e::TIMESTAMP;
  case OpenRTI::RECEIVE:
  default:
    return rti1516e::RECEIVE;
  }
}

static OpenRTI::OrderType translate(rti1516e::OrderType orderType)
{
  switch (orderType) {
  case rti1516e::TIMESTAMP:
    return OpenRTI::TIMESTAMP;
  case rti1516e::RECEIVE:
  default:
    return OpenRTI::RECEIVE;
  }
}

static rti1516e::TransportationType translate(OpenRTI::TransportationType transportationType)
{
  switch (transportationType) {
  case OpenRTI::BEST_EFFORT:
    return rti1516e::BEST_EFFORT;
  case OpenRTI::RELIABLE:
  default:
    return rti1516e::RELIABLE;
  }
}

static OpenRTI::TransportationType translate(rti1516e::TransportationType transportationType)
{
  switch (transportationType) {
  case rti1516e::BEST_EFFORT:
    return OpenRTI::BEST_EFFORT;
  case rti1516e::RELIABLE:
  default:
    return OpenRTI::RELIABLE;
  }
}

static OpenRTI::ResignAction translate(rti1516e::ResignAction resignAction)
{
  switch (resignAction) {
  case rti1516e::UNCONDITIONALLY_DIVEST_ATTRIBUTES:
    return OpenRTI::UNCONDITIONALLY_DIVEST_ATTRIBUTES;
  case rti1516e::DELETE_OBJECTS:
    return OpenRTI::DELETE_OBJECTS;
  case rti1516e::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS:
    return OpenRTI::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS;
  case rti1516e::DELETE_OBJECTS_THEN_DIVEST:
    return OpenRTI::DELETE_OBJECTS_THEN_DIVEST;
  case rti1516e::CANCEL_THEN_DELETE_THEN_DIVEST:
    return OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST;
  case rti1516e::NO_ACTION:
    return OpenRTI::NO_ACTION;
  default:
    return OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST;
  }
}

static rti1516e::ResignAction translate(OpenRTI::ResignAction resignAction)
{
  switch (resignAction) {
  case OpenRTI::UNCONDITIONALLY_DIVEST_ATTRIBUTES:
    return rti1516e::UNCONDITIONALLY_DIVEST_ATTRIBUTES;
  case OpenRTI::DELETE_OBJECTS:
    return rti1516e::DELETE_OBJECTS;
  case OpenRTI::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS:
    return rti1516e::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS;
  case OpenRTI::DELETE_OBJECTS_THEN_DIVEST:
    return rti1516e::DELETE_OBJECTS_THEN_DIVEST;
  case OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST:
    return rti1516e::CANCEL_THEN_DELETE_THEN_DIVEST;
  case OpenRTI::NO_ACTION:
    return rti1516e::NO_ACTION;
  default:
    return rti1516e::CANCEL_THEN_DELETE_THEN_DIVEST;
  }
}

static OpenRTI::ServiceGroupIndicator translate(rti1516e::ServiceGroup serviceGroup)
{
  switch (serviceGroup) {
  case rti1516e::FEDERATION_MANAGEMENT:
    return OpenRTI::FEDERATION_MANAGEMENT;
  case rti1516e::DECLARATION_MANAGEMENT:
    return OpenRTI::DECLARATION_MANAGEMENT;
  case rti1516e::OBJECT_MANAGEMENT:
    return OpenRTI::OBJECT_MANAGEMENT;
  case rti1516e::OWNERSHIP_MANAGEMENT:
    return OpenRTI::OWNERSHIP_MANAGEMENT;
  case rti1516e::TIME_MANAGEMENT:
    return OpenRTI::TIME_MANAGEMENT;
  case rti1516e::DATA_DISTRIBUTION_MANAGEMENT:
    return OpenRTI::DATA_DISTRIBUTION_MANAGEMENT;
  case rti1516e::SUPPORT_SERVICES:
    return OpenRTI::SUPPORT_SERVICES;
  default:
    throw OpenRTI::InvalidServiceGroup();
  }
}

class OPENRTI_LOCAL _I1516EFederateHandle : public OpenRTI::FederateHandle {
public:
  _I1516EFederateHandle(const rti1516e::FederateHandle& federateHandle)
  { rti1516e::FederateHandleFriend::copy(*this, federateHandle); }
};
class OPENRTI_LOCAL _I1516EObjectClassHandle : public OpenRTI::ObjectClassHandle {
public:
  _I1516EObjectClassHandle(const rti1516e::ObjectClassHandle& objectClassHandle)
  { rti1516e::ObjectClassHandleFriend::copy(*this, objectClassHandle); }
};
class OPENRTI_LOCAL _I1516EInteractionClassHandle : public OpenRTI::InteractionClassHandle {
public:
  _I1516EInteractionClassHandle(const rti1516e::InteractionClassHandle& interactionClassHandle)
  { rti1516e::InteractionClassHandleFriend::copy(*this, interactionClassHandle); }
};
class OPENRTI_LOCAL _I1516EObjectInstanceHandle : public OpenRTI::ObjectInstanceHandle {
public:
  _I1516EObjectInstanceHandle(const rti1516e::ObjectInstanceHandle& objectInstanceHandle)
  { rti1516e::ObjectInstanceHandleFriend::copy(*this, objectInstanceHandle); }
};
class OPENRTI_LOCAL _I1516EAttributeHandle : public OpenRTI::AttributeHandle {
public:
  _I1516EAttributeHandle(const rti1516e::AttributeHandle& attributeHandle)
  { rti1516e::AttributeHandleFriend::copy(*this, attributeHandle); }
};
class OPENRTI_LOCAL _I1516EParameterHandle : public OpenRTI::ParameterHandle {
public:
  _I1516EParameterHandle(const rti1516e::ParameterHandle& parameterHandle)
  { rti1516e::ParameterHandleFriend::copy(*this, parameterHandle); }
};
class OPENRTI_LOCAL _I1516EDimensionHandle : public OpenRTI::DimensionHandle {
public:
  _I1516EDimensionHandle(const rti1516e::DimensionHandle& dimensionHandle)
  { rti1516e::DimensionHandleFriend::copy(*this, dimensionHandle); }
};
class OPENRTI_LOCAL _I1516ERegionHandle : public OpenRTI::RegionHandle {
public:
  _I1516ERegionHandle(const rti1516e::RegionHandle& regionHandle)
  { rti1516e::RegionHandleFriend::copy(*this, regionHandle); }
};
class OPENRTI_LOCAL _I1516EMessageRetractionHandle : public OpenRTI::MessageRetractionHandle {
public:
  _I1516EMessageRetractionHandle(const rti1516e::MessageRetractionHandle& messageRetractionHandle)
  { rti1516e::MessageRetractionHandleFriend::copy(*this, messageRetractionHandle); }
};
class OPENRTI_LOCAL _I1516EVariableLengthData : public OpenRTI::VariableLengthData {
public:
  _I1516EVariableLengthData(const rti1516e::VariableLengthData& variableLengthData) :
    VariableLengthData(rti1516e::VariableLengthDataFriend::readPointer(variableLengthData))
  { }
};
class OPENRTI_LOCAL _I1516ERangeBounds : public OpenRTI::RangeBounds {
public:
  _I1516ERangeBounds(const rti1516e::RangeBounds& rangeBounds) :
    RangeBounds(rangeBounds.getLowerBound(), rangeBounds.getUpperBound())
  { }
};

class OPENRTI_LOCAL _I1516EFederateHandleVector : public OpenRTI::FederateHandleVector {
public:
  _I1516EFederateHandleVector(const rti1516e::FederateHandleSet& federateHandleSet)
  {
    reserve(federateHandleSet.size() + 1);
    for (rti1516e::FederateHandleSet::const_iterator i = federateHandleSet.begin(); i != federateHandleSet.end(); ++i)
      push_back(OpenRTI::_I1516EFederateHandle(*i));
  }
};
class OPENRTI_LOCAL _I1516EAttributeHandleVector : public OpenRTI::AttributeHandleVector {
public:
  _I1516EAttributeHandleVector(const rti1516e::AttributeHandleSet& attributeHandleSet)
  {
    reserve(attributeHandleSet.size() + 1);
    for (rti1516e::AttributeHandleSet::const_iterator i = attributeHandleSet.begin(); i != attributeHandleSet.end(); ++i)
      push_back(OpenRTI::_I1516EAttributeHandle(*i));
  }
};
class OPENRTI_LOCAL _I1516EDimensionHandleSet : public OpenRTI::DimensionHandleSet {
public:
  _I1516EDimensionHandleSet(const rti1516e::DimensionHandleSet& dimensionHandleSet)
  {
    for (rti1516e::DimensionHandleSet::const_iterator i = dimensionHandleSet.begin(); i != dimensionHandleSet.end(); ++i)
      insert(end(), OpenRTI::_I1516EDimensionHandle(*i));
  }
};
class OPENRTI_LOCAL _I1516ERegionHandleSet : public OpenRTI::RegionHandleSet {
public:
  _I1516ERegionHandleSet(const rti1516e::RegionHandleSet& regionHandleSet)
  {
    for (rti1516e::RegionHandleSet::const_iterator i = regionHandleSet.begin(); i != regionHandleSet.end(); ++i)
      insert(end(), OpenRTI::_I1516ERegionHandle(*i));
  }
};

class OPENRTI_LOCAL _I1516EAttributeHandleSetRegionHandleSetPairVector : public OpenRTI::AttributeHandleSetRegionHandleSetPairVector {
public:
  _I1516EAttributeHandleSetRegionHandleSetPairVector(const rti1516e::AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector)
  {
    reserve(attributeHandleSetRegionHandleSetPairVector.size());
    for (rti1516e::AttributeHandleSetRegionHandleSetPairVector::const_iterator i = attributeHandleSetRegionHandleSetPairVector.begin();
         i != attributeHandleSetRegionHandleSetPairVector.end(); ++i) {
      push_back(OpenRTI::AttributeHandleSetRegionHandleSetPair());
      for (rti1516e::AttributeHandleSet::const_iterator j = i->first.begin(); j != i->first.end(); ++j) {
        back().first.insert(back().first.end(), OpenRTI::_I1516EAttributeHandle(*j));
      }
      for (rti1516e::RegionHandleSet::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
        back().second.insert(back().second.end(), OpenRTI::_I1516ERegionHandle(*j));
      }
    }
  }
};

class OPENRTI_LOCAL _I1516EAttributeValueVector : public OpenRTI::AttributeValueVector {
public:
  _I1516EAttributeValueVector(const rti1516e::AttributeHandleValueMap& attributeHandleValueMap)
  {
    reserve(attributeHandleValueMap.size());
    for (rti1516e::AttributeHandleValueMap::const_iterator i = attributeHandleValueMap.begin();
         i != attributeHandleValueMap.end(); ++i) {
      push_back(OpenRTI::AttributeValue());
      back().setAttributeHandle(OpenRTI::_I1516EAttributeHandle(i->first));
      back().setValue(rti1516e::VariableLengthDataFriend::readPointer(i->second));
    }
  }
};
class OPENRTI_LOCAL _I1516EParameterValueVector : public OpenRTI::ParameterValueVector {
public:
  _I1516EParameterValueVector(const rti1516e::ParameterHandleValueMap& parameterHandleValueMap)
  {
    reserve(parameterHandleValueMap.size());
    for (rti1516e::ParameterHandleValueMap::const_iterator i = parameterHandleValueMap.begin();
         i != parameterHandleValueMap.end(); ++i) {
      push_back(OpenRTI::ParameterValue());
      back().setParameterHandle(OpenRTI::_I1516EParameterHandle(i->first));
      back().setValue(rti1516e::VariableLengthDataFriend::readPointer(i->second));
    }
  }
};

class OPENRTI_LOCAL _O1516EFederateHandle : public rti1516e::FederateHandle {
public:
  _O1516EFederateHandle(const OpenRTI::FederateHandle& federateHandle)
  { rti1516e::FederateHandleFriend::copy(*this, federateHandle); }
};
class OPENRTI_LOCAL _O1516EObjectClassHandle : public rti1516e::ObjectClassHandle {
public:
  _O1516EObjectClassHandle(const OpenRTI::ObjectClassHandle& objectClassHandle)
  { rti1516e::ObjectClassHandleFriend::copy(*this, objectClassHandle); }
};
class OPENRTI_LOCAL _O1516EInteractionClassHandle : public rti1516e::InteractionClassHandle {
public:
  _O1516EInteractionClassHandle(const OpenRTI::InteractionClassHandle& interactionClassHandle)
  { rti1516e::InteractionClassHandleFriend::copy(*this, interactionClassHandle); }
};
class OPENRTI_LOCAL _O1516EObjectInstanceHandle : public rti1516e::ObjectInstanceHandle {
public:
  _O1516EObjectInstanceHandle(const OpenRTI::ObjectInstanceHandle& objectInstanceHandle)
  { rti1516e::ObjectInstanceHandleFriend::copy(*this, objectInstanceHandle); }
};
class OPENRTI_LOCAL _O1516EAttributeHandle : public rti1516e::AttributeHandle {
public:
  _O1516EAttributeHandle(const OpenRTI::AttributeHandle& attributeHandle)
  { rti1516e::AttributeHandleFriend::copy(*this, attributeHandle); }
};
class OPENRTI_LOCAL _O1516EParameterHandle : public rti1516e::ParameterHandle {
public:
  _O1516EParameterHandle(const OpenRTI::ParameterHandle& parameterHandle)
  { rti1516e::ParameterHandleFriend::copy(*this, parameterHandle); }
};
class OPENRTI_LOCAL _O1516EDimensionHandle : public rti1516e::DimensionHandle {
public:
  _O1516EDimensionHandle(const OpenRTI::DimensionHandle& dimensionHandle)
  { rti1516e::DimensionHandleFriend::copy(*this, dimensionHandle); }
};
class OPENRTI_LOCAL _O1516ERegionHandle : public rti1516e::RegionHandle {
public:
  _O1516ERegionHandle(const OpenRTI::RegionHandle& regionHandle)
  { rti1516e::RegionHandleFriend::copy(*this, regionHandle); }
};
class OPENRTI_LOCAL _O1516EMessageRetractionHandle : public rti1516e::MessageRetractionHandle {
public:
  _O1516EMessageRetractionHandle(const OpenRTI::MessageRetractionHandle& messageRetractionHandle)
  { rti1516e::MessageRetractionHandleFriend::copy(*this, messageRetractionHandle); }
};
class OPENRTI_LOCAL _O1516EVariableLengthData : public rti1516e::VariableLengthData {
public:
  _O1516EVariableLengthData(const OpenRTI::VariableLengthData& variableLengthData)
  { rti1516e::VariableLengthDataFriend::writePointer(*this) = variableLengthData; }
};
class OPENRTI_LOCAL _O1516ERangeBounds : public rti1516e::RangeBounds {
public:
  _O1516ERangeBounds(const OpenRTI::RangeBounds& rangeBounds) :
    rti1516e::RangeBounds(rangeBounds.getLowerBound(), rangeBounds.getUpperBound())
  { }
};

class OPENRTI_LOCAL _O1516EAttributeHandleSet : public rti1516e::AttributeHandleSet {
public:
  _O1516EAttributeHandleSet(const OpenRTI::AttributeHandleVector& attributeHandleVector)
  {
    for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin();
         i != attributeHandleVector.end(); ++i)
      insert(end(), OpenRTI::_O1516EAttributeHandle(*i));
  }
};
class OPENRTI_LOCAL _O1516EDimensionHandleSet : public rti1516e::DimensionHandleSet {
public:
  _O1516EDimensionHandleSet(const OpenRTI::DimensionHandleSet& dimensionHandleVector)
  {
    for (OpenRTI::DimensionHandleSet::const_iterator i = dimensionHandleVector.begin();
         i != dimensionHandleVector.end(); ++i)
      insert(end(), OpenRTI::_O1516EDimensionHandle(*i));
  }
};
class OPENRTI_LOCAL _O1516EFederateHandleSet : public rti1516e::FederateHandleSet {
public:
  _O1516EFederateHandleSet(const OpenRTI::FederateHandleVector& federateHandleVector)
  {
    for (OpenRTI::FederateHandleVector::const_iterator i = federateHandleVector.begin();
         i != federateHandleVector.end(); ++i)
      insert(end(), OpenRTI::_O1516EFederateHandle(*i));
  }
};

class OPENRTI_LOCAL _O1516EAttributeHandleValueMap : public rti1516e::AttributeHandleValueMap {
public:
  _O1516EAttributeHandleValueMap(const OpenRTI::Federate::ObjectClass& objectClass,
                                 const OpenRTI::AttributeValueVector& attributeValueVector)
  {
    for (OpenRTI::AttributeValueVector::const_iterator i = attributeValueVector.begin();
         i != attributeValueVector.end(); ++i) {
        if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == Unsubscribed)
          continue;
        rti1516e::VariableLengthData& variableLengthData = (*this)[OpenRTI::_O1516EAttributeHandle(i->getAttributeHandle())];
        rti1516e::VariableLengthDataFriend::writePointer(variableLengthData) = i->getValue();
    }
  }
};
class OPENRTI_LOCAL _O1516EParameterHandleValueMap : public rti1516e::ParameterHandleValueMap {
public:
  _O1516EParameterHandleValueMap(const OpenRTI::Federate::InteractionClass& interactionClass,
                                 const OpenRTI::ParameterValueVector& parameterValueVector)
  {
    for (OpenRTI::ParameterValueVector::const_iterator i = parameterValueVector.begin();
         i != parameterValueVector.end(); ++i) {
        if (!interactionClass.getParameter(i->getParameterHandle()))
          continue;
        rti1516e::VariableLengthData& variableLengthData = (*this)[OpenRTI::_O1516EParameterHandle(i->getParameterHandle())];
        rti1516e::VariableLengthDataFriend::writePointer(variableLengthData) = i->getValue();
    }
  }
};

class OPENRTI_LOCAL RTI1516ETraits {
public:
  // The bindings have different logical times
  typedef rti1516e::LogicalTime NativeLogicalTime;
  typedef rti1516e::LogicalTimeInterval NativeLogicalTimeInterval;
};

class OPENRTI_LOCAL RTIambassadorImplementation::RTI1516EAmbassadorInterface : public OpenRTI::Ambassador<RTI1516ETraits> {
public:
  RTI1516EAmbassadorInterface() :
    Ambassador<RTI1516ETraits>(),
    _federateAmbassador(0)
  { }

  std::auto_ptr<rti1516e::LogicalTimeFactory> getTimeFactory()
  {
    // FIXME ask the time management about that
    OpenRTI::Federate* federate = getFederate();
    if (!federate)
	std::auto_ptr<rti1516e::LogicalTimeFactory>();
    return rti1516e::LogicalTimeFactoryFactory::makeLogicalTimeFactory(utf8ToUcs(federate->getLogicalTimeFactoryName()));
  }

  virtual TimeManagement<RTI1516ETraits>* createTimeManagement(Federate& federate)
  {
    std::string logicalTimeFactoryName = federate.getLogicalTimeFactoryName();
    std::auto_ptr<rti1516e::LogicalTimeFactory> logicalTimeFactory;
    logicalTimeFactory = rti1516e::LogicalTimeFactoryFactory::makeLogicalTimeFactory(utf8ToUcs(logicalTimeFactoryName));
    if (!logicalTimeFactory.get())
      return 0;

    // Get logical time and logical time interval. If they are the well known ones,
    // try to use the optimized implementation using the native time data types directly.
    // An implementation is considered equal if the implementation name is the same and they are assignable in each direction,
    // Also add a flag that forces the to use the opaque factory

    // FIXME: make that again configurable
    bool forceOpaqueTime = false;
    if (!forceOpaqueTime) {
      std::auto_ptr<rti1516e::LogicalTime> logicalTime = logicalTimeFactory->makeInitial();
      std::auto_ptr<rti1516e::LogicalTimeInterval> logicalTimeInterval = logicalTimeFactory->makeZero();
      try {
        rti1516e::HLAinteger64Time time;
        rti1516e::HLAinteger64Interval interval;
        if (time.implementationName() == logicalTime->implementationName() &&
            interval.implementationName() == logicalTimeInterval->implementationName()) {
          time = *logicalTime;
          interval = *logicalTimeInterval;
          *logicalTime = time;
          *logicalTimeInterval = interval;
          if (*logicalTime == time && *logicalTimeInterval == interval) {
            return new TemplateTimeManagement<RTI1516ETraits, RTI1516Einteger64TimeFactory>(RTI1516Einteger64TimeFactory(ucsToUtf8(time.implementationName())));
          }
        }
      } catch (...) {
      }

      try {
        rti1516e::HLAfloat64Time time;
        rti1516e::HLAfloat64Interval interval;
        if (time.implementationName() == logicalTime->implementationName() &&
            interval.implementationName() == logicalTimeInterval->implementationName()) {
          time = *logicalTime;
          interval = *logicalTimeInterval;
          *logicalTime = time;
          *logicalTimeInterval = interval;
          if (*logicalTime == time && *logicalTimeInterval == interval) {
            return new TemplateTimeManagement<RTI1516ETraits, RTI1516Efloat64TimeFactory>(RTI1516Efloat64TimeFactory(ucsToUtf8(time.implementationName())));
          }
        }
      } catch (...) {
      }
    }

    // Ok, we will just need to use the opaque logical time factory
    return new TemplateTimeManagement<RTI1516ETraits, RTI1516ELogicalTimeFactory>(RTI1516ELogicalTimeFactory(logicalTimeFactory));
  }

  virtual void connectionLost(const std::string& faultDescription)
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->connectionLost(utf8ToUcs(faultDescription));
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void reportFederationExecutions(const OpenRTI::FederationExecutionInformationVector& federationExecutionInformationVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::FederationExecutionInformationVector federationInformations;

      federationInformations.reserve(federationExecutionInformationVector.size());
      for (OpenRTI::FederationExecutionInformationVector::const_iterator i = federationExecutionInformationVector.begin();
           i != federationExecutionInformationVector.end(); ++i) {
        federationInformations.push_back(rti1516e::FederationExecutionInformation(utf8ToUcs(i->getFederationExecutionName()),
                                                                                  utf8ToUcs(i->getLogicalTimeFactoryName())));
      }

      _federateAmbassador->reportFederationExecutions(federationInformations);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void synchronizationPointRegistrationResponse(const std::string& label, RegisterFederationSynchronizationPointResponseType reason)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      switch (reason) {
      case OpenRTI::RegisterFederationSynchronizationPointResponseSuccess:
        _federateAmbassador->synchronizationPointRegistrationSucceeded(utf8ToUcs(label));
        break;
      case OpenRTI::RegisterFederationSynchronizationPointResponseLabelNotUnique:
        _federateAmbassador->synchronizationPointRegistrationFailed(utf8ToUcs(label), rti1516e::SYNCHRONIZATION_POINT_LABEL_NOT_UNIQUE);
        break;
      case OpenRTI::RegisterFederationSynchronizationPointResponseMemberNotJoined:
        _federateAmbassador->synchronizationPointRegistrationFailed(utf8ToUcs(label), rti1516e::SYNCHRONIZATION_SET_MEMBER_NOT_JOINED);
        break;
      }
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void announceSynchronizationPoint(const std::string& label, const OpenRTI::VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      _federateAmbassador->announceSynchronizationPoint(utf8ToUcs(label), rti1516Tag);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void federationSynchronized(const std::string& label, const OpenRTI::FederateHandleVector& federateHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EFederateHandleSet federateHandleSet(federateHandleVector);
      _federateAmbassador->federationSynchronized(utf8ToUcs(label), federateHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // // 4.12
  // virtual void
  // initiateFederateSave(std::string const& label)
  //   throw (OpenRTI::UnableToPerformSave,
  //          OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(label);
  //   } catch (const rti1516e::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(e.what());
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // initiateFederateSave(const std::string& label,
  //                      const LogicalTime& logicalTime)
  //   throw (OpenRTI::UnableToPerformSave,
  //          OpenRTI::InvalidLogicalTime,
  //          OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(label, RTI1516ELogicalTimeImplementation::getLogicalTime(logicalTime));
  //   } catch (const rti1516e::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(e.what());
  //   } catch (const rti1516e::InvalidLogicalTime& e) {
  //     throw OpenRTI::InvalidLogicalTime(e.what());
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.15
  // virtual void
  // federationSaved()
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationSaved();
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // federationNotSaved(OpenRTI::SaveFailureReason reason)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();

  //   rti1516e::SaveFailureReason rti1516Reason;
  //   switch (reason) {
  //   case OpenRTI::RTI_UNABLE_TO_SAVE:
  //     rti1516Reason = rti1516e::RTI_UNABLE_TO_SAVE;
  //     break;
  //   case OpenRTI::FEDERATE_REPORTED_FAILURE_DURING_SAVE:
  //     rti1516Reason = rti1516e::FEDERATE_REPORTED_FAILURE_DURING_SAVE;
  //     break;
  //   case OpenRTI::FEDERATE_RESIGNED_DURING_SAVE:
  //     rti1516Reason = rti1516e::FEDERATE_RESIGNED_DURING_SAVE;
  //     break;
  //   case OpenRTI::RTI_DETECTED_FAILURE_DURING_SAVE:
  //     rti1516Reason = rti1516e::RTI_DETECTED_FAILURE_DURING_SAVE;
  //     break;
  //   case OpenRTI::SAVE_TIME_CANNOT_BE_HONORED:
  //     rti1516Reason = rti1516e::SAVE_TIME_CANNOT_BE_HONORED;
  //     break;
  //   default:
  //     throw OpenRTI::FederateInternalError();
  //   }

  //   try {
  //     _federateAmbassador->federationNotSaved(rti1516Reason);
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }


  // // 4.17
  // virtual void
  // federationSaveStatusResponse(OpenRTI::FederateHandleSaveStatusPairVector const& federateStatusVector)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //  /* enum SaveStatus */
  //  /* { */
  //  /*    NO_SAVE_IN_PROGRESS, */
  //  /*    FEDERATE_INSTRUCTED_TO_SAVE, */
  //  /*    FEDERATE_SAVING, */
  //  /*    FEDERATE_WAITING_FOR_FEDERATION_TO_SAVE */
  //  /* }; */

  //   try {
  //     throw rti1516e::FederateInternalError(L"Not implemented");
  //     // _federateAmbassador
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.19
  // virtual void
  // requestFederationRestoreSucceeded(std::string const& label)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreSucceeded(label);
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // requestFederationRestoreFailed(std::string const& label)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreFailed(label);
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }


  // // 4.20
  // virtual void
  // federationRestoreBegun()
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationRestoreBegun();
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.21
  // virtual void
  // initiateFederateRestore(std::string const & label,
  //                         FederateHandle handle)
  //   throw (OpenRTI::SpecifiedSaveLabelDoesNotExist,
  //          OpenRTI::CouldNotInitiateRestore,
  //          OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     OpenRTI::_O1516EFederateHandle rti1516Handle(handle);
  //     _federateAmbassador->initiateFederateRestore(label, rti1516Handle);
  //   } catch (const rti1516e::SpecifiedSaveLabelDoesNotExist& e) {
  //     throw OpenRTI::SpecifiedSaveLabelDoesNotExist(e.what());
  //   } catch (const rti1516e::CouldNotInitiateRestore& e) {
  //     throw OpenRTI::CouldNotInitiateRestore(e.what());
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.23
  // virtual
  // void federationRestored()
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationRestored();
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // federationNotRestored(RestoreFailureReason reason)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();

  //   rti1516e::RestoreFailureReason rti1516Reason;
  //   switch (reason) {
  //   case OpenRTI::RTI_UNABLE_TO_RESTORE:
  //     rti1516Reason = rti1516e::RTI_UNABLE_TO_RESTORE;
  //     break;
  //   case OpenRTI::FEDERATE_REPORTED_FAILURE_DURING_RESTORE:
  //     rti1516Reason = rti1516e::FEDERATE_REPORTED_FAILURE_DURING_RESTORE;
  //     break;
  //   case OpenRTI::FEDERATE_RESIGNED_DURING_RESTORE:
  //     rti1516Reason = rti1516e::FEDERATE_RESIGNED_DURING_RESTORE;
  //     break;
  //   case OpenRTI::RTI_DETECTED_FAILURE_DURING_RESTORE:
  //     rti1516Reason = rti1516e::RTI_DETECTED_FAILURE_DURING_RESTORE;
  //     break;
  //   default:
  //     throw OpenRTI::FederateInternalError();
  //   }

  //   try {
  //     _federateAmbassador->federationNotRestored(rti1516Reason);
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.25
  // virtual void
  // federationRestoreStatusResponse(FederateHandleRestoreStatusPairVector const& theFederateStatusVector)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();


  //  /* enum RestoreStatus */
  //  /* { */
  //  /*    NO_RESTORE_IN_PROGRESS, */
  //  /*    FEDERATE_RESTORE_REQUEST_PENDING, */
  //  /*    FEDERATE_WAITING_FOR_RESTORE_TO_BEGIN, */
  //  /*    FEDERATE_PREPARED_TO_RESTORE, */
  //  /*    FEDERATE_RESTORING, */
  //  /*    FEDERATE_WAITING_FOR_FEDERATION_TO_RESTORE */
  //  /* }; */


  //   try {
  //     throw rti1516e::FederateInternalError(L"Not implemented");
  //     // _federateAmbassador
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  /////////////////////////////////////
  // Declaration Management Services //
  /////////////////////////////////////

  virtual void registrationForObjectClass(OpenRTI::ObjectClassHandle objectClassHandle, bool start)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectClassHandle rti1516Handle(objectClassHandle);
      if (start)
        _federateAmbassador->startRegistrationForObjectClass(rti1516Handle);
      else
        _federateAmbassador->stopRegistrationForObjectClass(rti1516Handle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void turnInteractionsOn(InteractionClassHandle interactionClassHandle, bool on)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EInteractionClassHandle rti1516Handle(interactionClassHandle);
      if (on)
        _federateAmbassador->turnInteractionsOn(rti1516Handle);
      else
        _federateAmbassador->turnInteractionsOff(rti1516Handle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  ////////////////////////////////
  // Object Management Services //
  ////////////////////////////////

  virtual void objectInstanceNameReservation(const OpenRTI::ReserveObjectInstanceNameResponseMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      if (message.getSuccess())
        _federateAmbassador->objectInstanceNameReservationSucceeded(utf8ToUcs(message.getObjectInstanceHandleNamePair().second));
      else
        _federateAmbassador->objectInstanceNameReservationFailed(utf8ToUcs(message.getObjectInstanceHandleNamePair().second));
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }
  virtual void objectInstanceNameReservation(const OpenRTI::ReserveMultipleObjectInstanceNameResponseMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      std::set<std::wstring> stringSet;
      for (OpenRTI::ObjectInstanceHandleNamePairVector::const_iterator i = message.getObjectInstanceHandleNamePairVector().begin();
           i != message.getObjectInstanceHandleNamePairVector().end(); ++i)
        stringSet.insert(utf8ToUcs(i->second));
      if (message.getSuccess())
        _federateAmbassador->multipleObjectInstanceNameReservationSucceeded(stringSet);
      else
        _federateAmbassador->multipleObjectInstanceNameReservationFailed(stringSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.5
  virtual
  void
  discoverObjectInstance(ObjectInstanceHandle objectInstanceHandle,
                         ObjectClassHandle objectClassHandle,
                         std::string const& name)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EObjectClassHandle rti1516ObjectClassHandle(objectClassHandle);
      _federateAmbassador->discoverObjectInstance(rti1516ObjectInstanceHandle, rti1516ObjectClassHandle, utf8ToUcs(name));
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void reflectAttributeValues(const Federate::ObjectClass& objectClass, const AttributeUpdateMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EAttributeHandleValueMap rti1516AttributeValues(objectClass, message.getAttributeValues());
      if (!rti1516AttributeValues.empty()) {
        OpenRTI::_O1516EVariableLengthData rti1516Tag(message.getTag());
        OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(message.getObjectInstanceHandle());

        rti1516e::TransportationType rti1516TransportationType = translate(message.getTransportationType());

        rti1516e::SupplementalReflectInfo rti1516SupplementalReflectInfo; // FIXME

        _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag, rti1516e::RECEIVE,
                                                    rti1516TransportationType, rti1516SupplementalReflectInfo);
      }
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void reflectAttributeValues(const Federate::ObjectClass& objectClass, bool flushQueueMode,
                                      OrderType orderType, const TimeStampedAttributeUpdateMessage& message,
                                      const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EAttributeHandleValueMap rti1516AttributeValues(objectClass, message.getAttributeValues());
      if (!rti1516AttributeValues.empty()) {
        OpenRTI::_O1516EVariableLengthData rti1516Tag(message.getTag());
        OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(message.getObjectInstanceHandle());
        rti1516e::OrderType rti1516SentOrderType = translate(message.getOrderType());
        rti1516e::OrderType rti1516ReceivedOrderType = translate(orderType);
        rti1516e::TransportationType rti1516TransportationType = translate(message.getTransportationType());

        rti1516e::SupplementalReflectInfo rti1516SupplementalReflectInfo; // FIXME

        if (flushQueueMode) {
          OpenRTI::_O1516EMessageRetractionHandle rti1516MessageRetractionHandle(message.getMessageRetractionHandle());
          _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag, rti1516SentOrderType,
                                                      rti1516TransportationType, logicalTime, rti1516ReceivedOrderType,
                                                      rti1516MessageRetractionHandle, rti1516SupplementalReflectInfo);
        } else {
          _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag, rti1516SentOrderType,
                                                      rti1516TransportationType, logicalTime, rti1516ReceivedOrderType,
                                                      rti1516SupplementalReflectInfo);
        }
      }
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.11
  virtual void removeObjectInstance(const DeleteObjectInstanceMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(message.getObjectInstanceHandle());
      OpenRTI::_O1516EVariableLengthData rti1516Tag(message.getTag());
      rti1516e::SupplementalRemoveInfo rti1516SupplementalRemoveInfo; // FIXME
      _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516e::RECEIVE, rti1516SupplementalRemoveInfo);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void removeObjectInstance(bool flushQueueMode, OrderType orderType,
                                    const TimeStampedDeleteObjectInstanceMessage& message, const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(message.getObjectInstanceHandle());
      OpenRTI::_O1516EVariableLengthData rti1516Tag(message.getTag());
      rti1516e::OrderType rti1516SentOrderType = translate(message.getOrderType());
      rti1516e::OrderType rti1516ReceivedOrderType = translate(orderType);
      rti1516e::SupplementalRemoveInfo rti1516SupplementalRemoveInfo; // FIXME
      if (flushQueueMode) {
        OpenRTI::_O1516EMessageRetractionHandle rti1516MessageRetractionHandle(message.getMessageRetractionHandle());
        _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516SentOrderType,
                                                  logicalTime, rti1516ReceivedOrderType, rti1516MessageRetractionHandle,
                                                  rti1516SupplementalRemoveInfo);
      } else {
        _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516SentOrderType,
                                                  logicalTime, rti1516ReceivedOrderType, rti1516SupplementalRemoveInfo);
      }
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }


  // 6.9
  virtual void
  receiveInteraction(const InteractionClassHandle& interactionClassHandle, const Federate::InteractionClass& interactionClass, const InteractionMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EParameterHandleValueMap rti1516ParameterValues(interactionClass, message.getParameterValues());
      if (!rti1516ParameterValues.empty()) {
        OpenRTI::_O1516EInteractionClassHandle rti1516InteractionClassHandle(interactionClassHandle);
        OpenRTI::_O1516EVariableLengthData rti1516Tag(message.getTag());
        rti1516e::TransportationType rti1516TransportationType;
        rti1516TransportationType = translate(message.getTransportationType());
        rti1516e::SupplementalReceiveInfo rti1516eSupplementalReceiveInfo; // FIXME
        _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516e::RECEIVE,
                                                rti1516TransportationType, rti1516eSupplementalReceiveInfo);
      }
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void
  receiveInteraction(const InteractionClassHandle& interactionClassHandle, const Federate::InteractionClass& interactionClass, bool flushQueueMode,
                     OrderType orderType, const TimeStampedInteractionMessage& message,
                     const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EParameterHandleValueMap rti1516ParameterValues(interactionClass, message.getParameterValues());
      if (!rti1516ParameterValues.empty()) {
        OpenRTI::_O1516EInteractionClassHandle rti1516InteractionClassHandle(interactionClassHandle);
        OpenRTI::_O1516EVariableLengthData rti1516Tag(message.getTag());
        rti1516e::OrderType rti1516SentOrderType = translate(message.getOrderType());
        rti1516e::OrderType rti1516ReceivedOrderType = translate(orderType);
        rti1516e::TransportationType rti1516TransportationType;
        rti1516TransportationType = translate(message.getTransportationType());
        rti1516e::SupplementalReceiveInfo rti1516eSupplementalReceiveInfo; // FIXME

        if (flushQueueMode) {
          OpenRTI::_O1516EMessageRetractionHandle rti1516MessageRetractionHandle(message.getMessageRetractionHandle());
          _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516SentOrderType,
                                                  rti1516TransportationType, logicalTime, rti1516ReceivedOrderType,
                                                  rti1516MessageRetractionHandle, rti1516eSupplementalReceiveInfo);
        } else {
          _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516SentOrderType,
                                                  rti1516TransportationType, logicalTime, rti1516ReceivedOrderType,
                                                  rti1516eSupplementalReceiveInfo);
        }
      }
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.15
  virtual
  void
  attributesInScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->attributesInScope(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.16
  virtual
  void
  attributesOutOfScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->attributesOutOfScope(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.18
  virtual
  void
  provideAttributeValueUpdate(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                              const VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      _federateAmbassador->provideAttributeValueUpdate(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.19
  virtual
  void
  turnUpdatesOnForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->turnUpdatesOnForObjectInstance(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.20
  virtual
  void
  turnUpdatesOffForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->turnUpdatesOffForObjectInstance(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  ///////////////////////////////////
  // Ownership Management Services //
  ///////////////////////////////////

  // 7.4
  virtual
  void
  requestAttributeOwnershipAssumption(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                      const VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      _federateAmbassador->requestAttributeOwnershipAssumption(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.5
  virtual
  void
  requestDivestitureConfirmation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->requestDivestitureConfirmation(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.7
  virtual
  void
  attributeOwnershipAcquisitionNotification(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                            const VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      _federateAmbassador->attributeOwnershipAcquisitionNotification(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.10
  virtual
  void
  attributeOwnershipUnavailable(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->attributeOwnershipUnavailable(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.11
  virtual
  void
  requestAttributeOwnershipRelease(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                   const VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516EVariableLengthData rti1516Tag(tag);
      _federateAmbassador->requestAttributeOwnershipRelease(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.15
  virtual
  void
  confirmAttributeOwnershipAcquisitionCancellation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->confirmAttributeOwnershipAcquisitionCancellation(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.17
  virtual
  void
  informAttributeOwnership(ObjectInstanceHandle objectInstanceHandle,
                           AttributeHandle attributeHandle,
                           FederateHandle federateHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandle rti1516AttributeHandle(attributeHandle);
      OpenRTI::_O1516EFederateHandle rti1516FederateHandle(federateHandle);
      _federateAmbassador->informAttributeOwnership(rti1516ObjectInstanceHandle,
                                                    rti1516AttributeHandle,
                                                    rti1516FederateHandle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual
  void
  attributeIsNotOwned(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandle rti1516AttributeHandle(attributeHandle);
      _federateAmbassador->attributeIsNotOwned(rti1516ObjectInstanceHandle, rti1516AttributeHandle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual
  void
  attributeIsOwnedByRTI(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516EAttributeHandle rti1516AttributeHandle(attributeHandle);
      _federateAmbassador->attributeIsOwnedByRTI(rti1516ObjectInstanceHandle, rti1516AttributeHandle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  //////////////////////////////
  // Time Management Services //
  //////////////////////////////

  // 8.3
  virtual void
  timeRegulationEnabled(const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeRegulationEnabled(logicalTime);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.6
  virtual void
  timeConstrainedEnabled(const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeConstrainedEnabled(logicalTime);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.13
  virtual void
  timeAdvanceGrant(const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeAdvanceGrant(logicalTime);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.22
  virtual
  void
  requestRetraction(MessageRetractionHandle messageRetractionHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516EMessageRetractionHandle rti1516MessageRetractionHandle(messageRetractionHandle);
      _federateAmbassador->requestRetraction(rti1516MessageRetractionHandle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  rti1516e::FederateAmbassador* _federateAmbassador;
};

RTIambassadorImplementation::RTIambassadorImplementation() throw () :
  _ambassadorInterface(new RTI1516EAmbassadorInterface)
{
}

RTIambassadorImplementation::~RTIambassadorImplementation()
{
  delete _ambassadorInterface;
  _ambassadorInterface = 0;
}

void
RTIambassadorImplementation::connect(rti1516e::FederateAmbassador & federateAmbassador, rti1516e::CallbackModel rti1516CallbackModel,
                                     std::wstring const & localSettingsDesignator)
  throw (rti1516e::ConnectionFailed,
         rti1516e::InvalidLocalSettingsDesignator,
         rti1516e::UnsupportedCallbackModel,
         rti1516e::AlreadyConnected,
         rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  if (rti1516CallbackModel != rti1516e::HLA_EVOKED)
    throw rti1516e::UnsupportedCallbackModel(L"Only HLA_EVOKED supported!");

  try {
    URL url = URL::fromUrl(ucsToUtf8(localSettingsDesignator));
    _ambassadorInterface->connect(url, StringStringListMap());
    _ambassadorInterface->_federateAmbassador = &federateAmbassador;
  } catch (const OpenRTI::ConnectionFailed& e) {
    throw rti1516e::ConnectionFailed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLocalSettingsDesignator& e) {
    throw rti1516e::InvalidLocalSettingsDesignator(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AlreadyConnected& e) {
    throw rti1516e::AlreadyConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CallNotAllowedFromWithinCallback& e) {
    throw rti1516e::CallNotAllowedFromWithinCallback(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disconnect()
  throw (rti1516e::FederateIsExecutionMember,
         rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->disconnect();
    _ambassadorInterface->_federateAmbassador = 0;
  } catch (const OpenRTI::FederateIsExecutionMember& e) {
    throw rti1516e::FederateIsExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CallNotAllowedFromWithinCallback& e) {
    throw rti1516e::CallNotAllowedFromWithinCallback(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::createFederationExecution(std::wstring const & federationExecutionName,
                                                       std::wstring const & fomModule,
                                                       std::wstring const & logicalTimeImplementationName)
  throw (rti1516e::CouldNotCreateLogicalTimeFactory,
         rti1516e::InconsistentFDD,
         rti1516e::CouldNotOpenFDD,
         rti1516e::ErrorReadingFDD,
         rti1516e::FederationExecutionAlreadyExists,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  // Make sure we can read the fdd file
  OpenRTI::FOMStringModuleList fomModuleList;
  {
    std::list<std::string> candidates = findHLAstandardMIMCandidates();
    for (std::list<std::string>::const_iterator i = candidates.begin(); i != candidates.end(); ++i) {
      std::ifstream stream(utf8ToLocale(*i).c_str());
      if (!stream.is_open())
        continue;

      try {
        fomModuleList.push_back(OpenRTI::FDD1516EFileReader::read(stream));
        break;
      } catch (const OpenRTI::Exception& e) {
        throw rti1516e::ErrorReadingFDD(OpenRTI::utf8ToUcs(e.what()));
      } catch (...) {
        throw rti1516e::RTIinternalError(L"Unknown error while reading fdd file");
      }
    }
    if (fomModuleList.empty())
      throw rti1516e::RTIinternalError(std::wstring(L"Could not find \"HLAstandardMIM.xml\"."));
  }

  {
    std::ifstream stream(OpenRTI::ucsToLocale(fomModule).c_str());
    if (!stream.is_open())
      throw rti1516e::CouldNotOpenFDD(fomModule);
    try {
      fomModuleList.push_back(OpenRTI::FDD1516EFileReader::read(stream));
    } catch (const OpenRTI::Exception& e) {
      throw rti1516e::ErrorReadingFDD(OpenRTI::utf8ToUcs(e.what()));
    } catch (...) {
      throw rti1516e::RTIinternalError(L"Unknown error while reading fdd file");
    }
  }

  try {
    std::string utf8FederationExecutionName = OpenRTI::ucsToUtf8(federationExecutionName);
    _ambassadorInterface->createFederationExecution(ucsToUtf8(federationExecutionName), fomModuleList, ucsToUtf8(logicalTimeImplementationName));
  } catch (const OpenRTI::FederationExecutionAlreadyExists& e) {
    throw rti1516e::FederationExecutionAlreadyExists(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516e::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::createFederationExecution(std::wstring const & federationExecutionName,
                                                       std::vector<std::wstring> const & fomModules,
                                                       std::wstring const & logicalTimeImplementationName)
  throw (rti1516e::CouldNotCreateLogicalTimeFactory,
         rti1516e::InconsistentFDD,
         rti1516e::CouldNotOpenFDD,
         rti1516e::ErrorReadingFDD,
         rti1516e::FederationExecutionAlreadyExists,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::FOMStringModuleList fomModuleList;
  {
    std::list<std::string> candidates = findHLAstandardMIMCandidates();
    for (std::list<std::string>::const_iterator i = candidates.begin(); i != candidates.end(); ++i) {
      std::ifstream stream(utf8ToLocale(*i).c_str());
      if (!stream.is_open())
        continue;

      try {
        fomModuleList.push_back(OpenRTI::FDD1516EFileReader::read(stream));
        break;
      } catch (const OpenRTI::Exception& e) {
        throw rti1516e::ErrorReadingFDD(OpenRTI::utf8ToUcs(e.what()));
      } catch (...) {
        throw rti1516e::RTIinternalError(L"Unknown error while reading fdd file");
      }
    }
    if (fomModuleList.empty())
      throw rti1516e::RTIinternalError(std::wstring(L"Could not find \"HLAstandardMIM.xml\"."));
  }

  for (std::vector<std::wstring>::const_iterator i = fomModules.begin(); i != fomModules.end(); ++i) {
    std::ifstream stream(OpenRTI::ucsToLocale(*i).c_str());
    if (!stream.is_open())
      throw rti1516e::CouldNotOpenFDD(*i);

    try {
      fomModuleList.push_back(OpenRTI::FDD1516EFileReader::read(stream));
    } catch (const OpenRTI::Exception& e) {
      throw rti1516e::ErrorReadingFDD(OpenRTI::utf8ToUcs(e.what()));
    } catch (...) {
      throw rti1516e::RTIinternalError(L"Unknown error while reading fdd file");
    }
  }

  try {
    _ambassadorInterface->createFederationExecution(ucsToUtf8(federationExecutionName), fomModuleList, ucsToUtf8(logicalTimeImplementationName));
  } catch (const OpenRTI::FederationExecutionAlreadyExists& e) {
    throw rti1516e::FederationExecutionAlreadyExists(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516e::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::createFederationExecutionWithMIM (std::wstring const & federationExecutionName,
                                                        std::vector<std::wstring> const & fomModules,
                                                        std::wstring const & mimModule,
                                                        std::wstring const & logicalTimeImplementationName)
  throw (rti1516e::CouldNotCreateLogicalTimeFactory,
         rti1516e::InconsistentFDD,
         rti1516e::CouldNotOpenFDD,
         rti1516e::ErrorReadingFDD,
         rti1516e::DesignatorIsHLAstandardMIM,
         rti1516e::ErrorReadingMIM,
         rti1516e::CouldNotOpenMIM,
         rti1516e::FederationExecutionAlreadyExists,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::FOMStringModuleList fomModuleList;

  std::ifstream stream(OpenRTI::ucsToLocale(mimModule).c_str());
  if (!stream.is_open())
    throw rti1516e::CouldNotOpenMIM(mimModule);

  try {
    fomModuleList.push_back(OpenRTI::FDD1516EFileReader::read(stream));
  } catch (const OpenRTI::Exception& e) {
    throw rti1516e::ErrorReadingMIM(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown error while reading mim file");
  }

  for (std::vector<std::wstring>::const_iterator i = fomModules.begin(); i != fomModules.end(); ++i) {
    std::ifstream stream(OpenRTI::ucsToLocale(*i).c_str());
    if (!stream.is_open())
      throw rti1516e::CouldNotOpenFDD(*i);

    try {
      fomModuleList.push_back(OpenRTI::FDD1516EFileReader::read(stream));
    } catch (const OpenRTI::Exception& e) {
      throw rti1516e::ErrorReadingFDD(OpenRTI::utf8ToUcs(e.what()));
    } catch (...) {
      throw rti1516e::RTIinternalError(L"Unknown error while reading fdd file");
    }
  }

  try {
    _ambassadorInterface->createFederationExecution(ucsToUtf8(federationExecutionName), fomModuleList, ucsToUtf8(logicalTimeImplementationName));
  } catch (const OpenRTI::FederationExecutionAlreadyExists& e) {
    throw rti1516e::FederationExecutionAlreadyExists(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516e::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::destroyFederationExecution(std::wstring const & federationExecutionName)
  throw (rti1516e::FederatesCurrentlyJoined,
         rti1516e::FederationExecutionDoesNotExist,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->destroyFederationExecution(ucsToUtf8(federationExecutionName));
  } catch (const OpenRTI::FederatesCurrentlyJoined& e) {
    throw rti1516e::FederatesCurrentlyJoined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederationExecutionDoesNotExist& e) {
    throw rti1516e::FederationExecutionDoesNotExist(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::listFederationExecutions()
  throw (rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->listFederationExecutions();
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::FederateHandle
RTIambassadorImplementation::joinFederationExecution(std::wstring const & federateType,
                                                     std::wstring const & federationExecutionName,
                                                     std::vector<std::wstring> const & additionalFomModules)
  throw (rti1516e::CouldNotCreateLogicalTimeFactory,
         rti1516e::FederationExecutionDoesNotExist,
         rti1516e::InconsistentFDD,
         rti1516e::ErrorReadingFDD,
         rti1516e::CouldNotOpenFDD,
         rti1516e::FederateAlreadyExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  OpenRTI::FOMStringModuleList fomModuleList;

  for (std::vector<std::wstring>::const_iterator i = additionalFomModules.begin(); i != additionalFomModules.end(); ++i) {
    std::ifstream stream(OpenRTI::ucsToLocale(*i).c_str());
    if (!stream.is_open())
      throw rti1516e::CouldNotOpenFDD(*i);

    try {
      fomModuleList.push_back(OpenRTI::FDD1516EFileReader::read(stream));
    } catch (const OpenRTI::Exception& e) {
      throw rti1516e::ErrorReadingFDD(OpenRTI::utf8ToUcs(e.what()));
    } catch (...) {
      throw rti1516e::RTIinternalError(L"Unknown error while reading fdd file");
    }
  }

  try {
    return OpenRTI::_O1516EFederateHandle(_ambassadorInterface->joinFederationExecution(std::string(), ucsToUtf8(federateType),
                                                                                        ucsToUtf8(federationExecutionName), fomModuleList));
  } catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516e::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederationExecutionDoesNotExist& e) {
    throw rti1516e::FederationExecutionDoesNotExist(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InconsistentFDD& e) {
    throw rti1516e::InconsistentFDD(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateAlreadyExecutionMember& e) {
    throw rti1516e::FederateAlreadyExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CallNotAllowedFromWithinCallback& e) {
    throw rti1516e::CallNotAllowedFromWithinCallback(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::FederateHandle
RTIambassadorImplementation::joinFederationExecution(std::wstring const & federateName,
                                                     std::wstring const & federateType,
                                                     std::wstring const & federationExecutionName,
                                                     std::vector<std::wstring> const & additionalFomModules)
  throw (rti1516e::CouldNotCreateLogicalTimeFactory,
         rti1516e::FederateNameAlreadyInUse,
         rti1516e::FederationExecutionDoesNotExist,
         rti1516e::InconsistentFDD,
         rti1516e::ErrorReadingFDD,
         rti1516e::CouldNotOpenFDD,
         rti1516e::FederateAlreadyExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  OpenRTI::FOMStringModuleList fomModuleList;

  for (std::vector<std::wstring>::const_iterator i = additionalFomModules.begin(); i != additionalFomModules.end(); ++i) {
    std::ifstream stream(OpenRTI::ucsToLocale(*i).c_str());
    if (!stream.is_open())
      throw rti1516e::CouldNotOpenFDD(*i);

    try {
      fomModuleList.push_back(OpenRTI::FDD1516EFileReader::read(stream));
    } catch (const OpenRTI::Exception& e) {
      throw rti1516e::ErrorReadingFDD(OpenRTI::utf8ToUcs(e.what()));
    } catch (...) {
      throw rti1516e::RTIinternalError(L"Unknown error while reading fdd file");
    }
  }

  try {
    return OpenRTI::_O1516EFederateHandle(_ambassadorInterface->joinFederationExecution(ucsToUtf8(federateName), ucsToUtf8(federateType), ucsToUtf8(federationExecutionName), fomModuleList));
  } catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516e::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederationExecutionDoesNotExist& e) {
    throw rti1516e::FederationExecutionDoesNotExist(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNameAlreadyInUse& e) {
    throw rti1516e::FederateNameAlreadyInUse(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InconsistentFDD& e) {
    throw rti1516e::InconsistentFDD(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateAlreadyExecutionMember& e) {
    throw rti1516e::FederateAlreadyExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CallNotAllowedFromWithinCallback& e) {
    throw rti1516e::CallNotAllowedFromWithinCallback(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::resignFederationExecution(rti1516e::ResignAction rti1516ResignAction)
  throw (rti1516e::OwnershipAcquisitionPending,
         rti1516e::FederateOwnsAttributes,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->resignFederationExecution(translate(rti1516ResignAction));
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516e::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw rti1516e::FederateOwnsAttributes(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::registerFederationSynchronizationPoint(std::wstring const & label,
                                                                    rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    // According to the standard, an empty set also means all federates currently joined.
    OpenRTI::FederateHandleVector federateHandleVector;
    _ambassadorInterface->registerFederationSynchronizationPoint(ucsToUtf8(label), tag, federateHandleVector);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::registerFederationSynchronizationPoint(std::wstring const & label,
                                                                    rti1516e::VariableLengthData const & rti1516Tag,
                                                                    rti1516e::FederateHandleSet const & rti1516FederateHandleSet)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    OpenRTI::_I1516EFederateHandleVector federateHandleVector(rti1516FederateHandleSet);
    _ambassadorInterface->registerFederationSynchronizationPoint(ucsToUtf8(label), tag, federateHandleVector);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::synchronizationPointAchieved(std::wstring const & label, bool successfully)
  throw (rti1516e::SynchronizationPointLabelNotAnnounced,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->synchronizationPointAchieved(ucsToUtf8(label), successfully);
  } catch (const OpenRTI::SynchronizationPointLabelNotAnnounced& e) {
    throw rti1516e::SynchronizationPointLabelNotAnnounced(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestFederationSave(std::wstring const & label)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->requestFederationSave(ucsToUtf8(label));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestFederationSave(const std::wstring& label,
                                                   const rti1516e::LogicalTime& rti1516LogicalTime)
  throw (rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InvalidLogicalTime,
         rti1516e::FederateUnableToUseTime,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->requestFederationSave(ucsToUtf8(label), rti1516LogicalTime);
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516e::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516e::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateUnableToUseTime& e) {
    throw rti1516e::FederateUnableToUseTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateSaveBegun()
  throw (rti1516e::SaveNotInitiated,
         rti1516e::FederateNotExecutionMember,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->federateSaveBegun();
  } catch (const OpenRTI::SaveNotInitiated& e) {
    throw rti1516e::SaveNotInitiated(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateSaveComplete()
  throw (rti1516e::FederateHasNotBegunSave,
         rti1516e::FederateNotExecutionMember,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->federateSaveComplete();
  } catch (const OpenRTI::FederateHasNotBegunSave& e) {
    throw rti1516e::FederateHasNotBegunSave(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateSaveNotComplete()
  throw (rti1516e::FederateHasNotBegunSave,
         rti1516e::FederateNotExecutionMember,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->federateSaveNotComplete();
  } catch (const OpenRTI::FederateHasNotBegunSave& e) {
    throw rti1516e::FederateHasNotBegunSave(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::abortFederationSave()
  throw (rti1516e::SaveNotInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->abortFederationSave();
  } catch (const OpenRTI::SaveNotInProgress& e) {
    throw rti1516e::SaveNotInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryFederationSaveStatus()
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->queryFederationSaveStatus();
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestFederationRestore(std::wstring const & label)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->requestFederationRestore(ucsToUtf8(label));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateRestoreComplete()
  throw (rti1516e::RestoreNotRequested,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->federateRestoreComplete();
  } catch (const OpenRTI::RestoreNotRequested& e) {
    throw rti1516e::RestoreNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateRestoreNotComplete()
  throw (rti1516e::RestoreNotRequested,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->federateRestoreNotComplete();
  } catch (const OpenRTI::RestoreNotRequested& e) {
    throw rti1516e::RestoreNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::abortFederationRestore()
  throw (rti1516e::RestoreNotInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->abortFederationRestore();
  } catch (const OpenRTI::RestoreNotInProgress& e) {
    throw rti1516e::RestoreNotInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryFederationRestoreStatus()
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->queryFederationRestoreStatus();
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::publishObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                          rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->publishObjectClassAttributes(objectClassHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unpublishObjectClass(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::OwnershipAcquisitionPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    _ambassadorInterface->unpublishObjectClass(objectClassHandle);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516e::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unpublishObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                            rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::OwnershipAcquisitionPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->unpublishObjectClassAttributes(objectClassHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516e::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::publishInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->publishInteractionClass(interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unpublishInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->unpublishInteractionClass(interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                            rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                            bool active, std::wstring const & updateRateDesignator)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::InvalidUpdateRateDesignator,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->subscribeObjectClassAttributes(objectClassHandle, attributeHandleVector, active, ucsToUtf8(updateRateDesignator));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidUpdateRateDesignator& e) {
    throw rti1516e::InvalidUpdateRateDesignator(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeObjectClass(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    _ambassadorInterface->unsubscribeObjectClass(objectClassHandle);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                              rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->unsubscribeObjectClassAttributes(objectClassHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                       bool active)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::FederateServiceInvocationsAreBeingReportedViaMOM,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->subscribeInteractionClass(interactionClassHandle, active);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateServiceInvocationsAreBeingReportedViaMOM& e) {
    throw rti1516e::FederateServiceInvocationsAreBeingReportedViaMOM(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->unsubscribeInteractionClass(interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::reserveObjectInstanceName(std::wstring const & objectInstanceName)
  throw (rti1516e::IllegalName,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->reserveObjectInstanceName(ucsToUtf8(objectInstanceName));
  } catch (const OpenRTI::IllegalName& e) {
    throw rti1516e::IllegalName(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::releaseObjectInstanceName(std::wstring const & objectInstanceName)
  throw (rti1516e::ObjectInstanceNameNotReserved,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->releaseObjectInstanceName(ucsToUtf8(objectInstanceName));
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw rti1516e::ObjectInstanceNameNotReserved(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::reserveMultipleObjectInstanceName(std::set<std::wstring> const & objectInstanceName)
  throw (rti1516e::IllegalName,
         rti1516e::NameSetWasEmpty,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    std::set<std::string> utf8ObjectInstanceName;
    for (std::set<std::wstring>::const_iterator i = objectInstanceName.begin(); i != objectInstanceName.end(); ++i)
      utf8ObjectInstanceName.insert(ucsToUtf8(*i));
    _ambassadorInterface->reserveMultipleObjectInstanceName(utf8ObjectInstanceName);
  } catch (const OpenRTI::IllegalName& e) {
    throw rti1516e::IllegalName(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NameSetWasEmpty& e) {
    throw rti1516e::NameSetWasEmpty(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::releaseMultipleObjectInstanceName(std::set<std::wstring> const & objectInstanceNameSet)
  throw (rti1516e::ObjectInstanceNameNotReserved,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    std::set<std::string> utf8ObjectInstanceName;
    for (std::set<std::wstring>::const_iterator i = objectInstanceNameSet.begin(); i != objectInstanceNameSet.end(); ++i)
      utf8ObjectInstanceName.insert(ucsToUtf8(*i));
    _ambassadorInterface->releaseMultipleObjectInstanceName(utf8ObjectInstanceName);
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw rti1516e::ObjectInstanceNameNotReserved(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstance(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::ObjectClassNotPublished,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return OpenRTI::_O1516EObjectInstanceHandle(_ambassadorInterface->registerObjectInstance(objectClassHandle));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516e::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstance(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                    std::wstring const & objectInstanceName)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::ObjectClassNotPublished,
         rti1516e::ObjectInstanceNameNotReserved,
         rti1516e::ObjectInstanceNameInUse,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return OpenRTI::_O1516EObjectInstanceHandle(_ambassadorInterface->registerObjectInstance(objectClassHandle, ucsToUtf8(objectInstanceName), false));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516e::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw rti1516e::ObjectInstanceNameNotReserved(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameInUse& e) {
    throw rti1516e::ObjectInstanceNameInUse(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::updateAttributeValues(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                   const rti1516e::AttributeHandleValueMap& rti1516AttributeHandleValueMap,
                                                   const rti1516e::VariableLengthData& rti1516Tag)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeValueVector attributeValueVector(rti1516AttributeHandleValueMap);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->updateAttributeValues(objectInstanceHandle, attributeValueVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516e::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::MessageRetractionHandle
RTIambassadorImplementation::updateAttributeValues(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                   const rti1516e::AttributeHandleValueMap& rti1516AttributeHandleValueMap,
                                                   const rti1516e::VariableLengthData& rti1516Tag,
                                                   const rti1516e::LogicalTime& rti1516LogicalTime)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::InvalidLogicalTime,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeValueVector attributeValueVector(rti1516AttributeHandleValueMap);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516EMessageRetractionHandle(_ambassadorInterface->updateAttributeValues(objectInstanceHandle, attributeValueVector, tag, rti1516LogicalTime));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516e::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516e::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::sendInteraction(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                             const rti1516e::ParameterHandleValueMap& rti1516ParameterHandleValueMap,
                                             const rti1516e::VariableLengthData& rti1516Tag)
  throw (rti1516e::InteractionClassNotPublished,
         rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionParameterNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->sendInteraction(interactionClassHandle, parameterValueVector, tag);
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516e::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516e::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::MessageRetractionHandle
RTIambassadorImplementation::sendInteraction(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                             const rti1516e::ParameterHandleValueMap& rti1516ParameterHandleValueMap,
                                             const rti1516e::VariableLengthData& rti1516Tag,
                                             const rti1516e::LogicalTime& rti1516LogicalTime)
  throw (rti1516e::InteractionClassNotPublished,
         rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionParameterNotDefined,
         rti1516e::InvalidLogicalTime,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516EMessageRetractionHandle(_ambassadorInterface->sendInteraction(interactionClassHandle, parameterValueVector, tag, rti1516LogicalTime));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516e::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516e::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516e::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::deleteObjectInstance(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                  const rti1516e::VariableLengthData& rti1516Tag)
  throw (rti1516e::DeletePrivilegeNotHeld,
         rti1516e::ObjectInstanceNotKnown,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->deleteObjectInstance(objectInstanceHandle, tag);
  } catch (const OpenRTI::DeletePrivilegeNotHeld& e) {
    throw rti1516e::DeletePrivilegeNotHeld(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::MessageRetractionHandle
RTIambassadorImplementation::deleteObjectInstance(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                  const rti1516e::VariableLengthData& rti1516Tag,
                                                  const rti1516e::LogicalTime& rti1516LogicalTime)
  throw (rti1516e::DeletePrivilegeNotHeld,
         rti1516e::ObjectInstanceNotKnown,
         rti1516e::InvalidLogicalTime,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516EMessageRetractionHandle(_ambassadorInterface->deleteObjectInstance(objectInstanceHandle, tag, rti1516LogicalTime));
  } catch (const OpenRTI::DeletePrivilegeNotHeld& e) {
    throw rti1516e::DeletePrivilegeNotHeld(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516e::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::localDeleteObjectInstance(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::FederateOwnsAttributes,
         rti1516e::OwnershipAcquisitionPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    _ambassadorInterface->localDeleteObjectInstance(objectInstanceHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw rti1516e::FederateOwnsAttributes(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516e::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::changeAttributeTransportationType(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                               rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                               rti1516e::TransportationType rti1516TransportationType)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::TransportationType transportationType = translate(rti1516TransportationType);
    _ambassadorInterface->changeAttributeTransportationType(objectInstanceHandle, attributeHandleVector, transportationType);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516e::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::changeInteractionTransportationType(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                                 rti1516e::TransportationType rti1516TransportationType)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionClassNotPublished,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::TransportationType transportationType = translate(rti1516TransportationType);
    _ambassadorInterface->changeInteractionTransportationType(interactionClassHandle, transportationType);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516e::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestAttributeValueUpdate(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                         rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                         rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->requestAttributeValueUpdate(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestAttributeValueUpdate(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                         rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                         rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->requestAttributeValueUpdate(objectClassHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestAttributeTransportationTypeChange(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                      rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                      rti1516e::TransportationType transportationType)
  throw (rti1516e::AttributeAlreadyBeingChanged,
         rti1516e::AttributeNotOwned,
         rti1516e::AttributeNotDefined,
         rti1516e::ObjectInstanceNotKnown,
         rti1516e::InvalidTransportationType,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->requestAttributeTransportationTypeChange(objectInstanceHandle, attributeHandleVector,
                                                                   translate(transportationType));
  } catch (const OpenRTI::AttributeAlreadyBeingChanged& e) {
    throw rti1516e::AttributeAlreadyBeingChanged(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516e::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidTransportationType& e) {
    throw rti1516e::InvalidTransportationType(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryAttributeTransportationType(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                              rti1516e::AttributeHandle rti1516AttributeHandle)
  throw (rti1516e::AttributeNotDefined,
         rti1516e::ObjectInstanceNotKnown,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    _ambassadorInterface->queryAttributeTransportationType(objectInstanceHandle, attributeHandle);
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestInteractionTransportationTypeChange(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                                        rti1516e::TransportationType transportationType)
  throw (rti1516e::InteractionClassAlreadyBeingChanged,
         rti1516e::InteractionClassNotPublished,
         rti1516e::InteractionClassNotDefined,
         rti1516e::InvalidTransportationType,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->requestInteractionTransportationTypeChange(interactionClassHandle, translate(transportationType));
  } catch (const OpenRTI::InteractionClassAlreadyBeingChanged& e) {
    throw rti1516e::InteractionClassAlreadyBeingChanged(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516e::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidTransportationType& e) {
    throw rti1516e::InvalidTransportationType(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryInteractionTransportationType(rti1516e::FederateHandle rti1516FederateHandle, rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EFederateHandle federateHandle(rti1516FederateHandle);
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->queryInteractionTransportationType(federateHandle, interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unconditionalAttributeOwnershipDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                        rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->unconditionalAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516e::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::negotiatedAttributeOwnershipDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                     rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                     rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::AttributeAlreadyBeingDivested,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->negotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516e::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAlreadyBeingDivested& e) {
    throw rti1516e::AttributeAlreadyBeingDivested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::confirmDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                rti1516e::AttributeHandleSet const& rti1516AttributeHandleSet,
                                                rti1516e::VariableLengthData const& rti1516Tag)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::AttributeDivestitureWasNotRequested,
         rti1516e::NoAcquisitionPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->confirmDivestiture(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516e::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeDivestitureWasNotRequested& e) {
    throw rti1516e::AttributeDivestitureWasNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NoAcquisitionPending& e) {
    throw rti1516e::NoAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::attributeOwnershipAcquisition(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                           rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                           rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::ObjectClassNotPublished,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotPublished,
         rti1516e::FederateOwnsAttributes,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->attributeOwnershipAcquisition(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516e::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516e::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::attributeOwnershipAcquisitionIfAvailable(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                      rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::ObjectClassNotPublished,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotPublished,
         rti1516e::FederateOwnsAttributes,
         rti1516e::AttributeAlreadyBeingAcquired,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->attributeOwnershipAcquisitionIfAvailable(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516e::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516e::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw rti1516e::FederateOwnsAttributes(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAlreadyBeingAcquired& e) {
    throw rti1516e::AttributeAlreadyBeingAcquired(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::attributeOwnershipReleaseDenied(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                             rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::AttributeNotOwned,
         rti1516e::AttributeNotDefined,
         rti1516e::ObjectInstanceNotKnown,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->attributeOwnershipReleaseDenied(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516e::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::attributeOwnershipDivestitureIfWanted(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                   rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                   rti1516e::AttributeHandleSet & rti1516DivestedAttributeSet)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::AttributeHandleVector divestedAttributeHandleVector;
    _ambassadorInterface->attributeOwnershipDivestitureIfWanted(objectInstanceHandle, attributeHandleVector, divestedAttributeHandleVector);
    rti1516DivestedAttributeSet.clear();
    for (OpenRTI::AttributeHandleVector::const_iterator i = divestedAttributeHandleVector.begin(); i != divestedAttributeHandleVector.end(); ++i)
      rti1516DivestedAttributeSet.insert(OpenRTI::_O1516EAttributeHandle(*i));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516e::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::cancelNegotiatedAttributeOwnershipDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                           rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::AttributeDivestitureWasNotRequested,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->cancelNegotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516e::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeDivestitureWasNotRequested& e) {
    throw rti1516e::AttributeDivestitureWasNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::cancelAttributeOwnershipAcquisition(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                 rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeAlreadyOwned,
         rti1516e::AttributeAcquisitionWasNotRequested,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->cancelAttributeOwnershipAcquisition(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAlreadyOwned& e) {
    throw rti1516e::AttributeAlreadyOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAcquisitionWasNotRequested& e) {
    throw rti1516e::AttributeAcquisitionWasNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryAttributeOwnership(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                     rti1516e::AttributeHandle rti1516AttributeHandle)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    _ambassadorInterface->queryAttributeOwnership(objectInstanceHandle, attributeHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::isAttributeOwnedByFederate(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                        rti1516e::AttributeHandle rti1516AttributeHandle)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    return _ambassadorInterface->isAttributeOwnedByFederate(objectInstanceHandle, attributeHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableTimeRegulation(const rti1516e::LogicalTimeInterval& rti1516Lookahead)
  throw (rti1516e::TimeRegulationAlreadyEnabled,
         rti1516e::InvalidLookahead,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->enableTimeRegulation(rti1516Lookahead);
  } catch (const OpenRTI::TimeRegulationAlreadyEnabled& e) {
    throw rti1516e::TimeRegulationAlreadyEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLookahead& e) {
    throw rti1516e::InvalidLookahead(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516e::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516e::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableTimeRegulation()
  throw (rti1516e::TimeRegulationIsNotEnabled,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->disableTimeRegulation();
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516e::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableTimeConstrained()
  throw (rti1516e::TimeConstrainedAlreadyEnabled,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->enableTimeConstrained();
  } catch (const OpenRTI::TimeConstrainedAlreadyEnabled& e) {
    throw rti1516e::TimeConstrainedAlreadyEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516e::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516e::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableTimeConstrained()
  throw (rti1516e::TimeConstrainedIsNotEnabled,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->disableTimeConstrained();
  } catch (const OpenRTI::TimeConstrainedIsNotEnabled& e) {
    throw rti1516e::TimeConstrainedIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::timeAdvanceRequest(const rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::InvalidLogicalTime,
         rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->timeAdvanceRequest(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516e::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516e::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516e::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516e::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516e::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::timeAdvanceRequestAvailable(const rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::InvalidLogicalTime,
         rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->timeAdvanceRequestAvailable(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516e::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516e::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516e::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516e::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516e::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::nextMessageRequest(const rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::InvalidLogicalTime,
         rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->nextMessageRequest(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516e::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516e::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516e::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516e::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516e::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::nextMessageRequestAvailable(const rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::InvalidLogicalTime,
         rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->nextMessageRequestAvailable(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516e::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516e::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516e::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516e::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516e::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::flushQueueRequest(const rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::InvalidLogicalTime,
         rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->flushQueueRequest(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516e::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516e::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516e::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516e::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516e::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableAsynchronousDelivery()
  throw (rti1516e::AsynchronousDeliveryAlreadyEnabled,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->enableAsynchronousDelivery();
  } catch (const OpenRTI::AsynchronousDeliveryAlreadyEnabled& e) {
    throw rti1516e::AsynchronousDeliveryAlreadyEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableAsynchronousDelivery()
  throw (rti1516e::AsynchronousDeliveryAlreadyDisabled,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->disableAsynchronousDelivery();
  } catch (const OpenRTI::AsynchronousDeliveryAlreadyDisabled& e) {
    throw rti1516e::AsynchronousDeliveryAlreadyDisabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::queryGALT(rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return _ambassadorInterface->queryGALT(logicalTime);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryLogicalTime(rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->queryLogicalTime(logicalTime);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::queryLITS(rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return _ambassadorInterface->queryLITS(logicalTime);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::modifyLookahead(const rti1516e::LogicalTimeInterval& lookahead)
  throw (rti1516e::TimeRegulationIsNotEnabled,
         rti1516e::InvalidLookahead,
         rti1516e::InTimeAdvancingState,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->modifyLookahead(lookahead, true/*checkForTimeRegulation*/);
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516e::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLookahead& e) {
    throw rti1516e::InvalidLookahead(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516e::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryLookahead(rti1516e::LogicalTimeInterval& lookahead)
  throw (rti1516e::TimeRegulationIsNotEnabled,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->queryLookahead(lookahead, true/*checkForTimeRegulation*/);
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516e::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::retract(rti1516e::MessageRetractionHandle rti1516MessageRetractionHandle)
  throw (rti1516e::InvalidMessageRetractionHandle,
         rti1516e::TimeRegulationIsNotEnabled,
         rti1516e::MessageCanNoLongerBeRetracted,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EMessageRetractionHandle messageRetractionHandle(rti1516MessageRetractionHandle);
    _ambassadorInterface->retract(messageRetractionHandle);
  } catch (const OpenRTI::InvalidMessageRetractionHandle& e) {
    throw rti1516e::InvalidMessageRetractionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516e::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::MessageCanNoLongerBeRetracted& e) {
    throw rti1516e::MessageCanNoLongerBeRetracted(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::changeAttributeOrderType(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                      rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                      rti1516e::OrderType rti1516OrderType)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::OrderType orderType = translate(rti1516OrderType);
    _ambassadorInterface->changeAttributeOrderType(objectInstanceHandle, attributeHandleVector, orderType);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516e::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::changeInteractionOrderType(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516e::OrderType rti1516OrderType)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionClassNotPublished,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::OrderType orderType = translate(rti1516OrderType);
    _ambassadorInterface->changeInteractionOrderType(interactionClassHandle, orderType);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516e::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::RegionHandle
RTIambassadorImplementation::createRegion(rti1516e::DimensionHandleSet const & rti1516DimensionHandleSet)
  throw (rti1516e::InvalidDimensionHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EDimensionHandleSet dimensionHandleSet(rti1516DimensionHandleSet);
    return OpenRTI::_O1516ERegionHandle(_ambassadorInterface->createRegion(dimensionHandleSet));
  } catch (const OpenRTI::InvalidDimensionHandle& e) {
    throw rti1516e::InvalidDimensionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::commitRegionModifications(rti1516e::RegionHandleSet const & rti1516RegionHandleSet)
  throw (rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516ERegionHandleSet regionHandleSet(rti1516RegionHandleSet);
    _ambassadorInterface->commitRegionModifications(regionHandleSet);
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::deleteRegion(const rti1516e::RegionHandle& rti1516RegionHandle)
  throw (rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::RegionInUseForUpdateOrSubscription,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516ERegionHandle regionHandle(rti1516RegionHandle);
    _ambassadorInterface->deleteRegion(regionHandle);
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionInUseForUpdateOrSubscription& e) {
    throw rti1516e::RegionInUseForUpdateOrSubscription(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstanceWithRegions(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                               rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                               rti1516AttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::ObjectClassNotPublished,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotPublished,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    return OpenRTI::_O1516EObjectInstanceHandle(_ambassadorInterface->registerObjectInstanceWithRegions(objectClassHandle,
                                                                                                        attributeHandleSetRegionHandleSetPairVector));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516e::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516e::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516e::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstanceWithRegions(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                               rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                               rti1516AttributeHandleSetRegionHandleSetPairVector,
                                                               std::wstring const & objectInstanceName)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::ObjectClassNotPublished,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotPublished,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::ObjectInstanceNameNotReserved,
         rti1516e::ObjectInstanceNameInUse,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    return OpenRTI::_O1516EObjectInstanceHandle(_ambassadorInterface->registerObjectInstanceWithRegions(objectClassHandle,
                                                                                                        attributeHandleSetRegionHandleSetPairVector,
                                                                                                        ucsToUtf8(objectInstanceName)));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516e::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516e::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516e::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw rti1516e::ObjectInstanceNameNotReserved(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameInUse& e) {
    throw rti1516e::ObjectInstanceNameInUse(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::associateRegionsForUpdates(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                        rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                        rti1516AttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->associateRegionsForUpdates(objectInstanceHandle, attributeHandleSetRegionHandleSetPairVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516e::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unassociateRegionsForUpdates(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                          rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                          rti1516AttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->unassociateRegionsForUpdates(objectInstanceHandle, attributeHandleSetRegionHandleSetPairVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeObjectClassAttributesWithRegions(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                                       rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                                       rti1516AttributeHandleSetRegionHandleSetPairVector,
                                                                       bool active, std::wstring const & updateRateDesignator)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateNotExecutionMember,
         rti1516e::InvalidUpdateRateDesignator,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->subscribeObjectClassAttributesWithRegions(objectClassHandle,
                                                                    attributeHandleSetRegionHandleSetPairVector,
                                                                    active, ucsToUtf8(updateRateDesignator));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516e::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidUpdateRateDesignator& e) {
    throw rti1516e::InvalidUpdateRateDesignator(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeObjectClassAttributesWithRegions(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                                         rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                                         rti1516AttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->unsubscribeObjectClassAttributesWithRegions(objectClassHandle,
                                                                      attributeHandleSetRegionHandleSetPairVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeInteractionClassWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                                  rti1516e::RegionHandleSet const & rti1516RegionHandleSet,
                                                                  bool active)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateServiceInvocationsAreBeingReportedViaMOM,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516ERegionHandleSet regionHandleSet(rti1516RegionHandleSet);
    _ambassadorInterface->subscribeInteractionClassWithRegions(interactionClassHandle, regionHandleSet, active);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516e::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateServiceInvocationsAreBeingReportedViaMOM& e) {
    throw rti1516e::FederateServiceInvocationsAreBeingReportedViaMOM(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeInteractionClassWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                                    rti1516e::RegionHandleSet const & rti1516RegionHandleSet)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516ERegionHandleSet regionHandleSet(rti1516RegionHandleSet);
    _ambassadorInterface->unsubscribeInteractionClassWithRegions(interactionClassHandle, regionHandleSet);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::sendInteractionWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516e::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                                                        rti1516e::RegionHandleSet const & rti1516RegionHandleSet,
                                                        rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionClassNotPublished,
         rti1516e::InteractionParameterNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516ERegionHandleSet regionHandleSet(rti1516RegionHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->sendInteractionWithRegions(interactionClassHandle, parameterValueVector, regionHandleSet, tag);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516e::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516e::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516e::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::MessageRetractionHandle
RTIambassadorImplementation::sendInteractionWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516e::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                                                        rti1516e::RegionHandleSet const & rti1516RegionHandleSet,
                                                        rti1516e::VariableLengthData const & rti1516Tag,
                                                        rti1516e::LogicalTime const & rti1516LogicalTime)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionClassNotPublished,
         rti1516e::InteractionParameterNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::InvalidLogicalTime,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516ERegionHandleSet regionHandleSet(rti1516RegionHandleSet);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516EMessageRetractionHandle(_ambassadorInterface->sendInteractionWithRegions(interactionClassHandle, parameterValueVector, regionHandleSet, tag, rti1516LogicalTime));
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516e::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516e::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516e::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516e::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516e::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestAttributeValueUpdateWithRegions(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                                    rti1516e::AttributeHandleSetRegionHandleSetPairVector const&
                                                                    rti1516AttributeHandleSetRegionHandleSetPairVector,
                                                                    rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandleSetRegionHandleSetPairVector attributeHandleSetRegionHandleSetPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    OpenRTI::_I1516EVariableLengthData tag(rti1516Tag);
    _ambassadorInterface->requestAttributeValueUpdateWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector, tag);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516e::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516e::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ResignAction
RTIambassadorImplementation::getAutomaticResignDirective()
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return translate(_ambassadorInterface->getAutomaticResignDirective());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::setAutomaticResignDirective(rti1516e::ResignAction resignAction)
  throw (rti1516e::InvalidResignAction,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->setAutomaticResignDirective(translate(resignAction));
  } catch (const OpenRTI::InvalidResignAction& e) {
    throw rti1516e::InvalidResignAction(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::FederateHandle
RTIambassadorImplementation::getFederateHandle(std::wstring const & name)
  throw (rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return OpenRTI::_O1516EFederateHandle(_ambassadorInterface->getFederateHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516e::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getFederateName(rti1516e::FederateHandle rti1516FederateHandle)
  throw (rti1516e::InvalidFederateHandle,
         rti1516e::FederateHandleNotKnown,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EFederateHandle federateHandle(rti1516FederateHandle);
    return utf8ToUcs(_ambassadorInterface->getFederateName(federateHandle));
  } catch (const OpenRTI::InvalidFederateHandle& e) {
    throw rti1516e::InvalidFederateHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateHandleNotKnown& e) {
    throw rti1516e::FederateHandleNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ObjectClassHandle
RTIambassadorImplementation::getObjectClassHandle(std::wstring const & name)
  throw (rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return OpenRTI::_O1516EObjectClassHandle(_ambassadorInterface->getObjectClassHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516e::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getObjectClassName(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516e::InvalidObjectClassHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return utf8ToUcs(_ambassadorInterface->getObjectClassName(objectClassHandle));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516e::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::AttributeHandle
RTIambassadorImplementation::getAttributeHandle(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                std::wstring const & attributeName)
  throw (rti1516e::InvalidObjectClassHandle,
         rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return OpenRTI::_O1516EAttributeHandle(_ambassadorInterface->getAttributeHandle(objectClassHandle, ucsToUtf8(attributeName)));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516e::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516e::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getAttributeName(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                              rti1516e::AttributeHandle rti1516AttributeHandle)
  throw (rti1516e::InvalidObjectClassHandle,
         rti1516e::InvalidAttributeHandle,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    return utf8ToUcs(_ambassadorInterface->getAttributeName(objectClassHandle, attributeHandle));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516e::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidAttributeHandle& e) {
    throw rti1516e::InvalidAttributeHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

double
RTIambassadorImplementation::getUpdateRateValue(std::wstring const & updateRateDesignator)
    throw (rti1516e::InvalidUpdateRateDesignator,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError)
{
  try {
    return _ambassadorInterface->getUpdateRateValue(ucsToUtf8(updateRateDesignator));
  } catch (const OpenRTI::InvalidUpdateRateDesignator& e) {
    throw rti1516e::InvalidUpdateRateDesignator(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

double
RTIambassadorImplementation::getUpdateRateValueForAttribute(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                            rti1516e::AttributeHandle rti1516AttributeHandle)
    throw (rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    return _ambassadorInterface->getUpdateRateValueForAttribute(objectInstanceHandle, attributeHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::InteractionClassHandle
RTIambassadorImplementation::getInteractionClassHandle(std::wstring const & name)
  throw (rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return OpenRTI::_O1516EInteractionClassHandle(_ambassadorInterface->getInteractionClassHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516e::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getInteractionClassName(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516e::InvalidInteractionClassHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    return utf8ToUcs(_ambassadorInterface->getInteractionClassName(interactionClassHandle));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516e::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ParameterHandle
RTIambassadorImplementation::getParameterHandle(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                std::wstring const & parameterName)
  throw (rti1516e::InvalidInteractionClassHandle,
         rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    return OpenRTI::_O1516EParameterHandle(_ambassadorInterface->getParameterHandle(interactionClassHandle, ucsToUtf8(parameterName)));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516e::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516e::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getParameterName(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                              rti1516e::ParameterHandle rti1516ParameterHandle)
  throw (rti1516e::InvalidInteractionClassHandle,
         rti1516e::InvalidParameterHandle,
         rti1516e::InteractionParameterNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516EParameterHandle parameterHandle(rti1516ParameterHandle);
    return utf8ToUcs(_ambassadorInterface->getParameterName(interactionClassHandle, parameterHandle));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516e::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidParameterHandle& e) {
    throw rti1516e::InvalidParameterHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516e::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::getObjectInstanceHandle(std::wstring const & name)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return OpenRTI::_O1516EObjectInstanceHandle(_ambassadorInterface->getObjectInstanceHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getObjectInstanceName(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    return utf8ToUcs(_ambassadorInterface->getObjectInstanceName(objectInstanceHandle));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::DimensionHandle
RTIambassadorImplementation::getDimensionHandle(std::wstring const & name)
  throw (rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return OpenRTI::_O1516EDimensionHandle(_ambassadorInterface->getDimensionHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516e::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getDimensionName(rti1516e::DimensionHandle rti1516DimensionHandle)
  throw (rti1516e::InvalidDimensionHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EDimensionHandle dimensionHandle(rti1516DimensionHandle);
    return utf8ToUcs(_ambassadorInterface->getDimensionName(dimensionHandle));
  } catch (const OpenRTI::InvalidDimensionHandle& e) {
    throw rti1516e::InvalidDimensionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

unsigned long
RTIambassadorImplementation::getDimensionUpperBound(rti1516e::DimensionHandle rti1516DimensionHandle)
  throw (rti1516e::InvalidDimensionHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EDimensionHandle dimensionHandle(rti1516DimensionHandle);
    return _ambassadorInterface->getDimensionUpperBound(dimensionHandle);
  } catch (const OpenRTI::InvalidDimensionHandle& e) {
    throw rti1516e::InvalidDimensionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::DimensionHandleSet
RTIambassadorImplementation::getAvailableDimensionsForClassAttribute(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                                     rti1516e::AttributeHandle rti1516AttributeHandle)
  throw (rti1516e::InvalidObjectClassHandle,
         rti1516e::InvalidAttributeHandle,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516EAttributeHandle attributeHandle(rti1516AttributeHandle);
    return OpenRTI::_O1516EDimensionHandleSet(_ambassadorInterface->getAvailableDimensionsForClassAttribute(objectClassHandle, attributeHandle));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516e::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidAttributeHandle& e) {
    throw rti1516e::InvalidAttributeHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516e::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ObjectClassHandle
RTIambassadorImplementation::getKnownObjectClassHandle(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    return OpenRTI::_O1516EObjectClassHandle(_ambassadorInterface->getKnownObjectClassHandle(objectInstanceHandle));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516e::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::DimensionHandleSet
RTIambassadorImplementation::getAvailableDimensionsForInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516e::InvalidInteractionClassHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EInteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    return OpenRTI::_O1516EDimensionHandleSet(_ambassadorInterface->getAvailableDimensionsForInteractionClass(interactionClassHandle));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516e::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::TransportationType
RTIambassadorImplementation::getTransportationType(std::wstring const & transportationName)
  throw (rti1516e::InvalidTransportationName,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return translate(_ambassadorInterface->getTransportationType(ucsToUtf8(transportationName)));
  } catch (const OpenRTI::InvalidTransportationName& e) {
    throw rti1516e::InvalidTransportationName(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getTransportationName(rti1516e::TransportationType transportationType)
  throw (rti1516e::InvalidTransportationType,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return utf8ToUcs(_ambassadorInterface->getTransportationName(translate(transportationType)));
  } catch (const OpenRTI::InvalidTransportationType& e) {
    throw rti1516e::InvalidTransportationType(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::OrderType
RTIambassadorImplementation::getOrderType(std::wstring const & orderName)
  throw (rti1516e::InvalidOrderName,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return translate(_ambassadorInterface->getOrderType(ucsToUtf8(orderName)));
  } catch (const OpenRTI::InvalidOrderName& e) {
    throw rti1516e::InvalidOrderName(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getOrderName(rti1516e::OrderType orderType)
  throw (rti1516e::InvalidOrderType,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return utf8ToUcs(_ambassadorInterface->getOrderName(translate(orderType)));
  } catch (const OpenRTI::InvalidOrderType& e) {
    throw rti1516e::InvalidOrderType(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableObjectClassRelevanceAdvisorySwitch()
  throw (rti1516e::ObjectClassRelevanceAdvisorySwitchIsOn,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->enableObjectClassRelevanceAdvisorySwitch();
  } catch (const OpenRTI::ObjectClassRelevanceAdvisorySwitchIsOn& e) {
    throw rti1516e::ObjectClassRelevanceAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableObjectClassRelevanceAdvisorySwitch()
  throw (rti1516e::ObjectClassRelevanceAdvisorySwitchIsOff,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->disableObjectClassRelevanceAdvisorySwitch();
  } catch (const OpenRTI::ObjectClassRelevanceAdvisorySwitchIsOff& e) {
    throw rti1516e::ObjectClassRelevanceAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableAttributeRelevanceAdvisorySwitch()
  throw (rti1516e::AttributeRelevanceAdvisorySwitchIsOn,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->enableAttributeRelevanceAdvisorySwitch();
  } catch (const OpenRTI::AttributeRelevanceAdvisorySwitchIsOn& e) {
    throw rti1516e::AttributeRelevanceAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableAttributeRelevanceAdvisorySwitch()
  throw (rti1516e::AttributeRelevanceAdvisorySwitchIsOff,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->disableAttributeRelevanceAdvisorySwitch();
  } catch (const OpenRTI::AttributeRelevanceAdvisorySwitchIsOff& e) {
    throw rti1516e::AttributeRelevanceAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableAttributeScopeAdvisorySwitch()
  throw (rti1516e::AttributeScopeAdvisorySwitchIsOn,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->enableAttributeScopeAdvisorySwitch();
  } catch (const OpenRTI::AttributeScopeAdvisorySwitchIsOn& e) {
    throw rti1516e::AttributeScopeAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableAttributeScopeAdvisorySwitch()
  throw (rti1516e::AttributeScopeAdvisorySwitchIsOff,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->disableAttributeScopeAdvisorySwitch();
  } catch (const OpenRTI::AttributeScopeAdvisorySwitchIsOff& e) {
    throw rti1516e::AttributeScopeAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableInteractionRelevanceAdvisorySwitch()
  throw (rti1516e::InteractionRelevanceAdvisorySwitchIsOn,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->enableInteractionRelevanceAdvisorySwitch();
  } catch (const OpenRTI::InteractionRelevanceAdvisorySwitchIsOn& e) {
    throw rti1516e::InteractionRelevanceAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableInteractionRelevanceAdvisorySwitch()
  throw (rti1516e::InteractionRelevanceAdvisorySwitchIsOff,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->disableInteractionRelevanceAdvisorySwitch();
  } catch (const OpenRTI::InteractionRelevanceAdvisorySwitchIsOff& e) {
    throw rti1516e::InteractionRelevanceAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::DimensionHandleSet
RTIambassadorImplementation::getDimensionHandleSet(rti1516e::RegionHandle rti1516RegionHandle)
  throw (rti1516e::InvalidRegion,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516ERegionHandle regionHandle(rti1516RegionHandle);
    return OpenRTI::_O1516EDimensionHandleSet(_ambassadorInterface->getDimensionHandleSet(regionHandle));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::RangeBounds
RTIambassadorImplementation::getRangeBounds(rti1516e::RegionHandle rti1516RegionHandle,
                                            rti1516e::DimensionHandle rti1516DimensionHandle)
  throw (rti1516e::InvalidRegion,
         rti1516e::RegionDoesNotContainSpecifiedDimension,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516ERegionHandle regionHandle(rti1516RegionHandle);
    OpenRTI::_I1516EDimensionHandle dimensionHandle(rti1516DimensionHandle);
    return OpenRTI::_O1516ERangeBounds(_ambassadorInterface->getRangeBounds(regionHandle, dimensionHandle));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionDoesNotContainSpecifiedDimension& e) {
    throw rti1516e::RegionDoesNotContainSpecifiedDimension(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::setRangeBounds(rti1516e::RegionHandle rti1516RegionHandle,
                                            rti1516e::DimensionHandle rti1516DimensionHandle,
                                            rti1516e::RangeBounds const & rti1516RangeBounds)
  throw (rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::RegionDoesNotContainSpecifiedDimension,
         rti1516e::InvalidRangeBound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516ERegionHandle regionHandle(rti1516RegionHandle);
    OpenRTI::_I1516EDimensionHandle dimensionHandle(rti1516DimensionHandle);
    OpenRTI::_I1516ERangeBounds rangeBounds(rti1516RangeBounds);
    _ambassadorInterface->setRangeBounds(regionHandle, dimensionHandle, rangeBounds);
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516e::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516e::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionDoesNotContainSpecifiedDimension& e) {
    throw rti1516e::RegionDoesNotContainSpecifiedDimension(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRangeBound& e) {
    throw rti1516e::InvalidRangeBound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

unsigned long
RTIambassadorImplementation::normalizeFederateHandle(rti1516e::FederateHandle rti1516FederateHandle)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::InvalidFederateHandle,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    OpenRTI::_I1516EFederateHandle federateHandle(rti1516FederateHandle);
    return _ambassadorInterface->normalizeFederateHandle(federateHandle);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidFederateHandle& e) {
    throw rti1516e::InvalidFederateHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

unsigned long
RTIambassadorImplementation::normalizeServiceGroup(rti1516e::ServiceGroup rti1516ServiceGroup)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::InvalidServiceGroup,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return _ambassadorInterface->normalizeServiceGroup(translate(rti1516ServiceGroup));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidServiceGroup& e) {
    throw rti1516e::InvalidServiceGroup(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::evokeCallback(double approximateMinimumTimeInSeconds)
  throw (rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  // FIXME need to handle rti1516e::CallNotAllowedFromWithinCallback
  try {
    return _ambassadorInterface->evokeCallback(approximateMinimumTimeInSeconds, false);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    // must not get here
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                                                    double approximateMaximumTimeInSeconds)
  throw (rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  // FIXME need to handle rti1516e::CallNotAllowedFromWithinCallback
  try {
    return _ambassadorInterface->evokeMultipleCallbacks(approximateMinimumTimeInSeconds,
                                                        approximateMaximumTimeInSeconds,
                                                        false);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    // must not get here
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableCallbacks()
  throw (rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->enableCallbacks(false);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    // must not get here
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableCallbacks()
  throw (rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::RTIinternalError)
{
  try {
    _ambassadorInterface->disableCallbacks(false);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    // must not get here
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516e::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516e::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

std::auto_ptr<rti1516e::LogicalTimeFactory>
RTIambassadorImplementation::getTimeFactory() const
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  if (!_ambassadorInterface->isConnected())
    throw rti1516e::NotConnected(L"");
  if (!_ambassadorInterface->getFederate())
    throw rti1516e::FederateNotExecutionMember(L"");
  try {
    return _ambassadorInterface->getTimeFactory();
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::FederateHandle
RTIambassadorImplementation::decodeFederateHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return rti1516e::FederateHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516e::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ObjectClassHandle
RTIambassadorImplementation::decodeObjectClassHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return rti1516e::ObjectClassHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516e::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::InteractionClassHandle
RTIambassadorImplementation::decodeInteractionClassHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return rti1516e::InteractionClassHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516e::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::decodeObjectInstanceHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return rti1516e::ObjectInstanceHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516e::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::AttributeHandle
RTIambassadorImplementation::decodeAttributeHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return rti1516e::AttributeHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516e::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::ParameterHandle
RTIambassadorImplementation::decodeParameterHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return rti1516e::ParameterHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516e::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::DimensionHandle
RTIambassadorImplementation::decodeDimensionHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return rti1516e::DimensionHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516e::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::MessageRetractionHandle
RTIambassadorImplementation::decodeMessageRetractionHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return rti1516e::MessageRetractionHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516e::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516e::RegionHandle
RTIambassadorImplementation::decodeRegionHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  try {
    return rti1516e::RegionHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516e::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516e::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516e::NotConnected(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516e::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516e::RTIinternalError(L"Unknown internal error!");
  }
}

}
