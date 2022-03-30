// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "TypeLib/DoConsumedData.hpp"
#include "TypeLib/DoConsumedEvent.hpp"
#include "TypeLib/DoConsumedField.hpp"
#include "TypeLib/DoConsumedMethod.hpp"
#include "TypeLib/DoProvidedData.hpp"
#include "TypeLib/DoProvidedEvent.hpp"
#include "TypeLib/DoProvidedField.hpp"
#include "TypeLib/DoProvidedMethod.hpp"
#include "TypeLib/EmbeddedMember.hpp"

#include "cla/IDo.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

// Forward Declarations
class Accessor;

/*! Abstract base class for all distributed objects and interfaces. */
class DistributedObjectBase
{
protected:
  // Aliases
  // -------------------

  using _Get = Get;
  using _Set = Set<true, false>;
  using _SetNoReturn = Set<false, false>;
  using _SetOneWay = Set<false, true>;
  using _Notify = Notify;

  // Protected Methods
  // -------------------

  DistributedObjectBase() = default;
  virtual ~DistributedObjectBase() = default;

  friend class Accessor; // To initialize / reset

  /*! \brief Initializes the distributed object.
   *  \param path The full path of the distributed object. */
  virtual void _Init(const std::string& path);

  /*! \brief Waits for the first transmission from CANoe for all members on which the blocking was enabled. */
  virtual void _WaitForFirstTransmission();

  /*! \brief Resets the distributed object. The distributed object will no longer be valid. */
  virtual void _Reset();

  // Protected Members
  // -------------------

  std::shared_ptr<Cla::IDo> mDo; ///< Handle to distributed object of the CLA.
};

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
