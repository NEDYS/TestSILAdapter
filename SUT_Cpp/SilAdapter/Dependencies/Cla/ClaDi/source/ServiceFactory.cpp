// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ClaDi/ServiceFactory.hpp"

#include "distSim/IDistSimIB.h"

namespace Vector {
namespace CANoe {
namespace ClaDi {

ServiceFactory::ServiceFactory()
{
  // NB: BindFactory (without Impl) must not be used here in the CTOR. Otherwise, you will end up with a deadlock. ;-)
  BindFactoryImpl<NDistSimIB::IDistSimIB>([](const NDistSimIB::IBConfig& config) { return CreateDistSimIB(config); });
}

auto ServiceFactory::Instance() -> ServiceFactory&
{
  static ServiceFactory instance;
  return instance;
}

} // namespace ClaDi
} // namespace CANoe
} // namespace Vector
