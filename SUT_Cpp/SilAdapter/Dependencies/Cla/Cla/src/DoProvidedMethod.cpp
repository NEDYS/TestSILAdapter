// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoProvidedMethod.hpp"
#include "ProviderCallContext.hpp"
#include "SabSerDes/Deserializer.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// DoProvidedMethod
// -------------------

DoProvidedMethod::DoProvidedMethod(std::string fullPath,
  std::shared_ptr<spdlog::logger> logger) :
  DoMethod(std::move(fullPath), MemberDirection::Provided, std::move(logger))
{
}

// -------------------
// IDoMember
// -------------------

MemberDirection DoProvidedMethod::GetDirection() const { return DoMember::GetDirection(); }
MemberType DoProvidedMethod::GetType() const { return DoMember::GetType(); }

// -------------------
// IDoProvidedMethod
// -------------------

void DoProvidedMethod::SetCallback(std::function<void(IProviderCallContext&)> callback)
{
  std::lock_guard<std::mutex> lock(mCallbackMutex);
  mCallback = std::move(callback);
}

// Private methods
// -------------------

int32_t DoProvidedMethod::OnReceiveData(const std::vector<uint8_t>& requestData, const int64_t&)
{
  // Get method call header
  Sab::Deserializer deserializer(requestData);
  const auto requestHeader = deserializer.Deserialize<Sab::MethodCallHeader>();
  const auto inputArguments = deserializer.GetRemainingBuffer();

  std::lock_guard<std::mutex> lock(mCallbackMutex);
  switch (requestHeader.GetMessageType())
  {
  case Sab::MethodCallMessageType::Request:
    if (mCallback)
    {
      ProviderCallContext callContext(mLogger, requestHeader.GetRequestId(), inputArguments);
      mCallback(callContext);

      if (callContext.WereOutputParametersSerialized())
      {
        const Sab::MethodCallHeader responseHeader(requestHeader.GetRequestId(),
          Sab::MethodCallMessageType::Response,
          Sab::MethodCallReturnCode::NoError);

        Sab::Serializer responseSerializer;
        responseSerializer.Serialize(responseHeader);
        responseSerializer.AppendBuffer(callContext.GetOutputParametersData());
        SendData(responseSerializer.ReleaseBuffer());
      }
    }
    else
    {
      // Send response with error and no data
      const Sab::MethodCallHeader responseHeader(requestHeader.GetRequestId(),
        Sab::MethodCallMessageType::Response,
        Sab::MethodCallReturnCode::NoCallbackRegistered);

      Sab::Serializer responseSerializer;
      responseSerializer.Serialize(responseHeader);
      SendData(responseSerializer.ReleaseBuffer());
    }
    break;
  case Sab::MethodCallMessageType::RequestNoReturn:
    if (mCallback)
    {
      ProviderCallContext callContext(mLogger, requestHeader.GetRequestId(), inputArguments);
      mCallback(callContext);
    }
    break;
  case Sab::MethodCallMessageType::Response:
    assert(false && "No method responses are expected to received by a provided method.");
    break;
  }
  return 0;
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
