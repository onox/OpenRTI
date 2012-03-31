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

#ifndef OpenRTI_FOMStringModuleBuilder_h
#define OpenRTI_FOMStringModuleBuilder_h

#include <list>
#include <string>
#include <vector>

#include "Message.h"
#include "Referenced.h"
#include "SharedPtr.h"

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

  void addDimension()
  {
    _module.getDimensionList().push_back(FOMStringDimension());
  }
  FOMStringDimension& getCurrentDimension()
  {
    if (_module.getDimensionList().empty())
      throw RTIinternalError("No current dimension.");
    return _module.getDimensionList().back();
  }

  void pushObjectClass()
  {
    if (_objectClassIndexStack.empty()) {
      _objectClassIndexStack.push_back(0);
      _parentObjectClassIndexVector.push_back(~size_t(0));
      _module.getObjectClassList().resize(1);
    } else {
      size_t parentIndex = _objectClassIndexStack.back();
      size_t index = _module.getObjectClassList().size();
      _objectClassIndexStack.push_back(index);
      _parentObjectClassIndexVector.push_back(parentIndex);
      _module.getObjectClassList().resize(index + 1);
    }
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
    if (_interactionClassIndexStack.empty()) {
      _interactionClassIndexStack.push_back(0);
      _parentInteractionClassIndexVector.push_back(~size_t(0));
      _module.getInteractionClassList().resize(1);
    } else {
      size_t parentIndex = _interactionClassIndexStack.back();
      size_t index = _module.getInteractionClassList().size();
      _interactionClassIndexStack.push_back(index);
      _parentInteractionClassIndexVector.push_back(parentIndex);
      _module.getInteractionClassList().resize(index + 1);
    }
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
    StringSet stringSet;
    for (size_t i = 0; i < _module.getInteractionClassList().size(); ++i) {
      std::string name = _module.getInteractionClassList()[i].getName();
      if (name.empty())
        throw ErrorReadingFDD(std::string("Empty interaction class name."));
      // FIXME: is allowed, must be unique within its parent
      // if (!stringSet.insert(name).second)
      //   throw ErrorReadingFDD(std::string("Duplicate interactionClass name \"") + name + "\".");

      // std::string transportationType = _module.getInteractionClassList()[i].getTransportationType();
      // if (!transportationType.empty()) {
      //   if (transportationTypeNames.find(transportationType) == transportationTypeNames.end())
      //     throw ErrorReadingFDD(std::string("Undefined transportation type for interactionClass name \"") + name + "\".");
      // }

      for (StringSet::iterator j = _module.getInteractionClassList()[i].getDimensionSet().begin();
           j != _module.getInteractionClassList()[i].getDimensionSet().end(); ++j) {
        std::string dimension = *j;
        if (dimensionNames.find(dimension) == dimensionNames.end())
          throw ErrorReadingFDD(std::string("Undefined dimension for interactionClass name \"") + name + "\".");
      }
    }
    stringSet.clear();
    for (size_t i = 0; i < _module.getObjectClassList().size(); ++i) {
      std::string name = _module.getObjectClassList()[i].getName();
      if (name.empty())
        throw ErrorReadingFDD(std::string("Empty object class name."));
      // FIXME: is allowed, must be unique within its parent
      // if (!stringSet.insert(name).second)
      //   throw ErrorReadingFDD(std::string("Duplicate objectClass name \"") + name + "\".");

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
                                + "\" in objectClass name \"" + name + "\".");
        }
      }
    }

    for (size_t i = 0; i < _module.getInteractionClassList().size(); ++i) {
      stringSet.clear();
      size_t j = i;
      while (_module.getInteractionClassList().size() <= j) {
        for (size_t k = 0; k < _module.getInteractionClassList()[j].getParameterList().size(); ++k) {
          std::string name = _module.getInteractionClassList()[j].getParameterList()[k].getName();
          if (!stringSet.insert(name).second)
            throw ErrorReadingFDD(std::string("Duplicate parameter name \"") + name + "\" in interactionClass \""
                                  + _module.getInteractionClassList()[i].getName() + "\".");
        }
        j = _parentInteractionClassIndexVector[j];
      }
    }

    for (size_t i = 0; i < _module.getObjectClassList().size(); ++i) {
      stringSet.clear();
      size_t j = i;
      while (_module.getObjectClassList().size() <= j) {
        for (size_t k = 0; k < _module.getObjectClassList()[j].getAttributeList().size(); ++k) {
          std::string name = _module.getObjectClassList()[j].getAttributeList()[k].getName();
          if (!stringSet.insert(name).second)
            throw ErrorReadingFDD(std::string("Duplicate attribute name \"") + name + "\" in objectClass \""
                                  + _module.getObjectClassList()[i].getName() + "\".");
        }
        j = _parentObjectClassIndexVector[j];
      }
    }

    // Finally set the parent class names
    for (size_t i = 1; i < _parentInteractionClassIndexVector.size(); ++i) {
      size_t parentIndex = _parentInteractionClassIndexVector[i];
      _module.getInteractionClassList()[i].setParentName(_module.getInteractionClassList()[parentIndex].getName());
    }
    for (size_t i = 1; i < _parentObjectClassIndexVector.size(); ++i) {
      size_t parentIndex = _parentObjectClassIndexVector[i];
      _module.getObjectClassList()[i].setParentName(_module.getObjectClassList()[parentIndex].getName());
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
