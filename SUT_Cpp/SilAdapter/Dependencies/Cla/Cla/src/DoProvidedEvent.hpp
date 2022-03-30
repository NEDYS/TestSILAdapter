// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IDoProvidedEvent.hpp"

#include "DoEvent.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a provided event member of a distributed object. */
class DoProvidedEvent : public DoEvent, public IDoProvidedEvent
{
public:
  // -------------------
  // DoProvidedEvent
  // -------------------

  /*! \brief Instantiates a new provided event member.
   *  \param fullPath The full path of the event member, i.e. "namespace::do.eventMember".
   *  \param txTrigger The mode which specifies when a transmit shall be triggered.
   *  \param logger The logger instance used by the event member and its child elements for logging. */
  DoProvidedEvent(std::string fullPath, TxTrigger txTrigger, std::shared_ptr<spdlog::logger> logger);

  /*! \brief Instantiates a new provided event member which uses a provided value entity.
   *  \param fullPath The full path of the event member, i.e. "namespace::do.eventMember".
   *  \param txTrigger The mode which specifies when a transmit shall be triggered.
   *  \param valueEntity The value entity which shall be used by the event.
   *  \param logger The logger instance used by the event member and its child elements for logging. */
  DoProvidedEvent(std::string fullPath,
    TxTrigger txTrigger,
    std::shared_ptr<ValueEntity> valueEntity,
    std::shared_ptr<spdlog::logger> logger);

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
  // IDoEvent
  // -------------------

  std::shared_ptr<IValueEntity> GetValue() const override;

private:
  // Private members
  // -------------------

  TxTrigger mTxTrigger;
  CallbackHandle mOnTransmitHandle{0};
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
