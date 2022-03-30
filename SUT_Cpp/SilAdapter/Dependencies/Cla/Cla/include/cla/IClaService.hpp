// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "IDo.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Interface which is the central entry point for the CLA and all its functionality. */
class IClaService
{
public:
  virtual ~IClaService() = default;

  /*! \brief Gets the distributed object with the specified full path.
   *  \param path The full path of the distributed object which shall be get.
   *  \returns the distributed object. */
  virtual std::shared_ptr<IDo> GetDo(const std::string& path) = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
