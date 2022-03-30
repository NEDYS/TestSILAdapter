/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#pragma once

#include <memory>
#include "IServices.h"

namespace NDistSimIB {

const std::string DistSimIBTaggedVersion = "v3.3.0";

struct IBConfig
{
  /// The IP address of the rtinode.
  std::string RtinodeAddress = "127.0.0.1";

  /// The unique name of this federate.
  std::string FederateName = "ExternalFederate";

  /// The non-unique type of this federate.
  std::string FederateType = "DistSimAdapterFederateType";

  /// The name of the federation this federate should join/create.
  /// If a federation with a partially matching name is found (e.g. CANoe, CANoe-1),
  /// this federate will join it. Otherwise, it will create it.
  std::string FederationNamePrefix = "CANoe";

  ///  Timeout (in milliseconds) for HLA connections/operations.
  uint32_t ConnectionTimeoutMS = 10000;
  
  /// Time regulation lookahead (in microseconds).
  /// This is an artificial delay to be added to each time stamped simulation event.
  /// Higher Lookahead delays will improve parallelism between simulation participants, but increase latency.
  /// Usually this delay should reflect modeled communication latencies / transmit times.
  VTimeNS Lookahead = 100000;

  /// Flag for the time synchronization mode 'Time regulating'.
  /// If enabled, a request for advance in simulation time will release all time constrained simulation participants 
  /// waiting for a time advance. If there is only a single time regulating participant, it will effectively drive 
  /// the simulation time of all time constrained participants. Not being time regulating means that other participants 
  /// will never wait for this participant to advance in simulation time.
  bool TimeRegulationEnabled = false;

  /// Flag for the time synchronization mode 'Time constrained'.
  /// If enabled, time stamped simulation events wil be processed in a chronological order across the federation.
  /// It also means that a request to advance in simulation time by this federate will cause all time regulating 
  /// simulation participants to wait until they have reached at least the requested simulation time.
  /// Not being time constrained results in receiving simulation events at any time (i.e. in the order they got sent) 
  /// with time stamps not being synchronous to the current simulation time and probably in a non-chronological order.
  bool TimeConstrainedEnabled = false;

  /// Flag to enable time interpolation for asynchronous federates.
  /// Federates with IBConfig::TimeRegulationEnabled == false and IBConfig::TimeRegulationEnabled == false will perform a 'best-effort'
  /// synchronization based on the timestamps in incoming events. If these events are sparse, the internal simulation 
  /// time between such events can be interpolated (e.g. for triggering timers in a more fine grid than the incoming evnets) 
  /// by enabling this flag.
  bool TimingInterpolationEnabled = false;

  /// Flag to forcedly wait for a common measurement start across the federation.
  /// This controls the behavior of NDistSimIB::Start() / NDistSimIB::Connect().
  /// With this flag enabled, this federate waits in a blocking fashion until all other participants have signalled
  /// readiness to also start the measurement. The actual start is triggered by any DistSimIB federate calling StartMeasurement()
  /// or any participating CANoe instance starting its measurement.
  /// If this flag is disabled, no start-up synchonization is performed in NDistSimIB::Start() / NDistSimIB::Connect().
  bool WaitForMeasurementStart = false;

  /// Flag to auto-join the subsequent federation on a measurement stop event.
  /// With this flag enabled, this federate reacts on a measurement stop by creating/joining the follow-up federation.
  /// The name of this federation usually is the IBConfig::FederationNamePrefix followed by an ascending number.
  /// The measurement stop is triggered by any DistSimIB federate calling StopMeasurement() or any participating CANoe 
  /// instance stopping its measurement.
  /// If this flag is disabled, this federate doesn't react on a measurement stop event and remains in the current federation. 
  bool RejoinOnMeasurementStop = true;

  /// Use the given string as the FOM. If empty, the default FOM is used
  std::string InjectFOM = "";

};


class IDistSimIB
{
public:
  virtual ~IDistSimIB() = default;

  // =========================================
  // Service registration

  /// Registers a specified service for further usage.
  /// Has to be called while in offline state (e.g. before calling Start()) and before the associated service getter can be called.
  /// Internally creates the unique service object, can only be called once per service.
  /// @param service: A named services of NDistSimIB::Service.
  virtual void RegisterService(Service service) = 0;

  /// Unregisters a specified service.
  /// Erases the service from internal bookkeeping. TODO: Undo HLA subscriptions laid out by the service.
  /// @param service: A named services of NDistSimIB::Service.
  virtual void UnregisterService(Service service) = 0;

  // =========================================
  // Service getters

  /// Gets the Distributed Objects service.
  /// Has to be registered beforehand with RegisterService()
  virtual IDOService* GetDOService() = 0;

  /// Gets the Bus Messaging service.
  /// Has to be registered beforehand with RegisterService()
  virtual IBusMessageService* GetBusMessageService() = 0;

  /// Gets the Timing service.
  /// Has to be registered beforehand with RegisterService()
  virtual ITimingService* GetTimingService() = 0;

  /// Gets the Text Logging service.
  /// Has to be registered beforehand with RegisterService()
  virtual ITextLogService* GetTextLogService() = 0;
  // NEW SERVICE REQUIRES IMPLEMENTATION HERE

  // =========================================
  // Versioning

  /// The current version string.
  /// Versioning scheme is MAJOR.MINOR.PATCH. 
  /// MAJOR versions are compatible with each other, 
  /// MINOR versions introduce new public API calls,
  /// PATCH versions for internal changes but unchanged public API.
  virtual const std::string& GetVersion() = 0;

  // =========================================
  // State handling

  /// Returns, if the connection to the runtime infrastructure is fully established.
  /// If True, the connection to the rtinode is established and a valid federation has been joined.
  virtual bool IsConnected() const = 0;
  
  // =========================================
  // Simulation control

  /// Simulation control command to start a measurement.
  /// Federates configured with IBConfig.WaitForMeasurementStart == true (NOT by default) wait for a synchronous start of the measurement, 
  /// which can be a CANoe instance starting it's measurement or any DistSimIB federate calling this command. 
  /// Triggers callbacks registered by RegisterOnSimulationStart().
  /// If a CANoe instance is part of the federation, this command will cause all stopped CANoe instances 
  /// to also start the measurement. 
  virtual void StartMeasurement() = 0;

  /// Simulation control command to stop a measurement.
  /// If IBConfig.RejoinOnMeasurementStop == true (default), DistSimIB federates will react by joining the new federation.
  /// Otherwise, they will stay in their current federation.
  /// Triggers callbacks registered by RegisterOnSimulationStop().
  /// If a CANoe instance is part of the federation, this command will cause all started CANoe instances 
  /// to also stop the measurement.
  virtual void StopMeasurement() = 0;

  using SimulationControlCallback = std::function<void()>;
  // Handle returned on registration, used to unregister callback
  using SimulationControlCallbackHandle = uint32_t;
  // Invalid handle constant
  static const SimulationControlCallbackHandle kInvalidSimulationControlCallbackHandle = 0;
  enum class SimulationControlEvent : uint8_t {
    Start = 0,
    Stop,
    Break,
    Resume
  };

  /// Callback registration for the simulation control flow.
  /// Start:  Callbacks are triggered on the start of the measurement, i.e. when CANoe starts it's measurement or another federate calls StartMeasurement().
  /// Stop:   Callbacks are triggered on the stop of the measurement, i.e. when CANoe stop it's measurement or another federate calls StopMeasurement().
  /// Break:  TRIGGER NOT IMPLEMENTED YET.
  /// Resume: TRIGGER NOT IMPLEMENTED YET.
  /// Returns a handle to unregister the callback with UnRegisterSimulationControlEvent().
  /// @param simulationEvent: The type of event.
  /// @param callback: The callback to be triggered.
  virtual SimulationControlCallbackHandle RegisterSimulationControlEvent(SimulationControlEvent simulationEvent, SimulationControlCallback callback) = 0;

  /// Callback unregistration for the simulation control flow.
  /// @param simulationEvent: The type of event.
  /// @param handle: The handle returned by RegisterSimulationControlEvent().
  virtual void UnRegisterSimulationControlEvent(SimulationControlEvent simulationEvent, SimulationControlCallbackHandle handle) = 0;

  // =========================================
  // Federation control
   
  using FederationControlCallback = std::function<void(const std::string& federationName)>;
  using FederationControlCallbackHandle = uint32_t;
  static const FederationControlCallbackHandle kInvalidFederationControlCallbackHandle = 0;
  enum class FederationControlEvent : uint8_t {
    Create = 0,
    Join,
    Resign,
    Destroy
  };

  /// Callback registration for the federation control flow.
  /// Create:   Callbacks are triggered when this federate created a federation.
  /// Join:     Callbacks are triggered when this federate successfully joins a federation.
  /// Resign:   Callbacks are triggered when this federate resigns from a federation. 
  /// Destroy:  Callbacks are triggered when this federate is the last participant in the current federation which is about to be destroyed.
  /// Returns a handle to unregister the callback with UnRegisterOnFederationJoined().
  /// @param callback: The callback to be triggered.
  virtual FederationControlCallbackHandle RegisterFederationControlEvent(FederationControlEvent federationEvent, FederationControlCallback callback) = 0;

  /// Callback unregistration for the federation control flow.
  /// @param federationEvent: The type of event.
  /// @param handle: The handle returned by RegisterOnFederationJoined().
  virtual void UnRegisterFederationControlEvent(FederationControlEvent federationEvent, FederationControlCallbackHandle handle) = 0;

  // =========================================
  // 'User-loop' usage commands

  /// 'User-Loop' command: Central call to establish the connection to the runtime infrastructure.
  /// Internally, this creates an ambassador object, connects to the rtinode, scans for available federations, 
  /// eventually creates a federation, joins the federation, handles the sync points, initializes the services, 
  /// processes buffered API calls and handles the internal state. 
  /// Should be called multiple times until IsConnected() == true.
  /// For TC/TR federates, Connect() waits for an incoming StartMeasurement event.
  virtual void Connect() = 0;

  /// 'User-Loop' command: Polls for incoming messages in a blocking fashion.
  /// Incoming messages are HLA callbacks that are directed to their responsible service. 
  /// @param seconds: The time in seconds to poll for new messages. If a message appears, 
  /// Poll() internally will try to receive more messages for another timespan of 'seconds'.
  virtual void Poll(double seconds) = 0;

  /// 'User-Loop' command: Resigns from the current federation and resets all registered services.
  /// If this is the last federate in the federation, the latter will be destroyed. 
  /// This command keeps the connection to the server node of the runtime infrastructure alive.
  virtual void Disconnect() = 0;

  // =========================================
  // 'Thread-based' usage commands

  /// 'Thread-based' command: Starts the manager thread for connection handling and message retrieval.
  /// This command takes care of what is done by calling Connect() and Poll() in the 'User-Loop' scenario, 
  /// including the necessary event loop running in a seperate thread. As Start() is intended not to block 
  /// the application, the connection is not established directly afterwards. IsConnected() can be used to check 
  /// the current state and to react on a fully established connection.
  /// For federates configured with IBConfig.WaitForMeasurementStart == true, Start() will wait internally 
  /// for an incoming StartMeasurement event.
  virtual void Start() = 0;

  /// 'Thread-based' command: Stops the manager thread.
  /// After stopping the manager thread, this command resigns from the current federation and resets all 
  /// registered services. If this is the last federate in the federation, the latter will be destroyed. 
  /// The connection to the rtinode is kept alive. Note that Stop() has to be called for a valid termination 
  /// of the application.
  virtual void Stop() = 0;

  // =========================================
  // Utility commands

  /// Closes the connection to the server node of the runtime infrastructure.
  /// This command can be used if a connection to a new endpoint is required. 
  virtual void DisconnectRTINode() = 0;

};

// =========================================
// Free functions

/// Returns an instance of the central DistSim IntegrationBus object.
/// Service registration, service getters, simulation control and 'Thread-based'/'User-loop' commands
/// are accessible via this object. Several instances can be created in one application.
/// @param config: The configuration struct for this instance. 
std::unique_ptr<IDistSimIB> CreateDistSimIB(const IBConfig& config = IBConfig());

} // namespace NDistSimIB
