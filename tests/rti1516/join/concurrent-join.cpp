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

      // create, should work
      try {
        ambassador.createFederationExecution(getFederationExecution(), getFddFile());
      } catch (const rti1516::FederationExecutionAlreadyExists&) {
        // Can happen in this test
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      bool joined = false;

      try {
        ambassador.joinFederationExecution(getFederationExecution(), getFederationExecution());
        joined = true;
      } catch (const rti1516::FederationExecutionDoesNotExist&) {
        // Can happen in this test
        // This join might race agains a destroy in an other thread.
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      try {
        ambassador.resignFederationExecution(rti1516::NO_ACTION);
      } catch (const rti1516::FederationExecutionDoesNotExist& e) {
        if (joined) {
          // If we have successfuly joined, this is considered an error
          std::wcout << L"rti1516::FederationExecutionDoesNotExist: \"" << e.what() << L"\"" << std::endl;
          return false;
        } else {
        }
      } catch (const rti1516::FederateNotExecutionMember& e) {
        if (joined) {
          // If we have successfuly joined, this is considered an error
          std::wcout << L"rti1516::FederateNotExecutionMember: \"" << e.what() << L"\"" << std::endl;
          return false;
        } else {
        }
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // destroy. Might work. Other could be faster
      try {
        ambassador.destroyFederationExecution(getFederationExecution());
      } catch (const rti1516::FederatesCurrentlyJoined&) {
        // Can happen in this test
        // Other threads just might have still joined.
      } catch (const rti1516::FederationExecutionDoesNotExist&) {
        // Can happen in this test
        // Other threads might habe been faster
      } catch (const rti1516::Exception& e) {
        std::wcout << e.what() << std::endl;
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
    RTITest(argc, argv, false)
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
