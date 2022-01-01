/* -*-c++-*- OpenRTI - Copyright (C) 2004-2022 Mathias Froehlich
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

#include "DefaultErrorHandler.h"

namespace OpenRTI {

DefaultErrorHandler::DefaultErrorHandler()
{
}

DefaultErrorHandler::~DefaultErrorHandler()
{
}

void
DefaultErrorHandler::error(const char* msg, unsigned line, unsigned col)
{
  _stream << "error: at line " << line << " in column " << col << ": \"" << msg << "\"" << std::endl;
}

void
DefaultErrorHandler::fatalError(const char* msg, unsigned line, unsigned col)
{
  _stream << "fatalError: at line " << line << " in column " << col << ": \"" << msg << "\"" << std::endl;
}

void
DefaultErrorHandler::warning(const char* msg, unsigned line, unsigned col)
{
}

std::string
DefaultErrorHandler::getMessages() const
{
  return _stream.str();
}

} // namespace OpenRTI
