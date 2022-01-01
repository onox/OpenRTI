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

#ifndef OpenRTI_XML_ExpatXMLReader_h
#define OpenRTI_XML_ExpatXMLReader_h

#include "XMLReader.h"

namespace OpenRTI {
namespace XML {

class ExpatXMLReader : public XMLReader {
public:
  ExpatXMLReader(void);
  virtual ~ExpatXMLReader(void);
  // returns bool instead of void to signal errors
  virtual void parse(std::istream& stream, const std::string& encoding);
};

} // namespace XML
} // namespace OpenRTI

#endif
