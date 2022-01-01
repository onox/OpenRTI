/* -*-c++-*- OpenRTI - Copyright (C) 2010-2022 Mathias Froehlich
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

#include "FEDFileReader.h"

#include <fstream>

#include "Exception.h"
#include "FOMStringModuleBuilder.h"
#include "ParenthesesReader.h"

namespace OpenRTI {

class OPENRTI_LOCAL FEDContentHandler : public ParenthesesReader::ContentHandler {
public:
  virtual void startDocument()
  {
    OpenRTIAssert(_modeStack.empty());
    _fomStringModuleBuilder.addTransportationType();
    _fomStringModuleBuilder.getCurrentTransportationType().setName(normalizeTransportationType("reliable"));
    _fomStringModuleBuilder.addTransportationType();
    _fomStringModuleBuilder.getCurrentTransportationType().setName(normalizeTransportationType("best_effort"));
  }
  virtual void endDocument()
  {
    OpenRTIAssert(_modeStack.empty());
    _fomStringModuleBuilder.validate();
  }

  virtual void startElement(const ParenthesesReader&, const StringVector& tokens)
  {
    OpenRTIAssert(!tokens.empty());
    const std::string& t0 = tokens.front();
    if (caseCompare(t0, "FED")) {
      if (!_modeStack.empty())
        throw ErrorReadingFDD("FED token is not at top level!");
      if (1 < tokens.size())
        throw ErrorReadingFDD("FED contains too many tokens!");
      _modeStack.push_back(FEDMode);
    } else if (caseCompare(t0, "Federation")) {
      if (getCurrentMode() != FEDMode)
        throw ErrorReadingFDD("Federation token is not under the FED level!");
      if (tokens.size() < 2)
        throw ErrorReadingFDD("Federation contains no name information!");
      if (2 < tokens.size())
        throw ErrorReadingFDD("Federation contains too many tokens!");
      _modeStack.push_back(FederationMode);
    } else if (caseCompare(t0, "FEDversion")) {
      if (getCurrentMode() != FEDMode)
        throw ErrorReadingFDD("FEDversion token is not under the FED level!");
      if (tokens.size() < 2)
        throw ErrorReadingFDD("FEDversion contains no version information!");
      if (2 < tokens.size())
        throw ErrorReadingFDD("FEDversion contains too many tokens!");
      if (tokens[1] != "1.3" && !caseCompare(tokens[1], "v1.3") && tokens[1] != "1_3" && !caseCompare(tokens[1], "v1_3"))
        throw ErrorReadingFDD("FEDversion \"" + tokens[1] + "\" is not supported!");
      _modeStack.push_back(FEDversionMode);
    } else if (caseCompare(t0, "spaces")) {
      if (getCurrentMode() != FEDMode)
        throw ErrorReadingFDD("spaces token is not under the FED level!");
      if (1 < tokens.size())
        throw ErrorReadingFDD("spaces contains too many tokens!");
      _modeStack.push_back(SpacesMode);
    } else if (caseCompare(t0, "space")) {
      if (getCurrentMode() != SpacesMode)
        throw ErrorReadingFDD("space token is not under the spaces level!");
      if (tokens.size() < 2)
        throw ErrorReadingFDD("space contains no name information!");
      if (2 < tokens.size())
        throw ErrorReadingFDD("space contains too many tokens!");
      _modeStack.push_back(SpaceMode);
    } else if (caseCompare(t0, "dimension")) {
      if (getCurrentMode() != SpaceMode)
        throw ErrorReadingFDD("dimension token is not under the space level!");
      if (tokens.size() < 2)
        throw ErrorReadingFDD("dimension contains no name information!");
      if (2 < tokens.size())
        throw ErrorReadingFDD("dimension contains too many tokens!");
      _modeStack.push_back(DimensionMode);
    } else if (caseCompare(t0, "objects")) {
      if (getCurrentMode() != FEDMode)
        throw ErrorReadingFDD("objects token is not under the FED level!");
      if (1 < tokens.size())
        throw ErrorReadingFDD("objects contains too many tokens!");
      _modeStack.push_back(ObjectsMode);
    } else if (caseCompare(t0, "interactions")) {
      if (getCurrentMode() != FEDMode)
        throw ErrorReadingFDD("interactions token is not under the FED level!");
      if (1 < tokens.size())
        throw ErrorReadingFDD("interactions contains too many tokens!");
      _modeStack.push_back(InteractionsMode);
    } else if (caseCompare(t0, "class")) {
      Mode currentMode = getCurrentMode();
      if (currentMode == ObjectsMode || currentMode == ObjectClassMode) {
        if (tokens.size() < 2)
          throw ErrorReadingFDD("object class contains no name information!");
        if (2 < tokens.size())
          throw ErrorReadingFDD("object class contains too many tokens!");
        _modeStack.push_back(ObjectClassMode);
        _fomStringModuleBuilder.pushObjectClass();
        _fomStringModuleBuilder.getCurrentObjectClass().getName().push_back(normalizeObjectClassName(tokens[1]));
      } else if (currentMode == InteractionsMode || currentMode == InteractionClassMode) {
        if (tokens.size() < 4)
          throw ErrorReadingFDD("interaction class contains too little information!");
        if (5 < tokens.size())
          throw ErrorReadingFDD("interaction class contains too many tokens!");
        _modeStack.push_back(InteractionClassMode);
        _fomStringModuleBuilder.pushInteractionClass();
        _fomStringModuleBuilder.getCurrentInteractionClass().getName().push_back(normalizeInteractionClassName(tokens[1]));
        _fomStringModuleBuilder.getCurrentInteractionClass().setTransportationType(normalizeTransportationType(tokens[2]));
        _fomStringModuleBuilder.getCurrentInteractionClass().setOrderType(normalizeOrderType(tokens[3]));
        /// FIXME
        // if (4 < tokens.size())
        //   _fomStringModuleBuilder.addInteractionSpace(tokens[4]);
      } else {
        throw ErrorReadingFDD("class only allowed in object class or interaction class definitions!");
      }
    } else if (caseCompare(t0, "attribute")) {
      if (getCurrentMode() != ObjectClassMode)
        throw ErrorReadingFDD("attribute token is not under an object class level!");
      if (tokens.size() < 4)
        throw ErrorReadingFDD("attribute contains too little information!");
      if (5 < tokens.size())
        throw ErrorReadingFDD("attribute contains too many tokens!");
      _modeStack.push_back(AttributeMode);
      _fomStringModuleBuilder.addAttribute();
      _fomStringModuleBuilder.getCurrentObjectClassAttribute().setName(normalizeObjectClassAttributeName(tokens[1]));
      _fomStringModuleBuilder.getCurrentObjectClassAttribute().setTransportationType(normalizeTransportationType(tokens[2]));
      _fomStringModuleBuilder.getCurrentObjectClassAttribute().setOrderType(normalizeOrderType(tokens[3]));
      /// FIXME
      // if (4 < tokens.size())
      //   _fomStringModuleBuilder.addAttributeSpace(tokens[4]);
    } else if (caseCompare(t0, "parameter")) {
      if (getCurrentMode() != InteractionClassMode)
        throw ErrorReadingFDD("parameter token is not under an interaction class level!");
      if (tokens.size() < 2)
        throw ErrorReadingFDD("interaction class contains no name information!");
      if (2 < tokens.size())
        throw ErrorReadingFDD("interaction class contains too many tokens!");
      _modeStack.push_back(ParameterMode);
      _fomStringModuleBuilder.addParameter();
      _fomStringModuleBuilder.getCurrentInteractionClassParameter().setName(tokens[1]);
    } else {
      _modeStack.push_back(UnknownMode);
    }
  }
  virtual void endElement()
  {
    Mode currentMode = getCurrentMode();
    if (currentMode == ObjectClassMode) {
      _fomStringModuleBuilder.popObjectClass();
    } else if (currentMode == InteractionClassMode) {
      _fomStringModuleBuilder.popInteractionClass();
    }
    _modeStack.pop_back();
  }

  const FOMStringModule& getFOMStringModule() const
  { return _fomStringModuleBuilder.getFOMStringModule(); }

  std::string normalizeTransportationType(const std::string& name)
  {
    if (caseCompare(name, "reliable")) {
      return "HLAreliable";
    } else if (caseCompare(name, "best_effort")) {
      return "HLAbestEffort";
    } else {
      // We allow other transportation types
      return name;
    }
  }

  std::string normalizeOrderType(const std::string& name)
  {
    if (caseCompare(name, "receive")) {
      return "Receive";
    } else {
      // Map everything else to timestamp, should be about safer.
      // If it is not meant timestamp ordered it is probably also not sent
      // as timestamped message and thus just sent as receive ordered in the c code
      // May be we should emit some kind of warning?!
      return "TimeStamp";
    }
  }

  std::string normalizeInteractionClassName(const std::string& name)
  {
    if (caseCompare(name, "InteractionRoot")) {
      return "HLAinteractionRoot";
    } else {
      return name;
    }
  }

  std::string normalizeObjectClassName(const std::string& name)
  {
    if (caseCompare(name, "ObjectRoot")) {
      return "HLAobjectRoot";
    } else {
      return name;
    }
  }

  std::string normalizeObjectClassAttributeName(const std::string& name)
  {
    if (caseCompare(name, "privilegeToDelete")) {
      return "HLAprivilegeToDeleteObject";
    } else {
      return name;
    }
  }

  // poor man's schema checking ...
  enum Mode {
    UnknownMode,

    FEDMode,

    FederationMode,
    FEDversionMode,
    SpacesMode,

    SpaceMode,
    DimensionMode,

    ObjectsMode,
    ObjectClassMode,
    AttributeMode,

    InteractionsMode,
    InteractionClassMode,
    ParameterMode
  };

  Mode getCurrentMode()
  {
    if (_modeStack.empty())
      return UnknownMode;
    return _modeStack.back();
  }

  // Current modes in a stack
  std::vector<Mode> _modeStack;

  // Helper to build up the data
  FOMStringModuleBuilder _fomStringModuleBuilder;
};

class OPENRTI_LOCAL FEDErrorHandler : public ParenthesesReader::ErrorHandler {
public:
  virtual void error(const ParenthesesReader& parenthesesReader, const char* msg)
  {
    std::stringstream stream;
    stream << "Error parsing fed file in line " << parenthesesReader.getLine() << " at column "
           << parenthesesReader.getColumn() << ": " << msg << std::endl;
    throw ErrorReadingFDD(stream.str());
  }
};

FOMStringModule
FEDFileReader::read(std::istream& stream)
{
  FEDContentHandler contentHandler;
  FEDErrorHandler errorHandler;
  ParenthesesReader parenthesesReader;
  if (!parenthesesReader.parse(stream, contentHandler, errorHandler))
    throw ErrorReadingFDD("ParenthesesReader returned false!");

  return contentHandler.getFOMStringModule();
}

} // namespace OpenRTI
