// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Class representing an embedded member of a distributed object.
 *  \tparam Interface The interface of the embedded member. */
template <class Interface>
class EmbeddedMember : public Interface
{
public:
  /*! \brief Instantiates a new uninitialized embedded member. */
  EmbeddedMember() = default;
};

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
