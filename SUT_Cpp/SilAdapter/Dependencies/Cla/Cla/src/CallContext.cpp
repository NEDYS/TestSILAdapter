// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CallContext.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// CallContext
// -------------------

int64_t CallContext::GetRequestId() const { return mRequestId; }

// Protected Methods
// -------------------

CallContext::CallContext(const std::shared_ptr<spdlog::logger>& logger, int64_t requestId) :
  mInputParameters(logger),
  mOutputParameters(logger),
  mRequestId(requestId)
{
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
