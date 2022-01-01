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

#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include <RTI/RTIambassadorFactory.h>
#include <RTI/RTIambassador.h>

#include <TestLib.h>
#include <RTI1516TestLib.h>

// Test concurrency of creation.
// That means all but one must fail.
// But exactly one must be successful.

namespace OpenRTI {

class OPENRTI_LOCAL TestAmbassador : public RTITest::Ambassador {
public:
  TestAmbassador(const RTITest::ConstructorArgs& constructorArgs) :
    RTITest::Ambassador(constructorArgs)
  { }

  virtual bool exec()
  {
    RTI1516SimpleAmbassador ambassador;
    ambassador.connect(getArgumentList());
    ambassador.setLogicalTimeFactory();

    // Try that several times. Ensure correct cleanup
    unsigned n = 99;
    for (unsigned i = 0; i < n; ++i) {

      // destroy uncreated.
      try {
        ambassador.destroyFederationExecution(getFederationExecution());
        std::wcout << L"destroyFederationExecution does not fail as required!" << std::endl;
        return false;
      } catch (const rti1516::FederationExecutionDoesNotExist&) {
        // Ok, must do that
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // join uncreated. Needs to fail
      try {
        ambassador.joinFederationExecution(L"federate", getFederationExecution());
        std::wcout << L"joinFederationExecution does not fail as required!" << std::endl;
        return false;
      } catch (const rti1516::FederationExecutionDoesNotExist&) {
        // Needs to happen here
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // resign not joined and not existing
      try {
        ambassador.resignFederationExecution(rti1516::NO_ACTION);
        std::wcout << L"resignFederationExecution does not fail as required!" << std::endl;
        return false;
      } catch (const rti1516::FederateNotExecutionMember&) {
        // Needs to happen here
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // create, must work
      try {
        ambassador.createFederationExecution(getFederationExecution(), getFddFile());
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // resign not joined and not existing
      try {
        ambassador.resignFederationExecution(rti1516::NO_ACTION);
        std::wcout << L"resignFederationExecution does not fail as required!" << std::endl;
        return false;
      } catch (const rti1516::FederateNotExecutionMember&) {
        // Needs to happen here
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // No join must work
      try {
        ambassador.joinFederationExecution(L"federate", getFederationExecution());
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // and now resign must work
      try {
        ambassador.resignFederationExecution(rti1516::NO_ACTION);
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // Join again with the same name. Must work
      try {
        ambassador.joinFederationExecution(L"federate", getFederationExecution());
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // destroy, must not work, still have federates
      try {
        ambassador.destroyFederationExecution(getFederationExecution());
        std::wcout << L"destroyFederationExecution does not fail as required!" << std::endl;
        return false;
      } catch (const rti1516::FederatesCurrentlyJoined&) {
        // Ok, must do that
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // and now resign, must work
      try {
        ambassador.resignFederationExecution(rti1516::NO_ACTION);
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // resign again, must not work
      try {
        ambassador.resignFederationExecution(rti1516::NO_ACTION);
        std::wcout << L"destroyFederationExecution does not fail as required!" << std::endl;
        return false;
      } catch (const rti1516::FederateNotExecutionMember&) {
        // Needs to happen here
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // destroy, must work
      try {
        ambassador.destroyFederationExecution(getFederationExecution());
      } catch (const rti1516::Exception& e) {
        std::wcout << e.what() << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // destroy again, must not work
      try {
        ambassador.destroyFederationExecution(getFederationExecution());
        std::wcout << L"destroyFederationExecution does not fail as required!" << std::endl;
        return false;
      } catch (const rti1516::FederationExecutionDoesNotExist&) {
        // Ok, must do that
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }
    }

    return true;
  }
};

class OPENRTI_LOCAL Test : public RTITest {
public:
  Test(int argc, const char* const argv[]) :
    RTITest(argc, argv, true)
  { }
  virtual Ambassador* createAmbassador(const ConstructorArgs& constructorArgs)
  {
    return new TestAmbassador(constructorArgs);
  }
};

}

int
main(int argc, char* argv[])
{
  OpenRTI::Test test(argc, argv);
  return test.exec();
}
