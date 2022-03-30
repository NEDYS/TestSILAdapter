// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <string>

#include "ClaMacros.hpp"
#include "DataTypes.hpp"
#include "IDoConsumedData.hpp"
#include "IDoProvidedData.hpp"
#include "IDoConsumedEvent.hpp"
#include "IDoProvidedEvent.hpp"
#include "IDoConsumedField.hpp"
#include "IDoProvidedField.hpp"
#include "IDoConsumedMethod.hpp"
#include "IDoProvidedMethod.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

class IDo
{
public:
  virtual ~IDo() = default;

  virtual std::shared_ptr<IDoConsumedData> GetConsumedData(const std::string& path) = 0;
  virtual std::shared_ptr<IDoProvidedData> GetProvidedData(const std::string& path, TxTrigger txTrigger) = 0;
  virtual std::shared_ptr<IDoConsumedEvent> GetConsumedEvent(const std::string& path) = 0;
  virtual std::shared_ptr<IDoProvidedEvent> GetProvidedEvent(const std::string& path, TxTrigger txTrigger) = 0;
  virtual std::shared_ptr<IDoConsumedField> GetConsumedField(const std::string& path) = 0;
  virtual std::shared_ptr<IDoProvidedField> GetProvidedField(const std::string& path, TxTrigger txTrigger) = 0;
  virtual std::shared_ptr<IDoConsumedMethod> GetConsumedMethod(const std::string& path) = 0;
  virtual std::shared_ptr<IDoProvidedMethod> GetProvidedMethod(const std::string& path) = 0;
};

CLAFUNCTIONDEF std::shared_ptr<IDo> GetDo(const std::string& path);

} // namespace Cla
} // namespace CANoe
} // namespace Vector
