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

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>

#include "RTIambassadorImplementation.h"

#include <algorithm>
#include <fstream>
#include <memory>

#include <RTI/RTIambassador.h>
#include <RTI/FederateAmbassador.h>
#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/RangeBounds.h>

#include "Ambassador.h"
#include "FDD1516FileReader.h"
#include "LogStream.h"
#include "TemplateTimeManagement.h"

#include "HandleFriend.h"
#include "RTI1516LogicalTimeFactory.h"
#include "RTI1516integer64TimeFactory.h"
#include "RTI1516float64TimeFactory.h"
#include "VariableLengthDataFriend.h"

namespace OpenRTI {

static void loadModule(OpenRTI::FOMStringModuleList& fomModuleList, std::istream& stream, const std::string& encoding)
{
  try {
    fomModuleList.push_back(OpenRTI::FDD1516FileReader::read(stream, encoding));
  } catch (const OpenRTI::Exception& e) {
    throw rti1516::ErrorReadingFDD(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown error while reading fdd file");
  }
}

static void loadModule(OpenRTI::FOMStringModuleList& fomModuleList, const std::wstring& fomModule)
{
  if (fomModule.empty())
    throw rti1516::CouldNotOpenFDD(L"Empty module.");
  std::ifstream stream(OpenRTI::ucsToLocale(fomModule).c_str());
  if (stream.is_open()) {
    loadModule(fomModuleList, stream, std::string());
  } else if (fomModule.compare(0, 8, L"file:///") == 0) {
    loadModule(fomModuleList, fomModule.substr(8));
  } else if (fomModule.compare(0, 16, L"data:text/plain,") == 0) {
    std::stringstream stream(ucsToUtf8(fomModule.substr(16)));
    loadModule(fomModuleList, stream, "UTF-8");
  } else if (fomModule.compare(0, 6, L"data:,") == 0) {
    std::stringstream stream(ucsToUtf8(fomModule.substr(6)));
    loadModule(fomModuleList, stream, "UTF-8");
  } else {
    throw rti1516::CouldNotOpenFDD(fomModule);
  }
}

static URL federationExecutionToUrl(const std::string& federationExecutionName)
{
  URL url;
  if (federationExecutionName.find("://") != std::string::npos)
    url = OpenRTI::URL::fromUrl(federationExecutionName);
  else
    url.setPath(federationExecutionName);
  return url;
}

static rti1516::OrderType translate(OpenRTI::OrderType orderType)
{
  switch (orderType) {
  case OpenRTI::TIMESTAMP:
    return rti1516::TIMESTAMP;
  case OpenRTI::RECEIVE:
  default:
    return rti1516::RECEIVE;
  }
}

static OpenRTI::OrderType translate(rti1516::OrderType orderType)
{
  switch (orderType) {
  case rti1516::TIMESTAMP:
    return OpenRTI::TIMESTAMP;
  case rti1516::RECEIVE:
  default:
    return OpenRTI::RECEIVE;
  }
}

static rti1516::TransportationType translate(OpenRTI::TransportationType transportationType)
{
  switch (transportationType) {
  case OpenRTI::BEST_EFFORT:
    return rti1516::BEST_EFFORT;
  case OpenRTI::RELIABLE:
  default:
    return rti1516::RELIABLE;
  }
}

static OpenRTI::TransportationType translate(rti1516::TransportationType transportationType)
{
  switch (transportationType) {
  case rti1516::BEST_EFFORT:
    return OpenRTI::BEST_EFFORT;
  case rti1516::RELIABLE:
  default:
    return OpenRTI::RELIABLE;
  }
}

class OPENRTI_LOCAL _I1516FederateHandle : public OpenRTI::FederateHandle {
public:
  _I1516FederateHandle(const rti1516::FederateHandle& federateHandle)
  { rti1516::FederateHandleFriend::copy(*this, federateHandle); }
};
class OPENRTI_LOCAL _I1516ObjectClassHandle : public OpenRTI::ObjectClassHandle {
public:
  _I1516ObjectClassHandle(const rti1516::ObjectClassHandle& objectClassHandle)
  { rti1516::ObjectClassHandleFriend::copy(*this, objectClassHandle); }
};
class OPENRTI_LOCAL _I1516InteractionClassHandle : public OpenRTI::InteractionClassHandle {
public:
  _I1516InteractionClassHandle(const rti1516::InteractionClassHandle& interactionClassHandle)
  { rti1516::InteractionClassHandleFriend::copy(*this, interactionClassHandle); }
};
class OPENRTI_LOCAL _I1516ObjectInstanceHandle : public OpenRTI::ObjectInstanceHandle {
public:
  _I1516ObjectInstanceHandle(const rti1516::ObjectInstanceHandle& objectInstanceHandle)
  { rti1516::ObjectInstanceHandleFriend::copy(*this, objectInstanceHandle); }
};
class OPENRTI_LOCAL _I1516AttributeHandle : public OpenRTI::AttributeHandle {
public:
  _I1516AttributeHandle(const rti1516::AttributeHandle& attributeHandle)
  { rti1516::AttributeHandleFriend::copy(*this, attributeHandle); }
};
class OPENRTI_LOCAL _I1516ParameterHandle : public OpenRTI::ParameterHandle {
public:
  _I1516ParameterHandle(const rti1516::ParameterHandle& parameterHandle)
  { rti1516::ParameterHandleFriend::copy(*this, parameterHandle); }
};
class OPENRTI_LOCAL _I1516DimensionHandle : public OpenRTI::DimensionHandle {
public:
  _I1516DimensionHandle(const rti1516::DimensionHandle& dimensionHandle)
  { rti1516::DimensionHandleFriend::copy(*this, dimensionHandle); }
};
class OPENRTI_LOCAL _I1516RegionHandle : public OpenRTI::RegionHandle {
public:
  _I1516RegionHandle(const rti1516::RegionHandle& regionHandle)
  { rti1516::RegionHandleFriend::copy(*this, regionHandle); }
};
class OPENRTI_LOCAL _I1516MessageRetractionHandle : public OpenRTI::MessageRetractionHandle {
public:
  _I1516MessageRetractionHandle(const rti1516::MessageRetractionHandle& messageRetractionHandle)
  { rti1516::MessageRetractionHandleFriend::copy(*this, messageRetractionHandle); }
};
class OPENRTI_LOCAL _I1516VariableLengthData : public OpenRTI::VariableLengthData {
public:
  _I1516VariableLengthData(const rti1516::VariableLengthData& variableLengthData) :
    VariableLengthData(rti1516::VariableLengthDataFriend::readPointer(variableLengthData))
  { }
};
class OPENRTI_LOCAL _I1516RangeBounds : public OpenRTI::RangeBounds {
public:
  _I1516RangeBounds(const rti1516::RangeBounds& rangeBounds) :
    RangeBounds(rangeBounds.getLowerBound(), rangeBounds.getUpperBound())
  { }
};

class OPENRTI_LOCAL _I1516FederateHandleVector : public OpenRTI::FederateHandleVector {
public:
  _I1516FederateHandleVector(const rti1516::FederateHandleSet& federateHandleSet)
  {
    reserve(federateHandleSet.size() + 1);
    for (rti1516::FederateHandleSet::const_iterator i = federateHandleSet.begin(); i != federateHandleSet.end(); ++i)
      push_back(OpenRTI::_I1516FederateHandle(*i));
  }
};
class OPENRTI_LOCAL _I1516AttributeHandleVector : public OpenRTI::AttributeHandleVector {
public:
  _I1516AttributeHandleVector(const rti1516::AttributeHandleSet& attributeHandleSet)
  {
    reserve(attributeHandleSet.size() + 1);
    for (rti1516::AttributeHandleSet::const_iterator i = attributeHandleSet.begin(); i != attributeHandleSet.end(); ++i)
      push_back(OpenRTI::_I1516AttributeHandle(*i));
  }
};
class OPENRTI_LOCAL _I1516DimensionHandleSet : public OpenRTI::DimensionHandleSet {
public:
  _I1516DimensionHandleSet(const rti1516::DimensionHandleSet& dimensionHandleSet)
  {
    for (rti1516::DimensionHandleSet::const_iterator i = dimensionHandleSet.begin(); i != dimensionHandleSet.end(); ++i)
      insert(end(), OpenRTI::_I1516DimensionHandle(*i));
  }
};
class OPENRTI_LOCAL _I1516RegionHandleVector : public OpenRTI::RegionHandleVector {
public:
  _I1516RegionHandleVector(const rti1516::RegionHandleSet& regionHandleSet)
  {
    reserve(regionHandleSet.size());
    for (rti1516::RegionHandleSet::const_iterator i = regionHandleSet.begin(); i != regionHandleSet.end(); ++i)
      push_back(OpenRTI::_I1516RegionHandle(*i));
  }
};

class OPENRTI_LOCAL _I1516AttributeHandleVectorRegionHandleVectorPairVector : public OpenRTI::AttributeHandleVectorRegionHandleVectorPairVector {
public:
  _I1516AttributeHandleVectorRegionHandleVectorPairVector(const rti1516::AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector)
  {
    reserve(attributeHandleSetRegionHandleSetPairVector.size());
    for (rti1516::AttributeHandleSetRegionHandleSetPairVector::const_iterator i = attributeHandleSetRegionHandleSetPairVector.begin();
         i != attributeHandleSetRegionHandleSetPairVector.end(); ++i) {
      push_back(OpenRTI::AttributeHandleVectorRegionHandleVectorPair());
      _I1516AttributeHandleVector(i->first).swap(back().first);
      _I1516RegionHandleVector(i->second).swap(back().second);
    }
  }
};

class OPENRTI_LOCAL _I1516AttributeValueVector : public OpenRTI::AttributeValueVector {
public:
  _I1516AttributeValueVector(const rti1516::AttributeHandleValueMap& attributeHandleValueMap)
  {
    reserve(attributeHandleValueMap.size());
    for (rti1516::AttributeHandleValueMap::const_iterator i = attributeHandleValueMap.begin();
         i != attributeHandleValueMap.end(); ++i) {
      push_back(OpenRTI::AttributeValue());
      back().setAttributeHandle(OpenRTI::_I1516AttributeHandle(i->first));
      back().setValue(rti1516::VariableLengthDataFriend::readPointer(i->second));
    }
  }
};
class OPENRTI_LOCAL _I1516ParameterValueVector : public OpenRTI::ParameterValueVector {
public:
  _I1516ParameterValueVector(const rti1516::ParameterHandleValueMap& parameterHandleValueMap)
  {
    reserve(parameterHandleValueMap.size());
    for (rti1516::ParameterHandleValueMap::const_iterator i = parameterHandleValueMap.begin();
         i != parameterHandleValueMap.end(); ++i) {
      push_back(OpenRTI::ParameterValue());
      back().setParameterHandle(OpenRTI::_I1516ParameterHandle(i->first));
      back().setValue(rti1516::VariableLengthDataFriend::readPointer(i->second));
    }
  }
};

class OPENRTI_LOCAL _O1516FederateHandle : public rti1516::FederateHandle {
public:
  _O1516FederateHandle(const OpenRTI::FederateHandle& federateHandle)
  { rti1516::FederateHandleFriend::copy(*this, federateHandle); }
};
class OPENRTI_LOCAL _O1516ObjectClassHandle : public rti1516::ObjectClassHandle {
public:
  _O1516ObjectClassHandle(const OpenRTI::ObjectClassHandle& objectClassHandle)
  { rti1516::ObjectClassHandleFriend::copy(*this, objectClassHandle); }
};
class OPENRTI_LOCAL _O1516InteractionClassHandle : public rti1516::InteractionClassHandle {
public:
  _O1516InteractionClassHandle(const OpenRTI::InteractionClassHandle& interactionClassHandle)
  { rti1516::InteractionClassHandleFriend::copy(*this, interactionClassHandle); }
};
class OPENRTI_LOCAL _O1516ObjectInstanceHandle : public rti1516::ObjectInstanceHandle {
public:
  _O1516ObjectInstanceHandle(const OpenRTI::ObjectInstanceHandle& objectInstanceHandle)
  { rti1516::ObjectInstanceHandleFriend::copy(*this, objectInstanceHandle); }
};
class OPENRTI_LOCAL _O1516AttributeHandle : public rti1516::AttributeHandle {
public:
  _O1516AttributeHandle(const OpenRTI::AttributeHandle& attributeHandle)
  { rti1516::AttributeHandleFriend::copy(*this, attributeHandle); }
};
class OPENRTI_LOCAL _O1516ParameterHandle : public rti1516::ParameterHandle {
public:
  _O1516ParameterHandle(const OpenRTI::ParameterHandle& parameterHandle)
  { rti1516::ParameterHandleFriend::copy(*this, parameterHandle); }
};
class OPENRTI_LOCAL _O1516DimensionHandle : public rti1516::DimensionHandle {
public:
  _O1516DimensionHandle(const OpenRTI::DimensionHandle& dimensionHandle)
  { rti1516::DimensionHandleFriend::copy(*this, dimensionHandle); }
};
class OPENRTI_LOCAL _O1516RegionHandle : public rti1516::RegionHandle {
public:
  _O1516RegionHandle(const OpenRTI::RegionHandle& regionHandle)
  { rti1516::RegionHandleFriend::copy(*this, regionHandle); }
};
class OPENRTI_LOCAL _O1516MessageRetractionHandle : public rti1516::MessageRetractionHandle {
public:
  _O1516MessageRetractionHandle(const OpenRTI::MessageRetractionHandle& messageRetractionHandle)
  { rti1516::MessageRetractionHandleFriend::copy(*this, messageRetractionHandle); }
};
class OPENRTI_LOCAL _O1516VariableLengthData : public rti1516::VariableLengthData {
public:
  _O1516VariableLengthData(const OpenRTI::VariableLengthData& variableLengthData)
  { rti1516::VariableLengthDataFriend::writePointer(*this) = variableLengthData; }
};
class OPENRTI_LOCAL _O1516RangeBounds : public rti1516::RangeBounds {
public:
  _O1516RangeBounds(const OpenRTI::RangeBounds& rangeBounds) :
    rti1516::RangeBounds(rangeBounds.getLowerBound(), rangeBounds.getUpperBound())
  { }
};
class OPENRTI_LOCAL _O1516String : public std::wstring {
public:
  _O1516String(const std::string& s)
  { utf8ToUcs(s).swap(*this); }
};

class OPENRTI_LOCAL _O1516AttributeHandleSet : public rti1516::AttributeHandleSet {
public:
  _O1516AttributeHandleSet(const OpenRTI::AttributeHandleVector& attributeHandleVector)
  {
    for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin();
         i != attributeHandleVector.end(); ++i)
      insert(end(), OpenRTI::_O1516AttributeHandle(*i));
  }
};
class OPENRTI_LOCAL _O1516DimensionHandleSet : public rti1516::DimensionHandleSet {
public:
  _O1516DimensionHandleSet(const OpenRTI::DimensionHandleSet& dimensionHandleVector)
  {
    for (OpenRTI::DimensionHandleSet::const_iterator i = dimensionHandleVector.begin();
         i != dimensionHandleVector.end(); ++i)
      insert(end(), OpenRTI::_O1516DimensionHandle(*i));
  }
};

class OPENRTI_LOCAL _O1516AttributeHandleValueMap : public rti1516::AttributeHandleValueMap {
public:
  _O1516AttributeHandleValueMap(const OpenRTI::Federate::ObjectClass& objectClass,
                                const OpenRTI::AttributeValueVector& attributeValueVector)
  {
    for (OpenRTI::AttributeValueVector::const_iterator i = attributeValueVector.begin();
         i != attributeValueVector.end(); ++i) {
        if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == Unsubscribed)
          continue;
        rti1516::VariableLengthData& variableLengthData = (*this)[OpenRTI::_O1516AttributeHandle(i->getAttributeHandle())];
        rti1516::VariableLengthDataFriend::writePointer(variableLengthData) = i->getValue();
    }
  }
};
class OPENRTI_LOCAL _O1516ParameterHandleValueMap : public rti1516::ParameterHandleValueMap {
public:
  _O1516ParameterHandleValueMap(const OpenRTI::Federate::InteractionClass& interactionClass,
                                const OpenRTI::ParameterValueVector& parameterValueVector)
  {
    for (OpenRTI::ParameterValueVector::const_iterator i = parameterValueVector.begin();
         i != parameterValueVector.end(); ++i) {
        if (!interactionClass.getParameter(i->getParameterHandle()))
          continue;
        rti1516::VariableLengthData& variableLengthData = (*this)[OpenRTI::_O1516ParameterHandle(i->getParameterHandle())];
        rti1516::VariableLengthDataFriend::writePointer(variableLengthData) = i->getValue();
    }
  }
};

class OPENRTI_LOCAL RTI1516Traits {
public:
  // The bindings have different logical times
  typedef rti1516::LogicalTime NativeLogicalTime;
  typedef rti1516::LogicalTimeInterval NativeLogicalTimeInterval;
};

class OPENRTI_LOCAL RTIambassadorImplementation::RTI1516AmbassadorInterface : public OpenRTI::Ambassador<RTI1516Traits> {
public:
  RTI1516AmbassadorInterface(const std::vector<std::wstring>& args) :
    Ambassador<RTI1516Traits>(),
    _federateAmbassador(0),
    _forceOpaqueLogicalTime(false)
  {
    for (unsigned i = 0; i < args.size(); ++i) {
      if (args[i] == L"force-opaque-time") {
        _forceOpaqueLogicalTime = true;
      } else {
        std::wstring::size_type pos = args[i].find('=');
        if (pos == std::wstring::npos)
          continue;
        if (args[i].size() <= pos + 1)
          continue;
        std::string key = ucsToUtf8(args[i].substr(0, pos));
        std::string value = ucsToUtf8(args[i].substr(pos + 1));
        if (key == "address") {
          URL url = URL::fromProtocolAddress(_defaultUrl.getProtocol(), value);
          _defaultUrl.setHost(url.getHost());
          _defaultUrl.setService(url.getService());
        } else if (key == "host" || key == "hostname") {
          _defaultUrl.setHost(value);
        } else if (key == "protocol") {
          _defaultUrl.setProtocol(value);
        } else if (key == "port" || key == "service") {
          _defaultUrl.setService(value);
        } else if (key == "url") {
          _defaultUrl = URL::fromUrl(value);
        } else {
          _stringStringListMap[key].push_back(value);
        }
      }
    }
  }

  virtual void connectionLost(const std::string& faultDescription)
  {
    throw rti1516::RTIinternalError(utf8ToUcs(faultDescription));
  }

  virtual void reportFederationExecutions(const FederationExecutionInformationVector& theFederationExecutionInformationList)
    RTI_NOEXCEPT
  { }

  virtual void synchronizationPointRegistrationResponse(const std::string& label, RegisterFederationSynchronizationPointResponseType reason)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516String rti1516Label(label);
      switch (reason) {
      case OpenRTI::RegisterFederationSynchronizationPointResponseSuccess:
        _federateAmbassador->synchronizationPointRegistrationSucceeded(rti1516Label);
        break;
      case OpenRTI::RegisterFederationSynchronizationPointResponseLabelNotUnique:
        _federateAmbassador->synchronizationPointRegistrationFailed(rti1516Label, rti1516::SYNCHRONIZATION_POINT_LABEL_NOT_UNIQUE);
        break;
      case OpenRTI::RegisterFederationSynchronizationPointResponseMemberNotJoined:
        _federateAmbassador->synchronizationPointRegistrationFailed(rti1516Label, rti1516::SYNCHRONIZATION_SET_MEMBER_NOT_JOINED);
        break;
      }
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void announceSynchronizationPoint(const std::string& label, const OpenRTI::VariableLengthData& tag)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516String rti1516Label(label);
      OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
      _federateAmbassador->announceSynchronizationPoint(rti1516Label, rti1516Tag);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void federationSynchronized(const std::string& label, const OpenRTI::FederateHandleBoolPairVector&)

    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516String rti1516Label(label);
      _federateAmbassador->federationSynchronized(rti1516Label);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // // 4.12
  // virtual void
  // initiateFederateSave(std::string const& label)
  //   RTI_THROW ((OpenRTI::UnableToPerformSave,
  //          OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(label);
  //   } catch (const rti1516::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(e.what());
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // initiateFederateSave(const std::string& label,
  //                      const LogicalTime& logicalTime)
  //   RTI_THROW ((OpenRTI::UnableToPerformSave,
  //          OpenRTI::InvalidLogicalTime,
  //          OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(label, RTI1516LogicalTimeImplementation::getLogicalTime(logicalTime));
  //   } catch (const rti1516::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(e.what());
  //   } catch (const rti1516::InvalidLogicalTime& e) {
  //     throw OpenRTI::InvalidLogicalTime(e.what());
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.15
  // virtual void
  // federationSaved()
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationSaved();
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // federationNotSaved(OpenRTI::SaveFailureReason reason)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();

  //   rti1516::SaveFailureReason rti1516Reason;
  //   switch (reason) {
  //   case OpenRTI::RTI_UNABLE_TO_SAVE:
  //     rti1516Reason = rti1516::RTI_UNABLE_TO_SAVE;
  //     break;
  //   case OpenRTI::FEDERATE_REPORTED_FAILURE_DURING_SAVE:
  //     rti1516Reason = rti1516::FEDERATE_REPORTED_FAILURE_DURING_SAVE;
  //     break;
  //   case OpenRTI::FEDERATE_RESIGNED_DURING_SAVE:
  //     rti1516Reason = rti1516::FEDERATE_RESIGNED_DURING_SAVE;
  //     break;
  //   case OpenRTI::RTI_DETECTED_FAILURE_DURING_SAVE:
  //     rti1516Reason = rti1516::RTI_DETECTED_FAILURE_DURING_SAVE;
  //     break;
  //   case OpenRTI::SAVE_TIME_CANNOT_BE_HONORED:
  //     rti1516Reason = rti1516::SAVE_TIME_CANNOT_BE_HONORED;
  //     break;
  //   default:
  //     throw OpenRTI::FederateInternalError();
  //   }

  //   try {
  //     _federateAmbassador->federationNotSaved(rti1516Reason);
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }


  // // 4.17
  // virtual void
  // federationSaveStatusResponse(OpenRTI::FederateHandleSaveStatusPairVector const& federateStatusVector)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
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
  //     throw rti1516::FederateInternalError(L"Not implemented");
  //     // _federateAmbassador
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.19
  // virtual void
  // requestFederationRestoreSucceeded(std::string const& label)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreSucceeded(label);
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // requestFederationRestoreFailed(std::string const& label)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreFailed(label);
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }


  // // 4.20
  // virtual void
  // federationRestoreBegun()
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationRestoreBegun();
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.21
  // virtual void
  // initiateFederateRestore(std::string const & label,
  //                         FederateHandle handle)
  //   RTI_THROW ((OpenRTI::SpecifiedSaveLabelDoesNotExist,
  //          OpenRTI::CouldNotInitiateRestore,
  //          OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     rti1516::FederateHandle rti1516Handle = rti1516::FederateHandleFriend::createHandle(handle);
  //     _federateAmbassador->initiateFederateRestore(label, rti1516Handle);
  //   } catch (const rti1516::SpecifiedSaveLabelDoesNotExist& e) {
  //     throw OpenRTI::SpecifiedSaveLabelDoesNotExist(e.what());
  //   } catch (const rti1516::CouldNotInitiateRestore& e) {
  //     throw OpenRTI::CouldNotInitiateRestore(e.what());
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.23
  // virtual
  // void federationRestored()
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationRestored();
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // federationNotRestored(RestoreFailureReason reason)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();

  //   rti1516::RestoreFailureReason rti1516Reason;
  //   switch (reason) {
  //   case OpenRTI::RTI_UNABLE_TO_RESTORE:
  //     rti1516Reason = rti1516::RTI_UNABLE_TO_RESTORE;
  //     break;
  //   case OpenRTI::FEDERATE_REPORTED_FAILURE_DURING_RESTORE:
  //     rti1516Reason = rti1516::FEDERATE_REPORTED_FAILURE_DURING_RESTORE;
  //     break;
  //   case OpenRTI::FEDERATE_RESIGNED_DURING_RESTORE:
  //     rti1516Reason = rti1516::FEDERATE_RESIGNED_DURING_RESTORE;
  //     break;
  //   case OpenRTI::RTI_DETECTED_FAILURE_DURING_RESTORE:
  //     rti1516Reason = rti1516::RTI_DETECTED_FAILURE_DURING_RESTORE;
  //     break;
  //   default:
  //     throw OpenRTI::FederateInternalError();
  //   }

  //   try {
  //     _federateAmbassador->federationNotRestored(rti1516Reason);
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.25
  // virtual void
  // federationRestoreStatusResponse(FederateHandleRestoreStatusPairVector const& theFederateStatusVector)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
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
  //     throw rti1516::FederateInternalError(L"Not implemented");
  //     // _federateAmbassador
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  /////////////////////////////////////
  // Declaration Management Services //
  /////////////////////////////////////

  virtual void registrationForObjectClass(OpenRTI::ObjectClassHandle objectClassHandle, bool start)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      if (start)
        _federateAmbassador->startRegistrationForObjectClass(OpenRTI::_O1516ObjectClassHandle(objectClassHandle));
      else
        _federateAmbassador->stopRegistrationForObjectClass(OpenRTI::_O1516ObjectClassHandle(objectClassHandle));
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void turnInteractionsOn(InteractionClassHandle interactionClassHandle, bool on)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      if (on)
        _federateAmbassador->turnInteractionsOn(OpenRTI::_O1516InteractionClassHandle(interactionClassHandle));
      else
        _federateAmbassador->turnInteractionsOff(OpenRTI::_O1516InteractionClassHandle(interactionClassHandle));
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  ////////////////////////////////
  // Object Management Services //
  ////////////////////////////////

  virtual void objectInstanceNameReservationSucceeded(const std::string& objectInstanceName)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516String rti1516ObjectInstanceName(objectInstanceName);
      _federateAmbassador->objectInstanceNameReservationSucceeded(rti1516ObjectInstanceName);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }
  virtual void objectInstanceNameReservationFailed(const std::string& objectInstanceName)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516String rti1516ObjectInstanceName(objectInstanceName);
      _federateAmbassador->objectInstanceNameReservationFailed(rti1516ObjectInstanceName);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void multipleObjectInstanceNameReservationSucceeded(const std::vector<std::string>&)
    RTI_NOEXCEPT
  {
    Log(FederateAmbassador, Warning) << "Ignoring unexpected multiple name reservation response!" << std::endl;
  }
  virtual void multipleObjectInstanceNameReservationFailed(const std::vector<std::string>&)
    RTI_NOEXCEPT
  {
    Log(FederateAmbassador, Warning) << "Ignoring unexpected multiple name reservation response!" << std::endl;
  }

  // 6.5
  virtual
  void
  discoverObjectInstance(ObjectInstanceHandle objectInstanceHandle,
                         ObjectClassHandle objectClassHandle,
                         std::string const& objectInstanceName)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516ObjectClassHandle rti1516ObjectClassHandle(objectClassHandle);
      OpenRTI::_O1516String rti1516ObjectInstanceName(objectInstanceName);
      _federateAmbassador->discoverObjectInstance(rti1516ObjectInstanceHandle, rti1516ObjectClassHandle, rti1516ObjectInstanceName);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void reflectAttributeValues(const Federate::ObjectClass& objectClass, ObjectInstanceHandle objectInstanceHandle,
                                      const AttributeValueVector& attributeValueVector, const VariableLengthData& tag,
                                      OrderType sentOrder, TransportationType transportationType, FederateHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516AttributeHandleValueMap rti1516AttributeValues(objectClass, attributeValueVector);
      if (!rti1516AttributeValues.empty()) {
        OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
        OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
        rti1516::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516::TransportationType rti1516TransportationType = translate(transportationType);
        _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag,
                                                    rti1516SentOrderType, rti1516TransportationType);
      }
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }
  virtual void reflectAttributeValues(const Federate::ObjectClass& objectClass, ObjectInstanceHandle objectInstanceHandle,
                                      const AttributeValueVector& attributeValueVector, const VariableLengthData& tag,
                                      OrderType sentOrder, TransportationType transportationType,
                                      const NativeLogicalTime& logicalTime, OrderType receivedOrder, FederateHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516AttributeHandleValueMap rti1516AttributeValues(objectClass, attributeValueVector);
      if (!rti1516AttributeValues.empty()) {
        OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
        OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
        rti1516::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516::TransportationType rti1516TransportationType = translate(transportationType);
        rti1516::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
        _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag,
                                                    rti1516SentOrderType, rti1516TransportationType, logicalTime,
                                                    rti1516ReceivedOrderType);
      }
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }
  virtual void reflectAttributeValues(const Federate::ObjectClass& objectClass, ObjectInstanceHandle objectInstanceHandle,
                                      const AttributeValueVector& attributeValueVector, const VariableLengthData& tag,
                                      OrderType sentOrder, TransportationType transportationType,
                                      const NativeLogicalTime& logicalTime, OrderType receivedOrder, FederateHandle,
                                      MessageRetractionHandle messageRetractionHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516AttributeHandleValueMap rti1516AttributeValues(objectClass, attributeValueVector);
      if (!rti1516AttributeValues.empty()) {
        OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
        OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
        rti1516::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516::TransportationType rti1516TransportationType = translate(transportationType);
        rti1516::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
        OpenRTI::_O1516MessageRetractionHandle rti1516MessageRetractionHandle(messageRetractionHandle);
        _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag,
                                                    rti1516SentOrderType, rti1516TransportationType, logicalTime,
                                                    rti1516ReceivedOrderType, rti1516MessageRetractionHandle);
      }
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void removeObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag, OrderType sentOrder, FederateHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
      rti1516::OrderType rti1516SentOrderType = translate(sentOrder);
      _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516SentOrderType);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }
  virtual void removeObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag, OrderType sentOrder,
                                    const NativeLogicalTime& logicalTime, OrderType receivedOrder, FederateHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
      rti1516::OrderType rti1516SentOrderType = translate(sentOrder);
      rti1516::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
      _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516SentOrderType,
                                                  logicalTime, rti1516ReceivedOrderType);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }
  virtual void removeObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag, OrderType sentOrder,
                                    const NativeLogicalTime& logicalTime, OrderType receivedOrder, FederateHandle,
                                    MessageRetractionHandle messageRetractionHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
      rti1516::OrderType rti1516SentOrderType = translate(sentOrder);
      rti1516::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
      OpenRTI::_O1516MessageRetractionHandle rti1516MessageRetractionHandle(messageRetractionHandle);
      _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516SentOrderType,
                                                logicalTime, rti1516ReceivedOrderType, rti1516MessageRetractionHandle);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void receiveInteraction(const Federate::InteractionClass& interactionClass, InteractionClassHandle interactionClassHandle,
                                  const ParameterValueVector& parameterValueVector, const VariableLengthData& tag,
                                  OrderType sentOrder, TransportationType transportationType, FederateHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ParameterHandleValueMap rti1516ParameterValues(interactionClass, parameterValueVector);
      if (!rti1516ParameterValues.empty()) {
        OpenRTI::_O1516InteractionClassHandle rti1516InteractionClassHandle(interactionClassHandle);
        OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
        rti1516::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516::TransportationType rti1516TransportationType = translate(transportationType);
        _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag,
                                                rti1516SentOrderType, rti1516TransportationType);
      }
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }
  virtual void receiveInteraction(const Federate::InteractionClass& interactionClass, InteractionClassHandle interactionClassHandle,
                                  const ParameterValueVector& parameterValueVector, const VariableLengthData& tag,
                                  OrderType sentOrder, TransportationType transportationType, const NativeLogicalTime& logicalTime,
                                  OrderType receivedOrder, FederateHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ParameterHandleValueMap rti1516ParameterValues(interactionClass, parameterValueVector);
      if (!rti1516ParameterValues.empty()) {
        OpenRTI::_O1516InteractionClassHandle rti1516InteractionClassHandle(interactionClassHandle);
        OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
        rti1516::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
        rti1516::TransportationType rti1516TransportationType = translate(transportationType);
        _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516SentOrderType,
                                                rti1516TransportationType, logicalTime, rti1516ReceivedOrderType);
      }
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }
  virtual void receiveInteraction(const Federate::InteractionClass& interactionClass, InteractionClassHandle interactionClassHandle,
                                  const ParameterValueVector& parameterValueVector, const VariableLengthData& tag,
                                  OrderType sentOrder, TransportationType transportationType, const NativeLogicalTime& logicalTime,
                                  OrderType receivedOrder, FederateHandle, MessageRetractionHandle messageRetractionHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ParameterHandleValueMap rti1516ParameterValues(interactionClass, parameterValueVector);
      if (!rti1516ParameterValues.empty()) {
        OpenRTI::_O1516InteractionClassHandle rti1516InteractionClassHandle(interactionClassHandle);
        OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
        rti1516::OrderType rti1516SentOrderType = translate(sentOrder);
        rti1516::OrderType rti1516ReceivedOrderType = translate(receivedOrder);
        rti1516::TransportationType rti1516TransportationType = translate(transportationType);
        OpenRTI::_O1516MessageRetractionHandle rti1516MessageRetractionHandle(messageRetractionHandle);
        _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516SentOrderType,
                                                rti1516TransportationType, logicalTime, rti1516ReceivedOrderType,
                                                rti1516MessageRetractionHandle);
      }
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.15
  virtual
  void
  attributesInScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->attributesInScope(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.16
  virtual
  void
  attributesOutOfScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->attributesOutOfScope(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.18
  virtual
  void
  provideAttributeValueUpdate(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                              const VariableLengthData& tag)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
      _federateAmbassador->provideAttributeValueUpdate(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.19
  virtual
  void
  turnUpdatesOnForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector, const std::string&)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->turnUpdatesOnForObjectInstance(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.20
  virtual
  void
  turnUpdatesOffForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->turnUpdatesOffForObjectInstance(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
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
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
      _federateAmbassador->requestAttributeOwnershipAssumption(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.5
  virtual
  void
  requestDivestitureConfirmation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->requestDivestitureConfirmation(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.7
  virtual
  void
  attributeOwnershipAcquisitionNotification(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                            const VariableLengthData& tag)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
      _federateAmbassador->attributeOwnershipAcquisitionNotification(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.10
  virtual
  void
  attributeOwnershipUnavailable(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->attributeOwnershipUnavailable(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.11
  virtual
  void
  requestAttributeOwnershipRelease(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                   const VariableLengthData& tag)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      OpenRTI::_O1516VariableLengthData rti1516Tag(tag);
      _federateAmbassador->requestAttributeOwnershipRelease(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.15
  virtual
  void
  confirmAttributeOwnershipAcquisitionCancellation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandleSet rti1516AttributeHandleSet(attributeHandleVector);
      _federateAmbassador->confirmAttributeOwnershipAcquisitionCancellation(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.17
  virtual
  void
  informAttributeOwnership(ObjectInstanceHandle objectInstanceHandle,
                           AttributeHandle attributeHandle,
                           FederateHandle federateHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandle rti1516AttributeHandle(attributeHandle);
      OpenRTI::_O1516FederateHandle rti1516FederateHandle(federateHandle);
      _federateAmbassador->informAttributeOwnership(rti1516ObjectInstanceHandle, rti1516AttributeHandle, rti1516FederateHandle);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual
  void
  attributeIsNotOwned(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandle rti1516AttributeHandle(attributeHandle);
      _federateAmbassador->attributeIsNotOwned(rti1516ObjectInstanceHandle, rti1516AttributeHandle);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual
  void
  attributeIsOwnedByRTI(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516ObjectInstanceHandle rti1516ObjectInstanceHandle(objectInstanceHandle);
      OpenRTI::_O1516AttributeHandle rti1516AttributeHandle(attributeHandle);
      _federateAmbassador->attributeIsOwnedByRTI(rti1516ObjectInstanceHandle, rti1516AttributeHandle);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  //////////////////////////////
  // Time Management Services //
  //////////////////////////////

  // 8.3
  virtual void
  timeRegulationEnabled(const rti1516::LogicalTime& logicalTime)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeRegulationEnabled(logicalTime);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.6
  virtual void
  timeConstrainedEnabled(const rti1516::LogicalTime& logicalTime)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeConstrainedEnabled(logicalTime);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.13
  virtual void
  timeAdvanceGrant(const rti1516::LogicalTime& logicalTime)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeAdvanceGrant(logicalTime);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.22
  virtual
  void
  requestRetraction(MessageRetractionHandle messageRetractionHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      OpenRTI::_O1516MessageRetractionHandle rti1516MessageRetractionHandle(messageRetractionHandle);
      _federateAmbassador->requestRetraction(rti1516MessageRetractionHandle);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual TimeManagement<RTI1516Traits>* createTimeManagement(Federate& federate)
  {
    std::string logicalTimeFactoryName = federate.getLogicalTimeFactoryName();
    RTI_UNIQUE_PTR<rti1516::LogicalTimeFactory> logicalTimeFactory;
    logicalTimeFactory = rti1516::LogicalTimeFactoryFactory::makeLogicalTimeFactory(utf8ToUcs(logicalTimeFactoryName));
    if (!logicalTimeFactory.get())
      return 0;

    // Get logical time and logical time interval. If they are the well known ones,
    // try to use the optimized implementation using the native time data types directly.
    // An implementation is considered equal if the implementation name is the same and they are assignable in each direction,
    // Also add a flag that forces the to use the opaque factory

    if (!_forceOpaqueLogicalTime) {
      RTI_UNIQUE_PTR<rti1516::LogicalTime> logicalTime = logicalTimeFactory->makeLogicalTime();
      RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval> logicalTimeInterval = logicalTimeFactory->makeLogicalTimeInterval();
      try {
        HLAinteger64Time time;
        HLAinteger64Interval interval;
        if (time.implementationName() == logicalTime->implementationName() &&
            interval.implementationName() == logicalTimeInterval->implementationName()) {
          time = *logicalTime;
          interval = *logicalTimeInterval;
          *logicalTime = time;
          *logicalTimeInterval = interval;
          if (*logicalTime == time && *logicalTimeInterval == interval) {
            return new TemplateTimeManagement<RTI1516Traits, RTI1516integer64TimeFactory>(RTI1516integer64TimeFactory(ucsToUtf8(time.implementationName())));
          }
        }
      } catch (...) {
      }

      try {
        HLAfloat64Time time;
        HLAfloat64Interval interval;
        if (time.implementationName() == logicalTime->implementationName() &&
            interval.implementationName() == logicalTimeInterval->implementationName()) {
          time = *logicalTime;
          interval = *logicalTimeInterval;
          *logicalTime = time;
          *logicalTimeInterval = interval;
          if (*logicalTime == time && *logicalTimeInterval == interval) {
            return new TemplateTimeManagement<RTI1516Traits, RTI1516float64TimeFactory>(RTI1516float64TimeFactory(ucsToUtf8(time.implementationName())));
          }
        }
      } catch (...) {
      }
    }

    // Ok, we will just need to use the opaque logical time factory
    return new TemplateTimeManagement<RTI1516Traits, RTI1516LogicalTimeFactory>(RTI1516LogicalTimeFactory(OpenRTI_MOVE(logicalTimeFactory)));
  }

  void ensureConnected(const URL& url)
  {
    URL connectUrl = url;
    // For the path component strip the file part which is the bare federation execution name
    if (1 < url.getPath().size())
      connectUrl.setPath(OpenRTI::getBasePart(url.getPath()));

    if (connectUrl.getProtocol().empty())
      connectUrl.setProtocol(_defaultUrl.getProtocol());
    if (connectUrl.getHost().empty())
      connectUrl.setHost(_defaultUrl.getHost());
    if (connectUrl.getService().empty())
      connectUrl.setService(_defaultUrl.getService());
    if (connectUrl.getPath().empty())
      connectUrl.setPath(_defaultUrl.getPath());

    if (!isConnected()) {
      Ambassador<RTI1516Traits>::connect(connectUrl, _stringStringListMap);

      _connectedUrl = connectUrl;;
    } else if (_connectedUrl != connectUrl) {
      throw rti1516::RTIinternalError(L"Connect url does not point to the same connection.");
    }
  }

  // The instantiation time argument list
  rti1516::FederateAmbassador* _federateAmbassador;
  URL _defaultUrl;
  StringStringListMap _stringStringListMap;
  URL _connectedUrl;
  bool _forceOpaqueLogicalTime;
};

RTIambassadorImplementation::RTIambassadorImplementation(const std::vector<std::wstring>& args) RTI_NOEXCEPT :
  _ambassadorInterface(new RTI1516AmbassadorInterface(args))
{
}

RTIambassadorImplementation::~RTIambassadorImplementation()
{
  delete _ambassadorInterface;
  _ambassadorInterface = 0;
}

void
RTIambassadorImplementation::createFederationExecution(std::wstring const & federationExecutionName,
                                                       std::wstring const & fullPathNameToTheFDDfile,
                                                       std::wstring const & logicalTimeImplementationName)
  RTI_THROW ((rti1516::FederationExecutionAlreadyExists,
         rti1516::CouldNotOpenFDD,
         rti1516::ErrorReadingFDD,
         rti1516::CouldNotCreateLogicalTimeFactory,
         rti1516::RTIinternalError))
{
  OpenRTI::FOMStringModuleList fomModuleList;
  loadModule(fomModuleList, fullPathNameToTheFDDfile);

  try {
    OpenRTI::URL url = federationExecutionToUrl(OpenRTI::ucsToUtf8(federationExecutionName));
    _ambassadorInterface->ensureConnected(url);
    _ambassadorInterface->createFederationExecution(getFilePart(url.getPath()), fomModuleList, ucsToUtf8(logicalTimeImplementationName));
  } catch (const OpenRTI::FederationExecutionAlreadyExists& e) {
    throw rti1516::FederationExecutionAlreadyExists(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::destroyFederationExecution(std::wstring const & federationExecutionName)
  RTI_THROW ((rti1516::FederatesCurrentlyJoined,
         rti1516::FederationExecutionDoesNotExist,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::URL url = federationExecutionToUrl(OpenRTI::ucsToUtf8(federationExecutionName));
    _ambassadorInterface->ensureConnected(url);
    _ambassadorInterface->destroyFederationExecution(getFilePart(url.getPath()));
  } catch (const OpenRTI::FederatesCurrentlyJoined& e) {
    throw rti1516::FederatesCurrentlyJoined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederationExecutionDoesNotExist& e) {
    throw rti1516::FederationExecutionDoesNotExist(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::FederateHandle
RTIambassadorImplementation::joinFederationExecution(std::wstring const & federateType,
                                                     std::wstring const & federationExecutionName,
                                                     rti1516::FederateAmbassador & federateAmbassador)
  RTI_THROW ((rti1516::FederateAlreadyExecutionMember,
         rti1516::FederationExecutionDoesNotExist,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::CouldNotCreateLogicalTimeFactory,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::URL url = federationExecutionToUrl(OpenRTI::ucsToUtf8(federationExecutionName));
    _ambassadorInterface->ensureConnected(url);
    _ambassadorInterface->_federateAmbassador = &federateAmbassador;

    FederateHandle federateHandle = _ambassadorInterface->joinFederationExecution(std::string(), ucsToUtf8(federateType),
                                                                                  getFilePart(url.getPath()),
                                                                                  OpenRTI::FOMStringModuleList());
    return OpenRTI::_O1516FederateHandle(federateHandle);
  } catch (const OpenRTI::CouldNotCreateLogicalTimeFactory& e) {
    throw rti1516::CouldNotCreateLogicalTimeFactory(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederationExecutionDoesNotExist& e) {
    throw rti1516::FederationExecutionDoesNotExist(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateAlreadyExecutionMember& e) {
    throw rti1516::FederateAlreadyExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::resignFederationExecution(rti1516::ResignAction rti1516ResignAction)
  RTI_THROW ((rti1516::OwnershipAcquisitionPending,
         rti1516::FederateOwnsAttributes,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::ResignAction resignAction;
    switch (rti1516ResignAction) {
    case rti1516::UNCONDITIONALLY_DIVEST_ATTRIBUTES:
      resignAction = OpenRTI::UNCONDITIONALLY_DIVEST_ATTRIBUTES;
      break;
    case rti1516::DELETE_OBJECTS:
      resignAction = OpenRTI::DELETE_OBJECTS;
      break;
    case rti1516::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS:
      resignAction = OpenRTI::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS;
      break;
    case rti1516::DELETE_OBJECTS_THEN_DIVEST:
      resignAction = OpenRTI::DELETE_OBJECTS_THEN_DIVEST;
      break;
    case rti1516::CANCEL_THEN_DELETE_THEN_DIVEST:
      resignAction = OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST;
      break;
    case rti1516::NO_ACTION:
      resignAction = OpenRTI::NO_ACTION;
      break;
    default:
      resignAction = OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST;
    }

    _ambassadorInterface->_federateAmbassador = 0;
    _ambassadorInterface->resignFederationExecution(resignAction);
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw rti1516::FederateOwnsAttributes(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::registerFederationSynchronizationPoint(std::wstring const & label,
                                                                    rti1516::VariableLengthData const & rti1516Tag)
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    // According to the standard, an empty set also means all federates currently joined.
    OpenRTI::FederateHandleVector federateHandleVector;
    _ambassadorInterface->registerFederationSynchronizationPoint(ucsToUtf8(label), tag, federateHandleVector);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::registerFederationSynchronizationPoint(std::wstring const & label,
                                                                    rti1516::VariableLengthData const & rti1516Tag,
                                                                    rti1516::FederateHandleSet const & rti1516FederateHandleSet)
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    OpenRTI::_I1516FederateHandleVector federateHandleVector(rti1516FederateHandleSet);
    _ambassadorInterface->registerFederationSynchronizationPoint(ucsToUtf8(label), tag, federateHandleVector);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::synchronizationPointAchieved(std::wstring const & label)
  RTI_THROW ((rti1516::SynchronizationPointLabelNotAnnounced,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->synchronizationPointAchieved(ucsToUtf8(label), true/*successfully*/);
  } catch (const OpenRTI::SynchronizationPointLabelNotAnnounced& e) {
    throw rti1516::SynchronizationPointLabelNotAnnounced(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestFederationSave(std::wstring const & label)
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->requestFederationSave(ucsToUtf8(label));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestFederationSave(const std::wstring& label,
                                                   const rti1516::LogicalTime& rti1516LogicalTime)
  RTI_THROW ((rti1516::LogicalTimeAlreadyPassed,
         rti1516::InvalidLogicalTime,
         rti1516::FederateUnableToUseTime,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->requestFederationSave(ucsToUtf8(label), rti1516LogicalTime);
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateUnableToUseTime& e) {
    throw rti1516::FederateUnableToUseTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateSaveBegun()
  RTI_THROW ((rti1516::SaveNotInitiated,
         rti1516::FederateNotExecutionMember,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->federateSaveBegun();
  } catch (const OpenRTI::SaveNotInitiated& e) {
    throw rti1516::SaveNotInitiated(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateSaveComplete()
  RTI_THROW ((rti1516::FederateHasNotBegunSave,
         rti1516::FederateNotExecutionMember,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->federateSaveComplete();
  } catch (const OpenRTI::FederateHasNotBegunSave& e) {
    throw rti1516::FederateHasNotBegunSave(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateSaveNotComplete()
  RTI_THROW ((rti1516::FederateHasNotBegunSave,
         rti1516::FederateNotExecutionMember,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->federateSaveNotComplete();
  } catch (const OpenRTI::FederateHasNotBegunSave& e) {
    throw rti1516::FederateHasNotBegunSave(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryFederationSaveStatus ()
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->queryFederationSaveStatus();
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestFederationRestore(std::wstring const & label)
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->requestFederationRestore(ucsToUtf8(label));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateRestoreComplete()
  RTI_THROW ((rti1516::RestoreNotRequested,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->federateRestoreComplete();
  } catch (const OpenRTI::RestoreNotRequested& e) {
    throw rti1516::RestoreNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::federateRestoreNotComplete()
  RTI_THROW ((rti1516::RestoreNotRequested,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->federateRestoreNotComplete();
  } catch (const OpenRTI::RestoreNotRequested& e) {
    throw rti1516::RestoreNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryFederationRestoreStatus()
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->queryFederationRestoreStatus();
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::publishObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                          rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->publishObjectClassAttributes(objectClassHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unpublishObjectClass(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::OwnershipAcquisitionPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    _ambassadorInterface->unpublishObjectClass(objectClassHandle);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unpublishObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                            rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::OwnershipAcquisitionPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->unpublishObjectClassAttributes(objectClassHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::publishInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
  RTI_THROW ((rti1516::InteractionClassNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->publishInteractionClass(interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unpublishInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
  RTI_THROW ((rti1516::InteractionClassNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->unpublishInteractionClass(interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                            rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                            bool active)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->subscribeObjectClassAttributes(objectClassHandle, attributeHandleVector, active, std::string());
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeObjectClass(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    _ambassadorInterface->unsubscribeObjectClass(objectClassHandle);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                              rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->unsubscribeObjectClassAttributes(objectClassHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                       bool active)
  RTI_THROW ((rti1516::InteractionClassNotDefined,
         rti1516::FederateServiceInvocationsAreBeingReportedViaMOM,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->subscribeInteractionClass(interactionClassHandle, active);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateServiceInvocationsAreBeingReportedViaMOM& e) {
    throw rti1516::FederateServiceInvocationsAreBeingReportedViaMOM(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
  RTI_THROW ((rti1516::InteractionClassNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    _ambassadorInterface->unsubscribeInteractionClass(interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::reserveObjectInstanceName(std::wstring const & objectInstanceName)
  RTI_THROW ((rti1516::IllegalName,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->reserveObjectInstanceName(ucsToUtf8(objectInstanceName));
  } catch (const OpenRTI::IllegalName& e) {
    throw rti1516::IllegalName(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstance(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::ObjectClassNotPublished,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return OpenRTI::_O1516ObjectInstanceHandle(_ambassadorInterface->registerObjectInstance(objectClassHandle));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstance(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                    std::wstring const & objectInstanceName)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::ObjectClassNotPublished,
         rti1516::ObjectInstanceNameNotReserved,
         rti1516::ObjectInstanceNameInUse,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return OpenRTI::_O1516ObjectInstanceHandle(_ambassadorInterface->registerObjectInstance(objectClassHandle, ucsToUtf8(objectInstanceName), false));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw rti1516::ObjectInstanceNameNotReserved(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameInUse& e) {
    throw rti1516::ObjectInstanceNameInUse(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::updateAttributeValues(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                   const rti1516::AttributeHandleValueMap& rti1516AttributeHandleValueMap,
                                                   const rti1516::VariableLengthData& rti1516Tag)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeValueVector attributeValueVector(rti1516AttributeHandleValueMap);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    _ambassadorInterface->updateAttributeValues(objectInstanceHandle, attributeValueVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::MessageRetractionHandle
RTIambassadorImplementation::updateAttributeValues(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                   const rti1516::AttributeHandleValueMap& rti1516AttributeHandleValueMap,
                                                   const rti1516::VariableLengthData& rti1516Tag,
                                                   const rti1516::LogicalTime& rti1516LogicalTime)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::InvalidLogicalTime,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeValueVector attributeValueVector(rti1516AttributeHandleValueMap);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516MessageRetractionHandle(_ambassadorInterface->updateAttributeValues(objectInstanceHandle, attributeValueVector, tag, rti1516LogicalTime));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::sendInteraction(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                             const rti1516::ParameterHandleValueMap& rti1516ParameterHandleValueMap,
                                             const rti1516::VariableLengthData& rti1516Tag)
  RTI_THROW ((rti1516::InteractionClassNotPublished,
         rti1516::InteractionClassNotDefined,
         rti1516::InteractionParameterNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516ParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    _ambassadorInterface->sendInteraction(interactionClassHandle, parameterValueVector, tag);
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::MessageRetractionHandle
RTIambassadorImplementation::sendInteraction(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                             const rti1516::ParameterHandleValueMap& rti1516ParameterHandleValueMap,
                                             const rti1516::VariableLengthData& rti1516Tag,
                                             const rti1516::LogicalTime& rti1516LogicalTime)
  RTI_THROW ((rti1516::InteractionClassNotPublished,
         rti1516::InteractionClassNotDefined,
         rti1516::InteractionParameterNotDefined,
         rti1516::InvalidLogicalTime,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516ParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516MessageRetractionHandle(_ambassadorInterface->sendInteraction(interactionClassHandle, parameterValueVector, tag, rti1516LogicalTime));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::deleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                  const rti1516::VariableLengthData& rti1516Tag)
  RTI_THROW ((rti1516::DeletePrivilegeNotHeld,
         rti1516::ObjectInstanceNotKnown,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    _ambassadorInterface->deleteObjectInstance(objectInstanceHandle, tag);
  } catch (const OpenRTI::DeletePrivilegeNotHeld& e) {
    throw rti1516::DeletePrivilegeNotHeld(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::MessageRetractionHandle
RTIambassadorImplementation::deleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                  const rti1516::VariableLengthData& rti1516Tag,
                                                  const rti1516::LogicalTime& rti1516LogicalTime)
  RTI_THROW ((rti1516::DeletePrivilegeNotHeld,
         rti1516::ObjectInstanceNotKnown,
         rti1516::InvalidLogicalTime,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516MessageRetractionHandle(_ambassadorInterface->deleteObjectInstance(objectInstanceHandle, tag, rti1516LogicalTime));
  } catch (const OpenRTI::DeletePrivilegeNotHeld& e) {
    throw rti1516::DeletePrivilegeNotHeld(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::localDeleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::FederateOwnsAttributes,
         rti1516::OwnershipAcquisitionPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    _ambassadorInterface->localDeleteObjectInstance(objectInstanceHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw rti1516::FederateOwnsAttributes(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw rti1516::OwnershipAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::changeAttributeTransportationType(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                               rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                               rti1516::TransportationType rti1516TransportationType)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::TransportationType transportationType = translate(rti1516TransportationType);
    _ambassadorInterface->changeAttributeTransportationType(objectInstanceHandle, attributeHandleVector, transportationType);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::changeInteractionTransportationType(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                                 rti1516::TransportationType rti1516TransportationType)
  RTI_THROW ((rti1516::InteractionClassNotDefined,
         rti1516::InteractionClassNotPublished,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::TransportationType transportationType = translate(rti1516TransportationType);
    _ambassadorInterface->changeInteractionTransportationType(interactionClassHandle, transportationType);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestAttributeValueUpdate(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                         rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                         rti1516::VariableLengthData const & rti1516Tag)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    _ambassadorInterface->requestAttributeValueUpdate(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestAttributeValueUpdate(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                         rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                         rti1516::VariableLengthData const & rti1516Tag)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    _ambassadorInterface->requestAttributeValueUpdate(objectClassHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unconditionalAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                        rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->unconditionalAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::negotiatedAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                     rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                     rti1516::VariableLengthData const & rti1516Tag)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::AttributeAlreadyBeingDivested,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    _ambassadorInterface->negotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAlreadyBeingDivested& e) {
    throw rti1516::AttributeAlreadyBeingDivested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::confirmDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                rti1516::AttributeHandleSet const& rti1516AttributeHandleSet,
                                                rti1516::VariableLengthData const& rti1516Tag)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::AttributeDivestitureWasNotRequested,
         rti1516::NoAcquisitionPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    _ambassadorInterface->confirmDivestiture(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeDivestitureWasNotRequested& e) {
    throw rti1516::AttributeDivestitureWasNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NoAcquisitionPending& e) {
    throw rti1516::NoAcquisitionPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::attributeOwnershipAcquisition(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                           rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                           rti1516::VariableLengthData const & rti1516Tag)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::ObjectClassNotPublished,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotPublished,
         rti1516::FederateOwnsAttributes,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    _ambassadorInterface->attributeOwnershipAcquisition(objectInstanceHandle, attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::attributeOwnershipAcquisitionIfAvailable(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                      rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::ObjectClassNotPublished,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotPublished,
         rti1516::FederateOwnsAttributes,
         rti1516::AttributeAlreadyBeingAcquired,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->attributeOwnershipAcquisitionIfAvailable(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw rti1516::FederateOwnsAttributes(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAlreadyBeingAcquired& e) {
    throw rti1516::AttributeAlreadyBeingAcquired(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::attributeOwnershipDivestitureIfWanted(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                   rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                   rti1516::AttributeHandleSet & rti1516DivestedAttributeSet)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::AttributeHandleVector divestedAttributeHandleVector;
    _ambassadorInterface->attributeOwnershipDivestitureIfWanted(objectInstanceHandle, attributeHandleVector, divestedAttributeHandleVector);
    rti1516DivestedAttributeSet.clear();
    for (OpenRTI::AttributeHandleVector::const_iterator i = divestedAttributeHandleVector.begin(); i != divestedAttributeHandleVector.end(); ++i)
      rti1516DivestedAttributeSet.insert(OpenRTI::_O1516AttributeHandle(*i));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::cancelNegotiatedAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                           rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::AttributeDivestitureWasNotRequested,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->cancelNegotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeDivestitureWasNotRequested& e) {
    throw rti1516::AttributeDivestitureWasNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::cancelAttributeOwnershipAcquisition(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                 rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeAlreadyOwned,
         rti1516::AttributeAcquisitionWasNotRequested,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    _ambassadorInterface->cancelAttributeOwnershipAcquisition(objectInstanceHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAlreadyOwned& e) {
    throw rti1516::AttributeAlreadyOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeAcquisitionWasNotRequested& e) {
    throw rti1516::AttributeAcquisitionWasNotRequested(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryAttributeOwnership(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                     rti1516::AttributeHandle rti1516AttributeHandle)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandle attributeHandle(rti1516AttributeHandle);
    _ambassadorInterface->queryAttributeOwnership(objectInstanceHandle, attributeHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::isAttributeOwnedByFederate(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                        rti1516::AttributeHandle rti1516AttributeHandle)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandle attributeHandle(rti1516AttributeHandle);
    return _ambassadorInterface->isAttributeOwnedByFederate(objectInstanceHandle, attributeHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableTimeRegulation(const rti1516::LogicalTimeInterval& rti1516Lookahead)
  RTI_THROW ((rti1516::TimeRegulationAlreadyEnabled,
         rti1516::InvalidLookahead,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->enableTimeRegulation(rti1516Lookahead);
  } catch (const OpenRTI::TimeRegulationAlreadyEnabled& e) {
    throw rti1516::TimeRegulationAlreadyEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLookahead& e) {
    throw rti1516::InvalidLookahead(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableTimeRegulation()
  RTI_THROW ((rti1516::TimeRegulationIsNotEnabled,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->disableTimeRegulation();
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableTimeConstrained()
  RTI_THROW ((rti1516::TimeConstrainedAlreadyEnabled,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->enableTimeConstrained();
  } catch (const OpenRTI::TimeConstrainedAlreadyEnabled& e) {
    throw rti1516::TimeConstrainedAlreadyEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableTimeConstrained()
  RTI_THROW ((rti1516::TimeConstrainedIsNotEnabled,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->disableTimeConstrained();
  } catch (const OpenRTI::TimeConstrainedIsNotEnabled& e) {
    throw rti1516::TimeConstrainedIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::timeAdvanceRequest(const rti1516::LogicalTime& logicalTime)
  RTI_THROW ((rti1516::InvalidLogicalTime,
         rti1516::LogicalTimeAlreadyPassed,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->timeAdvanceRequest(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::timeAdvanceRequestAvailable(const rti1516::LogicalTime& logicalTime)
  RTI_THROW ((rti1516::InvalidLogicalTime,
         rti1516::LogicalTimeAlreadyPassed,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->timeAdvanceRequestAvailable(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::nextMessageRequest(const rti1516::LogicalTime& logicalTime)
  RTI_THROW ((rti1516::InvalidLogicalTime,
         rti1516::LogicalTimeAlreadyPassed,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->nextMessageRequest(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::nextMessageRequestAvailable(const rti1516::LogicalTime& logicalTime)
  RTI_THROW ((rti1516::InvalidLogicalTime,
         rti1516::LogicalTimeAlreadyPassed,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->nextMessageRequestAvailable(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::flushQueueRequest(const rti1516::LogicalTime& logicalTime)
  RTI_THROW ((rti1516::InvalidLogicalTime,
         rti1516::LogicalTimeAlreadyPassed,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->flushQueueRequest(logicalTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw rti1516::LogicalTimeAlreadyPassed(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw rti1516::RequestForTimeRegulationPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw rti1516::RequestForTimeConstrainedPending(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableAsynchronousDelivery()
  RTI_THROW ((rti1516::AsynchronousDeliveryAlreadyEnabled,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->enableAsynchronousDelivery();
  } catch (const OpenRTI::AsynchronousDeliveryAlreadyEnabled& e) {
    throw rti1516::AsynchronousDeliveryAlreadyEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableAsynchronousDelivery()
  RTI_THROW ((rti1516::AsynchronousDeliveryAlreadyDisabled,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->disableAsynchronousDelivery();
  } catch (const OpenRTI::AsynchronousDeliveryAlreadyDisabled& e) {
    throw rti1516::AsynchronousDeliveryAlreadyDisabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::queryGALT(rti1516::LogicalTime& logicalTime)
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    return _ambassadorInterface->queryGALT(logicalTime);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryLogicalTime(rti1516::LogicalTime& logicalTime)
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->queryLogicalTime(logicalTime);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::queryLITS(rti1516::LogicalTime& logicalTime)
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    return _ambassadorInterface->queryLITS(logicalTime);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::modifyLookahead(const rti1516::LogicalTimeInterval& lookahead)
  RTI_THROW ((rti1516::TimeRegulationIsNotEnabled,
         rti1516::InvalidLookahead,
         rti1516::InTimeAdvancingState,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->modifyLookahead(lookahead, true/*checkForTimeRegulation*/);
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLookahead& e) {
    throw rti1516::InvalidLookahead(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw rti1516::InTimeAdvancingState(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::queryLookahead(rti1516::LogicalTimeInterval& lookahead)
  RTI_THROW ((rti1516::TimeRegulationIsNotEnabled,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->queryLookahead(lookahead, true/*checkForTimeRegulation*/);
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::retract(rti1516::MessageRetractionHandle rti1516MessageRetractionHandle)
  RTI_THROW ((rti1516::InvalidRetractionHandle,
         rti1516::TimeRegulationIsNotEnabled,
         rti1516::MessageCanNoLongerBeRetracted,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516MessageRetractionHandle messageRetractionHandle(rti1516MessageRetractionHandle);
    _ambassadorInterface->retract(messageRetractionHandle);
  } catch (const OpenRTI::InvalidMessageRetractionHandle& e) {
    throw rti1516::InvalidRetractionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw rti1516::TimeRegulationIsNotEnabled(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::MessageCanNoLongerBeRetracted& e) {
    throw rti1516::MessageCanNoLongerBeRetracted(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::changeAttributeOrderType(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                      rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                      rti1516::OrderType rti1516OrderType)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVector attributeHandleVector(rti1516AttributeHandleSet);
    OpenRTI::OrderType orderType = translate(rti1516OrderType);
    _ambassadorInterface->changeAttributeOrderType(objectInstanceHandle, attributeHandleVector, orderType);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw rti1516::AttributeNotOwned(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::changeInteractionOrderType(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516::OrderType rti1516OrderType)
  RTI_THROW ((rti1516::InteractionClassNotDefined,
         rti1516::InteractionClassNotPublished,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::OrderType orderType = translate(rti1516OrderType);
    _ambassadorInterface->changeInteractionOrderType(interactionClassHandle, orderType);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::RegionHandle
RTIambassadorImplementation::createRegion(rti1516::DimensionHandleSet const & rti1516DimensionHandleSet)
  RTI_THROW ((rti1516::InvalidDimensionHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516DimensionHandleSet dimensionHandleSet(rti1516DimensionHandleSet);
    return OpenRTI::_O1516RegionHandle(_ambassadorInterface->createRegion(dimensionHandleSet));
  } catch (const OpenRTI::InvalidDimensionHandle& e) {
    throw rti1516::InvalidDimensionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::commitRegionModifications(rti1516::RegionHandleSet const & rti1516RegionHandleSet)
  RTI_THROW ((rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516RegionHandleVector regionHandleVector(rti1516RegionHandleSet);
    _ambassadorInterface->commitRegionModifications(regionHandleVector);
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::deleteRegion(rti1516::RegionHandle rti1516RegionHandle)
  RTI_THROW ((rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::RegionInUseForUpdateOrSubscription,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516RegionHandle regionHandle(rti1516RegionHandle);
    _ambassadorInterface->deleteRegion(regionHandle);
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionInUseForUpdateOrSubscription& e) {
    throw rti1516::RegionInUseForUpdateOrSubscription(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstanceWithRegions(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                               rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                               rti1516AttributeHandleSetRegionHandleSetPairVector)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::ObjectClassNotPublished,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotPublished,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    return OpenRTI::_O1516ObjectInstanceHandle(_ambassadorInterface->registerObjectInstanceWithRegions(objectClassHandle, attributeHandleVectorRegionHandleVectorPairVector));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstanceWithRegions(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                               rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                               rti1516AttributeHandleSetRegionHandleSetPairVector,
                                                               std::wstring const & objectInstanceName)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::ObjectClassNotPublished,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotPublished,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::ObjectInstanceNameNotReserved,
         rti1516::ObjectInstanceNameInUse,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    return OpenRTI::_O1516ObjectInstanceHandle(_ambassadorInterface->registerObjectInstanceWithRegions(objectClassHandle,
                                                                                                       attributeHandleVectorRegionHandleVectorPairVector,
                                                                                                       ucsToUtf8(objectInstanceName)));
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw rti1516::ObjectClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw rti1516::AttributeNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw rti1516::ObjectInstanceNameNotReserved(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::ObjectInstanceNameInUse& e) {
    throw rti1516::ObjectInstanceNameInUse(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::associateRegionsForUpdates(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                        rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                        rti1516AttributeHandleSetRegionHandleSetPairVector)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->associateRegionsForUpdates(objectInstanceHandle, attributeHandleVectorRegionHandleVectorPairVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unassociateRegionsForUpdates(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                          rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                          rti1516AttributeHandleSetRegionHandleSetPairVector)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    OpenRTI::_I1516AttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->unassociateRegionsForUpdates(objectInstanceHandle, attributeHandleVectorRegionHandleVectorPairVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeObjectClassAttributesWithRegions(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                                       rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                                       rti1516AttributeHandleSetRegionHandleSetPairVector,
                                                                       bool active)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->subscribeObjectClassAttributesWithRegions(objectClassHandle,
                                                                    attributeHandleVectorRegionHandleVectorPairVector,
                                                                    active, std::string());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeObjectClassAttributesWithRegions(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                                         rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                                         rti1516AttributeHandleSetRegionHandleSetPairVector)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    _ambassadorInterface->unsubscribeObjectClassAttributesWithRegions(objectClassHandle,
                                                                      attributeHandleVectorRegionHandleVectorPairVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::subscribeInteractionClassWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                                  rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                                                                  bool active)
  RTI_THROW ((rti1516::InteractionClassNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateServiceInvocationsAreBeingReportedViaMOM,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516RegionHandleVector regionHandleVector(rti1516RegionHandleSet);
    _ambassadorInterface->subscribeInteractionClassWithRegions(interactionClassHandle, regionHandleVector, active);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateServiceInvocationsAreBeingReportedViaMOM& e) {
    throw rti1516::FederateServiceInvocationsAreBeingReportedViaMOM(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::unsubscribeInteractionClassWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                                    rti1516::RegionHandleSet const & rti1516RegionHandleSet)
  RTI_THROW ((rti1516::InteractionClassNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516RegionHandleVector regionHandleVector(rti1516RegionHandleSet);
    _ambassadorInterface->unsubscribeInteractionClassWithRegions(interactionClassHandle, regionHandleVector);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::sendInteractionWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                                                        rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                                                        rti1516::VariableLengthData const & rti1516Tag)
  RTI_THROW ((rti1516::InteractionClassNotDefined,
         rti1516::InteractionClassNotPublished,
         rti1516::InteractionParameterNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516ParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516RegionHandleVector regionHandleVector(rti1516RegionHandleSet);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    _ambassadorInterface->sendInteractionWithRegions(interactionClassHandle, parameterValueVector, regionHandleVector, tag);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::MessageRetractionHandle
RTIambassadorImplementation::sendInteractionWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                                                        rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                                                        rti1516::VariableLengthData const & rti1516Tag,
                                                        rti1516::LogicalTime const & rti1516LogicalTime)
RTI_THROW ((rti1516::InteractionClassNotDefined,
       rti1516::InteractionClassNotPublished,
       rti1516::InteractionParameterNotDefined,
       rti1516::InvalidRegion,
       rti1516::RegionNotCreatedByThisFederate,
       rti1516::InvalidRegionContext,
       rti1516::InvalidLogicalTime,
       rti1516::FederateNotExecutionMember,
       rti1516::SaveInProgress,
       rti1516::RestoreInProgress,
       rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516ParameterValueVector parameterValueVector(rti1516ParameterHandleValueMap);
    OpenRTI::_I1516RegionHandleVector regionHandleVector(rti1516RegionHandleSet);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    return OpenRTI::_O1516MessageRetractionHandle(_ambassadorInterface->sendInteractionWithRegions(interactionClassHandle, parameterValueVector, regionHandleVector, tag, rti1516LogicalTime));
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw rti1516::InteractionClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw rti1516::InteractionClassNotPublished(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw rti1516::InvalidLogicalTime(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::requestAttributeValueUpdateWithRegions(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                                    rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                                    rti1516AttributeHandleSetRegionHandleSetPairVector,
                                                                    rti1516::VariableLengthData const & rti1516Tag)
  RTI_THROW ((rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandleVectorRegionHandleVectorPairVector attributeHandleVectorRegionHandleVectorPairVector(rti1516AttributeHandleSetRegionHandleSetPairVector);
    OpenRTI::_I1516VariableLengthData tag(rti1516Tag);
    _ambassadorInterface->requestAttributeValueUpdateWithRegions(objectClassHandle, attributeHandleVectorRegionHandleVectorPairVector, tag);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw rti1516::ObjectClassNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRegionContext& e) {
    throw rti1516::InvalidRegionContext(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::ObjectClassHandle
RTIambassadorImplementation::getObjectClassHandle(std::wstring const & name)
  RTI_THROW ((rti1516::NameNotFound,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    return OpenRTI::_O1516ObjectClassHandle(_ambassadorInterface->getObjectClassHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getObjectClassName(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
  RTI_THROW ((rti1516::InvalidObjectClassHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return utf8ToUcs(_ambassadorInterface->getObjectClassName(objectClassHandle));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::AttributeHandle
RTIambassadorImplementation::getAttributeHandle(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                std::wstring const & attributeName)
  RTI_THROW ((rti1516::InvalidObjectClassHandle,
         rti1516::NameNotFound,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    return OpenRTI::_O1516AttributeHandle(_ambassadorInterface->getAttributeHandle(objectClassHandle, ucsToUtf8(attributeName)));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getAttributeName(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                              rti1516::AttributeHandle rti1516AttributeHandle)
  RTI_THROW ((rti1516::InvalidObjectClassHandle,
         rti1516::InvalidAttributeHandle,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandle attributeHandle(rti1516AttributeHandle);
    return utf8ToUcs(_ambassadorInterface->getAttributeName(objectClassHandle, attributeHandle));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidAttributeHandle& e) {
    throw rti1516::InvalidAttributeHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::InteractionClassHandle
RTIambassadorImplementation::getInteractionClassHandle(std::wstring const & name)
  RTI_THROW ((rti1516::NameNotFound,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    return OpenRTI::_O1516InteractionClassHandle(_ambassadorInterface->getInteractionClassHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getInteractionClassName(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
  RTI_THROW ((rti1516::InvalidInteractionClassHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    return utf8ToUcs(_ambassadorInterface->getInteractionClassName(interactionClassHandle));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::ParameterHandle
RTIambassadorImplementation::getParameterHandle(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                std::wstring const & parameterName)
  RTI_THROW ((rti1516::InvalidInteractionClassHandle,
         rti1516::NameNotFound,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    return OpenRTI::_O1516ParameterHandle(_ambassadorInterface->getParameterHandle(interactionClassHandle, ucsToUtf8(parameterName)));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getParameterName(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                              rti1516::ParameterHandle rti1516ParameterHandle)
  RTI_THROW ((rti1516::InvalidInteractionClassHandle,
         rti1516::InvalidParameterHandle,
         rti1516::InteractionParameterNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    OpenRTI::_I1516ParameterHandle parameterHandle(rti1516ParameterHandle);
    return utf8ToUcs(_ambassadorInterface->getParameterName(interactionClassHandle, parameterHandle));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidParameterHandle& e) {
    throw rti1516::InvalidParameterHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw rti1516::InteractionParameterNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::ObjectInstanceHandle
RTIambassadorImplementation::getObjectInstanceHandle(std::wstring const & name)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    return OpenRTI::_O1516ObjectInstanceHandle(_ambassadorInterface->getObjectInstanceHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getObjectInstanceName(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    return utf8ToUcs(_ambassadorInterface->getObjectInstanceName(objectInstanceHandle));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::DimensionHandle
RTIambassadorImplementation::getDimensionHandle(std::wstring const & name)
  RTI_THROW ((rti1516::NameNotFound,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    return OpenRTI::_O1516DimensionHandle(_ambassadorInterface->getDimensionHandle(ucsToUtf8(name)));
  } catch (const OpenRTI::NameNotFound& e) {
    throw rti1516::NameNotFound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getDimensionName(rti1516::DimensionHandle rti1516DimensionHandle)
  RTI_THROW ((rti1516::InvalidDimensionHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516DimensionHandle dimensionHandle(rti1516DimensionHandle);
    return utf8ToUcs(_ambassadorInterface->getDimensionName(dimensionHandle));
  } catch (const OpenRTI::InvalidDimensionHandle& e) {
    throw rti1516::InvalidDimensionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

unsigned long
RTIambassadorImplementation::getDimensionUpperBound(rti1516::DimensionHandle rti1516DimensionHandle)
  RTI_THROW ((rti1516::InvalidDimensionHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516DimensionHandle dimensionHandle(rti1516DimensionHandle);
    return _ambassadorInterface->getDimensionUpperBound(dimensionHandle);
  } catch (const OpenRTI::InvalidDimensionHandle& e) {
    throw rti1516::InvalidDimensionHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::DimensionHandleSet
RTIambassadorImplementation::getAvailableDimensionsForClassAttribute(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                                     rti1516::AttributeHandle rti1516AttributeHandle)
  RTI_THROW ((rti1516::InvalidObjectClassHandle,
         rti1516::InvalidAttributeHandle,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectClassHandle objectClassHandle(rti1516ObjectClassHandle);
    OpenRTI::_I1516AttributeHandle attributeHandle(rti1516AttributeHandle);
    return OpenRTI::_O1516DimensionHandleSet(_ambassadorInterface->getAvailableDimensionsForClassAttribute(objectClassHandle, attributeHandle));
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw rti1516::InvalidObjectClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidAttributeHandle& e) {
    throw rti1516::InvalidAttributeHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw rti1516::AttributeNotDefined(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::ObjectClassHandle
RTIambassadorImplementation::getKnownObjectClassHandle(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  RTI_THROW ((rti1516::ObjectInstanceNotKnown,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516ObjectInstanceHandle objectInstanceHandle(rti1516ObjectInstanceHandle);
    return OpenRTI::_O1516ObjectClassHandle(_ambassadorInterface->getKnownObjectClassHandle(objectInstanceHandle));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw rti1516::ObjectInstanceNotKnown(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::DimensionHandleSet
RTIambassadorImplementation::getAvailableDimensionsForInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
  RTI_THROW ((rti1516::InvalidInteractionClassHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516InteractionClassHandle interactionClassHandle(rti1516InteractionClassHandle);
    return OpenRTI::_O1516DimensionHandleSet(_ambassadorInterface->getAvailableDimensionsForInteractionClass(interactionClassHandle));
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw rti1516::InvalidInteractionClassHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::TransportationType
RTIambassadorImplementation::getTransportationType(std::wstring const & transportationName)
  RTI_THROW ((rti1516::InvalidTransportationName,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    return translate(_ambassadorInterface->getTransportationType(ucsToUtf8(transportationName)));
  } catch (const OpenRTI::InvalidTransportationName& e) {
    throw rti1516::InvalidTransportationName(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getTransportationName(rti1516::TransportationType transportationType)
  RTI_THROW ((rti1516::InvalidTransportationType,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    return utf8ToUcs(_ambassadorInterface->getTransportationName(translate(transportationType)));
  } catch (const OpenRTI::InvalidTransportationType& e) {
    throw rti1516::InvalidTransportationType(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::OrderType
RTIambassadorImplementation::getOrderType(std::wstring const & orderName)
  RTI_THROW ((rti1516::InvalidOrderName,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    return translate(_ambassadorInterface->getOrderType(ucsToUtf8(orderName)));
  } catch (const OpenRTI::InvalidOrderName& e) {
    throw rti1516::InvalidOrderName(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

std::wstring
RTIambassadorImplementation::getOrderName(rti1516::OrderType orderType)
  RTI_THROW ((rti1516::InvalidOrderType,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    return utf8ToUcs(_ambassadorInterface->getOrderName(translate(orderType)));
  } catch (const OpenRTI::InvalidOrderType& e) {
    throw rti1516::InvalidOrderType(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableObjectClassRelevanceAdvisorySwitch()
  RTI_THROW ((rti1516::ObjectClassRelevanceAdvisorySwitchIsOn,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->enableObjectClassRelevanceAdvisorySwitch();
  } catch (const OpenRTI::ObjectClassRelevanceAdvisorySwitchIsOn& e) {
    throw rti1516::ObjectClassRelevanceAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableObjectClassRelevanceAdvisorySwitch()
  RTI_THROW ((rti1516::ObjectClassRelevanceAdvisorySwitchIsOff,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->disableObjectClassRelevanceAdvisorySwitch();
  } catch (const OpenRTI::ObjectClassRelevanceAdvisorySwitchIsOff& e) {
    throw rti1516::ObjectClassRelevanceAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableAttributeRelevanceAdvisorySwitch ()
  RTI_THROW ((rti1516::AttributeRelevanceAdvisorySwitchIsOn,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->enableAttributeRelevanceAdvisorySwitch();
  } catch (const OpenRTI::AttributeRelevanceAdvisorySwitchIsOn& e) {
    throw rti1516::AttributeRelevanceAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableAttributeRelevanceAdvisorySwitch ()
  RTI_THROW ((rti1516::AttributeRelevanceAdvisorySwitchIsOff,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->disableAttributeRelevanceAdvisorySwitch();
  } catch (const OpenRTI::AttributeRelevanceAdvisorySwitchIsOff& e) {
    throw rti1516::AttributeRelevanceAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableAttributeScopeAdvisorySwitch ()
  RTI_THROW ((rti1516::AttributeScopeAdvisorySwitchIsOn,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->enableAttributeScopeAdvisorySwitch();
  } catch (const OpenRTI::AttributeScopeAdvisorySwitchIsOn& e) {
    throw rti1516::AttributeScopeAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableAttributeScopeAdvisorySwitch ()
  RTI_THROW ((rti1516::AttributeScopeAdvisorySwitchIsOff,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->disableAttributeScopeAdvisorySwitch();
  } catch (const OpenRTI::AttributeScopeAdvisorySwitchIsOff& e) {
    throw rti1516::AttributeScopeAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableInteractionRelevanceAdvisorySwitch ()
  RTI_THROW ((rti1516::InteractionRelevanceAdvisorySwitchIsOn,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->enableInteractionRelevanceAdvisorySwitch();
  } catch (const OpenRTI::InteractionRelevanceAdvisorySwitchIsOn& e) {
    throw rti1516::InteractionRelevanceAdvisorySwitchIsOn(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableInteractionRelevanceAdvisorySwitch ()
  RTI_THROW ((rti1516::InteractionRelevanceAdvisorySwitchIsOff,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    _ambassadorInterface->disableInteractionRelevanceAdvisorySwitch();
  } catch (const OpenRTI::InteractionRelevanceAdvisorySwitchIsOff& e) {
    throw rti1516::InteractionRelevanceAdvisorySwitchIsOff(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::DimensionHandleSet
RTIambassadorImplementation::getDimensionHandleSet(rti1516::RegionHandle rti1516RegionHandle)
  RTI_THROW ((rti1516::InvalidRegion,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516RegionHandle regionHandle(rti1516RegionHandle);
    return OpenRTI::_O1516DimensionHandleSet(_ambassadorInterface->getDimensionHandleSet(regionHandle));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::RangeBounds
RTIambassadorImplementation::getRangeBounds(rti1516::RegionHandle rti1516RegionHandle,
                                            rti1516::DimensionHandle rti1516DimensionHandle)
  RTI_THROW ((rti1516::InvalidRegion,
         rti1516::RegionDoesNotContainSpecifiedDimension,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516RegionHandle regionHandle(rti1516RegionHandle);
    OpenRTI::_I1516DimensionHandle dimensionHandle(rti1516DimensionHandle);
    return OpenRTI::_O1516RangeBounds(_ambassadorInterface->getRangeBounds(regionHandle, dimensionHandle));
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionDoesNotContainSpecifiedDimension& e) {
    throw rti1516::RegionDoesNotContainSpecifiedDimension(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::setRangeBounds(rti1516::RegionHandle rti1516RegionHandle,
                                            rti1516::DimensionHandle rti1516DimensionHandle,
                                            rti1516::RangeBounds const & rti1516RangeBounds)
  RTI_THROW ((rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::RegionDoesNotContainSpecifiedDimension,
         rti1516::InvalidRangeBound,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516RegionHandle regionHandle(rti1516RegionHandle);
    OpenRTI::_I1516DimensionHandle dimensionHandle(rti1516DimensionHandle);
    OpenRTI::_I1516RangeBounds rangeBounds(rti1516RangeBounds);
    _ambassadorInterface->setRangeBounds(regionHandle, dimensionHandle, rangeBounds);
  } catch (const OpenRTI::InvalidRegion& e) {
    throw rti1516::InvalidRegion(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionNotCreatedByThisFederate& e) {
    throw rti1516::RegionNotCreatedByThisFederate(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RegionDoesNotContainSpecifiedDimension& e) {
    throw rti1516::RegionDoesNotContainSpecifiedDimension(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidRangeBound& e) {
    throw rti1516::InvalidRangeBound(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

unsigned long
RTIambassadorImplementation::normalizeFederateHandle(rti1516::FederateHandle rti1516FederateHandle)
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::InvalidFederateHandle,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::_I1516FederateHandle federateHandle(rti1516FederateHandle);
    return _ambassadorInterface->normalizeFederateHandle(federateHandle);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidFederateHandle& e) {
    throw rti1516::InvalidFederateHandle(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

unsigned long
RTIambassadorImplementation::normalizeServiceGroup(rti1516::ServiceGroupIndicator rti1516ServiceGroup)
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::InvalidServiceGroup,
         rti1516::RTIinternalError))
{
  try {
    OpenRTI::ServiceGroupIndicator serviceGroup;
    switch (rti1516ServiceGroup) {
    case rti1516::FEDERATION_MANAGEMENT:
      serviceGroup = OpenRTI::FEDERATION_MANAGEMENT;
      break;
    case rti1516::DECLARATION_MANAGEMENT:
      serviceGroup = OpenRTI::DECLARATION_MANAGEMENT;
      break;
    case rti1516::OBJECT_MANAGEMENT:
      serviceGroup = OpenRTI::OBJECT_MANAGEMENT;
      break;
    case rti1516::OWNERSHIP_MANAGEMENT:
      serviceGroup = OpenRTI::OWNERSHIP_MANAGEMENT;
      break;
    case rti1516::TIME_MANAGEMENT:
      serviceGroup = OpenRTI::TIME_MANAGEMENT;
      break;
    case rti1516::DATA_DISTRIBUTION_MANAGEMENT:
      serviceGroup = OpenRTI::DATA_DISTRIBUTION_MANAGEMENT;
      break;
    case rti1516::SUPPORT_SERVICES:
      serviceGroup = OpenRTI::SUPPORT_SERVICES;
      break;
    default:
      throw OpenRTI::InvalidServiceGroup("");
    }

    return _ambassadorInterface->normalizeServiceGroup(serviceGroup);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::InvalidServiceGroup& e) {
    throw rti1516::InvalidServiceGroup(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::NotConnected& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::evokeCallback(double approximateMinimumTimeInSeconds)
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    if (!_ambassadorInterface->getFederate())
      throw OpenRTI::FederateNotExecutionMember();
    return _ambassadorInterface->evokeCallback(approximateMinimumTimeInSeconds);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

bool
RTIambassadorImplementation::evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                                                    double approximateMaximumTimeInSeconds)
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError))
{
  try {
    if (!_ambassadorInterface->getFederate())
      throw OpenRTI::FederateNotExecutionMember();
    return _ambassadorInterface->evokeMultipleCallbacks(approximateMinimumTimeInSeconds, approximateMaximumTimeInSeconds);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::enableCallbacks ()
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    if (!_ambassadorInterface->getFederate())
      throw OpenRTI::FederateNotExecutionMember();
    _ambassadorInterface->enableCallbacks();
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

void
RTIambassadorImplementation::disableCallbacks ()
  RTI_THROW ((rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError))
{
  try {
    if (!_ambassadorInterface->getFederate())
      throw OpenRTI::FederateNotExecutionMember();
    _ambassadorInterface->disableCallbacks();
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw rti1516::FederateNotExecutionMember(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::SaveInProgress& e) {
    throw rti1516::SaveInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw rti1516::RestoreInProgress(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::FederateHandle
RTIambassadorImplementation::decodeFederateHandle(rti1516::VariableLengthData const & encodedValue) const
{
  try {
    return rti1516::FederateHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::ObjectClassHandle
RTIambassadorImplementation::decodeObjectClassHandle(rti1516::VariableLengthData const & encodedValue) const
{
  try {
    return rti1516::ObjectClassHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::InteractionClassHandle
RTIambassadorImplementation::decodeInteractionClassHandle(rti1516::VariableLengthData const & encodedValue) const
{
  try {
    return rti1516::InteractionClassHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::ObjectInstanceHandle
RTIambassadorImplementation::decodeObjectInstanceHandle(rti1516::VariableLengthData const & encodedValue) const
{
  try {
    return rti1516::ObjectInstanceHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::AttributeHandle
RTIambassadorImplementation::decodeAttributeHandle(rti1516::VariableLengthData const & encodedValue) const
{
  try {
    return rti1516::AttributeHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::ParameterHandle
RTIambassadorImplementation::decodeParameterHandle(rti1516::VariableLengthData const & encodedValue) const
{
  try {
    return rti1516::ParameterHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::DimensionHandle
RTIambassadorImplementation::decodeDimensionHandle(rti1516::VariableLengthData const & encodedValue) const
{
  try {
    return rti1516::DimensionHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::MessageRetractionHandle
RTIambassadorImplementation::decodeMessageRetractionHandle(rti1516::VariableLengthData const & encodedValue) const
{
  try {
    return rti1516::MessageRetractionHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

rti1516::RegionHandle
RTIambassadorImplementation::decodeRegionHandle(rti1516::VariableLengthData const & encodedValue) const
{
  try {
    return rti1516::RegionHandleFriend::decode(encodedValue);
  } catch (const OpenRTI::CouldNotDecode& e) {
    throw rti1516::CouldNotDecode(OpenRTI::utf8ToUcs(e.what()));
  } catch (const std::exception& e) {
    throw rti1516::RTIinternalError(OpenRTI::utf8ToUcs(e.what()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown internal error!");
  }
}

}
