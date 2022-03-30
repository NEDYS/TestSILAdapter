// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <type_traits>

namespace NDistSimIB {
class IDistSimIB;
struct IBConfig;
} // namespace NDistSimIB

struct IDiFactory
{
  virtual ~IDiFactory() = default;
};

template <class T>
struct DiFactory : IDiFactory
{
  using CreateT = std::function<std::unique_ptr<T>()>;
  CreateT create;
};

template <>
struct DiFactory<NDistSimIB::IDistSimIB> : IDiFactory
{
  using CreateT = std::function<std::unique_ptr<NDistSimIB::IDistSimIB>(const NDistSimIB::IBConfig&)>;
  CreateT create;
};


namespace Vector {
namespace CANoe {
namespace ClaDi {

class UnboundServiceError : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};

/*! \brief Factory which provides the concrete implementations
 *         for services used by the CLA. */
class ServiceFactory
{
public:
  /*! \brief Creates an instance of a specified service type.
   *  \tparam T The service type which shall be created.
   *  \returns a service instance of type T.
   *  \throws UnboundServiceError if no factory has been bound for service type T */
  template <class T, typename... ArgT>
  static auto CreateService(ArgT&&... arg) -> std::unique_ptr<T>
  {
    return Instance().CreateServiceImpl<T>(std::forward<ArgT>(arg)...);
  }

  /*! \brief Binds a factory function for a specified service type.
   *  \paramt T The service type for which a factory function shall be bound.
   *  \param factory The factory function for service type T. */
  template <class T>
  static void BindFactory(typename DiFactory<T>::CreateT factory)
  {
    return Instance().BindFactoryImpl<T>(std::move(factory));
  }

private:
  // ----------------------------------------
  // private methods
  ServiceFactory();

  static auto Instance() -> ServiceFactory&;

  template <class T, typename... ArgT>
  auto CreateServiceImpl(ArgT&&... arg) -> std::unique_ptr<T>
  {
    auto iter = mCustomFactories.find(TypeKey<T>());
    if (iter != mCustomFactories.end())
    {
      return static_cast<DiFactory<T>*>(iter->second.get())->create(std::forward<ArgT>(arg)...);
    }
    else
    {
      throw UnboundServiceError{typeid(T).name()};
    }
  }

  template <class T>
  void BindFactoryImpl(typename DiFactory<T>::CreateT factory)
  {
    auto newFactory = std::unique_ptr<DiFactory<T>>(new DiFactory<T>());
    newFactory->create = std::move(factory);
    mCustomFactories[TypeKey<T>()] = std::move(newFactory);
  }

  template <class T>
  static auto TypeKey() -> std::type_index
  {
    return std::type_index(typeid(T));
  }

private:
  // ----------------------------------------
  // private data members
  std::map<std::type_index, std::unique_ptr<IDiFactory>> mCustomFactories;
};

} // namespace ClaDi
} // namespace CANoe
} // namespace Vector
