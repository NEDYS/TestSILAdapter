// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ConsumerCallContext.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// ConsumerCallContext
// -------------------

ConsumerCallContext::ConsumerCallContext(const std::shared_ptr<spdlog::logger>& logger,
  int64_t requestId,
  std::function<void(ConsumerCallContext*, bool)> onCall) :
  CallContext(logger, requestId),
  mOnCall(std::move(onCall))
{
  // Fill serializer and input buffer with valid data
  auto ser = mInputParameters.GetSerializer();
  ser->BeginSerialization();
  ser->EndSerialization();
}

std::vector<uint8_t> ConsumerCallContext::GetInputParametersData() const { return mInputParameters.CopyData(); }

bool ConsumerCallContext::OnReturn(std::vector<uint8_t> responseData)
{
  auto expectedState = CallState::Called;
  if (mCallState.compare_exchange_strong(expectedState, CallState::Returned))
  {
    mOutputParameters.SetData(std::move(responseData));
    mCallback();
    return true;
  }
  return false;
}
bool ConsumerCallContext::OnTimeout()
{
  auto expectedState = CallState::Called;
  if (mCallState.compare_exchange_strong(expectedState, CallState::Discarded))
  {
    mCallback();
    return true;
  }
  return false;
}

// -------------------
// ICallContext
// -------------------

ISerializer* ConsumerCallContext::GetInputParametersSerializer() { return mInputParameters.GetSerializer(); }
IDeserializer* ConsumerCallContext::GetInputParametersDeserializer() { return mInputParameters.GetDeserializer(); }
ISerializer* ConsumerCallContext::GetOutputParametersSerializer()
{
  throw std::runtime_error("Output parameters of a consumed method cannot be written.");
}
IDeserializer* ConsumerCallContext::GetOutputParametersDeserializer() { return mOutputParameters.GetDeserializer(); }
CallState ConsumerCallContext::GetCallState() const { return mCallState; }

// ---------------------
// IConsumerCallContext
// ---------------------

void ConsumerCallContext::CallAsync(std::function<void()> callback)
{
  auto expectedState = CallState::Initial;
  if (mCallState.compare_exchange_strong(expectedState, CallState::Called))
  {
    mCallback = std::move(callback);
    const auto isOneWayCall = mCallback == nullptr;
    mOnCall(this, isOneWayCall);
    return;
  }

  throw std::runtime_error("Couldn't call 'CallAsync' on CallContext: Not in 'Initial' state.");
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
