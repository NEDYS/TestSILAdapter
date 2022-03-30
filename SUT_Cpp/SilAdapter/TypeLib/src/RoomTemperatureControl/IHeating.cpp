// This code was generated automatically. Changes to this file may
// cause incorrect behavior and will be lost if the code is regenerated.

#include <future>
#include <cassert>

#include "TypeLib/RoomTemperatureControl/IHeating.hpp"
#include "TypeLib/Accessor.hpp"
#include "TypeLib/Deserializer.hpp"
#include "TypeLib/Serializer.hpp"
#include "TypeLib/ThreadContext.hpp"
#include "Path.hpp"
using namespace Vector::CANoe::TypeLib;

namespace RoomTemperatureControl {

void IHeating::_Init(const std::string& _path)
{
  const Path _doPath(_path);
  DistributedObjectBase::_Init(_doPath.GetFullDoPath());

  Accessor::Init(CoolingState, DistributedObjectBase::mDo.get(), _doPath.GetSubMemberPath("CoolingState"), TxTrigger::OnUpdate);
  Accessor::Init(HeatingState, DistributedObjectBase::mDo.get(), _doPath.GetSubMemberPath("HeatingState"), TxTrigger::OnUpdate);
}

void IHeating::_WaitForFirstTransmission()
{
  DistributedObjectBase::_WaitForFirstTransmission();

}

void IHeating::_Reset()
{
  Accessor::Reset(CoolingState);
  Accessor::Reset(HeatingState);

  DistributedObjectBase::_Reset();
}

} // namespace RoomTemperatureControl
