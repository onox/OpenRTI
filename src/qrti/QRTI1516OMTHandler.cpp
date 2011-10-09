/* -*-c++-*- OpenRTI - Copyright (C) 2010-2011 Mathias Froehlich
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

#include "QRTI1516OMTHandler.h"

#include "QRTIFederate.h"

static QStringList splitDimensions(const QString& s)
{
  QStringList splitList = s.split(", \t\n", QString::SkipEmptyParts);
  QStringList dimensionList;
  for (QStringList::const_iterator i = splitList.begin(); i != splitList.end(); ++i) {
    QString t = i->trimmed();
    if (t.isEmpty())
      continue;
    if (t == "NA")
      continue;
    dimensionList.push_back(t);
  }
  return dimensionList;
}

QRTI1516OMTHandler::QRTI1516OMTHandler()
{
}

QRTI1516OMTHandler::~QRTI1516OMTHandler()
{
}

void
QRTI1516OMTHandler::setDocumentLocator(QXmlLocator*)
{
}

bool
QRTI1516OMTHandler::startDocument()
{
  return _modeStack.isEmpty();
}

bool
QRTI1516OMTHandler::endDocument()
{
  return _modeStack.isEmpty();
}

bool
QRTI1516OMTHandler::startPrefixMapping(const QString& prefix, const QString& uri)
{
  return true;
}

bool
QRTI1516OMTHandler::endPrefixMapping(const QString& prefix)
{
  return true;
}

bool
QRTI1516OMTHandler::startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts)
{
  if (localName == "attribute") {
    if (getCurrentMode() != ObjectClassMode)
      // throw ErrorReadingFDD("attribute tag outside objectClass!");
      return false;
    _modeStack.push_back(AttributeMode);

    QString name = atts.value("name").trimmed();
    if (name.isEmpty())
      // throw ErrorReadingFDD("Empty parameter name is not allowed.");
      return false;

    getCurrentObjectClass()._attributeList.push_back(Attribute(name));
    getCurrentObjectClass()._attributeList.back()._orderType = atts.value("order").trimmed();
    getCurrentObjectClass()._attributeList.back()._transportationType = atts.value("transportation").trimmed();
    getCurrentObjectClass()._attributeList.back()._dimensionList = splitDimensions(atts.value("dimensions"));
    getCurrentObjectClass()._attributeList.back()._dataType = atts.value("dataType").trimmed();

  } else if (localName == "objectClass") {
    if (getCurrentMode() != ObjectsMode && getCurrentMode() != ObjectClassMode)
      // throw ErrorReadingFDD("objectClass tag outside objectClass or objects!");
      return false;
    _modeStack.push_back(ObjectClassMode);

    QString name = atts.value("name").trimmed();
    if (name.isEmpty())
      // throw ErrorReadingFDD("Empty object class name is not allowed.");
      return false;

    _objectClassList.push_back(ObjectClass(name));
    if (!_objectClassStack.isEmpty())
      _objectClassList.back()._parentName = _objectClassStack.back().first;
    _objectClassStack.push_back(StringIntPair(name, _objectClassList.size() - 1));

  } else if (localName == "objects") {
    if (getCurrentMode() != ObjectModelMode)
      // throw ErrorReadingFDD("objects tag outside objectModel!");
      return false;
    _modeStack.push_back(ObjectsMode);

  } else if (localName == "parameter") {
    if (getCurrentMode() != InteractionClassMode)
      // throw ErrorReadingFDD("parameter tag outside interactionClass!");
      return false;
    _modeStack.push_back(ParameterMode);

    QString name = atts.value("name").trimmed();
    if (name.isEmpty())
      // throw ErrorReadingFDD("Empty parameter name is not allowed.");
      return false;

    getCurrentInteractionClass()._parameterList.push_back(Parameter(name));
    getCurrentInteractionClass()._parameterList.back()._dataType = atts.value("dataType").trimmed();

  } else if (localName == "interactionClass") {
    if (getCurrentMode() != InteractionsMode && getCurrentMode() != InteractionClassMode)
      // throw ErrorReadingFDD("interactionClass tag outside interactions or interactionClass!");
      return false;
    _modeStack.push_back(InteractionClassMode);

    QString name = atts.value("name").trimmed();
    if (name.isEmpty())
      // throw ErrorReadingFDD("Empty interaction class name is not allowed.");
      return false;

    _interactionClassList.push_back(InteractionClass(name));
    if (!_interactionClassStack.isEmpty())
      _interactionClassList.back()._parentName = _interactionClassStack.back().first;
    _interactionClassStack.push_back(StringIntPair(name, _interactionClassList.size() - 1));
    getCurrentInteractionClass()._orderType = atts.value("order").trimmed();
    getCurrentInteractionClass()._transportationType = atts.value("transportation").trimmed();
    getCurrentInteractionClass()._dimensionList = splitDimensions(atts.value("dimensions"));

  } else if (localName == "interactions") {
    if (getCurrentMode() != ObjectModelMode)
      // throw ErrorReadingFDD("interactions tag outside objectModel!");
      return false;
    _modeStack.push_back(InteractionsMode);

  } else if (localName == "dimensions") {
    if (getCurrentMode() != ObjectModelMode)
      // throw ErrorReadingFDD("dimensions tag outside objectModel!");
      return false;
    _modeStack.push_back(DimensionsMode);

  } else if (localName == "dimension") {
    if (getCurrentMode() != DimensionsMode)
      // throw ErrorReadingFDD("dimension tag outside dimensions!");
      return false;
    _modeStack.push_back(DimensionMode);

    QString name = atts.value("name").trimmed();
    unsigned long upperBound = atts.value("upperBound").toULong();
    _dimensionList.push_back(Dimension(name, upperBound));

  } else if (localName == "transportation") {
    if (getCurrentMode() != TransportationsMode)
      // throw ErrorReadingFDD("transportation tag outside transportations!");
      return false;
    _modeStack.push_back(TransportationMode);
    _transportationTypeList.push_back(atts.value("name").trimmed());

  } else if (localName == "transportations") {
    if (getCurrentMode() != ObjectModelMode)
      // throw ErrorReadingFDD("transportations tag outside objectModel!");
      return false;
    _modeStack.push_back(TransportationsMode);

  } else if (localName == "basicData") {
    if (getCurrentMode() != BasicDataRepresentationsMode)
      // throw ErrorReadingFDD("basicData tag outside basicDataRepresentations!");
      return false;
    _modeStack.push_back(BasicDataMode);

    BasicDataList::iterator i = _basicDataList.insert(_basicDataList.end(), BasicData());
    i->_name = atts.value("name").trimmed();
    i->_size = atts.value("size").trimmed();
    i->_endian = atts.value("endian").trimmed();
    i->_interpretation = atts.value("interpretation");
    i->_encoding = atts.value("encoding");

  } else if (localName == "basicDataRepresentations") {
    if (getCurrentMode() != DataTypesMode)
      // throw ErrorReadingFDD("basicDataRepresentations tag outside dataTypes!");
      return false;
    _modeStack.push_back(BasicDataRepresentationsMode);

  } else if (localName == "simpleData") {
    if (getCurrentMode() != SimpleDataTypesMode)
      // throw ErrorReadingFDD("simpleData tag outside simpleDataTypes!");
      return false;
    _modeStack.push_back(SimpleDataMode);

    SimpleDataList::iterator i = _simpleDataList.insert(_simpleDataList.end(), SimpleData());
    i->_name = atts.value("name").trimmed();
    i->_representation = atts.value("representation").trimmed();
    i->_units = atts.value("units").trimmed();
    i->_resolution = atts.value("resolution").trimmed();
    i->_accuracy = atts.value("accuracy").trimmed();
    i->_semantics = atts.value("semantics");

  } else if (localName == "simpleDataTypes") {
    if (getCurrentMode() != DataTypesMode)
      // throw ErrorReadingFDD("simpleDataTypes tag outside dataTypes!");
      return false;
    _modeStack.push_back(SimpleDataTypesMode);

  } else if (localName == "enumerator") {
    if (getCurrentMode() != EnumeratedDataMode)
      // throw ErrorReadingFDD("enumerator tag outside enumeratedData!");
      return false;
    _modeStack.push_back(EnumeratorMode);
    if (_enumeratedDataList.isEmpty())
      return false;

    EnumeratedData& currentEnumeratedData = _enumeratedDataList.back();
    EnumeratorList::iterator i;
    i = currentEnumeratedData._enumeratorList.insert(currentEnumeratedData._enumeratorList.end(), Enumerator());
    i->_name = atts.value("name").trimmed();
    i->_values = atts.value("values");

  } else if (localName == "enumeratedData") {
    if (getCurrentMode() != EnumeratedDataTypesMode)
      // throw ErrorReadingFDD("enumeratedData tag outside enumeratedDataTypes!");
      return false;
    _modeStack.push_back(EnumeratedDataMode);

    EnumeratedDataList::iterator i;
    i = _enumeratedDataList.insert(_enumeratedDataList.end(), EnumeratedData());
    i->_name = atts.value("name").trimmed();
    i->_representation = atts.value("representation").trimmed();
    i->_semantics = atts.value("semantics");

  } else if (localName == "enumeratedDataTypes") {
    if (getCurrentMode() != DataTypesMode)
      // throw ErrorReadingFDD("enumeratedDataTypes tag outside dataTypes!");
      return false;
    _modeStack.push_back(EnumeratedDataTypesMode);

  } else if (localName == "arrayData") {
    if (getCurrentMode() != ArrayDataTypesMode)
      // throw ErrorReadingFDD("arrayData tag outside arrayDataTypes!");
      return false;
    _modeStack.push_back(ArrayDataMode);

    ArrayDataList::iterator i = _arrayDataList.insert(_arrayDataList.end(), ArrayData());
    i->_name = atts.value("name").trimmed();
    i->_dataType = atts.value("dataType").trimmed();
    i->_cardinality = atts.value("cardinality").trimmed();
    i->_encoding = atts.value("encoding").trimmed();
    i->_semantics = atts.value("semantics");

  } else if (localName == "arrayDataTypes") {
    if (getCurrentMode() != DataTypesMode)
      // throw ErrorReadingFDD("arrayDataTypes tag outside dataTypes!");
      return false;
    _modeStack.push_back(ArrayDataTypesMode);

  } else if (localName == "field") {
    if (getCurrentMode() != FixedRecordDataMode)
      // throw ErrorReadingFDD("field tag outside fixedRecordData!");
      return false;
    _modeStack.push_back(FieldMode);
    if (_fixedRecordDataList.isEmpty())
      return false;

    FixedRecordData& currentFixedRecordData = _fixedRecordDataList.back();
    FieldList::iterator i;
    i = currentFixedRecordData._fieldList.insert(currentFixedRecordData._fieldList.end(), Field());
    i->_name = atts.value("name").trimmed();
    i->_dataType = atts.value("dataType").trimmed();
    i->_semantics = atts.value("semantics");

  } else if (localName == "fixedRecordData") {
    if (getCurrentMode() != FixedRecordDataTypesMode)
      // throw ErrorReadingFDD("fixedRecordData tag outside fixedRecordDataTypes!");
      return false;
    _modeStack.push_back(FixedRecordDataMode);

    FixedRecordDataList::iterator i;
    i = _fixedRecordDataList.insert(_fixedRecordDataList.end(), FixedRecordData());
    i->_name = atts.value("name").trimmed();
    i->_encoding = atts.value("encoding").trimmed();
    i->_semantics = atts.value("semantics");

  } else if (localName == "fixedRecordDataTypes") {
    if (getCurrentMode() != DataTypesMode)
      // throw ErrorReadingFDD("fixedRecordDataTypes tag outside dataTypes!");
      return false;
    _modeStack.push_back(FixedRecordDataTypesMode);

  } else if (localName == "alternative") {
    if (getCurrentMode() != VariantRecordDataMode)
      // throw ErrorReadingFDD("alternative tag outside variantRecordData!");
      return false;
    _modeStack.push_back(AlternativeDataMode);
    if (_variantRecordDataList.isEmpty())
      return false;

    VariantRecordData& currentVariantRecordData = _variantRecordDataList.back();
    AlternativeList::iterator i;
    i = currentVariantRecordData._alternativeList.insert(currentVariantRecordData._alternativeList.end(), Alternative());
    i->_name = atts.value("name").trimmed();
    i->_dataType = atts.value("dataType").trimmed();
    i->_semantics = atts.value("semantics");
    i->_enumerator = atts.value("enumerator").trimmed();

  } else if (localName == "variantRecordData") {
    if (getCurrentMode() != VariantRecordDataTypesMode)
      // throw ErrorReadingFDD("variantRecordData tag outside variantRecordDataTypes!");
      return false;
    _modeStack.push_back(VariantRecordDataMode);

    VariantRecordDataList::iterator i;
    i = _variantRecordDataList.insert(_variantRecordDataList.end(), VariantRecordData());
    i->_name = atts.value("name").trimmed();
    i->_encoding = atts.value("encoding").trimmed();
    i->_dataType = atts.value("dataType").trimmed();
    i->_semantics = atts.value("semantics");

  } else if (localName == "variantRecordDataTypes") {
    if (getCurrentMode() != DataTypesMode)
      // throw ErrorReadingFDD("variantRecordDataTypes tag outside dataTypes!");
      return false;
    _modeStack.push_back(VariantRecordDataTypesMode);

  } else if (localName == "dataTypes") {
    if (getCurrentMode() != ObjectModelMode)
      // throw ErrorReadingFDD("dataTypes tag outside objectModel!");
      return false;
    _modeStack.push_back(DataTypesMode);

  } else if (localName == "objectModel") {
    if (!_modeStack.isEmpty())
      // throw ErrorReadingFDD("objectModel tag not at top level!");
      return false;
    _modeStack.push_back(ObjectModelMode);

  } else {
    _modeStack.push_back(UnknownMode);
  }
  return true;
}

bool
QRTI1516OMTHandler::endElement(const QString& namespaceURI, const QString& localName, const QString& qName)
{
  if (localName == "objectClass") {
    _objectClassStack.pop_back();
  } else if (localName == "interactionClass") {
    _interactionClassStack.pop_back();
  }
  _modeStack.pop_back();
  return true;
}

bool
QRTI1516OMTHandler::characters(const QString&)
{
  return true;
}

bool
QRTI1516OMTHandler::ignorableWhitespace(const QString&)
{
  return true;
}

bool
QRTI1516OMTHandler::processingInstruction(const QString& target, const QString& data)
{
  return true;
}

bool
QRTI1516OMTHandler::skippedEntity(const QString& name)
{
  return true;
}

bool
QRTI1516OMTHandler::warning(const QXmlParseException& parseException)
{
  return true;
}

bool
QRTI1516OMTHandler::error(const QXmlParseException& parseException)
{
  return true;
}

bool
QRTI1516OMTHandler::fatalError(const QXmlParseException& parseException)
{
  return true;
}

QString
QRTI1516OMTHandler::errorString() const
{
  return QString();
}

bool
QRTI1516OMTHandler::mergeToFederate(QRTIFederate& federate) const
{
  // Data types
  for (int i = 0; i < _basicDataList.size(); ++i) {
    if (federate.getDataType(_basicDataList[i]._name))
      /// Hmm may at some time compare and report conflicts
      continue;
    QHLABasicDataType* basicDataType = federate.createBasicDataType();
    basicDataType->setName(_basicDataList[i]._name);
    basicDataType->setSize(_basicDataList[i]._size.toUInt());
    basicDataType->setInterpretation(_basicDataList[i]._interpretation);
    if (_basicDataList[i]._endian == "Little")
      basicDataType->setEndian(QHLABasicDataType::LittleEndian);
    else
      basicDataType->setEndian(QHLABasicDataType::BigEndian);
    basicDataType->setEncoding(_basicDataList[i]._encoding);
    if (_basicDataList[i]._name.contains(QString("float")))
      basicDataType->setType(QHLABasicDataType::FloatType);
    else if (_basicDataList[i]._name.contains(QString("Float")))
      basicDataType->setType(QHLABasicDataType::FloatType);
    else if (_basicDataList[i]._name.contains(QString("unsigned")))
      basicDataType->setType(QHLABasicDataType::UnsignedType);
    else if (_basicDataList[i]._name.contains(QString("Unsigned")))
      basicDataType->setType(QHLABasicDataType::UnsignedType);
    else
      basicDataType->setType(QHLABasicDataType::SignedType);
  }
  for (int i = 0; i < _simpleDataList.size(); ++i) {
    if (federate.getDataType(_simpleDataList[i]._name))
      /// Hmm may at some time compare and report conflicts
      continue;
    QHLASimpleDataType* simpleDataType = federate.createSimpleDataType();
    simpleDataType->setName(_simpleDataList[i]._name);
    simpleDataType->setRepresentation(federate.getBasicDataType(_simpleDataList[i]._representation));
    simpleDataType->setUnits(_simpleDataList[i]._units);
    simpleDataType->setResolution(_simpleDataList[i]._resolution);
    simpleDataType->setAccuracy(_simpleDataList[i]._accuracy);
    simpleDataType->setSemantics(_simpleDataList[i]._semantics);
  }
  for (int i = 0; i < _enumeratedDataList.size(); ++i) {
    if (federate.getDataType(_enumeratedDataList[i]._name))
      /// Hmm may at some time compare and report conflicts
      continue;
    QHLAEnumeratedDataType* enumeratedDataType = federate.createEnumeratedDataType();
    enumeratedDataType->setName(_enumeratedDataList[i]._name);
    enumeratedDataType->setRepresentation(federate.getBasicDataType(_enumeratedDataList[i]._representation));
    enumeratedDataType->setSemantics(_enumeratedDataList[i]._semantics);
    for (int j = 0; j < _enumeratedDataList[i]._enumeratorList.size(); ++j) {
      QHLAEnumerator* enumerator = enumeratedDataType->createEnumerator();
      enumerator->setName(_enumeratedDataList[i]._enumeratorList[j]._name);
      enumerator->setValues(_enumeratedDataList[i]._enumeratorList[j]._values);
    }
  }
  for (int i = 0; i < _arrayDataList.size(); ++i) {
    if (federate.getDataType(_arrayDataList[i]._name))
      /// Hmm may at some time compare and report conflicts
      continue;
    QHLAArrayDataType* arrayDataType = federate.createArrayDataType();
    arrayDataType->setName(_arrayDataList[i]._name);
    arrayDataType->setCardinality(_arrayDataList[i]._cardinality);
    arrayDataType->setEncoding(_arrayDataList[i]._encoding);
    arrayDataType->setSemantics(_arrayDataList[i]._semantics);
    if (_arrayDataList[i]._name == "HLAASCIIstring")
      arrayDataType->setIsString(true);
    else if (_arrayDataList[i]._name == "HLAunicodeString")
      arrayDataType->setIsString(true);
    else if (_arrayDataList[i]._name == "HLAopaqueData")
      arrayDataType->setIsOpaqueData(true);
  }
  for (int i = 0; i < _fixedRecordDataList.size(); ++i) {
    if (federate.getDataType(_fixedRecordDataList[i]._name))
      /// Hmm may at some time compare and report conflicts
      continue;
    QHLAFixedRecordDataType* fixedRecordDataType = federate.createFixedRecordDataType();
    fixedRecordDataType->setName(_fixedRecordDataList[i]._name);
    fixedRecordDataType->setEncoding(_fixedRecordDataList[i]._encoding);
    fixedRecordDataType->setSemantics(_fixedRecordDataList[i]._semantics);
    for (int j = 0; j < _fixedRecordDataList[i]._fieldList.size(); ++j) {
      QHLAField* field = fixedRecordDataType->createField();
      field->setName(_fixedRecordDataList[i]._fieldList[j]._name);
      field->setSemantics(_fixedRecordDataList[i]._fieldList[j]._semantics);
    }
  }
  for (int i = 0; i < _variantRecordDataList.size(); ++i) {
    if (federate.getDataType(_variantRecordDataList[i]._name))
      /// Hmm may at some time compare and report conflicts
      continue;
    QHLAVariantRecordDataType* variantRecordDataType = federate.createVariantRecordDataType();
    variantRecordDataType->setName(_variantRecordDataList[i]._name);
    variantRecordDataType->setEncoding(_variantRecordDataList[i]._encoding);
    variantRecordDataType->setType(federate.getEnumeratedDataType(_variantRecordDataList[i]._dataType));
    variantRecordDataType->setSemantics(_variantRecordDataList[i]._semantics);
    for (int j = 0; j < _variantRecordDataList[i]._alternativeList.size(); ++j) {
      QHLAAlternative* alternaitve = variantRecordDataType->createAlternative();
      alternaitve->setName(_variantRecordDataList[i]._alternativeList[j]._name);
      alternaitve->setSemantics(_variantRecordDataList[i]._alternativeList[j]._semantics);
      alternaitve->setEnumerator(_variantRecordDataList[i]._alternativeList[j]._enumerator);
    }
  }

  // now again traverse these to resolve the may be cyclic data type references
  for (int i = 0; i < _arrayDataList.size(); ++i) {
    QHLAArrayDataType* arrayDataType = federate.getArrayDataType(_arrayDataList[i]._name);
    arrayDataType->setElementType(federate.getDataType(_arrayDataList[i]._dataType));
  }
  for (int i = 0; i < _fixedRecordDataList.size(); ++i) {
    QHLAFixedRecordDataType* fixedRecordDataType = federate.getFixedRecordDataType(_fixedRecordDataList[i]._name);
    for (int j = 0; j < _fixedRecordDataList[i]._fieldList.size(); ++j) {
      QHLAField* field = fixedRecordDataType->getField(j);
      field->setType(federate.getDataType(_fixedRecordDataList[i]._fieldList[j]._dataType));
    }
  }
  for (int i = 0; i < _variantRecordDataList.size(); ++i) {
    QHLAVariantRecordDataType* variantRecordDataType = federate.getVariantRecordDataType(_variantRecordDataList[i]._name);
    for (int j = 0; j < _variantRecordDataList[i]._alternativeList.size(); ++j) {
      QHLAAlternative* alternative = variantRecordDataType->getAlternative(j);
      alternative->setType(federate.getDataType(_variantRecordDataList[i]._alternativeList[j]._dataType));
    }
  }

  // The object model
  for (int i = 0; i < _orderTypeList.size(); ++i) {
  }
  for (int i = 0; i < _transportationTypeList.size(); ++i) {
  }
  for (int i = 0; i < _dimensionList.size(); ++i) {
  }

  // Interaction classes
  for (int i = 0; i < _interactionClassList.size(); ++i) {
    for (int i = 0; i < _interactionClassList[i]._parameterList.size(); ++i) {
    }
  }

  // Object classes
  for (int i = 0; i < _objectClassList.size(); ++i) {
    QRTIObjectClass* parentObjectClass = federate.getObjectClass(_objectClassList[i]._parentName);
    QRTIObjectClass* objectClass = federate._createObjectClass(_objectClassList[i]._name, parentObjectClass);
    if (!objectClass)
      continue;

    for (int j = 0; j < _objectClassList[i]._attributeList.size(); ++j) {
      QString attributeName = _objectClassList[i]._attributeList[j]._name;
      QRTIObjectClassAttribute* objectClassAttribute = objectClass->insertObjectClassAttribute(attributeName);
      objectClassAttribute->setDataType(federate.getDataType(_objectClassList[i]._attributeList[j]._dataType));
    }
    /// FIXME do this on demand!!!
    objectClass->subscribe();
  }

  return true;
}

QRTI1516OMTHandler::Mode
QRTI1516OMTHandler::getCurrentMode()
{
  if (_modeStack.isEmpty())
    return UnknownMode;
  return _modeStack.back();
}

QRTI1516OMTHandler::InteractionClass&
QRTI1516OMTHandler::getCurrentInteractionClass()
{
  return _interactionClassList[_interactionClassStack.back().second];
}

QRTI1516OMTHandler::ObjectClass&
QRTI1516OMTHandler::getCurrentObjectClass()
{
  return _objectClassList[_objectClassStack.back().second];
}
