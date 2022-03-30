// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoRegistry.hpp"
#include "Do.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// Public Methods
// ------------------

DoRegistry::DoRegistry(std::shared_ptr<spdlog::logger> logger) : mLogger(std::move(logger)) {}

std::shared_ptr<IDo> DoRegistry::GetDo(const std::string& path)
{
  if (path.empty())
  {
    throw std::runtime_error("Cannot get new DO with an empty path.");
  }

  if (mDistSimService)
  {
    throw std::runtime_error("Cannot get new DO while the CLA service is connected.");
  }

  auto&& it = mDoInstances.find(path);
  if (it != mDoInstances.end())
  {
    return it->second;
  }

  auto newInstance = std::make_shared<Do>(path, mLogger);

  mDoInstances[path] = newInstance;
  mLogger->debug("Created DO '{}'", path);
  return newInstance;
}

void DoRegistry::Connect(NDistSimIB::IDOService* distSimService)
{
  mDistSimService = distSimService;
  for (auto& doInstance : mDoInstances)
  {
    doInstance.second->Connect(mDistSimService);
  }
}

void DoRegistry::Disconnect()
{
  for (auto& doInstance : mDoInstances)
  {
    doInstance.second->Disconnect();
  }
  mDistSimService = nullptr;
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
