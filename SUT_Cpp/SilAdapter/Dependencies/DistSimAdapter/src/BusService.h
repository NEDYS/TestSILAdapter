/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/
#pragma once

#include <list>
#include "RemoteAmbassador.h"
#include "distSim/RTFederateDataTypes.h"
#include "distSim/RTFederateObjectDT.h"

namespace NDistSimIB
{
  struct BusManagementContainer;

  class BusService : public RemoteAmbassador::AbstractBusService
  {
  public:
    BusService(RemoteAmbassador &federate) noexcept;
    ~BusService() noexcept;

    // =========================================
    // Public DistSimAPI methods
    // // TODO these methods are not that public anymore...
    CallbackHandle RegisterBusMessage(VEventType eventType, std::shared_ptr<NRTFederateEncoding::IBusController> busController, MessageReceivedCallback msgCallback);
    CallbackHandle RegisterBusMessageTxReq(VEventType eventType, std::shared_ptr<NRTFederateEncoding::IBusController> busController, MessageReceivedCallback msgCallback);
    CallbackHandle RegisterBusMessageWithMessageId(VEventType eventType, std::shared_ptr<NRTFederateEncoding::IBusController> busController, MessageId messageId, MessageReceivedCallback msgCallback);
    CallbackHandle RegisterBusMessageTxReqWithMessageId(VEventType eventType, std::shared_ptr<NRTFederateEncoding::IBusController> busController, MessageId messageId, MessageReceivedCallback msgCallback);
    void UnregisterBusMessage(CallbackHandle handle);

    std::unique_ptr<NRTFederateEncoding::CANFrame> CreateCANFrame(MessageId id, std::vector<uint8_t> data) const override;
    std::unique_ptr<NRTFederateEncoding::CANFrame> CreateCANFrame(const NRTFederateEncoding::BusEvent& message) const override;
    std::shared_ptr<NRTFederateEncoding::IBusController> CreateCanBusController(NetworkId channelName) override;
    std::shared_ptr<NRTFederateEncoding::IBusController> CreateCanBusController(NetworkId channelName, std::string deviceName, std::string controllerName) override;

    void AddPublisher(VEventType eventType);
    void Publish(VEventType eventType);

    void RemovePublisher(VEventType eventType);
    void Unpublish(VEventType eventType);

    // Bus Management methods
    CallbackHandle RegisterBusManagement(std::shared_ptr<NRTFederateEncoding::IBusManagement> busControl, std::shared_ptr<IBusManagementHandler> mgmtHandler) override;
    void UnregisterBusManagement(CallbackHandle handle) override;
    std::shared_ptr<NRTFederateEncoding::ICanBusManagement> CreateCanBusManagement(NetworkId channelName) override;
    //virtual void GetBusControllerState(CallbackHandle handle) // TODO need an object representation for bus controllers first -- unnecessary for trivial simulator

    // =========================================
    // AbstractInteractionClassSubscriber
    void receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const &theParameterValues, rti::VariableLengthData const &theUserSuppliedTag, rti::SupplementalReceiveInfo theReceiveInfo) override;
    void receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const &theParameterValues, rti::VariableLengthData const &theUserSuppliedTag, rti::LogicalTime const &theTime, rti::SupplementalReceiveInfo theReceiveInfo) override;

    // =========================================
    // AbstractObjectClassSubscriber
    void discoverObjectInstance(rti::ObjectInstanceHandle theObject, rti::ObjectClassHandle theObjectClass, std::wstring const &theObjectInstanceName) override;
    void removeObjectInstance(rti::ObjectInstanceHandle theObject, rti::VariableLengthData const &theUserSuppliedTag, rti::LogicalTime const &theTime, rti::SupplementalRemoveInfo theRemoveInfo) override;
    void reflectAttributeValues(rti::ObjectInstanceHandle &theObject, rti::ObjectClassHandle theObjectClass, const rti::AttributeHandleValueMap &theAttributeValues, const rti::VariableLengthData &theUserSuppliedTag, rti::LogicalTime const& theTime,rti::FederateHandle producingFederateHandle) override;

    // =========================================
    // AbstractAttributeRequestProvider
    void provideAttributeValueUpdate(rti::ObjectClassHandle theObjectClass, rti::ObjectInstanceHandle theObject, rti::AttributeHandleSet const &theAttributes, rti::VariableLengthData const &theUserSuppliedTag) override;

    //=========================================
    // Methods called by RemoteAbmassador
    void OnJoined() override;
    void Reset() override;
    bool ProcessBuffers() override;

  private:
    // Handler maps
    // list of bus message handlers (generic)
    using BusMessageHandlerList = std::list<std::shared_ptr<VRTFederateRegisteredBusMsg> >;
    // list of bus message handlers, by message id
    using BusMessageIdHandlerMap = std::map<MessageId, BusMessageHandlerList>;
    using BusMessageChannelHandlerMap = std::map<NetworkId, BusMessageHandlerList>;

    // list of bus management handlers (generic)
    using BusManagementHandlerList = std::list<std::shared_ptr<VRTFederateRegisteredBusMgmt> >;
    
    void UnregisterBusMessageInternal(CallbackHandle handle, VEventType eventType, NetworkId networkId, 
      bool isTxReq, BusMessageHandlerList& msgHandlerList, BusMessageHandlerList::iterator where);
    void UnregisterBusMessageWithMessageIdInternal(CallbackHandle handle, VEventType eventType, NetworkId networkId, 
      bool isTxReq, MessageId messageId, BusMessageHandlerList& msgHandlerList, BusMessageHandlerList::iterator where);
    void UnregisterBusManagementInternal(CallbackHandle handle, NetworkId networkId,
      BusManagementHandlerList& mgmtHandlerList, BusManagementHandlerList::iterator where);

    std::unique_ptr<IBusMessageSender> GetMessageRequestSender(VEventType eventType, NetworkId networkId);

    static const wchar_t* GetMessageInteractionClassName(VEventType eventType);
    static NRTFederateEncoding::BusType GetBusType(VEventType eventType);

    //rti::InteractionClassHandle GetMessageInteractionClass(VEventType eventType);
    rti::InteractionClassHandle GetInteractionClassByEventType(VEventType eventType) const;
    VEventType GetEventType(rti::InteractionClassHandle handle) const;

    typedef std::function<rti::VariableLengthData(NRTFederateEncoding::BusEvent*)> BusEventEncoder;
    typedef std::function<void(NRTFederateEncoding::BusEvent*, const rti::VariableLengthData&)> BusEventDecoder;

    // create bus message encoder object for given eventType
    std::unique_ptr<NRTFederateEncoding::BusEvent> GetBusMessage(VEventType eventType);
    static BusEventEncoder GetBusMessageEncoder(VEventType eventType);
    static BusEventDecoder GetBusMessageDecoder(VEventType eventType);

    // =========================================
    //The parent federate
    RemoteAmbassador &mRemoteAmbassador;

    // =========================================
    // Private BusService methods

    // BusMessage logic
    // Init
    void InitializeHandles();
    void PublishSubscribe();
    void SubscribeNetwork(VEventType eventType, NetworkId networkId, bool subscribeRequest);
    void SubscribeMessageId(VEventType eventType, NetworkId networkId, MessageId messageId, bool subscribeRequest);

    void UnsubscribeNetwork(VEventType eventType, NetworkId networkId, bool isRequest);
    void UnsubscribeMessageId(VEventType eventType, NetworkId networkId, MessageId messageId, bool isRequest);

    bool IsEventPublished(VEventType eventType);
    bool IsNetworkSubscribed(VEventType eventType, NetworkId networkId);
    bool IsNetworkSubscribedAny(VEventType eventType, NetworkId networkId);
    bool IsMessageSubscribed(VEventType eventType, NetworkId networkId, MessageId messageId);

    bool IsFederationJoined();

    // Bus Management object handling
    void OnBusManagementObjectReservationCompleted(
      const std::string& instanceName,
      bool wasInstanceReservationSuccessful, 
      std::shared_ptr<NRTFederateEncoding::IBusManagement> busManagement, 
      std::shared_ptr<IBusManagementHandler> mgmtHandler);
    
    // Bus Controller object handling
    void OnBusControllerObjectReservationCompleted(
      const std::string& instanceName,
      bool wasInstanceReservationSuccessful,
      std::shared_ptr<NRTFederateEncoding::IBusController> busControl); 
    
    
    
    // =========================================
    // Private BusService data types

    // RTI Handles
    // BusMessage TransmitData Interaction
    rti::InteractionClassHandle mHandleBusMessage;
    rti::ParameterHandle        mBusMessageIsRequestParameter;
    rti::ParameterHandle        mBusMessageChannelNameParameter;
    rti::ParameterHandle        mBusMessageBusTypeParameter;
    rti::ParameterHandle        mBusMessageRequestingFederateParameter;
    rti::ParameterHandle        mBusMessageSenderParameter;
    rti::ParameterHandle        mBusMessageReceiverParameter;

    std::map<rti::InteractionClassHandle, VEventType> mEventTypeByInteraction;
    std::map<VEventType, rti::InteractionClassHandle> mInteractionByEventType;

    // Bus Management HLA Objects
    // base objects/attributes
    rti::ObjectClassHandle  mBusManagementObjClassHandle;
    rti::AttributeHandle    mBusManagementNetworkId;

    // CAN objects/attributes
    rti::ObjectClassHandle  mBusManagementCanObjClassHandle;
    rti::AttributeHandle    mBusManagementCanBusState;
    rti::AttributeHandle    mBusManagementCanTxErrorCount;
    rti::AttributeHandle    mBusManagementCanRxErrorCount;
    rti::AttributeHandle    mBusManagementCanSendMessagesAsRx;

    // Bus Controller HLA Objects
    // base objects/attributes
    rti::ObjectClassHandle  mBusControllerObjClassHandle;
    rti::AttributeHandle    mBusControllerNetworkId;
    rti::AttributeHandle    mBusControllerDeviceId;

    // CAN objects/attributes
    rti::ObjectClassHandle  mBusControllerCanObjClassHandle;
    rti::AttributeHandle    mBusControllerCanBaudRate;
    rti::AttributeHandle    mBusControllerCanDataBaudRate;
    rti::AttributeHandle    mBusControllerCanPreScaler;
    rti::AttributeHandle    mBusControllerCanOperationMode;
    rti::AttributeHandle    mBusControllerCanSync_Seg;
    rti::AttributeHandle    mBusControllerCanProp_Seg;
    rti::AttributeHandle    mBusControllerCanPhase_Seg1;
    rti::AttributeHandle    mBusControllerCanPhase_Seg2;
    rti::AttributeHandle    mBusControllerCanSamplingMode;

    CallbackHandle mCurrentCallbackHandle = 0;

    // generic message handlers for a specific channel
    std::map<VEventType, std::map<NetworkId, std::map<bool, BusMessageHandlerList>> > mBusMessageHandler;
    // specific handlers for a channel and an id
    std::map<VEventType, std::map<NetworkId, std::map<bool, BusMessageIdHandlerMap>> > mBusMessageHandlerByID;

    // generic management handlers for a specific channel
    std::map<NetworkId, BusManagementHandlerList> mBusManagementHandler;

    // bus message publications requested (number of requests, by event type)
    std::map<VEventType, uint32_t>                                            mRequestedBusMessagePublications;

    // bus message publications actually done (using publishInteractionClass)
    std::set<VEventType>                                                      mBusMessagePublications;
    //// # of bus message subscriptions requested requested, by channel
    //std::map<VEventType, std::set<NetworkId, uint32_t> >                      mRequestedBusMessageSubscriptions;
    // bus message subscriptions actually done, by channel and message id
    std::map<VEventType, std::set<NetworkId> >                                mBusMessageSubscriptions;
    //// # of bus message subscriptions requested requested, by channel and message id
    //std::map<VEventType, std::map<NetworkId, std::map<MessageId, uint32_t>> > mBusRequestedMessageSubscriptionsByID;
    // bus message subscriptions actually done, by channel and message id
    std::map<VEventType, std::map<NetworkId, std::set<MessageId>> >           mBusMessageSubscriptionsByID;


    // ** Bus Management **
    std::map<std::string, std::shared_ptr<NRTFederateEncoding::IBusManagement>> mBusManagementByIdentifier;
    std::map<rti::ObjectInstanceHandle, std::string> mNetworkNameByObjectHandle;

    // ** Bus Controller ** 
    std::map<rti::ObjectInstanceHandle, std::shared_ptr<NRTFederateEncoding::IBusController>>  mBusControllerByBusControllerObjectInstance;
    std::map<NetworkId, rti::ObjectInstanceHandle>  mBusControllerObjectInstanceByNetworkId;


    std::map<CallbackHandle, std::function<void()> > mUnregisterHandlerMap;

    struct CANMessageSender : public IBusMessageService::IBusMessageSender
    {
    public:
      CANMessageSender(BusService &parent, NetworkId networkId);
      ~CANMessageSender();
      virtual bool send(std::unique_ptr<NRTFederateEncoding::BusEvent> event, const rti::ObjectInstanceHandle& senderObjectHandle, bool isRequest) override;
      virtual NetworkId GetNetworkId() const noexcept override { return mNetworkId; }

    private:
      BusService &mParent;
      NetworkId mNetworkId;
    };

    static uint32_t sNextClientIndex;
  };
} // namespace NDistSimIB