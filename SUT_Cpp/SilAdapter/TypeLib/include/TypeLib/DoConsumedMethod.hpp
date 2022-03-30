// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "DoMethod.hpp"

#include "cla/IDoConsumedMethod.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Class representing a consumed method member of distributed object. */
class DoConsumedMethodBase : public DoMethod
{
protected:
  // Protected Methods
  // -------------------

  friend class Accessor; // To initialize / reset

  void _Init(Cla::IDo* distributedObject, const std::string& path) override;
  void _Reset() override;

  // Protected Member
  // -------------------

  std::shared_ptr<Cla::IDoConsumedMethod> mConsumedMethod = nullptr; ///< Handle to the consumed method in the CLA
};

template <class T>
class DoConsumedMethod : public T
{
};

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
