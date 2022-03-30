// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <atomic>
#include <unordered_map>

#include "cla/IDoConsumedMethod.hpp"

#include "ConsumerCallContext.hpp"
#include "DoMethod.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a consumed method which can be called by
 *         creating a consumer call context. */
class DoConsumedMethod : public DoMethod,
                         public IDoConsumedMethod,
                         public std::enable_shared_from_this<DoConsumedMethod>
{
public:
  // Public methods
  // -------------------

  /*! \brief Instantiates a new consumed method.
   *  \param fullPath The full path of the consumed method, i.e. 'Namespace::Do.MethodName'.
   *  \param logger The logger which is used for logging of this method
   *                and all its child elements, e.g. created call contexts. */
  DoConsumedMethod(std::string fullPath, std::shared_ptr<spdlog::logger> logger);

  // -------------------
  // IDoMember
  // -------------------

  MemberType GetType() const override;
  MemberDirection GetDirection() const override;

  // -------------------
  // IDoConsumedMethod
  // -------------------

  std::shared_ptr<IConsumerCallContext> CreateCallContext() override;

protected:
  // Protected methods
  // -------------------

  /*! \brief Is called when a response is received for the method.
   *  \param header The header of the response.
   *  \param payload The payload of the response.
   *  \returns `true` if the response was assigned to a call context and processed, otherwise `false`. */
  virtual bool OnReceiveResponse(const Sab::MethodCallHeader& header, const std::vector<uint8_t>& payload);

private:
  // Private members
  // -------------------

  std::atomic<int64_t> mNextRequestId{1};

  std::mutex mPendingCallContextsMutex;
  std::unordered_map<int64_t, std::weak_ptr<ConsumerCallContext>> mPendingCallContexts;

  // Private method
  // -------------------

  /*! \brief Creates a unique request ID for this method.
   *  \returns a unique request ID. */
  int64_t CreateUniqueRequestId();

  /*! \brief Removes a pending call context for a specific request ID.
   *  \param requestId The request ID of the call context which shall be removed.
   *  \returns the removed call context. `empty` if no call context was removed. */
  std::weak_ptr<ConsumerCallContext> PopCallContext(int64_t requestId);

  int32_t OnReceiveData(const std::vector<uint8_t>& data, const int64_t& timestamp) override;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
