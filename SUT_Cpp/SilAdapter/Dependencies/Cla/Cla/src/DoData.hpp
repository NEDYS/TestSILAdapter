// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IDoData.hpp"

#include "DoMember.hpp"
#include "ValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a data member of a distributed object. */
class DoData : public DoMember, public IDoData
{
public:
  // Public methods
  // -------------------

  /*! \brief Instantiates a new data member.
   *  \param fullPath The full path of the data member, i.e. "namespace::do.dataMember".
   *  \param direction The direction of the data member.
   *  \param logger The logger instance used by the data member and child elements for logging. */
  DoData(std::string fullPath, MemberDirection direction, std::shared_ptr<spdlog::logger> logger);

  // -------------------
  // IDoData
  // -------------------

  std::shared_ptr<IValueEntity> GetValue() const override;

protected:
  // Protected Members
  // -------------------

  uint32_t mDistSimHandle = 0;
  std::shared_ptr<ValueEntity> mValue;

  // Protected Methods
  // -------------------

  /*! \brief Returns the connection ID for this data member.
   *  \returns The connection ID for this data member. */
  std::string GetConnectionId() const;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
