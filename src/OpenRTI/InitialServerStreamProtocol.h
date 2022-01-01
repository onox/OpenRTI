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

#ifndef OpenRTI_InitialServerStreamProtocol_h
#define OpenRTI_InitialServerStreamProtocol_h

#include "InitialStreamProtocol.h"

namespace OpenRTI {

class AbstractServer;

class OPENRTI_API InitialServerStreamProtocol : public InitialStreamProtocol {
public:
  InitialServerStreamProtocol(AbstractServer& abstractServer);
  virtual ~InitialServerStreamProtocol();

  virtual void readOptionMap(const StringStringListMap& clientOptionMap);
  void errorResponse(const std::string& errorMessage);

private:
  AbstractServer& _abstractServer;
};

} // namespace OpenRTI

#endif
