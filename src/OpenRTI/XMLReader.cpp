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

#include "XMLReader.h"

namespace OpenRTI {
namespace XML {

XMLReader::~XMLReader(void)
{
}

ContentHandler*
XMLReader::getContentHandler(void) const
{
  const ContentHandler* ch = mContentHandler.get();
  return const_cast<ContentHandler*>(ch);
}

void
XMLReader::setContentHandler(ContentHandler* contentHandler)
{
  mContentHandler = contentHandler;
}

ErrorHandler*
XMLReader::getErrorHandler(void) const
{
  const ErrorHandler* eh = mErrorHandler.get();
  return const_cast<ErrorHandler*>(eh);
}

void
XMLReader::setErrorHandler(ErrorHandler* errorHandler)
{
  mErrorHandler = errorHandler;
}

} // namespace XML
} // namespace OpenRTI
