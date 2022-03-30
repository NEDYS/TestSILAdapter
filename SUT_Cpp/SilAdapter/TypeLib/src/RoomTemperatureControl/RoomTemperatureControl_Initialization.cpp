// This code was generated automatically. Changes to this file may
// cause incorrect behavior and will be lost if the code is regenerated.

#include "RoomTemperatureControl_Initialization.hpp"

#include "TypeLib/Accessor.hpp"
using namespace Vector::CANoe::TypeLib;

#include "cla/IDo.hpp"

namespace RoomTemperatureControl {

static void PreInit_Sensor1(Vector::CANoe::Cla::IDo* _do)
{
}

static void PostInit_Sensor1()
{
}

static void Init_Sensor1()
{
  const auto _do = Vector::CANoe::Cla::GetDo("RoomTemperatureControl::Sensor1");
  PreInit_Sensor1(_do.get());
  Accessor::Init(Sensor1, "RoomTemperatureControl::Sensor1");
  PostInit_Sensor1();
}

static void PreInit_Sensor2(Vector::CANoe::Cla::IDo* _do)
{
}

static void PostInit_Sensor2()
{
}

static void Init_Sensor2()
{
  const auto _do = Vector::CANoe::Cla::GetDo("RoomTemperatureControl::Sensor2");
  PreInit_Sensor2(_do.get());
  Accessor::Init(Sensor2, "RoomTemperatureControl::Sensor2");
  PostInit_Sensor2();
}

static void PreInit_Sensor3(Vector::CANoe::Cla::IDo* _do)
{
}

static void PostInit_Sensor3()
{
}

static void Init_Sensor3()
{
  const auto _do = Vector::CANoe::Cla::GetDo("RoomTemperatureControl::Sensor3");
  PreInit_Sensor3(_do.get());
  Accessor::Init(Sensor3, "RoomTemperatureControl::Sensor3");
  PostInit_Sensor3();
}

static void PreInit_Heating(Vector::CANoe::Cla::IDo* _do)
{
}

static void PostInit_Heating()
{
}

static void Init_Heating()
{
  const auto _do = Vector::CANoe::Cla::GetDo("RoomTemperatureControl::Heating");
  PreInit_Heating(_do.get());
  Accessor::Init(Heating, "RoomTemperatureControl::Heating");
  PostInit_Heating();
}

void Init()
{
  Init_Sensor1();
  Init_Sensor2();
  Init_Sensor3();
  Init_Heating();

}

void WaitForFirstTransmission()
{
  Accessor::WaitForFirstTransmission(Sensor1);
  Accessor::WaitForFirstTransmission(Sensor2);
  Accessor::WaitForFirstTransmission(Sensor3);
  Accessor::WaitForFirstTransmission(Heating);

}

void ShutDown()
{

  Accessor::Reset(Sensor1);
  Accessor::Reset(Sensor2);
  Accessor::Reset(Sensor3);
  Accessor::Reset(Heating);
}

} // namespace RoomTemperatureControl
