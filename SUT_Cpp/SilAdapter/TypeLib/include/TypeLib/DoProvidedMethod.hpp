// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "DoMethod.hpp"

#include "cla/IDoProvidedMethod.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Class representing a provided method member of a distributed object. */
class DoProvidedMethodBase : public DoMethod
{
protected:
  // Protected Methods
  // -------------------

  friend class Accessor; // To initialize / reset the member
  void _Init(Cla::IDo* distributedObject, const std::string& path) override;
  void _Reset() override;

  // Protected Member
  // -------------------

  std::shared_ptr<Cla::IDoProvidedMethod> mProvidedMethod = nullptr; ///< Handle to the provided method in the CLA
};

template <class T>
class DoProvidedMethod : public T
{
};

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
