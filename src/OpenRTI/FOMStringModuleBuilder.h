/* -*-c++-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich
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

  void addTransportationType(const std::string& name)
  {
    if (!_transportationTypeNames.insert(name).second)
      throw ErrorReadingFDD(std::string("Duplicate transportation type name \"") + name + "\".");
    _module.getTransportationTypeList().push_back(FOMStringTransportationType());
    _module.getTransportationTypeList().back().setName(name);
  }

  void addDimension(const std::string& name, Unsigned upperBound)
  {
    if (!_dimensionNames.insert(name).second)
      throw ErrorReadingFDD(std::string("Duplicate dimension name \"") + name + "\".");
    _module.getDimensionList().push_back(FOMStringDimension());
    _module.getDimensionList().back().setName(name);
    _module.getDimensionList().back().setUpperBound(upperBound);
  }

  void pushObjectClassData(const std::string& name)
  {
    if (name.empty())
      throw ErrorReadingFDD("Empty object class name is not allowed.");
    if (!_objectClassNames.insert(name).second)
      throw ErrorReadingFDD(std::string("Duplicate object class name \"") + name + "\".");

    if (_objectClassIndexStack.empty()) {
      _objectClassIndexStack.push_back(0);
      _module.getObjectClassList().resize(1);
      _attributeNameStack.push_back(StringSet());
    } else {
      size_t parentIndex = _objectClassIndexStack.back();
      size_t index = _module.getObjectClassList().size();
      _objectClassIndexStack.push_back(index);
      _module.getObjectClassList().resize(index + 1);
      _module.getObjectClassList().back().setParentName(_module.getObjectClassList()[parentIndex].getName());
      _attributeNameStack.push_back(_attributeNameStack.back());
    }
    _module.getObjectClassList().back().setName(name);
  }

  void popObjectClassData()
  {
    _objectClassIndexStack.pop_back();
    _attributeNameStack.pop_back();
  }

  FOMStringObjectClass& getCurrentObjectClass()
  {
    return _module.getObjectClassList()[_objectClassIndexStack.back()];
  }

  void addAttribute(const std::string& name, const std::string& orderType, const std::string& transportationType)
  {
    if (name.empty())
      throw ErrorReadingFDD("Empty attribute name is not allowed.");
    if (_objectClassIndexStack.empty())
      throw ErrorReadingFDD("No current object class.");
    if (!_attributeNameStack.back().insert(name).second)
      throw ErrorReadingFDD(std::string("Duplicate object class attribute name \"") + name + "\"");

    getCurrentObjectClass().getAttributeList().push_back(FOMStringAttribute());
    getCurrentObjectClass().getAttributeList().back().setName(name);
    getCurrentObjectClass().getAttributeList().back().setOrderType(orderType);
    getCurrentObjectClass().getAttributeList().back().setTransportationType(transportationType);
  }

  void addAttributeDimension(const std::string& name)
  {
    if (name.empty())
      throw ErrorReadingFDD("Empty dimension name is not allowed.");
    if (_objectClassIndexStack.empty())
      throw ErrorReadingFDD("No current object class.");
    if (getCurrentObjectClass().getAttributeList().empty())
      throw ErrorReadingFDD("No current attribute.");
    if (!getCurrentObjectClass().getAttributeList().back().getDimensionSet().insert(name).second) {
      std::stringstream ss;
      ss << "Duplicate dimension \"" << name << "\" while processing attribute \""
         << getCurrentObjectClass().getAttributeList().back().getName() << "\" for ObjectClass \""
         << getCurrentObjectClass().getName() << "\"";
      throw ErrorReadingFDD(ss.str());
    }
  }

  void pushInteractionClassData(const std::string& name, const std::string& orderType, const std::string& transportationType)
  {
    if (name.empty())
      throw ErrorReadingFDD("Empty object class name is not allowed.");
    if (!_interactionClassNames.insert(name).second)
      throw ErrorReadingFDD(std::string("Duplicate object class name \"") + name + "\".");

    if (_interactionClassIndexStack.empty()) {
      _interactionClassIndexStack.push_back(0);
      _module.getInteractionClassList().resize(1);
      _parameterNameStack.push_back(StringSet());
    } else {
      size_t parentIndex = _interactionClassIndexStack.back();
      size_t index = _module.getInteractionClassList().size();
      _interactionClassIndexStack.push_back(index);
      _module.getInteractionClassList().resize(index + 1);
      _module.getInteractionClassList().back().setParentName(_module.getInteractionClassList()[parentIndex].getName());
      _parameterNameStack.push_back(_parameterNameStack.back());
    }
    _module.getInteractionClassList().back().setName(name);
    _module.getInteractionClassList().back().setOrderType(orderType);
    _module.getInteractionClassList().back().setTransportationType(transportationType);
  }

  void popInteractionClassData()
  {
    _interactionClassIndexStack.pop_back();
    _parameterNameStack.pop_back();
  }

  FOMStringInteractionClass& getCurrentInteractionClass()
  {
    return _module.getInteractionClassList()[_interactionClassIndexStack.back()];
  }

  void addInteractionDimension(const std::string& name)
  {
    if (name.empty())
      throw ErrorReadingFDD("Empty dimension name is not allowed.");
    if (_interactionClassIndexStack.empty())
      throw ErrorReadingFDD("No current interaction class.");
    if (!getCurrentInteractionClass().getDimensionSet().insert(name).second) {
      std::stringstream ss;
      ss << "Duplicate dimension \"" << name << "\" while processing InteractionClass \""
         << getCurrentInteractionClass().getName() << "\"";
      throw ErrorReadingFDD(ss.str());
    }
  }

  void addParameter(const std::string& name)
  {
    if (name.empty())
      throw ErrorReadingFDD("Empty parameter name is not allowed.");
    if (_interactionClassIndexStack.empty())
      throw ErrorReadingFDD("No current interaction class.");
    if (!_parameterNameStack.back().insert(name).second)
      throw ErrorReadingFDD(std::string("Duplicate interaction class parameter name \"") + name + "\"");
    getCurrentInteractionClass().getParameterList().push_back(FOMStringParameter());
    getCurrentInteractionClass().getParameterList().back().setName(name);
  }

  const FOMStringModule& getFOMStringModule() const
  { return _module; }

private:
  StringSet _transportationTypeNames;
  StringSet _dimensionNames;

  typedef std::list<StringSet> StringSetStack;

  typedef std::list<size_t> ObjectClassIndexStack;
  ObjectClassIndexStack _objectClassIndexStack;
  StringSet _objectClassNames;
  StringSetStack _attributeNameStack;

  typedef std::list<size_t> InteractionClassIndexStack;
  InteractionClassIndexStack _interactionClassIndexStack;
  StringSet _interactionClassNames;
  StringSetStack _parameterNameStack;

  FOMStringModule _module;
};

}

#endif
