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

#include <StringUtils.h>

#include <RTI1516TestLib.h>

using namespace OpenRTI;

enum RequestType {
  Interaction1,
  Interaction2,
  Interaction3,
  Finished
};

class TestResponderAmbassador : public RTI1516TestAmbassador {
public:
  TestResponderAmbassador(const RTITest::ConstructorArgs& constructorArgs) :
    RTI1516TestAmbassador(constructorArgs)
  { }
  virtual ~TestResponderAmbassador()
    throw ()
  { }

  virtual bool execJoined(rti1516::RTIambassador& ambassador)
  {
    rti1516::InteractionClassHandle requestHandle;
    try {
      requestHandle = ambassador.getInteractionClassHandle(L"Request");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      _requestTypeHandle = ambassador.getParameterHandle(requestHandle, L"requestType");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    rti1516::InteractionClassHandle interactionClassHandle1;
    rti1516::InteractionClassHandle interactionClassHandle2;
    rti1516::InteractionClassHandle interactionClassHandle3;
    try {
      interactionClassHandle1 = ambassador.getInteractionClassHandle(L"InteractionClass1");
      interactionClassHandle2 = ambassador.getInteractionClassHandle(L"InteractionClass2");
      interactionClassHandle3 = ambassador.getInteractionClassHandle(L"InteractionClass3");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    rti1516::ParameterHandle class1Parameter1Handle;
    rti1516::ParameterHandle class2Parameter1Handle;
    rti1516::ParameterHandle class2Parameter2Handle;
    rti1516::ParameterHandle class3Parameter1Handle;
    rti1516::ParameterHandle class3Parameter2Handle;
    rti1516::ParameterHandle class3Parameter3Handle;
    try {
      class1Parameter1Handle = ambassador.getParameterHandle(interactionClassHandle1, L"parameter1");
      class2Parameter1Handle = ambassador.getParameterHandle(interactionClassHandle2, L"parameter1");
      class2Parameter2Handle = ambassador.getParameterHandle(interactionClassHandle2, L"parameter2");
      class3Parameter1Handle = ambassador.getParameterHandle(interactionClassHandle3, L"parameter1");
      class3Parameter2Handle = ambassador.getParameterHandle(interactionClassHandle3, L"parameter2");
      class3Parameter3Handle = ambassador.getParameterHandle(interactionClassHandle3, L"parameter3");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.subscribeInteractionClass(requestHandle);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.publishInteractionClass(interactionClassHandle1);
      ambassador.publishInteractionClass(interactionClassHandle2);
      ambassador.publishInteractionClass(interactionClassHandle3);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    if (!waitForAllFederates(ambassador))
      return false;

    _finishedCount = 1;
    while (_finishedCount < getFederateList().size()) {

      try {
        _receivedInteraction = false;
        Clock timeout = Clock::now() + Clock::fromSeconds(10);
        while (!_receivedInteraction) {
          if (ambassador.evokeCallback(10.0))
            continue;
          if (timeout < Clock::now()) {
            std::wcout << L"Timeout waiting for request!" << std::endl;
            return false;
          }
        }

      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }
      if (_requestType == Finished)
        continue;

      try {

        rti1516::InteractionClassHandle interactionClassHandle;
        rti1516::ParameterHandleValueMap parameterValues;
        rti1516::VariableLengthData tag = toVariableLengthData(Clock::now());

        switch (_requestType) {
        case Finished:
          std::wcout << L"unexpected Request type Finished!" << std::endl;
          return false;
        case Interaction1:
          interactionClassHandle = interactionClassHandle1;
          parameterValues[class1Parameter1Handle] = toVariableLengthData("parameter1");
          break;
        case Interaction2:
          interactionClassHandle = interactionClassHandle2;
          parameterValues[class2Parameter1Handle] = toVariableLengthData("parameter1");
          parameterValues[class2Parameter2Handle] = toVariableLengthData("parameter2");
          break;
        case Interaction3:
          interactionClassHandle = interactionClassHandle3;
          parameterValues[class3Parameter1Handle] = toVariableLengthData("parameter1");
          parameterValues[class3Parameter2Handle] = toVariableLengthData("parameter2");
          parameterValues[class3Parameter3Handle] = toVariableLengthData("parameter3");
          break;
        }

        ambassador.sendInteraction(interactionClassHandle, parameterValues, tag);

      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }
    }

    if (!waitForAllFederates(ambassador))
      return false;

    try {
      ambassador.unpublishInteractionClass(interactionClassHandle1);
      ambassador.unpublishInteractionClass(interactionClassHandle2);
      ambassador.unpublishInteractionClass(interactionClassHandle3);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.unsubscribeInteractionClass(requestHandle);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  void receiveInteraction(rti1516::InteractionClassHandle interactionClassHandle,
                          const rti1516::ParameterHandleValueMap& parameterValues,
                          const rti1516::VariableLengthData& tag,
                          rti1516::OrderType sentOrder,
                          rti1516::TransportationType theType)
      throw (rti1516::InteractionClassNotRecognized,
             rti1516::InteractionParameterNotRecognized,
             rti1516::InteractionClassNotSubscribed,
             rti1516::FederateInternalError)
  {
    _requestType = RequestType(toUnsigned(parameterValues.find(_requestTypeHandle)->second));
    if (_requestType == Finished)
      ++_finishedCount;
    _receivedInteraction = true;
  }

  bool _receivedInteraction;
  rti1516::ParameterHandle _requestTypeHandle;
  RequestType _requestType;
  unsigned _finishedCount;
};

class TestRequestorAmbassador : public RTI1516TestAmbassador {
public:
  TestRequestorAmbassador(const RTITest::ConstructorArgs& constructorArgs) :
    RTI1516TestAmbassador(constructorArgs)
  { }
  virtual ~TestRequestorAmbassador()
    throw ()
  { }

  virtual bool execJoined(rti1516::RTIambassador& ambassador)
  {
    rti1516::InteractionClassHandle requestHandle;
    try {
      requestHandle = ambassador.getInteractionClassHandle(L"Request");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    rti1516::ParameterHandle requestTypeHandle;
    try {
      requestTypeHandle = ambassador.getParameterHandle(requestHandle, L"requestType");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      interactionClassHandle1 = ambassador.getInteractionClassHandle(L"InteractionClass1");
      interactionClassHandle2 = ambassador.getInteractionClassHandle(L"InteractionClass2");
      interactionClassHandle3 = ambassador.getInteractionClassHandle(L"InteractionClass3");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      class1Parameter1Handle = ambassador.getParameterHandle(interactionClassHandle1, L"parameter1");
      class2Parameter1Handle = ambassador.getParameterHandle(interactionClassHandle2, L"parameter1");
      class2Parameter2Handle = ambassador.getParameterHandle(interactionClassHandle2, L"parameter2");
      class3Parameter1Handle = ambassador.getParameterHandle(interactionClassHandle3, L"parameter1");
      class3Parameter2Handle = ambassador.getParameterHandle(interactionClassHandle3, L"parameter2");
      class3Parameter3Handle = ambassador.getParameterHandle(interactionClassHandle3, L"parameter3");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.publishInteractionClass(requestHandle);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.subscribeInteractionClass(interactionClassHandle1);
      ambassador.subscribeInteractionClass(interactionClassHandle2);
      ambassador.subscribeInteractionClass(interactionClassHandle3);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    if (!waitForAllFederates(ambassador))
      return false;

    double t = 0;
    for (unsigned i = 0; i < 100; ++i) {

      Clock clock = Clock::now();

      try {
        rti1516::ParameterHandleValueMap parameterValues;
        rti1516::VariableLengthData tag = toVariableLengthData("parameter1");
        parameterValues[requestTypeHandle] = toVariableLengthData(unsigned(Interaction2));
        ambassador.sendInteraction(requestHandle, parameterValues, tag);

      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      try {
        _receivedInteraction = false;
        Clock timeout = Clock::now() + Clock::fromSeconds(10);
        while (!_receivedInteraction) {
          if (ambassador.evokeCallback(10.0))
            continue;
          if (timeout < Clock::now()) {
            std::wcout << L"Timeout waiting for response!" << std::endl;
            return false;
          }
        }

      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      clock = Clock::now() - clock;
      t += 1e-9*clock.getNSec();
    }

    std::cout << t / 100 << std::endl;

    try {
      rti1516::ParameterHandleValueMap parameterValues;
      rti1516::VariableLengthData tag = toVariableLengthData("parameter1");
      parameterValues[requestTypeHandle] = toVariableLengthData(unsigned(Finished));
      ambassador.sendInteraction(requestHandle, parameterValues, tag);

    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    if (!waitForAllFederates(ambassador))
      return false;

    try {
      ambassador.unpublishInteractionClass(requestHandle);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.unsubscribeInteractionClass(interactionClassHandle1);
      ambassador.unsubscribeInteractionClass(interactionClassHandle2);
      ambassador.unsubscribeInteractionClass(interactionClassHandle3);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  void receiveInteraction(rti1516::InteractionClassHandle interactionClassHandle,
                          const rti1516::ParameterHandleValueMap& parameterValues,
                          const rti1516::VariableLengthData& tag,
                          rti1516::OrderType sentOrder,
                          rti1516::TransportationType theType)
      throw (rti1516::InteractionClassNotRecognized,
             rti1516::InteractionParameterNotRecognized,
             rti1516::InteractionClassNotSubscribed,
             rti1516::FederateInternalError)
  {
    _clock = toClock(tag);
    _receivedInteraction = true;
  }

  Clock _clock;
  bool _receivedInteraction;

  rti1516::InteractionClassHandle interactionClassHandle1;
  rti1516::InteractionClassHandle interactionClassHandle2;
  rti1516::InteractionClassHandle interactionClassHandle3;

  rti1516::ParameterHandle class1Parameter1Handle;
  rti1516::ParameterHandle class2Parameter1Handle;
  rti1516::ParameterHandle class2Parameter2Handle;
  rti1516::ParameterHandle class3Parameter1Handle;
  rti1516::ParameterHandle class3Parameter2Handle;
  rti1516::ParameterHandle class3Parameter3Handle;
};















class Test : public RTITest {
public:
  Test(int argc, const char* const argv[]) :
    RTITest(argc, argv, false),
    _count(0)
  { }
  virtual Ambassador* createAmbassador(const ConstructorArgs& constructorArgs)
  {
    // There is one ambassador who answers the ping requests
    if (_count++ == 0)
      return new TestResponderAmbassador(constructorArgs);
    else
      return new TestRequestorAmbassador(constructorArgs);
  }
private:
  unsigned _count;
};

int
main(int argc, char* argv[])
{
  Test test(argc, argv);
  return test.exec();
}
