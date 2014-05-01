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

#include <RTI/RTIambassadorFactory.h>
#include <RTI/RTIambassador.h>

#include <Options.h>
#include <StringUtils.h>

#include <RTI1516TestLib.h>

int
main(int argc, char* argv[])
{
  std::wstring federationExecutionName(L"federationExecutionName");
  std::wstring fullPathNameToTheFDDfile(L"fdd.xml");

  std::wstring objectClassName;
  std::wstring objectClassAttributeName;

  std::wstring interactionClassName;
  std::wstring parameterName;

  std::wstring dimensionName;

  OpenRTI::Options options(argc, argv);
  std::vector<std::wstring> args;
  while (options.next("a:d:i:F:o:O:p:")) {
    switch (options.getOptChar()) {
    case 'a':
      objectClassAttributeName = OpenRTI::localeToUcs(options.getArgument());
      break;
    case 'd':
      dimensionName = OpenRTI::localeToUcs(options.getArgument());
      break;
    case 'i':
      interactionClassName = OpenRTI::localeToUcs(options.getArgument());
      break;
    case 'F':
      federationExecutionName = OpenRTI::localeToUcs(options.getArgument());
      break;
    case 'O':
      fullPathNameToTheFDDfile = OpenRTI::localeToUcs(options.getArgument());
      break;
    case 'o':
      objectClassName = OpenRTI::localeToUcs(options.getArgument());
      break;
    case 'p':
      parameterName = OpenRTI::localeToUcs(options.getArgument());
      break;
    case '\0':
      args.push_back(OpenRTI::localeToUcs(options.getArgument()));
      break;
    }
  }

  OpenRTI::RTI1516SimpleAmbassador ambassador;

  ambassador.connect(args);

  // create, must work
  try {
    ambassador.createFederationExecution(federationExecutionName, fullPathNameToTheFDDfile);
  } catch (const rti1516::Exception& e) {
    std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::wcout << L"Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  // join must work
  try {
    ambassador.joinFederationExecution(L"federate", federationExecutionName);
  } catch (const rti1516::Exception& e) {
    std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::wcout << L"Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  if (!objectClassName.empty()) {
    rti1516::ObjectClassHandle objectClassHandle;
    std::wstring rtiName;
    try {
      objectClassHandle = ambassador.getObjectClassHandle(objectClassName);
      rtiName = ambassador.getObjectClassName(objectClassHandle);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    std::wcout << objectClassName << L": \"" << rtiName << "\" " << objectClassHandle.toString() << std::endl;

    if (!objectClassAttributeName.empty()) {
      rti1516::AttributeHandle attributeHandle;
      try {
        attributeHandle = ambassador.getAttributeHandle(objectClassHandle, objectClassAttributeName);
        rtiName = ambassador.getAttributeName(objectClassHandle, attributeHandle);
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return EXIT_FAILURE;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return EXIT_FAILURE;
      }
      std::wcout << L"  " << objectClassAttributeName << L": \"" << rtiName << "\" " << attributeHandle.toString() << std::endl;
    }
  }

  if (!interactionClassName.empty()) {
    rti1516::InteractionClassHandle interactionClassHandle;
    std::wstring rtiName;
    try {
      interactionClassHandle = ambassador.getInteractionClassHandle(interactionClassName);
      rtiName = ambassador.getInteractionClassName(interactionClassHandle);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    std::wcout << interactionClassName << L": \"" << rtiName << "\" " << interactionClassHandle.toString() << std::endl;

    if (!parameterName.empty()) {
      rti1516::ParameterHandle parameterHandle;
      try {
        parameterHandle = ambassador.getParameterHandle(interactionClassHandle, parameterName);
        rtiName = ambassador.getParameterName(interactionClassHandle, parameterHandle);
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return EXIT_FAILURE;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return EXIT_FAILURE;
      }
      std::wcout << L"  " << parameterName << L": \"" << rtiName << "\" " << parameterHandle.toString() << std::endl;
    }
  }

  if (!dimensionName.empty()) {
    rti1516::DimensionHandle dimensionHandle;
    std::wstring rtiName;
    unsigned upperBound = 0;
    try {
      dimensionHandle = ambassador.getDimensionHandle(dimensionName);
      rtiName = ambassador.getDimensionName(dimensionHandle);
      upperBound = ambassador.getDimensionUpperBound(dimensionHandle);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    std::wcout << L"  " << dimensionName << L": \"" << rtiName << "\" " << dimensionHandle.toString()
               << L", upperBound = " << upperBound << std::endl;
  }

  // and now resign must work
  try {
    ambassador.resignFederationExecution(rti1516::NO_ACTION);
  } catch (const rti1516::Exception& e) {
    std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::wcout << L"Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  // destroy, must work
  try {
    ambassador.destroyFederationExecution(federationExecutionName);
  } catch (const rti1516::Exception& e) {
    std::wcout << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::wcout << L"Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
