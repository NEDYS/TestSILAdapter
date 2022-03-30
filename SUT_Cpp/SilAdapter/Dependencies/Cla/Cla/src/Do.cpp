// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Do.hpp"
#include "DoConsumedData.hpp"
#include "DoConsumedEvent.hpp"
#include "DoConsumedField.hpp"
#include "DoConsumedMethod.hpp"
#include "DoProvidedData.hpp"
#include "DoProvidedEvent.hpp"
#include "DoProvidedField.hpp"
#include "DoProvidedMethod.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// Do
// -------------------

Do::Do(std::string path, std::shared_ptr<spdlog::logger> logger) : mPath(std::move(path)), mLogger(std::move(logger)) {}

const std::string& Do::GetPath() const { return mPath; }

void Do::Connect(NDistSimIB::IDOService* service)
{
  mLogger->debug("Connecting members of {}...", mPath);
  mDistSimService = service;
  for (auto& member : mMembers)
  {
    member.second->Connect(service);
  }
}

void Do::Disconnect()
{
  for (auto& member : mMembers)
  {
    member.second->Disconnect();
  }
  mDistSimService = nullptr;
}

// -------------------
// IDo
// -------------------

std::shared_ptr<IDoConsumedData> Do::GetConsumedData(const std::string& path)
{
  return GetMember<DoConsumedData>(path);
}

std::shared_ptr<IDoProvidedData> Do::GetProvidedData(const std::string& path, TxTrigger txTrigger)
{
  return GetMember<DoProvidedData>(path, txTrigger);
}

std::shared_ptr<IDoConsumedMethod> Do::GetConsumedMethod(const std::string& path)
{
  return GetMember<DoConsumedMethod>(path);
}

std::shared_ptr<IDoProvidedMethod> Do::GetProvidedMethod(const std::string& path)
{
  return GetMember<DoProvidedMethod>(path);
}

std::shared_ptr<IDoConsumedEvent> Do::GetConsumedEvent(const std::string& path)
{
  return GetMember<DoConsumedEvent>(path);
}

std::shared_ptr<IDoProvidedEvent> Do::GetProvidedEvent(const std::string& path, TxTrigger txTrigger)
{
  return GetMember<DoProvidedEvent>(path, txTrigger);
}

std::shared_ptr<IDoConsumedField> Do::GetConsumedField(const std::string& path)
{
  return GetMember<DoConsumedField>(path);
}

std::shared_ptr<IDoProvidedField> Do::GetProvidedField(const std::string& path, TxTrigger txTrigger)
{
  return GetMember<DoProvidedField>(path, txTrigger);
}

template <class T, class... Args>
std::shared_ptr<T> Do::GetMember(const std::string& path, Args&&... args)
{
  if (mDistSimService)
  {
    throw std::runtime_error("Cannot get new DO member while the DO is connected.");
  }

  auto&& it = mMembers.find(path);
  if (it != mMembers.end())
  {
    auto member = std::dynamic_pointer_cast<T>(it->second);

    if (!member)
    {
      throw std::runtime_error("DO member with existing path '" + path + "' but different type was requested");
    }

    return member;
  }

  const auto fullPath = mPath + "." + path;
  auto newMember = std::make_shared<T>(fullPath, std::forward<Args>(args)..., mLogger);
  mMembers[path] = newMember;

  mLogger->debug("Created DO member '{}'", fullPath);
  return newMember;
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
