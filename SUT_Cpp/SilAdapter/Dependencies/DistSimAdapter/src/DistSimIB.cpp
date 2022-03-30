/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#include "DistSimIB.h"
#include "TimingService.h"

namespace NDistSimIB {

  // Free function: Create the central object
  std::unique_ptr<IDistSimIB> CreateDistSimIB(const IBConfig& config) 
  {
    return std::unique_ptr<IDistSimIB>(new DistSimIB(config));
  }

  // CTor
  DistSimIB::DistSimIB(const IBConfig& config) : mAmbassador(config)
  {
  }

  //DTor
  DistSimIB::~DistSimIB() 
  {
    DebugPrintf(SDebug, "%s\n", __FUNCTION__);
    if (mRunning)
    {
      Stop();
    }
  }

  // =========================================
  // Service registry
  void DistSimIB::RegisterService(Service service)
  {
    mAmbassador.RegisterService(service);
  }
  void DistSimIB::UnregisterService(Service service)
  {
    mAmbassador.UnregisterService(service);
  }

// =========================================
// Service getters
IDOService* DistSimIB::GetDOService()
{
  auto where = mAmbassador.mRegisteredServices.find(Service::DistributedObjects);
  if (where != mAmbassador.mRegisteredServices.end())
  {
    return mAmbassador.mInternalDOService.get();
  }
  else
  {
    Error("Distributed Objects Service not registered\n");
    return nullptr;
  }
}

IBusMessageService* DistSimIB::GetBusMessageService()
{
  auto where = mAmbassador.mRegisteredServices.find(Service::BusMessages);
  if (where != mAmbassador.mRegisteredServices.end())
  {
    return mAmbassador.mInternalBusService.get();
  }
  else
  {
    Error("Bus Message Service not registered\n");
    return nullptr;
  }
}

ITimingService* DistSimIB::GetTimingService()
{
  return mAmbassador.mInternalTimingService.get();
}

ITextLogService* DistSimIB::GetTextLogService()
{
  return mAmbassador.mInternalTextLogService.get();
}

// NEW SERVICE REQUIRES IMPLEMENTATION HERE


bool DistSimIB::IsConnected() const
{
  return mAmbassador.GetState() == RemoteAmbassador::AmbassadorState::Online || 
    mAmbassador.GetState() == RemoteAmbassador::AmbassadorState::WaitForMeasurementInit;
}

void DistSimIB::StartMeasurement()
{
  if (DistSimIB::IsConnected())
  {
    mAmbassador.SendMeasurementInit();
  }
}

void DistSimIB::StopMeasurement()
{
  if (DistSimIB::IsConnected())
  {
    mAmbassador.SendMeasurementStop();
    mAmbassador.SendManualSignal();
  }
}

// ========================================
// Simulation control

IDistSimIB::SimulationControlCallbackHandle DistSimIB::RegisterSimulationControlEvent(SimulationControlEvent SimulationEvent, SimulationControlCallback callback)
{
  SimulationControlCallbackHandle handle = ++mAmbassador.mSimulationControlCallbackHandle;
  mAmbassador.mSimulationControlCallbacks[SimulationEvent][handle] = callback;
  return handle;
}

void DistSimIB::UnRegisterSimulationControlEvent(SimulationControlEvent SimulationEvent, SimulationControlCallbackHandle handle)
{
  auto iter_event = mAmbassador.mSimulationControlCallbacks.find(SimulationEvent);
  if (iter_event != mAmbassador.mSimulationControlCallbacks.end())
  {
    auto iter_handle = iter_event->second.find(handle);
    if (iter_handle != iter_event->second.end())
    {
      iter_event->second.erase(handle);
    }
    else
    {
      DebugPrintf(SWarn, "%s: unknown callback handle %d\n", __FUNCTION__, handle);
    }
  }
  else
  {
    DebugPrintf(SWarn, "%s: unknown simulation event %d\n", __FUNCTION__, SimulationEvent);
  }
}

// ========================================
// Federation control

IDistSimIB::FederationControlCallbackHandle DistSimIB::RegisterFederationControlEvent(FederationControlEvent federationEvent, FederationControlCallback callback)
{
  FederationControlCallbackHandle handle = ++mAmbassador.mFederationControlCallbackHandle;
  mAmbassador.mFederationControlCallbacks[federationEvent][handle] = callback;
  return handle;
}

void DistSimIB::UnRegisterFederationControlEvent(FederationControlEvent federationEvent, FederationControlCallbackHandle handle)
{
  auto iter_event = mAmbassador.mFederationControlCallbacks.find(federationEvent);
  if (iter_event != mAmbassador.mFederationControlCallbacks.end())
  {
    auto iter_handle = iter_event->second.find(handle);
    if (iter_handle != iter_event->second.end())
    {
      iter_event->second.erase(handle);
    }
    else
    {
      DebugPrintf(SWarn, "%s: unknown callback handle %d\n", __FUNCTION__, handle);
    }
  }
  else
  {
    DebugPrintf(SWarn, "%s: unknown federation event %d\n", __FUNCTION__, federationEvent);
  }
}


// =========================================
// User loop
void DistSimIB::Connect()
{
  DebugPrintf(SDebug,"Connect (state = %s)\n", to_string(mAmbassador.GetState()).c_str());
  mAmbassador.Connect();
}

void DistSimIB::Disconnect()
{
  DebugPrintf(SDebug,"Disconnect\n");
  mAmbassador.Disconnect();
}

void DistSimIB::DisconnectRTINode()
{
  DebugPrintf(SDebug, "DisconnectRTINode\n");
  mAmbassador.DisconnectRTINode();
}

// Evoke callbacks (without notification by openRTI)
void DistSimIB::Poll(double seconds)
{
  if (mAmbassador.GetState() == RemoteAmbassador::AmbassadorState::Online)
  {
    mAmbassador.EvokeCallbacksPolled(seconds);
  }
}

// =========================================
// Thread based

// Evoke callbacks (if openRTI signals available callbacks)
void DistSimIB::PollOnSignal()
{
  if (mAmbassador.GetState() == RemoteAmbassador::AmbassadorState::Online)
  {
    mAmbassador.EvokeCallbacksOnEvent();
  }
}

void DistSimIB::Start()
{
  if (mAmbassador.GetState() == RemoteAmbassador::AmbassadorState::Offline)
  {
    DebugPrintf(SDebug, "Start\n");
    mRunning = true;
    mThread = std::thread(&DistSimIB::PollThread, this);
  }
  else
  {
    Error("%s: Wrong state\n", __FUNCTION__);
  }
}

void DistSimIB::Stop()
{
  if (mAmbassador.GetState() != RemoteAmbassador::AmbassadorState::Offline)
  {
    DebugPrintf(SDebug, "Stop\n");
    mRunning = false;
    // The SendManualSignal() is needed to get out of the WaitForSingleEvent(-1) in EvokeCallbacksOnEvent() to exit the thread
    mAmbassador.SendManualSignal();
    if (mThread.joinable())
      mThread.join();
    mAmbassador.Disconnect();
  }
  else
  {
    DebugPrintf(SDebug, "%s: Wrong state\n", __FUNCTION__);
  }
}

void DistSimIB::PollThread()
{
  while (mRunning)
  {
    if (mAmbassador.GetState() != RemoteAmbassador::AmbassadorState::Online)
    {
      // Needs to be called multiple times to proceed to a joined state
      mAmbassador.Connect();

      // After successful connection in thread-based mode, setup the notification handle (once, not on reconnect)
      if (mAmbassador.GetState() == RemoteAmbassador::AmbassadorState::Online)
      {
        mAmbassador.SetNotificationHandle();
      }
    }
    else
    {
      // Evoke callbacks (if openRTI signals available callbacks)
      PollOnSignal();
    }
  }
}

} // namespace NDistSimIB