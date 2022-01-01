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

#include <RTI.hh>

#include <cstring>
#include <fstream>
#include <memory>

#include "Ambassador.h"
#include "AttributeHandleSetCallback.h"
#include "AttributeHandleSetImplementation.h"
#include "AttributeHandleValuePairSetCallback.h"
#include "AttributeHandleValuePairSetImplementation.h"
#include "FederateHandleSetImplementation.h"
#include "FDD1516FileReader.h"
#include "FEDFileReader.h"
#include "LogStream.h"
#include "ParameterHandleValuePairSetCallback.h"
#include "ParameterHandleValuePairSetImplementation.h"
#include "RTI13LogicalTimeFactory.h"
#include "StringUtils.h"
#include "TemplateTimeManagement.h"

static void loadModule(OpenRTI::FOMStringModuleList& fomModuleList, std::istream& stream)
{
  try {
    stream >> std::skipws;
    if (stream.peek() == '<')
      fomModuleList.push_back(OpenRTI::FDD1516FileReader::read(stream, std::string()));
    else
      fomModuleList.push_back(OpenRTI::FEDFileReader::read(stream));
  } catch (const OpenRTI::Exception& e) {
    throw RTI::ErrorReadingFED(e.what());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown error while reading fed file");
  }
}

static void loadModule(OpenRTI::FOMStringModuleList& fomModuleList, const std::string& fomModule)
{
  if (fomModule.empty())
    throw RTI::CouldNotOpenFED("Empty module.");
  std::ifstream stream(OpenRTI::utf8ToLocale(fomModule).c_str());
  if (stream.is_open()) {
    loadModule(fomModuleList, stream);
  } else if (fomModule.compare(0, 8, "file:///") == 0) {
    loadModule(fomModuleList, fomModule.substr(8));
  } else if (fomModule.compare(0, 16, "data:text/plain,") == 0) {
    std::stringstream stream(fomModule.substr(16));
    loadModule(fomModuleList, stream);
  } else if (fomModule.compare(0, 6, "data:,") == 0) {
    std::stringstream stream(fomModule.substr(6));
    loadModule(fomModuleList, stream);
  } else {
    throw RTI::CouldNotOpenFED(fomModule.c_str());
  }
}

static OpenRTI::URL federationExecutionToUrl(const std::string& federationExecutionName)
{
  OpenRTI::URL url;
  if (federationExecutionName.find("://") != std::string::npos)
    url = OpenRTI::URL::fromUrl(federationExecutionName);
  else
    url.setPath(federationExecutionName);
  return url;
}

/// Returns a new string as required with RTI13 interfaces
static char* newUtf8ToLocale(const std::string& utf8)
{
  std::string s = OpenRTI::utf8ToLocale(utf8);
  char* data = new char[s.size()+1];
  data[s.size()] = 0;
  return std::strncpy(data, s.c_str(), s.size());
}

/// Helper to get conformant handles from internal handles
template<typename T>
static RTI::ULong rti13Handle(const OpenRTI::Handle<T>& handle)
{
  if (!handle.valid())
    return ~RTI::ULong(0);
  return handle.getHandle();
}

static RTI::EventRetractionHandle rti13MessageRetractionHandle(OpenRTI::MessageRetractionHandle messageRetractionHandle)
{
  RTI::EventRetractionHandle eventRetractionHandle;
  eventRetractionHandle.sendingFederate = messageRetractionHandle.getFederateHandle();
  eventRetractionHandle.theSerialNumber = messageRetractionHandle.getSerial();
  return eventRetractionHandle;
}

static OpenRTI::MessageRetractionHandle toOpenRTIMessageRetractionHandle(RTI::EventRetractionHandle eventRetractionHandle)
{
  return OpenRTI::MessageRetractionHandle(eventRetractionHandle.sendingFederate, eventRetractionHandle.theSerialNumber);
}

static RTI::TransportType rti13TransportType(OpenRTI::TransportationType transportationType)
{
  switch (transportationType) {
  case OpenRTI::RELIABLE:
    return 0;
  default:
    return 1;
  }
}

static OpenRTI::TransportationType toOpenRTITransportationType(RTI::TransportType transportationType)
{
  switch (transportationType) {
  case 0:
    return OpenRTI::RELIABLE;
  default:
    return OpenRTI::BEST_EFFORT;
  }
}

static RTI::OrderType rti13OrderType(OpenRTI::OrderType orderType)
{
  switch (orderType) {
  case OpenRTI::TIMESTAMP:
    return 0;
  default:
    return 1;
  }
}

static OpenRTI::OrderType toOpenRTIOrderType(RTI::OrderType orderType)
{
  switch (orderType) {
  case 0:
    return OpenRTI::TIMESTAMP;
  default:
    return OpenRTI::RECEIVE;
  }
}

namespace OpenRTI {

class OPENRTI_LOCAL _I13VariableLengthData : public OpenRTI::VariableLengthData {
public:
  _I13VariableLengthData(const char* tag)
  {
    if (tag)
      setData(tag, std::strlen(tag));
  }
};

class OPENRTI_LOCAL _I13AttributeHandleVector : public OpenRTI::AttributeHandleVector {
public:
  _I13AttributeHandleVector(const RTI::AttributeHandleSet& attributeHandleSet)
  {
    RTI::ULong attributeHandleSetSize = attributeHandleSet.size();
    reserve(attributeHandleSet.size() + 1);
    for (RTI::ULong i = 0; i < attributeHandleSetSize; ++i)
      push_back(attributeHandleSet.getHandle(i));
  }
};
class OPENRTI_LOCAL _I13FederateHandleVector : public OpenRTI::FederateHandleVector {
public:
  _I13FederateHandleVector(const RTI::FederateHandleSet& federateHandleSet)
  {
    RTI::ULong federateHandleSetSize = federateHandleSet.size();
    reserve(federateHandleSet.size() + 1);
    for (RTI::ULong i = 0; i < federateHandleSetSize; ++i)
      push_back(federateHandleSet.getHandle(i));
  }
};

class OPENRTI_LOCAL _I13AttributeValueVector : public OpenRTI::AttributeValueVector {
public:
  _I13AttributeValueVector(const RTI::AttributeHandleValuePairSet& attributeHandleValuePairSet)
  {
    RTI::ULong attributeHandleValuePairSetSize = attributeHandleValuePairSet.size();
    reserve(attributeHandleValuePairSetSize);
    for (RTI::ULong i = 0; i < attributeHandleValuePairSetSize; ++i) {
      push_back(OpenRTI::AttributeValue());
      back().setAttributeHandle(attributeHandleValuePairSet.getHandle(i));
      RTI::ULong length;
      char* value = attributeHandleValuePairSet.getValuePointer(i, length);
      back().getValue().setData(value, length);
    }
  }
};
class OPENRTI_LOCAL _I13ParameterValueVector : public OpenRTI::ParameterValueVector {
public:
  _I13ParameterValueVector(const RTI::ParameterHandleValuePairSet& parameterHandleValuePairSet)
  {
    RTI::ULong parameterHandleValuePairSetSize = parameterHandleValuePairSet.size();
    reserve(parameterHandleValuePairSetSize);
    for (RTI::ULong i = 0; i < parameterHandleValuePairSetSize; ++i) {
      push_back(OpenRTI::ParameterValue());
      back().setParameterHandle(parameterHandleValuePairSet.getHandle(i));
      RTI::ULong length;
      char* value = parameterHandleValuePairSet.getValuePointer(i, length);
      back().getValue().setData(value, length);
    }
  }
};

class OPENRTI_LOCAL RTI13Traits {
public:
  // The bindings have different logical times
  typedef RTI::FedTime NativeLogicalTime;
  typedef RTI::FedTime NativeLogicalTimeInterval;
};

} // namespace OpenRTI

class OPENRTI_LOCAL RTIambPrivateRefs : public OpenRTI::Ambassador<OpenRTI::RTI13Traits> {
public:
  RTIambPrivateRefs() :
    _concurrentAccess(false)
  { }

  virtual void connectionLost(const std::string& faultDescription)
  {
    throw RTI::RTIinternalError(faultDescription.c_str());
  }

  virtual void reportFederationExecutions(const OpenRTI::FederationExecutionInformationVector& theFederationExecutionInformationList)
    RTI_NOEXCEPT
  { }

  virtual void synchronizationPointRegistrationResponse(const std::string& label, OpenRTI::RegisterFederationSynchronizationPointResponseType reason)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      switch (reason) {
      case OpenRTI::RegisterFederationSynchronizationPointResponseSuccess:
        _federateAmbassador->synchronizationPointRegistrationSucceeded(OpenRTI::utf8ToLocale(label).c_str());
        break;
      default:
        _federateAmbassador->synchronizationPointRegistrationFailed(OpenRTI::utf8ToLocale(label).c_str());
        break;
      }
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
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
      _federateAmbassador->announceSynchronizationPoint(OpenRTI::utf8ToLocale(label).c_str(), rti13Tag(tag));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
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
      _federateAmbassador->federationSynchronized(OpenRTI::utf8ToLocale(label).c_str());
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
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
  //     _federateAmbassador->initiateFederateSave(OpenRTI::utf8ToLocale(label).c_str());
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   }
  // }

  // virtual
  // void
  // initiateFederateSave(std::string const& label,
  //                      OpenRTI::LogicalTime const&)
  //   RTI_THROW ((OpenRTI::UnableToPerformSave,
  //          OpenRTI::InvalidLogicalTime,
  //          OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(OpenRTI::utf8ToLocale(label).c_str());
  //   } catch (const RTI::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(OpenRTI::localeToUtf8(e._reason));
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
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
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // virtual void
  // federationNotSaved(OpenRTI::SaveFailureReason theSaveFailureReason)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationNotSaved();
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // // 4.17
  // virtual void
  // federationSaveStatusResponse(OpenRTI::FederateHandleSaveStatusPairVector const& theFederateStatusVector)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   /// Should never be called since there is also no queryFederationSaveStatus call in RTI13
  //   throw OpenRTI::FederateInternalError();
  // }

  // // 4.19
  // virtual void
  // requestFederationRestoreSucceeded(std::string const& label)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreSucceeded(OpenRTI::utf8ToLocale(label).c_str());
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // virtual void
  // requestFederationRestoreFailed(std::string const& label)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     /// FIXME, we might route that second argument through the internal interface
  //     _federateAmbassador->requestFederationRestoreFailed(OpenRTI::utf8ToLocale(label).c_str(), "Don't know why");
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
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
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // // 4.21
  // virtual void
  // initiateFederateRestore(std::string const & label,
  //                         OpenRTI::FederateHandle handle)
  //   RTI_THROW ((OpenRTI::SpecifiedSaveLabelDoesNotExist,
  //          OpenRTI::CouldNotInitiateRestore,
  //          OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateRestore(OpenRTI::utf8ToLocale(label).c_str(), rti13Handle(handle));
  //   } catch (const RTI::SpecifiedSaveLabelDoesNotExist& e) {
  //     throw OpenRTI::SpecifiedSaveLabelDoesNotExist(OpenRTI::localeToUtf8(e._reason));
  //   } catch (const RTI::CouldNotRestore& e) {
  //     throw OpenRTI::CouldNotInitiateRestore(OpenRTI::localeToUtf8(e._reason));
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
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
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // virtual void
  // federationNotRestored(OpenRTI::RestoreFailureReason)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationNotRestored();
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // // 4.25
  // virtual void
  // federationRestoreStatusResponse(OpenRTI::FederateHandleRestoreStatusPairVector const& theFederateStatusVector)
  //   RTI_THROW ((OpenRTI::FederateInternalError))
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     throw RTI::FederateInternalError("Not implemented");
  //     // _federateAmbassador
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
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
        _federateAmbassador->startRegistrationForObjectClass(rti13Handle(objectClassHandle));
      else
        _federateAmbassador->stopRegistrationForObjectClass(rti13Handle(objectClassHandle));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual void turnInteractionsOn(OpenRTI::InteractionClassHandle interactionClassHandle, bool on)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      if (on)
        _federateAmbassador->turnInteractionsOn(rti13Handle(interactionClassHandle));
      else
        _federateAmbassador->turnInteractionsOff(rti13Handle(interactionClassHandle));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  ////////////////////////////////
  // Object Management Services //
  ////////////////////////////////

  virtual void objectInstanceNameReservationSucceeded(const std::string& objectInstanceName)
    RTI_NOEXCEPT
  {
    Log(FederateAmbassador, Warning) << "Ignoring unexpected name reservation response!" << std::endl;
  }
  virtual void objectInstanceNameReservationFailed(const std::string& objectInstanceName)
    RTI_NOEXCEPT
  {
    Log(FederateAmbassador, Warning) << "Ignoring unexpected name reservation response!" << std::endl;
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
  discoverObjectInstance(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                         OpenRTI::ObjectClassHandle objectClassHandle,
                         std::string const& name)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->discoverObjectInstance(rti13Handle(objectInstanceHandle),
                                                  rti13Handle(objectClassHandle),
                                                  OpenRTI::utf8ToLocale(name).c_str());
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual void reflectAttributeValues(const OpenRTI::Federate::ObjectClass& objectClass, OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                                      const OpenRTI::AttributeValueVector& attributeValueVector, const OpenRTI::VariableLengthData& tag,
                                      OpenRTI::OrderType sentOrder, OpenRTI::TransportationType transportationType, OpenRTI::FederateHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      RTI::TransportType transportType = rti13TransportType(transportationType);
      RTI::OrderType orderType = rti13OrderType(sentOrder);
      AttributeHandleValuePairSetCallback attributeHandleValues(transportType, orderType);
      attributeHandleValues.getAttributeValues().reserve(attributeValueVector.size());
      for (OpenRTI::AttributeValueVector::const_iterator i = attributeValueVector.begin();
           i != attributeValueVector.end(); ++i) {
        if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == OpenRTI::Unsubscribed)
          continue;
        attributeHandleValues.getAttributeValues().push_back(*i);
      }
      if (!attributeHandleValues.getAttributeValues().empty())
        _federateAmbassador->reflectAttributeValues(rti13Handle(objectInstanceHandle), attributeHandleValues, rti13Tag(tag));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }
  virtual void reflectAttributeValues(const OpenRTI::Federate::ObjectClass& objectClass, OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                                      const OpenRTI::AttributeValueVector& attributeValueVector, const OpenRTI::VariableLengthData& tag,
                                      OpenRTI::OrderType sentOrder, OpenRTI::TransportationType transportationType,
                                      const NativeLogicalTime& logicalTime, OpenRTI::OrderType receivedOrder,
                                      OpenRTI::FederateHandle federateHandle)
    RTI_NOEXCEPT
  {
    reflectAttributeValues(objectClass, objectInstanceHandle,attributeValueVector, tag, sentOrder, transportationType,
                           logicalTime, receivedOrder, federateHandle, OpenRTI::MessageRetractionHandle());
  }
  virtual void reflectAttributeValues(const OpenRTI::Federate::ObjectClass& objectClass, OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                                      const OpenRTI::AttributeValueVector& attributeValueVector, const OpenRTI::VariableLengthData& tag,
                                      OpenRTI::OrderType sentOrder, OpenRTI::TransportationType transportationType,
                                      const NativeLogicalTime& logicalTime, OpenRTI::OrderType receivedOrder, OpenRTI::FederateHandle,
                                      OpenRTI::MessageRetractionHandle messageRetractionHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      RTI::TransportType transportType = rti13TransportType(transportationType);
      RTI::OrderType orderType = rti13OrderType(sentOrder);
      AttributeHandleValuePairSetCallback attributeHandleValues(transportType, orderType);
      attributeHandleValues.getAttributeValues().reserve(attributeValueVector.size());
      for (OpenRTI::AttributeValueVector::const_iterator i = attributeValueVector.begin();
           i != attributeValueVector.end(); ++i) {
        if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == OpenRTI::Unsubscribed)
          continue;
        attributeHandleValues.getAttributeValues().push_back(*i);
      }
      RTI::EventRetractionHandle eventRetractionHandle = rti13MessageRetractionHandle(messageRetractionHandle);
      if (!attributeHandleValues.getAttributeValues().empty())
        _federateAmbassador->reflectAttributeValues(rti13Handle(objectInstanceHandle), attributeHandleValues,
                                                    logicalTime, rti13Tag(tag), eventRetractionHandle);
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual void removeObjectInstance(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::VariableLengthData& tag,
                                    OpenRTI::OrderType, OpenRTI::FederateHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->removeObjectInstance(rti13Handle(objectInstanceHandle), rti13Tag(tag));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }
  virtual void removeObjectInstance(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::VariableLengthData& tag,
                                    OpenRTI::OrderType sentOrder, const NativeLogicalTime& logicalTime,
                                    OpenRTI::OrderType receivedOrder, OpenRTI::FederateHandle federateHandle)
    RTI_NOEXCEPT
  {
    removeObjectInstance(objectInstanceHandle, tag, sentOrder, logicalTime, receivedOrder, federateHandle, OpenRTI::MessageRetractionHandle());
  }
  virtual void removeObjectInstance(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::VariableLengthData& tag,
                                    OpenRTI::OrderType sentOrder, const NativeLogicalTime& logicalTime,
                                    OpenRTI::OrderType receivedOrder, OpenRTI::FederateHandle, OpenRTI::MessageRetractionHandle messageRetractionHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      RTI::EventRetractionHandle eventRetractionHandle = rti13MessageRetractionHandle(messageRetractionHandle);
      _federateAmbassador->removeObjectInstance(rti13Handle(objectInstanceHandle), logicalTime, rti13Tag(tag), eventRetractionHandle);
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual void receiveInteraction(const OpenRTI::Federate::InteractionClass& interactionClass,
                                  OpenRTI::InteractionClassHandle interactionClassHandle,
                                  const OpenRTI::ParameterValueVector& parameterValueVector, const OpenRTI::VariableLengthData& tag,
                                  OpenRTI::OrderType sentOrder, OpenRTI::TransportationType transportationType, OpenRTI::FederateHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      RTI::InteractionClassHandle rti13InteractionClassHandle = rti13Handle(interactionClassHandle);
      RTI::TransportType transportType = rti13TransportType(transportationType);
      RTI::OrderType orderType = rti13OrderType(sentOrder);
      // FIXME no regions so far
      ParameterHandleValuePairSetCallback parameterHandleArray(transportType, orderType, 0);
      parameterHandleArray.getParameterValues().reserve(parameterValueVector.size());
      for (std::vector<OpenRTI::ParameterValue>::const_iterator i = parameterValueVector.begin();
           i != parameterValueVector.end(); ++i) {
        if (!interactionClass.getParameter(i->getParameterHandle()))
          continue;
        parameterHandleArray.getParameterValues().push_back(*i);
      }
      _federateAmbassador->receiveInteraction(rti13InteractionClassHandle, parameterHandleArray, rti13Tag(tag));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }
  virtual void receiveInteraction(const OpenRTI::Federate::InteractionClass& interactionClass,
                                  OpenRTI::InteractionClassHandle interactionClassHandle,
                                  const OpenRTI::ParameterValueVector& parameterValueVector, const OpenRTI::VariableLengthData& tag,
                                  OpenRTI::OrderType sentOrder, OpenRTI::TransportationType transportationType,
                                  const NativeLogicalTime& logicalTime, OpenRTI::OrderType receivedOrder, OpenRTI::FederateHandle federateHandle)
    RTI_NOEXCEPT
  {
    receiveInteraction(interactionClass, interactionClassHandle, parameterValueVector, tag, sentOrder, transportationType,
                       logicalTime, receivedOrder, federateHandle, OpenRTI::MessageRetractionHandle());
  }
  virtual void receiveInteraction(const OpenRTI::Federate::InteractionClass& interactionClass,
                                  OpenRTI::InteractionClassHandle interactionClassHandle,
                                  const OpenRTI::ParameterValueVector& parameterValueVector, const OpenRTI::VariableLengthData& tag,
                                  OpenRTI::OrderType sentOrder, OpenRTI::TransportationType transportationType,
                                  const NativeLogicalTime& logicalTime, OpenRTI::OrderType receivedOrder, OpenRTI::FederateHandle,
                                  OpenRTI::MessageRetractionHandle messageRetractionHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      RTI::InteractionClassHandle rti13InteractionClassHandle = rti13Handle(interactionClassHandle);
      RTI::TransportType transportType = rti13TransportType(transportationType);
      RTI::OrderType orderType = rti13OrderType(sentOrder);
      // FIXME no regions so far
      ParameterHandleValuePairSetCallback parameterHandleArray(transportType, orderType, 0);
      parameterHandleArray.getParameterValues().reserve(parameterValueVector.size());
      for (std::vector<OpenRTI::ParameterValue>::const_iterator i = parameterValueVector.begin();
           i != parameterValueVector.end(); ++i) {
        if (!interactionClass.getParameter(i->getParameterHandle()))
          continue;
        parameterHandleArray.getParameterValues().push_back(*i);
      }
      RTI::EventRetractionHandle eventRetractionHandle = rti13MessageRetractionHandle(messageRetractionHandle);
      _federateAmbassador->receiveInteraction(rti13InteractionClassHandle, parameterHandleArray, logicalTime,
                                              rti13Tag(tag), eventRetractionHandle);
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.15
  virtual
  void
  attributesInScope(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributesInScope(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.16
  virtual
  void
  attributesOutOfScope(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributesOutOfScope(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.18
  virtual
  void
  provideAttributeValueUpdate(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                              const OpenRTI::AttributeHandleVector& attributeHandleVector,
                              const OpenRTI::VariableLengthData& tag)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->provideAttributeValueUpdate(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.19
  virtual
  void
  turnUpdatesOnForObjectInstance(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector, const std::string&)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->turnUpdatesOnForObjectInstance(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.20
  virtual
  void
  turnUpdatesOffForObjectInstance(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->turnUpdatesOffForObjectInstance(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  ///////////////////////////////////
  // Ownership Management Services //
  ///////////////////////////////////

  // 7.4
  virtual
  void
  requestAttributeOwnershipAssumption(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector,
                                      const OpenRTI::VariableLengthData& tag)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->requestAttributeOwnershipAssumption(rti13Handle(objectInstanceHandle),
                                                               AttributeHandleSetCallback(attributeHandleVector), rti13Tag(tag));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.5
  virtual
  void
  requestDivestitureConfirmation(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributeOwnershipDivestitureNotification(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.7
  virtual
  void
  attributeOwnershipAcquisitionNotification(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector,
                                            const OpenRTI::VariableLengthData& tag)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributeOwnershipAcquisitionNotification(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.10
  virtual
  void
  attributeOwnershipUnavailable(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributeOwnershipUnavailable(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.11
  virtual
  void
  requestAttributeOwnershipRelease(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector,
                                   const OpenRTI::VariableLengthData& tag)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->requestAttributeOwnershipRelease(rti13Handle(objectInstanceHandle),
                                                            AttributeHandleSetCallback(attributeHandleVector), rti13Tag(tag));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.15
  virtual
  void
  confirmAttributeOwnershipAcquisitionCancellation(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                                                   const OpenRTI::AttributeHandleVector& attributeHandleVector)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->confirmAttributeOwnershipAcquisitionCancellation(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.17
  virtual
  void
  informAttributeOwnership(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                           OpenRTI::AttributeHandle attributeHandle,
                           OpenRTI::FederateHandle federateHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->informAttributeOwnership(rti13Handle(objectInstanceHandle),
                                                    rti13Handle(attributeHandle),
                                                    rti13Handle(federateHandle));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual
  void
  attributeIsNotOwned(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                      OpenRTI::AttributeHandle attributeHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributeIsNotOwned(rti13Handle(objectInstanceHandle),
                                               rti13Handle(attributeHandle));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual
  void
  attributeIsOwnedByRTI(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                        OpenRTI::AttributeHandle attributeHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributeOwnedByRTI(rti13Handle(objectInstanceHandle),
                                               rti13Handle(attributeHandle));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  //////////////////////////////
  // Time Management Services //
  //////////////////////////////

  // 8.3
  virtual
  void
  timeRegulationEnabled(const RTI::FedTime& logicalTime)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeRegulationEnabled(logicalTime);
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 8.6
  virtual
  void
  timeConstrainedEnabled(const RTI::FedTime& logicalTime)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeConstrainedEnabled(logicalTime);
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 8.13
  virtual
  void
    timeAdvanceGrant(const RTI::FedTime& logicalTime)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeAdvanceGrant(logicalTime);
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 8.22
  virtual
  void
  requestRetraction(OpenRTI::MessageRetractionHandle theHandle)
    RTI_NOEXCEPT
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      // _federateAmbassador
      throw RTI::FederateInternalError("Not implemented");
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  /// Hmm, use that cached allocation, so that we only need to copy ...
  const char* rti13Tag(const OpenRTI::VariableLengthData& tag)
  {
    if (tag.empty())
      return "";
    _tagCache.assign(tag.charData(), tag.size());
    return _tagCache.c_str();
  }

  virtual OpenRTI::TimeManagement<Traits>* createTimeManagement(OpenRTI::Federate& federate)
  {
    return new OpenRTI::TemplateTimeManagement<Traits, OpenRTI::RTI13LogicalTimeFactory>(OpenRTI::RTI13LogicalTimeFactory(federate.getLogicalTimeFactoryName()));
  }

  struct ConcurrentAccessGuard {
    ConcurrentAccessGuard(RTIambPrivateRefs& p) : _p(p)
    {
      if (_p._concurrentAccess)
        throw RTI::ConcurrentAccessAttempted("Calling ambassador from callback!");
      _p._concurrentAccess = true;
    }
    ~ConcurrentAccessGuard()
    {
      _p._concurrentAccess = false;
    }

    RTIambPrivateRefs& _p;
  };

  void ensureConnected(const OpenRTI::URL& url)
  {
    OpenRTI::URL connectUrl = url;
    // For the path component strip the file part which is the bare federation execution name
    if (1 < url.getPath().size())
      connectUrl.setPath(OpenRTI::getBasePart(url.getPath()));

    if (!isConnected()) {
      connect(connectUrl, OpenRTI::StringStringListMap());

      _connectedUrl = connectUrl;
    } else if (_connectedUrl != connectUrl) {
      throw RTI::RTIinternalError("Connect url does not point to the same connection.");
    }
  }

  // The instantiation time argument list
  OpenRTI::URL _connectedUrl;

  bool _concurrentAccess;

  std::string _tagCache;

  RTI::FederateAmbassador* _federateAmbassador;
};

RTI::RTIambassador::RTIambassador()
  RTI_THROW ((MemoryExhausted, RTIinternalError)) :
  privateData(0),
  privateRefs(new RTIambPrivateRefs)
{
}

RTI::RTIambassador::~RTIambassador()
  RTI_THROW ((RTI::RTIinternalError))
{
  delete privateRefs;
  privateRefs = 0;
}

void
RTI::RTIambassador::createFederationExecution(const char* federationExecutionName,
                                              const char* fedFile)
  RTI_THROW ((RTI::FederationExecutionAlreadyExists,
         RTI::CouldNotOpenFED,
         RTI::ErrorReadingFED,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);

  // Make sure we can read the fed file
  if (!fedFile)
    throw RTI::CouldNotOpenFED("fedFile is NULL!");
  OpenRTI::FOMStringModuleList fomModuleList;
  loadModule(fomModuleList, fedFile);

  try {
    OpenRTI::URL url = federationExecutionToUrl(OpenRTI::localeToUtf8(federationExecutionName));
    privateRefs->ensureConnected(url);
    privateRefs->createFederationExecution(OpenRTI::getFilePart(url.getPath()), fomModuleList, "HLAfloat64Time");
  } catch (const OpenRTI::FederationExecutionAlreadyExists& e) {
    throw RTI::FederationExecutionAlreadyExists(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::destroyFederationExecution(const char* federationExecutionName)
  RTI_THROW ((RTI::FederatesCurrentlyJoined,
         RTI::FederationExecutionDoesNotExist,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);

  try {
    OpenRTI::URL url = federationExecutionToUrl(OpenRTI::localeToUtf8(federationExecutionName));
    privateRefs->ensureConnected(url);
    privateRefs->destroyFederationExecution(OpenRTI::getFilePart(url.getPath()));
  } catch (const OpenRTI::FederatesCurrentlyJoined& e) {
    throw RTI::FederatesCurrentlyJoined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederationExecutionDoesNotExist& e) {
    throw RTI::FederationExecutionDoesNotExist(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::FederateHandle
RTI::RTIambassador::joinFederationExecution(const char* federateType,
                                            const char* federationExecutionName,
                                            RTI::FederateAmbassadorPtr federateAmbassadorPointer)
  RTI_THROW ((RTI::FederateAlreadyExecutionMember,
         RTI::FederationExecutionDoesNotExist,
         RTI::CouldNotOpenFED,
         RTI::ErrorReadingFED,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!federateAmbassadorPointer)
    throw RTI::FederationExecutionDoesNotExist("Joining with a zero federate ambassador pointer!");

  try {
    OpenRTI::URL url = federationExecutionToUrl(OpenRTI::localeToUtf8(federationExecutionName));
    privateRefs->ensureConnected(url);
    std::string utf8FederateType = OpenRTI::localeToUtf8(federateType);
    FederateHandle federateHandle = privateRefs->joinFederationExecution(std::string(), utf8FederateType,
                                                                         OpenRTI::getFilePart(url.getPath()),
                                                                         OpenRTI::FOMStringModuleList());
    privateRefs->_federateAmbassador = federateAmbassadorPointer;
    return federateHandle;
  } catch (const OpenRTI::FederationExecutionDoesNotExist& e) {
    throw RTI::FederationExecutionDoesNotExist(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateAlreadyExecutionMember& e) {
    throw RTI::FederateAlreadyExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::resignFederationExecution(RTI::ResignAction RTI13resignAction)
  RTI_THROW ((RTI::FederateOwnsAttributes,
         RTI::FederateNotExecutionMember,
         RTI::InvalidResignAction,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::ResignAction resignAction;
  switch (RTI13resignAction) {
  case RTI::RELEASE_ATTRIBUTES:
    resignAction = OpenRTI::UNCONDITIONALLY_DIVEST_ATTRIBUTES;
    break;
  case RTI::DELETE_OBJECTS:
    resignAction = OpenRTI::DELETE_OBJECTS;
    break;
  case RTI::DELETE_OBJECTS_AND_RELEASE_ATTRIBUTES:
    resignAction = OpenRTI::DELETE_OBJECTS_THEN_DIVEST;
    break;
  case RTI::NO_ACTION:
    resignAction = OpenRTI::NO_ACTION;
    break;
  default:
    throw RTI::InvalidResignAction("Except");
  }

  try {
    privateRefs->_federateAmbassador = 0;
    privateRefs->resignFederationExecution(resignAction);
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw RTI::OwnershipAcquisitionPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw RTI::FederateOwnsAttributes(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::registerFederationSynchronizationPoint(const char* label,
                                                           const char* tag)
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    // According to the standard, an empty set also means all federates currently joined.
    OpenRTI::_I13VariableLengthData _tag(tag);
    OpenRTI::FederateHandleVector federateHandleVector;
    privateRefs->registerFederationSynchronizationPoint(OpenRTI::localeToUtf8(label), _tag, federateHandleVector);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::registerFederationSynchronizationPoint(const char* label,
                                                           const char* tag,
                                                           const RTI::FederateHandleSet& syncSet)
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13VariableLengthData _tag(tag);
    OpenRTI::_I13FederateHandleVector federateHandleVector(syncSet);
    privateRefs->registerFederationSynchronizationPoint(OpenRTI::localeToUtf8(label), _tag, federateHandleVector);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::synchronizationPointAchieved(const char* label)
  RTI_THROW ((RTI::SynchronizationPointLabelWasNotAnnounced,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->synchronizationPointAchieved(OpenRTI::localeToUtf8(label), true/*successfully*/);
  } catch (const OpenRTI::SynchronizationPointLabelNotAnnounced& e) {
    throw RTI::SynchronizationPointLabelWasNotAnnounced(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::requestFederationSave(const char* label, const RTI::FedTime& fedTime)
  RTI_THROW ((RTI::FederationTimeAlreadyPassed,
         RTI::InvalidFederationTime,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->requestFederationSave(OpenRTI::localeToUtf8(label), fedTime);
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw RTI::FederationTimeAlreadyPassed(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw RTI::InvalidFederationTime(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateUnableToUseTime& e) {
    throw RTI::InvalidFederationTime(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::requestFederationSave(const char* label)
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->requestFederationSave(OpenRTI::localeToUtf8(label));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::federateSaveBegun()
  RTI_THROW ((RTI::SaveNotInitiated,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->federateSaveBegun();
  } catch (const OpenRTI::SaveNotInitiated& e) {
    throw RTI::SaveNotInitiated(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::federateSaveComplete()
  RTI_THROW ((RTI::SaveNotInitiated,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->federateSaveComplete();
  } catch (const OpenRTI::FederateHasNotBegunSave& e) {
    throw RTI::SaveNotInitiated(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::federateSaveNotComplete()
  RTI_THROW ((RTI::SaveNotInitiated,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->federateSaveNotComplete();
  } catch (const OpenRTI::FederateHasNotBegunSave& e) {
    throw RTI::SaveNotInitiated(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::requestFederationRestore(const char* label)
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->requestFederationRestore(OpenRTI::localeToUtf8(label));
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::federateRestoreComplete()
  RTI_THROW ((RTI::RestoreNotRequested,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->federateRestoreComplete();
  } catch (const OpenRTI::RestoreNotRequested& e) {
    throw RTI::RestoreNotRequested(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::federateRestoreNotComplete()
  RTI_THROW ((RTI::RestoreNotRequested,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->federateRestoreNotComplete();
  } catch (const OpenRTI::RestoreNotRequested& e) {
    throw RTI::RestoreNotRequested(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::publishObjectClass(RTI::ObjectClassHandle objectClassHandle,
                                       const RTI::AttributeHandleSet& attributeHandleSet)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::OwnershipAcquisitionPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    privateRefs->publishObjectClassAttributes(objectClassHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw RTI::ObjectClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::unpublishObjectClass(RTI::ObjectClassHandle objectClassHandle)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotPublished,
         RTI::OwnershipAcquisitionPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->unpublishObjectClass(objectClassHandle);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw RTI::ObjectClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw RTI::OwnershipAcquisitionPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::publishInteractionClass(RTI::InteractionClassHandle interactionClassHandle)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::InteractionClassHandle OpenRTIInteractionClassHandle = interactionClassHandle;
    privateRefs->publishInteractionClass(OpenRTIInteractionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::unpublishInteractionClass(RTI::InteractionClassHandle interactionClassHandle)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::InteractionClassHandle OpenRTIInteractionClassHandle = interactionClassHandle;
    privateRefs->unpublishInteractionClass(OpenRTIInteractionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::subscribeObjectClassAttributes(RTI::ObjectClassHandle objectClassHandle,
                                                   const RTI::AttributeHandleSet& attributeHandleSet,
                                                   RTI::Boolean active)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    privateRefs->subscribeObjectClassAttributes(objectClassHandle, attributeHandleVector, active == RTI::RTI_TRUE, std::string());
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw RTI::ObjectClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::unsubscribeObjectClass(RTI::ObjectClassHandle objectClassHandle)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotSubscribed,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->unsubscribeObjectClass(objectClassHandle);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw RTI::ObjectClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::subscribeInteractionClass(RTI::InteractionClassHandle interactionClassHandle,
                                              RTI::Boolean active)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::FederateLoggingServiceCalls,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->subscribeInteractionClass(interactionClassHandle, active == RTI::RTI_TRUE);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateServiceInvocationsAreBeingReportedViaMOM& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::unsubscribeInteractionClass(RTI::InteractionClassHandle interactionClassHandle)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotSubscribed,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->unsubscribeInteractionClass(interactionClassHandle);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::ObjectHandle
RTI::RTIambassador::registerObjectInstance(RTI::ObjectClassHandle objectClassHandle,
                                           const char* name)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotPublished,
         RTI::ObjectAlreadyRegistered,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    return privateRefs->registerObjectInstance(objectClassHandle, OpenRTI::localeToUtf8(name), true);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw RTI::ObjectClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw RTI::ObjectClassNotPublished(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::ObjectInstanceNameNotReserved& e) {
    throw RTI::ObjectAlreadyRegistered(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::ObjectInstanceNameInUse& e) {
    throw RTI::ObjectAlreadyRegistered(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::ObjectHandle
RTI::RTIambassador::registerObjectInstance(RTI::ObjectClassHandle objectClassHandle)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotPublished,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    return privateRefs->registerObjectInstance(objectClassHandle);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw RTI::ObjectClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw RTI::ObjectClassNotPublished(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::EventRetractionHandle
RTI::RTIambassador::updateAttributeValues(RTI::ObjectHandle objectHandle,
                                          const RTI::AttributeHandleValuePairSet& attributeHandleArray,
                                          const RTI::FedTime& fedTime,
                                          const char* tag)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::InvalidFederationTime,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeValueVector attributeValueVector(attributeHandleArray);
    OpenRTI::_I13VariableLengthData _tag(tag);
    OpenRTI::MessageRetractionHandle messageRetractionHandle;
    messageRetractionHandle = privateRefs->updateAttributeValues(objectHandle, attributeValueVector, _tag, fedTime);
    return rti13MessageRetractionHandle(messageRetractionHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw RTI::AttributeNotOwned(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw RTI::InvalidFederationTime(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::updateAttributeValues(RTI::ObjectHandle objectHandle,
                                          const RTI::AttributeHandleValuePairSet& attributeHandleArray,
                                          const char* tag)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeValueVector attributeValueVector(attributeHandleArray);
    OpenRTI::_I13VariableLengthData _tag(tag);
    privateRefs->updateAttributeValues(objectHandle, attributeValueVector, _tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw RTI::AttributeNotOwned(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::EventRetractionHandle
RTI::RTIambassador::sendInteraction(RTI::InteractionClassHandle interactionClassHandle,
                                    const RTI::ParameterHandleValuePairSet& parameterHandleArray,
                                    const RTI::FedTime& fedTime,
                                    const char* tag)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InteractionParameterNotDefined,
         RTI::InvalidFederationTime,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::InteractionClassHandle OpenRTIInteractionClassHandle = interactionClassHandle;
    OpenRTI::_I13ParameterValueVector parameterValueVector(parameterHandleArray);
    OpenRTI::_I13VariableLengthData _tag(tag);
    // Note that the parametervaluevector is taken by the sendInteraction call. Thus on return it is empty
    OpenRTI::MessageRetractionHandle messageRetractionHandle;
    messageRetractionHandle = privateRefs->sendInteraction(OpenRTIInteractionClassHandle, parameterValueVector, _tag, fedTime);
    return rti13MessageRetractionHandle(messageRetractionHandle);
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw RTI::InteractionClassNotPublished(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw RTI::InteractionParameterNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw RTI::InvalidFederationTime(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::sendInteraction(RTI::InteractionClassHandle interactionClassHandle,
                                    const RTI::ParameterHandleValuePairSet& parameterHandleArray,
                                    const char* tag)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InteractionParameterNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::InteractionClassHandle OpenRTIInteractionClassHandle = interactionClassHandle;
    OpenRTI::_I13ParameterValueVector parameterValueVector(parameterHandleArray);
    OpenRTI::_I13VariableLengthData _tag(tag);
    // Note that the parametervaluevector is taken by the sendInteraction call. Thus on return it is empty
    privateRefs->sendInteraction(OpenRTIInteractionClassHandle, parameterValueVector, _tag);
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw RTI::InteractionClassNotPublished(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw RTI::InteractionParameterNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::EventRetractionHandle
RTI::RTIambassador::deleteObjectInstance(RTI::ObjectHandle objectHandle,
                                         const RTI::FedTime& fedTime,
                                         const char* tag)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::DeletePrivilegeNotHeld,
         RTI::InvalidFederationTime,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13VariableLengthData _tag(tag);
    OpenRTI::MessageRetractionHandle messageRetractionHandle;
    messageRetractionHandle = privateRefs->deleteObjectInstance(objectHandle, _tag, fedTime);
    return rti13MessageRetractionHandle(messageRetractionHandle);
  } catch (const OpenRTI::DeletePrivilegeNotHeld& e) {
    throw RTI::DeletePrivilegeNotHeld(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw RTI::InvalidFederationTime(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::deleteObjectInstance(RTI::ObjectHandle objectHandle,
                                         const char* tag)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::DeletePrivilegeNotHeld,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13VariableLengthData _tag(tag);
    privateRefs->deleteObjectInstance(objectHandle, _tag);
  } catch (const OpenRTI::DeletePrivilegeNotHeld& e) {
    throw RTI::DeletePrivilegeNotHeld(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::localDeleteObjectInstance(RTI::ObjectHandle objectHandle)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::FederateOwnsAttributes,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->localDeleteObjectInstance(objectHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw RTI::FederateOwnsAttributes(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::OwnershipAcquisitionPending& e) {
    throw RTI::OwnershipAcquisitionPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::changeAttributeTransportationType(RTI::ObjectHandle objectHandle,
                                                      const RTI::AttributeHandleSet& attributeHandleSet,
                                                      RTI::TransportationHandle transportationHandle)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::InvalidTransportationHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::TransportationType transportationType = toOpenRTITransportationType(transportationHandle);
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    privateRefs->changeAttributeTransportationType(objectHandle, attributeHandleVector, transportationType);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw RTI::AttributeNotOwned(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::changeInteractionTransportationType(RTI::InteractionClassHandle interactionClassHandle,
                                                        RTI::TransportationHandle transportationHandle)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InvalidTransportationHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::InteractionClassHandle OpenRTIInteractionClassHandle = interactionClassHandle;
    OpenRTI::TransportationType transportationType = toOpenRTITransportationType(transportationHandle);
    privateRefs->changeInteractionTransportationType(OpenRTIInteractionClassHandle, transportationType);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw RTI::InteractionClassNotPublished(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::requestObjectAttributeValueUpdate(RTI::ObjectHandle objectHandle,
                                                      const RTI::AttributeHandleSet& attributeHandleSet)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    OpenRTI::VariableLengthData tag;
    privateRefs->requestAttributeValueUpdate(OpenRTI::ObjectInstanceHandle(objectHandle), attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::requestClassAttributeValueUpdate(RTI::ObjectClassHandle objectClassHandle,
                                                     const RTI::AttributeHandleSet& attributeHandleSet)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    OpenRTI::VariableLengthData tag;
    privateRefs->requestAttributeValueUpdate(OpenRTI::ObjectClassHandle(objectClassHandle), attributeHandleVector, tag);
  } catch (const OpenRTI::ObjectClassNotDefined& e) {
    throw RTI::ObjectClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::unconditionalAttributeOwnershipDivestiture(RTI::ObjectHandle objectHandle,
                                                               const RTI::AttributeHandleSet& attributeHandleSet)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    privateRefs->unconditionalAttributeOwnershipDivestiture(objectHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw RTI::AttributeNotOwned(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::negotiatedAttributeOwnershipDivestiture(RTI::ObjectHandle objectHandle,
                                                            const RTI::AttributeHandleSet& attributeHandleSet,
                                                            const char* tag)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::AttributeAlreadyBeingDivested,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    OpenRTI::_I13VariableLengthData _tag(tag);
    privateRefs->negotiatedAttributeOwnershipDivestiture(objectHandle, attributeHandleVector, _tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw RTI::AttributeNotOwned(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeAlreadyBeingDivested& e) {
    throw RTI::AttributeAlreadyBeingDivested(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::attributeOwnershipAcquisition(RTI::ObjectHandle objectHandle,
                                                  const RTI::AttributeHandleSet& attributeHandleSet,
                                                  const char* tag)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::ObjectClassNotPublished,
         RTI::AttributeNotDefined,
         RTI::AttributeNotPublished,
         RTI::FederateOwnsAttributes,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    OpenRTI::_I13VariableLengthData _tag(tag);
    privateRefs->attributeOwnershipAcquisition(objectHandle, attributeHandleVector, _tag);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw RTI::ObjectClassNotPublished(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw RTI::AttributeNotPublished(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::attributeOwnershipAcquisitionIfAvailable(RTI::ObjectHandle objectHandle,
                                                             const AttributeHandleSet& attributeHandleSet)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::ObjectClassNotPublished,
         RTI::AttributeNotDefined,
         RTI::AttributeNotPublished,
         RTI::FederateOwnsAttributes,
         RTI::AttributeAlreadyBeingAcquired,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    privateRefs->attributeOwnershipAcquisitionIfAvailable(objectHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::ObjectClassNotPublished& e) {
    throw RTI::ObjectClassNotPublished(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotPublished& e) {
    throw RTI::AttributeNotPublished(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateOwnsAttributes& e) {
    throw RTI::FederateOwnsAttributes(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeAlreadyBeingAcquired& e) {
    throw RTI::AttributeAlreadyBeingAcquired(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::AttributeHandleSet*
RTI::RTIambassador::attributeOwnershipReleaseResponse(RTI::ObjectHandle objectHandle,
                                                      const RTI::AttributeHandleSet& attributeHandleSet)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::FederateWasNotAskedToReleaseAttribute,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    OpenRTI::AttributeHandleVector divestedAttributeHandleVector;
    privateRefs->attributeOwnershipDivestitureIfWanted(objectHandle, attributeHandleVector, divestedAttributeHandleVector);
    return new AttributeHandleSetCallback(divestedAttributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw RTI::AttributeNotOwned(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::cancelNegotiatedAttributeOwnershipDivestiture(RTI::ObjectHandle objectHandle,
                                                                  const RTI::AttributeHandleSet& attributeHandleSet)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::AttributeDivestitureWasNotRequested,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    privateRefs->cancelNegotiatedAttributeOwnershipDivestiture(objectHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw RTI::AttributeNotOwned(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeDivestitureWasNotRequested& e) {
    throw RTI::AttributeDivestitureWasNotRequested(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::cancelAttributeOwnershipAcquisition(RTI::ObjectHandle objectHandle,
                                                        const RTI::AttributeHandleSet& attributeHandleSet)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeAlreadyOwned,
         RTI::AttributeAcquisitionWasNotRequested,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    privateRefs->cancelAttributeOwnershipAcquisition(objectHandle, attributeHandleVector);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeAlreadyOwned& e) {
    throw RTI::AttributeAlreadyOwned(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeAcquisitionWasNotRequested& e) {
    throw RTI::AttributeAcquisitionWasNotRequested(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::queryAttributeOwnership(RTI::ObjectHandle objectHandle,
                                            RTI::AttributeHandle attributeHandle)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->queryAttributeOwnership(objectHandle, attributeHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::Boolean
RTI::RTIambassador::isAttributeOwnedByFederate(RTI::ObjectHandle objectHandle,
                                               RTI::AttributeHandle attributeHandle)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    bool owned = privateRefs->isAttributeOwnedByFederate(objectHandle, attributeHandle);
    return owned ? RTI::RTI_TRUE : RTI::RTI_FALSE;
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::enableTimeRegulation(const RTI::FedTime& time, const RTI::FedTime& lookahead)
  RTI_THROW ((RTI::TimeRegulationAlreadyEnabled,
         RTI::EnableTimeRegulationPending,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::InvalidFederationTime,
         RTI::InvalidLookahead,
         RTI::ConcurrentAccessAttempted,
         RTI::FederateNotExecutionMember,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->enableTimeRegulation(time, lookahead);
  } catch (const OpenRTI::TimeRegulationAlreadyEnabled& e) {
    throw RTI::TimeRegulationAlreadyEnabled(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InvalidLookahead& e) {
    throw RTI::InvalidLookahead(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw RTI::TimeAdvanceAlreadyInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw RTI::EnableTimeRegulationPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::disableTimeRegulation()
  RTI_THROW ((RTI::TimeRegulationWasNotEnabled,
         RTI::ConcurrentAccessAttempted,
         RTI::FederateNotExecutionMember,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->disableTimeRegulation();
  } catch (const OpenRTI::TimeRegulationIsNotEnabled& e) {
    throw RTI::TimeRegulationWasNotEnabled(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::enableTimeConstrained()
  RTI_THROW ((RTI::TimeConstrainedAlreadyEnabled,
         RTI::EnableTimeConstrainedPending,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->enableTimeConstrained();
  } catch (const OpenRTI::TimeConstrainedAlreadyEnabled& e) {
    throw RTI::TimeConstrainedAlreadyEnabled(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw RTI::TimeAdvanceAlreadyInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw RTI::EnableTimeConstrainedPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::disableTimeConstrained()
  RTI_THROW ((RTI::TimeConstrainedWasNotEnabled,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->disableTimeConstrained();
  } catch (const OpenRTI::TimeConstrainedIsNotEnabled& e) {
    throw RTI::TimeConstrainedWasNotEnabled(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::timeAdvanceRequest(const RTI::FedTime& fedTime)
  RTI_THROW ((RTI::InvalidFederationTime,
         RTI::FederationTimeAlreadyPassed,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::EnableTimeRegulationPending,
         RTI::EnableTimeConstrainedPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->timeAdvanceRequest(fedTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw RTI::InvalidFederationTime(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw RTI::FederationTimeAlreadyPassed(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw RTI::TimeAdvanceAlreadyInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw RTI::EnableTimeRegulationPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw RTI::EnableTimeConstrainedPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::timeAdvanceRequestAvailable(const RTI::FedTime& fedTime)
  RTI_THROW ((RTI::InvalidFederationTime,
         RTI::FederationTimeAlreadyPassed,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::EnableTimeRegulationPending,
         RTI::EnableTimeConstrainedPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->timeAdvanceRequestAvailable(fedTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw RTI::InvalidFederationTime(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw RTI::FederationTimeAlreadyPassed(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw RTI::TimeAdvanceAlreadyInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw RTI::EnableTimeRegulationPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw RTI::EnableTimeConstrainedPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::nextEventRequest(const RTI::FedTime& fedTime)
  RTI_THROW ((RTI::InvalidFederationTime,
         RTI::FederationTimeAlreadyPassed,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::EnableTimeRegulationPending,
         RTI::EnableTimeConstrainedPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->nextMessageRequest(fedTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw RTI::InvalidFederationTime(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw RTI::FederationTimeAlreadyPassed(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw RTI::TimeAdvanceAlreadyInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw RTI::EnableTimeRegulationPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw RTI::EnableTimeConstrainedPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::nextEventRequestAvailable(const RTI::FedTime& fedTime)
  RTI_THROW ((RTI::InvalidFederationTime,
         RTI::FederationTimeAlreadyPassed,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::EnableTimeRegulationPending,
         RTI::EnableTimeConstrainedPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->nextMessageRequestAvailable(fedTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw RTI::InvalidFederationTime(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw RTI::FederationTimeAlreadyPassed(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw RTI::TimeAdvanceAlreadyInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw RTI::EnableTimeRegulationPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw RTI::EnableTimeConstrainedPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::flushQueueRequest(const RTI::FedTime& fedTime)
  RTI_THROW ((RTI::InvalidFederationTime,
         RTI::FederationTimeAlreadyPassed,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::EnableTimeRegulationPending,
         RTI::EnableTimeConstrainedPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->flushQueueRequest(fedTime);
  } catch (const OpenRTI::InvalidLogicalTime& e) {
    throw RTI::InvalidFederationTime(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::LogicalTimeAlreadyPassed& e) {
    throw RTI::FederationTimeAlreadyPassed(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw RTI::TimeAdvanceAlreadyInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeRegulationPending& e) {
    throw RTI::EnableTimeRegulationPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RequestForTimeConstrainedPending& e) {
    throw RTI::EnableTimeConstrainedPending(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::enableAsynchronousDelivery()
  RTI_THROW ((RTI::AsynchronousDeliveryAlreadyEnabled,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->enableAsynchronousDelivery();
  } catch (const OpenRTI::AsynchronousDeliveryAlreadyEnabled& e) {
    throw RTI::AsynchronousDeliveryAlreadyEnabled(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::disableAsynchronousDelivery()
  RTI_THROW ((RTI::AsynchronousDeliveryAlreadyDisabled,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->disableAsynchronousDelivery();
  } catch (const OpenRTI::AsynchronousDeliveryAlreadyDisabled& e) {
    throw RTI::AsynchronousDeliveryAlreadyDisabled(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::queryLBTS(RTI::FedTime& fedTime)
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    if (privateRefs->queryGALT(fedTime))
      return;
    fedTime.setPositiveInfinity();
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::queryFederateTime(RTI::FedTime& fedTime)
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->queryLogicalTime(fedTime);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::queryMinNextEventTime(RTI::FedTime& fedTime)
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    if (privateRefs->queryLITS(fedTime))
      return;
    fedTime.setPositiveInfinity();
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::modifyLookahead(const RTI::FedTime& fedTime)
  RTI_THROW ((RTI::InvalidLookahead,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->modifyLookahead(fedTime, false/*checkForTimeRegulation*/);
  } catch (const OpenRTI::InvalidLookahead& e) {
    throw RTI::InvalidLookahead(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InTimeAdvancingState& e) {
    throw RTI::ConcurrentAccessAttempted(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::queryLookahead(RTI::FedTime& fedTime)
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->queryLookahead(fedTime, false/*checkForTimeRegulation*/);
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::retract(RTI::EventRetractionHandle theHandle)
  RTI_THROW ((RTI::InvalidRetractionHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->retract(toOpenRTIMessageRetractionHandle(theHandle));
  } catch (const OpenRTI::InvalidMessageRetractionHandle& e) {
    throw RTI::InvalidRetractionHandle(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::TimeRegulationIsNotEnabled&) {
  } catch (const OpenRTI::MessageCanNoLongerBeRetracted& e) {
    throw RTI::InvalidRetractionHandle(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::changeAttributeOrderType(RTI::ObjectHandle objectHandle,
                                             const RTI::AttributeHandleSet& attributeHandleSet,
                                             RTI::OrderingHandle orderingHandle)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::InvalidOrderingHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::OrderType orderType = toOpenRTIOrderType(orderingHandle);
    OpenRTI::_I13AttributeHandleVector attributeHandleVector(attributeHandleSet);
    privateRefs->changeAttributeOrderType(objectHandle, attributeHandleVector, orderType);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotOwned& e) {
    throw RTI::AttributeNotOwned(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::changeInteractionOrderType(RTI::InteractionClassHandle interactionClassHandle,
                                               RTI::OrderingHandle orderingHandle)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InvalidOrderingHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    OpenRTI::OrderType orderType = toOpenRTIOrderType(orderingHandle);
    privateRefs->changeInteractionOrderType(interactionClassHandle, orderType);
  } catch (const OpenRTI::InteractionClassNotDefined& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InteractionClassNotPublished& e) {
    throw RTI::InteractionClassNotPublished(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::Region*
RTI::RTIambassador::createRegion(RTI::SpaceHandle spaceHandle,
                                 RTI::ULong)
  RTI_THROW ((RTI::SpaceNotDefined,
         RTI::InvalidExtents,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::notifyAboutRegionModification(RTI::Region& theRegion)
  RTI_THROW ((RTI::RegionNotKnown,
         RTI::InvalidExtents,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::deleteRegion(RTI::Region* region)
  RTI_THROW ((RTI::RegionNotKnown,
         RTI::RegionInUse,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

RTI::ObjectHandle
RTI::RTIambassador::registerObjectInstanceWithRegion(RTI::ObjectClassHandle objectClassHandle,
                                                     const char* tag,
                                                     RTI::AttributeHandle [],
                                                     RTI::Region* theRegions[],
                                                     RTI::ULong)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotPublished,
         RTI::AttributeNotDefined,
         RTI::AttributeNotPublished,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::ObjectAlreadyRegistered,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

RTI::ObjectHandle
RTI::RTIambassador::registerObjectInstanceWithRegion(RTI::ObjectClassHandle objectClassHandle,
                                                     RTI::AttributeHandle [],
                                                     RTI::Region* regions[],
                                                     RTI::ULong)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotPublished,
         RTI::AttributeNotDefined,
         RTI::AttributeNotPublished,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::associateRegionForUpdates(RTI::Region& ,
                                              RTI::ObjectHandle,
                                              const RTI::AttributeHandleSet& attributeHandleSet)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::InvalidRegionContext,
         RTI::RegionNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::unassociateRegionForUpdates(RTI::Region&, RTI::ObjectHandle)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::InvalidRegionContext,
         RTI::RegionNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::subscribeObjectClassAttributesWithRegion(RTI::ObjectClassHandle objectClassHandle,
                                                             RTI::Region&,
                                                             const RTI::AttributeHandleSet&,
                                                             RTI::Boolean active)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::unsubscribeObjectClassWithRegion(RTI::ObjectClassHandle objectClassHandle,
                                                     RTI::Region&)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::RegionNotKnown,
         RTI::ObjectClassNotSubscribed,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::subscribeInteractionClassWithRegion(RTI::InteractionClassHandle interactionClassHandle,
                                                        RTI::Region&,
                                                        RTI::Boolean active)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::FederateLoggingServiceCalls,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::unsubscribeInteractionClassWithRegion(RTI::InteractionClassHandle interactionClassHandle,
                                                          RTI::Region&)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotSubscribed,
         RTI::RegionNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

RTI::EventRetractionHandle
RTI::RTIambassador::sendInteractionWithRegion(RTI::InteractionClassHandle interactionClassHandle,
                                              const RTI::ParameterHandleValuePairSet&,
                                              const RTI::FedTime&,
                                              const char* tag,
                                              const RTI::Region&)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InteractionParameterNotDefined,
         RTI::InvalidFederationTime,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::sendInteractionWithRegion(RTI::InteractionClassHandle interactionClassHandle,
                                              const RTI::ParameterHandleValuePairSet& ,
                                              const char* tag,
                                              const RTI::Region& )
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InteractionParameterNotDefined,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::requestClassAttributeValueUpdateWithRegion(RTI::ObjectClassHandle objectClassHandle,
                                                               const RTI::AttributeHandleSet&,
                                                               const RTI::Region&)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::RegionNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

RTI::ObjectClassHandle
RTI::RTIambassador::getObjectClassHandle(const char* _name)
  RTI_THROW ((RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!_name)
    throw RTI::NameNotFound("Zero name pointer.");
  try {
    std::string name = OpenRTI::localeToUtf8(_name);
    // special casing for the root object
    if (10 <= name.size() && OpenRTI::caseCompare(name.substr(0, 10), "ObjectRoot"))
      name.replace(0, 10, "HLAobjectRoot");
    return privateRefs->getObjectClassHandle(name);
  } catch (const OpenRTI::NameNotFound& e) {
    throw RTI::NameNotFound(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

char*
RTI::RTIambassador::getObjectClassName(RTI::ObjectClassHandle objectClassHandle)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    std::string name = privateRefs->getObjectClassName(objectClassHandle);
    // special casing for the privilege to delete
    if (13 <= name.size() && name.compare(0, 13, "HLAobjectRoot") == 0)
      name.replace(0, 13, "ObjectRoot");
    return newUtf8ToLocale(name);
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw RTI::ObjectClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::AttributeHandle
RTI::RTIambassador::getAttributeHandle(const char* _name,
                                       RTI::ObjectClassHandle objectClassHandle)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!_name)
    throw RTI::NameNotFound("Zero name pointer.");
  try {
    std::string name = OpenRTI::localeToUtf8(_name);
    // special casing for the root object
    if (10 <= name.size() && OpenRTI::caseCompare(name.substr(0, 10), "ObjectRoot"))
      name.replace(0, 10, "HLAobjectRoot");
    if (17 <= name.size() && OpenRTI::caseCompare(name.substr(name.size() - 17, 17), "privilegeToDelete"))
      name.replace(name.size() - 17, 17, "HLAprivilegeToDeleteObject");
    return privateRefs->getAttributeHandle(objectClassHandle, name);
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw RTI::ObjectClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NameNotFound& e) {
    throw RTI::NameNotFound(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

char*
RTI::RTIambassador::getAttributeName(RTI::AttributeHandle attributeHandle,
                                     RTI::ObjectClassHandle objectClassHandle)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    std::string name = privateRefs->getAttributeName(objectClassHandle, attributeHandle);
    // special casing for the privilege to delete
    if (25 <= name.size() && name.compare(name.size() - 25, 25, "HLAprivilegeToDeleteObject") == 0)
      name.replace(name.size() - 25, 25, "privilegeToDelete");
    if (13 <= name.size() && name.compare(0, 13, "HLAobjectRoot") == 0)
      name.replace(0, 13, "ObjectRoot");
    return newUtf8ToLocale(name);
  } catch (const OpenRTI::InvalidObjectClassHandle& e) {
    throw RTI::ObjectClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InvalidAttributeHandle& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::AttributeNotDefined& e) {
    throw RTI::AttributeNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::InteractionClassHandle
RTI::RTIambassador::getInteractionClassHandle(const char* _name)
  RTI_THROW ((RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!_name)
    throw RTI::NameNotFound("Zero name pointer.");
  try {
    std::string name = OpenRTI::localeToUtf8(_name);
    // special casing for the root object
    if (15 <= name.size() && OpenRTI::caseCompare(name.substr(0, 15), "InteractionRoot"))
      name.replace(0, 15, "HLAinteractionRoot");
    return privateRefs->getInteractionClassHandle(name);
  } catch (const OpenRTI::NameNotFound& e) {
    throw RTI::NameNotFound(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

char*
RTI::RTIambassador::getInteractionClassName(RTI::InteractionClassHandle interactionClassHandle)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    std::string name = privateRefs->getInteractionClassName(interactionClassHandle);
    // special casing for the root object
    if (18 <= name.size() && name.compare(0, 18, "HLAinteractionRoot") == 0)
      name.replace(0, 18, "InteractionRoot");
    return newUtf8ToLocale(name);
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::ParameterHandle
RTI::RTIambassador::getParameterHandle(const char* _name,
                                       RTI::InteractionClassHandle interactionClassHandle)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!_name)
    throw RTI::NameNotFound("Zero name pointer.");
  try {
    std::string name = OpenRTI::localeToUtf8(_name);
    // special casing for the root object
    if (15 <= name.size() && OpenRTI::caseCompare(name.substr(0, 15), "InteractionRoot"))
      name.replace(0, 15, "HLAinteractionRoot");
    return privateRefs->getParameterHandle(interactionClassHandle, name);
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NameNotFound& e) {
    throw RTI::NameNotFound(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

char*
RTI::RTIambassador::getParameterName(RTI::ParameterHandle parameterHandle,
                                     RTI::InteractionClassHandle interactionClassHandle)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::InteractionParameterNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    std::string name = privateRefs->getParameterName(interactionClassHandle, parameterHandle);
    // special casing for the root object
    if (18 <= name.size() && name.compare(0, 18, "HLAinteractionRoot") == 0)
      name.replace(0, 18, "InteractionRoot");
    return newUtf8ToLocale(name);
  } catch (const OpenRTI::InvalidInteractionClassHandle& e) {
    throw RTI::InteractionClassNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InvalidParameterHandle& e) {
    throw RTI::InteractionParameterNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::InteractionParameterNotDefined& e) {
    throw RTI::InteractionParameterNotDefined(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::ObjectHandle
RTI::RTIambassador::getObjectInstanceHandle(const char* name)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  try {
    return privateRefs->getObjectInstanceHandle(OpenRTI::localeToUtf8(name));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

char*
RTI::RTIambassador::getObjectInstanceName(RTI::ObjectHandle objectHandle)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    return newUtf8ToLocale(privateRefs->getObjectInstanceName(objectHandle));
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::SpaceHandle
RTI::RTIambassador::getRoutingSpaceHandle(const char* name)
  RTI_THROW ((RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  throw RTI::RTIinternalError("ENOIMP");
}

char*
RTI::RTIambassador::getRoutingSpaceName(RTI::SpaceHandle spaceHandle)
  RTI_THROW ((RTI::SpaceNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("ENOIMP");
}

RTI::DimensionHandle
RTI::RTIambassador::getDimensionHandle(const char* name,
                                       RTI::SpaceHandle spaceHandle)
  RTI_THROW ((RTI::SpaceNotDefined,
         RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  throw RTI::RTIinternalError("ENOIMP");
}

char*
RTI::RTIambassador::getDimensionName(RTI::DimensionHandle dimensionHandle,
                                     RTI::SpaceHandle spaceHandle)
  RTI_THROW ((RTI::SpaceNotDefined,
         RTI::DimensionNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("ENOIMP");
}

RTI::SpaceHandle
RTI::RTIambassador::getAttributeRoutingSpaceHandle(RTI::AttributeHandle attributeHandle,
                                                   RTI::ObjectClassHandle objectClassHandle)
  RTI_THROW ((RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("ENOIMP");
}

RTI::ObjectClassHandle
RTI::RTIambassador::getObjectClass(RTI::ObjectHandle objectHandle)
  RTI_THROW ((RTI::ObjectNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    return privateRefs->getKnownObjectClassHandle(objectHandle);
  } catch (const OpenRTI::ObjectInstanceNotKnown& e) {
    throw RTI::ObjectNotKnown(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::SpaceHandle
RTI::RTIambassador::getInteractionRoutingSpaceHandle(InteractionClassHandle interactionClassHandle)
  RTI_THROW ((RTI::InteractionClassNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("ENOIMP");
}

RTI::TransportationHandle
RTI::RTIambassador::getTransportationHandle(const char* name)
  RTI_THROW ((RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  try {
    if (OpenRTI::caseCompare(name, "reliable"))
      return rti13TransportType(privateRefs->getTransportationType("HLAreliable"));
    if (OpenRTI::caseCompare(name, "best_effort"))
      return rti13TransportType(privateRefs->getTransportationType("HLAbestEffort"));
    return rti13TransportType(privateRefs->getTransportationType(OpenRTI::localeToUtf8(name)));
  } catch (const OpenRTI::InvalidTransportationName& e) {
    throw RTI::NameNotFound(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

char*
RTI::RTIambassador::getTransportationName(RTI::TransportationHandle transportationHandle)
  RTI_THROW ((RTI::InvalidTransportationHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    std::string name = privateRefs->getTransportationName(toOpenRTITransportationType(transportationHandle));
    // special casing for the builtin types, the rest just maps through
    if (name == "HLAreliable")
      return newUtf8ToLocale("reliable");
    if (name == "HLAbestEffort")
      return newUtf8ToLocale("best_effort");
    return newUtf8ToLocale(name);
  } catch (const OpenRTI::InvalidTransportationType& e) {
    throw RTI::InvalidTransportationHandle(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::OrderingHandle
RTI::RTIambassador::getOrderingHandle(const char* name)
  RTI_THROW ((RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  try {
    if (OpenRTI::caseCompare(name, "timestamp"))
      return rti13OrderType(OpenRTI::TIMESTAMP);
    if (OpenRTI::caseCompare(name, "receive"))
      return rti13OrderType(OpenRTI::RECEIVE);
    throw RTI::NameNotFound(name);
  } catch (const OpenRTI::InvalidOrderName& e) {
    throw RTI::NameNotFound(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

char*
RTI::RTIambassador::getOrderingName(RTI::OrderingHandle orderingHandle)
  RTI_THROW ((RTI::InvalidOrderingHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    switch(toOpenRTIOrderType(orderingHandle)) {
    case OpenRTI::TIMESTAMP:
      return newUtf8ToLocale("timestamp");
    case OpenRTI::RECEIVE:
      return newUtf8ToLocale("receive");
    default:
      throw OpenRTI::InvalidOrderType();
    }
  } catch (const OpenRTI::InvalidOrderType& e) {
    throw RTI::InvalidOrderingHandle(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::enableClassRelevanceAdvisorySwitch()
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->enableObjectClassRelevanceAdvisorySwitch();
  } catch (const OpenRTI::ObjectClassRelevanceAdvisorySwitchIsOn&) {
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::disableClassRelevanceAdvisorySwitch()
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->disableObjectClassRelevanceAdvisorySwitch();
  } catch (const OpenRTI::ObjectClassRelevanceAdvisorySwitchIsOff&) {
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::enableAttributeRelevanceAdvisorySwitch()
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->enableAttributeRelevanceAdvisorySwitch();
  } catch (const OpenRTI::AttributeRelevanceAdvisorySwitchIsOn&) {
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::disableAttributeRelevanceAdvisorySwitch()
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->disableAttributeRelevanceAdvisorySwitch();
  } catch (const OpenRTI::AttributeRelevanceAdvisorySwitchIsOff&) {
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::enableAttributeScopeAdvisorySwitch()
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->enableAttributeScopeAdvisorySwitch();
  } catch (const OpenRTI::AttributeScopeAdvisorySwitchIsOn&) {
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::disableAttributeScopeAdvisorySwitch()
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->disableAttributeScopeAdvisorySwitch();
  } catch (const OpenRTI::AttributeScopeAdvisorySwitchIsOff&) {
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::enableInteractionRelevanceAdvisorySwitch()
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->enableInteractionRelevanceAdvisorySwitch();
  } catch (const OpenRTI::InteractionRelevanceAdvisorySwitchIsOn&) {
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

void
RTI::RTIambassador::disableInteractionRelevanceAdvisorySwitch()
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    privateRefs->disableInteractionRelevanceAdvisorySwitch();
  } catch (const OpenRTI::InteractionRelevanceAdvisorySwitchIsOff&) {
  } catch (const OpenRTI::FederateNotExecutionMember& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::SaveInProgress& e) {
    throw RTI::SaveInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::RestoreInProgress& e) {
    throw RTI::RestoreInProgress(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const OpenRTI::NotConnected& e) {
    throw RTI::FederateNotExecutionMember(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::Boolean
RTI::RTIambassador::tick()
  RTI_THROW ((RTI::SpecifiedSaveLabelDoesNotExist,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    bool pendingCallback = privateRefs->evokeMultipleCallbacks(0, std::numeric_limits<double>::infinity());
    return pendingCallback ? RTI::RTI_TRUE : RTI::RTI_FALSE;
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::Boolean
RTI::RTIambassador::tick(RTI::TickTime minTime,
                         RTI::TickTime maxTime)
  RTI_THROW ((RTI::SpecifiedSaveLabelDoesNotExist,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  try {
    bool pendingCallback = privateRefs->evokeMultipleCallbacks(minTime, maxTime);
    return pendingCallback ? RTI::RTI_TRUE : RTI::RTI_FALSE;
  } catch (const std::exception& e) {
    throw RTI::RTIinternalError(OpenRTI::utf8ToLocale(e.what()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown internal error!");
  }
}

RTI::RegionToken
RTI::RTIambassador::getRegionToken(RTI::Region*)
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RegionNotKnown,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("ENOIMP");
}

RTI::Region*
RTI::RTIambassador::getRegion(RTI::RegionToken)
  RTI_THROW ((RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RegionNotKnown,
         RTI::RTIinternalError))
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("ENOIMP");
}
