// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IDoProvidedMethod.hpp"

#include "DoMethod.hpp"
#include "ProviderCallContext.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a provided method member of a distributed object
 *         on which a callback can be registered. */
class DoProvidedMethod : public DoMethod, public IDoProvidedMethod
{
public:
  // -------------------
  // DoProvidedMethod
  // -------------------

  /*! \brief Instantiates a new provided method.
   *  \param fullPath The full path of the provided method, i.e. 'namespace::do.member'.
   *  \param logger The logger which shall be used for logging by the provided method. */
  DoProvidedMethod(std::string fullPath, std::shared_ptr<spdlog::logger> logger);

  // -------------------
  // IDoMember
  // -------------------

  MemberType GetType() const override;
  MemberDirection GetDirection() const override;

  // -------------------
  // IDoProvidedMethod
  // -------------------

  void SetCallback(std::function<void(IProviderCallContext&)> callback) override;

protected:
  // Protected members
  // -------------------

  std::function<void(ProviderCallContext&)> mCallback{nullptr};
  std::mutex mCallbackMutex;

private:
  // Private methods
  // -------------------

  int32_t OnReceiveData(const std::vector<uint8_t>& data, const int64_t& timestamp) override;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
