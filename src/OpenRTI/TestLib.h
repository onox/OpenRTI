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

#ifndef OpenRTI_TestLib_h
#define OpenRTI_TestLib_h

#include <iostream>

#include <string>
#include <vector>

#include <Condition.h>
#include <Clock.h>
#include <Options.h>
#include <Referenced.h>
#include <ScopeLock.h>
#include <ScopeUnlock.h>
#include <NetworkServer.h>
#include <Rand.h>
#include <SharedPtr.h>
#include <StringUtils.h>
#include <Thread.h>

#if !defined(_WIN32)
#include <sys/time.h>
#include <sys/resource.h>
#endif

namespace OpenRTI {

class OPENRTI_LOCAL ServerPool {
public:
  ServerPool()
  {
#if !defined(_WIN32)
    struct rlimit limit;
    getrlimit(RLIMIT_NOFILE, &limit);
    limit.rlim_cur = limit.rlim_max;
    setrlimit(RLIMIT_NOFILE, &limit);
#endif
  }
  ~ServerPool()
  { stopServerPool(); }

  void startServerPool(unsigned numServers, unsigned numClientsPerServers)
  {
    if (numServers <= 0)
      return;

    typedef std::list<SocketAddress> AddressList;
    AddressList addressList;

    SocketAddress listeningAddress = startServer(SocketAddress(), false);
    addressList.push_back(listeningAddress);

    AddressList parentAddressList = addressList;
    for (;addressList.size() < numServers;) {
      AddressList currentAddressList;
      for (AddressList::iterator j = parentAddressList.begin(); j != parentAddressList.end(); ++j) {
        for (unsigned k = 0; k < numClientsPerServers; ++k) {
          listeningAddress = startServer(*j, k%2 == 1);
          addressList.push_back(listeningAddress);
          currentAddressList.push_back(listeningAddress);
          if (numServers <= addressList.size())
            return;
        }
      }
      parentAddressList.swap(currentAddressList);
    }
  }

  void stopServerPool()
  {
    while (!_serverThreadList.empty()) {
      _serverThreadList.back()->stopServer();
      _serverThreadList.pop_back();
    }
  }

  std::string getAddress(unsigned i) const
  {
    if (_serverThreadList.empty())
      return std::string();
    return _serverThreadList[i % _serverThreadList.size()]->getAddress().getNumericName();
  }

private:
  class OPENRTI_LOCAL ServerThread : public Thread {
  public:
    void setupServer(const std::string& host, const SocketAddress& parentAddress, bool compress)
    {
      std::list<SocketAddress> addressList = SocketAddress::resolve(host, "0", true);
      // Set up a stream socket for the server connect
      bool success = false;
      while (!addressList.empty()) {
        SocketAddress address = addressList.front();
        addressList.pop_front();
        try {
          _address = _server.listenInet(address, 20);
          success = true;
          break;
        } catch (const OpenRTI::Exception&) {
          if (addressList.empty() && !success)
            throw;
        }
      }
      _server.setServerName(_address.getNumericName());

      if (parentAddress.valid()) {
        Clock abstime = Clock::now() + Clock::fromSeconds(1);
        _server.connectParentInetServer(parentAddress, compress, abstime);
      }

      start();
    }

    void stopServer()
    {
      _server.postDone();
      wait();
    }

    const SocketAddress& getAddress() const
    { return _address; }

  protected:
    virtual void run()
    { _server.exec(); }

    NetworkServer _server;
    SocketAddress _address;
  };

  SocketAddress startServer(const SocketAddress& parentAddress, bool compress)
  {
    SharedPtr<ServerThread> serverThread = new ServerThread;
    serverThread->setupServer("localhost", parentAddress, compress);
    _serverThreadList.push_back(serverThread);
    return serverThread->getAddress();
  }

  typedef std::vector<SharedPtr<ServerThread> > ServerThreadList;
  ServerThreadList _serverThreadList;
};

class OPENRTI_LOCAL RTITest {
public:
  // Helper class to test things lie create federation execution
  // where many concurrent attempts to do the same must fail n-1
  // times and succeed exactly once.
  class OPENRTI_LOCAL FederationBarrier : public Referenced {
  public:
    FederationBarrier(unsigned numThreads) :
      _numThreads(numThreads),
      _numWaitingThreads(0),
      _serial(-17),
      _successCount(0),
      _failCount(0)
    { }

    void addThread()
    {
      ScopeLock scopeLock(_mutex);
      if (_numThreads == ++_numWaitingThreads) {
        _resetAndSignal();
      } else {
        _wait(scopeLock);
      }
    }
    bool removeThread()
    {
      ScopeLock scopeLock(_mutex);
      if (--_numThreads == _numWaitingThreads) {
        _resetAndSignal();
        return _checkUniqueSuccessAndClear();
      }
      return true;
    }

    bool success()
    {
      ScopeLock scopeLock(_mutex);
      ++_successCount;
      if (_numThreads == ++_numWaitingThreads) {
        _resetAndSignal();
        return _checkUniqueSuccessAndClear();
      } else {
        _wait(scopeLock);
        return true;
      }
    }
    bool fail()
    {
      ScopeLock scopeLock(_mutex);
      ++_failCount;
      if (_numThreads == ++_numWaitingThreads) {
        _resetAndSignal();
        return _checkUniqueSuccessAndClear();
      } else {
        _wait(scopeLock);
        return true;
      }
    }

  private:
    void _resetAndSignal()
    {
      _numWaitingThreads = 0;
      ++_serial;
      _condition.notify_all();
    }
    void _wait(ScopeLock& scopeLock)
    {
      unsigned serial = _serial;
      do {
        _condition.wait(scopeLock);
      } while (serial == _serial);
    }
    bool _checkUniqueSuccessAndClear()
    {
      bool success = _successCount == std::min(1u, _numThreads);
      if (!success)
        std::cout << "Aborting due to non unique success: successCount = " << _successCount << std::endl;
      _successCount = 0;
      _failCount = 0;
      return success;
    }

    mutable Mutex _mutex;
    Condition _condition;

    unsigned _numThreads;
    unsigned _numWaitingThreads;
    unsigned _serial;

    unsigned _successCount;
    unsigned _failCount;
  };

  class OPENRTI_LOCAL LBTS : public Referenced {
  public:
    LBTS(unsigned numInitialThreads)
    {
      _lbtsMap[~0u] = numInitialThreads;
    }
    void replaceLBTS(unsigned oldLbts, unsigned newLbts)
    {
      ScopeLock scopeLock(_mutex);

      std::map<unsigned, unsigned>::iterator i = _lbtsMap.find(newLbts);
      if (i == _lbtsMap.end())
        _lbtsMap[newLbts] = 1;
      else
        i->second += 1;

      i = _lbtsMap.find(oldLbts);
      if (i == _lbtsMap.end())
        return;
      i->second -= 1;
      if (i->second != 0)
        return;

      // if ((i->first % 100) == 0)
      //   std::cout << i->first << std::endl;

      _lbtsMap.erase(i);
    }
    unsigned getLBTS(unsigned ownTime) const
    {
      ScopeLock scopeLock(_mutex);
      for (std::map<unsigned, unsigned>::const_iterator i = _lbtsMap.begin(); i != _lbtsMap.end(); ++i) {
        if (i->first != ownTime || 1 < i->second)
          return i->first;
      }
      return ~0u;
    }

  private:
    mutable Mutex _mutex;
    std::map<unsigned, unsigned> _lbtsMap;
  };

  struct ConstructorArgs {
    std::wstring _federationExecution;
    std::wstring _federateType;
    std::wstring _fddFile;
    std::wstring _mimFile;
    std::wstring _address;
    std::vector<std::wstring> _argumentList;
    std::vector<std::wstring> _federateList;
    bool _disjointFederations;
    bool _joinOnce;
    SharedPtr<FederationBarrier> _federationBarrier;
    SharedPtr<LBTS> _lbts;
    Rand _rand;
  };
  class OPENRTI_LOCAL Ambassador : public Referenced {
  public:
    Ambassador(const ConstructorArgs& constructorArgs) :
      _constructorArgs(constructorArgs),
      _ownTime(~0u)
    { }
    virtual ~Ambassador() {}

    bool callExec()
    {
      _constructorArgs._federationBarrier->addThread();
      bool result = exec();
      clearLBTS();
      _constructorArgs._federationBarrier->removeThread();
      return result;
    }

    virtual bool exec() = 0;

    const std::wstring& getFederationExecution() const
    { return _constructorArgs._federationExecution; }
    const std::wstring& getFederateType() const
    { return _constructorArgs._federateType; }
    const std::wstring& getFddFile() const
    { return _constructorArgs._fddFile; }
    std::vector<std::wstring> getFddFileList() const
    { std::vector<std::wstring> fddFileList; fddFileList.push_back(getFddFile()); return fddFileList; }
    const std::wstring& getMimFile() const
    { return _constructorArgs._mimFile; }
    std::vector<std::wstring> getArgumentList() const
    {
      std::vector<std::wstring> argumentList = _constructorArgs._argumentList;
      if (_constructorArgs._address.empty()) {
        argumentList.push_back(L"protocol=thread");
      } else {
        argumentList.push_back(L"protocol=rti");
        argumentList.push_back(std::wstring(L"address=") + _constructorArgs._address);
      }
      return argumentList;
    }
    std::wstring getConnectUrl() const
    {
      if (_constructorArgs._address.empty()) {
        return L"thread:///";
      } else {
        return std::wstring(L"rti://") + _constructorArgs._address + L"/";
      }
    }
    const std::vector<std::wstring>& getFederateList() const
    { return _constructorArgs._federateList; }
    bool getDisjointFederations() const
    { return _constructorArgs._disjointFederations; }

    const SharedPtr<FederationBarrier>& getFederationBarrier()
    { return _constructorArgs._federationBarrier; }

    // For the case where exactly one ambassador must finish successful, call this pair.
    // It ensures that exactly one success is accounted.
    bool success()
    { return _constructorArgs._federationBarrier->success(); }
    bool fail()
    { return _constructorArgs._federationBarrier->fail(); }

    // Wait for all ambassadors
    bool wait()
    {
      if (_constructorArgs._federateList.empty())
        return false;
      if (_constructorArgs._federateList[0] == getFederateType()) {
        return success();
      } else {
        return fail();
      }
    }

    unsigned getLBTS() const
    { return _constructorArgs._lbts->getLBTS(_ownTime); }
    void setLBTS(unsigned newTime)
    {
      if (_ownTime == newTime)
        return;
      _constructorArgs._lbts->replaceLBTS(_ownTime, newTime);
      _ownTime = newTime;
    }
    void clearLBTS()
    { setLBTS(~0u); }

    uint32_t getRandomNumber()
    { return _constructorArgs._rand.get(); }

  protected:
    ConstructorArgs _constructorArgs;
    unsigned _ownTime;
  };

  RTITest(int argc, const char* const argv[], bool disjointFederations) :
    _optionString("A:C:F:JM:O:S:"),
    _options(argc, argv),
    _federationExecution(L"FederationExecution"),
    _numServers(1),
    _numClientsPerServers(2),
    _numAmbassadorThreads(1),
    _disjointFederations(disjointFederations),
    _joinOnce(false)
  { }
  virtual ~RTITest()
  { }

  virtual Ambassador* createAmbassador(const ConstructorArgs&) = 0;

  void insertOptionString(const std::string& optionString)
  {
    for (std::string::size_type i = 0; i < optionString.size(); ++i) {
      if (optionString[i] == ':')
        continue;
      if (_optionString.find(optionString[i]) != std::string::npos)
        std::cerr << "Duplicate option string for test arguments!" << std::endl;
    }
    _optionString.append(optionString);
  }

  virtual bool processOption(char optchar, const std::string& argument)
  {
    switch (optchar) {
    case 'A':
      _numAmbassadorThreads = atoi(argument.c_str());
      return true;
    case 'C':
      _numClientsPerServers = atoi(argument.c_str());
      return true;
    case 'F':
      _federationExecution = localeToUcs(argument);
      return true;
    case 'M':
      _mimFile = localeToUcs(argument);
      return true;
    case 'O':
      _fddFile = localeToUcs(argument);
      return true;
    case 'S':
      _numServers = atoi(argument.c_str());
      return true;
    case 'J':
      _joinOnce = true;
      return true;
    case '\0':
      _globalArgumentList.push_back(localeToUcs(argument));
      return true;
    default:
      return false;
    }
  }

  int exec()
  {
    while (_options.next(_optionString.c_str())) {
      if (!processOption(_options.getOptChar(), _options.getArgument()))
        return EXIT_FAILURE;
    }

    // Start up the requested server tree.
    _serverPool.startServerPool(_numServers, _numClientsPerServers);

    if (_numAmbassadorThreads < 1)
      return EXIT_SUCCESS;

    typedef std::list<SharedPtr<AmbassadorThread> > AmbassadorThreadList;
    AmbassadorThreadList _ambassadorThreadList;

    Rand rand;
    ConstructorArgs constructorArgs;
    constructorArgs._fddFile = _fddFile;
    constructorArgs._mimFile = _mimFile;
    constructorArgs._federationBarrier = new FederationBarrier(_numAmbassadorThreads);
    constructorArgs._lbts = new LBTS(_numAmbassadorThreads);
    constructorArgs._argumentList = _globalArgumentList;
    constructorArgs._disjointFederations = _disjointFederations;
    constructorArgs._joinOnce = _joinOnce;
    for (unsigned i = 0; i < _numAmbassadorThreads; ++i) {
      std::wstringstream federateType;
      federateType << "Federate" << i;
      constructorArgs._federateList.push_back(federateType.str());
    }

    for (unsigned i = 1; i < _numAmbassadorThreads; ++i) {
      std::wstring federateType = constructorArgs._federateList[i];
      constructorArgs._federateType = federateType;
      if (_disjointFederations) {
        std::wstringstream federationExecution;
        federationExecution << _federationExecution << i;
        constructorArgs._federationExecution = federationExecution.str();
      } else {
        constructorArgs._federationExecution = _federationExecution;
      }
      constructorArgs._address = utf8ToUcs(_serverPool.getAddress(i));
      constructorArgs._rand = Rand(rand.get(), rand.get());

      SharedPtr<Ambassador> testAmbassador;
      testAmbassador = createAmbassador(constructorArgs);

      SharedPtr<AmbassadorThread> ambassadorThread;
      ambassadorThread = new AmbassadorThread(testAmbassador);
      _ambassadorThreadList.push_back(ambassadorThread);
      ambassadorThread->start();
    }

    // Execute one of them in the main thread
    std::wstring federateType = constructorArgs._federateList[0];
    constructorArgs._federateType = federateType;
    constructorArgs._address = utf8ToUcs(_serverPool.getAddress(0));
    constructorArgs._rand = Rand(rand.get(), rand.get());
    constructorArgs._federationExecution = _federationExecution;

    SharedPtr<Ambassador> testAmbassador;
    testAmbassador = createAmbassador(constructorArgs);

    bool success = testAmbassador->callExec();

    while (!_ambassadorThreadList.empty()) {
      _ambassadorThreadList.front()->wait();
      success = success && _ambassadorThreadList.front()->getSuccess();
      _ambassadorThreadList.pop_front();
    }
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  bool getDisjointFederations() const
  { return _disjointFederations; }
  void setDisjointFederations(bool disjointFederations)
  { _disjointFederations = disjointFederations; }

private:
  typedef std::vector<std::wstring> ArgumentList;

  class OPENRTI_LOCAL AmbassadorThread : public Thread {
  public:
    AmbassadorThread(const SharedPtr<Ambassador>& testAmbassador) :
      _testAmbassador(testAmbassador),
      _success(true)
    { }

    bool getSuccess() const
    { return _success; }

  protected:
    virtual void run()
    { _success = _testAmbassador->callExec(); }

  private:
    SharedPtr<Ambassador> _testAmbassador;
    bool _success;
  };

  ServerPool _serverPool;

  std::string _optionString;
  OpenRTI::Options _options;
  ArgumentList _globalArgumentList;

  std::wstring _fddFile;
  std::wstring _mimFile;
  std::wstring _federationExecution;

  unsigned _numServers;
  unsigned _numClientsPerServers;
  unsigned _numAmbassadorThreads;
  bool _disjointFederations;
  bool _joinOnce;
};

}

#endif
