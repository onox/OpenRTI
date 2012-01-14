/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich 
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
#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include <Options.h>
#include <StringUtils.h>
#include <Thread.h>

#include <RTI1516TestLib.h>

using namespace OpenRTI;

class TestAmbassador : public RTI1516TestAmbassador {
public:
  TestAmbassador(const RTITest::ConstructorArgs& constructorArgs) :
    RTI1516TestAmbassador(constructorArgs)
  { }
  virtual ~TestAmbassador()
    throw ()
  { }
  
  virtual bool execJoined(rti1516::RTIambassador& ambassador)
  {
    if (!waitForAllFederates(ambassador))
      return false;
    if (!waitForAllFederates(ambassador))
      return false;
    return true;
  }
};

class Test : public RTITest {
public:
  Test(int argc, const char* const argv[]) :
    RTITest(argc, argv, false)
  { }
  virtual Ambassador* createAmbassador(const ConstructorArgs& constructorArgs)
  {
    return new TestAmbassador(constructorArgs);
  }
};

int
main(int argc, char* argv[])
{
  Test test(argc, argv);
  return test.exec();
}
