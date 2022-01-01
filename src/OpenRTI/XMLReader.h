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

#ifndef OpenRTI_XML_XMLReader_h
#define OpenRTI_XML_XMLReader_h

#include <iosfwd>
#include <string>

#include "Referenced.h"
#include "SharedPtr.h"

#include "Attributes.h"
#include "ContentHandler.h"
#include "ErrorHandler.h"

namespace OpenRTI {
namespace XML {

class OPENRTI_API XMLReader : public Referenced {
public:
  virtual ~XMLReader(void);
  virtual void parse(std::istream& stream, const std::string& encoding) = 0;

  ContentHandler* getContentHandler(void) const;
  void setContentHandler(ContentHandler* contentHandler);

  ErrorHandler* getErrorHandler(void) const;
  void setErrorHandler(ErrorHandler* errorHandler);

protected:
  SharedPtr<ContentHandler> mContentHandler;
  SharedPtr<ErrorHandler> mErrorHandler;
};

} // namespace XML
} // namespace OpenRTI

#endif
