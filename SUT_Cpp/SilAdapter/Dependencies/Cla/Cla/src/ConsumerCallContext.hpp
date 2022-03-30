// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IConsumerCallContext.hpp"

#include "CallContext.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a consumer call context, which can be used to
 *         to execute the corresponding method call. */
class ConsumerCallContext : public CallContext, public IConsumerCallContext
{
public:
  // -------------------
  // ConsumerCallContext
  // -------------------

  /*! \brief Instantiates a new consumer call context.
   *  \param logger The logger instance used for logging by the call context and all child elements.
   *  \param requestId The request ID of the method call corresponding to this call context.
   *  \param onCall The callback when 'CallAsync' is called. Returns the instantiated call context and
   *                whether the call is a one-way-call or not. */
  ConsumerCallContext(const std::shared_ptr<spdlog::logger>& logger,
    int64_t requestId,
    std::function<void(ConsumerCallContext* callContext, bool isOneWayCall)> onCall);

  /*! \brief Gets the data buffer of the input parameters.
   *  \returns the data buffer of the input parameters. */
  std::vector<uint8_t> GetInputParametersData() const;

  /*! \brief Is called when the call via `CallAsync` returns with a response. Transitions
   *         the call context from `CallState::Called` to `CallState::Returned`.
   *  \param responseData The data of the returned method call response.
   *  \returns `false` if the call context was not in `CallState::Called`. */
  bool OnReturn(std::vector<uint8_t> responseData);

  /*! \brief Is called when the call via `CallAsync` did not return and a timeout occurs.
   *         Transitions the call context from `CallState::Called` to `CallState::Discarded`.
   *  \returns `false` if the call context was not in `CallState::Called`. */
  bool OnTimeout();

  // -------------------
  // ICallContext
  // -------------------

  ISerializer* GetInputParametersSerializer() override;
  IDeserializer* GetInputParametersDeserializer() override;
  ISerializer* GetOutputParametersSerializer() override;
  IDeserializer* GetOutputParametersDeserializer() override;
  CallState GetCallState() const override;

  // ---------------------
  // IConsumerCallContext
  // ---------------------

  void CallAsync(std::function<void()> callback) override;

private:
  // Private member
  // -------------------

  std::atomic<CallState> mCallState{CallState::Initial};
  std::function<void()> mCallback{nullptr};
  std::function<void(ConsumerCallContext*, bool)> mOnCall;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
