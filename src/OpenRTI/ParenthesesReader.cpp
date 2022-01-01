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

#include "ParenthesesReader.h"

#include <istream>

namespace OpenRTI {

ParenthesesReader::ContentHandler::~ContentHandler()
{
}

void
ParenthesesReader::ContentHandler::startDocument()
{
}

void
ParenthesesReader::ContentHandler::endDocument()
{
}

void
ParenthesesReader::ContentHandler::startElement(const ParenthesesReader& parenthesesReader, const StringVector& tokens)
{
}

void
ParenthesesReader::ContentHandler::endElement()
{
}

ParenthesesReader::ErrorHandler::~ErrorHandler(void)
{
}

void
ParenthesesReader::ErrorHandler::error(const ParenthesesReader& parenthesesReader, const char* msg)
{
}

ParenthesesReader::ParenthesesReader() :
  _line(0),
  _column(0)
{
}

ParenthesesReader::~ParenthesesReader()
{
}

bool
ParenthesesReader::parse(std::istream& stream, ContentHandler& contentHandler, ErrorHandler& errorHandler)
{
  contentHandler.startDocument();

  _line = 0;
  _column = 0;

  bool newToken = true;
  bool allowToken = false;
  StringVector tokenList;
  while (stream.good()) {
    int c = stream.get();
    ++_column;
    switch (c) {
    case -1:
      break;

    case '(':
      if (allowToken) {
        // flush the collected tokes so far
        if (tokenList.empty()) {
          errorHandler.error(*this, "Opening brace without any token before");
          return false;
        }
        contentHandler.startElement(*this, tokenList);
        tokenList.clear();
      }
      newToken = true;
      allowToken = true;
      break;

    case ')':
      // flush the collected tokes so far
      if (!tokenList.empty()) {
        contentHandler.startElement(*this, tokenList);
        tokenList.clear();
      }
      contentHandler.endElement();
      newToken = true;
      allowToken = false;
      break;

    case '\n':
      ++_line;
      // fallthrough
    case '\r':
      _column = 0;
      // fallthrough
    case ' ':
    case '\t':
      newToken = true;
      break;

    case ';':
      c = stream.get();
      if (c == -1)
        break;
      // It's only a comment if we have ';;'
      if (c == ';') {
        // Then skip the rest of the line
        while ((c = stream.get()) != -1) {
          if (c == '\n' || c == '\r') {
            stream.putback(c);
            break;
          }
        }
        break;
      } else {
        // undo the second character get
        stream.putback(c);
        // and restore the first character in c
        c = ';';
        // fallthrough, treat the ';' as the beginning of a token.
      }

    default:
      if (!allowToken) {
        errorHandler.error(*this, "Non whitespace character past closing brace");
        return false;
      }

      if (newToken) {
        tokenList.push_back(std::string());
        newToken = false;
      }
      tokenList.back().push_back(c);
      break;
    }
  }
  contentHandler.endDocument();
  return true;
}

} // namespace OpenRTI
