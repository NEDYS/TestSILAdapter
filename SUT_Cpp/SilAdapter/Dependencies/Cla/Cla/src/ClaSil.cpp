// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "cla/ClaSil.hpp"

#include "ClaService.hpp"
#include "spdlog/sinks/stdout_sinks.h"

namespace Vector {
namespace CANoe {
namespace Cla {

static IClaServiceSil& GetGlobalClaService()
{
  static ClaService s(std::make_shared<spdlog::logger>("ClaLogger", std::make_shared<spdlog::sinks::stdout_sink_mt>()));
  return s;
}

CLAFUNCTIONDEF void Connect() { GetGlobalClaService().Connect(); }

CLAFUNCTIONDEF void Disconnect() { GetGlobalClaService().Disconnect(); }

CLAFUNCTIONDEF std::shared_ptr<IDo> GetDo(const std::string& path) { return GetGlobalClaService().GetDo(path); }

CLAFUNCTIONDEF std::shared_ptr<IClaServiceSil> CreateClaService()
{
  auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
  auto logger = std::make_shared<spdlog::logger>("ClaLogger", std::move(sink));
  return std::make_shared<ClaService>(std::move(logger));
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
