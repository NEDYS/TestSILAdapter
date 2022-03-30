/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#pragma once
  
#include <functional>

#include <chrono>
#include <thread>
#include <atomic>

#include "distSim/IDistSimIB.h"
#include "RemoteAmbassador.h"

namespace NDistSimIB {

class DistSimIB : public IDistSimIB
{
public:

  DistSimIB(const IBConfig& config);
  ~DistSimIB();

  // Services
  void RegisterService(Service service) override;
  void UnregisterService(Service service) override;

  IDOService* GetDOService() override;
  IBusMessageService* GetBusMessageService() override;
  ITimingService* GetTimingService() override;
  ITextLogService* GetTextLogService() override;
  
  // NEW SERVICE REQUIRES IMPLEMENTATION HERE

  // Common
  const std::string& GetVersion() override { return DistSimIBTaggedVersion; };

  bool IsConnected() const override;
  void StartMeasurement() override;
  void StopMeasurement() override;

  // ========================================
  // Simulation control

 SimulationControlCallbackHandle RegisterSimulationControlEvent(SimulationControlEvent SimulationEvent, SimulationControlCallback callback) override;
 void UnRegisterSimulationControlEvent(SimulationControlEvent SimulationEvent, SimulationControlCallbackHandle handle) override;

  // ========================================
  // Federation control

 FederationControlCallbackHandle RegisterFederationControlEvent(FederationControlEvent federationEvent, FederationControlCallback callback) override;
 void UnRegisterFederationControlEvent(FederationControlEvent federationEvent, FederationControlCallbackHandle handle) override;

  //bkd TODO: Prevent mixed usage of thread based / user loop
  // User loop
  void Connect() override;
  void Poll(double seconds) override;
  void Disconnect() override;

  // Thread based
  void Start() override;
  void Stop() override;

  void DisconnectRTINode() override;

private:
  // Common
  RemoteAmbassador mAmbassador;

  // Thread based
  void PollThread();
  void PollOnSignal();

  std::atomic<bool> mRunning;
  std::thread mThread;

};

} // namespace NDistSimIB