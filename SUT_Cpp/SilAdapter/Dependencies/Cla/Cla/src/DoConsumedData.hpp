// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IDoConsumedData.hpp"

#include "DoData.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a consumed data member of a distributed object. */
class DoConsumedData : public DoData, public IDoConsumedData
{
public:
  // Public methods
  // -------------------

  /*! \brief Instantiates a new consumed data member.
   *  \param fullPath The full path of the data member, i.e. "namespace::do.dataMember".
   *  \param logger The logger instance used by the data member and all its child elements for logging. */
  DoConsumedData(std::string fullPath, std::shared_ptr<spdlog::logger> logger);

  // -------------------
  // DoMember
  // -------------------

  void Connect(NDistSimIB::IDOService* service) override;
  void Disconnect() override;

  // -------------------
  // IDoMember
  // -------------------

  MemberType GetType() const override;
  MemberDirection GetDirection() const override;

  // -------------------
  // IDoData
  // -------------------

  std::shared_ptr<IValueEntity> GetValue() const override;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
