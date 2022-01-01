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

#ifndef OpenRTI_RTI1516ETestLib_h
#define OpenRTI_RTI1516ETestLib_h

#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <RTI/FederateAmbassador.h>
#include <RTI/RTIambassadorFactory.h>
#include <RTI/RTIambassador.h>
#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/RangeBounds.h>

#include <TestLib.h>

namespace OpenRTI {

inline bool
operator==(const rti1516e::VariableLengthData& lhs, const rti1516e::VariableLengthData& rhs)
{
  size_t size = lhs.size();
  if (size != rhs.size())
    return false;
  return 0 == memcmp(lhs.data(), rhs.data(), size);
}

inline bool
operator!=(const rti1516e::VariableLengthData& lhs, const rti1516e::VariableLengthData& rhs)
{ return !operator==(lhs, rhs); }

inline rti1516e::VariableLengthData
toVariableLengthData(const char* s)
{
    rti1516e::VariableLengthData variableLengthData;
    if (s)
        variableLengthData.setData(s, strlen(s));
    return variableLengthData;
}

inline rti1516e::VariableLengthData
toVariableLengthData(const std::string& s)
{
    rti1516e::VariableLengthData variableLengthData;
    variableLengthData.setData(s.data(), s.size());
    return variableLengthData;
}

inline std::string
toString(const rti1516e::VariableLengthData& variableLengthData)
{
    if (!variableLengthData.size())
        return std::string();
    return std::string((const char*)variableLengthData.data(), variableLengthData.size());
}

inline rti1516e::VariableLengthData
toVariableLengthData(const std::wstring& s)
{
    rti1516e::VariableLengthData variableLengthData;
    variableLengthData.setData(s.data(), sizeof(std::wstring::value_type)*s.size());
    return variableLengthData;
}

inline std::wstring
toWString(const rti1516e::VariableLengthData& variableLengthData)
{
    if (!variableLengthData.size())
        return std::wstring();
    return std::wstring((const wchar_t*)variableLengthData.data(), variableLengthData.size()/sizeof(std::wstring::value_type));
}

inline rti1516e::VariableLengthData
toVariableLengthData(const Clock& c)
{
    // May be at some time make this endian safe
    rti1516e::VariableLengthData variableLengthData;
    variableLengthData.setData(&c, sizeof(c));
    return variableLengthData;
}

inline Clock
toClock(const rti1516e::VariableLengthData& variableLengthData)
{
    Clock c;
    // May be at some time make this endian safe
    if (variableLengthData.size() == sizeof(Clock))
        memcpy(&c, variableLengthData.data(), sizeof(Clock));
    return c;
}

inline rti1516e::VariableLengthData
toVariableLengthData(unsigned u)
{
    // May be at some time make this endian safe
    rti1516e::VariableLengthData variableLengthData;
    variableLengthData.setData(&u, sizeof(u));
    return variableLengthData;
}

inline unsigned
toUnsigned(const rti1516e::VariableLengthData& variableLengthData)
{
    unsigned u = -1;
    // May be at some time make this endian safe
    if (variableLengthData.size() == sizeof(unsigned))
        memcpy(&u, variableLengthData.data(), sizeof(unsigned));
    return u;
}

class OPENRTI_LOCAL RTI1516ETestAmbassador : public RTITest::Ambassador, public rti1516e::FederateAmbassador {
public:
  RTI1516ETestAmbassador(const RTITest::ConstructorArgs& constructorArgs) :
    RTITest::Ambassador(constructorArgs),
    _synchronized(0)
  { }
  virtual ~RTI1516ETestAmbassador()
    RTI_NOEXCEPT
  { }

  std::wstring getArgument() const
  {
    std::wstring arg;
    std::vector<std::wstring> args = getArgumentList();
    for (std::vector<std::wstring>::const_iterator i = args.begin(); i != args.end(); ++i) {
      if (!arg.empty())
        arg += L";";
      arg += *i;
    }
    return arg;
  }

  virtual bool execJoined(rti1516e::RTIambassador& ambassador) = 0;

  bool waitForAllFederates(rti1516e::RTIambassador& ambassador)
  {
    _synchronized = 0;

    // FIXME need a test for concurrent announces
    try {
      ambassador.registerFederationSynchronizationPoint(getFederateType(), rti1516e::VariableLengthData());
    } catch (const rti1516e::Exception& e) {
      std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      Clock timeout = Clock::now() + Clock::fromSeconds(10);
      while (!_federateSet.empty()) {
        if (ambassador.evokeCallback(10.0))
          continue;
        if (timeout < Clock::now()) {
          std::wcout << L"Timeout waiting for other federates to join!" << std::endl;
          return false;
        }
      }
    } catch (const rti1516e::Exception& e) {
      std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    // Fill for the next time
    _federateSet.insert(getFederateList().begin(), getFederateList().end());

    try {
      for (std::vector<std::wstring>::const_iterator i = getFederateList().begin(); i != getFederateList().end(); ++i) {
        ambassador.synchronizationPointAchieved(*i);
      }
    } catch (const rti1516e::Exception& e) {
      std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      Clock timeout = Clock::now() + Clock::fromSeconds(10);
      while (_synchronized < getFederateList().size()) {
        if (ambassador.evokeCallback(10.0))
          continue;
        if (timeout < Clock::now()) {
          std::wcout << L"Timeout waiting for other federates to synchronize!" << std::endl;
          return false;
        }
      }
    } catch (const rti1516e::Exception& e) {
      std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  virtual bool execJoinOnce()
  {
    RTI_UNIQUE_PTR<rti1516e::RTIambassador> ambassador;
    rti1516e::RTIambassadorFactory factory;
    ambassador = factory.createRTIambassador();
    ambassador->connect(*this, rti1516e::HLA_EVOKED, getConnectUrl());

    // create, must work once
    try {
      if (getMimFile().empty())
        ambassador->createFederationExecution(getFederationExecution(), getFddFileList(), std::wstring(L"HLAinteger64Time"));
      else
        ambassador->createFederationExecutionWithMIM(getFederationExecution(), getFddFileList(), getMimFile(), std::wstring(L"HLAinteger64Time"));

      if (!getFederationBarrier()->success())
        return false;
    } catch (const rti1516e::FederationExecutionAlreadyExists&) {
      // Can happen in this test

      if (!getFederationBarrier()->fail())
        return false;
    } catch (const rti1516e::Exception& e) {
      std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    // Try that several times. Ensure correct cleanup
    unsigned n = 99;
    for (unsigned i = 0; i < n; ++i) {

      // join must work
      try {
        ambassador->joinFederationExecution(getFederateType(), getFederationExecution());
      } catch (const rti1516e::Exception& e) {
        std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      _federateSet.insert(getFederateList().begin(), getFederateList().end());

      if (!execJoined(*ambassador))
        return false;

      // and now resign must work
      try {
        ambassador->resignFederationExecution(rti1516e::CANCEL_THEN_DELETE_THEN_DIVEST);
      } catch (const rti1516e::Exception& e) {
        std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

    }

    // Wait for all threads, this is to ensure that we do not destroy before we are ready
    wait();

    // destroy, must work once
    try {
      ambassador->destroyFederationExecution(getFederationExecution());

      if (!getFederationBarrier()->success())
        return false;
    } catch (const rti1516e::FederatesCurrentlyJoined&) {
      // Can happen in this test
      // Other threads just might have still joined.

      if (!getFederationBarrier()->fail())
        return false;
    } catch (const rti1516e::FederationExecutionDoesNotExist&) {
      // Can happen in this test
      // Other threads might have been faster

      if (!getFederationBarrier()->fail())
        return false;
    } catch (const rti1516e::Exception& e) {
      std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  virtual bool execJoinMultiple()
  {
    RTI_UNIQUE_PTR<rti1516e::RTIambassador> ambassador;
    rti1516e::RTIambassadorFactory factory;
    ambassador = factory.createRTIambassador();
    ambassador->connect(*this, rti1516e::HLA_EVOKED, getConnectUrl());

    // Try that several times. Ensure correct cleanup
    unsigned n = 99;
    for (unsigned i = 0; i < n; ++i) {

      // create, must work once
      try {
        if (getMimFile().empty())
          ambassador->createFederationExecution(getFederationExecution(), getFddFileList(), std::wstring(L"HLAinteger64Time"));
        else
          ambassador->createFederationExecutionWithMIM(getFederationExecution(), getFddFileList(), getMimFile(), std::wstring(L"HLAinteger64Time"));

        if (!getFederationBarrier()->success())
          return false;
      } catch (const rti1516e::FederationExecutionAlreadyExists&) {
        // Can happen in this test

        if (!getFederationBarrier()->fail())
          return false;
      } catch (const rti1516e::Exception& e) {
        std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // join must work
      try {
        ambassador->joinFederationExecution(getFederateType(), getFederationExecution());
      } catch (const rti1516e::Exception& e) {
        std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      _federateSet.insert(getFederateList().begin(), getFederateList().end());

      if (!execJoined(*ambassador))
        return false;

      // and now resign must work
      try {
        ambassador->resignFederationExecution(rti1516e::CANCEL_THEN_DELETE_THEN_DIVEST);
      } catch (const rti1516e::Exception& e) {
        std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // Wait for all threads, this is to ensure that we do not destroy before we are ready
      wait();

      // destroy, must work once
      try {
        ambassador->destroyFederationExecution(getFederationExecution());

        if (!getFederationBarrier()->success())
          return false;
      } catch (const rti1516e::FederatesCurrentlyJoined&) {
        // Can happen in this test
        // Other threads just might have still joined.

        if (!getFederationBarrier()->fail())
          return false;
      } catch (const rti1516e::FederationExecutionDoesNotExist&) {
        // Can happen in this test
        // Other threads might have been faster

        if (!getFederationBarrier()->fail())
          return false;
      } catch (const rti1516e::Exception& e) {
        std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
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

  virtual void connectionLost(const std::wstring& faultDescription)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void reportFederationExecutions(const rti1516e::FederationExecutionInformationVector& theFederationExecutionInformationList)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void synchronizationPointRegistrationSucceeded(const std::wstring& label)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void synchronizationPointRegistrationFailed(const std::wstring& label, rti1516e::SynchronizationPointFailureReason reason)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void announceSynchronizationPoint(const std::wstring& label, const rti1516e::VariableLengthData& tag)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    _federateSet.erase(label);
  }

  virtual void federationSynchronized(const std::wstring& label, const rti1516e::FederateHandleSet& failedToSyncSet)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    ++_synchronized;
  }

  virtual void initiateFederateSave(const std::wstring& label)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void initiateFederateSave(const std::wstring& label, const rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationSaved()
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationNotSaved(rti1516e::SaveFailureReason theSaveFailureReason)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationSaveStatusResponse(const rti1516e::FederateHandleSaveStatusPairVector& theFederateStatusVector)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void requestFederationRestoreSucceeded(const std::wstring& label)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void requestFederationRestoreFailed(const std::wstring& label)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationRestoreBegun()
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void initiateFederateRestore(const std::wstring& label, const std::wstring& federateName, rti1516e::FederateHandle handle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationRestored()
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationNotRestored(rti1516e::RestoreFailureReason theRestoreFailureReason)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationRestoreStatusResponse(const rti1516e::FederateRestoreStatusVector& theFederateStatusVector)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void startRegistrationForObjectClass(rti1516e::ObjectClassHandle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void stopRegistrationForObjectClass(rti1516e::ObjectClassHandle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void turnInteractionsOn(rti1516e::InteractionClassHandle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void turnInteractionsOff(rti1516e::InteractionClassHandle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void objectInstanceNameReservationSucceeded(const std::wstring&)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void objectInstanceNameReservationFailed(const std::wstring&)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void multipleObjectInstanceNameReservationSucceeded(const std::set<std::wstring>& theObjectInstanceNames)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void multipleObjectInstanceNameReservationFailed(const std::set<std::wstring>& theObjectInstanceNames)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void discoverObjectInstance(rti1516e::ObjectInstanceHandle theObject, rti1516e::ObjectClassHandle theObjectClass,
                                      std::wstring const & theObjectInstanceName)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void discoverObjectInstance(rti1516e::ObjectInstanceHandle theObject, rti1516e::ObjectClassHandle theObjectClass,
                                      std::wstring const & theObjectInstanceName, rti1516e::FederateHandle producingFederate)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    discoverObjectInstance(theObject, theObjectClass, theObjectInstanceName);
  }

  virtual void reflectAttributeValues(rti1516e::ObjectInstanceHandle theObject, rti1516e::AttributeHandleValueMap const & theAttributeValues,
                                      rti1516e::VariableLengthData const & theUserSuppliedTag, rti1516e::OrderType sentOrder,
                                      rti1516e::TransportationType theType, rti1516e::SupplementalReflectInfo theReflectInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void reflectAttributeValues(rti1516e::ObjectInstanceHandle theObject, rti1516e::AttributeHandleValueMap const & theAttributeValues,
                                      rti1516e::VariableLengthData const & theUserSuppliedTag, rti1516e::OrderType sentOrder,
                                      rti1516e::TransportationType theType, rti1516e::LogicalTime const & theTime,
                                      rti1516e::OrderType receivedOrder, rti1516e::SupplementalReflectInfo theReflectInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    reflectAttributeValues(theObject, theAttributeValues, theUserSuppliedTag, sentOrder, theType, theReflectInfo);
  }

  virtual void reflectAttributeValues(rti1516e::ObjectInstanceHandle theObject, rti1516e::AttributeHandleValueMap const & theAttributeValues,
                                      rti1516e::VariableLengthData const & theUserSuppliedTag, rti1516e::OrderType sentOrder,
                                      rti1516e::TransportationType theType, rti1516e::LogicalTime const & theTime, rti1516e::OrderType receivedOrder,
                                      rti1516e::MessageRetractionHandle theHandle, rti1516e::SupplementalReflectInfo theReflectInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    reflectAttributeValues(theObject, theAttributeValues, theUserSuppliedTag, sentOrder, theType, theTime, receivedOrder, theReflectInfo);
  }

  virtual void receiveInteraction(rti1516e::InteractionClassHandle theInteraction, rti1516e::ParameterHandleValueMap const & theParameterValues,
                                  rti1516e::VariableLengthData const & theUserSuppliedTag, rti1516e::OrderType sentOrder, rti1516e::TransportationType theType,
                                  rti1516e::SupplementalReceiveInfo theReceiveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void receiveInteraction(rti1516e::InteractionClassHandle theInteraction, rti1516e::ParameterHandleValueMap const & theParameterValues,
                                  rti1516e::VariableLengthData const & theUserSuppliedTag, rti1516e::OrderType sentOrder, rti1516e::TransportationType theType,
                                  rti1516e::LogicalTime const & theTime, rti1516e::OrderType receivedOrder, rti1516e::SupplementalReceiveInfo theReceiveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    receiveInteraction(theInteraction, theParameterValues, theUserSuppliedTag, sentOrder, theType, theReceiveInfo);
  }

  virtual void receiveInteraction(rti1516e::InteractionClassHandle theInteraction, rti1516e::ParameterHandleValueMap const & theParameterValues,
                                  rti1516e::VariableLengthData const & theUserSuppliedTag, rti1516e::OrderType sentOrder, rti1516e::TransportationType theType,
                                  rti1516e::LogicalTime const & theTime, rti1516e::OrderType receivedOrder, rti1516e::MessageRetractionHandle theHandle,
                                  rti1516e::SupplementalReceiveInfo theReceiveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    receiveInteraction(theInteraction, theParameterValues, theUserSuppliedTag, sentOrder, theType, theTime, receivedOrder, theReceiveInfo);
  }

  virtual void removeObjectInstance(rti1516e::ObjectInstanceHandle theObject, rti1516e::VariableLengthData const & theUserSuppliedTag,
                                    rti1516e::OrderType sentOrder, rti1516e::SupplementalRemoveInfo theRemoveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void removeObjectInstance(rti1516e::ObjectInstanceHandle theObject, rti1516e::VariableLengthData const & theUserSuppliedTag, rti1516e::OrderType sentOrder,
                                    rti1516e::LogicalTime const & theTime, rti1516e::OrderType receivedOrder, rti1516e::SupplementalRemoveInfo theRemoveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    removeObjectInstance(theObject, theUserSuppliedTag, sentOrder, theRemoveInfo);
  }

  virtual void removeObjectInstance(rti1516e::ObjectInstanceHandle theObject, rti1516e::VariableLengthData const & theUserSuppliedTag, rti1516e::OrderType sentOrder,
                                    rti1516e::LogicalTime const & theTime, rti1516e::OrderType receivedOrder, rti1516e::MessageRetractionHandle theHandle,
                                    rti1516e::SupplementalRemoveInfo theRemoveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    removeObjectInstance(theObject, theUserSuppliedTag, sentOrder, theTime, receivedOrder, theRemoveInfo);
  }

  virtual void attributesInScope(rti1516e::ObjectInstanceHandle theObject,
                                 rti1516e::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void attributesOutOfScope(rti1516e::ObjectInstanceHandle theObject,
                                    rti1516e::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void provideAttributeValueUpdate(rti1516e::ObjectInstanceHandle theObject,
                                           rti1516e::AttributeHandleSet const & theAttributes,
                                           rti1516e::VariableLengthData const & theUserSuppliedTag)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void turnUpdatesOnForObjectInstance(rti1516e::ObjectInstanceHandle theObject,
                                              rti1516e::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void turnUpdatesOnForObjectInstance(rti1516e::ObjectInstanceHandle theObject,
                                              rti1516e::AttributeHandleSet const & theAttributes,
                                              std::wstring const & updateRateDesignator)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void turnUpdatesOffForObjectInstance(rti1516e::ObjectInstanceHandle theObject,
                                               rti1516e::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void requestAttributeOwnershipAssumption(rti1516e::ObjectInstanceHandle theObject,
                                                   rti1516e::AttributeHandleSet const & offeredAttributes,
                                                   rti1516e::VariableLengthData const & theUserSuppliedTag)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void requestDivestitureConfirmation(rti1516e::ObjectInstanceHandle theObject,
                                              rti1516e::AttributeHandleSet const & releasedAttributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void attributeOwnershipAcquisitionNotification(rti1516e::ObjectInstanceHandle theObject,
                                                         rti1516e::AttributeHandleSet const & securedAttributes,
                                                         rti1516e::VariableLengthData const & theUserSuppliedTag)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void attributeOwnershipUnavailable(rti1516e::ObjectInstanceHandle theObject,
                                             rti1516e::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void requestAttributeOwnershipRelease(rti1516e::ObjectInstanceHandle theObject,
                                                rti1516e::AttributeHandleSet const & candidateAttributes,
                                                rti1516e::VariableLengthData const & theUserSuppliedTag)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void confirmAttributeOwnershipAcquisitionCancellation(rti1516e::ObjectInstanceHandle theObject,
                                                                rti1516e::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void informAttributeOwnership(rti1516e::ObjectInstanceHandle theObject,
                                        rti1516e::AttributeHandle theAttribute,
                                        rti1516e::FederateHandle theOwner)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void attributeIsNotOwned(rti1516e::ObjectInstanceHandle theObject,
                                   rti1516e::AttributeHandle theAttribute)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void attributeIsOwnedByRTI(rti1516e::ObjectInstanceHandle theObject,
                                     rti1516e::AttributeHandle theAttribute)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void timeRegulationEnabled(rti1516e::LogicalTime const & theFederateTime)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void timeConstrainedEnabled(rti1516e::LogicalTime const & theFederateTime)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void timeAdvanceGrant(rti1516e::LogicalTime const & theTime)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void requestRetraction(rti1516e::MessageRetractionHandle theHandle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

private:
  unsigned _synchronized;
  std::set<std::wstring> _federateSet;
};

class OPENRTI_LOCAL RTI1516ESimpleAmbassador : public rti1516e::FederateAmbassador {
public:
  RTI1516ESimpleAmbassador() :
    _useDataUrlObjectModels(false),
    // _fail(false),
    _timeRegulationEnabled(false),
    _timeConstrainedEnabled(false),
    _timeAdvancePending(false)
  { }
  virtual ~RTI1516ESimpleAmbassador()
    RTI_NOEXCEPT
  { }

  void setUseDataUrlObjectModels(bool useDataUrlObjectModels)
  { _useDataUrlObjectModels = useDataUrlObjectModels; }
  bool getUseDataUrlObjectModels() const
  { return _useDataUrlObjectModels; }

  // bool getFail() const
  // { return _fail; }

  bool getTimeRegulationEnabled() const
  { return _timeRegulationEnabled; }
  bool getTimeConstrainedEnabled() const
  { return _timeConstrainedEnabled; }
  bool getTimeAdvancePending() const
  { return _timeAdvancePending; }

  const rti1516e::FederateHandle& getFederateHandle() const
  { return _federateHandle; }

  void connect(const std::wstring& url)
  {
    rti1516e::RTIambassadorFactory factory;
    _ambassador = factory.createRTIambassador();
    _ambassador->connect(*this, rti1516e::HLA_EVOKED, url);
    setLogicalTimeFactory();
  }

  void setLogicalTimeFactory(const std::wstring& logicalTimeImplementationName = std::wstring(L"HLAinteger64Time"))
  {
    _logicalTimeImplementationName = logicalTimeImplementationName;
    _logicalTimeFactory = rti1516e::LogicalTimeFactoryFactory::makeLogicalTimeFactory(logicalTimeImplementationName);
  }

  void createFederationExecution(const std::wstring& federationExecutionName, std::wstring fddFile)
  {
    _replaceFileWithDataIfNeeded(fddFile);
    _ambassador->createFederationExecution(federationExecutionName, fddFile, _logicalTimeImplementationName);
  }
  void createFederationExecution(const std::wstring& federationExecutionName, std::vector<std::wstring> fomModules)
  {
    _replaceFilesWithDataIfNeeded(fomModules);
    _ambassador->createFederationExecution(federationExecutionName, fomModules, _logicalTimeImplementationName);
  }
  void createFederationExecutionWithMIM(const std::wstring& federationExecutionName, std::vector<std::wstring> fomModules,
                                        std::wstring mimModule)
  {
    _replaceFilesWithDataIfNeeded(fomModules);
    _replaceFileWithDataIfNeeded(mimModule);
    _ambassador->createFederationExecutionWithMIM(federationExecutionName, fomModules, mimModule, _logicalTimeImplementationName);
  }

  void destroyFederationExecution(const std::wstring& federationExecutionName)
  {
    _ambassador->destroyFederationExecution(federationExecutionName);
  }

  const rti1516e::FederateHandle& joinFederationExecution(const std::wstring& federateType,
                                                          const std::wstring& federationExecutionName,
                                                          std::vector<std::wstring> additionalFomModules = std::vector<std::wstring>())
  {
    _replaceFilesWithDataIfNeeded(additionalFomModules);
    _federateHandle = _ambassador->joinFederationExecution(federateType, federationExecutionName, additionalFomModules);
    _grantedLogicalTime = _logicalTimeFactory->makeInitial();
    return _federateHandle;
  }

  const rti1516e::FederateHandle& joinFederationExecution(const std::wstring& federateName, const std::wstring& federateType,
                                                          const std::wstring& federationExecutionName,
                                                          std::vector<std::wstring> additionalFomModules = std::vector<std::wstring>())
  {
    _replaceFilesWithDataIfNeeded(additionalFomModules);
    _federateHandle = _ambassador->joinFederationExecution(federateName, federateType, federationExecutionName, additionalFomModules);
    _grantedLogicalTime = _logicalTimeFactory->makeInitial();
    return _federateHandle;
  }

  void resignFederationExecution(rti1516e::ResignAction resignAction)
  {
    _ambassador->resignFederationExecution(resignAction);
    _federateHandle = rti1516e::FederateHandle();
  }

  void registerFederationSynchronizationPoint(const std::wstring& label, const rti1516e::VariableLengthData& tag)
  {
    _ambassador->registerFederationSynchronizationPoint(label, tag);
  }

  void registerFederationSynchronizationPoint(const std::wstring& label, const rti1516e::VariableLengthData& tag,
                                              const rti1516e::FederateHandleSet& federateHandleSet)
  {
    _ambassador->registerFederationSynchronizationPoint(label, tag, federateHandleSet);
  }

  void synchronizationPointAchieved(const std::wstring& label)
  {
    _ambassador->synchronizationPointAchieved(label);
  }

  void requestFederationSave(const std::wstring& label)
  {
    _ambassador->requestFederationSave(label);
  }

  void requestFederationSave(const std::wstring& label, const rti1516e::LogicalTime& logicalTime)
  {
    _ambassador->requestFederationSave(label, logicalTime);
  }

  void federateSaveComplete()
  {
    _ambassador->federateSaveComplete();
  }

  void federateSaveNotComplete()
  {
    _ambassador->federateSaveNotComplete();
  }

  void queryFederationSaveStatus()
  {
    _ambassador->queryFederationSaveStatus();
  }

  void requestFederationRestore(const std::wstring& label)
  {
    _ambassador->requestFederationRestore(label);
  }

  void federateRestoreComplete()
  {
    _ambassador->federateRestoreComplete();
  }

  void federateRestoreNotComplete()
  {
    _ambassador->federateRestoreNotComplete();
  }

  void queryFederationRestoreStatus()
  {
    _ambassador->queryFederationRestoreStatus();
  }

  void publishObjectClassAttributes(const rti1516e::ObjectClassHandle& objectClassHandle,
                                    const rti1516e::AttributeHandleSet& attributeList)
  {
    _ambassador->publishObjectClassAttributes(objectClassHandle, attributeList);
  }

  void unpublishObjectClass(const rti1516e::ObjectClassHandle& objectClassHandle)
  {
    _ambassador->unpublishObjectClass(objectClassHandle);
  }

  void unpublishObjectClassAttributes(const rti1516e::ObjectClassHandle& objectClassHandle,
                                      const rti1516e::AttributeHandleSet& attributeList)
  {
    _ambassador->unpublishObjectClassAttributes(objectClassHandle, attributeList);
  }

  void publishInteractionClass(const rti1516e::InteractionClassHandle& interactionClassHandle)
  {
    _ambassador->publishInteractionClass(interactionClassHandle);
  }

  void unpublishInteractionClass(const rti1516e::InteractionClassHandle& interactionClassHandle)
  {
    _ambassador->unpublishInteractionClass(interactionClassHandle);
  }

  void subscribeObjectClassAttributes(const rti1516e::ObjectClassHandle& objectClassHandle,
                                      const rti1516e::AttributeHandleSet& attributeHandleSet,
                                      bool active = true)
  {
      _ambassador->subscribeObjectClassAttributes(objectClassHandle, attributeHandleSet, active);
      // _subscribedObjectClassAttributeHandleSetMap[objectClassHandle].insert(attributeHandleSet.begin(), attributeHandleSet.end());
  }

  void unsubscribeObjectClass(const rti1516e::ObjectClassHandle& objectClassHandle)
  {
      _ambassador->unsubscribeObjectClass(objectClassHandle);
      // _subscribedObjectClassAttributeHandleSetMap.erase(objectClassHandle);
  }

  void unsubscribeObjectClassAttributes(const rti1516e::ObjectClassHandle& objectClassHandle,
                                        const rti1516e::AttributeHandleSet& attributeHandleSet)
  {
      _ambassador->unsubscribeObjectClassAttributes(objectClassHandle, attributeHandleSet);
      // ObjectClassAttributeHandleSetMap::iterator i = _subscribedObjectClassAttributeHandleSetMap.find(objectClassHandle);
      // for (rti1516e::AttributeHandleSet::const_iterator j = attributeHandleSet.begin();
      //      j != attributeHandleSet.end(); ++j)
      //     i->second.erase(*j);
  }

  void subscribeInteractionClass(const rti1516e::InteractionClassHandle& interactionClassHandle, bool active = true)
  {
      _ambassador->subscribeInteractionClass(interactionClassHandle, active);
  }

  void unsubscribeInteractionClass(const rti1516e::InteractionClassHandle& interactionClassHandle)
  {
      _ambassador->unsubscribeInteractionClass(interactionClassHandle);
  }

  void reserveObjectInstanceName(const std::wstring& objectInstanceName)
  {
      _ambassador->reserveObjectInstanceName(objectInstanceName);
  }

  rti1516e::ObjectInstanceHandle registerObjectInstance(const rti1516e::ObjectClassHandle& objectClassHandle)
  {
      return _ambassador->registerObjectInstance(objectClassHandle);
  }

  rti1516e::ObjectInstanceHandle registerObjectInstance(const rti1516e::ObjectClassHandle& objectClassHandle,
                                                       const std::wstring& objectInstanceName)
  {
      return _ambassador->registerObjectInstance(objectClassHandle, objectInstanceName);
  }

  void updateAttributeValues(const rti1516e::ObjectInstanceHandle& objectInstanceHandle,
                             const rti1516e::AttributeHandleValueMap& attributeHandleValueMap,
                             const rti1516e::VariableLengthData& tag)
  {
      _ambassador->updateAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag);
  }

  rti1516e::MessageRetractionHandle updateAttributeValues(const rti1516e::ObjectInstanceHandle& objectInstanceHandle,
                                                         const rti1516e::AttributeHandleValueMap& attributeHandleValueMap,
                                                         const rti1516e::VariableLengthData& tag,
                                                         const rti1516e::LogicalTime& logicalTime)
  {
    return _ambassador->updateAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag, logicalTime);
  }

  void sendInteraction(const rti1516e::InteractionClassHandle& interactionClassHandle,
                       const rti1516e::ParameterHandleValueMap& parameterHandleValueMap,
                       const rti1516e::VariableLengthData& tag)
  {
      _ambassador->sendInteraction(interactionClassHandle, parameterHandleValueMap, tag);
  }

  rti1516e::MessageRetractionHandle sendInteraction(const rti1516e::InteractionClassHandle& interactionClassHandle,
                                                   const rti1516e::ParameterHandleValueMap& parameterHandleValueMap,
                                                   const rti1516e::VariableLengthData& tag,
                                                   const rti1516e::LogicalTime& logicalTime)
  {
      return _ambassador->sendInteraction(interactionClassHandle, parameterHandleValueMap, tag, logicalTime);
  }

  void deleteObjectInstance(const rti1516e::ObjectInstanceHandle& objectInstanceHandle,
                            const rti1516e::VariableLengthData& tag)
  {
    _ambassador->deleteObjectInstance(objectInstanceHandle, tag);
  }

  rti1516e::MessageRetractionHandle deleteObjectInstance(const rti1516e::ObjectInstanceHandle& objectInstanceHandle,
                                                        const rti1516e::VariableLengthData& tag,
                                                        const rti1516e::LogicalTime& logicalTime)
  {
    return _ambassador->deleteObjectInstance(objectInstanceHandle, tag, logicalTime);
  }

  void localDeleteObjectInstance(const rti1516e::ObjectInstanceHandle& objectInstanceHandle)
  {
    _ambassador->localDeleteObjectInstance(objectInstanceHandle);
  }

  void requestAttributeTransportationTypeChange(const rti1516e::ObjectInstanceHandle& objectInstanceHandle,
                                                const rti1516e::AttributeHandleSet& attributeHandleSet,
                                                const rti1516e::TransportationType& transportationType)
  {
    _ambassador->requestAttributeTransportationTypeChange(objectInstanceHandle, attributeHandleSet, transportationType);
  }

  void requestInteractionTransportationTypeChange(const rti1516e::InteractionClassHandle& interactionClassHandle,
                                                  const rti1516e::TransportationType& transportationType)
  {
    _ambassador->requestInteractionTransportationTypeChange(interactionClassHandle, transportationType);
  }

  void requestAttributeValueUpdate(const rti1516e::ObjectInstanceHandle& objectInstanceHandle,
                                   const rti1516e::AttributeHandleSet& attributeHandleSet,
                                   const rti1516e::VariableLengthData& tag)
  {
    _ambassador->requestAttributeValueUpdate(objectInstanceHandle, attributeHandleSet, tag);
  }

  void requestAttributeValueUpdate(const rti1516e::ObjectClassHandle& objectClassHandle,
                                   const rti1516e::AttributeHandleSet& attributeHandleSet,
                                   const rti1516e::VariableLengthData& tag)
  {
    _ambassador->requestAttributeValueUpdate(objectClassHandle, attributeHandleSet, tag);
  }

  void unconditionalAttributeOwnershipDivestiture(const rti1516e::ObjectInstanceHandle& objectInstanceHandle,
                                                  const rti1516e::AttributeHandleSet& attributeHandleSet)
  {
    _ambassador->unconditionalAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet);
  }

    // // 7.3
    //  void negotiatedAttributeOwnershipDivestiture
    // (ObjectInstanceHandle objectInstanceHandle,
    //  AttributeHandleSet const & attributes,
    //  VariableLengthData const & tag)

    // // 7.6
    //  void confirmDivestiture
    // (ObjectInstanceHandle objectInstanceHandle,
    //  AttributeHandleSet const & confirmedAttributes,
    //  VariableLengthData const & tag)

    // // 7.8
    //  void attributeOwnershipAcquisition
    // (ObjectInstanceHandle objectInstanceHandle,
    //  AttributeHandleSet const & desiredAttributes,
    //  VariableLengthData const & tag)

    // // 7.9
    //  void attributeOwnershipAcquisitionIfAvailable
    // (ObjectInstanceHandle objectInstanceHandle,
    //  AttributeHandleSet const & desiredAttributes)

    // // 7.12
    //  void attributeOwnershipDivestitureIfWanted
    // (ObjectInstanceHandle objectInstanceHandle,
    //  AttributeHandleSet const & attributes,
    //  AttributeHandleSet & theDivestedAttributes) // filled by RTI

    // // 7.13
    //  void cancelNegotiatedAttributeOwnershipDivestiture
    // (ObjectInstanceHandle objectInstanceHandle,
    //  AttributeHandleSet const & attributes)

    // // 7.14
    //  void cancelAttributeOwnershipAcquisition
    // (ObjectInstanceHandle objectInstanceHandle,
    //  AttributeHandleSet const & attributes)

    // // 7.16
    //  void queryAttributeOwnership
    // (ObjectInstanceHandle objectInstanceHandle,
    //  AttributeHandle attribute)

    // // 7.18
    //  bool isAttributeOwnedByFederate
    // (ObjectInstanceHandle objectInstanceHandle,
    //  AttributeHandle attribute)

  void enableTimeRegulation(const rti1516e::LogicalTimeInterval& logicalTimeInterval)
  {
    _ambassador->enableTimeRegulation(logicalTimeInterval);
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

  void timeAdvanceRequest(const rti1516e::LogicalTime& logicalTime)
  {
    _timeAdvancePending = true;
    _ambassador->timeAdvanceRequest(logicalTime);
  }

  void timeAdvanceRequestAvailable(const rti1516e::LogicalTime& logicalTime)
  {
    _timeAdvancePending = true;
    _ambassador->timeAdvanceRequestAvailable(logicalTime);
  }

  void nextMessageRequest(const rti1516e::LogicalTime& logicalTime)
  {
    _timeAdvancePending = true;
    _ambassador->nextMessageRequest(logicalTime);
  }

  void nextMessageRequestAvailable(const rti1516e::LogicalTime& logicalTime)
  {
    _timeAdvancePending = true;
    _ambassador->nextMessageRequestAvailable(logicalTime);
  }

  void flushQueueRequest(const rti1516e::LogicalTime& logicalTime)
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

    //  bool queryGALT (LogicalTime & logicalTime)

    //  void queryLogicalTime (LogicalTime & logicalTime)

    //  bool queryLITS (LogicalTime & logicalTime)

    //  void modifyLookahead

    //  void queryLookahead (LogicalTimeInterval & interval)

    //  void retract
    // (MessageRetractionHandle theHandle)

    //  void changeAttributeOrderType
    // (ObjectInstanceHandle objectInstanceHandle,
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

    //  ObjectInstanceHandle registerObjectInstanceWithRegions
    // (ObjectClassHandle theClass,
    //  AttributeHandleSetRegionHandleSetPairVector const &
    //  attributeHandleSetRegionHandleSetPairVector)

    //  ObjectInstanceHandle registerObjectInstanceWithRegions
    // (ObjectClassHandle theClass,
    //  AttributeHandleSetRegionHandleSetPairVector const &
    //  attributeHandleSetRegionHandleSetPairVector,
    //  std::wstring const & objectInstanceName)

    //  void associateRegionsForUpdates
    // (ObjectInstanceHandle objectInstanceHandle,
    //  AttributeHandleSetRegionHandleSetPairVector const &
    //  attributeHandleSetRegionHandleSetPairVector)

    //  void unassociateRegionsForUpdates
    // (ObjectInstanceHandle objectInstanceHandle,
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
    //  ParameterHandleValueMap const & parameterValues,
    //  RegionHandleSet const & regionHandleSet,
    //  VariableLengthData const & tag)

    //  MessageRetractionHandle sendInteractionWithRegions
    // (InteractionClassHandle interaction,
    //  ParameterHandleValueMap const & parameterValues,
    //  RegionHandleSet const & regionHandleSet,
    //  VariableLengthData const & tag,
    //  LogicalTime const & logicalTime)

    //  void requestAttributeValueUpdateWithRegions
    // (ObjectClassHandle theClass,
    //  AttributeHandleSetRegionHandleSetPairVector const & theSet,
    //  VariableLengthData const & tag)

  rti1516e::ObjectClassHandle getObjectClassHandle(std::wstring const & theName)
  {
    return _ambassador->getObjectClassHandle(theName);
  }

  std::wstring getObjectClassName(rti1516e::ObjectClassHandle theHandle)
  {
    return _ambassador->getObjectClassName(theHandle);
  }

  rti1516e::AttributeHandle getAttributeHandle(rti1516e::ObjectClassHandle whichClass, std::wstring const & attributeName)
  {
    return _ambassador->getAttributeHandle(whichClass, attributeName);
  }

  std::wstring getAttributeName(rti1516e::ObjectClassHandle whichClass, rti1516e::AttributeHandle theHandle)
  {
    return _ambassador->getAttributeName(whichClass, theHandle);
  }

  double getUpdateRateValue(std::wstring const & updateRateDesignator)
  {
    return _ambassador->getUpdateRateValue(updateRateDesignator);
  }

  double getUpdateRateValueForAttribute(rti1516e::ObjectInstanceHandle objectInstanceHandle, rti1516e::AttributeHandle attributeHandle)
  {
    return _ambassador->getUpdateRateValueForAttribute(objectInstanceHandle, attributeHandle);
  }

  rti1516e::InteractionClassHandle getInteractionClassHandle(std::wstring const & theName)
  {
    return _ambassador->getInteractionClassHandle(theName);
  }

  std::wstring getInteractionClassName(rti1516e::InteractionClassHandle theHandle)
  {
    return _ambassador->getInteractionClassName(theHandle);
  }

  rti1516e::ParameterHandle getParameterHandle(rti1516e::InteractionClassHandle whichClass, std::wstring const & theName)
  {
    return _ambassador->getParameterHandle(whichClass, theName);
  }

  std::wstring getParameterName(rti1516e::InteractionClassHandle whichClass, rti1516e::ParameterHandle theHandle)
  {
    return _ambassador->getParameterName(whichClass, theHandle);
  }

  rti1516e::ObjectInstanceHandle getObjectInstanceHandle(std::wstring const & theName)
  {
    return _ambassador->getObjectInstanceHandle(theName);
  }

  std::wstring getObjectInstanceName(rti1516e::ObjectInstanceHandle theHandle)
  {
    return _ambassador->getObjectInstanceName(theHandle);
  }

  rti1516e::DimensionHandle getDimensionHandle(std::wstring const & theName)
  {
    return _ambassador->getDimensionHandle(theName);
  }

  std::wstring getDimensionName(rti1516e::DimensionHandle theHandle)
  {
    return _ambassador->getDimensionName(theHandle);
  }

  unsigned long getDimensionUpperBound(rti1516e::DimensionHandle theHandle)
  {
    return _ambassador->getDimensionUpperBound(theHandle);
  }

  rti1516e::DimensionHandleSet getAvailableDimensionsForClassAttribute(rti1516e::ObjectClassHandle theClass,
                                                                      rti1516e::AttributeHandle theHandle)
  {
    return _ambassador->getAvailableDimensionsForClassAttribute(theClass, theHandle);
  }

  rti1516e::ObjectClassHandle getKnownObjectClassHandle(rti1516e::ObjectInstanceHandle object)
  {
    return _ambassador->getKnownObjectClassHandle(object);
  }

  rti1516e::DimensionHandleSet getAvailableDimensionsForInteractionClass(rti1516e::InteractionClassHandle theClass)
  {
    return _ambassador->getAvailableDimensionsForInteractionClass(theClass);
  }

  rti1516e::TransportationType getTransportationType(std::wstring const & transportationName)
  {
    return _ambassador->getTransportationType(transportationName);
  }

  std::wstring getTransportationName(rti1516e::TransportationType transportationType)
  {
    return _ambassador->getTransportationName(transportationType);
  }

  rti1516e::OrderType getOrderType(std::wstring const & orderName)
  {
    return _ambassador->getOrderType(orderName);
  }

  std::wstring getOrderName(rti1516e::OrderType orderType)
  {
    return _ambassador->getOrderName(orderType);
  }

  void enableObjectClassRelevanceAdvisorySwitch()
  {
    _ambassador->enableObjectClassRelevanceAdvisorySwitch();
  }

  void disableObjectClassRelevanceAdvisorySwitch()
  {
    _ambassador->disableObjectClassRelevanceAdvisorySwitch();
  }

  void enableAttributeRelevanceAdvisorySwitch()
  {
    _ambassador->enableAttributeRelevanceAdvisorySwitch();
  }

  void disableAttributeRelevanceAdvisorySwitch()
  {
    _ambassador->disableAttributeRelevanceAdvisorySwitch();
  }

  void enableAttributeScopeAdvisorySwitch()
  {
    _ambassador->enableAttributeScopeAdvisorySwitch();
  }

  void disableAttributeScopeAdvisorySwitch()
  {
    _ambassador->disableAttributeScopeAdvisorySwitch();
  }

  void enableInteractionRelevanceAdvisorySwitch()
  {
    _ambassador->enableInteractionRelevanceAdvisorySwitch();
  }

  void disableInteractionRelevanceAdvisorySwitch()
  {
    _ambassador->disableInteractionRelevanceAdvisorySwitch();
  }

  rti1516e::DimensionHandleSet getDimensionHandleSet(rti1516e::RegionHandle regionHandle)
  {
    return _ambassador->getDimensionHandleSet(regionHandle);
  }

  rti1516e::RangeBounds getRangeBounds(rti1516e::RegionHandle regionHandle, rti1516e::DimensionHandle theDimensionHandle)
  {
    return _ambassador->getRangeBounds(regionHandle, theDimensionHandle);
  }

  void setRangeBounds(rti1516e::RegionHandle regionHandle, rti1516e::DimensionHandle theDimensionHandle,
                      rti1516e::RangeBounds const & rangeBounds)
  {
    return _ambassador->setRangeBounds(regionHandle, theDimensionHandle, rangeBounds);
  }

  unsigned long normalizeFederateHandle(rti1516e::FederateHandle federateHandle)
  {
    return _ambassador->normalizeFederateHandle(federateHandle);
  }

  unsigned long normalizeServiceGroup(rti1516e::ServiceGroup theServiceGroup)
  {
    return _ambassador->normalizeServiceGroup(theServiceGroup);
  }

  bool evokeCallback(double approximateMinimumTimeInSeconds)
  {
    return _ambassador->evokeCallback(approximateMinimumTimeInSeconds);
  }

  bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                              double approximateMaximumTimeInSeconds)
  {
    return _ambassador->evokeMultipleCallbacks(approximateMinimumTimeInSeconds, approximateMaximumTimeInSeconds);
  }

  void enableCallbacks()
  {
    _ambassador->enableCallbacks();
  }

  void disableCallbacks()
  {
    _ambassador->disableCallbacks();
  }

  rti1516e::FederateHandle decodeFederateHandle(rti1516e::VariableLengthData const& encodedValue) const
  {
    return _ambassador->decodeFederateHandle(encodedValue);
  }

  rti1516e::ObjectClassHandle decodeObjectClassHandle(rti1516e::VariableLengthData const & encodedValue) const
  {
    return _ambassador->decodeObjectClassHandle(encodedValue);
  }

  rti1516e::InteractionClassHandle decodeInteractionClassHandle(rti1516e::VariableLengthData const & encodedValue) const
  {
    return _ambassador->decodeInteractionClassHandle(encodedValue);
  }

  rti1516e::ObjectInstanceHandle decodeObjectInstanceHandle(rti1516e::VariableLengthData const & encodedValue) const
  {
    return _ambassador->decodeObjectInstanceHandle(encodedValue);
  }

  rti1516e::AttributeHandle decodeAttributeHandle(rti1516e::VariableLengthData const & encodedValue) const
  {
    return _ambassador->decodeAttributeHandle(encodedValue);
  }

  rti1516e::ParameterHandle decodeParameterHandle(rti1516e::VariableLengthData const & encodedValue) const
  {
    return _ambassador->decodeParameterHandle(encodedValue);
  }

  rti1516e::DimensionHandle decodeDimensionHandle(rti1516e::VariableLengthData const & encodedValue) const
  {
    return _ambassador->decodeDimensionHandle(encodedValue);
  }

  rti1516e::MessageRetractionHandle decodeMessageRetractionHandle(rti1516e::VariableLengthData const & encodedValue) const
  {
    return _ambassador->decodeMessageRetractionHandle(encodedValue);
  }

  rti1516e::RegionHandle decodeRegionHandle(rti1516e::VariableLengthData const & encodedValue) const
  {
    return _ambassador->decodeRegionHandle(encodedValue);
  }

protected:
  virtual void connectionLost(const std::wstring& faultDescription)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void reportFederationExecutions(const rti1516e::FederationExecutionInformationVector& theFederationExecutionInformationList)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void synchronizationPointRegistrationSucceeded(const std::wstring& label)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void synchronizationPointRegistrationFailed(const std::wstring& label, rti1516e::SynchronizationPointFailureReason reason)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void announceSynchronizationPoint(const std::wstring& label, const rti1516e::VariableLengthData& tag)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationSynchronized(const std::wstring& label, const rti1516e::FederateHandleSet& failedToSyncSet)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void initiateFederateSave(const std::wstring& label)
      RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void initiateFederateSave(const std::wstring& label, const rti1516e::LogicalTime& logicalTime)
      RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationSaved()
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationNotSaved(rti1516e::SaveFailureReason theSaveFailureReason)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationSaveStatusResponse(const rti1516e::FederateHandleSaveStatusPairVector& federateStatusVector)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void requestFederationRestoreSucceeded(const std::wstring& label)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void requestFederationRestoreFailed(const std::wstring& label)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationRestoreBegun()
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void initiateFederateRestore(const std::wstring& label, const std::wstring& federateName, rti1516e::FederateHandle handle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationRestored()
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationNotRestored(rti1516e::RestoreFailureReason restoreFailureReason)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void federationRestoreStatusResponse(const rti1516e::FederateRestoreStatusVector& federateStatusVector)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void startRegistrationForObjectClass(rti1516e::ObjectClassHandle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void stopRegistrationForObjectClass(rti1516e::ObjectClassHandle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void turnInteractionsOn(rti1516e::InteractionClassHandle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void turnInteractionsOff(rti1516e::InteractionClassHandle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void objectInstanceNameReservationSucceeded(const std::wstring&)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void objectInstanceNameReservationFailed(const std::wstring&)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void multipleObjectInstanceNameReservationSucceeded(const std::set<std::wstring>& theObjectInstanceNames)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void multipleObjectInstanceNameReservationFailed(const std::set<std::wstring>& theObjectInstanceNames)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void discoverObjectInstance(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                      rti1516e::ObjectClassHandle objectClassHandle,
                                      const std::wstring& objectInstanceName)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
      // ObjectClassAttributeHandleSetMap::iterator i = _subscribedObjectClassAttributeHandleSetMap.find(objectClassHandle);
      // if (i == _subscribedObjectClassAttributeHandleSetMap.end()) {
      //     fail();
      //     throw rti1516e::FederateInternalError(objectClassHandle.toString());
      // }

      // if (_objectInstanceMap.find(objectInstanceHandle) != _objectInstanceMap.end()) {
      //     fail();
      //     throw rti1516e::FederateInternalError(objectInstanceHandle.toString());
      // }

      // _objectInstanceMap[objectInstanceHandle]._objectClassHandle = objectClassHandle;
  }

  virtual void discoverObjectInstance(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                      rti1516e::ObjectClassHandle objectClassHandle, const std::wstring& objectInstanceName,
                                      rti1516e::FederateHandle producingFederate)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    discoverObjectInstance(objectInstanceHandle, objectClassHandle, objectInstanceName);
  }

  virtual void reflectAttributeValues(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                      const rti1516e::AttributeHandleValueMap& attributeHandleValueMap,
                                      const rti1516e::VariableLengthData& tag, rti1516e::OrderType, rti1516e::TransportationType,
                                      rti1516e::SupplementalReflectInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
      // _verifyReflectAttributeValues(objectInstanceHandle, attributeHandleValueMap);
  }

  virtual void reflectAttributeValues(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                      const rti1516e::AttributeHandleValueMap& attributeHandleValueMap,
                                      const rti1516e::VariableLengthData&, rti1516e::OrderType, rti1516e::TransportationType,
                                      const rti1516e::RegionHandleSet&, rti1516e::SupplementalReflectInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
      // _verifyReflectAttributeValues(objectInstanceHandle, attributeHandleValueMap);
  }

  virtual void reflectAttributeValues(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                      const rti1516e::AttributeHandleValueMap& attributeHandleValueMap,
                                      const rti1516e::VariableLengthData&, rti1516e::OrderType, rti1516e::TransportationType,
                                      const rti1516e::LogicalTime&, rti1516e::OrderType, rti1516e::SupplementalReflectInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
      // _verifyReflectAttributeValues(objectInstanceHandle, attributeHandleValueMap);
  }

  virtual void reflectAttributeValues(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                      const rti1516e::AttributeHandleValueMap& attributeHandleValueMap,
                                      const rti1516e::VariableLengthData&, rti1516e::OrderType, rti1516e::TransportationType,
                                      const rti1516e::LogicalTime&, rti1516e::OrderType, const rti1516e::RegionHandleSet&,
                                      rti1516e::SupplementalReflectInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
      // _verifyReflectAttributeValues(objectInstanceHandle, attributeHandleValueMap);
  }

  virtual void reflectAttributeValues(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                      const rti1516e::AttributeHandleValueMap& attributeHandleValueMap,
                                      const rti1516e::VariableLengthData&, rti1516e::OrderType, rti1516e::TransportationType,
                                      const rti1516e::LogicalTime&, rti1516e::OrderType, rti1516e::MessageRetractionHandle,
                                      rti1516e::SupplementalReflectInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
      // _verifyReflectAttributeValues(objectInstanceHandle, attributeHandleValueMap);
  }

  virtual void reflectAttributeValues(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                      const rti1516e::AttributeHandleValueMap& attributeHandleValueMap,
                                      const rti1516e::VariableLengthData&, rti1516e::OrderType, rti1516e::TransportationType,
                                      const rti1516e::LogicalTime&, rti1516e::OrderType, rti1516e::MessageRetractionHandle,
                                      const rti1516e::RegionHandleSet&, rti1516e::SupplementalReflectInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
      // _verifyReflectAttributeValues(objectInstanceHandle, attributeHandleValueMap);
  }

  virtual void receiveInteraction(rti1516e::InteractionClassHandle, const rti1516e::ParameterHandleValueMap&,
                                  const rti1516e::VariableLengthData&, rti1516e::OrderType, rti1516e::TransportationType,
                                  rti1516e::SupplementalReceiveInfo theReceiveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void receiveInteraction(rti1516e::InteractionClassHandle, const rti1516e::ParameterHandleValueMap&,
                                  const rti1516e::VariableLengthData&, rti1516e::OrderType, rti1516e::TransportationType,
                                  const rti1516e::RegionHandleSet&, rti1516e::SupplementalReceiveInfo theReceiveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void receiveInteraction(rti1516e::InteractionClassHandle interaction,
                                  rti1516e::ParameterHandleValueMap const & parameterValues,
                                  rti1516e::VariableLengthData const & tag,
                                  rti1516e::OrderType sentOrder,
                                  rti1516e::TransportationType theType,
                                  rti1516e::LogicalTime const & logicalTime,
                                  rti1516e::OrderType receivedOrder,
                                  rti1516e::SupplementalReceiveInfo theReceiveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void receiveInteraction(rti1516e::InteractionClassHandle interaction,
                                  rti1516e::ParameterHandleValueMap const & parameterValues,
                                  rti1516e::VariableLengthData const & tag,
                                  rti1516e::OrderType sentOrder,
                                  rti1516e::TransportationType theType,
                                  rti1516e::LogicalTime const & logicalTime,
                                  rti1516e::OrderType receivedOrder,
                                  rti1516e::RegionHandleSet const & theSentRegionHandleSet,
                                  rti1516e::SupplementalReceiveInfo theReceiveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void receiveInteraction(rti1516e::InteractionClassHandle interaction,
                                  rti1516e::ParameterHandleValueMap const & parameterValues,
                                  rti1516e::VariableLengthData const & tag,
                                  rti1516e::OrderType sentOrder,
                                  rti1516e::TransportationType theType,
                                  rti1516e::LogicalTime const & logicalTime,
                                  rti1516e::OrderType receivedOrder,
                                  rti1516e::MessageRetractionHandle theHandle,
                                  rti1516e::SupplementalReceiveInfo theReceiveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void receiveInteraction(rti1516e::InteractionClassHandle interaction,
                                  rti1516e::ParameterHandleValueMap const & parameterValues,
                                  rti1516e::VariableLengthData const & tag,
                                  rti1516e::OrderType sentOrder,
                                  rti1516e::TransportationType theType,
                                  rti1516e::LogicalTime const & logicalTime,
                                  rti1516e::OrderType receivedOrder,
                                  rti1516e::MessageRetractionHandle theHandle,
                                  rti1516e::RegionHandleSet const & theSentRegionHandleSet,
                                  rti1516e::SupplementalReceiveInfo theReceiveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void removeObjectInstance(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                    rti1516e::VariableLengthData const & tag,
                                    rti1516e::OrderType sentOrder,
                                    rti1516e::SupplementalRemoveInfo theRemoveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
      // _verifyRemoveObjectInstance(objectInstanceHandle);
  }

  virtual void removeObjectInstance(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                    rti1516e::VariableLengthData const & tag,
                                    rti1516e::OrderType sentOrder,
                                    rti1516e::LogicalTime const & logicalTime,
                                    rti1516e::OrderType receivedOrder,
                                    rti1516e::SupplementalRemoveInfo theRemoveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
      // _verifyRemoveObjectInstance(objectInstanceHandle);
  }

  virtual void removeObjectInstance(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                    rti1516e::VariableLengthData const & tag,
                                    rti1516e::OrderType sentOrder,
                                    rti1516e::LogicalTime const & logicalTime,
                                    rti1516e::OrderType receivedOrder,
                                    rti1516e::MessageRetractionHandle theHandle,
                                    rti1516e::SupplementalRemoveInfo theRemoveInfo)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
      // _verifyRemoveObjectInstance(objectInstanceHandle);
  }

  virtual void attributesInScope(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                 rti1516e::AttributeHandleSet const & attributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void attributesOutOfScope(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                    rti1516e::AttributeHandleSet const & attributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void provideAttributeValueUpdate(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                           rti1516e::AttributeHandleSet const & attributes,
                                           rti1516e::VariableLengthData const & tag)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void turnUpdatesOnForObjectInstance(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                              rti1516e::AttributeHandleSet const & attributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void turnUpdatesOnForObjectInstance(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                              rti1516e::AttributeHandleSet const & attributes,
                                              const std::wstring& updateRateDesignator)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void turnUpdatesOffForObjectInstance(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                               rti1516e::AttributeHandleSet const & attributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void confirmAttributeTransportationTypeChange(rti1516e::ObjectInstanceHandle theObject,
                                                        rti1516e::AttributeHandleSet theAttributes,
                                                        rti1516e::TransportationType theTransportation)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void reportAttributeTransportationType(rti1516e::ObjectInstanceHandle theObject,
                                                 rti1516e::AttributeHandle theAttribute,
                                                 rti1516e::TransportationType theTransportation)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void confirmInteractionTransportationTypeChange(rti1516e::InteractionClassHandle theInteraction,
                                                          rti1516e::TransportationType theTransportation)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void reportInteractionTransportationType(rti1516e::FederateHandle federateHandle,
                                                   rti1516e::InteractionClassHandle theInteraction,
                                                   rti1516e::TransportationType theTransportation)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void requestAttributeOwnershipAssumption(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                                   rti1516e::AttributeHandleSet const & offeredAttributes,
                                                   rti1516e::VariableLengthData const & tag)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void requestDivestitureConfirmation(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                              rti1516e::AttributeHandleSet const & releasedAttributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void attributeOwnershipAcquisitionNotification(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                                         rti1516e::AttributeHandleSet const & securedAttributes,
                                                         rti1516e::VariableLengthData const & tag)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void attributeOwnershipUnavailable(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                             rti1516e::AttributeHandleSet const & attributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void requestAttributeOwnershipRelease(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                                rti1516e::AttributeHandleSet const & candidateAttributes,
                                                rti1516e::VariableLengthData const & tag)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void confirmAttributeOwnershipAcquisitionCancellation(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                                                rti1516e::AttributeHandleSet const & attributes)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void informAttributeOwnership(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                        rti1516e::AttributeHandle attribute,
                                        rti1516e::FederateHandle owner)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void attributeIsNotOwned(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                   rti1516e::AttributeHandle attribute)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void attributeIsOwnedByRTI(rti1516e::ObjectInstanceHandle objectInstanceHandle,
                                     rti1516e::AttributeHandle attribute)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  virtual void timeRegulationEnabled(const rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    _timeRegulationEnabled = true;
    *_grantedLogicalTime = logicalTime;
  }

  virtual void timeConstrainedEnabled(const rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    _timeConstrainedEnabled = true;
    *_grantedLogicalTime = logicalTime;
  }

  virtual void timeAdvanceGrant(const rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
    _timeAdvancePending = false;
    *_grantedLogicalTime = logicalTime;
  }

  virtual void requestRetraction(rti1516e::MessageRetractionHandle theHandle)
    RTI_THROW ((rti1516e::FederateInternalError))
  {
  }

  // void fail()
  // { _fail = true; }

private:
  void _replaceFileWithDataIfNeeded(std::wstring& fddFile)
  {
    if (!_useDataUrlObjectModels)
      return;
    // already a data url
    if (fddFile.compare(0, 5, L"data:") == 0)
      return;
    std::wifstream stream;
    if (fddFile.compare(0, 8, L"file:///") == 0)
      stream.open(OpenRTI::ucsToLocale(fddFile.substr(8)).c_str());
    else
      stream.open(OpenRTI::ucsToLocale(fddFile).c_str());
    if (!stream.is_open())
      return;
    fddFile = L"data:,";
    std::copy(std::istreambuf_iterator<wchar_t>(stream), std::istreambuf_iterator<wchar_t>(), std::back_inserter(fddFile));
  }

  void _replaceFilesWithDataIfNeeded(std::vector<std::wstring>& fomModules)
  {
    for (std::vector<std::wstring>::iterator i = fomModules.begin(); i != fomModules.end(); ++i)
      _replaceFileWithDataIfNeeded(*i);
  }

  // bool _verifyGrantedLogicalTime(const rti1516e::LogicalTime& logicalTime) const
  // { return logicalTime == *_grantedLogicalTime; }

  // void _verifyReflectAttributeValues(const rti1516e::ObjectInstanceHandle& objectInstanceHandle,
  //                                    const rti1516e::AttributeHandleValueMap& attributeHandleValueMap)
  // {
  //     ObjectInstanceMap::iterator i = _objectInstanceMap.find(objectInstanceHandle);
  //     if (i == _objectInstanceMap.end()) {
  //         fail();
  //         throw rti1516e::ObjectInstanceNotKnown(objectInstanceHandle.toString());
  //     }

  //     ObjectClassAttributeHandleSetMap::iterator j = _subscribedObjectClassAttributeHandleSetMap.find(i->second._objectClassHandle);
  //     for (rti1516e::AttributeHandleValueMap::const_iterator k = attributeHandleValueMap.begin();
  //          k != attributeHandleValueMap.end(); ++k) {
  //         if (j->second.find(k->first) != j->second.end())
  //             continue;
  //         fail();
  //         throw rti1516e::AttributeNotSubscribed(k->first.toString());
  //     }
  // }

  // void _verifyRemoveObjectInstance(rti1516e::ObjectInstanceHandle objectInstanceHandle)
  // {
  //     if (_objectInstanceMap.find(objectInstanceHandle) == _objectInstanceMap.end()) {
  //         fail();
  //         throw rti1516e::ObjectInstanceNotKnown(objectInstanceHandle.toString());
  //     }

  //     _objectInstanceMap.erase(objectInstanceHandle);
  // }

  // bool _fail;

  RTI_UNIQUE_PTR<rti1516e::RTIambassador> _ambassador;

  bool _useDataUrlObjectModels;

  std::wstring _logicalTimeImplementationName;
  RTI_UNIQUE_PTR<rti1516e::LogicalTimeFactory> _logicalTimeFactory;

  rti1516e::FederateHandle _federateHandle;

  RTI_UNIQUE_PTR<rti1516e::LogicalTime> _grantedLogicalTime;
  bool _timeRegulationEnabled;
  bool _timeConstrainedEnabled;
  bool _timeAdvancePending;

  // Hmm, FIXME: make an additional derived checking ambassador for the tests, keep a simple one without expensive tests
  // FIXME make this and for example the simple log below callbacks that we can attach or not as apropriate

  // // FIXME implement subscription tracking also for regions at some time
  // typedef std::map<rti1516e::ObjectClassHandle, rti1516e::AttributeHandleSet> ObjectClassAttributeHandleSetMap;
  // ObjectClassAttributeHandleSetMap _subscribedObjectClassAttributeHandleSetMap;

  // struct ObjectInstance {
  //   rti1516e::ObjectClassHandle _objectClassHandle;
  // };

  // typedef std::map<rti1516e::ObjectInstanceHandle, ObjectInstance> ObjectInstanceMap;
  // ObjectInstanceMap _objectInstanceMap;
};

}

#endif
