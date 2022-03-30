// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoConsumedMethod.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// DoConsumedMethod
// -------------------

DoConsumedMethod::DoConsumedMethod(std::string fullPath, std::shared_ptr<spdlog::logger> logger) :
  DoMethod(std::move(fullPath), MemberDirection::Consumed, std::move(logger))
{
}

// -------------------
// IDoMember
// -------------------

MemberDirection DoConsumedMethod::GetDirection() const { return DoMember::GetDirection(); }
MemberType DoConsumedMethod::GetType() const { return DoMember::GetType(); }

// -------------------
// IDoConsumedMethod
// -------------------

std::shared_ptr<IConsumerCallContext> DoConsumedMethod::CreateCallContext()
{
  const auto requestId = CreateUniqueRequestId();

  // Pass a weak ptr of this method to the call context lambdas, since
  // the call context could be kept alive longer than the method itself.
  std::weak_ptr<DoConsumedMethod> thisWeak = shared_from_this();

  const auto onCall = [thisWeak](ConsumerCallContext* cc, bool isOneWayCall) {
    const auto thisShared = thisWeak.lock();
    if (!thisShared)
    {
      return;
    }

    const auto type = isOneWayCall ? Sab::MethodCallMessageType::RequestNoReturn : Sab::MethodCallMessageType::Request;
    const Sab::MethodCallHeader requestHeader(cc->GetRequestId(), type, Sab::MethodCallReturnCode::NoError);

    Sab::Serializer serializer;
    serializer.Serialize(requestHeader);
    serializer.AppendBuffer(cc->GetInputParametersData());

    if (isOneWayCall)
    {
      // Remove from container, since the call context given by
      // CreateCallContext could be kept alive by the user.
      thisShared->mPendingCallContexts.erase(cc->GetRequestId());
    }

    thisShared->SendData(serializer.ReleaseBuffer());
  };

  const auto onDestroy = [thisWeak](ConsumerCallContext* cc) {
    if (const auto thisShared = thisWeak.lock())
    {
      thisShared->PopCallContext(cc->GetRequestId());
    }

    delete cc;
  };

  auto callContext =
    std::shared_ptr<ConsumerCallContext>(new ConsumerCallContext(mLogger, requestId, onCall), onDestroy);

  std::lock_guard<std::mutex> lock(mPendingCallContextsMutex);
  mPendingCallContexts[requestId] = callContext;
  return callContext;
}

// Protected methods
// -------------------

bool DoConsumedMethod::OnReceiveResponse(const Sab::MethodCallHeader& header, const std::vector<uint8_t>& payload)
{
  const auto callContext = PopCallContext(header.GetRequestId()).lock();
  if (callContext)
  {
    return callContext->OnReturn(payload);
  }

  mLogger->warn("Received method response for non-existing call context with request ID {}.", header.GetRequestId());
  return false;
}

// Private methods
// -------------------

int64_t DoConsumedMethod::CreateUniqueRequestId() { return mNextRequestId++; }

std::weak_ptr<ConsumerCallContext> DoConsumedMethod::PopCallContext(int64_t requestId)
{
  std::lock_guard<std::mutex> lock(mPendingCallContextsMutex);
  const auto it = mPendingCallContexts.find(requestId);
  if (it != mPendingCallContexts.end())
  {
    const auto ptr = it->second.lock();
    mPendingCallContexts.erase(it);
    return ptr;
  }

  return {};
}

int32_t DoConsumedMethod::OnReceiveData(const std::vector<uint8_t>& data, const int64_t&)
{
  Sab::Deserializer deserializer(data);
  const auto responseHeader = deserializer.Deserialize<Sab::MethodCallHeader>();
  const auto responsePayload = deserializer.GetRemainingBuffer();

  switch (responseHeader.GetMessageType())
  {
  case Sab::MethodCallMessageType::Response:
    OnReceiveResponse(responseHeader, responsePayload);
    break;
  case Sab::MethodCallMessageType::Request:
    // [[fallthrough]]
  case Sab::MethodCallMessageType::RequestNoReturn:
    assert(false && "No method requests are expected to be received by a consumed method.");
  }

  return 0;
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
