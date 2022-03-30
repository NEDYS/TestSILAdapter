
/* -*-c++-*- OpenRTI - Copyright (C) 2009-2016 Mathias Froehlich
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenRTI. If not, see <http://www.gnu.org/licenses/>.
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
#include <string.h>


namespace OpenRTI {

template<typename T>
struct Hash;

template<typename T>
class Handle
{
  public:
    typedef T value_type;
    static const size_t kFixedHandleSize = sizeof(uint64_t);
    static constexpr T invalid() noexcept
    {
      return std::numeric_limits<T>::max();
    }

    Handle() noexcept : _handle(invalid()) {}
    Handle(const Handle& handle) noexcept :
      _handle(handle._handle)
    { }
    Handle(Handle&& handle) noexcept
    {
      _handle=std::move(handle._handle);
    }
    Handle(T handleValue) noexcept :
      _handle(handleValue)
    { }
    ~Handle() noexcept = default;
    Handle& operator=(const Handle& ref) = default;
    Handle& operator=(Handle&& ref) = default;
    bool valid() const noexcept
    {
      return _handle != invalid();
    }

    operator const T& () const noexcept
    {
      return _handle;
    }

    const T& getHandle() const noexcept
    {
      return _handle;
    }

    Handle& swap(Handle& handle) noexcept
    {
      std::swap(_handle, handle._handle);
      return *this;
    }

    Handle& operator++() noexcept
    {
      ++_handle;
      return *this;
    }
    Handle operator++(int) noexcept
    {
      Handle handle(*this);
      ++_handle;
      return handle;
    }
    Handle& operator--() noexcept
    {
      --_handle;
      return *this;
    }
    Handle operator--(int) noexcept
    {
      Handle handle(*this);
      --_handle;
      return handle;
    }

    bool operator<(const Handle& handle) const noexcept
    {
      return _handle < handle._handle;
    }
    bool operator>(const Handle& handle) const noexcept
    {
      return _handle > handle._handle;
    }
    bool operator<=(const Handle& handle) const noexcept
    {
      return _handle <= handle._handle;
    }
    bool operator>=(const Handle& handle) const noexcept
    {
      return _handle >= handle._handle;
    }
    bool operator==(const Handle& handle) const noexcept
    {
      return _handle == handle._handle;
    }
    bool operator!=(const Handle& handle) const noexcept
    {
      return _handle != handle._handle;
    }

    std::size_t getHash() const noexcept
    {
      return std::size_t(_handle);
    }

    std::string getReservedName(const char* prefix) const
    {
      std::stringstream ss;
      ss << prefix << _handle;
      return ss.str();
    }

    static unsigned getEncodedLength() noexcept
    {
      return 8;
    }

    void encode(void* buffer) const noexcept
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
    void decode(const void* buffer) noexcept
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

template<typename T> struct Hash<Handle<T>>
{
  std::size_t operator()(const Handle<T>& handle) const noexcept
  {
    return handle.getHash();
  }
};


class ObjectClassHandle : public Handle<uint32_t>
{
  public:
    ObjectClassHandle() noexcept = default;
    ObjectClassHandle(const ObjectClassHandle& handle) noexcept : Handle<uint32_t>(handle) { } 
    ObjectClassHandle(ObjectClassHandle&& handle) noexcept = default;
    ~ObjectClassHandle() noexcept = default;
    ObjectClassHandle& operator=(const ObjectClassHandle&) = default;
    ObjectClassHandle& operator=(ObjectClassHandle&&) = default;
    
    ObjectClassHandle(uint32_t handle) noexcept : Handle<uint32_t>(handle) { } 

    std::string toString() const
    {
      std::stringstream stream;
      stream << "ObjectClassHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const ObjectClassHandle& handle)
{
  os << "ObjectClassHandle" "(" << handle.getHandle() << ")";
  return os;
}

class ObjectInstanceHandle : public Handle<uint32_t>
{
  public:
    ObjectInstanceHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    // these may probably all be defaulted
    ObjectInstanceHandle() noexcept = default;
    ObjectInstanceHandle(const ObjectInstanceHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    ObjectInstanceHandle(ObjectInstanceHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    ~ObjectInstanceHandle() noexcept = default;
    ObjectInstanceHandle& operator=(const ObjectInstanceHandle&) = default;
    ObjectInstanceHandle& operator=(ObjectInstanceHandle&&) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "ObjectInstanceHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const ObjectInstanceHandle& handle)
{
  os << "ObjectInstanceHandle" "(" << handle.getHandle() << ")";
  return os;
}

class AttributeHandle : public Handle<uint32_t>
{
  public:
    AttributeHandle() noexcept = default;
    AttributeHandle(const AttributeHandle& handle) noexcept : Handle<uint32_t>(handle) { } 
    AttributeHandle(AttributeHandle&& handle) noexcept = default;
    AttributeHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { } 
    ~AttributeHandle() noexcept = default;
    AttributeHandle& operator=(const AttributeHandle&) = default;
    AttributeHandle& operator=(AttributeHandle&&) = default;

    std::string toString() const
    {
      std::stringstream stream;
      stream << "AttributeHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const AttributeHandle& handle)
{
  os << "AttributeHandle" "(" << handle.getHandle() << ")";
  return os;
}

class InteractionClassHandle : public Handle<uint32_t>
{
  public:
    InteractionClassHandle() noexcept = default;
    InteractionClassHandle(const InteractionClassHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    InteractionClassHandle(InteractionClassHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    InteractionClassHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~InteractionClassHandle() noexcept = default;
    InteractionClassHandle& operator=(const InteractionClassHandle& handle) = default;
    InteractionClassHandle& operator=(InteractionClassHandle&& handle) = default;

    std::string toString() const
    {
      std::stringstream stream;
      stream << "InteractionClassHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const InteractionClassHandle& handle)
{
  os << "InteractionClassHandle" "(" << handle.getHandle() << ")";
  return os;
}

class ParameterHandle : public Handle<uint32_t>
{
  public:
    ParameterHandle() noexcept = default;
    ParameterHandle(const ParameterHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    ParameterHandle(ParameterHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    ParameterHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~ParameterHandle() noexcept = default;
    ParameterHandle& operator=(const ParameterHandle& handle) = default;
    ParameterHandle& operator=(ParameterHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "ParameterHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const ParameterHandle& handle)
{
  os << "ParameterHandle" "(" << handle.getHandle() << ")";
  return os;
}

class FederateHandle : public Handle<uint32_t>
{
  public:
    FederateHandle() noexcept = default;
    FederateHandle(const FederateHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    FederateHandle(FederateHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    FederateHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~FederateHandle() noexcept = default;
    FederateHandle& operator=(const FederateHandle& handle) = default;
    FederateHandle& operator=(FederateHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "FederateHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const FederateHandle& handle)
{
  os << "FederateHandle" "(" << handle.getHandle() << ")";
  return os;
}

class FederationHandle : public Handle<uint16_t>
{
  public:
    FederationHandle() noexcept = default;
    FederationHandle(const FederationHandle& handle) noexcept : Handle<uint16_t>(handle) { } 
    FederationHandle(FederationHandle&& handle) noexcept = default;
    FederationHandle(const uint16_t& handle) noexcept : Handle<uint16_t>(handle) { } 
    ~FederationHandle() noexcept = default;
    FederationHandle& operator=(const FederationHandle& handle) = default;
    FederationHandle& operator=(FederationHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "FederationHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const FederationHandle& handle)
{
  os << "FederationHandle" "(" << handle.getHandle() << ")";
  return os;
}

class ModuleHandle : public Handle<uint16_t>
{
  public:
    ModuleHandle() noexcept = default;
    ModuleHandle(const ModuleHandle& handle) noexcept : Handle<uint16_t>(handle) { } 
    ModuleHandle(ModuleHandle&& handle) noexcept = default;
    ModuleHandle(const uint16_t& handle) noexcept : Handle<uint16_t>(handle) { } 
    ~ModuleHandle() noexcept = default;
    ModuleHandle& operator=(const ModuleHandle& handle) = default;
    ModuleHandle& operator=(ModuleHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "ModuleHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const ModuleHandle& handle)
{
  os << "ModuleHandle" "(" << handle.getHandle() << ")";
  return os;
}

class ConnectHandle : public Handle<unsigned>
{
  public:
    ConnectHandle() noexcept = default;
    ConnectHandle(const ConnectHandle& handle) noexcept : Handle<unsigned>(handle) { }
    ConnectHandle(ConnectHandle&& handle) noexcept : Handle<unsigned>(handle) { }
    ConnectHandle(const unsigned& handle) noexcept : Handle<unsigned>(handle) { }
    ~ConnectHandle() noexcept = default;
    ConnectHandle& operator=(const ConnectHandle& handle) = default;
    ConnectHandle& operator=(ConnectHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "ConnectHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const ConnectHandle& handle)
{
  os << "ConnectHandle" "(" << handle.getHandle() << ")";
  return os;
}

class TransportationHandle : public Handle<uint8_t>
{
  public:
    TransportationHandle() noexcept = default;
    TransportationHandle(const TransportationHandle& handle) noexcept : Handle<uint8_t>(handle) { }
    TransportationHandle(TransportationHandle&& handle) noexcept : Handle<uint8_t>(handle) { }
    TransportationHandle(const uint8_t& handle) noexcept : Handle<uint8_t>(handle) { }
    ~TransportationHandle() noexcept = default;
    TransportationHandle& operator=(const TransportationHandle& handle) = default;
    TransportationHandle& operator=(TransportationHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "TransportationHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const TransportationHandle& handle)
{
  os << "TransportationHandle" "(" << handle.getHandle() << ")";
  return os;
}

class OrderingHandle : public Handle<uint8_t>
{
  public:
    OrderingHandle() noexcept = default;
    OrderingHandle(const OrderingHandle& handle) noexcept : Handle<uint8_t>(handle) { }
    OrderingHandle(OrderingHandle&& handle) noexcept : Handle<uint8_t>(handle) { }
    OrderingHandle(const uint8_t& handle) noexcept : Handle<uint8_t>(handle) { }
    ~OrderingHandle() noexcept = default;
    OrderingHandle& operator=(const OrderingHandle& handle) = default;
    OrderingHandle& operator=(OrderingHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "OrderingHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const OrderingHandle& handle)
{
  os << "OrderingHandle" "(" << handle.getHandle() << ")";
  return os;
}

class DimensionHandle : public Handle<uint32_t>
{
  public:
    DimensionHandle() noexcept = default;
    DimensionHandle(const DimensionHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    DimensionHandle(DimensionHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    DimensionHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~DimensionHandle() noexcept = default;
    DimensionHandle& operator=(const DimensionHandle& handle) = default;
    DimensionHandle& operator=(DimensionHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "DimensionHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const DimensionHandle& handle)
{
  os << "DimensionHandle" "(" << handle.getHandle() << ")";
  return os;
}

class SpaceHandle : public Handle<uint32_t>
{
  public:
    SpaceHandle() noexcept = default;
    SpaceHandle(const SpaceHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    SpaceHandle(SpaceHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    SpaceHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~SpaceHandle() noexcept = default;
    SpaceHandle& operator=(const SpaceHandle& handle) = default;
    SpaceHandle& operator=(SpaceHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "SpaceHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const SpaceHandle& handle)
{
  os << "SpaceHandle" "(" << handle.getHandle() << ")";
  return os;
}

class UpdateRateHandle : public Handle<uint32_t>
{
  public:
    UpdateRateHandle() noexcept = default;
    UpdateRateHandle(const UpdateRateHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    UpdateRateHandle(UpdateRateHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    UpdateRateHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~UpdateRateHandle() noexcept = default;
    UpdateRateHandle& operator=(const UpdateRateHandle& handle) = default;
    UpdateRateHandle& operator=(UpdateRateHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "UpdateRateHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const UpdateRateHandle& handle)
{
  os << "UpdateRateHandle" "(" << handle.getHandle() << ")";
  return os;
}

class BasicDataTypeHandle : public Handle<uint32_t>
{
  public:
    BasicDataTypeHandle() noexcept = default;
    BasicDataTypeHandle(const BasicDataTypeHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    BasicDataTypeHandle(BasicDataTypeHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    BasicDataTypeHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~BasicDataTypeHandle() noexcept = default;
    BasicDataTypeHandle& operator=(const BasicDataTypeHandle& handle) = default;
    BasicDataTypeHandle& operator=(BasicDataTypeHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "BasicDataTypeHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const BasicDataTypeHandle& handle)
{
  os << "BasicDataTypeHandle" "(" << handle.getHandle() << ")";
  return os;
}

class SimpleDataTypeHandle : public Handle<uint32_t>
{
  public:
    SimpleDataTypeHandle() noexcept = default;
    SimpleDataTypeHandle(const SimpleDataTypeHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    SimpleDataTypeHandle(SimpleDataTypeHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    SimpleDataTypeHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~SimpleDataTypeHandle() noexcept = default;
    SimpleDataTypeHandle& operator=(const SimpleDataTypeHandle& handle) = default;
    SimpleDataTypeHandle& operator=(SimpleDataTypeHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "SimpleDataTypeHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const SimpleDataTypeHandle& handle)
{
  os << "SimpleDataTypeHandle" "(" << handle.getHandle() << ")";
  return os;
}

class EnumeratedDataTypeHandle : public Handle<uint32_t>
{
  public:
    EnumeratedDataTypeHandle() noexcept = default;
    EnumeratedDataTypeHandle(const EnumeratedDataTypeHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    EnumeratedDataTypeHandle(EnumeratedDataTypeHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    EnumeratedDataTypeHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~EnumeratedDataTypeHandle() noexcept = default;
    EnumeratedDataTypeHandle& operator=(const EnumeratedDataTypeHandle& handle) = default;
    EnumeratedDataTypeHandle& operator=(EnumeratedDataTypeHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "EnumeratedDataTypeHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const EnumeratedDataTypeHandle& handle)
{
  os << "EnumeratedDataTypeHandle" "(" << handle.getHandle() << ")";
  return os;
}

class ArrayDataTypeHandle : public Handle<uint32_t>
{
  public:
    ArrayDataTypeHandle() noexcept = default;
    ArrayDataTypeHandle(const ArrayDataTypeHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    ArrayDataTypeHandle(ArrayDataTypeHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    ArrayDataTypeHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~ArrayDataTypeHandle() noexcept = default;
    ArrayDataTypeHandle& operator=(const ArrayDataTypeHandle& handle) = default;
    ArrayDataTypeHandle& operator=(ArrayDataTypeHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "ArrayDataTypeHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const ArrayDataTypeHandle& handle)
{
  os << "ArrayDataTypeHandle" "(" << handle.getHandle() << ")";
  return os;
}

class FixedRecordDataTypeHandle : public Handle<uint32_t>
{
  public:
    FixedRecordDataTypeHandle() noexcept = default;
    FixedRecordDataTypeHandle(const FixedRecordDataTypeHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    FixedRecordDataTypeHandle(FixedRecordDataTypeHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    FixedRecordDataTypeHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~FixedRecordDataTypeHandle() noexcept = default;
    FixedRecordDataTypeHandle& operator=(const FixedRecordDataTypeHandle& handle) = default;
    FixedRecordDataTypeHandle& operator=(FixedRecordDataTypeHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "FixedRecordDataTypeHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const FixedRecordDataTypeHandle& handle)
{
  os << "FixedRecordDataTypeHandle" "(" << handle.getHandle() << ")";
  return os;
}

class VariantRecordDataTypeHandle : public Handle<uint32_t>
{
  public:
    VariantRecordDataTypeHandle() noexcept = default;
    VariantRecordDataTypeHandle(const VariantRecordDataTypeHandle& handle) noexcept : Handle<uint32_t>(handle) { }
    VariantRecordDataTypeHandle(VariantRecordDataTypeHandle&& handle) noexcept : Handle<uint32_t>(handle) { }
    VariantRecordDataTypeHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { }
    ~VariantRecordDataTypeHandle() noexcept = default;
    VariantRecordDataTypeHandle& operator=(const VariantRecordDataTypeHandle& handle) = default;
    VariantRecordDataTypeHandle& operator=(VariantRecordDataTypeHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "VariantRecordDataTypeHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const VariantRecordDataTypeHandle& handle)
{
  os << "VariantRecordDataTypeHandle" "(" << handle.getHandle() << ")";
  return os;
}

class MessageRetractionHandle : public Handle<uint64_t>
{
  public:
    MessageRetractionHandle() noexcept = default;
    MessageRetractionHandle(const MessageRetractionHandle& handle) noexcept : Handle<uint64_t>(handle) { }
    MessageRetractionHandle(MessageRetractionHandle&& handle) noexcept : Handle<uint64_t>(handle) { }
    MessageRetractionHandle(uint64_t handle) noexcept :
      Handle<uint64_t>(handle)
    { }
    MessageRetractionHandle(const FederateHandle& handle, uint32_t serial) noexcept :
      Handle<uint64_t>(uint64_t{handle.getHandle()} << 32 | serial)
    { }
    ~MessageRetractionHandle() noexcept = default;
    MessageRetractionHandle& operator=(const MessageRetractionHandle& handle) = default;
    MessageRetractionHandle& operator=(MessageRetractionHandle&& handle) = default;

    FederateHandle getFederateHandle() const noexcept
    {
      return FederateHandle(static_cast<uint32_t>(getHandle() >> 32));
    }
    uint32_t getSerial() const noexcept
    {
      return static_cast<uint32_t>(getHandle() & 0xffffffff);
    }

    std::string toString() const
    {
      std::stringstream stream;
      stream << "MessageRetractionHandle(" << getFederateHandle().getHandle() << "," << getSerial() << ")";
      return stream.str();
    }
};

// The regions are private to the creator, so prefix the regions with the federate handle,
// This way we can avoid tracking the region handles globally.
class LocalRegionHandle : public Handle<uint32_t>
{
  public:
    LocalRegionHandle() noexcept {} 
    LocalRegionHandle(const LocalRegionHandle& handle) noexcept : Handle<uint32_t>(handle) { } 
    LocalRegionHandle(LocalRegionHandle&& handle) noexcept = default;
    LocalRegionHandle(const uint32_t& handle) noexcept : Handle<uint32_t>(handle) { } 
    ~LocalRegionHandle() noexcept = default;
    LocalRegionHandle& operator=(const LocalRegionHandle& handle) = default;
    LocalRegionHandle& operator=(LocalRegionHandle&& handle) = default;
    std::string toString() const
    {
      std::stringstream stream;
      stream << "LocalRegionHandle" "(" << getHandle() << ")";
      return stream.str();
    }
};

template<typename char_type, typename traits_type> 
inline std::basic_ostream<char_type, traits_type>& operator<<(std::basic_ostream<char_type, traits_type>& os, const LocalRegionHandle& handle)
{
  os << "LocalRegionHandle" "(" << handle.getHandle() << ")";
  return os;
}
class RegionHandle : public Handle<uint64_t>
{
  public:
    RegionHandle() noexcept = default;
    RegionHandle(const RegionHandle& handle) noexcept : Handle<uint64_t>(handle) { }
    RegionHandle(RegionHandle&& handle) noexcept = default;
    RegionHandle(uint64_t handle) noexcept :
      Handle<uint64_t>(handle)
    { }
    RegionHandle(const FederateHandle& handle, const LocalRegionHandle& localRegionHandle) noexcept :
      Handle<uint64_t>(uint64_t(handle.getHandle()) << 32 | localRegionHandle.getHandle())
    { }
    ~RegionHandle() noexcept = default;
    RegionHandle& operator=(const RegionHandle& handle) = default;
    RegionHandle& operator=(RegionHandle&& handle) = default;

    FederateHandle getFederateHandle() const noexcept
    {
      return FederateHandle(static_cast<uint32_t>(getHandle() >> 32));
    }

    LocalRegionHandle getLocalRegionHandle() const noexcept
    {
      return LocalRegionHandle(static_cast<uint32_t>(getHandle() & 0xffffffff));
    }

    std::string toString() const
    {
      std::stringstream stream;
      stream << "RegionHandle(" << getFederateHandle().getHandle() << "," << getLocalRegionHandle().getHandle() << ")";
      return stream.str();
    }
};

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