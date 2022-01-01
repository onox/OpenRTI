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

#include <Options.h>
#include <StringUtils.h>

#include <RTI1516TestLib.h>

namespace OpenRTI {

class OPENRTI_LOCAL TestAmbassador : public RTI1516TestAmbassador {
public:
  TestAmbassador(const RTITest::ConstructorArgs& constructorArgs) :
    RTI1516TestAmbassador(constructorArgs)
  { }
  virtual ~TestAmbassador()
    RTI_NOEXCEPT
  { }

  virtual bool execJoined(rti1516::RTIambassador& ambassador)
  {
    if (!waitForAllFederates(ambassador))
      return false;

    rti1516::ObjectClassHandle objectClassHandle1;
    try {
      objectClassHandle1 = ambassador.getObjectClassHandle(L"ObjectClass1");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    std::set<rti1516::AttributeHandle> attributeHandles;

    try {
      ambassador.subscribeObjectClassAttributes(objectClassHandle1, attributeHandles);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.publishObjectClassAttributes(objectClassHandle1, attributeHandles);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    // Try to reserve, only one can win
    unsigned count = 10;
    try {
      for (unsigned i = 0; i < count; ++i) {
        std::wstringstream stream;
        stream << "ObjectInstanceName" << i;
        ambassador.reserveObjectInstanceName(stream.str());
      }
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      Clock timeout = Clock::now() + Clock::fromSeconds(10);
      while (_nameReservationMap.size() < count) {
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

    // Check if only one could reserve the name
    for (NameReservationMap::iterator i = _nameReservationMap.begin(); i != _nameReservationMap.end(); ++i) {
      if (i->second) {
        if (!getFederationBarrier()->success())
          return false;
      } else {
        if (!getFederationBarrier()->fail())
          return false;
      }
    }

    // Check if we can register one of the failed names
    for (NameReservationMap::iterator i = _nameReservationMap.begin(); i != _nameReservationMap.end(); ++i) {
      try {
        if (!i->second) {
          ambassador.registerObjectInstance(objectClassHandle1, i->first);
          // If we get the object but reservation fails, we should not get here
          return false;
        }
      } catch (const rti1516::ObjectInstanceNameNotReserved&) {
        // One of them, FIXME: do not know which one is the right one
      } catch (const rti1516::ObjectInstanceNameInUse&) {
        // One of them, FIXME: do not know which one is the right one
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }
    }

    std::set<rti1516::ObjectInstanceHandle> objectInstanceHandles;
    try {
      // skip the first one, ensure propper cleanup when resigning with reserved names
      NameReservationMap::iterator i = _nameReservationMap.begin();
      if (i != _nameReservationMap.end()) {
        for (++i; i != _nameReservationMap.end(); ++i) {
          if (i->second) {
            objectInstanceHandles.insert(ambassador.registerObjectInstance(objectClassHandle1, i->first));
          }
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
      for (std::set<rti1516::ObjectInstanceHandle>::iterator i = objectInstanceHandles.begin();
           i != objectInstanceHandles.end(); ++i) {
        ambassador.deleteObjectInstance(*i, toVariableLengthData("tag"));
      }
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    _nameReservationMap.clear();

    try {
      ambassador.unpublishObjectClass(objectClassHandle1);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.unsubscribeObjectClass(objectClassHandle1);
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
  objectInstanceNameReservationSucceeded(const std::wstring& objectInstanceName)
    RTI_THROW ((rti1516::UnknownName, rti1516::FederateInternalError))
  {
    _nameReservationMap[objectInstanceName] = true;
  }

  virtual void
  objectInstanceNameReservationFailed(const std::wstring& objectInstanceName)
    RTI_THROW ((rti1516::UnknownName, rti1516::FederateInternalError))
  {
    _nameReservationMap[objectInstanceName] = false;
  }

  typedef std::map<std::wstring, bool> NameReservationMap;
  NameReservationMap _nameReservationMap;
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
