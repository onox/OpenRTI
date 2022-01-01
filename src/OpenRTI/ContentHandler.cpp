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

#include "ContentHandler.h"

namespace OpenRTI {
namespace XML {

ContentHandler::~ContentHandler(void)
{
}

void
ContentHandler::characters(const char* data, unsigned length)
{
}

void
ContentHandler::comment(const char* commentData, unsigned length)
{
}

void
ContentHandler::startDocument(void)
{
}

void
ContentHandler::endDocument(void)
{
}

void
ContentHandler::startElement(const char*, const char*, const char*,
                             const Attributes*)
{
}

void
ContentHandler::endElement(const char*, const char*, const char*)
{
}

void
ContentHandler::skippedEntity(const char* name)
{
}

void
ContentHandler::processingInstruction(const char*, const char*)
{
}

} // namespace XML
} // namespace OpenRTI
