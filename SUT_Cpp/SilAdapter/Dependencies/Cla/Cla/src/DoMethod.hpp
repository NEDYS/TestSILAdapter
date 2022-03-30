// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IDoMethod.hpp"

#include "DoMember.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a method member of a distributed object. */
class DoMethod : public DoMember, public IDoMethod
{
public:
  // -------------------
  // DoMethod
  // -------------------

  /*! \brief Instantiates a new method member.
   *  \param fullPath The full path of the method member, i.e. 'namespace::do.member'.
   *  \param direction The direction of the method member.
   *  \param logger The logger instance used by the method member for logging. */
  DoMethod(std::string fullPath, MemberDirection direction, std::shared_ptr<spdlog::logger> logger);

  // -------------------
  // DoMember
  // -------------------

  void Connect(NDistSimIB::IDOService* service) override;
  void Disconnect() override;

protected:
  // Protected methods
  // -------------------

  /*! \brief Sends data with the DistSim-Adapter via the outgoing connection
   *  \param data The data which shall be sent.
   *  \returns The transmit result. */
  NDistSimIB::IDOService::TransmitResult SendData(const std::vector<uint8_t>& data) const;

private:
  // Private member
  // -------------------

  uint32_t mDistSimHandleSender{0};   ///< The handle for the outgoing connection to send data
  uint32_t mDistSimHandleReceiver{0}; ///< The handle for the ingoing connection to receive data

  // Private methods
  // -------------------

  /*! \brief Gets the connection ID for method requests.
   *  \returns the connection ID for method requests. */
  std::string GetConnectionIdRequest() const;

  /*! \brief Gets the connection ID for method responses.
   *  \returns the connection ID for method responses. */
  std::string GetConnectionIdResponse() const;

  /*! \brief Callback when method call data is received from the DistSim-Adapter via the ingoing connection.
   *  \param data The method call data.
   *  \param timestamp The timestamp the data was received. */
  virtual int32_t OnReceiveData(const std::vector<uint8_t>& data, const int64_t& timestamp) = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
