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

#include "FDD1516FileReader.h"

#include "DefaultErrorHandler.h"
#include "Exception.h"
#include "ExpatXMLReader.h"
#include "FOMStringModuleBuilder.h"
#include "Message.h"
#include "StringUtils.h"

namespace OpenRTI {

class OPENRTI_LOCAL FDD1516ContentHandler : public XML::ContentHandler {
public:
  FDD1516ContentHandler();
  virtual ~FDD1516ContentHandler();

  virtual void startDocument(void);
  virtual void endDocument(void);
  virtual void startElement(const char* uri, const char* name,
                            const char* qName, const XML::Attributes* atts);
  virtual void endElement(const char* uri, const char* name, const char* qName);

  // poor man's schema checking ...
  enum Mode {
    UnknownMode,

    ObjectModelMode,

    ObjectsMode,
    ObjectClassMode,
    AttributeMode,

    InteractionsMode,
    InteractionClassMode,
    ParameterMode,

    DimensionsMode,
    DimensionMode,

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

    TransportationsMode,
    TransportationMode,

    // SwitchesMode,

    DataTypesMode,
    BasicDataRepresentationsMode,
    BasicDataMode,
    SimpleDataTypesMode,
    SimpleDataMode,
    EnumeratedDataTypesMode,
    EnumeratedDataMode,
    EnumeratorMode,
    ArrayDataTypesMode,
    ArrayDataMode,
    FixedRecordDataTypesMode,
    FixedRecordDataMode,
    FieldMode,
    VariantRecordDataTypesMode,
    VariantRecordDataMode,
    AlternativeDataMode// ,

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

  // Helper to build up the data
  FOMStringModuleBuilder _fomStringModuleBuilder;
};

FDD1516ContentHandler::FDD1516ContentHandler()
{
}

FDD1516ContentHandler::~FDD1516ContentHandler()
{
}

void
FDD1516ContentHandler::startDocument(void)
{
  OpenRTIAssert(_modeStack.empty());
}

void
FDD1516ContentHandler::endDocument(void)
{
  OpenRTIAssert(_modeStack.empty());
  _fomStringModuleBuilder.validate();
}

void
FDD1516ContentHandler::startElement(const char* uri, const char* name,
                                    const char* qName, const XML::Attributes* atts)
{
  if (strcmp(name, "attribute") == 0) {
    if (getCurrentMode() != ObjectClassMode)
      throw ErrorReadingFDD("attribute tag outside objectClass!");
    _modeStack.push_back(AttributeMode);

    _fomStringModuleBuilder.addAttribute();

    std::string name = trim(atts->getValue("name"));
    _fomStringModuleBuilder.getCurrentObjectClassAttribute().setName(name);
    std::string order = trim(atts->getValue("order"));
    _fomStringModuleBuilder.getCurrentObjectClassAttribute().setOrderType(order);
    std::string transportation = trim(atts->getValue("transportation"));
    _fomStringModuleBuilder.getCurrentObjectClassAttribute().setTransportationType(transportation);

    std::vector<std::string> dimensionList = split(atts->getValue("dimensions"), ", \t\n");
    for (std::vector<std::string>::const_iterator i = dimensionList.begin(); i != dimensionList.end(); ++i) {
      std::string trimmed = trim(*i);
      if (trimmed.empty())
        continue;
      if (trimmed == "NA")
        continue;
      _fomStringModuleBuilder.addAttributeDimension(trimmed);
    }

  } else if (strcmp(name, "objectClass") == 0) {
    if (getCurrentMode() != ObjectsMode && getCurrentMode() != ObjectClassMode)
      throw ErrorReadingFDD("objectClass tag outside objectClass or objects!");
    _modeStack.push_back(ObjectClassMode);

    _fomStringModuleBuilder.pushObjectClass();

    std::string name = classNamePart(trim(atts->getValue("name")));
    _fomStringModuleBuilder.getCurrentObjectClass().getName().push_back(name);

  } else if (strcmp(name, "objects") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("objects tag outside objectModel!");
    _modeStack.push_back(ObjectsMode);

  } else if (strcmp(name, "parameter") == 0) {
    if (getCurrentMode() != InteractionClassMode)
      throw ErrorReadingFDD("parameter tag outside interactionClass!");
    _modeStack.push_back(ParameterMode);

    _fomStringModuleBuilder.addParameter();

    std::string name = trim(atts->getValue("name"));
    _fomStringModuleBuilder.getCurrentInteractionClassParameter().setName(name);

  } else if (strcmp(name, "interactionClass") == 0) {
    if (getCurrentMode() != InteractionsMode && getCurrentMode() != InteractionClassMode)
      throw ErrorReadingFDD("interactionClass tag outside interactions or interactionClass!");
    _modeStack.push_back(InteractionClassMode);

    _fomStringModuleBuilder.pushInteractionClass();

    std::string name = classNamePart(trim(atts->getValue("name")));
    _fomStringModuleBuilder.getCurrentInteractionClass().getName().push_back(name);
    std::string order = trim(atts->getValue("order"));
    _fomStringModuleBuilder.getCurrentInteractionClass().setOrderType(order);
    std::string transportation = trim(atts->getValue("transportation"));
    _fomStringModuleBuilder.getCurrentInteractionClass().setTransportationType(transportation);

    std::vector<std::string> dimensionList = split(atts->getValue("dimensions"),  ", \t\n");
    for (std::vector<std::string>::const_iterator i = dimensionList.begin(); i != dimensionList.end(); ++i) {
      std::string trimmed = trim(*i);
      if (trimmed.empty())
        continue;
      if (trimmed == "NA")
        continue;
      _fomStringModuleBuilder.addInteractionDimension(trimmed);
    }

  } else if (strcmp(name, "interactions") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("interactions tag outside objectModel!");
    _modeStack.push_back(InteractionsMode);

  } else if (strcmp(name, "dimensions") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("dimensions tag outside objectModel!");
    _modeStack.push_back(DimensionsMode);

  } else if (strcmp(name, "dimension") == 0) {
    if (getCurrentMode() != DimensionsMode)
      throw ErrorReadingFDD("dimension tag outside dimensions!");
    _modeStack.push_back(DimensionMode);

    _fomStringModuleBuilder.addDimension();

    std::string name = trim(atts->getValue("name"));
    _fomStringModuleBuilder.getCurrentDimension().setName(name);

    std::stringstream ss(atts->getValue("upperBound"));
    Unsigned upperBound = 0;
    ss >> upperBound;
    _fomStringModuleBuilder.getCurrentDimension().setUpperBound(upperBound);

  } else if (strcmp(name, "transportation") == 0) {
    if (getCurrentMode() != TransportationsMode)
      throw ErrorReadingFDD("transportation tag outside transportations!");
    _modeStack.push_back(TransportationMode);

    _fomStringModuleBuilder.addTransportationType();

    std::string name = trim(atts->getValue("name"));
    _fomStringModuleBuilder.getCurrentTransportationType().setName(name);

  } else if (strcmp(name, "transportations") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("transportations tag outside objectModel!");
    _modeStack.push_back(TransportationsMode);

  } else if (strcmp(name, "basicData") == 0) {
    if (getCurrentMode() != BasicDataRepresentationsMode)
      throw ErrorReadingFDD("basicData tag outside basicDataRepresentations!");
    _modeStack.push_back(BasicDataMode);

  } else if (strcmp(name, "basicDataRepresentations") == 0) {
    if (getCurrentMode() != DataTypesMode)
      throw ErrorReadingFDD("basicDataRepresentations tag outside dataTypes!");
    _modeStack.push_back(BasicDataRepresentationsMode);

  } else if (strcmp(name, "simpleData") == 0) {
    if (getCurrentMode() != SimpleDataTypesMode)
      throw ErrorReadingFDD("simpleData tag outside simpleDataTypes!");
    _modeStack.push_back(SimpleDataMode);

  } else if (strcmp(name, "simpleDataTypes") == 0) {
    if (getCurrentMode() != DataTypesMode)
      throw ErrorReadingFDD("simpleDataTypes tag outside dataTypes!");
    _modeStack.push_back(SimpleDataTypesMode);

  } else if (strcmp(name, "enumerator") == 0) {
    if (getCurrentMode() != EnumeratedDataMode)
      throw ErrorReadingFDD("enumerator tag outside enumeratedData!");
    _modeStack.push_back(EnumeratorMode);

  } else if (strcmp(name, "enumeratedData") == 0) {
    if (getCurrentMode() != EnumeratedDataTypesMode)
      throw ErrorReadingFDD("enumeratedData tag outside enumeratedDataTypes!");
    _modeStack.push_back(EnumeratedDataMode);

  } else if (strcmp(name, "enumeratedDataTypes") == 0) {
    if (getCurrentMode() != DataTypesMode)
      throw ErrorReadingFDD("enumeratedDataTypes tag outside dataTypes!");
    _modeStack.push_back(EnumeratedDataTypesMode);

  } else if (strcmp(name, "arrayData") == 0) {
    if (getCurrentMode() != ArrayDataTypesMode)
      throw ErrorReadingFDD("arrayData tag outside arrayDataTypes!");
    _modeStack.push_back(ArrayDataMode);

  } else if (strcmp(name, "arrayDataTypes") == 0) {
    if (getCurrentMode() != DataTypesMode)
      throw ErrorReadingFDD("arrayDataTypes tag outside dataTypes!");
    _modeStack.push_back(ArrayDataTypesMode);

  } else if (strcmp(name, "field") == 0) {
    if (getCurrentMode() != FixedRecordDataMode)
      throw ErrorReadingFDD("field tag outside fixedRecordData!");
    _modeStack.push_back(FieldMode);

  } else if (strcmp(name, "fixedRecordData") == 0) {
    if (getCurrentMode() != FixedRecordDataTypesMode)
      throw ErrorReadingFDD("fixedRecordData tag outside fixedRecordDataTypes!");
    _modeStack.push_back(FixedRecordDataMode);

  } else if (strcmp(name, "fixedRecordDataTypes") == 0) {
    if (getCurrentMode() != DataTypesMode)
      throw ErrorReadingFDD("fixedRecordDataTypes tag outside dataTypes!");
    _modeStack.push_back(FixedRecordDataTypesMode);

  } else if (strcmp(name, "alternative") == 0) {
    if (getCurrentMode() != VariantRecordDataMode)
      throw ErrorReadingFDD("alternative tag outside variantRecordData!");
    _modeStack.push_back(AlternativeDataMode);

  } else if (strcmp(name, "variantRecordData") == 0) {
    if (getCurrentMode() != VariantRecordDataTypesMode)
      throw ErrorReadingFDD("variantRecordData tag outside variantRecordDataTypes!");
    _modeStack.push_back(VariantRecordDataMode);

  } else if (strcmp(name, "variantRecordDataTypes") == 0) {
    if (getCurrentMode() != DataTypesMode)
      throw ErrorReadingFDD("variantRecordDataTypes tag outside dataTypes!");
    _modeStack.push_back(VariantRecordDataTypesMode);

  } else if (strcmp(name, "dataTypes") == 0) {
    if (getCurrentMode() != ObjectModelMode)
      throw ErrorReadingFDD("dataTypes tag outside objectModel!");
    _modeStack.push_back(DataTypesMode);

  } else if (strcmp(name, "objectModel") == 0) {
    if (!_modeStack.empty())
      throw ErrorReadingFDD("objectModel tag not at top level!");
    _modeStack.push_back(ObjectModelMode);

  } else {
    _modeStack.push_back(UnknownMode);
  }
}

void
FDD1516ContentHandler::endElement(const char* uri, const char* name, const char* qName)
{
  if (strcmp(name, "objectClass") == 0) {
    _fomStringModuleBuilder.popObjectClass();
  } else if (strcmp(name, "interactionClass") == 0) {
    _fomStringModuleBuilder.popInteractionClass();
  }
  _modeStack.pop_back();
}

FOMStringModule
FDD1516FileReader::read(std::istream& stream, const std::string& encoding)
{
  // Set up the fdd parser
  SharedPtr<XML::XMLReader> reader;
  reader = new XML::ExpatXMLReader;

  SharedPtr<FDD1516ContentHandler> contentHandler = new FDD1516ContentHandler;
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
