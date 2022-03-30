// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "cla/ClaExperimental.hpp"

#include "ClaServiceExperimental.hpp"

#include "spdlog/sinks/stdout_sinks.h"

namespace Vector {
namespace CANoe {
namespace Cla {
namespace Experimental {

CLAFUNCTIONDEF std::shared_ptr<IClaServiceExperimental> CreateClaService()
{
  auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
  auto logger = std::make_shared<spdlog::logger>("ClaLogger", sink);
  return std::make_shared<ClaServiceExperimental>(logger);
}
	
} // namespace Experimental
} // namespace Cla
} // namespace CANoe
} // namespace Vector