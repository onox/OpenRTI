/* -*-c++-*- OpenRTI - Copyright (C) 2013-2022 Mathias Froehlich
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

#ifndef OpenRTI_DynamicModule_h
#define OpenRTI_DynamicModule_h

#include <string>
#include "Export.h"

namespace OpenRTI {

class OPENRTI_API DynamicModule {
public:

  /// Returns the file name of the code object that contains the given address.
  /// If the system does not support this functionality or some error occurs the
  /// returned file name is empty.
  static std::string getFileNameForAddress(const void* address);

private:
  DynamicModule(const DynamicModule&);
  DynamicModule& operator=(const DynamicModule&);

  // struct PrivateData;
  // PrivateData* _privateData;
};

} // namespace OpenRTI

#endif
