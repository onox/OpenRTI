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

#include <RTI/encoding/DataElement.h>

#include <typeinfo>

namespace rti1516e
{

DataElement::~DataElement()
{
}

bool
DataElement::isSameTypeAs(DataElement const& inData) const
{
  return typeid(*this) == typeid(inData);
}

Integer64
DataElement::hash() const
{
  try {
    std::vector<Octet> buffer;
    encodeInto(buffer);
    Integer64 hash = 8191;
    for (std::vector<Octet>::const_iterator i = buffer.begin(); i != buffer.end(); ++i)
      hash = hash*Integer64(127) + Integer64(*i);
    return hash;
  } catch (const EncoderException&) {
    return -1;
  }
}

}
