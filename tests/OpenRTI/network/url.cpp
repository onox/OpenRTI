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

#include <cstdlib>
#include <iostream>

#include "URL.h"

static bool
test(const OpenRTI::URL& url)
{
  if (url != url)
    return false;
  if (!(url == url))
    return false;
  if (url < url)
    return false;
  // if (!(url <= url))
  //   return false;
  // if (url > url)
  //   return false;
  // if (!(url >= url))
  //   return false;

  OpenRTI::URL url2 = OpenRTI::URL::fromUrl(url.str());
  if (url2 != url)
    return false;
  if (url2.str() != url.str())
    return false;

  return true;
}

int main(int argc, char* argv[])
{
  OpenRTI::URL url;
  url.setProtocol("http");
  if (!test(url))
    return EXIT_FAILURE;

  url.setHost("host");
  if (!test(url))
    return EXIT_FAILURE;

  url.setService("17");
  if (!test(url))
    return EXIT_FAILURE;

  url.setPath("/path/to/file");
  if (!test(url))
    return EXIT_FAILURE;

  OpenRTI::StringPairVector queries;
  queries.push_back(OpenRTI::StringPair("key1", "value1"));
  queries.push_back(OpenRTI::StringPair("key2", "value2"));
  url.setQuery(queries);
  if (!test(url))
    return EXIT_FAILURE;

  url.setRef("ref");
  if (!test(url))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
