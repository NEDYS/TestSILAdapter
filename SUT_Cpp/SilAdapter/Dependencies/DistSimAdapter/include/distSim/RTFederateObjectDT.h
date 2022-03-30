
#pragma once

#include <map>
#include <string>
#include <vector>

#include "IServices.h"

// TODO: Refactor *Internal (with rti dependencies) to src/ and remove the following two includes
#include "RTI/Handle.h"
#include "../src/DistSimIBConfig.h"

namespace NDistSimIB {
  enum class VEventType : unsigned short;
  struct IBusMessageService;
namespace NRTFederateEncoding {
  class IBusController;
  using CallbackHandle = uint32_t;
  using MessageId = uint32_t;
  typedef std::function<void(const NRTFederateEncoding::BusEvent& event, std::shared_ptr<IBusController> controller, const int64_t& dispatchTime, bool requestedBySelf)> MessageReceivedCallback;

  class IBusEventInternal
  {
  public:
    virtual ~IBusEventInternal() noexcept {}

    virtual void SetOriginalSenderHandle(rti::ObjectInstanceHandle originalSenderHandle) = 0;
    virtual rti::ObjectInstanceHandle GetOriginalSenderHandle() const = 0;
  };

  // Bus Controller Objects

  class IBusControllerInternal
  {
  public:
    virtual ~IBusControllerInternal() noexcept {}

    virtual void SetControllerObjectInstanceHandle(rti::ObjectInstanceHandle originalSenderHandle) = 0;
    virtual rti::ObjectInstanceHandle GetControllerObjectInstanceHandle() const = 0;
  };

  class BusController
  {
  public:
    virtual ~BusController() noexcept{}

    virtual void SetNetworkId(std::string networkId) = 0;
    virtual std::string GetNetworkId() const = 0;

    virtual void SetDeviceId(std::string deviceId) = 0;
    virtual std::string GetDeviceId() const = 0;
  };

  class IBusController : public BusController 
  {
  public:
    virtual ~IBusController() noexcept {}

    virtual BusType GetBusType() const = 0;

    virtual bool Send(VEventType eventType, std::unique_ptr<NRTFederateEncoding::BusEvent> event, bool isRequest = true, bool isBusManager = false) = 0;

    virtual CallbackHandle RegisterBusMessageRxTx(VEventType eventType, MessageReceivedCallback msgCallback) const = 0;
    virtual CallbackHandle RegisterBusMessageTxReq(VEventType eventType, MessageReceivedCallback msgCallback) const = 0;

    virtual CallbackHandle RegisterBusMessageRxTxWithMessageId(
      VEventType eventType, MessageId messageId, MessageReceivedCallback msgCallback) const = 0;
    virtual CallbackHandle RegisterBusMessageTxReqWithMessageId(
      VEventType eventType, MessageId messageId, MessageReceivedCallback msgCallback) const = 0;

    virtual void UnregisterBusMessage(CallbackHandle handle) = 0;
  };

  class ICanBusController : public IBusController
  {
  public:
    virtual ~ICanBusController() noexcept {}

    virtual void SetBaudRate(int32_t value) = 0;
    virtual int32_t GetBaudRate() const = 0;

    virtual void SetDataBaudRate(int32_t value) = 0;
    virtual int32_t GetDataBaudRate() const = 0;

    virtual void SetPreScaler(int32_t value) = 0;
    virtual int32_t GetPreScaler() const = 0;

    virtual void SetOperationMode(CanOperationMode mode) = 0;
    virtual CanOperationMode GetOperationMode() const = 0;

    virtual void SetSync_Seg(int32_t value) = 0;
    virtual int32_t GetSync_Seg() const = 0;

    virtual void SetProp_Seg(int32_t value) = 0;
    virtual int32_t GetProp_Seg() const = 0;

    virtual void SetPhase_Seg1(int32_t value) = 0;
    virtual int32_t GetPhase_Seg1() const = 0;

    virtual void SetPhase_Seg2(int32_t value) = 0;
    virtual int32_t GetPhase_Seg2() const = 0;

    virtual void SetSamplingMode(CanSamplingMode mode) = 0;
    virtual CanSamplingMode GetSamplingMode() const = 0;
  };


  class IBusManagementInternal
  {
  public:
    virtual ~IBusManagementInternal() noexcept {}

    virtual bool GetIsInitialized() const = 0;
    virtual void SetIsInitialized(bool value) = 0;

    virtual rti::ObjectInstanceHandle GetManagementObjectInstanceHandle() const = 0;
    virtual void SetControllerObjectInstanceHandle(rti::ObjectInstanceHandle managementHandle) = 0;
  };

  // Bus Management Objects
  class IBusManagementGenerated
  {
  public:
    virtual ~IBusManagementGenerated() noexcept {}

    virtual void SetNetworkId(std::string networkId) = 0;
    virtual std::string GetNetworkId() const = 0;
  };

  class IBusManagement : public IBusManagementGenerated
  {
  public:
    virtual BusType GetBusType() const = 0;
  };

  class ICanBusManagement : public IBusManagement
  {
  public:
    virtual CanBusState GetBusState() const = 0;
    virtual void SetBusState(CanBusState state) = 0;

    virtual bool GetSendMessagesAsRx() const = 0;
    virtual void SetRequestMessagesAsRx(bool sendAsRx) = 0;

    virtual uint32_t GetRxErrorCount() const = 0;
    virtual void SetRxErrorCount(uint32_t value) = 0;

    virtual uint32_t GetTxErrorCount() const = 0;
    virtual void SetTxErrorCount(uint32_t value) = 0;
  };

} // namespace NDistributedSimulation
} // namespace NRTFederateEncoding
