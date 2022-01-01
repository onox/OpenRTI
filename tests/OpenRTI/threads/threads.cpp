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
#include <iostream>
#include <sstream>

#include "Clock.h"
#include "Condition.h"
#include "Mutex.h"
#include "ScopeLock.h"
#include "ScopeUnlock.h"
#include "Thread.h"

namespace OpenRTI {

class ThreadTest : public Thread {
public:

  static bool exec()
  {
    ThreadTest testThread;
    if (!testThread.start())
      return false;
    testThread.wait();
    if (!testThread._threadWasRun)
      return false;
    return true;
  }

protected:
  ThreadTest() : _threadWasRun(false)
  { }
  virtual void run()
  { _threadWasRun = true; }
  bool _threadWasRun;
};

/// Long living thread that does never end. Make sure this does not crash
class DetachedTest : public Thread {
public:

  static bool exec()
  {
    SharedPtr<DetachedTest> testThread = new DetachedTest;
    if (!testThread->start())
      return false;
    return true;
  }

protected:
  virtual void run()
  {
    Clock::sleep_for(Clock::max());
  }
};

class AtomicTest : public Thread {
public:

  static bool exec()
  {
    Atomic counter;
    AtomicTest threads[4];
    for (unsigned i = 0; i < sizeof(threads)/sizeof(threads[0]); ++i)
      threads[i].start(&counter);
    for (unsigned i = 0; i < sizeof(threads)/sizeof(threads[0]); ++i)
      threads[i].wait();
    return counter == 0;
  }

protected:
  AtomicTest() :
    _counter(0)
  { }
  void start(Atomic* counter)
  {
    _counter = counter;
    Thread::start();
  }
  virtual void run()
  {
    for (unsigned i = 0; i < 1000000; ++i) {
      ++(*_counter);
      --(*_counter);
    }
  }
  Atomic* _counter;
};

class MutexTest : public Thread {
public:

  static bool exec()
  {
    LockedData lockedData;
    MutexTest threads[4];
    for (unsigned i = 0; i < sizeof(threads)/sizeof(threads[0]); ++i)
      threads[i].start(&lockedData);
    for (unsigned i = 0; i < sizeof(threads)/sizeof(threads[0]); ++i)
      threads[i].wait();
    return lockedData._count == lockedData._atomic;
  }

protected:
  struct LockedData {
    LockedData() : _count(0) {}
    void exec()
    {
      ++_atomic;
      ScopeLock scopeLock(_mutex);
      ++_count;
    }

    Atomic _atomic;
    Mutex _mutex;
    unsigned _count;
  };

  MutexTest() :
    _lockedData(0)
  { }
  void start(LockedData* lockedData)
  {
    _lockedData = lockedData;
    Thread::start();
  }
  virtual void run()
  {
    for (unsigned i = 0; i < 1000000; ++i)
      _lockedData->exec();
  }
  LockedData* _lockedData;
};


class ConditionTest : public Thread {
public:

  static bool exec()
  {
    ConditionData ping, pong;
    ConditionTest testThread(ping, pong);
    testThread.start();

    ping.notify_one();
    pong.wait();

    Clock start = Clock::now();
    for (unsigned i = 0; i < 10000; ++i) {
      ping.notify_one();
      pong.wait();
    }
    Clock stop = Clock::now();

    std::cout << "Average thread conditon latency is: " << (stop - start).getNSec()*1e-9/10000 << std::endl;

    testThread.wait();
    return true;
  }

protected:
  virtual void run()
  {
    _ping.wait();
    _pong.notify_one();
    for (unsigned i = 0; i < 10000; ++i) {
      _ping.wait();
      _pong.notify_one();
    }
  }

  struct ConditionData {
    ConditionData() : _signaled(false) {}
    void notify_one()
    {
      ScopeLock scopeLock(_mutex);
      _signaled = true;
      _condition.notify_one();
    }
    void wait()
    {
      ScopeLock scopeLock(_mutex);
      while (!_signaled)
        _condition.wait(scopeLock);
      _signaled = false;
    }

    Condition _condition;
    Mutex _mutex;
    bool _signaled;
  };

  ConditionTest(ConditionData& ping, ConditionData& pong) :
    _ping(ping),
    _pong(pong)
  { }

  ConditionData& _ping;
  ConditionData& _pong;
};

} // namespace OpenRTI

int
main(int argc, char* argv[])
{
  if (!OpenRTI::ThreadTest::exec()) {
    std::cerr << "ThreadTest failed!" << std::endl;
    return EXIT_FAILURE;
  }
  if (!OpenRTI::DetachedTest::exec()) {
    std::cerr << "DetachedTest failed!" << std::endl;
    return EXIT_FAILURE;
  }
  if (!OpenRTI::AtomicTest::exec()) {
    std::cerr << "AtomicTest failed!" << std::endl;
    return EXIT_FAILURE;
  }
  if (!OpenRTI::MutexTest::exec()) {
    std::cerr << "MutexTest failed!" << std::endl;
    return EXIT_FAILURE;
  }
  if (!OpenRTI::ConditionTest::exec()) {
    std::cerr << "ConditionTest failed!" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
