// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TypeLib/DoConsumedMethod.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

void DoConsumedMethodBase::_Init(Cla::IDo* distributedObject, const std::string& path)
{
  DoMethod::_Init(distributedObject, path);
  mConsumedMethod = distributedObject->GetConsumedMethod(path);
}

void DoConsumedMethodBase::_Reset()
{
  DoMethod::_Reset();
  mConsumedMethod.reset();
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
