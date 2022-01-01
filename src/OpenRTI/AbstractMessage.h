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

#ifndef OpenRTI_AbstractMessage_h
#define OpenRTI_AbstractMessage_h

#include <iosfwd>
#include <list>
#include "Export.h"
#include "Handle.h"
#include "Referenced.h"

namespace OpenRTI {

class AbstractMessageDispatcher;

template<typename F>
class FunctorMessageDispatcher;
template<typename F>
class ConstFunctorMessageDispatcher;

template<typename T>
class SharedPtr;

class OPENRTI_API AbstractMessage : public Referenced {
public:
  virtual ~AbstractMessage();

  virtual const char* getTypeName() const = 0;
  virtual void out(std::ostream& os) const = 0;
  virtual void dispatch(const AbstractMessageDispatcher&) const = 0;

  // For testing of the transport implementation
  virtual bool operator==(const AbstractMessage&) const = 0;
  bool operator!=(const AbstractMessage& message) const
  { return ! operator==(message); }

  template<typename F>
  void dispatchFunctor(F& functor) const
  { dispatch(FunctorMessageDispatcher<F>(functor)); }
  template<typename F>
  void dispatchFunctor(const F& functor) const
  { dispatch(ConstFunctorMessageDispatcher<F>(functor)); }

  // Returns true if the message needs to be reliably sent or not.
  // The default implementation returns true. Interaction and attribute
  // update messages will provide a dynamic implementation for that.
  virtual bool getReliable() const;

  // Returns the object instance handle this message is targeting at.
  // The default implementation returns an invalid handle.
  // This is used to throw out messages for object instances that are already deleted.
  virtual ObjectInstanceHandle getObjectInstanceHandleForMessage() const;
};

inline std::ostream&
operator<<(std::ostream& os, const AbstractMessage& message)
{ message.out(os); return os; }

typedef std::list<SharedPtr<const AbstractMessage> > MessageList;

} // namespace OpenRTI

#endif
