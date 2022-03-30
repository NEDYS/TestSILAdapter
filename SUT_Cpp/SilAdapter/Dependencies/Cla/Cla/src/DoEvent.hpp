// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IDoEvent.hpp"

#include "DoMember.hpp"
#include "ValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a event member of a distributed object. */
class DoEvent : public DoMember, public IDoEvent
{
public:
  // Public methods
  // -------------------

  /*! \brief Instantiates a new event member.
   *  \param fullPath The full path of the event member, i.e. "namespace::do.eventMember".
   *  \param direction The direction of the event member.
   *  \param logger The logger instance used by the event member and child elements for logging. */
  DoEvent(std::string fullPath, MemberDirection direction, std::shared_ptr<spdlog::logger> logger);

  /*! \brief Instantiates a new event member which uses provided value entity.
   *  \param fullPath The full path of the event member, i.e. "namespace::do.eventMember".
   *  \param direction The direction of the event member.
   *  \param valueEntity The value entity which shall be used by the event member.
   *  \param logger The logger instance used by the event member and child elements for logging. */
  DoEvent(std::string fullPath,
    MemberDirection direction,
    std::shared_ptr<ValueEntity> valueEntity,
    std::shared_ptr<spdlog::logger> logger);

  // -------------------
  // IDoEvent
  // -------------------

  std::shared_ptr<IValueEntity> GetValue() const override;

protected:
  // Protected Members
  // -------------------

  uint32_t mDistSimHandle = 0;
  std::shared_ptr<ValueEntity> mValue;

  // Protected Methods
  // -------------------

  /*! \brief Returns the connection ID for this event member.
   *  \returns The connection ID for this event member. */
  std::string GetConnectionId() const;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
