/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
 *
 * This file is part of OpenRTI.
 *
 * OpenRTI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * OpenRTI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenRTI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "DebugNew.h"
#include "AbstractMessage.h"
#include "Message.h"
#include "ServerModel.h"

namespace OpenRTI {

std::string AbstractMessage::toString() const
{
  std::ostringstream s;
  out(s);
  return s.str();
}


std::string AbstractMessage::toString(ServerModel::Federation* federation) const
{
  std::ostringstream s;
  out(s, federation);
  return s.str();
}

bool
AbstractMessage::getReliable() const noexcept
{
  return true;
}

ObjectInstanceHandle
AbstractMessage::getObjectInstanceHandleForMessage() const noexcept
{
  return ObjectInstanceHandle();
}

std::ostream& prettyprint(std::ostream& os, const InteractionClassHandle& value, ServerModel::Federation* federation)
{
  const auto* interactionClass = federation->getInteractionClass(value);
  os << interactionClass->getFQName();
  return os;
}


std::ostream& prettyprint(std::ostream& os, const ParameterHandle& value, ServerModel::InteractionClass* interactionClass)
{
  const auto* parameter = interactionClass->findParameterDefinition(value);
  os << parameter->getName();
  return os;
}

} // namespace OpenRTI
