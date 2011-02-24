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

#ifndef OpenRTI_ThreadRegistry_h
#define OpenRTI_ThreadRegistry_h

#include <map>
#include <string>

#include "Condition.h"
#include "Exception.h"
#include "Mutex.h"
#include "Referenced.h"
#include "ScopeLock.h"
#include "ScopeUnlock.h"
#include "SharedPtr.h"
#include "Thread.h"
#include "WeakPtr.h"
#include "WeakReferenced.h"

namespace OpenRTI {

class OPENRTI_LOCAL ThreadRegistry : public WeakReferenced {
public:
  virtual ~ThreadRegistry() {}

  class NamedThread;

  // Abstract class that just does something with the thread.
  // That callback happens in the thread itself and thus does not need to be serialized
  // with the thread.
  class ThreadProcedureCallback : public Referenced {
  public:
    virtual ~ThreadProcedureCallback() {}
    virtual void exec(NamedThread& thread) = 0;
  };

  // The execution thread type that is managed with this registry
  // Note that we do not have any lock here.
  // From outside, all calls here are serialized by the registry.
  // In the thread, the outside triggered job is
  // executed from within the thread itself, though we don't
  // need locking for that.
  class NamedThread : public Thread {
  public:
    NamedThread(ThreadRegistry* registry, const std::wstring& name) :
      _registry(registry),
      _name(name),
      _done(false)
    {
    }

    virtual void run()
    {
      SharedPtr<ThreadRegistry> registry = _registry.lock();
      OpenRTIAssert(registry.valid());

      registry->registerThread(this);

      while (!getDone()) {
        // We get woken up on every change of the connection.
        // This just makes the dispatcher return from exec by setting
        // the dispatcher itself to 'done'. So when restarting exec,
        // make sure we set the dispatchers back to 'undone'.
        exec();

        // Just always when we get here, ask the parent to execute a callback.
        // In the non error case this is the only occation to get here
        registry->execCallback(*this);
      }

      registry->deregisterThread(*this);
    }

    virtual void wakeUp() = 0;
    virtual void exec() = 0;

    void stopThread()
    { _done = true; }

    const std::wstring& getName() const
    { return _name; }

  protected:
    bool getDone() const
    { return _done; }

  private:
    // The parent registry
    WeakPtr<ThreadRegistry> _registry;
    // The name this thread belongs to. Think of this as a connection or federation name
    // FIXME: hmm, anyway a template with a 'key_type' and a 'thread_type' ????
    const std::wstring _name;

    // Signals if we should continue
    bool _done;
  };

  virtual SharedPtr<NamedThread> createNewThread(const std::wstring& name) = 0;

  // connect/disconnect must go through the top level class.
  // That means connecting and disconnecting is serialized completely in an application.
  // That avoids a race between a thread going out of service but still connecting.
  // Since connect just happens on creat/destroy federation and on join and in the
  // destructor of an ambassador - which are all calls to the rti which should not have
  // realtime constraints, this is acceptable.
  //
  // But adding/removing connects must not block the server itself.
  // So the thread class just can have a lock protecting its internal datastructures.
  // In other words it is ok, if a connect/disconnect blocks the thread waiting in the ambassador
  // for a longer time until somebody can handle the request.
  // But a may be processing federation server must not block for longer time than just making sure its internals
  // are consistent. And it must not block until some of its users has disconnected completely.

  SharedPtr<NamedThread> getOrCreateThread(const std::wstring& name)
  {
    // Only one thread startup/shutdown at a time
    ScopeLock startupScopeLock(_startupMutex);

    // The mutex protecting the map and that is responsible for
    // setting up the wait condition
    ScopeLock scopeLock(_registerMutex);

    ThreadMap::iterator i = _threadMap.find(name);
    if (i != _threadMap.end())
      return i->second;

    // create, startup ...
    SharedPtr<NamedThread> thread = createNewThread(name);
    if (!thread.valid())
      return 0;
    thread->start();

    // FIXME add some timeout to catch problems with thread startup
    // we can not see here??

    // ... and wait until it registers itself
    while (_threadMap.find(thread->getName()) == _threadMap.end())
      _condition.wait(_registerMutex);

    return thread;
  }

  class ThreadStopCallback : public ThreadProcedureCallback {
  public:
    virtual void exec(NamedThread& thread)
    { thread.stopThread(); }
  };

  void destroyThread(const std::wstring& name)
  {
    // just call the rempte procedure to stop the thread
    execThreadProcedure(name, new ThreadStopCallback, false);
  }

  // Syncronous execute the callback in the thread.
  // return true when successful
  bool execThreadProcedure(const std::wstring& name, SharedPtr<ThreadProcedureCallback> callback, bool create)
  {
    // Fast exit and make sure that we rally have someting todo.
    if (!callback.valid())
      return false;

    // Only one thread startup/shutdown/procedure at a time
    ScopeLock startupScopeLock(_startupMutex);

    // The mutex protecting the map and that is responsible for
    // setting up the wait condition
    ScopeLock scopeLock(_registerMutex);

    SharedPtr<NamedThread> thread;
    ThreadMap::iterator i = _threadMap.find(name);
    if (i != _threadMap.end()) {
      thread = i->second;
    } else if (create) {
      // create, startup ...
      thread = createNewThread(name);
      if (!thread.valid())
        return false;

      thread->start();

      // FIXME add some timeout to catch problems with thread startup
      // we can not see here??

      // ... and wait until it registers itself
      while (_threadMap.find(thread->getName()) == _threadMap.end())
        _condition.wait(_registerMutex);
    } else {
      return false;
    }

    // Store that here for execution by the callback
    _threadProcedureCallback = callback;
    thread->wakeUp();

    // FIXME also timeout???

    // ... and wait until that has succeeded
    while (_threadProcedureCallback.valid())
      _condition.wait(_registerMutex);

    // Wait for any stopped thread
    if (_stoppedThread.valid()) {
      _stoppedThread->wait();
      _stoppedThread = 0;
    }

    return true;
  }

private:
  // all those are called from within the thread itself.
  void deregisterThread(NamedThread& thread)
  {
    // Hmm, don't know what is better, when we get here, the thread procedure call already has the lock
    ScopeLock scopeLock(_registerMutex);
    ThreadMap::iterator i = _threadMap.find(thread.getName());
    if (i == _threadMap.end())
      return;
    _stoppedThread = i->second;
    _threadMap.erase(i);
  }
  void registerThread(SharedPtr<NamedThread> thread)
  {
    ScopeLock scopeLock(_registerMutex);
    _threadMap.insert(ThreadMap::value_type(thread->getName(), thread));
    _condition.signal();
  }
  void execCallback(NamedThread& thread)
  {
    ScopeLock scopeLock(_registerMutex);
    SharedPtr<ThreadProcedureCallback> threadProcedureCallback;
    threadProcedureCallback.swap(_threadProcedureCallback);
    if (threadProcedureCallback.valid()) {
      ScopeUnlock scopeUnlock(_registerMutex);
      threadProcedureCallback->exec(thread);
    }
    _condition.signal();
  }

  // Mutex serializing thread startup to avoid two successive threads with the
  // same name starting up concurrently
  Mutex _startupMutex;

  // Mutex to protect the map and condition for started up threads
  Condition _condition;
  Mutex _registerMutex;
  typedef std::map<std::wstring, SharedPtr<NamedThread> > ThreadMap;
  ThreadMap _threadMap;
  // Is put there when we have something deregistered, so that we can wait for it
  SharedPtr<NamedThread> _stoppedThread;

  // Current thread callback.
  // Is also used to deactivate a thread.
  // At least therefore the _startupMutex must be held when doing
  // thread procedure calls.
  SharedPtr<ThreadProcedureCallback> _threadProcedureCallback;
};

} // namespace OpenRTI

#endif
