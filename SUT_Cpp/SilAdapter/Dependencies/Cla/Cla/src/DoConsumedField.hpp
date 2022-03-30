// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IDoConsumedField.hpp"

#include "DoField.hpp"
#include "DoConsumedEvent.hpp"
#include "DoConsumedMethod.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a consumed field member. */
class DoConsumedField : public DoField, public IDoConsumedField
{
public:
  // Public methods
  // -------------------

  /*!
   * \brief Instantiates a new consumed field member.
   * \param fullPath The full path of the field member, i.e. "namespace::do.field".
   * \param logger The logger instance used by the field and all its child elements for logging.
   */
  DoConsumedField(std::string fullPath, std::shared_ptr<spdlog::logger> logger);

  // -------------------
  // DoMember
  // -------------------

  void Connect(NDistSimIB::IDOService* service) override;
  void Disconnect() override;

  // -------------------
  // IDoMember
  // -------------------

  MemberType GetType() const override;
  MemberDirection GetDirection() const override;

  // -------------------
  // IDoField
  // -------------------

  std::shared_ptr<IValueEntity> GetValue() const override;
  std::shared_ptr<IDoMethod> GetGetter() override;
  std::shared_ptr<IDoMethod> GetSetter() override;
  std::shared_ptr<IDoEvent> GetEvent() override;

  // -------------------
  // IDoConsumedField
  // -------------------

  std::shared_ptr<IDoConsumedMethod> GetConsumedGetter() const override;
  std::shared_ptr<IDoConsumedMethod> GetConsumedSetter() const override;
  std::shared_ptr<IDoConsumedEvent> GetConsumedEvent() const override;

private:
  // Nested classes
  // -------------------

  /*! \brief Class representing the getter method of a consumed field. In comparison
   *         to the normal consumed method, the getter automatically updates the field value
   *         if it returns. */
  class Getter : public DoConsumedMethod
  {
  public:
    /*! \brief Instantiates a new getter method for a consumed field.
     *  \param fullPath The full path of the getter method, i.e. 'namespace::do.field.Get'
     *  \param valueEntity The value entity whose value shall be set when the getter returns.
     *  \param logger The logger which shall be used for logging by the getter. */
    Getter(std::string fullPath, std::shared_ptr<ValueEntity> valueEntity, std::shared_ptr<spdlog::logger> logger);

  private:
    // Private members
    // -------------------

    std::shared_ptr<ValueEntity> mValue;

    // Private methods
    // -------------------

    bool OnReceiveResponse(const Sab::MethodCallHeader& header, const std::vector<uint8_t>& payload) override;
  };

  /*! \brief Class representing the setter method of a consumed field. Same as
   *         the getter, since both update the VE value automatically when they return with data. */
  using Setter = Getter;

  // Private members
  // -------------------
 
  mutable std::shared_ptr<DoConsumedEvent> mEvent{nullptr};
  mutable std::shared_ptr<Getter> mGetter{nullptr};
  mutable std::shared_ptr<Setter> mSetter{nullptr};
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
