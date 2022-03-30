// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Gets whether the current thread is within a callback. See \see StartCallback and \see EndCallback.
 *  \return `true` if the current thread is within callback, otherwise `false`. */
bool IsWithinCallback();

/*! \brief Starts a callback scope. \see IsWithinCallback will return `true` while \see EndCallback wasn't called. */
void StartCallback();

/*! \brief Ends a callback scope. Must be called once for each \see StartCallback call to
 *         leave the callback scope. */
void EndCallback();

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
