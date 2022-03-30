// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IProviderCallContext.hpp"

#include "CallContext.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing the implementation of a provider call context. */
class ProviderCallContext : public CallContext, public IProviderCallContext
{
public:
  // -------------------
  // ProviderCallContext
  // -------------------

  /*! \brief Instantiates a new provider call context.
   *  \param logger The logger which is used by the call context and all its child elements for logging.
   *  \param requestId The request ID of the method call corresponding to this call context.
   *  \param inputParametersData The data buffer for the input parameters / arguments. */
  ProviderCallContext(const std::shared_ptr<spdlog::logger>& logger,
    int64_t requestId,
    std::vector<uint8_t> inputParametersData);

  /*! \brief Gets the data buffer of the input parameters.
   *  \returns the data buffer of the output parameters. */
  std::vector<uint8_t> GetInputParametersData() const;

  /*! \brief Gets the data buffer of the output parameters.
   *  \returns the data buffer of the output parameters. */
  std::vector<uint8_t> GetOutputParametersData() const;

  /*! \brief Sets the data of the output parameters.
   *  \param data The data buffer of the output parameters.
   *  \remarks Setting the data of the output parameters will be treated as if
   *           output parameters were serialized. */
  void SetOutputParametersData(std::vector<uint8_t> data);

  /*! \brief Returns whether output parameters / arguments were serialized.
   *  \returns `true` if output parameters / arguments were serialized, otherwise `false`. */
  bool WereOutputParametersSerialized() const;

  // -------------------
  // ICallContext
  // -------------------

  ISerializer* GetInputParametersSerializer() override;
  IDeserializer* GetInputParametersDeserializer() override;
  ISerializer* GetOutputParametersSerializer() override;
  IDeserializer* GetOutputParametersDeserializer() override;
  CallState GetCallState() const override;

private:
  // Private members
  // -------------------

  bool mWereOutputParametersSerialized{false};
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
