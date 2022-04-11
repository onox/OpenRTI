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

#include <RTI/VariableLengthData.h>

#include <limits>
#include <iosfwd>
#include <RTI/Exception.h>
#include "VariableLengthDataFriend.h"

namespace rti1516e
{

static void
variableLengthDataDeleteFunction(void* data)
{
  ::operator delete(data);
}

class OPENRTI_LOCAL VariableLengthDataImplementation : public OpenRTI::Referenced {
public:
  VariableLengthDataImplementation()
  { }
  VariableLengthDataImplementation(const OpenRTI::VariableLengthData& variableLengthData) :
    _variableLengthData(variableLengthData)
  { }
  VariableLengthDataImplementation(const void* data, size_t size) :
    _variableLengthData(data, size)
  { }
  VariableLengthDataImplementation(const VariableLengthDataImplementation& v) :
    _variableLengthData(v._variableLengthData)
  {
  }
  static void putAndDelete(VariableLengthDataImplementation* data)
  {
    if (OpenRTI::Referenced::put(data))
      return;
    delete data;
  }

  OpenRTI::VariableLengthData _variableLengthData;
};

// Note that the VariableLengthDataFriend implementation relies on
// this method setting impl to zero.
VariableLengthData::VariableLengthData() :
  _impl(new VariableLengthDataImplementation)
{
  VariableLengthDataImplementation::get(_impl);
}

VariableLengthData::VariableLengthData(void const* data, size_t size) :
  _impl(new VariableLengthDataImplementation(data, size))
{
  VariableLengthDataImplementation::get(_impl);
}

VariableLengthData::VariableLengthData(VariableLengthData const & rhs) :
  _impl(rhs._impl)
{
  VariableLengthDataImplementation::get(_impl);
}

VariableLengthData::~VariableLengthData()
{
  VariableLengthDataImplementation::putAndDelete(_impl);
}

VariableLengthData &
VariableLengthData::operator=(VariableLengthData const & rhs)
{
  if (_impl == rhs._impl)
    return *this;
  VariableLengthDataImplementation::get(rhs._impl);
  VariableLengthDataImplementation::putAndDelete(_impl);
  _impl = rhs._impl;
  return *this;
}

void const*
VariableLengthData::data() const
{
  if (!_impl)
    return 0;
  return _impl->_variableLengthData.constData();
}

size_t
VariableLengthData::size() const
{
  if (!_impl)
    return 0;
  return _impl->_variableLengthData.size();
}

void
VariableLengthData::setData(void const * inData, size_t inSize)
{
  // Note that we do not copy the old content here since we
  // will write a new content in any case
  if (1 < VariableLengthDataImplementation::count(_impl)) {
    VariableLengthDataImplementation::putAndDelete(_impl);
    _impl = new VariableLengthDataImplementation;
    VariableLengthDataImplementation::get(_impl);
  } else if (!_impl) {
    _impl = new VariableLengthDataImplementation;
    VariableLengthDataImplementation::get(_impl);
  }
  _impl->_variableLengthData.setData(inData, inSize);
}

void
VariableLengthData::setDataPointer(void* inData, size_t inSize)
{
  // Note that we do not copy the old content here since we
  // will write a new content in any case
  if (1 < VariableLengthDataImplementation::count(_impl)) {
    VariableLengthDataImplementation::putAndDelete(_impl);
    _impl = new VariableLengthDataImplementation;
    VariableLengthDataImplementation::get(_impl);
  } else if (!_impl) {
    _impl = new VariableLengthDataImplementation;
    VariableLengthDataImplementation::get(_impl);
  }
  /// FIXME: for now copy the external stuff in any case ...
  /// First think about multithreading before optimizing that
  // _impl->setDataPointer(inData, inSize);
  _impl->_variableLengthData.setData(inData, inSize);
}

void
VariableLengthData::takeDataPointer(void* inData, size_t inSize, VariableLengthDataDeleteFunction func)
{
  // Note that we do not copy the old content here since we
  // will write a new content in any case
  if (1 < VariableLengthDataImplementation::count(_impl)) {
    VariableLengthDataImplementation::putAndDelete(_impl);
    _impl = new VariableLengthDataImplementation;
    VariableLengthDataImplementation::get(_impl);
  } else if (!_impl) {
    _impl = new VariableLengthDataImplementation;
    VariableLengthDataImplementation::get(_impl);
  }
  // Not sure if this is correct. Anyhow, this kind of preserves current behavior
  if (!func)
    func = variableLengthDataDeleteFunction;
  _impl->_variableLengthData.takeDataPointer(inData, inSize, func);
}

VariableLengthData
VariableLengthDataFriend::create(const OpenRTI::VariableLengthData& variableLengthData)
{
  VariableLengthData rti1516VariableLengthData;
  rti1516VariableLengthData._impl->_variableLengthData = variableLengthData;
  return rti1516VariableLengthData;
}

const OpenRTI::VariableLengthData&
VariableLengthDataFriend::readPointer(const rti1516e::VariableLengthData& variableLengthData)
{
  return variableLengthData._impl->_variableLengthData;
}

OpenRTI::VariableLengthData&
VariableLengthDataFriend::writePointer(rti1516e::VariableLengthData& variableLengthData)
{
  return variableLengthData._impl->_variableLengthData;
}

}
