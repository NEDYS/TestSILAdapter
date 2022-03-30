// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*!
 * \brief Friend class of all static and generated type lib classes
 *        to access the internal member methods.
 */
class Accessor
{
public:
  /*!
   * \brief Calls the `_Init` method on the specified object.
   * \tparam T The type of the object on which the `_Init` method shall be called.
   * \tparam InitArgs The types of the initialization arguments.
   * \param object The object on which the `_Init` method shall be called.
   * \param args The initialization arguments.
   */
  template <class T, class... InitArgs>
  static void Init(T& object, InitArgs&&... args)
  {
    object._Init(std::forward<InitArgs>(args)...);
  }

    /*!
   * \brief Calls the `_SetInitialValue` method on the specified object.
   * \tparam T The type of the object on which the `_SetInitialValue` method shall be called.
   * \tparam SetArgs The types of the set arguments.
   * \param object The object on which the `_SetInitialValue` method shall be called.
   * \param args The set arguments.
   */
  template <class T, class... SetArgs>
  static void SetInitialValue(T& object, SetArgs&&... args)
  {
    object._SetInitialValue(std::forward<SetArgs>(args)...);
  }

  /*!
   * \brief Calls the `_WaitForFirstTransmission` method on the specified object.
   * \tparam T The type of the object on which the `_Set` method shall be called.
   * \param object The object on which the `_WaitForFirstTransmission` method shall be called.
  */
  template <class T>
  static void WaitForFirstTransmission(T& object)
  {
    object._WaitForFirstTransmission();
  }

  /*!
   * \brief Calls the `_Reset` method on the specified object.
   * \tparam T The type of the object on which the `_Reset` method shall be called.
   * \param object The object on which the `_Reset` method shall be called.
   */
  template <class T>
  static void Reset(T& object)
  {
    object._Reset();
  }
};

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
