/* -*-c++-*- OpenRTI - Copyright (C) 2009-2023 Mathias Froehlich
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

#ifndef OpenRTI_FDD1516FileReader_h
#define OpenRTI_FDD1516FileReader_h

#include <iosfwd>
#include <string>
#include "Export.h"

namespace OpenRTI {

class FOMStringModule;

class OPENRTI_API FDD1516FileReader {
public:
  static FOMStringModule read(std::istream& stream, const std::string& encoding);
};

} // namespace OpenRTI

#endif
