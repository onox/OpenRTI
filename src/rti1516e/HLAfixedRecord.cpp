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

#include <RTI/encoding/HLAfixedRecord.h>

#include <RTI/VariableLengthData.h>
#include <RTI/encoding/DataElement.h>

#include <cstring>
#include <vector>

#include "Encoding.h"
#include "Export.h"

namespace rti1516e
{

typedef std::vector<DataElement*> DataElementVector;

class OPENRTI_LOCAL HLAfixedRecordImplementation {
public:
  HLAfixedRecordImplementation() :
    _octetBoundary(0)
  {
  }
  HLAfixedRecordImplementation(const HLAfixedRecordImplementation& rhs) :
    _octetBoundary(0)
  {
    _dataElementVector.reserve(rhs._dataElementVector.size());
    for (DataElementVector::const_iterator i = rhs._dataElementVector.begin();
         i != rhs._dataElementVector.end(); ++i)
      _dataElementVector.push_back((*i)->clone().release());
  }
  ~HLAfixedRecordImplementation()
  {
    for (DataElementVector::iterator i = _dataElementVector.begin();
         i != _dataElementVector.end(); ++i) {
      delete *i;
      *i = 0;
    }
  }

  void encodeInto(std::vector<Octet>& buffer)
  {
    align(buffer, getOctetBoundary());
    for (DataElementVector::const_iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      align(buffer, (*i)->getOctetBoundary());
      (*i)->encodeInto(buffer);
    }
  }

  size_t decodeFrom(std::vector<Octet> const& buffer, size_t index)
  {
    index = align(index, getOctetBoundary());
    for (DataElementVector::iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      index = align(index, (*i)->getOctetBoundary());
      index = (*i)->decodeFrom(buffer, index);
    }
    return index;
  }

  size_t getEncodedLength() const
  {
    size_t length = 0;
    for (DataElementVector::const_iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      length = align(length, (*i)->getOctetBoundary());
      length += (*i)->getEncodedLength();
    }
    return length;
  }

  unsigned int getOctetBoundary()
  {
    if (_octetBoundary)
      return _octetBoundary;

    _octetBoundary = 1;
    for (DataElementVector::const_iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      unsigned int octetBoundary = (*i)->getOctetBoundary();
      if (octetBoundary < _octetBoundary)
        continue;
      _octetBoundary = octetBoundary;
    }
    return _octetBoundary;
  }

  bool isSameTypeAs(const HLAfixedRecordImplementation& rhs) const
  {
    if (_dataElementVector.size() != rhs._dataElementVector.size())
      return false;
    for (DataElementVector::const_iterator i = _dataElementVector.begin(), j = rhs._dataElementVector.begin();
         i != _dataElementVector.end(); ++i, ++j) {
      if (!(*i)->isSameTypeAs(**j))
        return false;
    }
    return true;
  }

  bool hasElementSameTypeAs(size_t index, DataElement const& inData) const
  {
    if (_dataElementVector.size() <= index)
      return false;
    return inData.isSameTypeAs(*_dataElementVector[index]);
  }

  size_t size() const
  { return _dataElementVector.size(); }

  void appendElement(const DataElement& dataElement)
  {
    _dataElementVector.push_back(dataElement.clone().release());
    _octetBoundary = 0;
  }

  void appendElementPointer(DataElement* dataElement)
  {
    if (!dataElement)
      throw EncoderException(L"HLAfixedRecord::appendElementPointer: Null pointer given!");
    _dataElementVector.push_back(dataElement);
    _octetBoundary = 0;
  }

  void set(size_t index, const DataElement& dataElement)
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAfixedRecord::setElement(): Index out of range!");
    if (!_dataElementVector[index]->isSameTypeAs(dataElement))
      throw EncoderException(L"HLAfixedRecord::setElement(): Incompatible dataElements!");
    delete _dataElementVector[index];
    _dataElementVector[index] = dataElement.clone().release();
    _octetBoundary = 0;
  }

  void setElementPointer(size_t index, DataElement* dataElement)
  {
    if (!dataElement)
      throw EncoderException(L"HLAfixedRecord::setElementPointer(): Null pointer given!");
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAfixedRecord::setElementPointer(): Index out of range!");
    if (!_dataElementVector[index]->isSameTypeAs(*dataElement))
      throw EncoderException(L"HLAfixedRecord::setElementPointer(): Incompatible dataElements!");
    delete _dataElementVector[index];
    _dataElementVector[index] = dataElement;
    _octetBoundary = 0;
  }

  const DataElement& get(size_t index) const
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAfixedRecord::get(size_t): Index out of range!");
    return *_dataElementVector[index];
  }

  const DataElement& arrayget(size_t index) const
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAfixedRecord::operator[](size_t): Index out of range!");
    return *_dataElementVector[index];
  }

  DataElementVector _dataElementVector;
  unsigned _octetBoundary;
};

HLAfixedRecord::HLAfixedRecord() :
  _impl(new HLAfixedRecordImplementation)
{
}

HLAfixedRecord::HLAfixedRecord(HLAfixedRecord const & rhs) :
  _impl(new HLAfixedRecordImplementation(*rhs._impl))
{
}

HLAfixedRecord::~HLAfixedRecord()
{
  delete _impl;
  _impl = 0;
}

RTI_UNIQUE_PTR<DataElement>
HLAfixedRecord::clone() const
{
  return RTI_UNIQUE_PTR<DataElement>(new HLAfixedRecord(*this));
}

VariableLengthData
HLAfixedRecord::encode() const
  RTI_THROW ((EncoderException))
{
  VariableLengthData variableLengthData;
  encode(variableLengthData);
  return variableLengthData;
}

void
HLAfixedRecord::encode(VariableLengthData& inData) const
  RTI_THROW ((EncoderException))
{
  std::vector<Octet> buffer;
  buffer.reserve(getEncodedLength());
  encodeInto(buffer);
  if (!buffer.empty())
    inData.setData(&buffer.front(), buffer.size());
}

void
HLAfixedRecord::encodeInto(std::vector<Octet>& buffer) const
  RTI_THROW ((EncoderException))
{
  _impl->encodeInto(buffer);
}

void
HLAfixedRecord::decode(VariableLengthData const& inData)
  RTI_THROW ((EncoderException))
{
  std::vector<Octet> buffer(inData.size());
  if (!buffer.empty())
    std::memcpy(&buffer.front(), inData.data(), inData.size());
  decodeFrom(buffer, 0);
}

size_t
HLAfixedRecord::decodeFrom(std::vector<Octet> const& buffer, size_t index)
  RTI_THROW ((EncoderException))
{
  return _impl->decodeFrom(buffer, index);
}

size_t
HLAfixedRecord::getEncodedLength() const
  RTI_THROW ((EncoderException))
{
  return _impl->getEncodedLength();
}

unsigned int
HLAfixedRecord::getOctetBoundary() const
{
  return _impl->getOctetBoundary();
}

bool
HLAfixedRecord::isSameTypeAs(DataElement const& inData ) const
{
  if (!DataElement::isSameTypeAs(inData))
    return false;
  const HLAfixedRecord* fixedRecord = dynamic_cast<const HLAfixedRecord*>(&inData);
  if (!fixedRecord)
    return false;
  return _impl->isSameTypeAs(*fixedRecord->_impl);
}

bool
HLAfixedRecord::hasElementSameTypeAs(size_t index, DataElement const& inData) const
{
  return _impl->hasElementSameTypeAs(index, inData);
}

size_t
HLAfixedRecord::size() const
{
  return _impl->size();
}

void
HLAfixedRecord::appendElement(const DataElement& dataElement)
{
  _impl->appendElement(dataElement);
}

void
HLAfixedRecord::appendElementPointer(DataElement* dataElement)
{
  _impl->appendElementPointer(dataElement);
}

void
HLAfixedRecord::set(size_t index, const DataElement& dataElement)
  RTI_THROW ((EncoderException))
{
  _impl->set(index, dataElement);
}

void
HLAfixedRecord::setElementPointer(size_t index, DataElement* dataElement)
  RTI_THROW ((EncoderException))
{
  _impl->setElementPointer(index, dataElement);
}

const DataElement&
HLAfixedRecord::get(size_t index) const
  RTI_THROW ((EncoderException))
{
  return _impl->get(index);
}

DataElement const&
HLAfixedRecord::operator[](size_t index) const
  RTI_THROW ((EncoderException))
{
  return _impl->arrayget(index);
}

}
