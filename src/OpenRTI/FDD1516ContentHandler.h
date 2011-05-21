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

#ifndef OpenRTI_FDD1516ContentHandler_h
#define OpenRTI_FDD1516ContentHandler_h

#include <vector>

#include "Attributes.h"
#include "ContentHandler.h"
#include "FOMStringModuleBuilder.h"

namespace OpenRTI {

// FIXME the rti1516e content is very different, implement that ...
class OPENRTI_LOCAL FDD1516ContentHandler : public XML::ContentHandler {
public:
  FDD1516ContentHandler();
  virtual ~FDD1516ContentHandler();

  virtual void startDocument(void);
  virtual void endDocument(void);
  virtual void startElement(const char* uri, const char* name,
                            const char* qName, const XML::Attributes* atts);
  virtual void endElement(const char* uri, const char* name, const char* qName);

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

}

#endif
