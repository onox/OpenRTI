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

#include "FDD1516EFileReader.h"

#include "DefaultErrorHandler.h"
#include "Exception.h"
#include "ExpatXMLReader.h"
#include "FOMStringModuleBuilder.h"
#include "Message.h"
#include "StringUtils.h"

namespace OpenRTI {

class OPENRTI_LOCAL FDD1516EContentHandler : public XML::ContentHandler {
public:
  FDD1516EContentHandler();
  virtual ~FDD1516EContentHandler();

  virtual void startDocument(void);
  virtual void endDocument(void);
  virtual void startElement(const char* uri, const char* name,
                            const char* qName, const XML::Attributes* atts);
  virtual void endElement(const char* uri, const char* name, const char* qName);

  virtual void characters(const char* data, unsigned length);

  // poor man's schema checking ...
  enum Mode {
    UnknownMode,

    // Top level tag
    ObjectModelMode,

    // First level tags
    ModelIdentificationMode,
    ObjectsMode,
    InteractionsMode,
    DimensionsMode,
    TimeMode,
    TagsMode,
    SynchronizationsMode,
    TransportationsMode,
    SwitchesMode,
    UpdateRatesMode,
    DataTypesMode,
    NotesMode,

    // Second level tags grouped by first level parent
    // Skip ModelIdentification for now

    // The second level and nested object class hierarchy
    ObjectClassMode,

    // Child elements of objectClass
    ObjectClassNameMode,
    ObjectClassSharingMode,
    ObjectClassSemanticsMode,
    ObjectClassAttributeMode,

    // Child elements of attribute
    ObjectClassAttributeNameMode,
    ObjectClassAttributeDataTypeMode,
    ObjectClassAttributeUpdateTypeMode,
    ObjectClassAttributeUpdateConditionMode,
    ObjectClassAttributeOwnershipMode,
    ObjectClassAttributeSharingMode,
    ObjectClassAttributeTransportationMode,
    ObjectClassAttributeOrderMode,
    ObjectClassAttributeSemanticsMode,
    ObjectClassAttributeDimensionsMode,

    // Child elements of attribute dimensions
    ObjectClassAttributeDimensionsDimensionMode,


    // The second level and nested interaction class hierarchy
    InteractionClassMode,

    // Child elements of interactionClass
    InteractionClassNameMode,
    InteractionClassSharingMode,
    InteractionClassTransportationMode,
    InteractionClassOrderMode,
    InteractionClassSemanticsMode,
    InteractionClassParameterMode,
    InteractionClassDimensionsMode,

    // Child elements of parameter
    InteractionClassParameterNameMode,
    InteractionClassParameterDataTypeMode,
    InteractionClassParameterSemanticsMode,

    // Child elements of interaction class dimensions
    InteractionClassDimensionsDimensionMode,


    // Dimensions
    DimensionsDimensionMode,
    DimensionsDimensionNameMode,
    DimensionsDimensionDataTypeMode,
    DimensionsDimensionUpperBoundMode,
    DimensionsDimensionNormalizationMode,
    DimensionsDimensionValueMode,

    // TimeMode,
    // TimeStampMode,
    // LookaheadMode,

    // TagsMode,
    // UpdateReflectTagMode,
    // SendReceiveTagMode,
    // DeleteRemoveTagMode,
    // DivestitureRequestTagMode,
    // DivestitureCompletionTagMode,
    // AcquisitionRequestTagMode,
    // RequestUpdateTagMode,

    // SynchronizationsMode,
    // SynchronizationMode,

    TransportationMode,
    TransportationNameMode,
    TransportationReliableMode,
    TransportationSemanticsMode,

    // SwitchesMode,

    UpdateRateMode,
    UpdateRateNameMode,
    UpdateRateRateMode //,

    // BasicDataRepresentationsMode,
    // BasicDataMode,
    // SimpleDataTypesMode,
    // SimpleDataMode,
    // EnumeratedDataTypesMode,
    // EnumeratedDataMode,
    // EnumeratorMode,
    // ArrayDataTypesMode,
    // ArrayDataMode,
    // FixedRecordDataTypesMode,
    // FixedRecordDataMode,
    // FieldMode,
    // VariantRecordDataTypesMode,
    // VariantRecordDataMode,
    // AlternativeDataMode,

    // NotesMode,
    // NoteMode
  };

  Mode getCurrentMode()
  {
    if (_modeStack.empty())
      return UnknownMode;
    return _modeStack.back();
  }

  // Current modes in a stack
  std::vector<Mode> _modeStack;

  // Character data content. Is only accumulated in the leaf elements.
  std::string _characterData;

  // Helper to build up the data
  FOMStringModuleBuilder _fomStringModuleBuilder;
};

FDD1516EContentHandler::FDD1516EContentHandler()
{
}

FDD1516EContentHandler::~FDD1516EContentHandler()
{
}

void
FDD1516EContentHandler::startDocument(void)
{
  OpenRTIAssert(_modeStack.empty());
}

void
FDD1516EContentHandler::endDocument(void)
{
  OpenRTIAssert(_modeStack.empty());
  _fomStringModuleBuilder.validate();
}

void
FDD1516EContentHandler::startElement(const char* uri, const char* name,
                                     const char* qName, const XML::Attributes* atts)
{
  /// Tags that can happen at multiple places
  if (strcmp(name, "name") == 0) {
    switch (getCurrentMode()) {
    case ObjectClassMode:
      _modeStack.push_back(ObjectClassNameMode);
      break;
    case ObjectClassAttributeMode:
      _modeStack.push_back(ObjectClassAttributeNameMode);
      break;
    case InteractionClassMode:
      _modeStack.push_back(InteractionClassNameMode);
      break;
    case InteractionClassParameterMode:
      _modeStack.push_back(InteractionClassParameterNameMode);
      break;
    case DimensionsDimensionMode:
      _modeStack.push_back(DimensionsDimensionNameMode);
      break;
    case TransportationMode:
      _modeStack.push_back(TransportationNameMode);
      break;
    case UpdateRateMode:
      _modeStack.push_back(UpdateRateNameMode);
      break;
    default:
      // throw ErrorReadingFDD("unexpected name tag!");
      _modeStack.push_back(UnknownMode);
      break;
    }

  } else if (strcmp(name, "sharing") == 0) {
    switch (getCurrentMode()) {
    case UnknownMode:
      _modeStack.push_back(UnknownMode);
      break;
    case ObjectClassMode:
      _modeStack.push_back(ObjectClassSharingMode);
      break;
    case ObjectClassAttributeMode:
      _modeStack.push_back(ObjectClassAttributeSharingMode);
      break;
    case InteractionClassMode:
      _modeStack.push_back(InteractionClassSharingMode);
      break;
    default:
      // throw ErrorReadingFDD("unexpected sharing tag!");
      _modeStack.push_back(UnknownMode);
      break;
    }

  } else if (strcmp(name, "semantics") == 0) {
    switch (getCurrentMode()) {
    case UnknownMode:
      _modeStack.push_back(UnknownMode);
      break;
    case ObjectClassMode:
      _modeStack.push_back(ObjectClassSemanticsMode);
      break;
    case ObjectClassAttributeMode:
      _modeStack.push_back(ObjectClassAttributeSemanticsMode);
      break;
    case InteractionClassMode:
      _modeStack.push_back(InteractionClassSemanticsMode);
      break;
    case InteractionClassParameterMode:
      _modeStack.push_back(InteractionClassParameterSemanticsMode);
      break;
    case TransportationMode:
      _modeStack.push_back(TransportationSemanticsMode);
      break;
    default:
      // throw ErrorReadingFDD("unexpected semantics tag!");
      _modeStack.push_back(UnknownMode);
      break;
    }

  } else if (strcmp(name, "dataType") == 0) {
    switch (getCurrentMode()) {
    case ObjectClassAttributeMode:
      _modeStack.push_back(ObjectClassAttributeDataTypeMode);
      break;
    case InteractionClassParameterMode:
      _modeStack.push_back(InteractionClassParameterDataTypeMode);
      break;
    case DimensionsDimensionMode:
      _modeStack.push_back(DimensionsDimensionDataTypeMode);
      break;
    default:
      // throw ErrorReadingFDD("unexpected dataType tag!");
      _modeStack.push_back(UnknownMode);
      break;
    }

  } else if (strcmp(name, "transportation") == 0) {
    switch (getCurrentMode()) {
    case ObjectClassAttributeMode:
      _modeStack.push_back(ObjectClassAttributeTransportationMode);
      break;
    case InteractionClassMode:
      _modeStack.push_back(InteractionClassTransportationMode);
      break;
    case TransportationsMode:
      _modeStack.push_back(TransportationMode);
      _fomStringModuleBuilder.addTransportationType();
      break;
    default:
      // throw ErrorReadingFDD("unexpected transportation tag!");
      _modeStack.push_back(UnknownMode);
      break;
    }

  } else if (strcmp(name, "order") == 0) {
    switch (getCurrentMode()) {
    case ObjectClassAttributeMode:
      _modeStack.push_back(ObjectClassAttributeOrderMode);
      break;
    case InteractionClassMode:
      _modeStack.push_back(InteractionClassOrderMode);
      break;
    default:
      // throw ErrorReadingFDD("unexpected order tag!");
      _modeStack.push_back(UnknownMode);
      break;
    }

  } else if (strcmp(name, "dimensions") == 0) {
    switch (getCurrentMode()) {
    case ObjectClassAttributeMode:
      _modeStack.push_back(ObjectClassAttributeDimensionsMode);
      break;
    case InteractionClassMode:
      _modeStack.push_back(InteractionClassDimensionsMode);
      break;
    case ObjectModelMode:
      _modeStack.push_back(DimensionsMode);
      break;
    default:
      // throw ErrorReadingFDD("unexpected dimensions tag!");
      _modeStack.push_back(UnknownMode);
      break;
    }

  } else if (strcmp(name, "dimension") == 0) {
    switch (getCurrentMode()) {
    case ObjectClassAttributeDimensionsMode:
      _modeStack.push_back(ObjectClassAttributeDimensionsDimensionMode);
      break;
    case InteractionClassDimensionsMode:
      _modeStack.push_back(InteractionClassDimensionsDimensionMode);
      break;
    case DimensionsMode:
      _modeStack.push_back(DimensionsDimensionMode);
      _fomStringModuleBuilder.addDimension();
      break;
    default:
      // throw ErrorReadingFDD("unexpected dimension tag!");
      _modeStack.push_back(UnknownMode);
      break;
    }


    /// Top level tag
  } else if (strcmp(name, "objectModel") == 0) {
    _modeStack.push_back(ObjectModelMode);


    /// First level tags
  } else if (strcmp(name, "modelIdentification") == 0) {
    _modeStack.push_back(ModelIdentificationMode);

  } else if (strcmp(name, "objects") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("objects tag outside objectModel!");
    _modeStack.push_back(ObjectsMode);

  } else if (strcmp(name, "interactions") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("interactions tag outside objectModel!");
    _modeStack.push_back(InteractionsMode);

  } else if (strcmp(name, "time") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("time tag outside objectModel!");
    _modeStack.push_back(TimeMode);

  } else if (strcmp(name, "tags") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("tags tag outside objectModel!");
    _modeStack.push_back(TagsMode);

  } else if (strcmp(name, "synchronizations") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("synchronizations tag outside objectModel!");
    _modeStack.push_back(SynchronizationsMode);

  } else if (strcmp(name, "transportations") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("transportations tag outside objectModel!");
    _modeStack.push_back(TransportationsMode);

  } else if (strcmp(name, "switches") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("switches tag outside objectModel!");
    _modeStack.push_back(SwitchesMode);

  } else if (strcmp(name, "dataTypes") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("dataTypes tag outside objectModel!");
    _modeStack.push_back(DataTypesMode);

  } else if (strcmp(name, "notes") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("notes tag outside objectModel!");
    _modeStack.push_back(NotesMode);


    /// updateRates
  } else if (strcmp(name, "updateRates") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("updateRates tag outside objectModel!");
    _modeStack.push_back(UpdateRatesMode);

  } else if (strcmp(name, "updateRate") == 0) {
    if (getCurrentMode() != UpdateRatesMode)
      throw ErrorReadingFDD("updateRates tag outside updateRates!");
    _modeStack.push_back(UpdateRateMode);
    _fomStringModuleBuilder.addUpdateRate();

  } else if (strcmp(name, "rate") == 0) {
    if (getCurrentMode() != UpdateRateMode)
      throw ErrorReadingFDD("rate tag outside updateRate!");
    _modeStack.push_back(UpdateRateRateMode);


    /// objectClass hierarchies
  } else if (strcmp(name, "objectClass") == 0) {
    if (getCurrentMode() != ObjectsMode && getCurrentMode() != ObjectClassMode)
      throw ErrorReadingFDD("objectClass tag outside objectClass or objects!");
    _modeStack.push_back(ObjectClassMode);

    _fomStringModuleBuilder.pushObjectClass();

  } else if (strcmp(name, "attribute") == 0) {
    if (getCurrentMode() != ObjectClassMode)
      throw ErrorReadingFDD("attribute tag outside objectClass!");
    _modeStack.push_back(ObjectClassAttributeMode);

    _fomStringModuleBuilder.addAttribute();

  } else if (strcmp(name, "updateType") == 0) {
    if (getCurrentMode() != ObjectClassAttributeMode)
      throw ErrorReadingFDD("updateType tag outside attribute!");
    _modeStack.push_back(ObjectClassAttributeUpdateTypeMode);

  } else if (strcmp(name, "updateCondition") == 0) {
    if (getCurrentMode() != ObjectClassAttributeMode)
      throw ErrorReadingFDD("updateCondition tag outside attribute!");
    _modeStack.push_back(ObjectClassAttributeUpdateConditionMode);

  } else if (strcmp(name, "ownership") == 0) {
    if (getCurrentMode() != ObjectClassAttributeMode)
      throw ErrorReadingFDD("ownership tag outside attribute!");
    _modeStack.push_back(ObjectClassAttributeOwnershipMode);


    /// interactionClass hierarchies
  } else if (strcmp(name, "interactionClass") == 0) {
    if (getCurrentMode() != InteractionsMode && getCurrentMode() != InteractionClassMode)
      throw ErrorReadingFDD("interactionClass tag outside interactionClass or interactions!");
    _modeStack.push_back(InteractionClassMode);

    _fomStringModuleBuilder.pushInteractionClass();

  } else if (strcmp(name, "parameter") == 0) {
    if (getCurrentMode() != InteractionClassMode)
      throw ErrorReadingFDD("parameter tag outside interactionClass!");
    _modeStack.push_back(InteractionClassParameterMode);

    _fomStringModuleBuilder.addParameter();


    /// dimensions
  } else if (strcmp(name, "upperBound") == 0) {
    if (getCurrentMode() != DimensionsDimensionMode)
      throw ErrorReadingFDD("upperBound tag outside dimension!");
    _modeStack.push_back(DimensionsDimensionUpperBoundMode);

  } else if (strcmp(name, "normalization") == 0) {
    if (getCurrentMode() != DimensionsDimensionMode)
      throw ErrorReadingFDD("normalization tag outside dimension!");
    _modeStack.push_back(DimensionsDimensionNormalizationMode);

  } else if (strcmp(name, "value") == 0) {
    if (getCurrentMode() != DimensionsDimensionMode && getCurrentMode() != UnknownMode)
      throw ErrorReadingFDD("value tag outside dimension!");
    _modeStack.push_back(DimensionsDimensionValueMode);


    /// Transportations
  } else if (strcmp(name, "reliable") == 0) {
    if (getCurrentMode() != TransportationMode)
      throw ErrorReadingFDD("reliable tag outside transportation!");
    _modeStack.push_back(TransportationReliableMode);

  } else {
    _modeStack.push_back(UnknownMode);
  }

  _characterData.clear();
}

void
FDD1516EContentHandler::endElement(const char* uri, const char* name, const char* qName)
{
  if (strcmp(name, "name") == 0) {
    switch (getCurrentMode()) {
    case ObjectClassNameMode:
      if (!_fomStringModuleBuilder.getCurrentObjectClass().getName().empty())
        throw ErrorReadingFDD("Duplicate name tag for object class \"" + _characterData + "\"!");
      _fomStringModuleBuilder.getCurrentObjectClass().getName().push_back(_characterData);
      break;
    case ObjectClassAttributeNameMode:
      if (!_fomStringModuleBuilder.getCurrentObjectClassAttribute().getName().empty())
        throw ErrorReadingFDD("Duplicate name tag for object class attribute \"" +
                              _fomStringModuleBuilder.getCurrentObjectClassAttribute().getName() + "\"!");
      _fomStringModuleBuilder.getCurrentObjectClassAttribute().setName(_characterData);
      break;
    case InteractionClassNameMode:
      if (!_fomStringModuleBuilder.getCurrentInteractionClass().getName().empty())
        throw ErrorReadingFDD("Duplicate name tag for interaction class \"" + _characterData + "\"!");
      _fomStringModuleBuilder.getCurrentInteractionClass().getName().push_back(_characterData);
      break;
    case InteractionClassParameterNameMode:
      if (!_fomStringModuleBuilder.getCurrentInteractionClassParameter().getName().empty())
        throw ErrorReadingFDD("Duplicate name tag for interaction class parameter \"" +
                              _fomStringModuleBuilder.getCurrentInteractionClassParameter().getName() + "\"!");
      _fomStringModuleBuilder.getCurrentInteractionClassParameter().setName(_characterData);
      break;
    case DimensionsDimensionNameMode:
      if (!_fomStringModuleBuilder.getCurrentDimension().getName().empty())
        throw ErrorReadingFDD("Duplicate name tag for dimension \"" +
                              _fomStringModuleBuilder.getCurrentDimension().getName() + "\"!");
      _fomStringModuleBuilder.getCurrentDimension().setName(_characterData);
      break;
    case TransportationNameMode:
      if (!_fomStringModuleBuilder.getCurrentTransportationType().getName().empty())
        throw ErrorReadingFDD("Duplicate name tag for transportation \"" +
                              _fomStringModuleBuilder.getCurrentTransportationType().getName() + "\"!");
      _fomStringModuleBuilder.getCurrentTransportationType().setName(_characterData);
      break;
    case UpdateRateNameMode:
      if (!_fomStringModuleBuilder.getCurrentUpdateRate().getName().empty())
        throw ErrorReadingFDD("Duplicate name tag for updateRate \"" +
                              _fomStringModuleBuilder.getCurrentUpdateRate().getName() + "\"!");
      _fomStringModuleBuilder.getCurrentUpdateRate().setName(_characterData);
      break;
    default:
      break;
    }

  } else if (strcmp(name, "transportation") == 0) {
    switch (getCurrentMode()) {
    case ObjectClassAttributeTransportationMode:
      _fomStringModuleBuilder.getCurrentObjectClassAttribute().setTransportationType(_characterData);
      break;
    case InteractionClassTransportationMode:
      _fomStringModuleBuilder.getCurrentInteractionClass().setTransportationType(_characterData);
      break;
    case TransportationNameMode:
      if (_fomStringModuleBuilder.getCurrentTransportationType().getName().empty())
        throw ErrorReadingFDD("No name given for transportation!");
      break;
    default:
      break;
    }

  } else if (strcmp(name, "rate") == 0) {
    std::stringstream ss(_characterData);
    double rate = 0;
    ss >> rate;
    _fomStringModuleBuilder.getCurrentUpdateRate().setRate(rate);

  } else if (strcmp(name, "order") == 0) {
    switch (getCurrentMode()) {
    case ObjectClassAttributeOrderMode:
      _fomStringModuleBuilder.getCurrentObjectClassAttribute().setOrderType(_characterData);
      break;
    case InteractionClassOrderMode:
      _fomStringModuleBuilder.getCurrentInteractionClass().setOrderType(_characterData);
      break;
    default:
      break;
    }

  } else if (strcmp(name, "dimension") == 0) {
    switch (getCurrentMode()) {
    case ObjectClassAttributeDimensionsDimensionMode:
      _fomStringModuleBuilder.addAttributeDimension(_characterData);
      break;
    case InteractionClassDimensionsDimensionMode:
      _fomStringModuleBuilder.addInteractionDimension(_characterData);
      break;
    case DimensionsDimensionNameMode:
      if (_fomStringModuleBuilder.getCurrentDimension().getName().empty())
        throw ErrorReadingFDD("No name given for dimension!");
      break;
    default:
      break;
    }

  } else if (strcmp(name, "upperBound") == 0) {
    std::stringstream ss(_characterData);
    Unsigned upperBound = 0;
    ss >> upperBound;
    _fomStringModuleBuilder.getCurrentDimension().setUpperBound(upperBound);


  } else if (strcmp(name, "objectClass") == 0) {
    if (_fomStringModuleBuilder.getCurrentObjectClass().getName().size() != 1)
      throw ErrorReadingFDD("No name given for object class!");
    if (_fomStringModuleBuilder.getCurrentObjectClass().getName().front().empty())
      throw ErrorReadingFDD("Empty name given for object class!");
    _fomStringModuleBuilder.popObjectClass();

  } else if (strcmp(name, "attribute") == 0) {
    if (_fomStringModuleBuilder.getCurrentObjectClassAttribute().getName().empty())
      throw ErrorReadingFDD("No or empty name given for object class attribute!");


  } else if (strcmp(name, "interactionClass") == 0) {
    if (_fomStringModuleBuilder.getCurrentInteractionClass().getName().size() != 1)
      throw ErrorReadingFDD("No name given for interaction class!");
    if (_fomStringModuleBuilder.getCurrentInteractionClass().getName().front().empty())
      throw ErrorReadingFDD("Empty name given for interaction class!");
    _fomStringModuleBuilder.popInteractionClass();

  } else if (strcmp(name, "parameter") == 0) {
    if (_fomStringModuleBuilder.getCurrentInteractionClassParameter().getName().empty())
      throw ErrorReadingFDD("No or empty name given for interaction class parameter!");
  }
  _modeStack.pop_back();
  _characterData.clear();
}

void
FDD1516EContentHandler::characters(const char* data, unsigned length)
{
  _characterData.append(data, length);
}

FOMStringModule
FDD1516EFileReader::read(std::istream& stream, const std::string& encoding)
{
  // Set up the fdd parser
  SharedPtr<XML::XMLReader> reader;
  reader = new XML::ExpatXMLReader;

  SharedPtr<FDD1516EContentHandler> contentHandler = new FDD1516EContentHandler;
  reader->setContentHandler(contentHandler.get());
  SharedPtr<DefaultErrorHandler> errorHandler = new DefaultErrorHandler;
  reader->setErrorHandler(errorHandler.get());

  reader->parse(stream, encoding);

  std::string errorMessage = errorHandler->getMessages();
  if (!errorMessage.empty())
    throw ErrorReadingFDD(errorMessage);

  return contentHandler->_fomStringModuleBuilder.getFOMStringModule();
}

} // namespace OpenRTI
