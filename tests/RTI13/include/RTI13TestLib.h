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

#ifndef OpenRTI_RTI13TestLib_h
#define OpenRTI_RTI13TestLib_h

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <RTI.hh>
#include <fedtime.hh>

#include <TestLib.h>

namespace OpenRTI {

// inline rti1516::VariableLengthData
// toVariableLengthData(const char* s)
// {
//     rti1516::VariableLengthData variableLengthData;
//     if (s)
//         variableLengthData.setData(s, strlen(s));
//     return variableLengthData;
// }

// inline rti1516::VariableLengthData
// toVariableLengthData(const std::string& s)
// {
//     rti1516::VariableLengthData variableLengthData;
//     variableLengthData.setData(s.data(), s.size());
//     return variableLengthData;
// }

// inline std::string
// toString(const rti1516::VariableLengthData& variableLengthData)
// {
//     if (!variableLengthData.size())
//         return std::string();
//     return std::string((const char*)variableLengthData.data(), variableLengthData.size());
// }

// inline rti1516::VariableLengthData
// toVariableLengthData(const std::string& s)
// {
//     rti1516::VariableLengthData variableLengthData;
//     variableLengthData.setData(s.data(), sizeof(std::string::value_type)*s.size());
//     return variableLengthData;
// }

// inline std::string
// toString(const rti1516::VariableLengthData& variableLengthData)
// {
//     if (!variableLengthData.size())
//         return std::string();
//     return std::string((const wchar_t*)variableLengthData.data(), variableLengthData.size()/sizeof(std::string::value_type));
// }

// inline rti1516::VariableLengthData
// toVariableLengthData(const Clock& c)
// {
//     // May be at some time make this endian safe
//     rti1516::VariableLengthData variableLengthData;
//     variableLengthData.setData(&c, sizeof(c));
//     return variableLengthData;
// }

// inline Clock
// toClock(const rti1516::VariableLengthData& variableLengthData)
// {
//     Clock c;
//     // May be at some time make this endian safe
//     if (variableLengthData.size() == sizeof(Clock))
//         memcpy(&c, variableLengthData.data(), sizeof(Clock));
//     return c;
// }

// inline rti1516::VariableLengthData
// toVariableLengthData(unsigned u)
// {
//     // May be at some time make this endian safe
//     rti1516::VariableLengthData variableLengthData;
//     variableLengthData.setData(&u, sizeof(u));
//     return variableLengthData;
// }

// inline unsigned
// toUnsigned(const rti1516::VariableLengthData& variableLengthData)
// {
//     unsigned u = -1;
//     // May be at some time make this endian safe
//     if (variableLengthData.size() == sizeof(unsigned))
//         memcpy(&u, variableLengthData.data(), sizeof(unsigned));
//     return u;
// }

class OPENRTI_LOCAL RTI13TestAmbassador : public RTITest::Ambassador, public RTI::FederateAmbassador {
public:
  RTI13TestAmbassador(const RTITest::ConstructorArgs& constructorArgs) :
    RTITest::Ambassador(constructorArgs),
    _synchronized(0)
  { }
  virtual ~RTI13TestAmbassador()
    RTI_NOEXCEPT
  { }

  virtual bool execJoined(RTI::RTIambassador& ambassador) = 0;

  bool waitForAllFederates(RTI::RTIambassador& ambassador)
  {
    _synchronized = 0;

    // FIXME need a test for concurrent announces
    try {
      ambassador.registerFederationSynchronizationPoint(ucsToLocale(getFederateType()).c_str(), "");
    } catch (const RTI::Exception& e) {
      std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
      return false;
    } catch (...) {
      std::cout << "Unknown Exception!" << std::endl;
      return false;
    }

    try {
      Clock timeout = Clock::now() + Clock::fromSeconds(10);
      while (!_federateSet.empty()) {
        if (ambassador.tick(10.0, 0))
          continue;
        if (timeout < Clock::now()) {
          std::cout << "Timeout waiting for other federates to join!" << std::endl;
          return false;
        }
      }
    } catch (const RTI::Exception& e) {
      std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
      return false;
    } catch (...) {
      std::cout << "Unknown Exception!" << std::endl;
      return false;
    }

    // Fill for the next time
    for (std::vector<std::wstring>::const_iterator i = getFederateList().begin();
         i != getFederateList().end(); ++i)
      _federateSet.insert(ucsToLocale(*i));

    try {
      for (std::vector<std::wstring>::const_iterator i = getFederateList().begin(); i != getFederateList().end(); ++i) {
        ambassador.synchronizationPointAchieved(ucsToLocale(*i).c_str());
      }
    } catch (const RTI::Exception& e) {
      std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
      return false;
    } catch (...) {
      std::cout << "Unknown Exception!" << std::endl;
      return false;
    }

    try {
      Clock timeout = Clock::now() + Clock::fromSeconds(10);
      while (_synchronized < getFederateList().size()) {
        if (ambassador.tick(10.0, 0))
          continue;
        if (timeout < Clock::now()) {
          std::cout << "Timeout waiting for other federates to synchronize!" << std::endl;
          return false;
        }
      }
    } catch (const RTI::Exception& e) {
      std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
      return false;
    } catch (...) {
      std::cout << "Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  virtual bool execJoinOnce()
  {
    RTI_UNIQUE_PTR<RTI::RTIambassador> ambassador;
    ambassador.reset(new RTI::RTIambassador);

    // create, must work once
    try {
      ambassador->createFederationExecution(ucsToLocale(getFederationExecution()).c_str(), ucsToLocale(getFddFile()).c_str());

      if (!getFederationBarrier()->success())
        return false;
    } catch (const RTI::FederationExecutionAlreadyExists&) {
      // Can happen in this test

      if (!getFederationBarrier()->fail())
        return false;
    } catch (const RTI::Exception& e) {
      std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
      return false;
    } catch (...) {
      std::cout << "Unknown Exception!" << std::endl;
      return false;
    }

    // Try that several times. Ensure correct cleanup
    unsigned n = 99;
    for (unsigned i = 0; i < n; ++i) {

      // join must work
      try {
        ambassador->joinFederationExecution(ucsToLocale(getFederateType()).c_str(), ucsToLocale(getFederationExecution()).c_str(), this);
      } catch (const RTI::Exception& e) {
        std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
        return false;
      } catch (...) {
        std::cout << "Unknown Exception!" << std::endl;
        return false;
      }

      for (std::vector<std::wstring>::const_iterator i = getFederateList().begin();
           i != getFederateList().end(); ++i)
        _federateSet.insert(ucsToLocale(*i));

      if (!execJoined(*ambassador))
        return false;

      // and now resign must work
      try {
        ambassador->resignFederationExecution(RTI::DELETE_OBJECTS_AND_RELEASE_ATTRIBUTES);
      } catch (const RTI::Exception& e) {
        std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
        return false;
      } catch (...) {
        std::cout << "Unknown Exception!" << std::endl;
        return false;
      }

    }

    // Wait for all threads, this is to ensure that we do not destroy before we are ready
    wait();

    // destroy, must work once
    try {
      ambassador->destroyFederationExecution(ucsToLocale(getFederationExecution()).c_str());

      if (!getFederationBarrier()->success())
        return false;
    } catch (const RTI::FederatesCurrentlyJoined&) {
      // Can happen in this test
      // Other threads just might have still joined.

      if (!getFederationBarrier()->fail())
        return false;
    } catch (const RTI::FederationExecutionDoesNotExist&) {
      // Can happen in this test
      // Other threads might have been faster

      if (!getFederationBarrier()->fail())
        return false;
    } catch (const RTI::Exception& e) {
      std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
      return false;
    } catch (...) {
      std::cout << "Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  virtual bool execJoinMultiple()
  {
    RTI_UNIQUE_PTR<RTI::RTIambassador> ambassador;
    ambassador.reset(new RTI::RTIambassador);

    // Try that several times. Ensure correct cleanup
    unsigned n = 99;
    for (unsigned i = 0; i < n; ++i) {

      // create, must work once
      try {
        ambassador->createFederationExecution(ucsToLocale(getFederationExecution()).c_str(), ucsToLocale(getFddFile()).c_str());

        if (!getFederationBarrier()->success())
          return false;
      } catch (const RTI::FederationExecutionAlreadyExists&) {
        // Can happen in this test

        if (!getFederationBarrier()->fail())
          return false;
      } catch (const RTI::Exception& e) {
        std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
        return false;
      } catch (...) {
        std::cout << "Unknown Exception!" << std::endl;
        return false;
      }

      // join must work
      try {
        ambassador->joinFederationExecution(ucsToLocale(getFederateType()).c_str(), ucsToLocale(getFederationExecution()).c_str(), this);
      } catch (const RTI::Exception& e) {
        std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
        return false;
      } catch (...) {
        std::cout << "Unknown Exception!" << std::endl;
        return false;
      }

      for (std::vector<std::wstring>::const_iterator i = getFederateList().begin();
           i != getFederateList().end(); ++i)
        _federateSet.insert(ucsToLocale(*i));

      if (!execJoined(*ambassador))
        return false;

      // and now resign must work
      try {
        ambassador->resignFederationExecution(RTI::DELETE_OBJECTS_AND_RELEASE_ATTRIBUTES);
      } catch (const RTI::Exception& e) {
        std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
        return false;
      } catch (...) {
        std::cout << "Unknown Exception!" << std::endl;
        return false;
      }

      // Wait for all threads, this is to ensure that we do not destroy before we are ready
      wait();

      // destroy, must work once
      try {
        ambassador->destroyFederationExecution(ucsToLocale(getFederationExecution()).c_str());

        if (!getFederationBarrier()->success())
          return false;
      } catch (const RTI::FederatesCurrentlyJoined&) {
        // Can happen in this test
        // Other threads just might have still joined.

        if (!getFederationBarrier()->fail())
          return false;
      } catch (const RTI::FederationExecutionDoesNotExist&) {
        // Can happen in this test
        // Other threads might have been faster

        if (!getFederationBarrier()->fail())
          return false;
      } catch (const RTI::Exception& e) {
        std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
        return false;
      } catch (...) {
        std::cout << "Unknown Exception!" << std::endl;
        return false;
      }
    }

    return true;
  }

  virtual bool exec()
  {
    if (_constructorArgs._joinOnce)
      return execJoinOnce();
    else
      return execJoinMultiple();
  }

  virtual void synchronizationPointRegistrationSucceeded(const char* label)
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void synchronizationPointRegistrationFailed(const char* label)
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void announceSynchronizationPoint(const char* label, const char* tag)
    RTI_THROW ((RTI::FederateInternalError))
  {
    _federateSet.erase(label);
  }

  virtual void federationSynchronized(const char* label)
    RTI_THROW ((RTI::FederateInternalError))
  {
    ++_synchronized;
  }

  virtual void initiateFederateSave(const char* label)
      RTI_THROW ((RTI::UnableToPerformSave,
             RTI::FederateInternalError))
  {
  }

  virtual void federationSaved()
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void federationNotSaved()
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void requestFederationRestoreSucceeded(const char* label)
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void requestFederationRestoreFailed(const char* label, const char *)
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void federationRestoreBegun()
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void initiateFederateRestore(const char* label, RTI::FederateHandle handle)
    RTI_THROW ((RTI::SpecifiedSaveLabelDoesNotExist,
           RTI::CouldNotRestore,
           RTI::FederateInternalError))
  {
  }

  virtual void federationRestored()
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void federationNotRestored()
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void startRegistrationForObjectClass(RTI::ObjectClassHandle)
    RTI_THROW ((RTI::ObjectClassNotPublished,
           RTI::FederateInternalError))
  {
  }

  virtual void stopRegistrationForObjectClass(RTI::ObjectClassHandle)
    RTI_THROW ((RTI::ObjectClassNotPublished,
           RTI::FederateInternalError))
  {
  }

  virtual void turnInteractionsOn(RTI::InteractionClassHandle)
    RTI_THROW ((RTI::InteractionClassNotPublished,
           RTI::FederateInternalError))
  {
  }

  virtual void turnInteractionsOff(RTI::InteractionClassHandle)
    RTI_THROW ((RTI::InteractionClassNotPublished,
           RTI::FederateInternalError))
  {
  }

  virtual void discoverObjectInstance(RTI::ObjectHandle, RTI::ObjectClassHandle, const char*)
    RTI_THROW ((RTI::CouldNotDiscover,
           RTI::ObjectClassNotKnown,
           RTI::FederateInternalError))
  {
  }

  virtual void reflectAttributeValues(RTI::ObjectHandle, const RTI::AttributeHandleValuePairSet &,
                                      const RTI::FedTime &, const char *, RTI::EventRetractionHandle)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotKnown,
           RTI::FederateOwnsAttributes,
           RTI::InvalidFederationTime,
           RTI::FederateInternalError))
  {
      // _verifyReflectAttributeValues(objectHandle, attributeHandleValueMap);
  }

  virtual void reflectAttributeValues(RTI::ObjectHandle, const RTI::AttributeHandleValuePairSet &, const char *)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotKnown,
           RTI::FederateOwnsAttributes,
           RTI::FederateInternalError))
  {
      // _verifyReflectAttributeValues(objectHandle, attributeHandleValueMap);
  }

  virtual void receiveInteraction(RTI::InteractionClassHandle, const RTI::ParameterHandleValuePairSet &,
                                  const RTI::FedTime &, const char *, RTI::EventRetractionHandle)
    RTI_THROW ((RTI::InteractionClassNotKnown,
           RTI::InteractionParameterNotKnown,
           RTI::InvalidFederationTime,
           RTI::FederateInternalError))
  {
  }

  virtual void receiveInteraction(RTI::InteractionClassHandle, const RTI::ParameterHandleValuePairSet &,
                                  const char *)
    RTI_THROW ((RTI::InteractionClassNotKnown,
           RTI::InteractionParameterNotKnown,
           RTI::FederateInternalError))
  {
  }

  virtual void removeObjectInstance(RTI::ObjectHandle, const RTI::FedTime &, const char *,
                                    RTI::EventRetractionHandle)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::InvalidFederationTime,
           RTI::FederateInternalError))
  {
  }

  virtual void removeObjectInstance(RTI::ObjectHandle, const char *)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::FederateInternalError))
  {
  }

  virtual void attributesInScope(RTI::ObjectHandle, const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotKnown,
           RTI::FederateInternalError))
  {
  }

  virtual void attributesOutOfScope(RTI::ObjectHandle, const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotKnown,
           RTI::FederateInternalError))
  {
  }

  virtual void provideAttributeValueUpdate(RTI::ObjectHandle theObject,
                                           RTI::AttributeHandleSet const & theAttributes)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotKnown,
           RTI::AttributeNotOwned,
           RTI::FederateInternalError))
  {
  }

  virtual void turnUpdatesOnForObjectInstance(RTI::ObjectHandle theObject,
                                              RTI::AttributeHandleSet const & theAttributes)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotOwned,
           RTI::FederateInternalError))
  {
  }

  virtual void turnUpdatesOffForObjectInstance(RTI::ObjectHandle theObject,
                                               RTI::AttributeHandleSet const & theAttributes)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotOwned,
           RTI::FederateInternalError))
  {
  }

  virtual void requestAttributeOwnershipAssumption(RTI::ObjectHandle, const RTI::AttributeHandleSet &,
                                                   const char *)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAlreadyOwned,
           RTI::AttributeNotPublished, RTI::FederateInternalError)) { }

  virtual void attributeOwnershipDivestitureNotification(RTI::ObjectHandle, const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned,
           RTI::AttributeDivestitureWasNotRequested, RTI::FederateInternalError)) { }

  virtual void attributeOwnershipAcquisitionNotification(RTI::ObjectHandle, const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAcquisitionWasNotRequested,
           RTI::AttributeAlreadyOwned, RTI::AttributeNotPublished, RTI::FederateInternalError)) { }

  virtual void attributeOwnershipUnavailable(RTI::ObjectHandle, const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAlreadyOwned,
           RTI::AttributeAcquisitionWasNotRequested, RTI::FederateInternalError)) { }

  virtual void requestAttributeOwnershipRelease(RTI::ObjectHandle, const RTI::AttributeHandleSet &,
                                                const char *)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError)) { }

  virtual void confirmAttributeOwnershipAcquisitionCancellation(RTI::ObjectHandle,
                                                                const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAlreadyOwned,
           RTI::AttributeAcquisitionWasNotCanceled, RTI::FederateInternalError)) { }

  virtual void informAttributeOwnership(RTI::ObjectHandle, RTI::AttributeHandle, RTI::FederateHandle)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError)) { }

  virtual void attributeIsNotOwned(RTI::ObjectHandle, RTI::AttributeHandle)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError)) { }

  virtual void attributeOwnedByRTI(RTI::ObjectHandle, RTI::AttributeHandle)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError)) { }

  virtual void timeRegulationEnabled(RTI::FedTime const & theFederateTime)
    RTI_THROW ((RTI::InvalidFederationTime,
           RTI::EnableTimeRegulationWasNotPending,
           RTI::FederateInternalError))
  {
  }

  virtual void timeConstrainedEnabled(RTI::FedTime const & theFederateTime)
    RTI_THROW ((RTI::InvalidFederationTime,
           RTI::EnableTimeConstrainedWasNotPending,
           RTI::FederateInternalError))
  {
  }

  virtual void timeAdvanceGrant(RTI::FedTime const & theTime)
    RTI_THROW ((RTI::InvalidFederationTime,
           RTI::TimeAdvanceWasNotInProgress,
           RTI::FederateInternalError))
  {
  }

  virtual void requestRetraction(RTI::EventRetractionHandle theHandle)
    RTI_THROW ((RTI::EventNotKnown,
           RTI::FederateInternalError))
  {
  }

private:
  unsigned _synchronized;
  std::set<std::string> _federateSet;
};

class OPENRTI_LOCAL RTI13SimpleAmbassador : public RTI::FederateAmbassador {
public:
  RTI13SimpleAmbassador() :
    // _fail(false),
    _useDataUrlObjectModels(false),
    _timeRegulationEnabled(false),
    _timeConstrainedEnabled(false),
    _timeAdvancePending(false)
  { }
  virtual ~RTI13SimpleAmbassador()
    RTI_NOEXCEPT
  { }

  // bool getFail() const
  // { return _fail; }

  void setUseDataUrlObjectModels(bool useDataUrlObjectModels)
  { _useDataUrlObjectModels = useDataUrlObjectModels; }
  bool getUseDataUrlObjectModels() const
  { return _useDataUrlObjectModels; }

  bool getTimeRegulationEnabled() const
  { return _timeRegulationEnabled; }
  bool getTimeConstrainedEnabled() const
  { return _timeConstrainedEnabled; }
  bool getTimeAdvancePending() const
  { return _timeAdvancePending; }

  const RTI::FederateHandle& getFederateHandle() const
  { return _federateHandle; }

  void connect(std::vector<std::string> args)
  {
    _ambassador.reset(new RTI::RTIambassador);
  }

  void createFederationExecution(const std::string& federationExecutionName, std::string fddFile)
  {
    _replaceFileWithDataIfNeeded(fddFile);
    _ambassador->createFederationExecution(federationExecutionName.c_str(), fddFile.c_str());
  }

  void destroyFederationExecution(const std::string& federationExecutionName)
  {
    _ambassador->destroyFederationExecution(federationExecutionName.c_str());
  }

  const RTI::FederateHandle& joinFederationExecution(const std::string& federateType,
                                                         const std::string& federationExecutionName)
  {
    _federateHandle = _ambassador->joinFederationExecution(federateType.c_str(), federationExecutionName.c_str(), this);
    _grantedFedTime.reset(new RTIfedTime);
    _grantedFedTime->setZero();
    return _federateHandle;
  }

  void resignFederationExecution(RTI::ResignAction resignAction)
  {
    _ambassador->resignFederationExecution(resignAction);
    _federateHandle = RTI::FederateHandle();
  }

  void registerFederationSynchronizationPoint(const std::string& label, const std::string& tag)
  {
    _ambassador->registerFederationSynchronizationPoint(label.c_str(), tag.c_str());
  }

  void registerFederationSynchronizationPoint(const std::string& label, const std::string& tag,
                                              const RTI::FederateHandleSet& federateHandleSet)
  {
    _ambassador->registerFederationSynchronizationPoint(label.c_str(), tag.c_str(), federateHandleSet);
  }

  void synchronizationPointAchieved(const std::string& label)
  {
    _ambassador->synchronizationPointAchieved(label.c_str());
  }

  void requestFederationSave(const std::string& label)
  {
    _ambassador->requestFederationSave(label.c_str());
  }

  void requestFederationSave(const std::string& label, const RTI::FedTime& logicalTime)
  {
    _ambassador->requestFederationSave(label.c_str(), logicalTime);
  }

  void federateSaveComplete()
  {
    _ambassador->federateSaveComplete();
  }

  void federateSaveNotComplete()
  {
    _ambassador->federateSaveNotComplete();
  }

  // void queryFederationSaveStatus()
  // {
  //   _ambassador->queryFederationSaveStatus();
  // }

  void requestFederationRestore(const std::string& label)
  {
    _ambassador->requestFederationRestore(label.c_str());
  }

  void federateRestoreComplete()
  {
    _ambassador->federateRestoreComplete();
  }

  void federateRestoreNotComplete()
  {
    _ambassador->federateRestoreNotComplete();
  }

  // void queryFederationRestoreStatus()
  // {
  //   _ambassador->queryFederationRestoreStatus();
  // }

  void publishObjectClass(const RTI::ObjectClassHandle& objectClassHandle,
                          const RTI::AttributeHandleSet& attributeList)
  {
    _ambassador->publishObjectClass(objectClassHandle, attributeList);
  }

  void unpublishObjectClass(const RTI::ObjectClassHandle& objectClassHandle)
  {
    _ambassador->unpublishObjectClass(objectClassHandle);
  }

  void publishInteractionClass(const RTI::InteractionClassHandle& interactionClassHandle)
  {
    _ambassador->publishInteractionClass(interactionClassHandle);
  }

  void unpublishInteractionClass(const RTI::InteractionClassHandle& interactionClassHandle)
  {
    _ambassador->unpublishInteractionClass(interactionClassHandle);
  }

  void subscribeObjectClassAttributes(const RTI::ObjectClassHandle& objectClassHandle,
                                      const RTI::AttributeHandleSet& attributeHandleSet,
                                      bool active = true)
  {
    _ambassador->subscribeObjectClassAttributes(objectClassHandle, attributeHandleSet, RTI::Boolean(active));
      // _subscribedObjectClassAttributeHandleSetMap[objectClassHandle].insert(attributeHandleSet.begin(), attributeHandleSet.end());
  }

  void unsubscribeObjectClass(const RTI::ObjectClassHandle& objectClassHandle)
  {
      _ambassador->unsubscribeObjectClass(objectClassHandle);
      // _subscribedObjectClassAttributeHandleSetMap.erase(objectClassHandle);
  }

  void subscribeInteractionClass(const RTI::InteractionClassHandle& interactionClassHandle, bool active = true)
  {
    _ambassador->subscribeInteractionClass(interactionClassHandle, RTI::Boolean(active));
  }

  void unsubscribeInteractionClass(const RTI::InteractionClassHandle& interactionClassHandle)
  {
    _ambassador->unsubscribeInteractionClass(interactionClassHandle);
  }

  RTI::ObjectHandle registerObjectInstance(const RTI::ObjectClassHandle& objectClassHandle)
  {
    return _ambassador->registerObjectInstance(objectClassHandle);
  }

  RTI::ObjectHandle registerObjectInstance(const RTI::ObjectClassHandle& objectClassHandle,
                                           const std::string& objectInstanceName)
  {
    return _ambassador->registerObjectInstance(objectClassHandle, objectInstanceName.c_str());
  }

  void updateAttributeValues(const RTI::ObjectHandle& objectHandle,
                             const RTI::AttributeHandleValuePairSet& attributeHandleValueMap,
                             const std::string& tag)
  {
    _ambassador->updateAttributeValues(objectHandle, attributeHandleValueMap, tag.c_str());
  }

  RTI::EventRetractionHandle updateAttributeValues(const RTI::ObjectHandle& objectHandle,
                                                     const RTI::AttributeHandleValuePairSet& attributeHandleValueMap,
                                                     const std::string& tag,
                                                     const RTI::FedTime& logicalTime)
  {
    return _ambassador->updateAttributeValues(objectHandle, attributeHandleValueMap, logicalTime, tag.c_str());
  }

  void sendInteraction(const RTI::InteractionClassHandle& interactionClassHandle,
                       const RTI::ParameterHandleValuePairSet& parameterHandleValueMap,
                       const std::string& tag)
  {
      _ambassador->sendInteraction(interactionClassHandle, parameterHandleValueMap, tag.c_str());
  }

  RTI::EventRetractionHandle sendInteraction(const RTI::InteractionClassHandle& interactionClassHandle,
                                               const RTI::ParameterHandleValuePairSet& parameterHandleValueMap,
                                               const std::string& tag,
                                               const RTI::FedTime& logicalTime)
  {
      return _ambassador->sendInteraction(interactionClassHandle, parameterHandleValueMap, logicalTime, tag.c_str());
  }

  void deleteObjectInstance(const RTI::ObjectHandle& objectHandle,
                            const char* tag)
  {
    _ambassador->deleteObjectInstance(objectHandle, tag);
  }

  RTI::EventRetractionHandle deleteObjectInstance(const RTI::ObjectHandle& objectHandle,
                                                        const char* tag,
                                                        const RTI::FedTime& logicalTime)
  {
    return _ambassador->deleteObjectInstance(objectHandle, logicalTime, tag);
  }

  void localDeleteObjectInstance(const RTI::ObjectHandle& objectHandle)
  {
    _ambassador->localDeleteObjectInstance(objectHandle);
  }

  // void changeAttributeTransportationType(const RTI::ObjectHandle& objectHandle,
  //                                        const RTI::AttributeHandleSet& attributeHandleSet,
  //                                        const RTI::TransportationHandle& transportationHandle)
  // {
  //   _ambassador->changeAttributeTransportationType(objectHandle, attributeHandleSet, transportationHandle);
  // }

  // void changeInteractionTransportationType(const RTI::InteractionClassHandle& interactionClassHandle,
  //                                          const RTI::TransportationHandle& transportationHandle)
  // {
  //   _ambassador->changeInteractionTransportationType(interactionClassHandle, transportationHandle);
  // }

  // void requestObjectAttributeValueUpdate(const RTI::ObjectHandle& objectHandle,
  //                                        const RTI::AttributeHandleSet& attributeHandleSet,
  //                                        const char* tag)
  // {
  //   _ambassador->requestObjectAttributeValueUpdate(objectHandle, attributeHandleSet, tag);
  // }

  // void requestClassAttributeValueUpdate(const RTI::ObjectClassHandle& objectClassHandle,
  //                                       const RTI::AttributeHandleSet& attributeHandleSet,
  //                                       const char* tag)
  // {
  //   _ambassador->requestClassAttributeValueUpdate(objectClassHandle, attributeHandleSet, tag);
  // }

  // void unconditionalAttributeOwnershipDivestiture(const RTI::ObjectHandle& objectHandle,
  //                                                 const RTI::AttributeHandleSet& attributeHandleSet)
  // {
  //   _ambassador->unconditionalAttributeOwnershipDivestiture(objectHandle, attributeHandleSet);
  // }

    // // 7.3
    //  void negotiatedAttributeOwnershipDivestiture
    // (ObjectHandle objectHandle,
    //  AttributeHandleSet const & attributes,
    //  const char* tag)

    // // 7.6
    //  void confirmDivestiture
    // (ObjectHandle objectHandle,
    //  AttributeHandleSet const & confirmedAttributes,
    //  const char* tag)

    // // 7.8
    //  void attributeOwnershipAcquisition
    // (ObjectHandle objectHandle,
    //  AttributeHandleSet const & desiredAttributes,
    //  const char* tag)

    // // 7.9
    //  void attributeOwnershipAcquisitionIfAvailable
    // (ObjectHandle objectHandle,
    //  AttributeHandleSet const & desiredAttributes)

    // // 7.12
    //  void attributeOwnershipDivestitureIfWanted
    // (ObjectHandle objectHandle,
    //  AttributeHandleSet const & attributes,
    //  AttributeHandleSet & theDivestedAttributes) // filled by RTI

    // // 7.13
    //  void cancelNegotiatedAttributeOwnershipDivestiture
    // (ObjectHandle objectHandle,
    //  AttributeHandleSet const & attributes)

    // // 7.14
    //  void cancelAttributeOwnershipAcquisition
    // (ObjectHandle objectHandle,
    //  AttributeHandleSet const & attributes)

    // // 7.16
    //  void queryAttributeOwnership
    // (ObjectHandle objectHandle,
    //  AttributeHandle attribute)

    // // 7.18
    //  bool isAttributeOwnedByFederate
    // (ObjectHandle objectHandle,
    //  AttributeHandle attribute)

  void enableTimeRegulation(const RTI::FedTime& fedTime, const RTI::FedTime& logicalTimeInterval)
  {
    _ambassador->enableTimeRegulation(fedTime, logicalTimeInterval);
  }

  void disableTimeRegulation()
  {
    _timeRegulationEnabled = false;
    _ambassador->disableTimeRegulation();
  }

  void enableTimeConstrained()
  {
    _ambassador->enableTimeConstrained();
  }

  void disableTimeConstrained()
  {
    _timeConstrainedEnabled = false;
    _ambassador->disableTimeConstrained();
  }

  void timeAdvanceRequest(const RTI::FedTime& logicalTime)
  {
    _timeAdvancePending = true;
    _ambassador->timeAdvanceRequest(logicalTime);
  }

  void timeAdvanceRequestAvailable(const RTI::FedTime& logicalTime)
  {
    _timeAdvancePending = true;
    _ambassador->timeAdvanceRequestAvailable(logicalTime);
  }

  // void nextMessageRequest(const RTI::FedTime& logicalTime)
  // {
  //   _timeAdvancePending = true;
  //   _ambassador->nextMessageRequest(logicalTime);
  // }

  // void nextMessageRequestAvailable(const RTI::FedTime& logicalTime)
  // {
  //   _timeAdvancePending = true;
  //   _ambassador->nextMessageRequestAvailable(logicalTime);
  // }

  void flushQueueRequest(const RTI::FedTime& logicalTime)
  {
    // _timeAdvancePending = true;
    _ambassador->flushQueueRequest(logicalTime);
  }

  void enableAsynchronousDelivery()
  {
    _ambassador->enableAsynchronousDelivery();
  }

  void disableAsynchronousDelivery()
  {
    _ambassador->disableAsynchronousDelivery();
  }

    //  bool queryGALT (FedTime & logicalTime)

    //  void queryFedTime (FedTime & logicalTime)

    //  bool queryLITS (FedTime & logicalTime)

    //  void modifyLookahead

    //  void queryLookahead (FedTime & interval)

    //  void retract
    // (EventRetractionHandle theHandle)

    //  void changeAttributeOrderType
    // (ObjectHandle objectHandle,
    //  AttributeHandleSet const & attributes,
    //  OrderType theType)

    //  void changeInteractionOrderType
    // (InteractionClassHandle theClass,
    //  OrderType theType)

    //  RegionHandle createRegion
    // (DimensionHandleSet const & theDimensions)

    //  void commitRegionModifications
    // (RegionHandleSet const & regionHandleSet)

    //  void deleteRegion
    // (RegionHandle region)

    //  ObjectHandle registerObjectInstanceWithRegions
    // (ObjectClassHandle theClass,
    //  AttributeHandleSetRegionHandleSetPairVector const &
    //  attributeHandleSetRegionHandleSetPairVector)

    //  ObjectHandle registerObjectInstanceWithRegions
    // (ObjectClassHandle theClass,
    //  AttributeHandleSetRegionHandleSetPairVector const &
    //  attributeHandleSetRegionHandleSetPairVector,
    //  std::string const & objectInstanceName)

    //  void associateRegionsForUpdates
    // (ObjectHandle objectHandle,
    //  AttributeHandleSetRegionHandleSetPairVector const &
    //  attributeHandleSetRegionHandleSetPairVector)

    //  void unassociateRegionsForUpdates
    // (ObjectHandle objectHandle,
    //  AttributeHandleSetRegionHandleSetPairVector const &
    //  attributeHandleSetRegionHandleSetPairVector)

    //  void subscribeObjectClassAttributesWithRegions
    // (ObjectClassHandle theClass,
    //  AttributeHandleSetRegionHandleSetPairVector const &
    //  attributeHandleSetRegionHandleSetPairVector,
    //  bool active = true)

    //  void unsubscribeObjectClassAttributesWithRegions
    // (ObjectClassHandle theClass,
    //  AttributeHandleSetRegionHandleSetPairVector const &
    //  attributeHandleSetRegionHandleSetPairVector)

    //  void subscribeInteractionClassWithRegions
    // (InteractionClassHandle theClass,
    //  RegionHandleSet const & regionHandleSet,
    //  bool active = true)

    //  void unsubscribeInteractionClassWithRegions
    // (InteractionClassHandle theClass,
    //  RegionHandleSet const & regionHandleSet)

    //  void sendInteractionWithRegions
    // (InteractionClassHandle interaction,
    //  ParameterHandleValuePairSet const & parameterValues,
    //  RegionHandleSet const & regionHandleSet,
    //  const char* tag)

    //  EventRetractionHandle sendInteractionWithRegions
    // (InteractionClassHandle interaction,
    //  ParameterHandleValuePairSet const & parameterValues,
    //  RegionHandleSet const & regionHandleSet,
    //  const char* tag,
    //  FedTime const & logicalTime)

    //  void requestAttributeValueUpdateWithRegions
    // (ObjectClassHandle theClass,
    //  AttributeHandleSetRegionHandleSetPairVector const & theSet,
    //  const char* tag)

  RTI::ObjectClassHandle getObjectClassHandle(std::string const & theName)
  {
    return _ambassador->getObjectClassHandle(theName.c_str());
  }

  std::string getObjectClassName(RTI::ObjectClassHandle theHandle)
  {
    return _ambassador->getObjectClassName(theHandle);
  }

  RTI::AttributeHandle getAttributeHandle(RTI::ObjectClassHandle whichClass, std::string const & attributeName)
  {
    return _ambassador->getAttributeHandle(attributeName.c_str(), whichClass);
  }

  std::string getAttributeName(RTI::ObjectClassHandle whichClass, RTI::AttributeHandle theHandle)
  {
    return _ambassador->getAttributeName(theHandle, whichClass);
  }

  RTI::InteractionClassHandle getInteractionClassHandle(std::string const & theName)
  {
    return _ambassador->getInteractionClassHandle(theName.c_str());
  }

  std::string getInteractionClassName(RTI::InteractionClassHandle theHandle)
  {
    return _ambassador->getInteractionClassName(theHandle);
  }

  RTI::ParameterHandle getParameterHandle(RTI::InteractionClassHandle whichClass, std::string const & theName)
  {
    return _ambassador->getParameterHandle(theName.c_str(), whichClass);
  }

  std::string getParameterName(RTI::InteractionClassHandle whichClass, RTI::ParameterHandle theHandle)
  {
    return _ambassador->getParameterName(theHandle, whichClass);
  }

  RTI::ObjectHandle getObjectInstanceHandle(std::string const & theName)
  {
    return _ambassador->getObjectInstanceHandle(theName.c_str());
  }

  std::string getObjectInstanceName(RTI::ObjectHandle theHandle)
  {
    return _ambassador->getObjectInstanceName(theHandle);
  }

  // RTI::DimensionHandle getDimensionHandle(std::string const & theName)
  // {
  //   return _ambassador->getDimensionHandle(theName.c_str());
  // }

  // std::string getDimensionName(RTI::DimensionHandle theHandle)
  // {
  //   return _ambassador->getDimensionName(theHandle);
  // }

  // unsigned long getDimensionUpperBound(RTI::DimensionHandle theHandle)
  // {
  //   return _ambassador->getDimensionUpperBound(theHandle);
  // }

  // RTI::DimensionHandleSet getAvailableDimensionsForClassAttribute(RTI::ObjectClassHandle theClass,
  //                                                                     RTI::AttributeHandle theHandle)
  // {
  //   return _ambassador->getAvailableDimensionsForClassAttribute(theClass, theHandle);
  // }

  // RTI::ObjectClassHandle getKnownObjectClassHandle(RTI::ObjectHandle object)
  // {
  //   return _ambassador->getKnownObjectClassHandle(object);
  // }

  // RTI::DimensionHandleSet getAvailableDimensionsForInteractionClass(RTI::InteractionClassHandle theClass)
  // {
  //   return _ambassador->getAvailableDimensionsForInteractionClass(theClass);
  // }

  // RTI::TransportationHandle getTransportationType(std::string const & transportationName)
  // {
  //   return _ambassador->getTransportationType(transportationName.c_str());
  // }

  // std::string getTransportationName(RTI::TransportationHandle transportationHandle)
  // {
  //   return _ambassador->getTransportationName(transportationHandle);
  // }

  // RTI::OrderType getOrderType(std::string const & orderName)
  // {
  //   return _ambassador->getOrderType(orderName.c_str());
  // }

  // std::string getOrderName(RTI::OrderType orderType)
  // {
  //   return _ambassador->getOrderName(orderType);
  // }

  // void enableObjectClassRelevanceAdvisorySwitch()
  // {
  //   _ambassador->enableObjectClassRelevanceAdvisorySwitch();
  // }

  // void disableObjectClassRelevanceAdvisorySwitch()
  // {
  //   _ambassador->disableObjectClassRelevanceAdvisorySwitch();
  // }

  // void enableAttributeRelevanceAdvisorySwitch()
  // {
  //   _ambassador->enableAttributeRelevanceAdvisorySwitch();
  // }

  // void disableAttributeRelevanceAdvisorySwitch()
  // {
  //   _ambassador->disableAttributeRelevanceAdvisorySwitch();
  // }

  // void enableAttributeScopeAdvisorySwitch()
  // {
  //   _ambassador->enableAttributeScopeAdvisorySwitch();
  // }

  // void disableAttributeScopeAdvisorySwitch()
  // {
  //   _ambassador->disableAttributeScopeAdvisorySwitch();
  // }

  // void enableInteractionRelevanceAdvisorySwitch()
  // {
  //   _ambassador->enableInteractionRelevanceAdvisorySwitch();
  // }

  // void disableInteractionRelevanceAdvisorySwitch()
  // {
  //   _ambassador->disableInteractionRelevanceAdvisorySwitch();
  // }

  // RTI::DimensionHandleSet getDimensionHandleSet(RTI::RegionHandle regionHandle)
  // {
  //   return _ambassador->getDimensionHandleSet(regionHandle);
  // }

  // RTI::RangeBounds getRangeBounds(RTI::RegionHandle regionHandle, RTI::DimensionHandle theDimensionHandle)
  // {
  //   return _ambassador->getRangeBounds(regionHandle, theDimensionHandle);
  // }

  // void setRangeBounds(RTI::RegionHandle regionHandle, RTI::DimensionHandle theDimensionHandle,
  //                     RTI::RangeBounds const & rangeBounds)
  // {
  //   return _ambassador->setRangeBounds(regionHandle, theDimensionHandle, rangeBounds);
  // }

  bool evokeCallback(double approximateMinimumTimeInSeconds)
  {
    return _ambassador->tick(approximateMinimumTimeInSeconds, 0);
  }

  bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                              double approximateMaximumTimeInSeconds)
  {
    return _ambassador->tick(approximateMinimumTimeInSeconds, approximateMaximumTimeInSeconds);
  }

  // void enableCallbacks()
  // {
  //   _ambassador->enableCallbacks();
  // }

  // void disableCallbacks()
  // {
  //   _ambassador->disableCallbacks();
  // }

protected:
  virtual void synchronizationPointRegistrationSucceeded(const char *label)
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void synchronizationPointRegistrationFailed(const char *label)
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void announceSynchronizationPoint(const char *label, const char *tag)
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void federationSynchronized(const char *label)
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void initiateFederateSave(const char* label)
      RTI_THROW ((RTI::UnableToPerformSave,
             RTI::FederateInternalError))
  {
  }

  virtual void federationSaved()
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void federationNotSaved()
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void requestFederationRestoreSucceeded(const char* label)
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void requestFederationRestoreFailed(const char* label, const char* tag)
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void federationRestoreBegun()
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void initiateFederateRestore(const char* label, RTI::FederateHandle handle)
    RTI_THROW ((RTI::SpecifiedSaveLabelDoesNotExist,
           RTI::CouldNotRestore,
           RTI::FederateInternalError))
  {
  }

  virtual void federationRestored()
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void federationNotRestored()
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  virtual void startRegistrationForObjectClass(RTI::ObjectClassHandle)
    RTI_THROW ((RTI::ObjectClassNotPublished,
           RTI::FederateInternalError))
  {
  }

  virtual void stopRegistrationForObjectClass(RTI::ObjectClassHandle)
    RTI_THROW ((RTI::ObjectClassNotPublished,
           RTI::FederateInternalError))
  {
  }

  virtual void turnInteractionsOn(RTI::InteractionClassHandle)
    RTI_THROW ((RTI::InteractionClassNotPublished,
           RTI::FederateInternalError))
  {
  }

  virtual void turnInteractionsOff(RTI::InteractionClassHandle)
    RTI_THROW ((RTI::InteractionClassNotPublished,
           RTI::FederateInternalError))
  {
  }

  virtual void discoverObjectInstance(RTI::ObjectHandle objectHandle,
                                      RTI::ObjectClassHandle objectClassHandle,
                                      const char* objectInstanceName)
    RTI_THROW ((RTI::CouldNotDiscover,
           RTI::ObjectClassNotKnown,
           RTI::FederateInternalError))
  {
      // ObjectClassAttributeHandleSetMap::iterator i = _subscribedObjectClassAttributeHandleSetMap.find(objectClassHandle);
      // if (i == _subscribedObjectClassAttributeHandleSetMap.end()) {
      //     fail();
      //     throw RTI::ObjectClassNotKnown(objectClassHandle.toString());
      // }

      // if (_objectInstanceMap.find(objectHandle) != _objectInstanceMap.end()) {
      //     fail();
      //     throw RTI::CouldNotDiscover(objectHandle.toString());
      // }

      // _objectInstanceMap[objectHandle]._objectClassHandle = objectClassHandle;
  }

  virtual void reflectAttributeValues(RTI::ObjectHandle, const RTI::AttributeHandleValuePairSet &,
                                      const RTI::FedTime &, const char *, RTI::EventRetractionHandle)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotKnown,
           RTI::FederateOwnsAttributes,
           RTI::InvalidFederationTime,
           RTI::FederateInternalError))
  {
      // _verifyReflectAttributeValues(objectHandle, attributeHandleValueMap);
  }

  virtual void reflectAttributeValues(RTI::ObjectHandle, const RTI::AttributeHandleValuePairSet &, const char *)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotKnown,
           RTI::FederateOwnsAttributes,
           RTI::FederateInternalError))
  {
      // _verifyReflectAttributeValues(objectHandle, attributeHandleValueMap);
  }

  virtual void receiveInteraction(RTI::InteractionClassHandle, const RTI::ParameterHandleValuePairSet &,
                                  const RTI::FedTime &, const char *, RTI::EventRetractionHandle)
    RTI_THROW ((RTI::InteractionClassNotKnown,
           RTI::InteractionParameterNotKnown,
           RTI::InvalidFederationTime,
           RTI::FederateInternalError))
  {
  }

  virtual void receiveInteraction(RTI::InteractionClassHandle, const RTI::ParameterHandleValuePairSet &,
                                  const char *)
    RTI_THROW ((RTI::InteractionClassNotKnown,
           RTI::InteractionParameterNotKnown,
           RTI::FederateInternalError))
  {
  }

  virtual void removeObjectInstance(RTI::ObjectHandle, const RTI::FedTime &, const char *,
                                    RTI::EventRetractionHandle)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::InvalidFederationTime,
           RTI::FederateInternalError))
  {
  }

  virtual void removeObjectInstance(RTI::ObjectHandle, const char *)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::FederateInternalError))
  {
  }

  virtual void attributesInScope(RTI::ObjectHandle, const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotKnown,
           RTI::FederateInternalError))
  {
  }

  virtual void attributesOutOfScope(RTI::ObjectHandle, const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotKnown,
           RTI::FederateInternalError))
  {
  }

  virtual void provideAttributeValueUpdate(RTI::ObjectHandle theObject,
                                           RTI::AttributeHandleSet const & theAttributes)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotKnown,
           RTI::AttributeNotOwned,
           RTI::FederateInternalError))
  {
  }

  virtual void turnUpdatesOnForObjectInstance(RTI::ObjectHandle objectHandle,
                                              RTI::AttributeHandleSet const & attributes)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotOwned,
           RTI::FederateInternalError))
  {
  }

  virtual void turnUpdatesOffForObjectInstance(RTI::ObjectHandle objectHandle,
                                               RTI::AttributeHandleSet const & attributes)
    RTI_THROW ((RTI::ObjectNotKnown,
           RTI::AttributeNotOwned,
           RTI::FederateInternalError))
  {
  }

  virtual void requestAttributeOwnershipAssumption(RTI::ObjectHandle, const RTI::AttributeHandleSet &,
                                                   const char *)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAlreadyOwned,
           RTI::AttributeNotPublished, RTI::FederateInternalError)) { }

  virtual void attributeOwnershipDivestitureNotification(RTI::ObjectHandle, const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned,
           RTI::AttributeDivestitureWasNotRequested, RTI::FederateInternalError)) { }

  virtual void attributeOwnershipAcquisitionNotification(RTI::ObjectHandle, const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAcquisitionWasNotRequested,
           RTI::AttributeAlreadyOwned, RTI::AttributeNotPublished, RTI::FederateInternalError)) { }

  virtual void attributeOwnershipUnavailable(RTI::ObjectHandle, const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAlreadyOwned,
           RTI::AttributeAcquisitionWasNotRequested, RTI::FederateInternalError)) { }

  virtual void requestAttributeOwnershipRelease(RTI::ObjectHandle, const RTI::AttributeHandleSet &,
                                                const char *)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError)) { }

  virtual void confirmAttributeOwnershipAcquisitionCancellation(RTI::ObjectHandle,
                                                                const RTI::AttributeHandleSet &)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAlreadyOwned,
           RTI::AttributeAcquisitionWasNotCanceled, RTI::FederateInternalError)) { }

  virtual void informAttributeOwnership(RTI::ObjectHandle, RTI::AttributeHandle, RTI::FederateHandle)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError)) { }

  virtual void attributeIsNotOwned(RTI::ObjectHandle, RTI::AttributeHandle)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError)) { }

  virtual void attributeOwnedByRTI(RTI::ObjectHandle, RTI::AttributeHandle)
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError)) { }

  virtual void timeRegulationEnabled(RTI::FedTime const & theFederateTime)
    RTI_THROW ((RTI::InvalidFederationTime,
           RTI::EnableTimeRegulationWasNotPending,
           RTI::FederateInternalError))
  {
    _timeRegulationEnabled = true;
    *_grantedFedTime = theFederateTime;
  }

  virtual void timeConstrainedEnabled(RTI::FedTime const & theFederateTime)
    RTI_THROW ((RTI::InvalidFederationTime,
           RTI::EnableTimeConstrainedWasNotPending,
           RTI::FederateInternalError))
  {
    _timeConstrainedEnabled = true;
    *_grantedFedTime = theFederateTime;
  }

  virtual void timeAdvanceGrant(RTI::FedTime const & theFederateTime)
    RTI_THROW ((RTI::InvalidFederationTime,
           RTI::TimeAdvanceWasNotInProgress,
           RTI::FederateInternalError))
  {
    _timeAdvancePending = false;
    *_grantedFedTime = theFederateTime;
  }

  virtual void requestRetraction(RTI::EventRetractionHandle theHandle)
    RTI_THROW ((RTI::FederateInternalError))
  {
  }

  // void fail()
  // { _fail = true; }

private:
  void _replaceFileWithDataIfNeeded(std::string& fddFile)
  {
    if (!_useDataUrlObjectModels)
      return;
    // already a data url
    if (fddFile.compare(0, 5, "data:") == 0)
      return;
    std::ifstream stream;
    if (fddFile.compare(0, 8, "file:///") == 0)
      stream.open(fddFile.substr(8).c_str());
    else
      stream.open(fddFile.c_str());
    if (!stream.is_open())
      return;
    fddFile = "data:,";
    std::copy(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>(), std::back_inserter(fddFile));
  }

  // bool _verifyGrantedFedTime(const RTI::FedTime& logicalTime) const
  // { return logicalTime == *_grantedFedTime; }

  // void _verifyReflectAttributeValues(const RTI::ObjectHandle& objectHandle,
  //                                    const RTI::AttributeHandleValuePairSet& attributeHandleValueMap)
  // {
  //     ObjectInstanceMap::iterator i = _objectInstanceMap.find(objectHandle);
  //     if (i == _objectInstanceMap.end()) {
  //         fail();
  //         throw RTI::ObjectNotKnown(objectHandle.toString());
  //     }

  //     ObjectClassAttributeHandleSetMap::iterator j = _subscribedObjectClassAttributeHandleSetMap.find(i->second._objectClassHandle);
  //     for (RTI::AttributeHandleValuePairSet::const_iterator k = attributeHandleValueMap.begin();
  //          k != attributeHandleValueMap.end(); ++k) {
  //         if (j->second.find(k->first) != j->second.end())
  //             continue;
  //         fail();
  //         throw RTI::AttributeNotSubscribed(k->first.toString());
  //     }
  // }

  // void _verifyRemoveObjectInstance(RTI::ObjectHandle objectHandle)
  // {
  //     if (_objectInstanceMap.find(objectHandle) == _objectInstanceMap.end()) {
  //         fail();
  //         throw RTI::ObjectNotKnown(objectHandle.toString());
  //     }

  //     _objectInstanceMap.erase(objectHandle);
  // }

  // bool _fail;

  RTI_UNIQUE_PTR<RTI::RTIambassador> _ambassador;

  bool _useDataUrlObjectModels;

  RTI::FederateHandle _federateHandle;

  RTI_UNIQUE_PTR<RTI::FedTime> _grantedFedTime;
  bool _timeRegulationEnabled;
  bool _timeConstrainedEnabled;
  bool _timeAdvancePending;

  // Hmm, FIXME: make an additional derived checking ambassador for the tests, keep a simple one without expensive tests
  // FIXME make this and for example the simple log below callbacks that we can attach or not as apropriate

  // // FIXME implement subscription tracking also for regions at some time
  // typedef std::map<RTI::ObjectClassHandle, RTI::AttributeHandleSet> ObjectClassAttributeHandleSetMap;
  // ObjectClassAttributeHandleSetMap _subscribedObjectClassAttributeHandleSetMap;

  // struct ObjectInstance {
  //   RTI::ObjectClassHandle _objectClassHandle;
  // };

  // typedef std::map<RTI::ObjectHandle, ObjectInstance> ObjectInstanceMap;
  // ObjectInstanceMap _objectInstanceMap;
};

}

#endif
