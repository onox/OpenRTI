/* -*-c++-*- OpenRTI - Copyright (C) 2004-2011 Mathias Froehlich
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

#include "LogStream.h"

#ifndef _WIN32
// FIXME: abstract that thread local stuff and implement the same on win32
#include <pthread.h>
#endif

#include <cstdlib>
#include <iostream>
#include <sstream>
#include "Mutex.h"
#include "Referenced.h"
#include "ScopeLock.h"
#include "SharedPtr.h"

namespace OpenRTI {

#ifndef _WIN32

struct LogStream::AtomicStreamBuf : public std::stringbuf {
public:
  AtomicStreamBuf(std::ostream& stream) :
    _referencedMutex(ReferencedMutex::getReferencedMutex()),
    _stream(stream)
  { }

protected:
  virtual int sync()
  {
    ScopeLock scopeLock(_referencedMutex->_mutex);
    const char* base = pbase();
    _stream.write(base, pptr() - base);
    pubseekpos(0, std::ios_base::out);
    return 0;
  }

private:
  struct ReferencedMutex : public Referenced {
    Mutex _mutex;
    static SharedPtr<ReferencedMutex> getReferencedMutex()
    {
      static SharedPtr<ReferencedMutex> referencedMutex = new ReferencedMutex;
      return referencedMutex;
    }
  };

  SharedPtr<ReferencedMutex> _referencedMutex;
  std::ostream& _stream;
};

static pthread_key_t key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

struct LogStream::StreamPair {
  StreamPair() :
    _outBuf(std::cout),
    _errBuf(std::cerr),
    _outStream(&_outBuf),
    _errStream(&_errBuf)
  { }

  // FIXME: the destructor is not called for the main threads data.
  // Clean that up also
  static void destructor(void* data)
  {
    delete static_cast<StreamPair*>(data);
  }

  static void make_key()
  {
    pthread_key_create(&key, destructor);
  }

  static LogStream::StreamPair& getStreamPair()
  {
    pthread_once(&key_once, make_key);

    StreamPair* streamPair;
    streamPair = static_cast<StreamPair*>(pthread_getspecific(key));
    if (!streamPair) {
      streamPair = new StreamPair;
      pthread_setspecific(key, streamPair);
    }
    return *streamPair;
  }

  AtomicStreamBuf _outBuf;
  AtomicStreamBuf _errBuf;
  std::ostream _outStream;
  std::ostream _errStream;
};

#endif

static unsigned
atou(const char* s)
{
  if (!s)
    return 0u;

  std::stringstream strstream(s);
  unsigned value;
  strstream >> value;
  if (!strstream)
    return 0u;

  return value;
}

void
LogStream::setCategoryEnable(LogStream::Category category, bool enable)
{
  LogStream& logger = Instance();
  if (enable) {
    logger.mCategory |= category;
  } else {
    logger.mCategory &= ~category;
  }
}

void
LogStream::setCategoryDisable(LogStream::Category category)
{
  setCategoryEnable(category, false);
}

void
LogStream::setPriority(LogStream::Priority priority)
{
  LogStream& logger = Instance();
  logger.mPriority = priority;
}

LogStream&
LogStream::Instance(void)
{
  static LogStream logStreamInstance;
  return logStreamInstance;
}

std::ostream*
LogStream::getStream(Category category, LogStream::Priority priority)
{
  if (!getEnabled(category, priority))
    return 0;

#ifdef _WIN32
  if (Info <= priority)
    return &std::cout;
  else
    return &std::cerr;
#else
  if (Info <= priority)
    return &StreamPair::getStreamPair()._outStream;
  else
    return &StreamPair::getStreamPair()._errStream;
#endif
}

LogStream::LogStream() :
  mCategory(~0u),
  mPriority(Error)
{
  // Set some defaults from the environment
  unsigned value = atou(std::getenv("OPENRTI_DEBUG_PRIORITY"));
  if (value)
    mPriority = value;

  value = atou(std::getenv("OPENRTI_DEBUG_CATEGORY"));
  if (value)
    mCategory = value;
}

} // namespace OpenRTI
