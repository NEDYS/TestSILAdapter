/* -*-c++-*- OpenRTI - Copyright (C) 2009-2013 Mathias Froehlich
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
#include "InternalTimeManagement.h"

namespace OpenRTI {

InternalTimeManagement::InternalTimeManagement() :
  _timeRegulationMode(TimeRegulationDisabled),
  _timeConstrainedMode(TimeConstrainedDisabled),
  _timeAdvanceMode(TimeAdvanceGranted),
  _asynchronousDeliveryEnabled(false),
  _messageRetractionSerial(0)
{
}

std::string to_string(InternalTimeManagement::TimeAdvanceMode mode)
{
  switch (mode)
  {
    case InternalTimeManagement::TimeAdvanceMode::TimeAdvanceGranted:
      return "TimeAdvanceGranted";
    case InternalTimeManagement::TimeAdvanceMode::TimeAdvanceRequest:
      return "TimeAdvanceRequest";
    case InternalTimeManagement::TimeAdvanceMode::TimeAdvanceRequestAvailable:
      return "TimeAdvanceRequestAvailable";
    case InternalTimeManagement::TimeAdvanceMode::NextMessageRequest:
      return "NextMessageRequest";
    case InternalTimeManagement::TimeAdvanceMode::NextMessageRequestAvailable:
      return "NextMessageRequestAvailable";
    case InternalTimeManagement::TimeAdvanceMode::FlushQueueRequest:
      return "FlushQueueRequest";
    default:
      return "TimeAdvanceMode::Unknown";
  }
}

}
