// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/ICallContext.hpp"

#include "ValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a call context for a method call. */
class CallContext : public ICallContext
{
public:
  // -------------------
  // CallContext
  // -------------------

  /*! \brief Returns the request ID of the call context.
   *  \returns the request ID of the call context. */
  int64_t GetRequestId() const;

protected:
  // Protected Members
  // -------------------

  ValueEntity mInputParameters;  ///< Buffer for the input parameters.
  ValueEntity mOutputParameters; ///< Buffer for the output parameters.

  // Protected Methods
  // -------------------

  /*! \brief Instantiates a new call context.
   *  \param logger The logger instance used by the call context and all child elements.
   *  \param requestId The request ID of the corresponding method call described by the call context. */
  CallContext(const std::shared_ptr<spdlog::logger>& logger, int64_t requestId);

private:
  // Private Members
  // -------------------

  int64_t mRequestId; ///< The request ID of the call context.
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
