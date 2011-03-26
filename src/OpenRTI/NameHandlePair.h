/* -*-c++-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich 
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

#ifndef OpenRTI_NameHandlePair_h
#define OpenRTI_NameHandlePair_h

#include <string>
#include "Referenced.h"

namespace OpenRTI {

template<typename H>
class NameHandlePair : public Referenced {
public:
  typedef H Handle;

  NameHandlePair(const std::string& name, const Handle& handle) :
    _name(name), _handle(handle)
  { }

  const std::string& getName() const
  { return _name; }

  const Handle& getHandle() const
  { return _handle; }

private:  
  const std::string _name;
  const Handle _handle;
};

} // namespace OpenRTI

#endif
