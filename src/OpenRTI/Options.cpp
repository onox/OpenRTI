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

#include "Options.h"

#include <cstring>
#include <iostream>

namespace OpenRTI {

Options::Options(int argc, const char* const argv[]) :
  _argc(argc),
  _argv(argv),
  _optCharIndex(1),
  _optIndex(1),
  _optChar(0)
{
}

Options::~Options()
{
}

bool
Options::next(const char *opts)
{
  _optChar = 0;
  _argument.clear();

  // Nothing more to process
  if (_argc <= _optIndex) {
    return false;
  }
  
  // We are past a -- argument
  if (_optCharIndex == 0) {
    _argument.assign(_argv[_optIndex++]);
    return true;
  }

  if (strcmp(_argv[_optIndex], "--") == 0) {
    // Have an option terminator.
    // switch to 'past -- argument' mode
    _optCharIndex = 0;
    if (_argc <= ++_optIndex)
      return false;
    // report the next argument
    _argument.assign(_argv[_optIndex++]);
    return true;
  }

  if (strcmp(_argv[_optIndex], "-") == 0) {
    // Have a single '-' which is a non option
    _argument.assign(_argv[_optIndex++]);
    return true;
  }

  // Just a non option argument, that is something not sterting with '-'
  if (_argv[_optIndex][0] != '-') {
    _argument.assign(_argv[_optIndex++]);
    return true;
  }
    
  // Now we try to match an option
  _optChar = _argv[_optIndex][_optCharIndex];
  const char* optionPosition;
  if (_optChar == ':' || (optionPosition = strchr(opts, _optChar)) == 0) {
    std::cerr << _argv[0] << ": illegal option \'" << _optChar
              << "\'" << std::endl;
    if (_argv[_optIndex][++_optCharIndex] == '\0') {
      ++_optIndex;
      _optCharIndex = 1;
    }
    _optChar = 0;
    return true;
  }
  if (optionPosition[1] == ':') {
    if (_argv[_optIndex][_optCharIndex+1] != '\0')
      _argument.assign(&_argv[_optIndex++][_optCharIndex+1]);
    else if (++_optIndex >= _argc) {
      std::cerr << _argv[0] << ": option \'" << _optChar
                << "\' requires an argument" << std::endl;
      _optCharIndex = 1;
      _optChar = 0;
      return true;
    } else
      _argument.assign(_argv[_optIndex++]);
    _optCharIndex = 1;
  } else {
    if (_argv[_optIndex][++_optCharIndex] == '\0') {
      _optCharIndex = 1;
      ++_optIndex;
    }
  }
  return true;
}

}
