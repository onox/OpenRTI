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

#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include <RTI/RTIambassadorFactory.h>
#include <RTI/RTIambassador.h>

#include <Options.h>
#include <StringUtils.h>

#include <RTI1516ETestLib.h>

int
main(int argc, char* argv[])
{
  std::wstring federationExecutionName(L"federationExecutionName");
  bool useDataUrlObjectModels = false;
  std::vector<std::wstring> fomModules;
  std::wstring mimModule;

  std::wstring objectClassName;
  std::vector<std::wstring> attributeNameVector;

  std::wstring interactionClassName;
  std::vector<std::wstring> parameterNameVector;

  std::vector<std::wstring> dimensionNameVector;
  std::vector<std::wstring> updateRateNameVector;

  OpenRTI::Options options(argc, argv);
  std::vector<std::wstring> args;
  while (options.next("a:d:Di:F:M:o:O:p:u:")) {
    switch (options.getOptChar()) {
    case 'a':
      attributeNameVector.push_back(OpenRTI::localeToUcs(options.getArgument()));
      break;
    case 'd':
      dimensionNameVector.push_back(OpenRTI::localeToUcs(options.getArgument()));
      break;
    case 'D':
      useDataUrlObjectModels = true;
      break;
    case 'i':
      interactionClassName = OpenRTI::localeToUcs(options.getArgument());
      break;
    case 'F':
      federationExecutionName = OpenRTI::localeToUcs(options.getArgument());
      break;
    case 'M':
      mimModule = OpenRTI::localeToUcs(options.getArgument());
      break;
    case 'O':
      fomModules.push_back(OpenRTI::localeToUcs(options.getArgument()));
      break;
    case 'o':
      objectClassName = OpenRTI::localeToUcs(options.getArgument());
      break;
    case 'p':
      parameterNameVector.push_back(OpenRTI::localeToUcs(options.getArgument()));
      break;
    case 'u':
      updateRateNameVector.push_back(OpenRTI::localeToUcs(options.getArgument()));
      break;
    case '\0':
      args.push_back(OpenRTI::localeToUcs(options.getArgument()));
      break;
    }
  }

  OpenRTI::RTI1516ESimpleAmbassador ambassador;
  ambassador.setUseDataUrlObjectModels(useDataUrlObjectModels);

  try {
    if (args.empty())
      ambassador.connect(L"thread://");
    else
      ambassador.connect(args[0]);
  } catch (const rti1516e::Exception& e) {
    std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::wcout << L"Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  // create, must work
  try {
    if (mimModule.empty())
      ambassador.createFederationExecution(federationExecutionName, fomModules);
    else
      ambassador.createFederationExecutionWithMIM(federationExecutionName, fomModules, mimModule);
  } catch (const rti1516e::Exception& e) {
    std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::wcout << L"Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  // join must work
  try {
    ambassador.joinFederationExecution(L"federate", federationExecutionName);
  } catch (const rti1516e::Exception& e) {
    std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::wcout << L"Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  if (!objectClassName.empty()) {
    rti1516e::ObjectClassHandle objectClassHandle;
    std::wstring rtiName;
    try {
      objectClassHandle = ambassador.getObjectClassHandle(objectClassName);
      rtiName = ambassador.getObjectClassName(objectClassHandle);
    } catch (const rti1516e::Exception& e) {
      std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    std::wcout << objectClassName << L": \"" << rtiName << "\" " << objectClassHandle.toString() << std::endl;

    for (std::vector<std::wstring>::const_iterator i = attributeNameVector.begin(); i != attributeNameVector.end(); ++i) {
      rti1516e::AttributeHandle attributeHandle;
      try {
        attributeHandle = ambassador.getAttributeHandle(objectClassHandle, *i);
        rtiName = ambassador.getAttributeName(objectClassHandle, attributeHandle);
      } catch (const rti1516e::Exception& e) {
        std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
        return EXIT_FAILURE;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return EXIT_FAILURE;
      }
      std::wcout << L"  " << *i << L": \"" << rtiName << "\" " << attributeHandle.toString() << std::endl;
    }
  }

  if (!interactionClassName.empty()) {
    rti1516e::InteractionClassHandle interactionClassHandle;
    std::wstring rtiName;
    try {
      interactionClassHandle = ambassador.getInteractionClassHandle(interactionClassName);
      rtiName = ambassador.getInteractionClassName(interactionClassHandle);
    } catch (const rti1516e::Exception& e) {
      std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    std::wcout << interactionClassName << L": \"" << rtiName << "\" " << interactionClassHandle.toString() << std::endl;

   for (std::vector<std::wstring>::const_iterator i = parameterNameVector.begin(); i != parameterNameVector.end(); ++i) {
      rti1516e::ParameterHandle parameterHandle;
      try {
        parameterHandle = ambassador.getParameterHandle(interactionClassHandle, *i);
        rtiName = ambassador.getParameterName(interactionClassHandle, parameterHandle);
      } catch (const rti1516e::Exception& e) {
        std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
        return EXIT_FAILURE;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return EXIT_FAILURE;
      }
      std::wcout << L"  " << *i << L": \"" << rtiName << "\" " << parameterHandle.toString() << std::endl;
    }
  }

  for (std::vector<std::wstring>::const_iterator i = dimensionNameVector.begin(); i != dimensionNameVector.end(); ++i) {
    rti1516e::DimensionHandle dimensionHandle;
    std::wstring rtiName;
    unsigned upperBound = 0;
    try {
      dimensionHandle = ambassador.getDimensionHandle(*i);
      rtiName = ambassador.getDimensionName(dimensionHandle);
      upperBound = ambassador.getDimensionUpperBound(dimensionHandle);
    } catch (const rti1516e::Exception& e) {
      std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    std::wcout << L"  " << *i << L": \"" << rtiName << "\" " << dimensionHandle.toString()
               << L", upperBound = " << upperBound << std::endl;
  }

  for (std::vector<std::wstring>::const_iterator i = updateRateNameVector.begin(); i != updateRateNameVector.end(); ++i) {
    double updateRateValue = 0;
    try {
      updateRateValue = ambassador.getUpdateRateValue(*i);
    } catch (const rti1516e::Exception& e) {
      std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    std::wcout << L"  " << *i << L": " << updateRateValue << std::endl;
  }

  // and now resign must work
  try {
    ambassador.resignFederationExecution(rti1516e::NO_ACTION);
  } catch (const rti1516e::Exception& e) {
    std::wcout << L"rti1516e::Exception: \"" << e.what() << L"\"" << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::wcout << L"Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  // destroy, must work
  try {
    ambassador.destroyFederationExecution(federationExecutionName);
  } catch (const rti1516e::Exception& e) {
    std::wcout << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::wcout << L"Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
