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

#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include <RTI/HLAinteger64Time.h>
#include <RTI/HLAinteger64Interval.h>

#include <Options.h>
#include <StringUtils.h>
#include <Thread.h>

#include <RTI1516TestLib.h>

using namespace OpenRTI;

class TestAmbassador : public RTI1516TestAmbassador {
public:
  TestAmbassador(const RTITest::ConstructorArgs& constructorArgs) :
    RTI1516TestAmbassador(constructorArgs),
    _timeRegulationEnabled(false),
    _timeConstrainedEnabled(false),
    _timeAdvancePending(false),
    _lookahead(1),
    _fail(false)
  { }
  virtual ~TestAmbassador()
    throw ()
  { }

  virtual bool execJoined(rti1516::RTIambassador& ambassador)
  {
    // Get some handles
    rti1516::InteractionClassHandle interactionClassHandle0;
    try {
      interactionClassHandle0 = ambassador.getInteractionClassHandle(L"InteractionClass0");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    rti1516::ParameterHandle class0Parameter0Handle;
    try {
      class0Parameter0Handle = ambassador.getParameterHandle(interactionClassHandle0, L"parameter0");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    // Enable time constrained
    try {
      ambassador.enableTimeConstrained();
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      Clock timeout = Clock::now() + Clock::fromSeconds(10);
      while (!_timeConstrainedEnabled) {
        if (ambassador.evokeCallback(60.0))
          continue;
        if (timeout < Clock::now()) {
          std::wcout << L"Timeout waiting for next message" << std::endl;
          return false;
        }
      }

    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    // Now that we are constrained, subscribe
    try {
      ambassador.subscribeInteractionClass(interactionClassHandle0);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    // Enable time regulation
    try {
      ambassador.enableTimeRegulation(_lookahead);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      Clock timeout = Clock::now() + Clock::fromSeconds(60);
      while (!_timeRegulationEnabled) {
        if (ambassador.evokeCallback(10.0))
          continue;
        if (timeout < Clock::now()) {
          std::wcout << L"Timeout waiting for next message" << std::endl;
          return false;
        }
      }

    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    // Now that we are regulating, publish
    try {
      ambassador.publishInteractionClass(interactionClassHandle0);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    // Ok, here time advance and so on
    try {
      ambassador.queryLogicalTime(_logicalTime);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    unsigned maxSteps = 300;
    for (unsigned i = 0; i < maxSteps; ++i) {
      /// Send some interactions.

      try {
        rti1516::ParameterHandleValueMap parameterValues;
        rti1516::VariableLengthData tag = toVariableLengthData("withoutTimestamp");
        parameterValues[class0Parameter0Handle] = _logicalTime.encode();
        ambassador.sendInteraction(interactionClassHandle0, parameterValues, tag);
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      for (unsigned j = 0; j < 10 && i + j < maxSteps; ++j) {
        HLAinteger64Time logicalTime = _logicalTime + HLAinteger64Interval(j);

        try {
          rti1516::ParameterHandleValueMap parameterValues;
          rti1516::VariableLengthData tag = toVariableLengthData("withTimestamp");
          parameterValues[class0Parameter0Handle] = _logicalTime.encode();
          ambassador.sendInteraction(interactionClassHandle0, parameterValues, tag, logicalTime);

          // It's not ok to succees if we try with an already passed logical time.
          if (_lookahead == HLAinteger64Interval(0)) {
            if (logicalTime <= _logicalTime) {
              std::wcout << L"Accepted logical time in the past for message delivery!" << std::endl;
              return false;
            }
          } else {
            if (logicalTime < _logicalTime + _lookahead) {
              std::wcout << L"Accepted logical time in the past for message delivery!" << std::endl;
              return false;
            }
          }

        } catch (const rti1516::InvalidLogicalTime& e) {
          // It's ok to fail if we try with an already passed logical time.
          // But if the timestamp is valid this is fatal!!!
          if (_lookahead == HLAinteger64Interval(0)) {
            if (_logicalTime < logicalTime) {
              std::wcout << L"rti1516::InvalidLogicalTime: \"" << e.what() << L"\"" << std::endl;
              return false;
            }
          } else {
            if (_logicalTime + _lookahead <= logicalTime) {
              std::wcout << L"rti1516::InvalidLogicalTime: \"" << e.what() << L"\"" << std::endl;
              return false;
            }
          }
        } catch (const rti1516::Exception& e) {
          std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
          return false;
        } catch (...) {
          std::wcout << L"Unknown Exception!" << std::endl;
          return false;
        }
      }

      // Do the time advance
      try {
        _advanceLogicalTime = _logicalTime + HLAinteger64Interval(1);
        ambassador.timeAdvanceRequest(_advanceLogicalTime);
        _timeAdvancePending = true;
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      try {
        Clock timeout = Clock::now() + Clock::fromSeconds(10);
        while (_timeAdvancePending) {
          if (ambassador.evokeCallback(60.0))
            continue;
          if (timeout < Clock::now()) {
            std::wcout << L"Timeout waiting for next message" << std::endl;
            return false;
          }
        }
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      if (_advanceLogicalTime != _logicalTime) {
        std::wcout << L"Times do not match" << std::endl;
        return false;
      }
    }

    // Cleanup time management

    try {
      ambassador.disableTimeRegulation();
      _timeRegulationEnabled = false;
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.disableTimeConstrained();
      _timeConstrainedEnabled = false;
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return !_fail;
  }

  virtual void
  timeRegulationEnabled(const rti1516::LogicalTime& logicalTime)
    throw (rti1516::InvalidLogicalTime,
           rti1516::NoRequestToEnableTimeRegulationWasPending,
           rti1516::FederateInternalError)
  {
    _logicalTime = logicalTime;
    _timeRegulationEnabled = true;
  }

  virtual void
  timeConstrainedEnabled(const rti1516::LogicalTime& logicalTime)
    throw (rti1516::InvalidLogicalTime,
           rti1516::NoRequestToEnableTimeConstrainedWasPending,
           rti1516::FederateInternalError)
  {
    _logicalTime = logicalTime;
    _timeConstrainedEnabled = true;
  }

  virtual void
  timeAdvanceGrant(const rti1516::LogicalTime& logicalTime)
    throw (rti1516::InvalidLogicalTime,
           rti1516::JoinedFederateIsNotInTimeAdvancingState,
           rti1516::FederateInternalError)
  {
    _logicalTime = logicalTime;
    _timeAdvancePending = false;
    if (_logicalTime != _advanceLogicalTime) {
        _fail = true;
        std::wcout << L"Time advance grant time does not match the requested time!" << std::endl;
    }
  }

  virtual void
  receiveInteraction(rti1516::InteractionClassHandle, const rti1516::ParameterHandleValueMap&,
                     const rti1516::VariableLengthData& tag, rti1516::OrderType sentOrder, rti1516::TransportationType)
    throw (rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::FederateInternalError)
  {
    if (strncmp("withoutTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        _fail = true;
        std::wcout << L"Got timestamp order message that was recieved as receive order!" << std::endl;
    }
    if (sentOrder != rti1516::RECEIVE) {
        _fail = true;
        std::wcout << L"Got recieve order message that was recieved as timestamp order!" << std::endl;
    }
  }

  virtual void
  receiveInteraction(rti1516::InteractionClassHandle theInteraction,
                     rti1516::ParameterHandleValueMap const & theParameterValues,
                     rti1516::VariableLengthData const & tag,
                     rti1516::OrderType sentOrder,
                     rti1516::TransportationType theType,
                     rti1516::LogicalTime const & theTime,
                     rti1516::OrderType receivedOrder)
    throw (rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::FederateInternalError)
  {
    if (strncmp("withTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        _fail = true;
        std::wcout << L"Got recieve order message that was recieved as timestamp order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP && sentOrder != rti1516::TIMESTAMP) {
        _fail = true;
        std::wcout << L"Received timestamp order message that was sent as receive order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP)
      checkLogicalTime(theTime);
  }

  virtual void
  receiveInteraction(rti1516::InteractionClassHandle theInteraction,
                     rti1516::ParameterHandleValueMap const & theParameterValues,
                     rti1516::VariableLengthData const & tag,
                     rti1516::OrderType sentOrder,
                     rti1516::TransportationType theType,
                     rti1516::LogicalTime const & theTime,
                     rti1516::OrderType receivedOrder,
                     rti1516::MessageRetractionHandle theHandle)
    throw (rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError)
  {
    if (strncmp("withTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        std::wcout << L"Got recieve order message over timestamped delivery!" << std::endl;
        _fail = true;
    }
    if (receivedOrder == rti1516::TIMESTAMP && sentOrder != rti1516::TIMESTAMP) {
        _fail = true;
        std::wcout << L"Received timestamp order message that was sent as receive order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP)
      checkLogicalTime(theTime);
  }

  void checkLogicalTime(rti1516::LogicalTime const & logicalTime)
  {
    if (logicalTime <= _logicalTime) {
      _fail = true;
      std::wcout << L"Received timestamp order message with timestamp in the past: ["
                 << _logicalTime << ", " << _advanceLogicalTime << "] " << logicalTime << std::endl;
    }
    if (_advanceLogicalTime < logicalTime) {
      _fail = true;
      std::wcout << L"Received timestamp order message with timestamp in the future: ["
                 << _logicalTime << ", " << _advanceLogicalTime << "] " << logicalTime << std::endl;
    }
  }

private:
  bool _timeRegulationEnabled;
  bool _timeConstrainedEnabled;
  bool _timeAdvancePending;
  HLAinteger64Time _logicalTime;
  HLAinteger64Time _advanceLogicalTime;
  HLAinteger64Interval _lookahead;
  bool _fail;
};

class Test : public RTITest {
public:
  Test(int argc, const char* const argv[]) :
    RTITest(argc, argv, false)
  { }
  virtual Ambassador* createAmbassador(const ConstructorArgs& constructorArgs)
  {
    return new TestAmbassador(constructorArgs);
  }
};

int
main(int argc, char* argv[])
{
  Test test(argc, argv);
  return test.exec();
}
