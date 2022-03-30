/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#pragma once
  
#include <functional>
#include <string>
#include <vector>
#include <chrono>

#include "RTFederateDataTypes.h"
#include "RTFederateObjectDT.h"

// TODO: The last rti dependency here is ObjectInstanceHandle in IBusMessageSender::send.
// Refactor and remove the following include and class forwarding
#include "../src/DistSimIBConfig.h"
class ObjectInstanceHandle;

namespace NDistSimIB {

typedef int64_t VTimeNS;

enum class Service
{
  DistributedObjects,
  BusMessages,
  TextLog,
  Timing            
  // NEW SERVICE REQUIRES IMPLEMENTATION HERE
};

class IDODiscovererGlobal
{
public:
  virtual ~IDODiscovererGlobal() = default;
  virtual void OnDiscoverDOSource(const std::string& connectionID, const std::string& uuid, const std::string& connectionType, const std::string& producingFederateName) = 0;
  virtual void OnDiscoverDOTarget(const std::string& connectionID, const std::string& uuid, const std::string& connectionType, const std::string& producingFederateName) = 0;
  virtual void OnLostDOSource(const std::string& connectionID, const std::string& uuid, const std::string& connectionType, const std::string& producingFederateName) = 0;
  virtual void OnLostDOTarget(const std::string& connectionID, const std::string& uuid, const std::string& connectionType, const std::string& producingFederateName) = 0;
};

class IDODiscovererCounterpart
{
public:
  virtual ~IDODiscovererCounterpart() = default;
  virtual void OnDiscoverCounterpart(const std::string& connectionId, const std::string& uuid, const std::string& connectionType, const std::string& producingFederateName) = 0;
  virtual void OnLostCounterpart(const std::string& connectionId, const std::string& uuid, const std::string& connectionType, const std::string& producingFederateName) = 0;
};

class IDOService
{
public:
  virtual ~IDOService() = default;

  using DOMemberReceptionCallback = std::function<int32_t(const std::vector<uint8_t>& dataVec, const int64_t & time)>;
  enum class TransmitResult { OK, Queued, NotConnected, NotRegistered, TransmitError };
  using DOHandle = uint32_t;
  static const DOHandle kInvalidRegistrationHandle = 0;

  virtual TransmitResult TransmitDOMember(DOHandle sourceHandle, const std::vector<uint8_t>& dataVec) = 0;

  virtual DOHandle RegisterDOMemberSource(const std::string& connectionId) = 0;
  virtual DOHandle RegisterDOMemberSource(const std::string& connectionId, IDODiscovererCounterpart* discoverer) = 0;
  virtual DOHandle RegisterDOMemberSource(const std::string& connectionId, IDODiscovererCounterpart* discoverer, const std::string& connectionType) = 0;
  
  virtual DOHandle RegisterDOMemberTarget(const std::string& connectionId, DOMemberReceptionCallback receptionCallback) = 0;
  virtual DOHandle RegisterDOMemberTarget(const std::string& connectionId, DOMemberReceptionCallback receptionCallback, IDODiscovererCounterpart* discoverer) = 0;
  virtual DOHandle RegisterDOMemberTarget(const std::string& connectionId, DOMemberReceptionCallback receptionCallback, IDODiscovererCounterpart* discoverer, const std::string& connectionType) = 0;
  
  virtual DOHandle RegisterDOMemberTargetBySourceUUID(const std::string& sourceUUID, DOMemberReceptionCallback receptionCallback) = 0;
  virtual DOHandle RegisterDOMemberTargetBySourceUUID(const std::string& sourceUUID, DOMemberReceptionCallback receptionCallback, const std::string& connectionType) = 0;

  virtual void UnregisterDOMemberSource(DOHandle handle) = 0;
  virtual void UnregisterDOMemberTarget(DOHandle handle) = 0;

  virtual void RegisterGlobalDODiscovery(IDODiscovererGlobal* discoverer) = 0;
  virtual void RegisterGlobalDODiscovery(IDODiscovererGlobal* discoverer, const std::string& connectionType) = 0;
  
  virtual void UnregisterGlobalDODiscovery() = 0;
  virtual void UnregisterGlobalDODiscovery(const std::string& connectionType) = 0;

};

enum class VEventType : uint16_t
{
  kEvCanMessage = 0,
  kEvErrorFrame = 9,
  /* Daten- oder Remotebotschaft */ //value is used in Vector.CANalyzer.CAN.InteractiveGenerator.dll, keep synchronized

  kEvLinMessage = 1100,

  kEvEthernetPacket = 1500,
  kEvEthernetStatus = 1502,
  kEvEthernetPacketError = 1503,
  kEvEthernetPacketForwarded = 1517,
  kEvEthernetPacketErrorForwarded = 1518,

  kEvInvalid = 0xffff
};

// All supported event types 
const VEventType kEventTypes[] = {VEventType::kEvCanMessage, VEventType::kEvErrorFrame};
// All CAN event types
const VEventType kEventTypesCan[] = { VEventType::kEvCanMessage, VEventType::kEvErrorFrame };

struct IBusMessageService
{
  virtual ~IBusMessageService() = default;
  
  using NetworkId = std::string;
  using MessageId = uint32_t;
  using CallbackHandle = uint32_t;

  static const CallbackHandle kInvalidCallbackHandle = 0;

  class VRTFederateRegisteredBusMsg;
  class VRTFederateRegisteredBusMgmt;
  struct IBusManagementHandler
  {
    // handle bus message
    virtual void OnBusControllerDiscovered(const NRTFederateEncoding::BusEvent& event, NetworkId networkId, const int64_t& dispatchTime, bool requestedBySelf) = 0;
  };

  struct IBusMessageSender
  {
    virtual ~IBusMessageSender() = default;
    virtual bool send(std::unique_ptr<NRTFederateEncoding::BusEvent> event, const rti::ObjectInstanceHandle& senderObjectHandle, bool isRequest = true) = 0;
    virtual NetworkId GetNetworkId() const noexcept = 0;
  };

  typedef std::function<void(const NRTFederateEncoding::BusEvent& event, std::shared_ptr<NRTFederateEncoding::IBusController> controller, const int64_t& dispatchTime, bool requestedBySelf)> MessageReceivedCallback;

  virtual std::shared_ptr<NRTFederateEncoding::IBusController> CreateCanBusController(NetworkId channelName) = 0;
  virtual std::shared_ptr<NRTFederateEncoding::IBusController> CreateCanBusController(NetworkId channelName, std::string deviceName, std::string controllerName) = 0;
  virtual std::unique_ptr<NRTFederateEncoding::CANFrame> CreateCANFrame(uint32_t id, std::vector<uint8_t> data) const = 0;
  virtual std::unique_ptr<NRTFederateEncoding::CANFrame> CreateCANFrame(const NRTFederateEncoding::BusEvent& message) const = 0;

  //virtual std::unique_ptr<IRTBusMessageSender> GetMessageSender(enum VEventType eventType) = 0;
  //virtual std::unique_ptr<IBusMessageSender> GetMessageRequestSender(VEventType eventType, NetworkId networkId) = 0;
  //virtual void UpdateConfiguredChannels() = 0;

  // Bus Management API
  virtual CallbackHandle RegisterBusManagement(std::shared_ptr<NRTFederateEncoding::IBusManagement> busManagement, std::shared_ptr<IBusManagementHandler> mgmtHandler) = 0;
  virtual void UnregisterBusManagement(CallbackHandle handle) = 0;
  //virtual void GetBusControllerState(CallbackHandle handle) // TODO need an object representation for bus controllers first -- unnecessary for trivial simulator
  virtual std::shared_ptr<NRTFederateEncoding::ICanBusManagement> CreateCanBusManagement(NetworkId channelName) = 0;
};


class ITimingService
{
public:
  virtual ~ITimingService() = default;

  // TODO bkd: return value?
  using TimerCallback = std::function<int32_t(const std::chrono::nanoseconds timestamp)>;
  class ITimer
  {
  public:
    virtual ~ITimer() = default;
    virtual void Cancel() = 0;
    virtual void SetPeriod(VTimeNS newPeriod) = 0;
  };

  // Timer creation
  virtual std::shared_ptr<ITimer> CreateOneshotTimer(TimerCallback callback, std::chrono::duration<VTimeNS, std::nano> timeFromNow) = 0;
  virtual std::shared_ptr<ITimer> CreateCyclicTimer(TimerCallback callback, std::chrono::duration<VTimeNS, std::nano> cycleTime) = 0;
  virtual std::shared_ptr<ITimer> CreateCyclicTimer(TimerCallback callback, std::chrono::duration<VTimeNS, std::nano> offsetTime, std::chrono::duration<VTimeNS, std::nano> cycleTime) = 0;

  // Simulation control

  virtual void RunTo(std::chrono::duration<VTimeNS, std::nano> finalTimestamp) = 0; // user-loop only

  virtual void RequestSimulationTime(std::chrono::duration<VTimeNS, std::nano> newTime) = 0;
  virtual void RequestSimulationTimeStep(std::chrono::duration<VTimeNS, std::nano> delta) = 0;
  
  virtual std::chrono::nanoseconds GetSimTime() const = 0;

};


struct ITextLogService
{
  using TextLogCallback = std::function<void(const std::string& message, const std::string& sender)>;

  virtual ~ITextLogService() = default;
  virtual void LogText(const std::string& text) = 0;

  using CallbackHandle = uint32_t;
  static const CallbackHandle kInvalidCallbackHandle = 0;
  virtual CallbackHandle RegisterReceiveCallback(TextLogCallback callback) = 0;
  virtual void UnRegisterReceiveCallback(CallbackHandle handle) = 0;

};

  // NEW SERVICE REQUIRES IMPLEMENTATION HERE

} // namespace NDistSimIB
