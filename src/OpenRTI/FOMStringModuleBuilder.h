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

#ifndef OpenRTI_FOMStringModuleBuilder_h
#define OpenRTI_FOMStringModuleBuilder_h

#include <list>
#include <string>
#include <vector>

#include "Message.h"
#include "Referenced.h"
#include "SharedPtr.h"
#include "StringUtils.h"

namespace OpenRTI {

class OPENRTI_LOCAL FOMStringModuleBuilder {
public:

  void addTransportationType()
  {
    _module.getTransportationTypeList().push_back(FOMStringTransportationType());
  }
  FOMStringTransportationType& getCurrentTransportationType()
  {
    if (_module.getTransportationTypeList().empty())
      throw RTIinternalError("No current transportation type.");
    return _module.getTransportationTypeList().back();
  }

  void addUpdateRate()
  {
    _module.getUpdateRateList().push_back(FOMStringUpdateRate());
    _module.getUpdateRateList().back().setRate(0);
  }
  FOMStringUpdateRate& getCurrentUpdateRate()
  {
    if (_module.getUpdateRateList().empty())
      throw RTIinternalError("No current updateRate.");
    return _module.getUpdateRateList().back();
  }

  void addDimension()
  {
    _module.getDimensionList().push_back(FOMStringDimension());
    _module.getDimensionList().back().setUpperBound(0);
  }
  FOMStringDimension& getCurrentDimension()
  {
    if (_module.getDimensionList().empty())
      throw RTIinternalError("No current dimension.");
    return _module.getDimensionList().back();
  }

  void pushObjectClass()
  {
    size_t parentIndex;
    if (_objectClassIndexStack.empty()) {
      parentIndex = ~size_t(0);
    } else {
      parentIndex = _objectClassIndexStack.back();
    }
    size_t index = _module.getObjectClassList().size();
    _objectClassIndexStack.push_back(index);
    _parentObjectClassIndexVector.push_back(parentIndex);
    _module.getObjectClassList().resize(index + 1);
  }
  void popObjectClass()
  {
    _objectClassIndexStack.pop_back();
  }
  FOMStringObjectClass& getCurrentObjectClass()
  {
    if (_objectClassIndexStack.empty())
      throw RTIinternalError("No current object class.");
    if (_module.getObjectClassList().size() <= _objectClassIndexStack.back())
      throw RTIinternalError("No current object class.");
    return _module.getObjectClassList()[_objectClassIndexStack.back()];
  }

  void addAttribute()
  {
    getCurrentObjectClass().getAttributeList().push_back(FOMStringAttribute());
  }
  FOMStringAttribute& getCurrentObjectClassAttribute()
  {
    if (getCurrentObjectClass().getAttributeList().empty())
      throw RTIinternalError("No current attribute.");
    return getCurrentObjectClass().getAttributeList().back();
  }
  void addAttributeDimension(const std::string& name)
  {
    if (!getCurrentObjectClassAttribute().getDimensionSet().insert(name).second) {
      std::stringstream ss;
      ss << "Duplicate dimension \"" << name << "\" while processing attribute \""
         << getCurrentObjectClass().getAttributeList().back().getName() << "\" for ObjectClass \""
         << getCurrentObjectClass().getName() << "\"";
      throw ErrorReadingFDD(ss.str());
    }
  }

  void pushInteractionClass()
  {
    size_t parentIndex;
    if (_interactionClassIndexStack.empty()) {
      parentIndex = ~size_t(0);
    } else {
      parentIndex = _interactionClassIndexStack.back();
    }
    size_t index = _module.getInteractionClassList().size();
    _interactionClassIndexStack.push_back(index);
    _parentInteractionClassIndexVector.push_back(parentIndex);
    _module.getInteractionClassList().resize(index + 1);
  }
  void popInteractionClass()
  {
    _interactionClassIndexStack.pop_back();
  }
  FOMStringInteractionClass& getCurrentInteractionClass()
  {
    if (_interactionClassIndexStack.empty())
      throw RTIinternalError("No current interaction class.");
    if (_module.getInteractionClassList().size() <= _interactionClassIndexStack.back())
      throw RTIinternalError("No current interaction class.");
    return _module.getInteractionClassList()[_interactionClassIndexStack.back()];
  }
  void addInteractionDimension(const std::string& name)
  {
    if (!getCurrentInteractionClass().getDimensionSet().insert(name).second) {
      std::stringstream ss;
      ss << "Duplicate dimension \"" << name << "\" while processing InteractionClass \""
         << getCurrentInteractionClass().getName() << "\"";
      throw ErrorReadingFDD(ss.str());
    }
  }

  void addParameter()
  {
    getCurrentInteractionClass().getParameterList().push_back(FOMStringParameter());
  }
  FOMStringParameter& getCurrentInteractionClassParameter()
  {
    if (getCurrentInteractionClass().getParameterList().empty())
      throw RTIinternalError("No current parameter.");
    return getCurrentInteractionClass().getParameterList().back();
  }

  void validate()
  {
    // Complete the parent class names
    OpenRTIAssert(_parentInteractionClassIndexVector.size() == _module.getInteractionClassList().size());
    for (size_t i = 0; i < _parentInteractionClassIndexVector.size(); ++i) {
      size_t parentIndex = _parentInteractionClassIndexVector[i];
      if (parentIndex == ~std::size_t(0))
        continue;
      OpenRTIAssert(parentIndex < _parentInteractionClassIndexVector.size());
      StringVector stringVector = _module.getInteractionClassList()[parentIndex].getName();
      OpenRTIAssert(_module.getInteractionClassList()[i].getName().size() == 1);
      stringVector.push_back(_module.getInteractionClassList()[i].getName().front());
      _module.getInteractionClassList()[i].getName().swap(stringVector);
    }
    OpenRTIAssert(_parentObjectClassIndexVector.size() == _module.getObjectClassList().size());
    for (size_t i = 0; i < _parentObjectClassIndexVector.size(); ++i) {
      size_t parentIndex = _parentObjectClassIndexVector[i];
      if (parentIndex == ~std::size_t(0))
        continue;
      OpenRTIAssert(parentIndex < _parentObjectClassIndexVector.size());
      StringVector stringVector = _module.getObjectClassList()[parentIndex].getName();
      OpenRTIAssert(_module.getObjectClassList()[i].getName().size() == 1);
      stringVector.push_back(_module.getObjectClassList()[i].getName().front());
      _module.getObjectClassList()[i].getName().swap(stringVector);
    }

    // Check the interaction/object class names to be distinct
    StringSet transportationTypeNames;
    for (size_t i = 0; i < _module.getTransportationTypeList().size(); ++i) {
      std::string name = _module.getTransportationTypeList()[i].getName();
      if (name.empty())
        throw ErrorReadingFDD(std::string("Empty transportation type name."));
      if (!transportationTypeNames.insert(name).second)
        throw ErrorReadingFDD(std::string("Duplicate transportation type name \"") + name + "\".");
    }
    StringSet dimensionNames;
    for (size_t i = 0; i < _module.getDimensionList().size(); ++i) {
      std::string name = _module.getDimensionList()[i].getName();
      if (name.empty())
        throw ErrorReadingFDD(std::string("Empty dimension name."));
      if (!dimensionNames.insert(name).second)
        throw ErrorReadingFDD(std::string("Duplicate dimension name \"") + name + "\".");
    }
    std::set<StringVector> stringVectorSet;
    for (size_t i = 0; i < _module.getInteractionClassList().size(); ++i) {
      StringVector name = _module.getInteractionClassList()[i].getName();
      if (name.empty())
        throw ErrorReadingFDD(std::string("Empty interaction class name."));
      if (!stringVectorSet.insert(name).second)
        throw ErrorReadingFDD(std::string("Duplicate interactionClass name \"") + fqClassName(name) + "\".");

      // std::string transportationType = _module.getInteractionClassList()[i].getTransportationType();
      // if (!transportationType.empty()) {
      //   if (transportationTypeNames.find(transportationType) == transportationTypeNames.end())
      //     throw ErrorReadingFDD(std::string("Undefined transportation type for interactionClass name \"") + name + "\".");
      // }

      for (StringSet::iterator j = _module.getInteractionClassList()[i].getDimensionSet().begin();
           j != _module.getInteractionClassList()[i].getDimensionSet().end(); ++j) {
        std::string dimension = *j;
        if (dimensionNames.find(dimension) == dimensionNames.end())
          throw ErrorReadingFDD(std::string("Undefined dimension for interactionClass name \"") + fqClassName(name) + "\".");
      }
    }
    stringVectorSet.clear();
    for (size_t i = 0; i < _module.getObjectClassList().size(); ++i) {
      StringVector name = _module.getObjectClassList()[i].getName();
      if (name.empty())
        throw ErrorReadingFDD(std::string("Empty object class name."));
      if (!stringVectorSet.insert(name).second)
        throw ErrorReadingFDD(std::string("Duplicate objectClass name \"") + fqClassName(name) + "\".");

      for (size_t j = 0; j < _module.getObjectClassList()[i].getAttributeList().size(); ++j) {
      //   std::string transportationType = _module.getObjectClassList()[i].getAttributeList()[j].getTransportationType();
      //   if (!transportationType.empty()) {
      //     if (transportationTypeNames.find(transportationType) == transportationTypeNames.end())
      //       throw ErrorReadingFDD(std::string("Undefined transportation type for attribute \"")
      //                             + _module.getObjectClassList()[i].getAttributeList()[j].getName()
      //                             + "\" in objectClass name \"" + name + "\".");
      //   }

        for (StringSet::iterator k = _module.getObjectClassList()[i].getAttributeList()[j].getDimensionSet().begin();
             k != _module.getObjectClassList()[i].getAttributeList()[j].getDimensionSet().end(); ++k) {
          std::string dimension = *k;
          if (dimensionNames.find(dimension) == dimensionNames.end())
            throw ErrorReadingFDD(std::string("Undefined dimension for attribute \"")
                                + _module.getObjectClassList()[i].getAttributeList()[j].getName()
                                + "\" in objectClass name \"" + fqClassName(name) + "\".");
        }
      }
    }

    // Insert an artificial interaction root if not yet there.
    // note that the rti13 fed file reader normalizes its different name to the rti1516* standard one checked here.
    StringVector interactionRootName;
    // Hmm may be just the reverse? throw out this object and mention the non standard names for this
    interactionRootName.push_back("HLAinteractionRoot");
    if (_module.getInteractionClassList().empty() || _module.getInteractionClassList()[0].getName() != interactionRootName) {
      _module.setArtificialInteractionRoot(true);
      _module.getInteractionClassList().insert(_module.getInteractionClassList().begin(), FOMStringInteractionClass());
      for (size_t i = 0; i < _module.getInteractionClassList().size(); ++i) {
        _module.getInteractionClassList()[i].getName().insert(_module.getInteractionClassList()[i].getName().begin(), interactionRootName.front());
      }
    } else {
      _module.setArtificialInteractionRoot(false);
    }

    for (size_t i = 0; i < _module.getInteractionClassList().size(); ++i) {
      StringSet stringSet;
      size_t j = i;
      while (_module.getInteractionClassList().size() <= j) {
        for (size_t k = 0; k < _module.getInteractionClassList()[j].getParameterList().size(); ++k) {
          std::string name = _module.getInteractionClassList()[j].getParameterList()[k].getName();
          if (!stringSet.insert(name).second)
            throw ErrorReadingFDD(std::string("Duplicate parameter name \"") + name + "\" in interactionClass \""
                                  + fqClassName(_module.getInteractionClassList()[i].getName()) + "\".");
        }
        j = _parentInteractionClassIndexVector[j];
      }
    }

    // Insert an artificial object class if not yet there.
    // note that the rti13 fed file reader normalizes its different name to the rti1516* standard one checked here.
    StringVector objectRootName;
    objectRootName.push_back("HLAobjectRoot");
    if (_module.getObjectClassList().empty() || _module.getObjectClassList()[0].getName() != objectRootName) {
      _module.setArtificialObjectRoot(true);
      _module.getObjectClassList().insert(_module.getObjectClassList().begin(), FOMStringObjectClass());
      for (size_t i = 0; i < _module.getObjectClassList().size(); ++i) {
        _module.getObjectClassList()[i].getName().insert(_module.getObjectClassList()[i].getName().begin(), objectRootName.front());
      }
    } else {
      _module.setArtificialObjectRoot(false);
    }
    // on any price this may be artificial object class must have exactly this single attribute
    // Note that this resize preserves everything originally contained there.
    // Only the name is normalized in any case.
    _module.getObjectClassList()[0].getAttributeList().resize(1);
    _module.getObjectClassList()[0].getAttributeList()[0].setName("HLAprivilegeToDeleteObject");

    for (size_t i = 0; i < _module.getObjectClassList().size(); ++i) {
      StringSet stringSet;
      size_t j = i;
      while (_module.getObjectClassList().size() <= j) {
        for (size_t k = 0; k < _module.getObjectClassList()[j].getAttributeList().size(); ++k) {
          std::string name = _module.getObjectClassList()[j].getAttributeList()[k].getName();
          if (!stringSet.insert(name).second)
            throw ErrorReadingFDD(std::string("Duplicate attribute name \"") + name + "\" in objectClass \""
                                  + fqClassName(_module.getObjectClassList()[i].getName()) + "\".");
        }
        j = _parentObjectClassIndexVector[j];
      }
    }
  }

  const FOMStringModule& getFOMStringModule() const
  { return _module; }

private:
  typedef std::list<size_t> ObjectClassIndexStack;
  ObjectClassIndexStack _objectClassIndexStack;
  typedef std::vector<size_t> ObjectClassIndexVector;
  ObjectClassIndexVector _parentObjectClassIndexVector;

  typedef std::list<size_t> InteractionClassIndexStack;
  InteractionClassIndexStack _interactionClassIndexStack;
  typedef std::vector<size_t> InteractionClassIndexVector;
  InteractionClassIndexVector _parentInteractionClassIndexVector;

  FOMStringModule _module;
};

}

#endif
