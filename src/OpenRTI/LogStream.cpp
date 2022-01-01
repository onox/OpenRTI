/* -*-c++-*- OpenRTI - Copyright (C) 2004-2022 Mathias Froehlich
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

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include "Mutex.h"
#include "Referenced.h"
#include "ScopeLock.h"
#include "SharedPtr.h"
#include "SingletonPtr.h"
#include "StringUtils.h"
#include "ThreadLocal.h"

namespace OpenRTI {

struct OPENRTI_LOCAL LogStream::StreamPair {
  // The reason for that not just being a static is that
  // we can guarantee that the mutex only dies if its last
  // user has died. This is hard to guarantee if it's just
  // static in here.
  struct OPENRTI_LOCAL ReferencedMutex : public Referenced {
    Mutex _mutex;
  };
  static SharedPtr<ReferencedMutex> getReferencedMutex()
  {
    static SingletonPtr<ReferencedMutex> referencedMutex;
    return referencedMutex.get();
  }

  struct OPENRTI_LOCAL StreamBuf : public std::stringbuf {
  public:
    StreamBuf(std::ostream& stream) :
      _stream(stream),
      _referencedMutex(getReferencedMutex())
    { }
    virtual ~StreamBuf()
    {
      sync();
    }

  protected:
    virtual int sync()
    {
      // Not having the mutex here can only happen in weired circumstances.
      // But if so, do not hold back the message.
      if (_referencedMutex.valid()) {
        ScopeLock scopeLock(_referencedMutex->_mutex);
        return _syncUnlocked();
      } else {
        return _syncUnlocked();
      }
    }

    int _syncUnlocked()
    {
      const char* base = pbase();
      size_t count = pptr() - base;
      if (count <= 0)
        return 0;
      _stream.write(base, count);
      pubseekpos(0, std::ios_base::out);
      return 0;
    }

  private:
    std::ostream& _stream;
    SharedPtr<ReferencedMutex> _referencedMutex;
  };

  struct OPENRTI_LOCAL Stream {
    Stream(std::ostream& stream) :
      _streamBuf(stream),
      _stream(&_streamBuf)
    { }

    // The order of these fields is to make sure initialization
    // does not use an uninitialized streambuf.
    StreamBuf _streamBuf;
    std::ostream _stream;
  };

  StreamPair() :
    _outStream(std::cout),
    _errStream(std::cerr)
  { }

  static StreamPair* getStreamPair()
  {
    static ThreadLocal<StreamPair> streamPair;
    return streamPair.instance();
  }

  Stream _outStream;
  Stream _errStream;
};

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

  StreamPair* streamPair = StreamPair::getStreamPair();
  if (!streamPair)
    return 0;
  if (Info <= priority)
    return &streamPair->_outStream._stream;
  else
    return &streamPair->_errStream._stream;
}

LogStream::LogStream() :
  mCategory(~0u),
  mPriority(Error)
{
  // Set some defaults from the environment
  unsigned value = atou(std::getenv("OPENRTI_DEBUG_PRIORITY"));
  if (value)
    mPriority = value;

  const char* env = std::getenv("OPENRTI_DEBUG_CATEGORY");
  if (env) {
    std::vector<std::string> categories = split(env, ", |");
    if (!categories.empty()) {
      mCategory = 0;
      for (std::vector<std::string>::const_iterator i = categories.begin();
           i != categories.end(); ++i) {

        static const struct {
          const char* name;
          Category bit;
        } categories[] = {
#define CATEGORY(name) { #name, name }
          CATEGORY(Assert),
          CATEGORY(Network),
          CATEGORY(MessageCoding),
          CATEGORY(FederateAmbassador),
          CATEGORY(ServerConnect),
          CATEGORY(ServerFederation),
          CATEGORY(ServerFederate),
          CATEGORY(ServerSyncronization),
          CATEGORY(ServerTime),
          CATEGORY(ServerObjectInstance)
#undef CATEGORY
        };

        for (unsigned j = 0; j < sizeof(categories)/sizeof(categories[0]); ++j) {
          if (*i == categories[j].name)
            mCategory |= categories[j].bit;
        }
      }
    }
    // last resort, if nothing matched
    if (mCategory == 0) {
      mCategory = Category(atou(env));
    }
  }
}

} // namespace OpenRTI
