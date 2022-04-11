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

#ifndef OpenRTI_Handle_h
#define OpenRTI_Handle_h

#include "Export.h"
#include "Types.h"
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace OpenRTI {

template<typename T>
struct Hash;

template<typename T>
class OPENRTI_LOCAL Handle {
public:
  typedef T value_type;

  static T invalid()
  { return std::numeric_limits<T>::max(); }

  Handle() : _handle(invalid()) {}
  Handle(const Handle& handle) :
    _handle(handle._handle)
  { }
  Handle(const T& handle) :
    _handle(handle)
  { }

  bool valid() const
  { return _handle != invalid(); }

  operator const T&() const
  { return _handle; }

  const T& getHandle() const
  { return _handle; }

  Handle& swap(Handle& handle)
  { std::swap(_handle, handle._handle); return *this; }

  Handle& operator++()
  { ++_handle; return *this; }
  Handle operator++(int)
  { Handle handle(*this); ++_handle; return handle; }
  Handle& operator--()
  { --_handle; return *this; }
  Handle operator--(int)
  { Handle handle(*this); --_handle; return handle; }

  bool operator<(const Handle& handle) const
  { return _handle < handle._handle; }
  bool operator>(const Handle& handle) const
  { return _handle > handle._handle; }
  bool operator<=(const Handle& handle) const
  { return _handle <= handle._handle; }
  bool operator>=(const Handle& handle) const
  { return _handle >= handle._handle; }
  bool operator==(const Handle& handle) const
  { return _handle == handle._handle; }
  bool operator!=(const Handle& handle) const
  { return _handle != handle._handle; }

  std::size_t getHash() const
  { return std::size_t(_handle); }

  std::string getReservedName(const char* prefix) const
  {
    std::stringstream ss;
    ss << prefix << _handle;
    return ss.str();
  }

  unsigned getEncodedLength() const
  { return 8; }

  void encode(void* buffer) const
  {
    uint8_t* data = static_cast<uint8_t*>(buffer);
    uint64_t value = _handle;
    data[0] = uint8_t(value >> 56);
    data[1] = uint8_t(value >> 48);
    data[2] = uint8_t(value >> 40);
    data[3] = uint8_t(value >> 32);
    data[4] = uint8_t(value >> 24);
    data[5] = uint8_t(value >> 16);
    data[6] = uint8_t(value >> 8);
    data[7] = uint8_t(value);
  }
  void decode(const void* buffer)
  {
    const uint8_t* data = static_cast<const uint8_t*>(buffer);
    uint64_t value = uint64_t(data[0]) << 56;
    value |= uint64_t(data[1]) << 48;
    value |= uint64_t(data[2]) << 40;
    value |= uint64_t(data[3]) << 32;
    value |= uint64_t(data[4]) << 24;
    value |= uint64_t(data[5]) << 16;
    value |= uint64_t(data[6]) << 8;
    value |= uint64_t(data[7]);
    _handle = value_type(value);
  }

private:
  T _handle;
};

#define DECLARE_HANDLE_TYPE(HandleName, Type)                     \
class OPENRTI_API HandleName : public Handle<Type> {              \
public:                                                           \
  HandleName() {}                                                 \
  HandleName(const Handle<Type>& handle) :                        \
    Handle<Type>(handle)                                          \
  { }                                                             \
  template<typename T>                                            \
  HandleName(const T& handle) :                                   \
    Handle<Type>(Type(handle))                                    \
  { }                                                             \
                                                                  \
  std::string toString() const                                    \
  {                                                               \
    std::stringstream stream;                                     \
    stream << #HandleName "(" << getHandle() << ")";              \
    return stream.str();                                          \
  }                                                               \
};                                                                \
                                                                  \
template<>                                                        \
struct OPENRTI_LOCAL Hash<HandleName> {                           \
  std::size_t operator()(const HandleName& handle) const          \
  { return handle.getHash(); }                                    \
};                                                                \
                                                                  \
template<typename char_type, typename traits_type>                \
inline                                                            \
std::basic_ostream<char_type, traits_type>&                       \
operator<<(std::basic_ostream<char_type, traits_type>& os,        \
           const HandleName & handle)                             \
{                                                                 \
  os << #HandleName "(" << handle.getHandle() << ")";             \
  return os;                                                      \
}

DECLARE_HANDLE_TYPE(ObjectClassHandle, uint32_t)
DECLARE_HANDLE_TYPE(ObjectInstanceHandle, uint32_t)
DECLARE_HANDLE_TYPE(AttributeHandle, uint32_t)
DECLARE_HANDLE_TYPE(InteractionClassHandle, uint32_t)
DECLARE_HANDLE_TYPE(ParameterHandle, uint32_t)

DECLARE_HANDLE_TYPE(FederateHandle, uint32_t)
DECLARE_HANDLE_TYPE(FederationHandle, uint16_t)
DECLARE_HANDLE_TYPE(ModuleHandle, uint16_t)

DECLARE_HANDLE_TYPE(ConnectHandle, unsigned)

DECLARE_HANDLE_TYPE(TransportationHandle, uint8_t)
DECLARE_HANDLE_TYPE(OrderingHandle, uint8_t)

DECLARE_HANDLE_TYPE(DimensionHandle, uint32_t)
DECLARE_HANDLE_TYPE(SpaceHandle, uint32_t)
DECLARE_HANDLE_TYPE(UpdateRateHandle, uint32_t)

class OPENRTI_API MessageRetractionHandle : public Handle<uint64_t> {
public:
  MessageRetractionHandle() {}
  MessageRetractionHandle(const MessageRetractionHandle& handle) :
    Handle<uint64_t>(static_cast<const Handle<uint64_t>&>(handle))
  { }
  MessageRetractionHandle(uint64_t handle) :
    Handle<uint64_t>(handle)
  { }
  MessageRetractionHandle(const FederateHandle& handle, uint32_t serial) :
    Handle<uint64_t>(uint64_t(handle.getHandle()) << 32 | serial)
  { }

  FederateHandle getFederateHandle() const
  { return FederateHandle(uint32_t(getHandle() >> 32)); }
  uint32_t getSerial() const
  { return uint32_t(getHandle() & 0xffffffff); }

  std::string toString() const
  {
    std::stringstream stream;
    stream << "MessageRetractionHandle(" << getFederateHandle().getHandle() << "," << getSerial() << ")";
    return stream.str();
  }
};

template<>
struct OPENRTI_LOCAL Hash<MessageRetractionHandle> {
public:
  std::size_t operator()(const MessageRetractionHandle& handle) const
  { return handle.getHash(); }
};

// The regions are private to the creator, so prefix the regions with the federate handle,
// This way we can avoid tracking the region handles globally.
DECLARE_HANDLE_TYPE(LocalRegionHandle, uint32_t)
class OPENRTI_API RegionHandle : public Handle<uint64_t> {
public:
  RegionHandle() {}
  RegionHandle(const RegionHandle& handle) :
    Handle<uint64_t>(static_cast<const Handle<uint64_t>&>(handle))
  { }
  RegionHandle(uint64_t handle) :
    Handle<uint64_t>(handle)
  { }
  RegionHandle(const FederateHandle& handle, const LocalRegionHandle& localRegionHandle) :
    Handle<uint64_t>(uint64_t(handle.getHandle()) << 32 | localRegionHandle.getHandle())
  { }

  FederateHandle getFederateHandle() const
  { return FederateHandle(uint32_t(getHandle() >> 32)); }
  LocalRegionHandle getLocalRegionHandle() const
  { return LocalRegionHandle(uint32_t(getHandle() & 0xffffffff)); }

  std::string toString() const
  {
    std::stringstream stream;
    stream << "RegionHandle(" << getFederateHandle().getHandle() << "," << getLocalRegionHandle().getHandle() << ")";
    return stream.str();
  }
};

template<>
struct OPENRTI_LOCAL Hash<RegionHandle> {
public:
  std::size_t operator()(const RegionHandle& handle) const
  { return handle.getHash(); }
};

#undef DECLARE_HANDLE_TYPE

typedef std::set<ConnectHandle> ConnectHandleSet;
typedef std::set<AttributeHandle> AttributeHandleSet;
typedef std::set<DimensionHandle> DimensionHandleSet;
typedef std::set<ParameterHandle> ParameterHandleSet;
typedef std::set<RegionHandle> RegionHandleSet;
typedef std::set<FederateHandle> FederateHandleSet;
typedef std::set<ObjectInstanceHandle> ObjectInstanceHandleSet;

typedef std::vector<AttributeHandle> AttributeHandleVector;
typedef std::vector<DimensionHandle> DimensionHandleVector;
typedef std::vector<ParameterHandle> ParameterHandleVector;
typedef std::vector<RegionHandle> RegionHandleVector;
typedef std::vector<FederateHandle> FederateHandleVector;
typedef std::vector<ObjectInstanceHandle> ObjectInstanceHandleVector;

typedef std::pair<AttributeHandleVector, RegionHandleVector> AttributeHandleVectorRegionHandleVectorPair;
typedef std::vector<AttributeHandleVectorRegionHandleVectorPair> AttributeHandleVectorRegionHandleVectorPairVector;

} // namespace OpenRTI

#endif
