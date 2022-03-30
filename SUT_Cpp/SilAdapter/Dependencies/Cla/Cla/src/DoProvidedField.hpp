// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IDoProvidedField.hpp"

#include "DoField.hpp"
#include "DoProvidedEvent.hpp"
#include "DoProvidedMethod.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing a provided field member. */
class DoProvidedField : public DoField, public IDoProvidedField
{
public:
  // Public methods
  // -------------------

  /*!
   * \brief Instantiates a new provided field member.
   * \param fullPath The full path of the field member, i.e. "namespace::do.field".
   *  \param txTrigger The mode which specifies when a transmit shall be triggered.
   * \param logger The logger instance used by the field and all its child elements for logging.
   */
  DoProvidedField(std::string fullPath, TxTrigger txTrigger, std::shared_ptr<spdlog::logger> logger);

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
  // IDoProvidedField
  // -------------------

  std::shared_ptr<IDoProvidedMethod> GetProvidedGetter() const override;
  std::shared_ptr<IDoProvidedMethod> GetProvidedSetter() const override;
  std::shared_ptr<IDoProvidedEvent> GetProvidedEvent() const override;

private:
  // Nested classes
  // -------------------

  /*! \brief Class representing the getter method of a provided field. In comparison to
   *         the normal provided methods, the getter provides a default implementation
   *         if the callback is set to `nullptr`. */
  class Getter : public DoProvidedMethod
  {
  public:
    /*! \brief Instantiates a new getter method for a provided field.
     *  \param fullPath The full path of the getter method, i.e. 'namespace::do.member'.
     *  \param valueEntity The value entity which shall be used by the default getter implementation.
     *  \param logger The logger which shall be used for logging by the provided getter. */
    Getter(std::string fullPath,
      std::shared_ptr<ValueEntity> valueEntity,
      std::shared_ptr<spdlog::logger> logger);

    // -------------------
    // IDoProvidedMethod
    // -------------------

    void SetCallback(std::function<void(IProviderCallContext&)> callback) override;

  private:
    // Private members
    // -------------------

    std::shared_ptr<ValueEntity> mValue;

    // Private methods
    // -------------------

    void SetDefaultCallback();
  };

  /*! \brief Class representing the setter method of a provided field. In comparison to
   *         the normal provided methods, the setter provides a default implementation
   *         if the callback is set to `nullptr`. */
  class Setter : public DoProvidedMethod
  {
  public:
    /*! \brief Instantiates a new setter method for a provided field.
     *  \param fullPath The full path of the provided setter method, i.e. 'namespace::do.member'.
     *  \param valueEntity The value entity which shall be used by the default getter implementation.
     *  \param logger The logger which shall be used for logging by the provided setter method. */
    Setter(std::string fullPath,
      std::shared_ptr<ValueEntity> valueEntity,
      std::shared_ptr<spdlog::logger> logger);

    // -------------------
    // IDoProvidedMethod
    // -------------------

    void SetCallback(std::function<void(IProviderCallContext&)> callback) override;

  protected:
    // Protected members
    // -------------------

    std::shared_ptr<ValueEntity> mValue;

  private:
    // Private methods
    // -------------------

    void SetDefaultCallback();
  };

  // Private members
  // -------------------

  TxTrigger mTxTrigger;
  mutable std::shared_ptr<DoProvidedEvent> mEvent{nullptr};
  mutable std::shared_ptr<Getter> mGetter{nullptr};
  mutable std::shared_ptr<Setter> mSetter{nullptr};
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
