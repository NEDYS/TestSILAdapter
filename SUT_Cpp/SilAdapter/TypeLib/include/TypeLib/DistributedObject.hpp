// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*!
 * \brief Class representing a concrete instantiation of an distributed object interface.
 * \tparam Interface The type of the interface of the distributed object.
 */
template <class Interface>
class DistributedObject : public Interface
{
public:
  /*! * \brief Instantiates a new uninitialized distributed object. */
  DistributedObject() = default;
};

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
