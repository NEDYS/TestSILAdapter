// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TypeLib/DoProvidedMethod.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

void DoProvidedMethodBase::_Init(Cla::IDo* distributedObject, const std::string& path)
{
  DoMethod::_Init(distributedObject, path);
  mProvidedMethod = distributedObject->GetProvidedMethod(path);
}

void DoProvidedMethodBase::_Reset()
{
  DoMethod::_Reset();
  mProvidedMethod.reset();
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
