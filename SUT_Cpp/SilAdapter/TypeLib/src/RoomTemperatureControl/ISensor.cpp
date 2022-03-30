// This code was generated automatically. Changes to this file may
// cause incorrect behavior and will be lost if the code is regenerated.

#include <future>
#include <cassert>

#include "TypeLib/RoomTemperatureControl/ISensor.hpp"
#include "TypeLib/Accessor.hpp"
#include "TypeLib/Deserializer.hpp"
#include "TypeLib/Serializer.hpp"
#include "TypeLib/ThreadContext.hpp"
#include "Path.hpp"
using namespace Vector::CANoe::TypeLib;

namespace RoomTemperatureControl {

void ISensor::_Init(const std::string& _path)
{
  const Path _doPath(_path);
  DistributedObjectBase::_Init(_doPath.GetFullDoPath());

  Accessor::Init(Temperature, DistributedObjectBase::mDo.get(), _doPath.GetSubMemberPath("Temperature"));
}

void ISensor::_WaitForFirstTransmission()
{
  DistributedObjectBase::_WaitForFirstTransmission();

  Accessor::WaitForFirstTransmission(Temperature);
}

void ISensor::_Reset()
{
  Accessor::Reset(Temperature);

  DistributedObjectBase::_Reset();
}

} // namespace RoomTemperatureControl
