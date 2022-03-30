// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ProviderCallContext.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// ProviderCallContext
// -------------------

ProviderCallContext::ProviderCallContext(const std::shared_ptr<spdlog::logger>& logger,
  int64_t requestId,
  std::vector<uint8_t> inputParametersData) :
  CallContext(logger, requestId)
{
  mInputParameters.SetData(std::move(inputParametersData));
  auto onOutputParameterUpdate = [this](IValueEntity&) { mWereOutputParametersSerialized = true; };
  mOutputParameters.RegisterCallback(std::move(onOutputParameterUpdate), ValueUpdateMode::OnUpdate);
}
std::vector<uint8_t> ProviderCallContext::GetInputParametersData() const { return mInputParameters.CopyData(); }
std::vector<uint8_t> ProviderCallContext::GetOutputParametersData() const { return mOutputParameters.CopyData(); }
void ProviderCallContext::SetOutputParametersData(std::vector<uint8_t> data)
{
  // mWereOutputParametersSerialized is set via the VE OnUpdate
  mOutputParameters.SetData(std::move(data));
}
bool ProviderCallContext::WereOutputParametersSerialized() const { return mWereOutputParametersSerialized; }

// -------------------
// ICallContext
// -------------------

ISerializer* ProviderCallContext::GetInputParametersSerializer()
{
  throw std::runtime_error("Input parameters of a provided method cannot be written.");
}
IDeserializer* ProviderCallContext::GetInputParametersDeserializer() { return mInputParameters.GetDeserializer(); }
ISerializer* ProviderCallContext::GetOutputParametersSerializer() { return mOutputParameters.GetSerializer(); }
IDeserializer* ProviderCallContext::GetOutputParametersDeserializer() { return mOutputParameters.GetDeserializer(); }
CallState ProviderCallContext::GetCallState() const { return CallState::Called; }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
