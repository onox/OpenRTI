/* -*-c++-*- OpenRTI - Copyright (C) 2010-2022 Mathias Froehlich
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

#ifndef OpenRTI_ParenthesesReader_h
#define OpenRTI_ParenthesesReader_h

#include <iosfwd>

#include "Export.h"
#include "StringUtils.h"

namespace OpenRTI {

/// Parser for lisp like parentheses syntax files.
/// Is used to parse rti 1.3 fed files.
class OPENRTI_API ParenthesesReader {
public:
  class OPENRTI_API ContentHandler {
  public:
    virtual ~ContentHandler();

    virtual void startDocument();
    virtual void endDocument();

    virtual void startElement(const ParenthesesReader& parenthesesReader, const StringVector& tokens);
    virtual void endElement();
  };

  class OPENRTI_API ErrorHandler {
  public:
    virtual ~ErrorHandler(void);

    virtual void error(const ParenthesesReader& parenthesesReader, const char* msg);
  };

  ParenthesesReader();
  ~ParenthesesReader();

  bool parse(std::istream& stream, ContentHandler& contentHandler, ErrorHandler& errorHandler);

  unsigned getLine() const
  { return _line; }
  unsigned getColumn() const
  { return _column; }

private:
  unsigned _line;
  unsigned _column;
};

} // namespace OpenRTI

#endif
