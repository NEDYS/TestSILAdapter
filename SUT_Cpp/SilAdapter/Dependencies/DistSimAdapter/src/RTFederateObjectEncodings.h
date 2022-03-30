#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "RTI/encoding/BasicDataElements.h"

#include "distSim/RTFederateObjectDT.h"
#include "BusService.h"
#include "Utils.h"
#include "DistSimIBConfig.h"

namespace NDistSimIB {
  namespace NRTFederateEncoding {

    class BusEventInternal : public IBusEventInternal
    {
    public:
      BusEventInternal()
      {

      }
      BusEventInternal(const BusEventInternal& copy) : mOriginalSenderHandle(copy.mOriginalSenderHandle) { }
      void SetOriginalSenderHandle(rti::ObjectInstanceHandle originalSenderHandle)
      {
        mOriginalSenderHandle = originalSenderHandle;
      }
      rti::ObjectInstanceHandle GetOriginalSenderHandle() const
      {
        return mOriginalSenderHandle;
      }

    private:
      rti::ObjectInstanceHandle mOriginalSenderHandle;
    };

    class CANFrameEncodingInternal : public CANFrameEncoding, public BusEventInternal
    {
    public:
      CANFrameEncodingInternal()
        : CANFrameEncoding(), BusEventInternal()
      {
      }

      CANFrameEncodingInternal(const CANFrameEncodingInternal& copy)
        : CANFrameEncoding(copy), BusEventInternal(copy)
      {
        BusEventInternal::SetOriginalSenderHandle(copy.GetOriginalSenderHandle());
      }
    };

    class BusControllerInternal : IBusControllerInternal
    {
    public:
      BusControllerInternal()
      {
        mSenderMap = std::make_shared<std::map<VEventType, std::unique_ptr<IBusMessageService::IBusMessageSender>>>();
      }
      void SetSender(VEventType eventType, std::unique_ptr<IBusMessageService::IBusMessageSender> sender)
      {
        mSenderMap->insert(std::make_pair(eventType, std::move(sender)));
      }
      std::shared_ptr<std::map<VEventType, std::unique_ptr<IBusMessageService::IBusMessageSender>>> GetSenderMap() const
      {
        return mSenderMap;
      }

      void SetControllerObjectInstanceHandle(rti::ObjectInstanceHandle originalSenderHandle)
      {
        mControllerObjectInstanceHandle = originalSenderHandle;
      }
      rti::ObjectInstanceHandle GetControllerObjectInstanceHandle() const
      {
        return mControllerObjectInstanceHandle;
      }

      void SetBusService(IBusMessageService* busService)
      {
        mBusService = dynamic_cast<BusService*>(busService);
      }
      BusService* GetBusService() const
      {
        return mBusService;
      }

      void SetSelfReference(std::weak_ptr<IBusController> thisRef)
      {
        mThisRef = std::move(thisRef);
      }

    protected:
      std::weak_ptr<IBusController> mThisRef;

    private:
      std::shared_ptr<std::map<VEventType, std::unique_ptr<IBusMessageService::IBusMessageSender>>> mSenderMap;
      rti::ObjectInstanceHandle mControllerObjectInstanceHandle;
      BusService* mBusService;
    };

    // TODO introduce dummy class for aggregation (currently not necessary)
    class CanBusControllerEncoding : public ICanBusController, public BusControllerInternal
    {
    public:
      // TODO the values could be used for a default controller - insert proper values
      CanBusControllerEncoding() :
        mBaudRate(500000),
        mDataBaudRate(0),
        mPreScaler(0),
        mOperationMode(CanOperationMode::kCanOperationModeCan),
        mSync_Seg(CanBusState::kCanBusStateOnline),
        mProp_Seg(false),
        mPhase_Seg1(0),
        mPhase_Seg2(0),
        mSamplingMode(CanSamplingMode::kCanSamplingModeSampling_3)
      {
      };

      // implementation of IBusController
      BusType GetBusType() const override
      {
        return kBtCAN;
      }

      bool Send(VEventType eventType, std::unique_ptr<NRTFederateEncoding::BusEvent> event, bool isRequest = true, bool isBusManager = false) override
      {
        auto senderMap = GetSenderMap();
        auto find = senderMap->find(eventType);
        if (find == senderMap->end())
        {
          DebugPrintf(SWarn, "%s: Tried to send a message without having registered on its event type. Ignoring request.\n", __FUNCTION__);
          return false;
        }

        rti::ObjectInstanceHandle originalSender;
        if (isBusManager)
        {
          {
            BusEventInternal* tmpEvent = dynamic_cast<BusEventInternal*>(event.get());
            originalSender = tmpEvent->GetOriginalSenderHandle();
          }
        }
        else
        {
          originalSender = GetControllerObjectInstanceHandle();
        }

        return find->second->send(std::move(event), originalSender, isRequest);
      }

      CallbackHandle RegisterBusMessageRxTx(VEventType eventType, MessageReceivedCallback msgCallback) const override
      {
        return GetBusService()->RegisterBusMessage(eventType, mThisRef.lock(), msgCallback);
      }


      CallbackHandle RegisterBusMessageTxReq(VEventType eventType, MessageReceivedCallback msgCallback) const override
      {
        return GetBusService()->RegisterBusMessageTxReq(eventType, mThisRef.lock(), msgCallback);
      }

      CallbackHandle RegisterBusMessageRxTxWithMessageId(
        VEventType eventType, MessageId messageId, MessageReceivedCallback msgCallback) const override
      {
        return GetBusService()->RegisterBusMessageTxReqWithMessageId(eventType, mThisRef.lock(), messageId,
                                                                     std::move(msgCallback));
      }
      CallbackHandle RegisterBusMessageTxReqWithMessageId(
        VEventType eventType, MessageId messageId, MessageReceivedCallback msgCallback) const override
      {
        return GetBusService()->RegisterBusMessageTxReqWithMessageId(eventType, mThisRef.lock(), messageId,
                                                                     std::move(msgCallback));
      }
      void UnregisterBusMessage(CallbackHandle handle) override
      {
        GetBusService()->UnregisterBusMessage(handle);
      }

      // implementation of BusController (generated part of IBusController) TODO needs some more improvements before it can be automatically generated
      void SetNetworkId(std::string networkId) override
      {
        mNetworkId.set(networkId);
      }
      std::string GetNetworkId() const override
      {
        return mNetworkId.get();
      }

      void SetDeviceId(std::string deviceId) override
      {
        mDeviceId.set(deviceId);
      }
      std::string GetDeviceId() const override
      {
        return mDeviceId.get();
      }

      // implementation of ICanBusController
      void SetBaudRate(int32_t value) override
      {
        mBaudRate.set(value);
      }
      int32_t GetBaudRate() const override
      {
        return mBaudRate.get();
      }

      void SetDataBaudRate(int32_t value) override
      {
        mDataBaudRate.set(value);
      }
      int32_t GetDataBaudRate() const override
      {
        return mDataBaudRate.get();
      }

      void SetPreScaler(int32_t value) override
      {
        mPreScaler.set(value);
      }
      int32_t GetPreScaler() const override
      {
        return mPreScaler.get();
      }

      void SetOperationMode(CanOperationMode mode) override
      {
        mOperationMode = mode;
      }
      CanOperationMode GetOperationMode() const override
      {
        return mOperationMode;
      }

      void SetSync_Seg(int32_t value) override
      {
        mSync_Seg.set(value);
      }
      int32_t GetSync_Seg() const override
      {
        return mSync_Seg.get();
      }

      void SetProp_Seg(int32_t value) override
      {
        mProp_Seg.set(value);
      }
      int32_t GetProp_Seg() const override
      {
        return mProp_Seg.get();
      }

      void SetPhase_Seg1(int32_t value) override
      {
        mPhase_Seg1.set(value);
      }
      int32_t GetPhase_Seg1() const override
      {
        return mPhase_Seg1.get();
      }

      void SetPhase_Seg2(int32_t value) override
      {
        mPhase_Seg2.set(value);
      }
      int32_t GetPhase_Seg2() const override
      {
        return mPhase_Seg2.get();
      }

      void SetSamplingMode(CanSamplingMode mode) override
      {
        mSamplingMode = mode;
      }
      CanSamplingMode GetSamplingMode() const override
      {
        return mSamplingMode;
      }

    protected:
      rti::HLAASCIIstring mNetworkId;
      rti::HLAASCIIstring mDeviceId;
    private:
      rti::HLAinteger32LE mBaudRate;
      rti::HLAinteger32LE mDataBaudRate;
      rti::HLAinteger32LE mPreScaler;
      NRTFederateEncoding::CanOperationMode mOperationMode;
      rti::HLAinteger32LE mSync_Seg;
      rti::HLAinteger32LE mProp_Seg;
      rti::HLAinteger32LE mPhase_Seg1;
      rti::HLAinteger32LE mPhase_Seg2;
      NRTFederateEncoding::CanSamplingMode mSamplingMode;
    };

    class BusManagementInternal : public IBusManagementInternal
    {
    public:
      BusManagementInternal() : mIsInitialized(false) {}

      bool GetIsInitialized() const override
      {
        return mIsInitialized;
      }
      void SetIsInitialized(bool value) override
      {
        mIsInitialized = value;
      }

      void SetControllerObjectInstanceHandle(rti::ObjectInstanceHandle managementHandle) override
      {
        mManagementObjectInstanceHandle = managementHandle;
      }
      rti::ObjectInstanceHandle GetManagementObjectInstanceHandle() const override
      {
        return mManagementObjectInstanceHandle;
      }

    private:
      bool mIsInitialized;
      rti::ObjectInstanceHandle mManagementObjectInstanceHandle;
    };

    // Bus Management Objects
    // Generated implementation will not have base implementations...
    class CanBusManagementEncoding : public ICanBusManagement
    {
    public:
      CanBusManagementEncoding() :
        mCanBusState(CanBusState::kCanBusStateOnline),
        mSendMessagesAsRx(false),
        mRxErrorCount(0),
        mTxErrorCount(0)
      {
      };
      void SetNetworkId(std::string networkId) override
      {
        mNetworkId.set(networkId);
      }
      std::string GetNetworkId() const override
      {
        return mNetworkId.get();
      }

      CanBusState GetBusState() const override
      {
        CanBusState busState = static_cast<CanBusState>(mCanBusState.get());
        return busState;
      }
      void SetBusState(CanBusState state) override
      {
        mCanBusState.set(state);
      }

      bool GetSendMessagesAsRx() const override
      {
        return mSendMessagesAsRx.get();
      }
      void SetRequestMessagesAsRx(bool sendAsRx) override
      {
        mSendMessagesAsRx.set(sendAsRx);
      }

      uint32_t GetRxErrorCount() const override
      {
        return mRxErrorCount.get();
      }
      void SetRxErrorCount(uint32_t value) override
      {
        mRxErrorCount.set(value);
      }

      uint32_t GetTxErrorCount() const override
      {
        return mTxErrorCount.get();
      }
      void SetTxErrorCount(uint32_t value) override
      {
        mTxErrorCount.set(value);
      }

    protected:
      rti::HLAASCIIstring mNetworkId;
      rti::HLAinteger32LE mCanBusState;
      rti::HLAboolean mSendMessagesAsRx;
      rti::HLAinteger32LE mRxErrorCount;
      rti::HLAinteger32LE mTxErrorCount;
    };

    class CanBusManagementInternal : public CanBusManagementEncoding, public BusManagementInternal
    {
    public:
      CanBusManagementInternal()
        : CanBusManagementEncoding(), BusManagementInternal()
      {
      }

      BusType GetBusType() const override
      {
        return BusType::kBtCAN;
      }
    };
  } // namespace NRTFederateEncoding
} // namespace NDistSimIB
