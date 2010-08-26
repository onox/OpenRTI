/* -*-c++-*- OpenRTI - Copyright (C) 2009-2010 Mathias Froehlich
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

#ifndef OpenRTI_FDDContentHandler_h
#define OpenRTI_FDDContentHandler_h

#include <iosfwd>
#include <sstream>
#include <cstring>
#include <vector>

#include "Attributes.h"
#include "ContentHandler.h"
#include "ExpatXMLReader.h"
#include "FOMStringModuleBuilder.h"
#include "Message.h"
#include "StringUtils.h"

namespace OpenRTI {

// FIXME the rti1516e content is very different, implement that ...
class OPENRTI_LOCAL FEDContentHandler : public XML::ContentHandler {
public:

  virtual void startDocument(void)
  {
    OpenRTIAssert(_modeStack.empty());
  }
  virtual void endDocument(void)
  {
    OpenRTIAssert(_modeStack.empty());
  }

  virtual void startElement(const char* uri, const char* name,
                            const char* qName, const XML::Attributes* atts)
  {
    if (strcmp(name, "attribute") == 0) {
      if (getCurrentMode() != ObjectClassMode)
        throw ErrorReadingFDD(L"attribute tag outside objectClass!");
      _modeStack.push_back(AttributeMode);

      std::wstring name = trim(utf8ToUcs(atts->getValue("name")));
      std::wstring order = trim(utf8ToUcs(atts->getValue("order")));
      std::wstring transportation = trim(utf8ToUcs(atts->getValue("transportation")));

      _fomStringModuleBuilder.addAttribute(name, order, transportation);

      std::vector<std::wstring> dimensionList = split(utf8ToUcs(atts->getValue("dimensions")), L", \t\n");
      for (std::vector<std::wstring>::const_iterator i = dimensionList.begin(); i != dimensionList.end(); ++i) {
        std::wstring trimmed = trim(*i);
        if (trimmed.empty())
          continue;
        if (trimmed == L"NA")
          continue;
        _fomStringModuleBuilder.addAttributeDimension(trimmed);
      }

    } else if (strcmp(name, "objectClass") == 0) {
      if (getCurrentMode() != ObjectsMode && getCurrentMode() != ObjectClassMode)
        throw ErrorReadingFDD(L"objectClass tag outside objectClass or objects!");
      _modeStack.push_back(ObjectClassMode);

      std::wstring name = trim(utf8ToUcs(atts->getValue("name")));
      _fomStringModuleBuilder.pushObjectClassData(name);

    } else if (strcmp(name, "objects") == 0) {
      if (getCurrentMode() != ObjectModelMode)
        throw ErrorReadingFDD(L"objects tag outside objectModel!");
      _modeStack.push_back(ObjectsMode);

    } else if (strcmp(name, "parameter") == 0) {
      if (getCurrentMode() != InteractionClassMode)
        throw ErrorReadingFDD(L"parameter tag outside interactionClass!");
      _modeStack.push_back(ParameterMode);

      std::wstring name = trim(utf8ToUcs(atts->getValue("name")));
      _fomStringModuleBuilder.addParameter(name);

    } else if (strcmp(name, "interactionClass") == 0) {
      if (getCurrentMode() != InteractionsMode && getCurrentMode() != InteractionClassMode)
        throw ErrorReadingFDD(L"interactionClass tag outside interactions or interactionClass!");
      _modeStack.push_back(InteractionClassMode);

      std::wstring name = trim(utf8ToUcs(atts->getValue("name")));
      std::wstring order = trim(utf8ToUcs(atts->getValue("order")));
      std::wstring transportation = trim(utf8ToUcs(atts->getValue("transportation")));
      _fomStringModuleBuilder.pushInteractionClassData(name, order, transportation);

      std::vector<std::wstring> dimensionList = split(utf8ToUcs(atts->getValue("dimensions")),  L", \t\n");
      for (std::vector<std::wstring>::const_iterator i = dimensionList.begin(); i != dimensionList.end(); ++i) {
        std::wstring trimmed = trim(*i);
        if (trimmed.empty())
          continue;
        if (trimmed == L"NA")
          continue;
        _fomStringModuleBuilder.addInteractionDimension(trimmed);
      }

    } else if (strcmp(name, "interactions") == 0) {
      if (getCurrentMode() != ObjectModelMode)
        throw ErrorReadingFDD(L"interactions tag outside objectModel!");
      _modeStack.push_back(InteractionsMode);

    } else if (strcmp(name, "dimensions") == 0) {
      if (getCurrentMode() != ObjectModelMode)
        throw ErrorReadingFDD(L"dimensions tag outside objectModel!");
      _modeStack.push_back(DimensionsMode);

    } else if (strcmp(name, "dimension") == 0) {
      if (getCurrentMode() != DimensionsMode)
        throw ErrorReadingFDD(L"dimension tag outside dimensions!");
      _modeStack.push_back(DimensionMode);

      std::wstring name = trim(utf8ToUcs(atts->getValue("name")));
      std::stringstream ss(atts->getValue("upperBound"));
      Unsigned upperBound = 0;
      ss >> upperBound;
      _fomStringModuleBuilder.addDimension(name, upperBound);

    } else if (strcmp(name, "transportation") == 0) {
      if (getCurrentMode() != TransportationsMode)
        throw ErrorReadingFDD(L"transportation tag outside transportations!");
      _modeStack.push_back(TransportationMode);

      std::wstring name = trim(utf8ToUcs(atts->getValue("name")));
      _fomStringModuleBuilder.addTransportationType(name);

    } else if (strcmp(name, "transportations") == 0) {
      if (getCurrentMode() != ObjectModelMode)
        throw ErrorReadingFDD(L"transportations tag outside objectModel!");
      _modeStack.push_back(TransportationsMode);

    } else if (strcmp(name, "basicData") == 0) {
      if (getCurrentMode() != BasicDataRepresentationsMode)
        throw ErrorReadingFDD(L"basicData tag outside basicDataRepresentations!");
      _modeStack.push_back(BasicDataMode);

    } else if (strcmp(name, "basicDataRepresentations") == 0) {
      if (getCurrentMode() != DataTypesMode)
        throw ErrorReadingFDD(L"basicDataRepresentations tag outside dataTypes!");
      _modeStack.push_back(BasicDataRepresentationsMode);

    } else if (strcmp(name, "simpleData") == 0) {
      if (getCurrentMode() != SimpleDataTypesMode)
        throw ErrorReadingFDD(L"simpleData tag outside simpleDataTypes!");
      _modeStack.push_back(SimpleDataMode);

    } else if (strcmp(name, "simpleDataTypes") == 0) {
      if (getCurrentMode() != DataTypesMode)
        throw ErrorReadingFDD(L"simpleDataTypes tag outside dataTypes!");
      _modeStack.push_back(SimpleDataTypesMode);

    } else if (strcmp(name, "enumerator") == 0) {
      if (getCurrentMode() != EnumeratedDataMode)
        throw ErrorReadingFDD(L"enumerator tag outside enumeratedData!");
      _modeStack.push_back(EnumeratorMode);

    } else if (strcmp(name, "enumeratedData") == 0) {
      if (getCurrentMode() != EnumeratedDataTypesMode)
        throw ErrorReadingFDD(L"enumeratedData tag outside enumeratedDataTypes!");
      _modeStack.push_back(EnumeratedDataMode);

    } else if (strcmp(name, "enumeratedDataTypes") == 0) {
      if (getCurrentMode() != DataTypesMode)
        throw ErrorReadingFDD(L"enumeratedDataTypes tag outside dataTypes!");
      _modeStack.push_back(EnumeratedDataTypesMode);

    } else if (strcmp(name, "arrayData") == 0) {
      if (getCurrentMode() != ArrayDataTypesMode)
        throw ErrorReadingFDD(L"arrayData tag outside arrayDataTypes!");
      _modeStack.push_back(ArrayDataMode);

    } else if (strcmp(name, "arrayDataTypes") == 0) {
      if (getCurrentMode() != DataTypesMode)
        throw ErrorReadingFDD(L"arrayDataTypes tag outside dataTypes!");
      _modeStack.push_back(ArrayDataTypesMode);

    } else if (strcmp(name, "field") == 0) {
      if (getCurrentMode() != FixedRecordDataMode)
        throw ErrorReadingFDD(L"field tag outside fixedRecordData!");
      _modeStack.push_back(FieldMode);

    } else if (strcmp(name, "fixedRecordData") == 0) {
      if (getCurrentMode() != FixedRecordDataTypesMode)
        throw ErrorReadingFDD(L"fixedRecordData tag outside fixedRecordDataTypes!");
      _modeStack.push_back(FixedRecordDataMode);

    } else if (strcmp(name, "fixedRecordDataTypes") == 0) {
      if (getCurrentMode() != DataTypesMode)
        throw ErrorReadingFDD(L"fixedRecordDataTypes tag outside dataTypes!");
      _modeStack.push_back(FixedRecordDataTypesMode);

    } else if (strcmp(name, "alternative") == 0) {
      if (getCurrentMode() != VariantRecordDataMode)
        throw ErrorReadingFDD(L"alternative tag outside variantRecordData!");
      _modeStack.push_back(AlternativeDataMode);

    } else if (strcmp(name, "variantRecordData") == 0) {
      if (getCurrentMode() != VariantRecordDataTypesMode)
        throw ErrorReadingFDD(L"variantRecordData tag outside variantRecordDataTypes!");
      _modeStack.push_back(VariantRecordDataMode);

    } else if (strcmp(name, "variantRecordDataTypes") == 0) {
      if (getCurrentMode() != DataTypesMode)
        throw ErrorReadingFDD(L"variantRecordDataTypes tag outside dataTypes!");
      _modeStack.push_back(VariantRecordDataTypesMode);

    } else if (strcmp(name, "dataTypes") == 0) {
      if (getCurrentMode() != ObjectModelMode)
        throw ErrorReadingFDD(L"dataTypes tag outside objectModel!");
      _modeStack.push_back(DataTypesMode);

    } else if (strcmp(name, "objectModel") == 0) {
      if (!_modeStack.empty())
        throw ErrorReadingFDD(L"objectModel tag not at top level!");
      _modeStack.push_back(ObjectModelMode);

    } else {
      _modeStack.push_back(UnknownMode);
    }
  }
  virtual void endElement(const char* uri, const char* name, const char* qName)
  {
    if (strcmp(name, "objectClass") == 0) {
      _fomStringModuleBuilder.popObjectClassData();
    } else if (strcmp(name, "interactionClass") == 0) {
      _fomStringModuleBuilder.popInteractionClassData();
    }
    _modeStack.pop_back();
  }

  const FOMStringModule& getFOMStringModule() const
  { return _fomStringModuleBuilder.getFOMStringModule(); }

private:
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

class FEDErrorHandler : public XML::ErrorHandler {
public:
  virtual void error(const char* msg, unsigned line, unsigned col)
  {
    _stream << "error: at line " << line << " in column " << col << ": \""
            << msg << "\"" << std::endl;
  }
  virtual void fatalError(const char* msg, unsigned line, unsigned col)
  {
    _stream << "fatalError: at line " << line << " in column " << col << ": \""
            << msg << "\"" << std::endl;
  }
  virtual void warning(const char* msg, unsigned line, unsigned col)
  {
  }

  std::wstring getMessages() const
  { return utf8ToUcs(_stream.str()); }

private:
  std::stringstream _stream;
};

}

#endif
