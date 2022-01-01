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

#include <LogStream.h>
#include <Options.h>
#include <StringUtils.h>

#include <RTI1516TestLib.h>

namespace OpenRTI {

class OPENRTI_LOCAL TestAmbassador : public RTI1516TestAmbassador {
public:
  TestAmbassador(const RTITest::ConstructorArgs& constructorArgs) :
    RTI1516TestAmbassador(constructorArgs),
    _fail(false)
  { }
  virtual ~TestAmbassador()
    RTI_NOEXCEPT
  { }

  virtual bool execJoined(rti1516::RTIambassador& ambassador)
  {
    try {
      rti1516::ObjectClassHandle objectClassHandle = ambassador.getObjectClassHandle(L"ObjectClass0");
      rti1516::ObjectClassHandle fqObjectClassHandle = ambassador.getObjectClassHandle(L"HLAobjectRoot.ObjectClass0");
      if (objectClassHandle != fqObjectClassHandle) {
        std::wcout << L"Full qualified object class lookup failed" << std::endl;
        return false;
      }
      _objectClassAttributeHandleSet[objectClassHandle].insert(ambassador.getAttributeHandle(objectClassHandle, L"attribute0"));
      _objectClassObjectClassHandleSet[objectClassHandle];
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      rti1516::ObjectClassHandle objectClassHandle = ambassador.getObjectClassHandle(L"ObjectClass1");
      rti1516::ObjectClassHandle fqObjectClassHandle = ambassador.getObjectClassHandle(L"HLAobjectRoot.ObjectClass0.ObjectClass1");
      if (objectClassHandle != fqObjectClassHandle) {
        std::wcout << L"Full qualified object class lookup failed" << std::endl;
        return false;
      }
      _objectClassAttributeHandleSet[objectClassHandle].insert(ambassador.getAttributeHandle(objectClassHandle, L"attribute0"));
      _objectClassAttributeHandleSet[objectClassHandle].insert(ambassador.getAttributeHandle(objectClassHandle, L"attribute1"));
      _objectClassObjectClassHandleSet[objectClassHandle].insert(ambassador.getObjectClassHandle(L"ObjectClass0"));
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      rti1516::ObjectClassHandle objectClassHandle = ambassador.getObjectClassHandle(L"ObjectClass2");
      rti1516::ObjectClassHandle fqObjectClassHandle = ambassador.getObjectClassHandle(L"HLAobjectRoot.ObjectClass0.ObjectClass1.ObjectClass2");
      if (objectClassHandle != fqObjectClassHandle) {
        std::wcout << L"Full qualified object class lookup failed" << std::endl;
        return false;
      }
      _objectClassAttributeHandleSet[objectClassHandle].insert(ambassador.getAttributeHandle(objectClassHandle, L"attribute0"));
      _objectClassAttributeHandleSet[objectClassHandle].insert(ambassador.getAttributeHandle(objectClassHandle, L"attribute1"));
      _objectClassAttributeHandleSet[objectClassHandle].insert(ambassador.getAttributeHandle(objectClassHandle, L"attribute2"));
      _objectClassObjectClassHandleSet[objectClassHandle].insert(ambassador.getObjectClassHandle(L"ObjectClass0"));
      _objectClassObjectClassHandleSet[objectClassHandle].insert(ambassador.getObjectClassHandle(L"ObjectClass1"));
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      rti1516::ObjectClassHandle objectClassHandle = ambassador.getObjectClassHandle(L"ObjectClass3");
      rti1516::ObjectClassHandle fqObjectClassHandle = ambassador.getObjectClassHandle(L"HLAobjectRoot.ObjectClass0.ObjectClass1.ObjectClass3");
      if (objectClassHandle != fqObjectClassHandle) {
        std::wcout << L"Full qualified object class lookup failed" << std::endl;
        return false;
      }
      _objectClassAttributeHandleSet[objectClassHandle].insert(ambassador.getAttributeHandle(objectClassHandle, L"attribute0"));
      _objectClassAttributeHandleSet[objectClassHandle].insert(ambassador.getAttributeHandle(objectClassHandle, L"attribute1"));
      _objectClassAttributeHandleSet[objectClassHandle].insert(ambassador.getAttributeHandle(objectClassHandle, L"attribute3"));
      _objectClassObjectClassHandleSet[objectClassHandle].insert(ambassador.getObjectClassHandle(L"ObjectClass0"));
      _objectClassObjectClassHandleSet[objectClassHandle].insert(ambassador.getObjectClassHandle(L"ObjectClass1"));
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    for (std::map<rti1516::ObjectClassHandle, rti1516::AttributeHandleSet>::const_iterator i = _objectClassAttributeHandleSet.begin();
         i != _objectClassAttributeHandleSet.end(); ++i) {
      try {
        ambassador.publishObjectClassAttributes(i->first, i->second);
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }
    }

    // All is published, now subscribe step by step and see what we receive

    for (ObjectClassAttributeHandleSet::const_iterator i = _objectClassAttributeHandleSet.begin();
         i != _objectClassAttributeHandleSet.end(); ++i) {
      rti1516::ObjectClassHandle subscribedObjectClass = i->first;
      rti1516::AttributeHandleSet subscribedAttributeHandles = i->second;

      try {
        ambassador.subscribeObjectClassAttributes(subscribedObjectClass, subscribedAttributeHandles);
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      if (!waitForAllFederates(ambassador))
        return false;

      for (ObjectClassAttributeHandleSet::const_iterator j = _objectClassAttributeHandleSet.begin();
           j != _objectClassAttributeHandleSet.end(); ++j) {
        rti1516::ObjectClassHandle registeredObjectClass = j->first;
        rti1516::ObjectInstanceHandle objectInstanceHandle;
        bool expectDiscovered = true;

        if (registeredObjectClass == subscribedObjectClass) {
          _expectedObjectClassHandle = subscribedObjectClass;
          _expectedAttributeHandles = subscribedAttributeHandles;
        } else if (_objectClassObjectClassHandleSet[registeredObjectClass].count(subscribedObjectClass)) {
          _expectedObjectClassHandle = subscribedObjectClass;
          _expectedAttributeHandles = j->second;
        } else {
          _expectedObjectClassHandle = rti1516::ObjectClassHandle();
          _expectedAttributeHandles.clear();
          expectDiscovered = false;
        }

        try {
          objectInstanceHandle = ambassador.registerObjectInstance(registeredObjectClass);
        } catch (const rti1516::Exception& e) {
          std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
          return false;
        } catch (...) {
          std::wcout << L"Unknown Exception!" << std::endl;
          return false;
        }

        if (expectDiscovered) {
          try {
            Clock timeout = Clock::now() + Clock::fromSeconds(10);
            while (getFederateList().size() != _foreignObjectInstanceHandles.size() + 1 && !_fail) {
              if (ambassador.evokeCallback(60.0))
                continue;
              if (timeout < Clock::now()) {
                std::wcout << L"Timeout waiting for next message" << std::endl;
                return false;
              }
            }
            if (_fail)
              return false;

          } catch (const rti1516::Exception& e) {
            std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
            return false;
          } catch (...) {
            std::wcout << L"Unknown Exception!" << std::endl;
            return false;
          }
        }

        try {
          rti1516::AttributeHandleValueMap attributeValues;
          rti1516::VariableLengthData tag = toVariableLengthData(Clock::now());
          for (rti1516::AttributeHandleSet::const_iterator k = j->second.begin();
               k != j->second.end(); ++k) {
            attributeValues[*k] = toVariableLengthData(ambassador.getAttributeName(registeredObjectClass, *k));
          }
          ambassador.updateAttributeValues(objectInstanceHandle, attributeValues, tag);
        } catch (const rti1516::Exception& e) {
          std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
          return false;
        } catch (...) {
          std::wcout << L"Unknown Exception!" << std::endl;
          return false;
        }

        try {
          ambassador.deleteObjectInstance(objectInstanceHandle, toVariableLengthData("tag"));
        } catch (const rti1516::Exception& e) {
          std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
          return false;
        } catch (...) {
          std::wcout << L"Unknown Exception!" << std::endl;
          return false;
        }

        if (expectDiscovered)
        {
          try {
            Clock timeout = Clock::now() + Clock::fromSeconds(10);
            while (!_foreignObjectInstanceHandles.empty() && !_fail) {
              if (ambassador.evokeCallback(60.0))
                continue;
              if (timeout < Clock::now()) {
                std::wcout << L"Timeout waiting for next message" << std::endl;
                return false;
              }
            }
            if (_fail)
              return false;

          } catch (const rti1516::Exception& e) {
            std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
            return false;
          } catch (...) {
            std::wcout << L"Unknown Exception!" << std::endl;
            return false;
          }
        }

        if (!waitForAllFederates(ambassador))
          return false;
      }

      try {
        ambassador.unsubscribeObjectClass(subscribedObjectClass);
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

  void discoverObjectInstance(rti1516::ObjectInstanceHandle objectInstanceHandle,
                              rti1516::ObjectClassHandle objectClassHandle,
                              const std::wstring& objectInstanceName)
      RTI_THROW ((rti1516::CouldNotDiscover,
             rti1516::ObjectClassNotKnown,
             rti1516::FederateInternalError))
  {
    // Log(Assert, Error) << "discover "  << objectClassHandle.toString() << " " << objectInstanceHandle.toString() << std::endl;
    if (_expectedObjectClassHandle != objectClassHandle) {
      Log(Assert, Error) << "Expected object class " << _expectedObjectClassHandle.toString()
                         << ", but discovered object class "  << objectClassHandle.toString() << std::endl;
      _fail = true;
    }
    if (_foreignObjectInstanceHandles.find(objectInstanceHandle) != _foreignObjectInstanceHandles.end()) {
      Log(Assert, Error) << "Duplicate discoverObjectInstance callback for object instance "
                         << objectInstanceHandle.toString() << std::endl;
      _fail = true;
    }
    _foreignObjectInstanceHandles.insert(objectInstanceHandle);
  }

  void removeObjectInstance(rti1516::ObjectInstanceHandle objectInstanceHandle,
                            const rti1516::VariableLengthData& tag,
                            rti1516::OrderType sentOrder)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::FederateInternalError))
  {
    if (_foreignObjectInstanceHandles.find(objectInstanceHandle) == _foreignObjectInstanceHandles.end()) {
      Log(Assert, Error) << "Spurious removeObjectInstance callback for object instance "
                         << objectInstanceHandle.toString() << std::endl;
      _fail = true;
    }
    // Log(Assert, Error) << "remove " << objectInstanceHandle.toString() << std::endl;
    _foreignObjectInstanceHandles.erase(objectInstanceHandle);

    // Check if we got a single attribute update
    if (1 != _objectInstanceHandleSet.erase(objectInstanceHandle)) {
      Log(Assert, Error) << "Should have revieved at least a single message for "
                         << objectInstanceHandle.toString() << std::endl;
      _fail = true;
    }
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeValues,
                                      const rti1516::VariableLengthData&, rti1516::OrderType, rti1516::TransportationType)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    _checkReflectedAttributeValues(objectInstanceHandle, attributeValues);
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeValues,
                                      const rti1516::VariableLengthData&, rti1516::OrderType, rti1516::TransportationType,
                                      const rti1516::RegionHandleSet&)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    _checkReflectedAttributeValues(objectInstanceHandle, attributeValues);
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeValues,
                                      const rti1516::VariableLengthData&, rti1516::OrderType, rti1516::TransportationType,
                                      const rti1516::LogicalTime&, rti1516::OrderType)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    _checkReflectedAttributeValues(objectInstanceHandle, attributeValues);
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeValues,
                                      const rti1516::VariableLengthData&, rti1516::OrderType, rti1516::TransportationType,
                                      const rti1516::LogicalTime&, rti1516::OrderType, const rti1516::RegionHandleSet&)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    _checkReflectedAttributeValues(objectInstanceHandle, attributeValues);
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeValues,
                                      const rti1516::VariableLengthData&, rti1516::OrderType, rti1516::TransportationType,
                                      const rti1516::LogicalTime&, rti1516::OrderType, rti1516::MessageRetractionHandle)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    _checkReflectedAttributeValues(objectInstanceHandle, attributeValues);
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeValues,
                                      const rti1516::VariableLengthData&, rti1516::OrderType, rti1516::TransportationType,
                                      const rti1516::LogicalTime&, rti1516::OrderType, rti1516::MessageRetractionHandle,
                                      const rti1516::RegionHandleSet&)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    _checkReflectedAttributeValues(objectInstanceHandle, attributeValues);
  }

  void _checkReflectedAttributeValues(const rti1516::ObjectInstanceHandle& objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeValues)
  {
    // Check if we got at most a single attribute update
    if (!_objectInstanceHandleSet.insert(objectInstanceHandle).second) {
      Log(Assert, Error) << "Duplicate reflectAttributeValues." << std::endl;
      _fail = true;
    }
    // Log(Assert, Error) << "reflect " << objectInstanceHandle.toString() << std::endl;
    for (rti1516::AttributeHandleValueMap::const_iterator i = attributeValues.begin();
         i != attributeValues.end(); ++i) {
      if (_expectedAttributeHandles.find(i->first) != _expectedAttributeHandles.end())
        continue;
      Log(Assert, Error) << "Received attribute value for unsubscribed attribute " << i->first.toString() << std::endl;
      _fail = true;
    }
  }

  bool _fail;

  // Store the 'is derived of' relation
  typedef std::map<rti1516::ObjectClassHandle, std::set<rti1516::ObjectClassHandle> > ObjectClassObjectClassHandleSet;
  ObjectClassObjectClassHandleSet _objectClassObjectClassHandleSet;

  // The available object class, attribute set
  typedef std::map<rti1516::ObjectClassHandle, rti1516::AttributeHandleSet> ObjectClassAttributeHandleSet;
  ObjectClassAttributeHandleSet _objectClassAttributeHandleSet;

  typedef std::set<rti1516::ObjectInstanceHandle> ObjectInstanceHandleSet;
  ObjectInstanceHandleSet _objectInstanceHandleSet;

  rti1516::ObjectClassHandle _expectedObjectClassHandle;
  rti1516::AttributeHandleSet _expectedAttributeHandles;

  std::set<rti1516::ObjectInstanceHandle> _foreignObjectInstanceHandles;
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
