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

#include "RTI.hh"
#include <cstring>
#include <ostream>

RTI::Exception::Exception(const char* reason) :
  _serial(0),
  _reason(0),
  _name(0)
{
  if (!reason)
    return;
  std::size_t size = strlen(reason) + 1;
  char* buffer = new char[size];
  memcpy(buffer, reason, size);
  _reason = buffer;
}

RTI::Exception::Exception(ULong serial, const char *reason) :
  _serial(serial),
  _reason(0),
  _name(0)
{
  if (!reason)
    return;
  std::size_t size = strlen(reason) + 1;
  char* buffer = new char[size];
  memcpy(buffer, reason, size);
  _reason = buffer;
}

RTI::Exception::Exception(const Exception& exception) :
  _serial(exception._serial),
  _reason(0),
  _name(exception._name)
{
  std::size_t size = strlen(exception._reason) + 1;
  char* buffer = new char[size];
  memcpy(buffer, exception._reason, size);
  _reason = buffer;
}

RTI::Exception::~Exception()
{
  delete [] _reason;
}

RTI::Exception&
RTI::Exception::operator=(const Exception& exception)
{
  _serial = exception._serial;
  delete _reason;
  std::size_t size = strlen(exception._reason) + 1;
  char* buffer = new char[size];
  memcpy(buffer, exception._reason, size);
  _reason = buffer;
  _name = exception._name;
  return *this;
}

std::ostream&
operator<<(std::ostream& os, const RTI::Exception* ex)
{
  return os << "RTI::Exception: reason: " << ex->_reason << " name: " << ex->_name;
}

std::ostream&
operator<<(std::ostream& os, const RTI::Exception& ex)
{
  return os << "RTI::Exception: reason: " << ex._reason << " name: " << ex._name;
}
