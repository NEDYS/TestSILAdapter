// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <unordered_map>

#include "spdlog/logger.h"

#include "cla/IDo.hpp"

#include "DoMember.hpp"

#include "distSim/IServices.h"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing the implementation of a distributed object. */
class Do : public IDo
{
public:
  // -------------------
  // Do
  // -------------------

  /*! \brief Instantiates a new distributed object.
   *  \param path The full path of the distributed object, i.e. "namespace::name".
   *  \param logger The logger which is used by the distributed object and all its members for logging. */
  Do(std::string path, std::shared_ptr<spdlog::logger> logger);

  /*! \brief Gets the path of the distributed object.
   *  \returns the path of the distributed object. */
  const std::string& GetPath() const;

  /*! \brief Connects all members of the DO with CANoe.
   *  \param service The DistSim DO service the DO should use for the connection. */
  void Connect(NDistSimIB::IDOService* service);

  /*! \brief Disconnects all members of the DO with CANoe. */
  void Disconnect();

  // -------------------
  // IDo
  // -------------------

  std::shared_ptr<IDoConsumedData> GetConsumedData(const std::string& path) override;
  std::shared_ptr<IDoProvidedData> GetProvidedData(const std::string& path, TxTrigger txTrigger) override;
  std::shared_ptr<IDoConsumedMethod> GetConsumedMethod(const std::string& path) override;
  std::shared_ptr<IDoProvidedMethod> GetProvidedMethod(const std::string& path) override;
  std::shared_ptr<IDoConsumedEvent> GetConsumedEvent(const std::string& path) override;
  std::shared_ptr<IDoProvidedEvent> GetProvidedEvent(const std::string& path, TxTrigger txTrigger) override;
  std::shared_ptr<IDoConsumedField> GetConsumedField(const std::string& path) override;
  std::shared_ptr<IDoProvidedField> GetProvidedField(const std::string& path, TxTrigger txTrigger) override;

private:
  // Private Members
  // -------------------

  std::unordered_map<std::string, std::shared_ptr<DoMember>> mMembers;

  std::string mPath;
  std::shared_ptr<spdlog::logger> mLogger;

  NDistSimIB::IDOService* mDistSimService = nullptr;

  // Private methods
  // -------------------

  template <class T, class... Args>
  std::shared_ptr<T> GetMember(const std::string& path, Args&&... args);
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
