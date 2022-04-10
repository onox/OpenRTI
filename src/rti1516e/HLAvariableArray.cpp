/* -*-c++-*- OpenRTI - Copyright (C) 2011-2022 Mathias Froehlich
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

#include <RTI/encoding/HLAvariableArray.h>

#include <RTI/VariableLengthData.h>
#include <RTI/encoding/DataElement.h>

#include <algorithm>
#include <limits>
#include <cstring>
#include <vector>

#include "Encoding.h"
#include "Export.h"
#include "Types.h"

namespace rti1516e
{

class OPENRTI_LOCAL HLAvariableArrayImplementation {
public:
  HLAvariableArrayImplementation(const DataElement& protoType) :
    _protoType(protoType.clone().release())
  {
  }
  HLAvariableArrayImplementation(const HLAvariableArrayImplementation& rhs) :
    _protoType(rhs._protoType->clone().release())
  {
    _dataElementVector.resize(rhs._dataElementVector.size(), 0);
    for (size_t i = 0; i < rhs._dataElementVector.size(); ++i) {
      if (!rhs._dataElementVector[i])
        continue;
      _dataElementVector[i] = rhs._dataElementVector[i]->clone().release();
    }
  }
  virtual ~HLAvariableArrayImplementation()
  {
    for (DataElementVector::iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      delete *i;
      *i = 0;
    }
    delete _protoType;
    _protoType = 0;
  }

  void encodeInto(std::vector<Octet>& buffer) const
  {
    unsigned int octetBoundary = getOctetBoundary();
    align(buffer, octetBoundary);
    size_t length = _dataElementVector.size();
    if (0xffffffffu < length)
      throw EncoderException(L"HLAvariableArray::encodeInto(): array size is too big to encode!");
    buffer.push_back(uint8_t(length >> 24));
    buffer.push_back(uint8_t(length >> 16));
    buffer.push_back(uint8_t(length >> 8));
    buffer.push_back(uint8_t(length));
    align(buffer, octetBoundary);

    for (DataElementVector::const_iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      if (!*i)
        throw EncoderException(L"HLAvariableArray::encodeInto(): dataElement is zero!");
      (*i)->encodeInto(buffer);
    }
  }

  size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
  {
    unsigned int octetBoundary = getOctetBoundary();
    index = align(index, octetBoundary);
    if (buffer.size() < index + 4)
      throw EncoderException(L"Insufficient buffer size for decoding!");
    size_t length = size_t(uint8_t(buffer[index])) << 24;
    length |= size_t(uint8_t(buffer[index + 1])) << 16;
    length |= size_t(uint8_t(buffer[index + 2])) << 8;
    length |= size_t(uint8_t(buffer[index + 3]));
    index = align(index + 4, octetBoundary);

    while (length < _dataElementVector.size()) {
      delete _dataElementVector.back();
      _dataElementVector.back() = 0;
      _dataElementVector.pop_back();
    }

    _dataElementVector.reserve(length);
    while (_dataElementVector.size() < length)
      _dataElementVector.push_back(_protoType->clone().release());

    for (DataElementVector::const_iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      if (!*i)
        throw EncoderException(L"HLAvariableArray::decodeFrom(): dataElement is zero!");
      index = (*i)->decodeFrom(buffer, index);
    }

    return index;
  }

  size_t getEncodedLength() const
  {
    size_t encodedLength = 4;
    if (_protoType)
      encodedLength = align(encodedLength, _protoType->getOctetBoundary());
    for (DataElementVector::const_iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      const DataElement* dataElement = *i;
      if (!dataElement)
        dataElement = _protoType;
      if (!dataElement)
        continue;
      encodedLength = align(encodedLength, dataElement->getOctetBoundary()) + dataElement->getEncodedLength();
    }
    return encodedLength;
  }

  unsigned int getOctetBoundary() const
  { return std::max(_protoType->getOctetBoundary(), 4u); }

  size_t size () const
  { return _dataElementVector.size(); }

  void addElement(const DataElement& dataElement)
  {
    if (!dataElement.isSameTypeAs(*_protoType))
      throw EncoderException(L"HLAvariableArray::addElement(): Data type is not compatible!");
    _dataElementVector.push_back(dataElement.clone().release());
  }

  void addElementPointer(DataElement* dataElement)
  {
    if (!dataElement)
      throw EncoderException(L"HLAvariableArray::addElementPointer(): dataElement is zero!");
    if (!dataElement->isSameTypeAs(*_protoType))
      throw EncoderException(L"HLAvariableArray::addElementPointer(): Data type is not compatible!");
    _dataElementVector.push_back(dataElement);
  }

  void set(size_t index, const DataElement& dataElement)
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAvariableArray::set(): Index out of range!");
    if (!dataElement.isSameTypeAs(*_protoType))
      throw EncoderException(L"HLAvariableArray::set(): Data type is not compatible!");
    delete _dataElementVector[index];
    _dataElementVector[index] = dataElement.clone().release();
  }

  void setElementPointer(size_t index, DataElement* dataElement)
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAvariableArray::setElementPointer(): Index out of range!");
    if (!dataElement)
      throw EncoderException(L"HLAvariableArray::setElementPointer(): dataElement is zero!");
    if (!dataElement->isSameTypeAs(*_protoType))
      throw EncoderException(L"HLAvariableArray::setElementPointer(): Data type is not compatible!");
    delete _dataElementVector[index];
    _dataElementVector[index] = dataElement;
  }

  const DataElement& get(size_t index) const
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAvariableArray::get(): Index out of range!");
    if (!_dataElementVector[index])
      throw EncoderException(L"HLAvariableArray::get(): dataElement is zero!");
    return *_dataElementVector[index];
  }

  bool isSameTypeAs(const HLAvariableArrayImplementation& rhs) const
  {
    return _protoType->isSameTypeAs(*rhs._protoType);
  }

  DataElement* _protoType;

  typedef std::vector<DataElement*> DataElementVector;
  DataElementVector _dataElementVector;
};

HLAvariableArray::HLAvariableArray(const DataElement& protoType) :
  _impl(new HLAvariableArrayImplementation(protoType))
{
}

HLAvariableArray::HLAvariableArray(HLAvariableArray const & rhs) :
  _impl(new HLAvariableArrayImplementation(*rhs._impl))
{
}

HLAvariableArray::~HLAvariableArray()
{
  delete _impl;
  _impl = 0;
}

RTI_UNIQUE_PTR<DataElement>
HLAvariableArray::clone () const
{
  return RTI_UNIQUE_PTR<rti1516e::DataElement>(new HLAvariableArray(*this));
}

VariableLengthData
HLAvariableArray::encode () const
  RTI_THROW ((EncoderException))
{
  VariableLengthData variableLengthData;
  encode(variableLengthData);
  return variableLengthData;
}

void
HLAvariableArray::encode(VariableLengthData& inData) const
  RTI_THROW ((EncoderException))
{
  std::vector<Octet> buffer;
  buffer.reserve(getEncodedLength());
  encodeInto(buffer);
  if (!buffer.empty())
    inData.setData(&buffer.front(), buffer.size());
}

void
HLAvariableArray::encodeInto(std::vector<Octet>& buffer) const
  RTI_THROW ((EncoderException))
{
  _impl->encodeInto(buffer);
}

void HLAvariableArray::decode(VariableLengthData const & inData)
  RTI_THROW ((EncoderException))
{
  std::vector<Octet> buffer(inData.size());
  if (!buffer.empty())
    std::memcpy(&buffer.front(), inData.data(), inData.size());
  decodeFrom(buffer, 0);
}

size_t
HLAvariableArray::decodeFrom(std::vector<Octet> const & buffer, size_t index)
  RTI_THROW ((EncoderException))
{
  return _impl->decodeFrom(buffer, index);
}

size_t
HLAvariableArray::getEncodedLength() const
  RTI_THROW ((EncoderException))
{
  return _impl->getEncodedLength();
}

unsigned int
HLAvariableArray::getOctetBoundary() const
{
  return _impl->getOctetBoundary();
}

size_t
HLAvariableArray::size() const
{
  return _impl->size();
}

bool
HLAvariableArray::isSameTypeAs(DataElement const& inData ) const
{
  if (!DataElement::isSameTypeAs(inData))
    return false;
  const HLAvariableArray* variableArray = dynamic_cast<const HLAvariableArray*>(&inData);
  if (!variableArray)
    return false;
  return _impl->isSameTypeAs(*variableArray->_impl);
}

bool
HLAvariableArray::hasPrototypeSameTypeAs(DataElement const& dataElement) const
{
  const HLAvariableArray* variableArray = dynamic_cast<const HLAvariableArray*>(&dataElement);
  if (!variableArray)
    return false;
  return _impl->isSameTypeAs(*variableArray->_impl);
}

void
HLAvariableArray::addElement(const DataElement& dataElement)
  RTI_THROW ((EncoderException))
{
  _impl->addElement(dataElement);
}

void
HLAvariableArray::addElementPointer(DataElement* dataElement)
  RTI_THROW ((EncoderException))
{
  _impl->addElementPointer(dataElement);
}

void
HLAvariableArray::set(size_t index, const DataElement& dataElement)
  RTI_THROW ((EncoderException))
{
  _impl->set(index, dataElement);
}

void
HLAvariableArray::setElementPointer(size_t index, DataElement* dataElement)
  RTI_THROW ((EncoderException))
{
  _impl->setElementPointer(index, dataElement);
}

const DataElement&
HLAvariableArray::get(size_t index) const
  RTI_THROW ((EncoderException))
{
  return _impl->get(index);
}

DataElement const&
HLAvariableArray::operator [](size_t index) const
  RTI_THROW ((EncoderException))
{
  return _impl->get(index);
}

}
