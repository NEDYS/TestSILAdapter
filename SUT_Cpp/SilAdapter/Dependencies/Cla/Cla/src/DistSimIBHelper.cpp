// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DistSimIBHelper.hpp"

#include <stdexcept>

namespace Vector {
namespace CANoe {
namespace Cla {

std::string to_string(NDistSimIB::IDOService::TransmitResult value)
{
  switch (value)
  {
  case NDistSimIB::IDOService::TransmitResult::NotConnected:
    return "NotConnected";
  case NDistSimIB::IDOService::TransmitResult::NotRegistered:
    return "NotRegistered";
  case NDistSimIB::IDOService::TransmitResult::OK:
    return "OK";
  case NDistSimIB::IDOService::TransmitResult::Queued:
    return "Queued";
  case NDistSimIB::IDOService::TransmitResult::TransmitError:
    return "TransmitError";
  }

  throw std::runtime_error("Unknown Transmit Result");
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
