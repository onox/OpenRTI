/* -*-c++-*- OpenRTI - Copyright (C) 2010-2012 Mathias Froehlich
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

#ifndef QRTI1516OMTHandler_h
#define QRTI1516OMTHandler_h

#include <Qt/QtCore>
#include <Qt/QtXml>

class QRTIFederate;

class QRTI1516OMTHandler : public QXmlContentHandler, public QXmlErrorHandler {
public:
  QRTI1516OMTHandler();
  virtual ~QRTI1516OMTHandler();

  virtual void setDocumentLocator(QXmlLocator*);
  virtual bool startDocument();
  virtual bool endDocument();
  virtual bool startPrefixMapping(const QString& prefix, const QString& uri);
  virtual bool endPrefixMapping(const QString& prefix);
  virtual bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts);
  virtual bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName);
  virtual bool characters(const QString&);
  virtual bool ignorableWhitespace(const QString&);
  virtual bool processingInstruction(const QString& target, const QString& data);
  virtual bool skippedEntity(const QString& name);

  virtual bool warning(const QXmlParseException& parseException);
  virtual bool error(const QXmlParseException& parseException);
  virtual bool fatalError(const QXmlParseException& parseException);
  virtual QString errorString() const;

  bool mergeToFederate(QRTIFederate& federate) const;

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

  Mode getCurrentMode();

  // Current modes in a stack
  QList<Mode> _modeStack;

  class Dimension {
  public:
    Dimension() :
      _upperBound(0)
    { }
    Dimension(const QString& name, unsigned long upperBound) :
      _name(name), _upperBound(upperBound)
    { }
    QString _name;
    unsigned long _upperBound;
  };
  typedef QList<Dimension> DimensionList;

  class Parameter {
  public:
    Parameter(const QString& name) :
      _name(name)
    { }
    QString _name;
    QString _dataType;
  };
  typedef QList<Parameter> ParameterList;

  class InteractionClass {
  public:
    InteractionClass() { }
    InteractionClass(const QString& name) :
      _name(name)
    { }
    QString _name;
    QString _parentName;
    QString _transportationType;
    QString _orderType;
    QStringList _dimensionList;
    ParameterList _parameterList;
  };
  typedef QList<InteractionClass> InteractionClassList;

  class Attribute {
  public:
    Attribute(const QString& name) :
      _name(name)
    { }
    QString _name;
    QString _transportationType;
    QString _orderType;
    QStringList _dimensionList;
    QString _dataType;
  };
  typedef QList<Attribute> AttributeList;

  class ObjectClass {
  public:
    ObjectClass() {}
    ObjectClass(const QString& name) : _name(name) {}
    QString _name;
    QString _parentName;
    AttributeList _attributeList;
  };
  typedef QList<ObjectClass> ObjectClassList;

  InteractionClass& getCurrentInteractionClass();
  ObjectClass& getCurrentObjectClass();

  typedef std::pair<QString, int> StringIntPair;

  QStringList _orderTypeList;
  QStringList _transportationTypeList;
  DimensionList _dimensionList;
  InteractionClassList _interactionClassList;
  ObjectClassList _objectClassList;

  typedef QList<StringIntPair> ObjectClassStack;
  ObjectClassStack _objectClassStack;

  typedef QList<StringIntPair> InteractionClassStack;
  InteractionClassStack _interactionClassStack;

  // The data types like the were found in the omt file
  struct BasicData {
    QString _name;
    QString _size;
    QString _endian;
    QString _interpretation;
    QString _encoding;
  };
  typedef QList<BasicData> BasicDataList;

  struct SimpleData {
    QString _name;
    QString _representation;
    QString _units;
    QString _resolution;
    QString _accuracy;
    QString _semantics;
  };
  typedef QList<SimpleData> SimpleDataList;

  struct Enumerator {
    QString _name;
    QString _values;
  };
  typedef QList<Enumerator> EnumeratorList;

  struct EnumeratedData {
    QString _name;
    QString _representation;
    QString _semantics;
    EnumeratorList _enumeratorList;
  };
  typedef QList<EnumeratedData> EnumeratedDataList;

  struct ArrayData {
    QString _name;
    QString _dataType;
    QString _cardinality;
    QString _encoding;
    QString _semantics;
  };
  typedef QList<ArrayData> ArrayDataList;

  struct Field {
    QString _name;
    QString _dataType;
    QString _semantics;
  };
  typedef QList<Field> FieldList;

  struct FixedRecordData {
    QString _name;
    QString _encoding;
    QString _semantics;
    FieldList _fieldList;
  };
  typedef QList<FixedRecordData> FixedRecordDataList;

  struct Alternative {
    QString _name;
    QString _dataType;
    QString _semantics;
    QString _enumerator;
  };
  typedef QList<Alternative> AlternativeList;

  struct VariantRecordData {
    QString _name;
    QString _encoding;
    QString _dataType;
    QString _semantics;
    AlternativeList _alternativeList;
  };
  typedef QList<VariantRecordData> VariantRecordDataList;

  BasicDataList _basicDataList;
  SimpleDataList _simpleDataList;
  EnumeratedDataList _enumeratedDataList;
  ArrayDataList _arrayDataList;
  FixedRecordDataList _fixedRecordDataList;
  VariantRecordDataList _variantRecordDataList;
};

#endif
