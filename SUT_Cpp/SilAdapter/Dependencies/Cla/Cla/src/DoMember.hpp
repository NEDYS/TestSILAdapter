// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IDoMember.hpp"

#include "spdlog/logger.h"

#include "distSim/IDistSimIB.h"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a member of a distributed object. */
class DoMember : public IDoMember
{
public:
  /*! \brief Instantiates a new member of a distributed object.
   *  \param fullPath The full path of the member, i.e. "namespace::do.member"
   *  \param type The type of the member.
   *  \param direction The direction of the member.
   *  \param logger The logger used by the member and its child elements for logging. */
  DoMember(std::string fullPath, MemberType type, MemberDirection direction, std::shared_ptr<spdlog::logger> logger);

  /*! \brief Returns the full path of the member, i.e. "namespace::do.member".
   *  \returns the full path of the member. */
  const std::string& GetFullPath() const;

  /*! \brief Returns the name of the virtual network of the data member.
   *  \returns The name of the virtual network of this data member. */
  std::string GetVirtualNetworkName() const;

  /*! \brief Connects the member with CANoe.
   *  \param service The DistSim service to establish the connection. */
  virtual void Connect(NDistSimIB::IDOService* service) = 0;

  /*! \brief Disconnects the member with CANoe. */
  virtual void Disconnect() = 0;

  // -------------------
  // IDoMember
  // -------------------

  MemberType GetType() const override;
  MemberDirection GetDirection() const override;

protected:
  // Protected Members
  // -------------------

  std::shared_ptr<spdlog::logger> mLogger;
  NDistSimIB::IDOService* mDistSimService = nullptr;

private:
  // Private Members
  // -------------------

  std::string mFullPath;
  MemberType mType;
  MemberDirection mDirection;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
