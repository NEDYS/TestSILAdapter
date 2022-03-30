// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>

#include "cla/IDo.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

// Forward Declarations
class Accessor;

/*! \brief Class representing a method member of a distributed object. */
class DoMethod
{
protected:
  // Protected Methods
  // -------------------

  DoMethod() = default;
  virtual ~DoMethod() = default;

  friend class Accessor; // To initialize / reset the member

  /*! \brief Initializes the method member.
   *  \param distributedObject The distributed object this method member is part of.
   *  \param path The member path of this method member (not the full path). */
  virtual void _Init(Cla::IDo* distributedObject, const std::string& path);

  /*! \brief Resets the method member. The method member will no longer be valid. */
  virtual void _Reset();

  // Protected Member
  // -------------------

  std::string mPath; ///< The path of the parent, without the member path.
};

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
