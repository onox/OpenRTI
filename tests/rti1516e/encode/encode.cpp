/* -*-c++-*- OpenRTI - Copyright (C) 2017-2022 Mathias Froehlich
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

#include <algorithm>
#include <cstring>
#include <string>
#include <memory>

#include <iostream>

#include <RTI/VariableLengthData.h>
#include <RTI/encoding/BasicDataElements.h>
#include <RTI/encoding/DataElement.h>
#include <RTI/encoding/EncodingExceptions.h>
#include <RTI/encoding/EncodingConfig.h>
#include <RTI/encoding/HLAfixedArray.h>
#include <RTI/encoding/HLAfixedRecord.h>
#include <RTI/encoding/HLAopaqueData.h>
#include <RTI/encoding/HLAvariableArray.h>
#include <RTI/encoding/HLAvariantRecord.h>

#include "Rand.h"

struct DataType {
  enum Type {
    // scalar data types
    HLAASCIIchar,
    HLAunicodeChar,
    HLAboolean,
    HLAbyte,
    HLAoctet,
    HLAoctetPairBE,
    HLAoctetPairLE,
    HLAinteger16LE,
    HLAinteger16BE,
    HLAinteger32BE,
    HLAinteger32LE,
    HLAinteger64BE,
    HLAinteger64LE,
    HLAfloat32BE,
    HLAfloat32LE,
    HLAfloat64BE,
    HLAfloat64LE,

    // string data types
    HLAASCIIstring,
    HLAunicodeString,

    // aggregate data types
    HLAfixedArray,
    HLAvariableArray,
    HLAfixedRecord
  };
  Type type;
  // depending on type this contains the nested data types
  std::size_t count;
  const DataType* dataTypes[8];

  std::size_t getAlignment() const;
  rti1516e::DataElement* createDataElement(OpenRTI::Rand& rand) const;
};

std::size_t
DataType::getAlignment() const
{
  switch (type) {
  case HLAASCIIchar:
  case HLAunicodeChar:
  case HLAboolean:
  case HLAbyte:
  case HLAoctet:
  case HLAoctetPairBE:
  case HLAoctetPairLE:
  case HLAinteger16LE:
  case HLAinteger16BE:
  case HLAinteger32BE:
  case HLAinteger32LE:
  case HLAinteger64BE:
  case HLAinteger64LE:
  case HLAfloat32BE:
  case HLAfloat32LE:
  case HLAfloat64BE:
  case HLAfloat64LE:
    return count;

  case HLAASCIIstring:
  case HLAunicodeString:
    return 4;

  case HLAfixedArray:
    return dataTypes[0]->getAlignment();
  case HLAvariableArray:
    return std::max(std::size_t(4), dataTypes[0]->getAlignment());
  case HLAfixedRecord:
    {
      std::size_t alignment = 1;
      for (std::size_t i = 0; i < count; ++i)
        alignment = std::max(alignment, dataTypes[i]->getAlignment());
      return alignment;
    }

  default:
    return 1;
  }
}

rti1516e::DataElement*
DataType::createDataElement(OpenRTI::Rand& rand) const
{
  switch (type) {
  case HLAASCIIchar:
    return new rti1516e::HLAASCIIchar(rand.get());
  case HLAunicodeChar:
    // under linux wchar_t is 32 bits but unicode char is only 16 bits ...
    return new rti1516e::HLAunicodeChar(0xffff & rand.get());
  case HLAboolean:
    return new rti1516e::HLAboolean(rand.get());
  case HLAbyte:
    return new rti1516e::HLAbyte(rand.get());
  case HLAoctet:
    return new rti1516e::HLAoctet(rand.get());
  case HLAoctetPairBE:
    return new rti1516e::HLAoctetPairBE(rti1516e::OctetPair(rand.get(), rand.get()));
  case HLAoctetPairLE:
    return new rti1516e::HLAoctetPairLE(rti1516e::OctetPair(rand.get(), rand.get()));
  case HLAinteger16LE:
    return new rti1516e::HLAinteger16LE(rand.get());
  case HLAinteger16BE:
    return new rti1516e::HLAinteger16BE(rand.get());
  case HLAinteger32BE:
    return new rti1516e::HLAinteger32BE(rand.get());
  case HLAinteger32LE:
    return new rti1516e::HLAinteger32LE(rand.get());
  case HLAinteger64BE:
    return new rti1516e::HLAinteger64BE(rti1516e::Integer64(rand.get()) | rti1516e::Integer64(rand.get()) << 32);
  case HLAinteger64LE:
    return new rti1516e::HLAinteger64LE(rti1516e::Integer64(rand.get()) | rti1516e::Integer64(rand.get()) << 32);
  case HLAfloat32BE:
    return new rti1516e::HLAfloat32BE(float(rand.get()));
  case HLAfloat32LE:
    return new rti1516e::HLAfloat32LE(float(rand.get()));
  case HLAfloat64BE:
    return new rti1516e::HLAfloat64BE(double(rand.get()));
  case HLAfloat64LE:
    return new rti1516e::HLAfloat64LE(double(rand.get()));

  case HLAASCIIstring:
    {
      std::string s;
      s.resize(0xff & rand.get());
      for (std::size_t i = 0; i < s.size(); ++i)
        s[i] = rand.get();
      return new rti1516e::HLAASCIIstring(s);
    }
  case HLAunicodeString:
    {
      std::wstring s;
      s.resize(0xff & rand.get());
      for (std::size_t i = 0; i < s.size(); ++i)
        s[i] = 0xffff & rand.get();
      return new rti1516e::HLAunicodeString(s);
    }

  case HLAfixedArray:
    {
      RTI_UNIQUE_PTR<rti1516e::DataElement> dataElement(dataTypes[0]->createDataElement(rand));
      return new rti1516e::HLAfixedArray(*dataElement, count);
    }
  case HLAvariableArray:
    {
      RTI_UNIQUE_PTR<rti1516e::DataElement> dataElement(dataTypes[0]->createDataElement(rand));
      return new rti1516e::HLAvariableArray(*dataElement);
    }
  case HLAfixedRecord:
    {
      rti1516e::HLAfixedRecord* fixedRecord = new rti1516e::HLAfixedRecord;
      for (std::size_t i = 0; i < count; ++i) {
        RTI_UNIQUE_PTR<rti1516e::DataElement> dataElement(dataTypes[i]->createDataElement(rand));
        fixedRecord->appendElement(*dataElement);
      }
      return fixedRecord;
    }

  default:
    return new rti1516e::HLAopaqueData;
  }
}

bool
equal(const rti1516e::DataElement& left, const rti1516e::DataElement& right);

bool
equal(const rti1516e::HLAASCIIchar& left, const rti1516e::HLAASCIIchar& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAunicodeChar& left, const rti1516e::HLAunicodeChar& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAboolean& left, const rti1516e::HLAboolean& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAbyte& left, const rti1516e::HLAbyte& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAoctet& left, const rti1516e::HLAoctet& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAoctetPairBE& left, const rti1516e::HLAoctetPairBE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAoctetPairLE& left, const rti1516e::HLAoctetPairLE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAinteger16BE& left, const rti1516e::HLAinteger16BE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAinteger16LE& left, const rti1516e::HLAinteger16LE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAinteger32BE& left, const rti1516e::HLAinteger32BE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAinteger32LE& left, const rti1516e::HLAinteger32LE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAinteger64BE& left, const rti1516e::HLAinteger64BE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAinteger64LE& left, const rti1516e::HLAinteger64LE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAfloat32BE& left, const rti1516e::HLAfloat32BE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAfloat32LE& left, const rti1516e::HLAfloat32LE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAfloat64BE& left, const rti1516e::HLAfloat64BE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAfloat64LE& left, const rti1516e::HLAfloat64LE& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAASCIIstring& left, const rti1516e::HLAASCIIstring& right)
{ return left.get() == right.get(); }
bool
equal(const rti1516e::HLAunicodeString& left, const rti1516e::HLAunicodeString& right)
{ return left.get() == right.get(); }

bool
equal(const rti1516e::HLAfixedArray& left, const rti1516e::HLAfixedArray& right)
{
  if (left.size() != right.size())
    return false;
  for (std::size_t i = 0; i < left.size(); ++i) {
    if (!equal(left.get(i), right.get(i)))
      return false;
  }
  return true;
}
bool
equal(const rti1516e::HLAvariableArray& left, const rti1516e::HLAvariableArray& right)
{
  if (left.size() != right.size())
    return false;
  for (std::size_t i = 0; i < left.size(); ++i) {
    if (!equal(left.get(i), right.get(i)))
      return false;
  }
  return true;
}
bool
equal(const rti1516e::HLAfixedRecord& left, const rti1516e::HLAfixedRecord& right)
{
  if (left.size() != right.size())
    return false;
  for (std::size_t i = 0; i < left.size(); ++i) {
    if (!equal(left.get(i), right.get(i)))
      return false;
  }
  return true;
}

bool
equal(const rti1516e::HLAopaqueData& left, const rti1516e::HLAopaqueData& right)
{
  if (left.dataLength() != right.dataLength())
    return false;
  return 0 == std::memcmp(left.get(), right.get(), left.dataLength());
}

bool
equal(const rti1516e::VariableLengthData& left, const rti1516e::VariableLengthData& right)
{
  if (left.size() != right.size())
    return false;
  return 0 == std::memcmp(left.data(), right.data(), left.size());
}

bool
equal(const std::vector<rti1516e::Octet>& left, const rti1516e::VariableLengthData& right)
{
  if (left.size() != right.size())
    return false;
#if __cplusplus < 201103L
  return 0 == std::memcmp(&left[0], right.data(), left.size());
#else
  return 0 == std::memcmp(left.data(), right.data(), left.size());
#endif
}

template<typename T>
bool
typedEqual(const rti1516e::DataElement& left, const rti1516e::DataElement& right)
{
  try {
    return equal(dynamic_cast<const T&>(left), dynamic_cast<const T&>(right));
  } catch (...) {
    return false;
  }
}

bool
equal(const rti1516e::DataElement& left, const rti1516e::DataElement& right)
{
  if (typedEqual<rti1516e::HLAASCIIchar>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAunicodeChar>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAboolean>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAbyte>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAoctet>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAoctetPairBE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAoctetPairLE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAinteger16BE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAinteger16LE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAinteger32BE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAinteger32LE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAinteger64BE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAinteger64LE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAfloat32BE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAfloat32LE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAfloat64BE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAfloat64LE>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAASCIIstring>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAunicodeString>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAfixedArray>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAvariableArray>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAfixedRecord>(left, right))
    return true;
  if (typedEqual<rti1516e::HLAopaqueData>(left, right))
    return true;
  return false;
}

bool testDataElementEncoding(const DataType& dataType)
{
  OpenRTI::Rand rand;
  for (unsigned i = 0; i < 100; ++i) {
    RTI_UNIQUE_PTR<rti1516e::DataElement> dataElement(dataType.createDataElement(rand));
    if (!dataElement.get()) {
      std::cerr << "Failed to create a rti1516e::DataElement to test with!" << std::endl;
      return false;
    }

    try {
      dataElement->decode(rti1516e::VariableLengthData());
    } catch (const rti1516e::EncoderException&) {
      // Ok, shall happen here
    } catch (...) {
      std::cerr << "Unexpected exception while decoding empty data!" << std::endl;
      return false;
    }

    rti1516e::VariableLengthData variableLengthData;
    try {
      variableLengthData = dataElement->encode();
    } catch (...) {
      std::cerr << "Unexpected exception while encoding!" << std::endl;
      return false;
    }

    RTI_UNIQUE_PTR<rti1516e::DataElement> dataElement2(dataType.createDataElement(rand));
    try {
      dataElement2->decode(variableLengthData);
    } catch (...) {
      std::cerr << "Unexpected exception while decoding!" << std::endl;
      return false;
    }

    if (!equal(*dataElement, *dataElement2)) {
      std::cerr << "Data elements are not equal!" << std::endl;
      return false;
    }

    rti1516e::VariableLengthData variableLengthData2;
    try {
      variableLengthData2 = dataElement2->encode();
    } catch (...) {
      std::cerr << "Unexpected exception while encoding!" << std::endl;
      return false;
    }

    if (!equal(variableLengthData, variableLengthData2)) {
      std::cerr << "Encoded data is not equal!" << std::endl;
      return false;
    }

    std::vector<rti1516e::Octet> octetData;
    try {
      dataElement->encodeInto(octetData);
    } catch (...) {
      std::cerr << "Unexpected exception while encoding!" << std::endl;
      return false;
    }

    RTI_UNIQUE_PTR<rti1516e::DataElement> dataElement3(dataType.createDataElement(rand));
    try {
      dataElement3->decodeFrom(octetData, 0);
    } catch (...) {
      std::cerr << "Unexpected exception while decoding!" << std::endl;
      return false;
    }

    if (!equal(*dataElement, *dataElement3)) {
      std::cerr << "Data elements are not equal!" << std::endl;
      return false;
    }
    if (!equal(*dataElement2, *dataElement3)) {
      std::cerr << "Data elements are not equal!" << std::endl;
      return false;
    }


    std::vector<rti1516e::Octet> octetData3;
    try {
      dataElement3->encodeInto(octetData3);
    } catch (...) {
      std::cerr << "Unexpected exception while encoding!" << std::endl;
      return false;
    }

    if (octetData != octetData3) {
      std::cerr << "Encoded data is not equal!" << std::endl;
      return false;
    }

    if (!equal(octetData, variableLengthData)) {
      std::cerr << "Encoded data is not equal!" << std::endl;
      return false;
    }
  }

  return true;
}


static const DataType HLAASCIIcharDataType = {
  DataType::HLAASCIIchar, 1,
};
static const DataType HLAunicodeCharDataType = {
  DataType::HLAunicodeChar, 2,
};
static const DataType HLAbooleanDataType = {
  DataType::HLAboolean, 1,
};
static const DataType HLAbyteDataType = {
  DataType::HLAbyte, 1,
};
static const DataType HLAoctetDataType = {
  DataType::HLAoctet, 1,
};
static const DataType HLAoctetPairBEDataType = {
  DataType::HLAoctetPairBE, 2,
};
static const DataType HLAoctetPairLEDataType = {
  DataType::HLAoctetPairLE, 2,
};
static const DataType HLAinteger16BEDataType = {
  DataType::HLAinteger16BE, 2,
};
static const DataType HLAinteger16LEDataType = {
  DataType::HLAinteger16LE, 2,
};
static const DataType HLAinteger32BEDataType = {
  DataType::HLAinteger32BE, 4,
};
static const DataType HLAinteger32LEDataType = {
  DataType::HLAinteger32LE, 4,
};
static const DataType HLAinteger64BEDataType = {
  DataType::HLAinteger64BE, 8,
};
static const DataType HLAinteger64LEDataType = {
  DataType::HLAinteger64LE, 8,
};
static const DataType HLAfloat32BEDataType = {
  DataType::HLAfloat32BE, 4,
};
static const DataType HLAfloat32LEDataType = {
  DataType::HLAfloat32LE, 4,
};
static const DataType HLAfloat64BEDataType = {
  DataType::HLAfloat64BE, 8,
};
static const DataType HLAfloat64LEDataType = {
  DataType::HLAfloat64LE, 8,
};

static const DataType HLAASCIIstringDataType = {
  DataType::HLAASCIIstring, 8,
};
static const DataType HLAunicodeStringDataType = {
  DataType::HLAunicodeString, 8,
};

static const DataType Vec3fDataType = {
  DataType::HLAfixedArray, 3, { &HLAfloat32BEDataType, }
};
static const DataType Vec3dDataType = {
  DataType::HLAfixedArray, 3, { &HLAfloat64BEDataType, }
};

static const DataType VecfDataType = {
  DataType::HLAvariableArray, 17, { &HLAfloat32LEDataType, }
};
static const DataType VecdDataType = {
  DataType::HLAvariableArray, 17, { &HLAfloat64LEDataType, }
};

static const DataType GeodfDataType = {
  DataType::HLAfixedRecord, 3, { &HLAfloat32LEDataType, &HLAfloat32LEDataType, &HLAfloat32LEDataType, }
};
static const DataType GeoddDataType = {
  DataType::HLAfixedRecord, 3, { &HLAfloat64LEDataType, &HLAfloat64LEDataType, &HLAfloat64LEDataType, }
};

static const DataType StructAlignDataType = {
  DataType::HLAfixedRecord, 2, { &HLAfloat32LEDataType, &HLAfloat64LEDataType, }
};
static const DataType StructAlign2DataType = {
  DataType::HLAfixedRecord, 3, { &HLAinteger16LEDataType, &StructAlignDataType, &HLAunicodeStringDataType, }
};

// add more tests ...

int
main(int argc, char* argv[])
{
  if (!testDataElementEncoding(HLAASCIIcharDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAunicodeCharDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAbooleanDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAbyteDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAoctetDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAoctetPairBEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAoctetPairLEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAinteger16BEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAinteger16LEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAinteger32BEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAinteger32LEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAinteger64BEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAinteger64LEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAfloat32BEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAfloat32LEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAfloat64BEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAfloat64LEDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAASCIIstringDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(HLAunicodeStringDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(Vec3fDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(Vec3dDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(VecfDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(VecdDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(GeodfDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(GeoddDataType))
    return EXIT_FAILURE;

  if (!testDataElementEncoding(StructAlignDataType))
    return EXIT_FAILURE;
  if (!testDataElementEncoding(StructAlign2DataType))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
