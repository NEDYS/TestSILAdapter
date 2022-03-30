// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IDoField.hpp"

#include "DoMember.hpp"
#include "ValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a field member of a distributed object. */
class DoField : public DoMember, public IDoField
{
public:
  // Public methods
  // -------------------

  /*! \brief Instantiates a new field member.
   *  \param fullPath The full path of the field member, i.e. "namespace::do.field".
   *  \param direction The direction of the field member.
   *  \param logger The logger instance used by the field member and all child elements. */
  DoField(std::string fullPath, MemberDirection direction, std::shared_ptr<spdlog::logger> logger);

  // -------------------
  // IDoField
  // -------------------

  std::shared_ptr<IValueEntity> GetValue() const override;

protected:
  // Protected Members
  // -------------------

  std::shared_ptr<ValueEntity> mValue;

  // Protected Methods
  // -------------------

  /*!
   * \brief Returns the member path of the field's getter.
   * \return the member path of the field's getter.
  */
  std::string GetGetterPath() const;

  /*! 
   * \brief Returns the member path of the field's setter.
   * \return the member path of the field's setter.
   */
  std::string GetSetterPath() const;

  /*! 
   * \brief Returns the member path of the field's event / notification.
   * \return the member path of the field's event / notification.
   */
  std::string GetEventPath() const;

};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
