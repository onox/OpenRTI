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
#include <cwchar>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <set>
#include <iostream>

#include <RTI/RTIambassadorFactory.h>
#include <RTI/RTIambassador.h>

#include <TestLib.h>
#include <RTI1516ETestLib.h>

namespace OpenRTI {

struct OPENRTI_LOCAL InteractionClass {
  std::set<std::wstring> _parameterNames;

  void insert(const std::wstring& name)
  { _parameterNames.insert(name); }
  bool getParameterDefined(const std::wstring& name) const
  { return _parameterNames.find(name) != _parameterNames.end(); }
};

struct OPENRTI_LOCAL ObjectClass {
  std::set<std::wstring> _attributeNames;

  void insert(const std::wstring& name)
  { _attributeNames.insert(name); }
  bool getAttributeDefined(const std::wstring& name) const
  { return _attributeNames.find(name) != _attributeNames.end(); }
};

struct OPENRTI_LOCAL Module {
  std::wstring _name;
  std::set<std::wstring> _incompatibleModuleNames;

  std::map<std::wstring, InteractionClass> _interactionClasses;
  std::map<std::wstring, ObjectClass> _objectClasses;

  std::vector<std::wstring> _dimensionNames;
  std::vector<std::wstring> _updateRateNames;

  Module()
  {
    _interactionClasses[L"HLAinteractionRoot"];
    _objectClasses[L"HLAobjectRoot"].insert(L"HLAprivilegeToDeleteObject");
  }

  bool isIncompatible(const std::wstring& name) const
  { return _incompatibleModuleNames.find(name) != _incompatibleModuleNames.end(); }

  bool getInteractionClassDefined(const std::wstring& name) const
  { return _interactionClasses.find(name) != _interactionClasses.end(); }
  bool getParameterDefined(const std::wstring& className, const std::wstring& name) const
  {
    std::map<std::wstring, InteractionClass>::const_iterator i = _interactionClasses.find(className);
    if (i == _interactionClasses.end())
      return false;
    return i->second.getParameterDefined(name);
  }

  bool getObjectClassDefined(const std::wstring& name) const
  { return _objectClasses.find(name) != _objectClasses.end(); }
  bool getAttributeDefined(const std::wstring& className, const std::wstring& name) const
  {
    std::map<std::wstring, ObjectClass>::const_iterator i = _objectClasses.find(className);
    if (i == _objectClasses.end())
      return false;
    return i->second.getAttributeDefined(name);
  }

  void merge(const Module& module)
  {
    for (std::map<std::wstring, InteractionClass>::const_iterator i = module._interactionClasses.begin();
         i != module._interactionClasses.end(); ++i) {
      _interactionClasses[i->first]._parameterNames.insert(i->second._parameterNames.begin(), i->second._parameterNames.end());
    }

    for (std::map<std::wstring, ObjectClass>::const_iterator i = module._objectClasses.begin();
         i != module._objectClasses.end(); ++i) {
      _objectClasses[i->first]._attributeNames.insert(i->second._attributeNames.begin(), i->second._attributeNames.end());
    }
  }
};

struct OPENRTI_LOCAL ModuleList {
  ModuleList()
  {
    {
      Module module;
      module._name = L"InteractionClass1.xml";

      module._interactionClasses[L"InteractionClass1"];

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"InteractionClass1-1.xml";
      module._incompatibleModuleNames.insert(L"InteractionClass1.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass1-2.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass2.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass2-1.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass2-2.xml");

      module._interactionClasses[L"InteractionClass1"].insert(L"Parameter1");

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"InteractionClass1-2.xml";
      module._incompatibleModuleNames.insert(L"InteractionClass1.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass1-1.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass2.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass2-1.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass2-2.xml");

      module._interactionClasses[L"InteractionClass1"]._parameterNames.insert(L"OtherParameter1");

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"InteractionClass2.xml";

      module._interactionClasses[L"InteractionClass1"];
      module._interactionClasses[L"InteractionClass2"];

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"InteractionClass2-1.xml";
      module._incompatibleModuleNames.insert(L"InteractionClass2.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass2-2.xml");

      module._interactionClasses[L"InteractionClass1"];
      module._interactionClasses[L"InteractionClass2"]._parameterNames.insert(L"Parameter2");

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"InteractionClass2-2.xml";
      module._incompatibleModuleNames.insert(L"InteractionClass2.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass2-1.xml");

      module._interactionClasses[L"InteractionClass1"];
      module._interactionClasses[L"InteractionClass2"]._parameterNames.insert(L"OtherParameter2");

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"InteractionClass3.xml";

      module._interactionClasses[L"InteractionClass3"];

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"InteractionClass3-1.xml";
      module._incompatibleModuleNames.insert(L"InteractionClass3.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass3-2.xml");

      module._interactionClasses[L"InteractionClass3"]._parameterNames.insert(L"Parameter3");

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"InteractionClass3-2.xml";
      module._incompatibleModuleNames.insert(L"InteractionClass3.xml");
      module._incompatibleModuleNames.insert(L"InteractionClass3-1.xml");

      module._interactionClasses[L"InteractionClass3"]._parameterNames.insert(L"OtherParameter3");

      _modules.push_back(module);
    }

    /////////////////////////////////////////////////////////////////

    {
      Module module;
      module._name = L"ObjectClass1.xml";

      module._objectClasses[L"ObjectClass1"];

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"ObjectClass1-1.xml";
      module._incompatibleModuleNames.insert(L"ObjectClass1.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass1-2.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass2.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass2-1.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass2-2.xml");

      module._objectClasses[L"ObjectClass1"].insert(L"Attribute1");

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"ObjectClass1-2.xml";
      module._incompatibleModuleNames.insert(L"ObjectClass1.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass1-1.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass2.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass2-1.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass2-2.xml");

      module._objectClasses[L"ObjectClass1"].insert(L"OtherAttribute1");

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"ObjectClass2.xml";

      module._objectClasses[L"ObjectClass1"];
      module._objectClasses[L"ObjectClass2"];

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"ObjectClass2-1.xml";
      module._incompatibleModuleNames.insert(L"ObjectClass2.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass2-2.xml");

      module._objectClasses[L"ObjectClass1"];
      module._objectClasses[L"ObjectClass2"].insert(L"Attribute2");

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"ObjectClass2-2.xml";
      module._incompatibleModuleNames.insert(L"ObjectClass2.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass2-1.xml");

      module._objectClasses[L"ObjectClass1"];
      module._objectClasses[L"ObjectClass2"].insert(L"OtherAttribute2");

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"ObjectClass3.xml";

      module._objectClasses[L"ObjectClass3"];

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"ObjectClass3-1.xml";
      module._incompatibleModuleNames.insert(L"ObjectClass3.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass3-2.xml");

      module._objectClasses[L"ObjectClass3"].insert(L"Attribute3");

      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"ObjectClass3-2.xml";
      module._incompatibleModuleNames.insert(L"ObjectClass3.xml");
      module._incompatibleModuleNames.insert(L"ObjectClass3-1.xml");

      module._objectClasses[L"ObjectClass3"].insert(L"OtherAttribute3");

      _modules.push_back(module);
    }

    /////////////////////////////////////////////////////////////

    {
      Module module;
      module._name = L"Dimension1-1.xml";
      module._incompatibleModuleNames.insert(L"Dimension1-2.xml");

      module._dimensionNames.push_back(L"Dimension1");
      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"Dimension1-2.xml";
      module._incompatibleModuleNames.insert(L"Dimension1-1.xml");

      module._dimensionNames.push_back(L"Dimension1");
      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"Dimension2-1.xml";
      module._incompatibleModuleNames.insert(L"Dimension2-2.xml");

      module._dimensionNames.push_back(L"Dimension2");
      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"Dimension2-2.xml";
      module._incompatibleModuleNames.insert(L"Dimension2-1.xml");

      module._dimensionNames.push_back(L"Dimension2");
      _modules.push_back(module);
    }

    /////////////////////////////////////////////////////////////

    {
      Module module;
      module._name = L"UpdateRate1-1.xml";
      module._incompatibleModuleNames.insert(L"UpdateRate1-2.xml");

      module._updateRateNames.push_back(L"UpdateRate1");
      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"UpdateRate1-2.xml";
      module._incompatibleModuleNames.insert(L"UpdateRate1-1.xml");

      module._updateRateNames.push_back(L"UpdateRate1");
      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"UpdateRate2-1.xml";
      module._incompatibleModuleNames.insert(L"UpdateRate2-2.xml");

      module._updateRateNames.push_back(L"UpdateRate2");
      _modules.push_back(module);
    }

    {
      Module module;
      module._name = L"UpdateRate2-2.xml";
      module._incompatibleModuleNames.insert(L"UpdateRate2-1.xml");

      module._updateRateNames.push_back(L"UpdateRate2");
      _modules.push_back(module);
    }
  }

  bool fomModuleCombinationValid(const std::wstring& previousFileName, const std::wstring& fileName) const
  {
    for (std::size_t i = 0; i < _modules.size(); ++i) {
      if (fileName != _modules[i]._name)
        continue;
      if (_modules[i].isIncompatible(previousFileName))
        return false;
    }
    return true;
  }

  bool fomModuleListValid(const std::vector<std::wstring>& fomModules) const
  {
    for (std::vector<std::wstring>::const_iterator i = fomModules.begin(); i != fomModules.end(); ++i) {
      for (std::vector<std::wstring>::const_iterator j = fomModules.begin(); j != i; ++j) {
        // It's just ok, if it's included a second time.
        if (*j == *i)
          break;
        if (!fomModuleCombinationValid(*j, *i))
          return false;
      }
    }
    return true;
  }

  Module getDefined(const std::vector<std::wstring>& fomModules)
  {
    Module module;
    for (std::size_t i = 0; i < _modules.size(); ++i) {
      if (std::find(fomModules.begin(), fomModules.end(), _modules[i]._name) == fomModules.end())
        continue;
      module.merge(_modules[i]);
    }
    return module;
  }

  Module getDefined()
  {
    Module module;
    for (std::size_t i = 0; i < _modules.size(); ++i)
      module.merge(_modules[i]);
    return module;
  }

  void buildModuleList(std::vector<std::wstring>& fomModules, unsigned rnd) const
  {
    rnd = rnd & (rnd << 1);
    for (std::size_t i = 0; i < _modules.size(); ++i) {
      if (rnd & (1 << i))
        fomModules.push_back(_modules[i]._name);
    }
  }

  std::vector<Module> _modules;
};

static std::vector<std::wstring> prependPath(const std::wstring& path, const std::vector<std::wstring>& additionalFomModules)
{
  std::vector<std::wstring> withPath;
  withPath.reserve(additionalFomModules.size());
  for (std::vector<std::wstring>::const_iterator i = additionalFomModules.begin(); i != additionalFomModules.end(); ++i)
    withPath.push_back(path + L"/" + *i);
  return withPath;
}

class OPENRTI_LOCAL TestAmbassador : public RTITest::Ambassador {
public:
  TestAmbassador(const RTITest::ConstructorArgs& constructorArgs) :
    RTITest::Ambassador(constructorArgs)
  { }

  virtual bool exec()
  {
    RTI1516ESimpleAmbassador ambassador;
    ambassador.connect(getConnectUrl());
    ambassador.setLogicalTimeFactory();

    Module allModule = _moduleList.getDefined();

    // Try that several times. Ensure correct cleanup
    unsigned n = 100;
    for (unsigned i = 0; i < n; ++i) {
      std::vector<std::wstring> fomModules;
      // We are doing two different kinds of tests with this program.
      // If we have disjoint federates we test the whole infrastructure if we correctly fail
      // with several mixed object models at different places. We test the the expected set
      // available entities for availability and unavailability - that means none of these
      // is allowed to bleed into a federate not having this in the union of supplied modules.
      // The second set of tests works on a shared federation where we still want to test
      // for the availability and unavailability, but for that we need to know the base object
      // model that is in place, which is then just the mim.
      if (getDisjointFederations())
        _moduleList.buildModuleList(fomModules, getRandomNumber());

      bool expectCreateSuccess = _moduleList.fomModuleListValid(fomModules);
      bool successfullyCreated = false;

      try {
        if (getMimFile().empty())
          ambassador.createFederationExecution(getFederationExecution(), prependPath(getFddFile(), fomModules));
        else
          ambassador.createFederationExecutionWithMIM(getFederationExecution(), prependPath(getFddFile(), fomModules), getMimFile());
        successfullyCreated = true;
        if (!expectCreateSuccess) {
          std::wcout << L"createFederationExecution does not fail as required!" << std::endl;
          return false;
        }
      } catch (const rti1516e::InconsistentFDD& e) {
        if (expectCreateSuccess && getDisjointFederations()) {
          std::wcout << L"createFederationExecution does not succeed as required!" << std::endl;
          std::wcout << L"rti1516e::InconsistentFDD: \"" << e.what() << L"\"" << std::endl;
          return false;
        }
      } catch (const rti1516e::FederationExecutionAlreadyExists& e) {
        if (getDisjointFederations()) {
          std::wcout << L"createFederationExecution does not succeed as required!" << std::endl;
          std::wcout << L"rti1516e::FederationExecutionAlreadyExists: \"" << e.what() << L"\"" << std::endl;
          return false;
        }
      } catch (const rti1516e::Exception& e) {
        std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      for (unsigned j = 0; j < 10; ++j) {
        std::vector<std::wstring> additionalFomModules;
        _moduleList.buildModuleList(additionalFomModules, getRandomNumber());

        std::vector<std::wstring> fullFomModules = fomModules;
        fullFomModules.insert(fullFomModules.end(), additionalFomModules.begin(), additionalFomModules.end());

        bool expectJoinSuccess = _moduleList.fomModuleListValid(fullFomModules);
        if (getDisjointFederations() && expectJoinSuccess)
          fomModules = fullFomModules;

        bool successfullyJoined = false;

        try {
          ambassador.joinFederationExecution(L"federate", getFederationExecution(), prependPath(getFddFile(), additionalFomModules));
          successfullyJoined = true;
          if ((!expectJoinSuccess || !successfullyCreated) && getDisjointFederations()) {
            std::wcout << L"joinFederationExecution does not fail as required!" << std::endl;
            return false;
          }
        } catch (const rti1516e::FederationExecutionDoesNotExist& e) {
          if (expectCreateSuccess && getDisjointFederations()) {
            std::wcout << L"joinFederationExecution does not succeed as required!" << std::endl;
            std::wcout << L"rti1516e::FederationExecutionDoesNotExist: \"" << e.what() << L"\"" << std::endl;
            return false;
          }
        } catch (const rti1516e::InconsistentFDD& e) {
          if (expectJoinSuccess && getDisjointFederations()) {
            std::wcout << L"joinFederationExecution does not succeed as required!" << std::endl;
            std::wcout << L"rti1516e::InconsistentFDD: \"" << e.what() << L"\"" << std::endl;
            return false;
          }
        } catch (const rti1516e::Exception& e) {
          std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
          return false;
        } catch (...) {
          std::wcout << L"Unknown Exception!" << std::endl;
          return false;
        }

        // If successfully joined, check some handles
        if (successfullyJoined) {
          Module definedModule = _moduleList.getDefined(fullFomModules);

          for (std::map<std::wstring, InteractionClass>::const_iterator i = allModule._interactionClasses.begin();
               i != allModule._interactionClasses.end(); ++i) {
            bool expectedAvailable = definedModule.getInteractionClassDefined(i->first);

            rti1516e::InteractionClassHandle interactionClassHandle;
            try {
              interactionClassHandle = ambassador.getInteractionClassHandle(i->first);
              // We can only know in the disjoint case what is finally not included
              // in the object model, since other federates might have pushed object
              // models that we don't know of
              if (!expectedAvailable && getDisjointFederations()) {
                std::wcout << L"getInteractionClassHandle(" << i->first
                           << L") does not fail as required!" << std::endl;
                return false;
              }
            } catch (const rti1516e::NameNotFound& e) {
              if (expectedAvailable) {
                std::wcout << L"getInteractionClassHandle(" << i->first
                           << L") does not succeed as required!" << std::endl;
                std::wcout << L"rti1516e::NameNotFound: \"" << e.what() << L"\"" << std::endl;
                return false;
              }
            } catch (const rti1516e::Exception& e) {
              std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
              return false;
            } catch (...) {
              std::wcout << L"Unknown Exception!" << std::endl;
              return false;
            }

            if (interactionClassHandle.isValid()) {
              for (std::set<std::wstring>::const_iterator k = i->second._parameterNames.begin();
                   k != i->second._parameterNames.end(); ++k) {
                bool expectedParameterAvailable = definedModule.getParameterDefined(i->first, *k);
                try {
                  ambassador.getParameterHandle(interactionClassHandle, *k);
                  // We can only know in the disjoint case what is finally not included
                  // in the object model, since other federates might have pushed object
                  // models that we don't know of
                  if (!expectedParameterAvailable && getDisjointFederations()) {
                    std::wcout << L"getParameterHandle(" << i->first << L", "
                               << *k << ") does not fail as required!" << std::endl;
                    return false;
                  }
                } catch (const rti1516e::NameNotFound& e) {
                  if (expectedParameterAvailable) {
                    std::wcout << L"getParameterHandle(" << i->first << L", "
                               << *k << L") does not succeed as required!" << std::endl;
                    std::wcout << L"rti1516e::NameNotFound: \"" << e.what() << L"\"" << std::endl;
                    return false;
                  }
                } catch (const rti1516e::Exception& e) {
                  std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
                  return false;
                } catch (...) {
                  std::wcout << L"Unknown Exception!" << std::endl;
                  return false;
                }
              }
            }
          }

          for (std::map<std::wstring, ObjectClass>::const_iterator i = allModule._objectClasses.begin();
               i != allModule._objectClasses.end(); ++i) {
            bool expectedAvailable = definedModule.getObjectClassDefined(i->first);

            rti1516e::ObjectClassHandle objectClassHandle;
            try {
              objectClassHandle = ambassador.getObjectClassHandle(i->first);
              // We can only know in the disjoint case what is finally not included
              // in the object model, since other federates might have pushed object
              // models that we don't know of
              if (!expectedAvailable && getDisjointFederations()) {
                std::wcout << L"getObjectClassHandle(" << i->first
                           << L") does not fail as required!" << std::endl;
                return false;
              }
            } catch (const rti1516e::NameNotFound& e) {
              if (expectedAvailable) {
                std::wcout << L"getObjectClassHandle(" << i->first
                           << L") does not succeed as required!" << std::endl;
                std::wcout << L"rti1516e::NameNotFound: \"" << e.what() << L"\"" << std::endl;
                return false;
              }
            } catch (const rti1516e::Exception& e) {
              std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
              return false;
            } catch (...) {
              std::wcout << L"Unknown Exception!" << std::endl;
              return false;
            }

            if (objectClassHandle.isValid()) {
              for (std::set<std::wstring>::const_iterator k = i->second._attributeNames.begin();
                   k != i->second._attributeNames.end(); ++k) {
                bool expectedAttributeAvailable = definedModule.getAttributeDefined(i->first, *k);
                try {
                  ambassador.getAttributeHandle(objectClassHandle, *k);
                  // We can only know in the disjoint case what is finally not included
                  // in the object model, since other federates might have pushed object
                  // models that we don't know of
                  if (!expectedAttributeAvailable && getDisjointFederations()) {
                    std::wcout << L"getAttributeHandle(" << i->first << L", "
                               << *k << ") does not fail as required!" << std::endl;
                    return false;
                  }
                } catch (const rti1516e::NameNotFound& e) {
                  if (expectedAttributeAvailable) {
                    std::wcout << L"getAttributeHandle(" << i->first << L", "
                               << *k << L") does not succeed as required!" << std::endl;
                    std::wcout << L"rti1516e::NameNotFound: \"" << e.what() << L"\"" << std::endl;
                    return false;
                  }
                } catch (const rti1516e::Exception& e) {
                  std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
                  return false;
                } catch (...) {
                  std::wcout << L"Unknown Exception!" << std::endl;
                  return false;
                }
              }
            }
          }
        }

        // and now resign must work
        try {
          ambassador.resignFederationExecution(rti1516e::NO_ACTION);
          if (!successfullyJoined) {
            std::wcout << L"resignFederationExecution does not fail as required!" << std::endl;
            return false;
          }
        } catch (const rti1516e::FederateNotExecutionMember& e) {
          if (successfullyJoined) {
            std::wcout << L"resignFederationExecution does not succeed as required!" << std::endl;
            std::wcout << L"rti1516e::FederateNotExecutionMember: \"" << e.what() << L"\"" << std::endl;
            return false;
          }
        } catch (const rti1516e::Exception& e) {
          std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
          return false;
        } catch (...) {
          std::wcout << L"Unknown Exception!" << std::endl;
          return false;
        }

      }

      // destroy, must work if created
      try {
        ambassador.destroyFederationExecution(getFederationExecution());
        if (!successfullyCreated && getDisjointFederations()) {
          std::wcout << L"destroyFederationExecution does not fail as required!" << std::endl;
          return false;
        }
      } catch (const rti1516e::FederationExecutionDoesNotExist& e) {
        if (successfullyCreated && getDisjointFederations()) {
          std::wcout << L"destroyFederationExecution does not succeed as required!" << std::endl;
          std::wcout << L"rti1516e::FederationExecutionDoesNotExist: \"" << e.what() << L"\"" << std::endl;
          return false;
        }
      } catch (const rti1516e::FederatesCurrentlyJoined& e) {
        if (successfullyCreated && getDisjointFederations()) {
          std::wcout << L"destroyFederationExecution does not succeed as required!" << std::endl;
          std::wcout << L"rti1516e::FederatesCurrentlyJoined: \"" << e.what() << L"\"" << std::endl;
          return false;
        }
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

  ModuleList _moduleList;
};

class OPENRTI_LOCAL Test : public RTITest {
public:
  Test(int argc, const char* const argv[]) :
    RTITest(argc, argv, false)
  {
    insertOptionString("D");
  }

  virtual bool processOption(char optchar, const std::string& argument)
  {
    switch (optchar) {
    case 'D':
      setDisjointFederations(true);
      return true;
    default:
      return RTITest::processOption(optchar, argument);
    }
  }

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
