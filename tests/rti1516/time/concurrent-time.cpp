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
    _timeAdvancePending(false)
  { }
  virtual ~TestAmbassador()
    throw ()
  { }

  virtual bool execJoined(rti1516::RTIambassador& ambassador)
  {
    // Enable time constrained
    try {
      ambassador.disableTimeConstrained();
      std::wcout << L"disableTimeConstrained does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::TimeConstrainedIsNotEnabled&) {
      // Ok, must do that
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

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
      ambassador.enableTimeConstrained();
      std::wcout << L"enableTimeConstrained does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::RequestForTimeConstrainedPending&) {
      // Ok, must do that
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

    try {
      ambassador.enableTimeConstrained();
      std::wcout << L"enableTimeConstrained does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::TimeConstrainedAlreadyEnabled&) {
      // Ok, must do that
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    // Enable time regulation
    try {
      ambassador.disableTimeRegulation();
      std::wcout << L"disableTimeRegulation does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::TimeRegulationIsNotEnabled&) {
      // Ok, must do that
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    HLAinteger64Interval lookahead(1);
    try {
      ambassador.enableTimeRegulation(lookahead);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.enableTimeRegulation(lookahead);
      std::wcout << L"enableTimeRegulation does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::RequestForTimeRegulationPending&) {
      // Ok, must do that
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

    try {
      ambassador.enableTimeRegulation(lookahead);
      std::wcout << L"enableTimeRegulation does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::TimeRegulationAlreadyEnabled&) {
      // Ok, must do that
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

    for (unsigned i = 0; i < 300; ++i) {
      _logicalTime += lookahead;
      setLBTS(_logicalTime.getTime());
      try {
        ambassador.timeAdvanceRequest(_logicalTime);
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

      unsigned commonStamp = getLBTS();
      if (commonStamp < _logicalTime.getTime()) {
        std::wcout << L"Time advance grant to early: i = " << i << ", lbts: " << commonStamp
                   << ", our time: " << _logicalTime.getTime() << std::endl;
        return false;
      }

      // std::wcout << _logicalTime.getTime() << std::endl;
    }

    // Cleanup time management

    clearLBTS();

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

    return true;
  }

  virtual void
  timeRegulationEnabled(const rti1516::LogicalTime& logicalTime)
    throw (rti1516::InvalidLogicalTime,
           rti1516::NoRequestToEnableTimeRegulationWasPending,
           rti1516::FederateInternalError)
  {
    _logicalTime = logicalTime;
    _timeRegulationEnabled = true;
    // std::wcout << "timeRegulationEnabled: " << logicalTime.toString() << std::endl;
  }

  virtual void
  timeConstrainedEnabled(const rti1516::LogicalTime& logicalTime)
    throw (rti1516::InvalidLogicalTime,
           rti1516::NoRequestToEnableTimeConstrainedWasPending,
           rti1516::FederateInternalError)
  {
    _logicalTime = logicalTime;
    _timeConstrainedEnabled = true;
    // std::wcout << "timeConstrainedEnabled: " << logicalTime.toString() << std::endl;
  }

  virtual void
  timeAdvanceGrant(const rti1516::LogicalTime& logicalTime)
    throw (rti1516::InvalidLogicalTime,
           rti1516::JoinedFederateIsNotInTimeAdvancingState,
           rti1516::FederateInternalError)
  {
    _timeAdvancePending = false;
    // std::wcout << "timeAdvanceGrant: " << logicalTime.toString() << std::endl;
  }

private:
  bool _timeRegulationEnabled;
  bool _timeConstrainedEnabled;
  bool _timeAdvancePending;
  HLAinteger64Time _logicalTime;
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
