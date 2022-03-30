// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <unordered_map>

#include "spdlog/spdlog.h"

#include "cla/IDo.hpp"

#include "Do.hpp"

#include "distSim/IServices.h"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class managing a set of distributed objects. */
class DoRegistry final
{
public:
  /*! \brief Instantiates a new registry for distributed objects.
   *  \param logger The logger instance for the registry and all its distributed objects. */
  DoRegistry(std::shared_ptr<spdlog::logger> logger);

  /*! \brief Returns a distributed object with the specified path.
   *         The distributed object is created if it does not exist.
   *  \param path The path of the distributed object which shall be get.
   *  \returns the distributed object with the specified path. */
  std::shared_ptr<IDo> GetDo(const std::string& path);

  /*! \brief Connects the members of all distributed objects to CANoe via the DistSim-adapter
   *  \param distSimService The DistSim service to connect the members to CANoe. */
  void Connect(NDistSimIB::IDOService* distSimService);

  /*! \brief Closes the connections of all previously distributed object members to CANoe. */
  void Disconnect();

private:
  // Private Members
  // ------------------

  std::unordered_map<std::string, std::shared_ptr<Do>> mDoInstances;
  std::shared_ptr<spdlog::logger> mLogger;

  NDistSimIB::IDOService* mDistSimService = nullptr;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
