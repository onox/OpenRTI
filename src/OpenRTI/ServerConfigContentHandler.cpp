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

#include "ServerConfigContentHandler.h"

#include <iosfwd>
#include <sstream>
#include <cstring>
#include <vector>

#include "Attributes.h"
#include "ContentHandler.h"
#include "ExpatXMLReader.h"
#include "StringUtils.h"

namespace OpenRTI {

static bool enableFlagToBool(const char* enableValue)
{
  if (!enableValue)
    return false;
  std::string enable = trim(enableValue);
  if (!enable.empty()) {
    if (enable[0] == '1' || enable[0] == 't' || enable[0] == 'T')
      return true;
    if (enable[0] == '0' || enable[0] == 'f' || enable[0] == 'F')
      return false;
  }
  throw RTIinternalError("Invalid enable flag!");
}

ServerConfigContentHandler::ServerConfigContentHandler() :
  _permitTimeRegulation(true),
  _enableZLibCompression(true)
{
}

ServerConfigContentHandler::~ServerConfigContentHandler()
{
}

void
ServerConfigContentHandler::startDocument(void)
{
  OpenRTIAssert(_modeStack.empty());
}

void
ServerConfigContentHandler::endDocument(void)
{
  OpenRTIAssert(_modeStack.empty());
}

void
ServerConfigContentHandler::startElement(const char* uri, const char* name,
                                         const char* qName, const XML::Attributes* atts)
{
  if (strcmp(name, "OpenRTIServerConfig") == 0) {
    if (!_modeStack.empty())
      throw RTIinternalError("OpenRTIServerConfig is not top level tag!");
    _modeStack.push_back(OpenRTIServerConfigMode);

    std::string version = trim(atts->getValue("version"));
    if (version != "1")
      throw RTIinternalError("Unknown OpenRTIServerConfig version!");

  } else if (strcmp(name, "parentServer") == 0) {
    if (getCurrentMode() != OpenRTIServerConfigMode)
      throw RTIinternalError("parentServer tag not inside of OpenRTIServerConfig tag!");
    _modeStack.push_back(ParentServerMode);

    _parentServerUrl = trim(atts->getValue("url"));

  } else if (strcmp(name, "permitTimeRegulation") == 0) {
    if (getCurrentMode() != OpenRTIServerConfigMode && getCurrentMode() != ListenMode)
      throw RTIinternalError("permitTimeRegulation tag not inside of OpenRTIServerConfig or listen tag!");
    _modeStack.push_back(PermitTimeRegulationMode);

    bool enable = enableFlagToBool(atts->getValue("enable"));
    _permitTimeRegulation = enable;

  } else if (strcmp(name, "enableZLibCompression") == 0) {
    if (getCurrentMode() != OpenRTIServerConfigMode)
      throw RTIinternalError("enableZLibCompression tag not inside of OpenRTIServerConfig or listen tag!");
    _modeStack.push_back(EnableZLibCompressionMode);

    bool enable = enableFlagToBool(atts->getValue("enable"));
    _enableZLibCompression = enable;

  } else if (strcmp(name, "listen") == 0) {
    if (getCurrentMode() != OpenRTIServerConfigMode)
      throw RTIinternalError("listen tag not inside of OpenRTIServerConfig!");
    _modeStack.push_back(ListenMode);

    _listenConfig.push_back(ListenConfig());
    _listenConfig.back()._url = trim(atts->getValue("url"));

  } else {
    throw RTIinternalError("Unknown tag!");
    _modeStack.push_back(UnknownMode);
  }
}

void
ServerConfigContentHandler::endElement(const char* uri, const char* name, const char* qName)
{
  _modeStack.pop_back();
}

}
