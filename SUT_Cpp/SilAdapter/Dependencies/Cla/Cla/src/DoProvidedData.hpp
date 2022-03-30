// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IDoProvidedData.hpp"

#include "DoData.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a provided data member of a distributed object. */
class DoProvidedData : public DoData, public IDoProvidedData
{
public:
  // -------------------
  // DoProvidedData
  // -------------------

  /*! \brief Instantiates a new provided data member.
   *  \param fullPath The full path of the data member, i.e. "namespace::do.dataMember".
   *  \param txTrigger The mode for which a transmit shall be triggered.
   *  \param logger The logger instance used by the data member and its child elements for logging. */
  DoProvidedData(std::string fullPath, TxTrigger txTrigger, std::shared_ptr<spdlog::logger> logger);

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

private:
  // Private members
  // -------------------

  TxTrigger mTxTrigger;
  CallbackHandle mOnTransmitHandle{0};
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
