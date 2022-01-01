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

#include "ThreadLocal.h"

#include <vector>
#include <pthread.h>

namespace OpenRTI {

static pthread_key_t key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

struct OPENRTI_LOCAL AbstractThreadLocal::_Provider {
  typedef std::vector<AbstractThreadLocal::_AbstractData*> ThreadLocalVector;

  static void destructor(void* data);
  static void make_key();
  static _Provider& instance();

  _Provider();
  ~_Provider();
  unsigned getNextIndex();

  _AbstractData* getData(unsigned index);
  void setData(unsigned index, _AbstractData* abstractThreadLocal);

  ThreadLocalVector& _tlsVector();

  unsigned _index;
};

void
AbstractThreadLocal::_Provider::destructor(void* data)
{
  if (!data)
    return;
  ThreadLocalVector* tlsVector = static_cast<ThreadLocalVector*>(data);
  for (ThreadLocalVector::iterator i = tlsVector->begin();
       i != tlsVector->end(); ++i)
    delete *i;
  delete tlsVector;
}

void
AbstractThreadLocal::_Provider::make_key()
{
  pthread_key_create(&key, destructor);
}

AbstractThreadLocal::_Provider&
AbstractThreadLocal::_Provider::instance()
{
  static _Provider provider;
  return provider;
}

AbstractThreadLocal::_Provider::_Provider() :
  _index(0)
{
}

AbstractThreadLocal::_Provider::~_Provider()
{
  pthread_once(&key_once, make_key);
  destructor(pthread_getspecific(key));
  pthread_setspecific(key, NULL);
}

unsigned
AbstractThreadLocal::_Provider::getNextIndex()
{
  return _index++;
}

AbstractThreadLocal::_AbstractData*
AbstractThreadLocal::_Provider::getData(unsigned index)
{
  ThreadLocalVector& tlsVector = _tlsVector();
  if (tlsVector.size() <= index)
    return 0;
  return tlsVector[index];
}

void
AbstractThreadLocal::_Provider::setData(unsigned index, AbstractThreadLocal::_AbstractData* abstractThreadLocal)
{
  ThreadLocalVector& tlsVector = _tlsVector();
  if (tlsVector.size() <= index)
    tlsVector.resize(index + 1, 0);
  delete tlsVector[index];
  tlsVector[index] = abstractThreadLocal;
}

AbstractThreadLocal::_Provider::ThreadLocalVector&
AbstractThreadLocal::_Provider::_tlsVector()
{
  pthread_once(&key_once, make_key);

  ThreadLocalVector* tlsVector;
  tlsVector = static_cast<ThreadLocalVector*>(pthread_getspecific(key));
  if (tlsVector)
    return *tlsVector;

  tlsVector = new ThreadLocalVector;
  pthread_setspecific(key, tlsVector);
  return *tlsVector;
}

AbstractThreadLocal::_AbstractData::~_AbstractData()
{
}

AbstractThreadLocal::AbstractThreadLocal() :
  _index(_Provider::instance().getNextIndex())
{
}

AbstractThreadLocal::~AbstractThreadLocal()
{
}


AbstractThreadLocal::_AbstractData*
AbstractThreadLocal::_get()
{
  return _Provider::instance().getData(_index);
}

void
AbstractThreadLocal::_set(AbstractThreadLocal::_AbstractData* abstractThreadLocal)
{
  return _Provider::instance().setData(_index, abstractThreadLocal);
}

} // namespace OpenRTI
