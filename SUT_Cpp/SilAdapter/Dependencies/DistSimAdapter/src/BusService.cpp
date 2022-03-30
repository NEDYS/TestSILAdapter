/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#include "BusService.h"
#include "UUIDT4.h"
#include "RTFederateEncodings.h"
#include "RTFederateObjectEncodings.h"
#include <sstream>
#include <iostream>
#include <utility>

namespace NDistSimIB {

template <VEventType EventType> struct EventClassOfEventType {};

template <> struct EventClassOfEventType<VEventType::kEvEthernetPacket>
{
  static const NRTFederateEncoding::BusType kBusType = NRTFederateEncoding::BusType::kBtEthernet;
  static const wchar_t* kInteractionClassName;
  using EncoderClass = NRTFederateEncoding::EthernetPacketEncoding;
  static rti::VariableLengthData encode(NRTFederateEncoding::BusEvent* message)
  {
    EncoderClass* encoder = dynamic_cast<EncoderClass*>(message);
    return encoder->encode();
  }
  static void decode(NRTFederateEncoding::BusEvent* message, const rti::VariableLengthData& data)
  {
    EncoderClass* decoder = dynamic_cast<EncoderClass*>(message);
    return decoder->decode(data);
  }
};
const wchar_t* EventClassOfEventType<VEventType::kEvEthernetPacket>::kInteractionClassName = L"HLAinteractionRoot.BusMessage.EthPacket";

template <> struct EventClassOfEventType<VEventType::kEvEthernetPacketForwarded>
{
  static const NRTFederateEncoding::BusType kBusType = NRTFederateEncoding::BusType::kBtEthernet;
  static const wchar_t* kInteractionClassName;
  using EncoderClass = NRTFederateEncoding::EthernetPacketForwardedEncoding;
  static rti::VariableLengthData encode(NRTFederateEncoding::BusEvent* message)
  {
    EncoderClass* encoder = dynamic_cast<EncoderClass*>(message);
    return encoder->encode();
  }
  static void decode(NRTFederateEncoding::BusEvent* message, const rti::VariableLengthData& data)
  {
    EncoderClass* decoder = dynamic_cast<EncoderClass*>(message);
    return decoder->decode(data);
  }
};
const wchar_t* EventClassOfEventType<VEventType::kEvEthernetPacketForwarded>::kInteractionClassName = L"HLAinteractionRoot.BusMessage.EthForwardedPacket";

template <> struct EventClassOfEventType<VEventType::kEvEthernetStatus>
{
  static const NRTFederateEncoding::BusType kBusType = NRTFederateEncoding::BusType::kBtEthernet;
  static const wchar_t* kInteractionClassName;
  using EncoderClass = NRTFederateEncoding::EthernetPacketEncoding;
  static rti::VariableLengthData encode(NRTFederateEncoding::BusEvent* message)
  {
    EncoderClass* encoder = dynamic_cast<EncoderClass*>(message);
    return encoder->encode();
  }
  static void decode(NRTFederateEncoding::BusEvent* message, const rti::VariableLengthData& data)
  {
    EncoderClass* decoder = dynamic_cast<EncoderClass*>(message);
    return decoder->decode(data);
  }
};
const wchar_t* EventClassOfEventType<VEventType::kEvEthernetStatus>::kInteractionClassName = L"HLAinteractionRoot.BusMessage.EthStatus";

template <> struct EventClassOfEventType<VEventType::kEvEthernetPacketError>
{
  static const NRTFederateEncoding::BusType kBusType = NRTFederateEncoding::BusType::kBtEthernet;
  static const wchar_t* kInteractionClassName;
  using EncoderClass = NRTFederateEncoding::EthernetPacketErrorEncoding;
  static rti::VariableLengthData encode(NRTFederateEncoding::BusEvent* message)
  {
    EncoderClass* encoder = dynamic_cast<EncoderClass*>(message);
    return encoder->encode();
  }
  static void decode(NRTFederateEncoding::BusEvent* message, const rti::VariableLengthData& data)
  {
    EncoderClass* decoder = dynamic_cast<EncoderClass*>(message);
    return decoder->decode(data);
  }
};
const wchar_t* EventClassOfEventType<VEventType::kEvEthernetPacketError>::kInteractionClassName = L"HLAinteractionRoot.BusMessage.EthPacketError";

template <> struct EventClassOfEventType<VEventType::kEvEthernetPacketErrorForwarded>
{
  static const NRTFederateEncoding::BusType kBusType = NRTFederateEncoding::BusType::kBtEthernet;
  static const wchar_t* kInteractionClassName;
  using EncoderClass = NRTFederateEncoding::EthernetPacketErrorForwardedEncoding;
  static rti::VariableLengthData encode(NRTFederateEncoding::BusEvent* message)
  {
    EncoderClass* encoder = dynamic_cast<EncoderClass*>(message);
    return encoder->encode();
  }
  static void decode(NRTFederateEncoding::BusEvent* message, const rti::VariableLengthData& data)
  {
    EncoderClass* decoder = dynamic_cast<EncoderClass*>(message);
    return decoder->decode(data);
  }
};
const wchar_t* EventClassOfEventType<VEventType::kEvEthernetPacketErrorForwarded>::kInteractionClassName = L"HLAinteractionRoot.BusMessage.EthPacketErrorForwarded";

template <> struct EventClassOfEventType<VEventType::kEvCanMessage>
{
  static const NRTFederateEncoding::BusType kBusType = NRTFederateEncoding::BusType::kBtCAN;
  static const wchar_t* kInteractionClassName;
  using EncoderClass = NRTFederateEncoding::CANFrameEncodingInternal;
  static rti::VariableLengthData encode(NRTFederateEncoding::BusEvent* message)
  {
    EncoderClass* encoder = dynamic_cast<EncoderClass*>(message);
    return encoder->encode();
  }
  static void decode(NRTFederateEncoding::BusEvent* message, const rti::VariableLengthData& data)
  {
    EncoderClass* decoder = dynamic_cast<EncoderClass*>(message);
    return decoder->decode(data);
  }
};
const wchar_t* EventClassOfEventType<VEventType::kEvCanMessage>::kInteractionClassName = L"HLAinteractionRoot.BusMessage.CANMessage";

template <> struct EventClassOfEventType<VEventType::kEvErrorFrame>
{
  static const NRTFederateEncoding::BusType kBusType = NRTFederateEncoding::BusType::kBtCAN;
  static const wchar_t* kInteractionClassName;
  using EncoderClass = NRTFederateEncoding::CANErrorFrameEncoding;
  static rti::VariableLengthData encode(NRTFederateEncoding::BusEvent* message)
  {
    EncoderClass* encoder = dynamic_cast<EncoderClass*>(message);
    return encoder->encode();
  }
  static void decode(NRTFederateEncoding::BusEvent* message, const rti::VariableLengthData& data)
  {
    EncoderClass* decoder = dynamic_cast<EncoderClass*>(message);
    return decoder->decode(data);
  }
};
const wchar_t* EventClassOfEventType<VEventType::kEvErrorFrame>::kInteractionClassName = L"HLAinteractionRoot.BusMessage.CANErrorFrame";

uint32_t BusService::sNextClientIndex = 0;

class IBusMessageService::VRTFederateRegisteredBusMsg
{
  public:
    VRTFederateRegisteredBusMsg(VEventType eventType,
      std::shared_ptr<NRTFederateEncoding::IBusController> busController, 
      MessageReceivedCallback msgCallback)
      : mMsgCallback(std::move(msgCallback))
      , mEventType(eventType)
      , mBusController(std::move(busController))
    {}

  public:
    rti::ParameterHandle           mFrameParameter;
    MessageReceivedCallback             mMsgCallback;
    VEventType                          mEventType;
    std::shared_ptr<NRTFederateEncoding::IBusController>  mBusController;
};

class IBusMessageService::VRTFederateRegisteredBusMgmt
{
  public:
    VRTFederateRegisteredBusMgmt(NetworkId networkId, std::shared_ptr<IBusManagementHandler> mgmtHandler)
      : mMgmtHandler(mgmtHandler)
    {
      auto ctrl = std::make_shared<NRTFederateEncoding::CanBusControllerEncoding>(); // TODO add generator method similar to bus messages to fix this
      ctrl->SetNetworkId(std::move(networkId));
    }

    std::shared_ptr<IBusManagementHandler> mMgmtHandler;
};

// =========================================
// Creates an instance of the Service
std::unique_ptr<RemoteAmbassador::AbstractBusService> CreateBusService(RemoteAmbassador& remoteAmb)
{
  return std::unique_ptr<RemoteAmbassador::AbstractBusService>(new BusService(remoteAmb));
}

BusService::BusService(RemoteAmbassador& remoteAmb) noexcept
  : mRemoteAmbassador(remoteAmb)
{
}

BusService::~BusService() noexcept
{
  DebugPrintf(SDebug, "~BusService\n");
}

//=========================================
// Methods called by RemoteAbmassador
void BusService::OnJoined()
{
  InitializeHandles();
  PublishSubscribe();

  // RegisterBufferedBusMessageSources();
}

void BusService::Reset()
{
  mBusMessagePublications.clear();
  mBusMessageSubscriptions.clear();
  mBusMessageSubscriptionsByID.clear();

  mNetworkNameByObjectHandle.clear();
}

bool BusService::ProcessBuffers()
{
  return false;
}

//=========================================
// InteractionSubscriber implementation

// Signature with time
void BusService::receiveInteraction(
  rti::InteractionClassHandle theInteraction, 
  rti::ParameterHandleValueMap const& theParameterValues, 
  rti::VariableLengthData const& theUserSuppliedTag, 
  rti::LogicalTime const& theTime, 
  rti::SupplementalReceiveInfo theReceiveInfo)
{
  // isRequest
  rti::VariableLengthData isRequest_vld;
  if (!mRemoteAmbassador.ReadParameter(isRequest_vld, theInteraction, theParameterValues, mBusMessageIsRequestParameter))
  {
    return; // Abort if missing
  }
  rti::HLAboolean isRequest;
  isRequest.decode(isRequest_vld);

  // networkID aka channelName
  rti::VariableLengthData networkId_vld;
  if (!mRemoteAmbassador.ReadParameter(networkId_vld, theInteraction, theParameterValues, mBusMessageChannelNameParameter))
  {
    return; // Abort if missing
  }
  rti::HLAASCIIstring networkId;
  networkId.decode(networkId_vld);
  
  VEventType eventType = GetEventType(theInteraction);
  {
    auto eventTypeIter = mBusMessageHandler.find(eventType);
    if (eventTypeIter != mBusMessageHandler.end())
    {
      const auto& channelTable = eventTypeIter->second;
      auto channelTableIter = channelTable.find(networkId);
      if (channelTableIter != channelTable.end())
      {
        auto& isRequestList = channelTableIter->second;// [isRequest] ;
        auto where = isRequestList.find(isRequest.get());
        if (where != isRequestList.end())
        {
          auto& handlerList = where->second;
          if (!handlerList.empty())
          {
            std::unique_ptr<NRTFederateEncoding::BusEvent> busMessage = GetBusMessage(eventType);
            auto decoder = GetBusMessageDecoder(eventType);

            // Frame data
            rti::ParameterHandle frameParameter = mRemoteAmbassador.mRTIAmb->getParameterHandle(theInteraction, FOMInteractions::kBusMessageSubclass_Frame);
            rti::VariableLengthData frameParameter_vld;
            if (!mRemoteAmbassador.ReadParameter(frameParameter_vld, theInteraction, theParameterValues, frameParameter))
            {
              return; // Abort if missing
            }

            decoder(busMessage.get(), frameParameter_vld);
            auto timestamp = convertTime(theTime);
            bool requestedBySelf;

            auto find = theParameterValues.find(mBusMessageSenderParameter);
            rti::ObjectInstanceHandle hdl;
            if (find != theParameterValues.end())
            {
              hdl = mRemoteAmbassador.mRTIAmb->decodeObjectInstanceHandle(find->second);
              auto* tmp = dynamic_cast<NRTFederateEncoding::BusEventInternal*>(busMessage.get());
              tmp->SetOriginalSenderHandle(hdl);
              // delete(tmp); // TODO necessary?
            }

            for (auto handler : handlerList)
            {
              auto internalController = std::dynamic_pointer_cast<NRTFederateEncoding::BusControllerInternal>(handler->mBusController);
              if (hdl.isValid() && hdl == internalController->GetControllerObjectInstanceHandle())
              {
                requestedBySelf = true;
              }
              else
              {
                // TODO REMARK is it legit to return false if the object instance handle is not valid?
                requestedBySelf = false;
              }

              handler->mMsgCallback(*busMessage, handler->mBusController, timestamp, requestedBySelf);
            }
          }
        }
      }
    }
  }

  rti::VariableLengthData messageId_vld;
  rti::ParameterHandle idParameter = mRemoteAmbassador.mRTIAmb->getParameterHandle(theInteraction, FOMInteractions::kBusMessageSubclass_Id);
  if (mRemoteAmbassador.ReadParameter(messageId_vld, theInteraction, theParameterValues, idParameter))
  {
    rti::HLAinteger32LE messageId;
    messageId.decode(messageId_vld);
    DebugPrintf(SDebug, "%s: received message on channel '%s' with id=%d (isRequest=%d)\n", __FUNCTION__, networkId.get().c_str(), messageId.get(), isRequest.get());

    auto eventTypeIter = mBusMessageHandlerByID.find(eventType);
    if (eventTypeIter != mBusMessageHandlerByID.end())
    {
      const auto& channelTable = eventTypeIter->second;
      auto channelTableIter = channelTable.find(networkId);
      if (channelTableIter != channelTable.end())
      {
        auto& isRequestList = channelTableIter->second;
        auto where = isRequestList.find(isRequest.get());
        if (where != isRequestList.end())
        {
          const auto& messageIdTable = where->second;
          auto messageIdTableIter = messageIdTable.find(messageId.get());
          if (messageIdTableIter != messageIdTable.end())
          {
            // NOTE: copy the handler list, it may get modified in a callback
            auto handlerList = messageIdTableIter->second;
            if (!handlerList.empty())
            {
              std::unique_ptr<NRTFederateEncoding::BusEvent> busMessage = GetBusMessage(eventType);
              auto decoder = GetBusMessageDecoder(eventType);
              
              rti::ParameterHandle frameParameter = mRemoteAmbassador.mRTIAmb->getParameterHandle(theInteraction, FOMInteractions::kBusMessageSubclass_Frame);
              rti::VariableLengthData frameParameter_vld;
              if (!mRemoteAmbassador.ReadParameter(frameParameter_vld, theInteraction, theParameterValues, frameParameter))
              {
                return; // Abort if missing
              }
              decoder(busMessage.get(), frameParameter_vld);

              bool requestedBySelf = false;
              rti::VariableLengthData senderFederateHandle_vld;
              if (mRemoteAmbassador.ReadParameter(senderFederateHandle_vld, theInteraction, theParameterValues, mBusMessageSenderParameter))
              {
                try
                {
                  rti::FederateHandle hdl = mRemoteAmbassador.mRTIAmb->decodeFederateHandle(senderFederateHandle_vld);
                  requestedBySelf = hdl == mRemoteAmbassador.mFederateHandle;
                }
                catch (rti::FederateNotExecutionMember)
                {
                  DebugPrintf(SWarn , "%s: Sender federate handle not found in current federation.\n", __FUNCTION__);
                }
                catch (...)
                {
                  Error("%s: Could not decode Sender federate handle.\n", __FUNCTION__);
                }
              }

              auto timestamp = convertTime(theTime);
              for (auto handler : handlerList)
              {
                handler->mMsgCallback(*busMessage, handler->mBusController, timestamp, requestedBySelf);
              }
            }
          }
        }
      }
    }
  }
}

// Signature without time
void BusService::receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& theParameterValues, rti::VariableLengthData const& theUserSuppliedTag, rti::SupplementalReceiveInfo theReceiveInfo)
{
  rti::HLAinteger64Time theTime(0); // dummy time
  receiveInteraction(theInteraction, theParameterValues, theUserSuppliedTag, theTime, theReceiveInfo);
}

// =========================================
// ObjectClassSubscriber implementation
void BusService::discoverObjectInstance(rti::ObjectInstanceHandle theObject, rti::ObjectClassHandle theObjectClass, std::wstring const& theObjectInstanceName)
{
  if (theObjectClass == mBusManagementCanObjClassHandle)
  {
    auto busManagementIter = mBusManagementByIdentifier.find(wstring2string(theObjectInstanceName));
    if (busManagementIter != mBusManagementByIdentifier.end())
    {
      Error("Discovered an object instance that was already known. Ignoring discovery...");
      return;
    }
    auto mgmt = std::make_shared<NRTFederateEncoding::CanBusManagementInternal>();
    mgmt->SetControllerObjectInstanceHandle(theObject);

    mBusManagementByIdentifier.insert(
      std::make_pair(wstring2string(theObjectInstanceName), mgmt));
    mNetworkNameByObjectHandle.insert(std::make_pair(theObject, wstring2string(theObjectInstanceName)));

    rti::AttributeHandleSet attributesOfBusManagement;
    attributesOfBusManagement.insert(mBusManagementNetworkId);
    attributesOfBusManagement.insert(mBusManagementCanBusState);
    attributesOfBusManagement.insert(mBusManagementCanTxErrorCount);
    attributesOfBusManagement.insert(mBusManagementCanRxErrorCount);
    attributesOfBusManagement.insert(mBusManagementCanSendMessagesAsRx);
    try
    {
      DebugPrintf(SInfo, "%s: Bus Management: discovered new object instance with ID '%s'\n", __FUNCTION__, wstring2string(theObjectInstanceName).c_str());
      mRemoteAmbassador.mRTIAmb->requestAttributeValueUpdate(theObject, attributesOfBusManagement, rti::HLAASCIIstring("Name").encode());
    }
    catch (const rti::ObjectInstanceNotKnown& e)
    {
      // Federate holding the object left
      DebugPrintf(SDebug, "%s: Bus Management: requestAttributeValueUpdate failed: %S\n", __FUNCTION__, e.what().c_str());
      (void)e;
    }
  }
  // Extension hint: Add more Bus Management classes once they become available
}
void BusService::removeObjectInstance(rti::ObjectInstanceHandle theObject, rti::VariableLengthData const& theUserSuppliedTag, rti::LogicalTime const& theTime, rti::SupplementalRemoveInfo theRemoveInfo)
{
  // TODO handle object removal for bus controllers (once handled in object discovery...)

  auto networkIdIter = mNetworkNameByObjectHandle.find(theObject);
  if (networkIdIter != mNetworkNameByObjectHandle.end())
  {
    DebugPrintf(SInfo, "%s: HLA object '%s' was removed.\n", __FUNCTION__, networkIdIter->second.c_str())
    auto busMgmtIter = mBusManagementByIdentifier.find(networkIdIter->second);
    if (busMgmtIter != mBusManagementByIdentifier.end())
    {
      // the removed object is a bus management
      // remove object from maps
      mBusManagementByIdentifier.erase(busMgmtIter);
      mNetworkNameByObjectHandle.erase(networkIdIter);
    }
  }
}

void BusService::reflectAttributeValues(rti::ObjectInstanceHandle& theObject, rti::ObjectClassHandle theObjectClass, const rti::AttributeHandleValueMap& theAttributeValues, const rti::VariableLengthData& theUserSuppliedTag, rti::LogicalTime const& theTime, rti::FederateHandle producingFederateHandle)
{
  if (theObjectClass == mBusManagementCanObjClassHandle)
  {
    auto networkNameIter = mNetworkNameByObjectHandle.find(theObject);
    if (networkNameIter != mNetworkNameByObjectHandle.end())
    {
      auto busMgmt = mBusManagementByIdentifier[networkNameIter->second];
      auto canBusMgmt = std::dynamic_pointer_cast<NRTFederateEncoding::ICanBusManagement>(busMgmt);
      auto internalBusManagement = std::dynamic_pointer_cast<NRTFederateEncoding::IBusManagementInternal>(busMgmt);

      if (!internalBusManagement->GetIsInitialized())
      {
        // check, if all necessary attributes are available
        auto isInitialized =
          theAttributeValues.count(mBusManagementNetworkId) &&
          theAttributeValues.count(mBusManagementCanBusState) &&
          theAttributeValues.count(mBusManagementCanTxErrorCount) &&
          theAttributeValues.count(mBusManagementCanRxErrorCount) &&
          theAttributeValues.count(mBusManagementCanSendMessagesAsRx);

        if (isInitialized)
        {
          internalBusManagement->SetIsInitialized(isInitialized);
        }
        else
        {
          return;
        }
        // Extension-hint: if CAN is modified, this needs to be adapted 
      }
      
      // update attributes
      for (auto const& kvp : theAttributeValues)
      {
        if (kvp.first == mBusManagementNetworkId)
        {
          rti::HLAASCIIstring decode;
          decode.decode(kvp.second);
          canBusMgmt->SetNetworkId(decode.get());
        }
        else if (kvp.first == mBusManagementCanBusState)
        {
          rti::HLAinteger32LE decode;
          decode.decode(kvp.second);
          canBusMgmt->SetBusState(static_cast<NRTFederateEncoding::CanBusState>(static_cast<int32_t>(decode.get())));
        }
        else if (kvp.first == mBusManagementCanTxErrorCount)
        {
          rti::HLAinteger32LE decode;
          decode.decode(kvp.second);
          canBusMgmt->SetTxErrorCount(decode.get());
        }
        else if (kvp.first == mBusManagementCanRxErrorCount)
        {
          rti::HLAinteger32LE decode;
          decode.decode(kvp.second);
          canBusMgmt->SetRxErrorCount(decode.get());
        }
        else if (kvp.first == mBusManagementCanSendMessagesAsRx)
        {
          rti::HLAboolean decode;
          decode.decode(kvp.second);
          canBusMgmt->SetRequestMessagesAsRx(decode.get());
        }
      }
      DebugPrintf(SInfo, "%s: Received initial values for '%s'.\n", __FUNCTION__, busMgmt->GetNetworkId().c_str());
    }
  }
  // Extension-hint: add more bus managements once they are available
  
  
  // if (theObjectClass == mBusMessageSourceObjClassHandle)
  // {
  //   //if (!sentBySelf)
  //   {
  //     HandleBusMessageSourceDiscovery(theObject, theAttributeValues, theTime);
  //   }
  // }
}

// =========================================
// ObjectAttributeRequestProvider implementation
void BusService::provideAttributeValueUpdate(rti::ObjectClassHandle theObjectClass, rti::ObjectInstanceHandle theObject, rti::AttributeHandleSet const& theAttributes, rti::VariableLengthData const& theUserSuppliedTag)
{
  // Look for the objHandle value in mBusMessageObjInstanceHandleByUUID
  // Could be replaced by reversed map that is filled on object creation
  // auto where_UUID = mBusMessageUUIByObjInstanceHandle.find(theObject);
  // if (where_UUID != mBusMessageUUIByObjInstanceHandle.end())
  // {
  //   UpdateBusMessageSourceObject(where_UUID->second);
  // }
  
  if (theObjectClass == mBusManagementCanObjClassHandle)
  {
    auto find = mNetworkNameByObjectHandle.find(theObject);
    if (find == mNetworkNameByObjectHandle.end())
    {
      DebugPrintf(SWarn, "%s: object not handled by this service (handle=%s).\n", __FUNCTION__, wstring2string(theObject.toString()).c_str());
      return;
    }
    auto instanceName = find->second;
    auto busManagement = mBusManagementByIdentifier[instanceName];

    // TODO FIXME iterate through all requested attributes and send only the requested ones
    // Currently, all fields are returned!
    rti::AttributeHandleValueMap attributeValues;

    auto busType = busManagement->GetBusType();
    // figure out what kind of control we actually have
    // TODO move into bus management (analogous to CANoe)
    if (busType == NRTFederateEncoding::kBtCAN)
    {
      const auto canManagement = std::dynamic_pointer_cast<NRTFederateEncoding::ICanBusManagement>(busManagement);
      attributeValues[mBusManagementNetworkId] = rti::HLAASCIIstring(canManagement->GetNetworkId()).encode();
      attributeValues[mBusManagementCanBusState] = rti::HLAinteger32LE(canManagement->GetBusState()).encode();
      attributeValues[mBusManagementCanTxErrorCount] = rti::HLAinteger32LE(canManagement->GetTxErrorCount()).encode();
      attributeValues[mBusManagementCanRxErrorCount] = rti::HLAinteger32LE(canManagement->GetTxErrorCount()).encode();
      attributeValues[mBusManagementCanSendMessagesAsRx] = rti::HLAboolean(canManagement->GetSendMessagesAsRx()).encode();
    }

    DebugPrintf(SDebug, "%s: Value request received and processed. Sending response.\n", __FUNCTION__);

    rti::HLAinteger64Time sendTime = mRemoteAmbassador.mInternalTimingService->GetEarliestSendTime();
    mRemoteAmbassador.mRTIAmb->updateAttributeValues(theObject, attributeValues, rti::VariableLengthData(), sendTime);
    
  }
  // Extension-Hint insert more bus types here once they are available
}

void BusService::OnBusControllerObjectReservationCompleted(
  const std::string& instanceName,
  bool wasInstanceReservationSuccessful,
  std::shared_ptr<NRTFederateEncoding::IBusController> busController)
{
  rti::ObjectInstanceHandle handle;
  // assert, if instance name is already reserved
  if (!wasInstanceReservationSuccessful)
  {
    auto where = mBusControllerObjectInstanceByNetworkId.find(busController->GetNetworkId());
    if (where == mBusControllerObjectInstanceByNetworkId.end())
    {
      DebugPrintf(SDebug, "Registered already known bus controller - skipping object creation.")
      void();
    }
    else
    {
      handle = where->second;
    }
  }
  else
  {
    auto handle = mRemoteAmbassador.mRTIAmb->registerObjectInstance(mBusControllerCanObjClassHandle, string2wstring(instanceName));
    // update all attribute values
    rti::AttributeHandleValueMap attributeValues;

    attributeValues[mBusControllerNetworkId] = rti::HLAASCIIstring(busController->GetNetworkId()).encode();
    attributeValues[mBusControllerDeviceId] = rti::HLAASCIIstring(busController->GetDeviceId()).encode();

    auto busType = busController->GetBusType();
    if (busType == NRTFederateEncoding::kBtCAN)
    {
      const auto canController = std::dynamic_pointer_cast<NRTFederateEncoding::CanBusControllerEncoding>(busController);
      attributeValues[mBusControllerCanBaudRate] = rti::HLAinteger32LE(canController->GetBaudRate()).encode();
      attributeValues[mBusControllerCanDataBaudRate] = rti::HLAinteger32LE(canController->GetDataBaudRate()).encode();
      attributeValues[mBusControllerCanPreScaler] = rti::HLAinteger32LE(canController->GetPreScaler()).encode();
      attributeValues[mBusControllerCanOperationMode] = rti::HLAinteger32LE(canController->GetOperationMode()).encode();
      attributeValues[mBusControllerCanSync_Seg] = rti::HLAinteger32LE(canController->GetSync_Seg()).encode();
      attributeValues[mBusControllerCanProp_Seg] = rti::HLAinteger32LE(canController->GetProp_Seg()).encode();
      attributeValues[mBusControllerCanPhase_Seg1] = rti::HLAinteger32LE(canController->GetPhase_Seg1()).encode();
      attributeValues[mBusControllerCanPhase_Seg2] = rti::HLAinteger32LE(canController->GetPhase_Seg2()).encode();
      attributeValues[mBusControllerCanSamplingMode] = rti::HLAinteger32LE(canController->GetSamplingMode()).encode();
      canController->SetControllerObjectInstanceHandle(handle);
    }
    // Extension-hint: if more bus controllers are available, then try next cast - otherwise print error and cancel 
    else
    {
      Error("unknown Bus Controller detected - aborting!");
      return;
    }

    // store object instance for later 
    mBusControllerByBusControllerObjectInstance.insert(std::make_pair(handle, busController));
    mBusControllerObjectInstanceByNetworkId.insert(std::make_pair(busController->GetNetworkId(), handle));

    // TODO subscription should be done here, not before object creation
  }
  // Extension-hint extend once more bus types are supported

  // TODO actually do something with the object -- see OnBusManagementObjectReservationCompleted
}

IBusMessageService::CallbackHandle BusService::RegisterBusMessage(VEventType eventType, std::shared_ptr<NRTFederateEncoding::IBusController> busController, MessageReceivedCallback msgCallback)
{
  std::shared_ptr<VRTFederateRegisteredBusMsg> reg = std::make_shared<VRTFederateRegisteredBusMsg>(
    eventType, busController, msgCallback);
  auto networkId = busController->GetNetworkId();

  auto internalBusController = std::dynamic_pointer_cast<NRTFederateEncoding::BusControllerInternal>(busController);
  if (internalBusController != nullptr)
  {
    // add message sender to bus controller
    internalBusController->SetSender(eventType, GetMessageRequestSender(eventType, networkId));
  }

  auto& msgHandlerList = mBusMessageHandler[eventType][networkId][false];
  auto it1 = msgHandlerList.insert(msgHandlerList.end(), reg);

  // TODO move to a point after the controller object was created to subscribe for targeted messages
  SubscribeNetwork(eventType, networkId, false);

  uint32_t handle = ++mCurrentCallbackHandle;
  //TODO move along with subscribeNetwork
  mUnregisterHandlerMap[handle] = [this, handle, eventType, networkId, &msgHandlerList, it1]()
  {
    UnregisterBusMessageInternal(handle, eventType, networkId, false, msgHandlerList, it1);
  };


  // Reserve name
  auto controllerInstanceName = busController->GetNetworkId() + "::" + busController->GetDeviceId();
  mRemoteAmbassador.reserveObjectInstanceName(
    string2wstring(controllerInstanceName),
    [this, busController](const std::wstring& theObjectInstanceName, bool succeeded)
    {
      OnBusControllerObjectReservationCompleted(wstring2string(theObjectInstanceName), succeeded, busController);
    });
  return handle;
}

IBusMessageService::CallbackHandle BusService::RegisterBusMessageTxReq(VEventType eventType, std::shared_ptr<NRTFederateEncoding::IBusController> busController, MessageReceivedCallback msgCallback)
{
  std::shared_ptr<VRTFederateRegisteredBusMsg> reg = std::make_shared<VRTFederateRegisteredBusMsg>(
    eventType, busController, msgCallback);
  auto networkId = busController->GetNetworkId();
  auto internalBusController = std::dynamic_pointer_cast<NRTFederateEncoding::BusControllerInternal>(busController);
  if (internalBusController != nullptr)
  {
    // add message sender to bus controller
    internalBusController->SetSender(eventType, GetMessageRequestSender(eventType, networkId));
  }
  // Extension-hint extend here once more bus controllers become available
  else
  {
    Error("Unknown Controller detected - skipping bus registration.");
    return -1;
  }
  auto& msgHandlerList = mBusMessageHandler[eventType][networkId][true];
  auto it1 = msgHandlerList.insert(msgHandlerList.end(), reg);

  SubscribeNetwork(eventType, networkId, true);

  uint32_t handle = ++mCurrentCallbackHandle;
  mUnregisterHandlerMap[handle] = [this, handle, eventType, networkId, &msgHandlerList, it1]()
  {
    UnregisterBusMessageInternal(handle, eventType, networkId, true, msgHandlerList, it1);
  };

  // Reserve name
  auto controllerInstanceName = busController->GetNetworkId() + "::" + busController->GetDeviceId();
  mRemoteAmbassador.reserveObjectInstanceName(
    string2wstring(controllerInstanceName),
    [this, busController](const std::wstring& theObjectInstanceName, bool succeeded)
    {
      OnBusControllerObjectReservationCompleted(wstring2string(theObjectInstanceName), succeeded, busController);
    });
  return handle;
}

IBusMessageService::CallbackHandle BusService::RegisterBusMessageWithMessageId(
  VEventType eventType, 
  std::shared_ptr<NRTFederateEncoding::IBusController> busController, 
  uint32_t messageId, 
  MessageReceivedCallback msgCallback)
{
  std::shared_ptr<VRTFederateRegisteredBusMsg> reg = std::make_shared<VRTFederateRegisteredBusMsg>(
    eventType, busController, msgCallback);
  auto networkId = busController->GetNetworkId();
  auto internalBusController = std::dynamic_pointer_cast<NRTFederateEncoding::BusControllerInternal>(busController);
  if (internalBusController != nullptr)
  {
    // add message sender to bus controller
    internalBusController->SetSender(eventType, GetMessageRequestSender(eventType, networkId));
  }
  // Extension-hint extend here once more bus controllers become available
  else
  {
    Error("Unknown Controller detected - skipping bus registration.");
    return -1;
  }
  auto& msgHandlerList = mBusMessageHandlerByID[eventType][networkId][false][messageId];
  auto it1 = msgHandlerList.insert(msgHandlerList.end(), reg);

  SubscribeMessageId(eventType, networkId, messageId, false);

  uint32_t handle = ++mCurrentCallbackHandle;
  mUnregisterHandlerMap[handle] = [this, handle, eventType, networkId, messageId, &msgHandlerList, it1]()
  {
    UnregisterBusMessageWithMessageIdInternal(handle, eventType, networkId, false, messageId, msgHandlerList, it1);
  };

  // Reserve name
  auto controllerInstanceName = busController->GetNetworkId() + "::" + busController->GetDeviceId();
  mRemoteAmbassador.reserveObjectInstanceName(
    string2wstring(controllerInstanceName),
    [this, busController](const std::wstring& theObjectInstanceName, bool succeeded)
    {
      OnBusControllerObjectReservationCompleted(wstring2string(theObjectInstanceName), succeeded, busController);
    });
  return handle;
}

IBusMessageService::CallbackHandle BusService::RegisterBusMessageTxReqWithMessageId(
  VEventType eventType, std::shared_ptr<NRTFederateEncoding::IBusController> busController, 
  uint32_t messageId, MessageReceivedCallback msgCallback)
{
  std::shared_ptr<VRTFederateRegisteredBusMsg> reg = std::make_shared<VRTFederateRegisteredBusMsg>(
    eventType, busController, msgCallback);
  auto networkId = busController->GetNetworkId();
  auto internalBusController = std::dynamic_pointer_cast<NRTFederateEncoding::BusControllerInternal>(busController);
  if (internalBusController != nullptr)
  {
    // add message sender to bus controller
    internalBusController->SetSender(eventType, GetMessageRequestSender(eventType, networkId));
  }
  // Extension-hint extend here once more bus controllers become available
  else
  {
    Error("Unknown Controller detected - skipping bus registration.");
    return -1;
  }

  auto& msgHandlerList = mBusMessageHandlerByID[eventType][networkId][true][messageId];
  auto it1 = msgHandlerList.insert(msgHandlerList.end(), reg);

  SubscribeMessageId(eventType, networkId, messageId, true);

  uint32_t handle = ++mCurrentCallbackHandle;
  mUnregisterHandlerMap[handle] = [this, handle, eventType, networkId, messageId, &msgHandlerList, it1]()
  {
    UnregisterBusMessageWithMessageIdInternal(handle, eventType, networkId, true, messageId, msgHandlerList, it1);
  };

  // Reserve name
  auto controllerInstanceName = busController->GetNetworkId() + "::" + busController->GetDeviceId();
  mRemoteAmbassador.reserveObjectInstanceName(
    string2wstring(controllerInstanceName),
    [this, busController](const std::wstring& theObjectInstanceName, bool succeeded)
    {
      OnBusControllerObjectReservationCompleted(wstring2string(theObjectInstanceName), succeeded, busController);
    });
  return handle;
}

void BusService::UnregisterBusMessage(CallbackHandle handle)
{
  auto iter = mUnregisterHandlerMap.find(handle);
  if (iter != mUnregisterHandlerMap.end())
  {
    mUnregisterHandlerMap[handle]();
  }
  else
  {    
    DebugPrintf(SWarn, "%s: unknown callback handle %d\n", __FUNCTION__, handle);
  }
}

// =========================================
// BusMessage internal methods

// precondition: federation has been joined, FOM available
void BusService::InitializeHandles()
{
  // access fields from BusMessage interaction base
  rti::InteractionClassHandle busMessageInteractionClass = mRemoteAmbassador.mRTIAmb->getInteractionClassHandle(FOMInteractions::kBusMessage);
  mBusMessageIsRequestParameter           = mRemoteAmbassador.mRTIAmb->getParameterHandle(busMessageInteractionClass, FOMInteractions::kBusMessage_IsRequest);
  mBusMessageChannelNameParameter         = mRemoteAmbassador.mRTIAmb->getParameterHandle(busMessageInteractionClass, FOMInteractions::kBusMessage_ChannelName);
  mBusMessageBusTypeParameter             = mRemoteAmbassador.mRTIAmb->getParameterHandle(busMessageInteractionClass, FOMInteractions::kBusMessage_BusType);
  mBusMessageRequestingFederateParameter  = mRemoteAmbassador.mRTIAmb->getParameterHandle(busMessageInteractionClass, FOMInteractions::kBusMessage_RequestingFederate);
  mBusMessageSenderParameter              = mRemoteAmbassador.mRTIAmb->getParameterHandle(busMessageInteractionClass, FOMInteractions::kBusMessage_Sender);
  mBusMessageReceiverParameter            = mRemoteAmbassador.mRTIAmb->getParameterHandle(busMessageInteractionClass, FOMInteractions::kBusMessage_Receiver);

  // fill maps EventType <=> InteractionClassHandle
  for (VEventType et : kEventTypes)
  {
    const wchar_t* name = GetMessageInteractionClassName(et);
    rti::InteractionClassHandle handle = mRemoteAmbassador.mRTIAmb->getInteractionClassHandle(name);
    mInteractionByEventType[et] = handle;
    mEventTypeByInteraction[handle] = et;
  }

  // Bus Management
  // base management objects/attributes
  mBusManagementObjClassHandle  = mRemoteAmbassador.mRTIAmb->getObjectClassHandle(FOMObjects::kBusManagement);
  mBusManagementNetworkId       = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusManagementObjClassHandle, FOMObjects::kBusManagement_Attr_NetworkID);

  // CAN management objects/attributes
  mBusManagementCanObjClassHandle   = mRemoteAmbassador.mRTIAmb->getObjectClassHandle(FOMObjects::kBusManagement_CAN);
  mBusManagementCanBusState         = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusManagementCanObjClassHandle, FOMObjects::kBusManagement_CAN_Attr_BusState);
  mBusManagementCanTxErrorCount     = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusManagementCanObjClassHandle, FOMObjects::kBusManagement_CAN_Attr_TxErrorCount);
  mBusManagementCanRxErrorCount     = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusManagementCanObjClassHandle, FOMObjects::kBusManagement_CAN_Attr_RxErrorCount);
  mBusManagementCanSendMessagesAsRx = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusManagementCanObjClassHandle, FOMObjects::kBusManagement_CAN_Attr_SendMessagesAsRx);

  // Bus Controller
  // base controller objects/attributes
  mBusControllerObjClassHandle  = mRemoteAmbassador.mRTIAmb->getObjectClassHandle(FOMObjects::kBusController);
  mBusControllerNetworkId       = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusControllerObjClassHandle, FOMObjects::kBusController_Attr_NetworkID);
  mBusControllerDeviceId        = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusControllerObjClassHandle, FOMObjects::kBusController_Attr_DeviceID);

  // CAN controller objects/attributes
  mBusControllerCanObjClassHandle     = mRemoteAmbassador.mRTIAmb->getObjectClassHandle(FOMObjects::kBusController_CAN);
  mBusControllerCanBaudRate           = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusControllerCanObjClassHandle, FOMObjects::kBusController_CAN_Attr_BaudRate);
  mBusControllerCanDataBaudRate       = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusControllerCanObjClassHandle, FOMObjects::kBusController_CAN_Attr_DataBaudRate);
  mBusControllerCanPreScaler          = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusControllerCanObjClassHandle, FOMObjects::kBusController_CAN_Attr_PreScaler);
  mBusControllerCanOperationMode      = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusControllerCanObjClassHandle, FOMObjects::kBusController_CAN_Attr_OperationMode);
  mBusControllerCanSync_Seg           = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusControllerCanObjClassHandle, FOMObjects::kBusController_CAN_Attr_Sync_Seg);
  mBusControllerCanProp_Seg           = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusControllerCanObjClassHandle, FOMObjects::kBusController_CAN_Attr_Prop_Seg);
  mBusControllerCanPhase_Seg1         = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusControllerCanObjClassHandle, FOMObjects::kBusController_CAN_Attr_Phase_Seg1);
  mBusControllerCanPhase_Seg2         = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusControllerCanObjClassHandle, FOMObjects::kBusController_CAN_Attr_Phase_Seg2);
  mBusControllerCanSamplingMode       = mRemoteAmbassador.mRTIAmb->getAttributeHandle(mBusControllerCanObjClassHandle, FOMObjects::kBusController_CAN_Attr_SamplingMode);
}

// set up publish subscribe for all bus messages registered meanwhile (before join)
// Please note that d
void BusService::PublishSubscribe()
{
  // By (current) definition, a bus management must be registered AFTER the federation was joined
  // Thus, it is not necessary to iterate through the bus management handlers at this point in time
  // REVIEW: is this ok for now? Otherwise we buffers for the management (e.g., because we cannot directly create the needed HLA objects)
  for (auto& eventTypeEntry : mBusMessageHandler)
  {
    auto eventType = eventTypeEntry.first;
    for (auto& networkTableEntry : eventTypeEntry.second)
    {
      NetworkId networkId = networkTableEntry.first;
      if (!networkTableEntry.second.empty())
        for (auto& isRequestEntry : networkTableEntry.second)
        {
          bool isRequest = isRequestEntry.first;
          if (!isRequestEntry.second.empty())
          {
            SubscribeNetwork(eventType, networkId, isRequest);
          }
        }
    }
  }

  for (auto& eventTypeEntry : mBusMessageHandlerByID)
  {
    auto eventType = eventTypeEntry.first;
    for (auto& networkTableEntry : eventTypeEntry.second)
    {
      auto networkId = networkTableEntry.first;
      if (!networkTableEntry.second.empty())
        for (auto& isRequestEntry : networkTableEntry.second)
        {
          bool isRequest = isRequestEntry.first;
          for (auto& idTableEntry : isRequestEntry.second)
          {
            auto messageId = idTableEntry.first;
            auto& registeredBusMessages = idTableEntry.second;
            if (!registeredBusMessages.empty())
            {
              SubscribeMessageId(eventType, networkId, messageId, isRequest);
            }
          }
        }
    }
  }
  for (auto requestedPublication : mRequestedBusMessagePublications)
  {
    if (requestedPublication.second > 0)
    {
      Publish(requestedPublication.first);
    }
  }

  // bus management
  // Note remember to include attributes from base class (e.g., NetworkID)
  rti::AttributeHandleSet attributesOfBusManagementCanClass;
  attributesOfBusManagementCanClass.insert(mBusManagementNetworkId);
  attributesOfBusManagementCanClass.insert(mBusManagementCanBusState);
  attributesOfBusManagementCanClass.insert(mBusManagementCanTxErrorCount);
  attributesOfBusManagementCanClass.insert(mBusManagementCanRxErrorCount);
  attributesOfBusManagementCanClass.insert(mBusManagementCanSendMessagesAsRx);
  mRemoteAmbassador.mRTIAmb->publishObjectClassAttributes(mBusManagementCanObjClassHandle, attributesOfBusManagementCanClass);

  mRemoteAmbassador.subscribeObjectClassAttributes(mBusManagementCanObjClassHandle, attributesOfBusManagementCanClass, this);
  mRemoteAmbassador.mRTIAmb->setObjectClassDeliverToSelf(mBusManagementCanObjClassHandle, false);
  mRemoteAmbassador.registerAttributeRequestProvider(mBusManagementCanObjClassHandle, this);

  // bus controller
  // Note remember to include attributes from base class (e.g., NetworkID, DeviceID)
  rti::AttributeHandleSet attributesOfBusControllerCanClass;
  attributesOfBusControllerCanClass.insert(mBusControllerNetworkId);
  attributesOfBusControllerCanClass.insert(mBusControllerDeviceId);
  attributesOfBusControllerCanClass.insert(mBusControllerCanBaudRate);
  attributesOfBusControllerCanClass.insert(mBusControllerCanDataBaudRate);
  attributesOfBusControllerCanClass.insert(mBusControllerCanPreScaler);
  attributesOfBusControllerCanClass.insert(mBusControllerCanOperationMode);
  attributesOfBusControllerCanClass.insert(mBusControllerCanSync_Seg);
  attributesOfBusControllerCanClass.insert(mBusControllerCanProp_Seg);
  attributesOfBusControllerCanClass.insert(mBusControllerCanPhase_Seg1);
  attributesOfBusControllerCanClass.insert(mBusControllerCanPhase_Seg2);
  attributesOfBusControllerCanClass.insert(mBusControllerCanSamplingMode);
  mRemoteAmbassador.mRTIAmb->publishObjectClassAttributes(mBusControllerCanObjClassHandle, attributesOfBusControllerCanClass);

  mRemoteAmbassador.subscribeObjectClassAttributes(mBusControllerCanObjClassHandle, attributesOfBusControllerCanClass, this);
  mRemoteAmbassador.mRTIAmb->setObjectClassDeliverToSelf(mBusControllerCanObjClassHandle, false);
  mRemoteAmbassador.registerAttributeRequestProvider(mBusControllerCanObjClassHandle, this);

  std::vector<std::tuple<std::shared_ptr<NRTFederateEncoding::IBusManagement>, std::shared_ptr<IBusManagementHandler>>> tmpList;
  // collect objects from previous simulation runs
  for (auto mgmtByNetworkIdIter  = mBusManagementHandler.begin(); mgmtByNetworkIdIter != mBusManagementHandler.end(); ++mgmtByNetworkIdIter)
  {
    auto networkId = mgmtByNetworkIdIter->first;
    auto busManagementIter = mBusManagementByIdentifier.find(networkId);
    assert(busManagementIter != mBusManagementByIdentifier.end());
    for (auto handlerListIter = mgmtByNetworkIdIter->second.begin(); handlerListIter != mgmtByNetworkIdIter->second.end(); ++handlerListIter)
    {
      auto handler = handlerListIter->get()->mMgmtHandler;
      tmpList.emplace_back(std::make_tuple(busManagementIter->second, handler));

    }
  }
  // reset handler lists before recovery
  mBusManagementHandler.clear();
  mBusManagementByIdentifier.clear();

  for (auto entryIter = tmpList.begin(); entryIter != tmpList.end(); ++entryIter)
  {
    RegisterBusManagement(std::get<0>(*entryIter), (std::get<1>(*entryIter)));
  }
}

void BusService::AddPublisher(VEventType eventType)
{
  auto iter = mRequestedBusMessagePublications.find(eventType);
  if (iter == mRequestedBusMessagePublications.end())
  {
    iter = mRequestedBusMessagePublications.insert(std::make_pair(eventType, 0)).first;
  }
  iter->second++;
  Publish(eventType);
}

void BusService::Publish(VEventType eventType)
{
  if (IsFederationJoined() && !IsEventPublished(eventType))
  {
    auto interactionClassHandle = GetInteractionClassByEventType(eventType);
    // We publish Tx Requests (TODO: remember publications and call only once)
    mRemoteAmbassador.mRTIAmb->publishInteractionClass(interactionClassHandle);
    mBusMessagePublications.insert(eventType);
  }
}
void BusService::RemovePublisher(VEventType eventType)
{
  auto iter = mRequestedBusMessagePublications.find(eventType);
  if (iter == mRequestedBusMessagePublications.end() || iter->second == 0)
  {
    return;
  }
  iter->second--;
  if (iter->second == 0)
  {
    Unpublish(eventType);
  }
}

void BusService::Unpublish(VEventType eventType)
{
  if (IsFederationJoined() && IsEventPublished(eventType))
  {
    auto interactionClassHandle = GetInteractionClassByEventType(eventType);
    // We publish Tx Requests (TODO: remember publications and call only once)
    mRemoteAmbassador.mRTIAmb->unpublishInteractionClass(interactionClassHandle);
    mBusMessagePublications.erase(eventType);
  }
}

void BusService::SubscribeNetwork(VEventType eventType, NetworkId networkId, bool subscribeRequest)
{
  // TODO: IsNetworkRegistered
  auto& msgHandlerList = mBusMessageHandler[eventType][networkId][subscribeRequest];
  if (msgHandlerList.empty())
  {
    return;
  }
  if (IsFederationJoined() /*&& !IsNetworkSubscribed(eventType, networkId)*/)
  {
    auto interactionClassHandle = GetInteractionClassByEventType(eventType);
    // set up filtered interaction
    rti::ParameterHandleValueMap filterParameters;
    // IMPORTANT - we want to receive rx messages only
    // our own TX messages are delivered always (filtering is always inactive for self-delivery)
    filterParameters[mBusMessageIsRequestParameter] = rti::HLAboolean(subscribeRequest).encode();
    // for now, receive from any channel
    filterParameters[mBusMessageBusTypeParameter] = rti::HLAinteger32LE(GetBusType(eventType)).encode();
    filterParameters[mBusMessageChannelNameParameter] = rti::HLAASCIIstring(networkId).encode();
    // TODO FIXME need access to object instance handle to subscribe to correct interactions
    //filterParameters[mBusMessageReceiverParameter] = mRemoteAmbassador.mFederateHandle.encode();
    DebugPrintf(SInfo, "%s: subscribe interaction class=%S (name=%S)\n", __FUNCTION__, interactionClassHandle.toString().c_str(), mRemoteAmbassador.mRTIAmb->getInteractionClassName(interactionClassHandle).c_str());
    DebugPrintf(SDebug, "%s: subscribe isRequest=%d busType=%d networkId=%s\n", __FUNCTION__, subscribeRequest, GetBusType(eventType), networkId.c_str());
    mRemoteAmbassador.subscribeInteractionClassWithFilter(interactionClassHandle, filterParameters, this);
    mRemoteAmbassador.mRTIAmb->setInteractionClassDeliverToSelf(interactionClassHandle, true);

    // multiple subscriptions can occur if a federate is simulation participant and bus simulator
    if (mBusMessageSubscriptions[eventType].find(networkId) == mBusMessageSubscriptions[eventType].end())
    {
      mBusMessageSubscriptions[eventType].insert(networkId);
    }
  }
}

void BusService::SubscribeMessageId(VEventType eventType, NetworkId networkId, MessageId messageId, bool subscribeRequest)
{
  auto& msgHandlerList = mBusMessageHandlerByID[eventType][networkId][subscribeRequest][messageId];

  if (!msgHandlerList.empty())
  {
    // if the network has already been subscribed, there is no need to subscribe again for a specific id
    if (IsFederationJoined() && !IsMessageSubscribed(eventType, networkId, messageId))
    {
      auto interactionClassHandle = GetInteractionClassByEventType(eventType);
      // set up filtered interaction
      rti::ParameterHandleValueMap filterParameters;
      // IMPORTANT - we want to receive rx messages only
      // our own TX messages are delivered always (filtering is always inactive for self-delivery) // TODO REMARK: TX->TX-Req?
      filterParameters[mBusMessageIsRequestParameter] = rti::HLAboolean(subscribeRequest).encode();
      // for now, receive from any channel
      filterParameters[mBusMessageBusTypeParameter] = rti::HLAinteger32LE(GetBusType(eventType)).encode();
      filterParameters[mBusMessageChannelNameParameter] = rti::HLAASCIIstring(networkId).encode();
      // TODO FIXME see above
      //filterParameters[mBusMessageReceiverParameter] = mRemoteAmbassador.mFederateHandle.encode();
      try
      {
        rti::ParameterHandle idParameter = mRemoteAmbassador.mRTIAmb->getParameterHandle(interactionClassHandle, FOMInteractions::kBusMessageSubclass_Id);
        filterParameters[idParameter] = rti::HLAinteger32LE(messageId).encode();
        DebugPrintf(SDebug, "%s: subscribe isRequest=%d busType=%d networkId=%d messageId=%d\n", __FUNCTION__, true, GetBusType(eventType), networkId.c_str(), messageId);
      }
      catch (const rti::NameNotFound&)
      {
        DebugPrintf(SDebug, "%s: subscribe isRequest=%d busType=%d networkId=%d\n", __FUNCTION__, true, GetBusType(eventType), networkId.c_str());
        // ignore
      }
      mRemoteAmbassador.subscribeInteractionClassWithFilter(interactionClassHandle, filterParameters, this);
      mRemoteAmbassador.mRTIAmb->setInteractionClassDeliverToSelf(interactionClassHandle, true);
      mBusMessageSubscriptionsByID[eventType][networkId].insert(messageId);
    }
  }
}

void BusService::UnsubscribeNetwork(VEventType eventType, NetworkId networkId, bool isRequest)
{
  if (IsFederationJoined() && IsNetworkSubscribed(eventType, networkId))
  {
    auto interactionClassHandle = GetInteractionClassByEventType(eventType);
    // set up interaction filter parameters
    rti::ParameterHandleValueMap filterParameters;
    // IMPORTANT - we want to receive rx messages only
    // our own TX messages are delivered always (filtering is always inactive for self-delivery) // TODO REMARK: TX -> TX-Req?
    filterParameters[mBusMessageIsRequestParameter] = rti::HLAboolean(isRequest).encode();
    // for now, receive from any channel
    filterParameters[mBusMessageBusTypeParameter] = rti::HLAinteger32LE(GetBusType(eventType)).encode();
    filterParameters[mBusMessageChannelNameParameter] = rti::HLAASCIIstring(networkId).encode();
    // TODO FIXME see above
    //filterParameters[mBusMessageReceiverParameter] = mRemoteAmbassador.mFederateHandle.encode();
    // NOT: mRemoteAmbassador.mRTIAmb->setInteractionClassDeliverToSelf(interactionClassHandle, false);
    DebugPrintf(SDebug, "%s: unsubscribe isRequest=%d busType=%d networkId=%s\n", __FUNCTION__, true, GetBusType(eventType), networkId.c_str());

    try
    {
      mRemoteAmbassador.unsubscribeInteractionClassWithFilter(interactionClassHandle, filterParameters, this);
    }
    catch (rti::Exception & e)
    {
      Error("%s: %S\n", __FUNCTION__, e.what().c_str());
      (void)e;
    }

    mBusMessageSubscriptions[eventType].erase(networkId);
  }
}

void BusService::UnsubscribeMessageId(VEventType eventType, NetworkId networkId, uint32_t messageId, bool isRequest)
{
  // first check if there is any id specific subscriber for this network at all
  if (IsNetworkSubscribedAny(eventType, networkId))
  {
    if (IsFederationJoined() && IsMessageSubscribed(eventType, networkId, messageId))
    {
      auto interactionClassHandle = GetInteractionClassByEventType(eventType);
      // set up interaction filter parameters
      rti::ParameterHandleValueMap filterParameters;
      filterParameters[mBusMessageIsRequestParameter] = rti::HLAboolean(isRequest).encode();
      // for now, receive from any channel
      filterParameters[mBusMessageBusTypeParameter] = rti::HLAinteger32LE(GetBusType(eventType)).encode();
      filterParameters[mBusMessageChannelNameParameter] = rti::HLAASCIIstring(networkId).encode();
      // TODO FIXME see above
      //filterParameters[mBusMessageReceiverParameter] = mRemoteAmbassador.mFederateHandle.encode();
      // NOT: mRemoteAmbassador.mRTIAmb->setInteractionClassDeliverToSelf(interactionClassHandle, false);
      try
      {
        rti::ParameterHandle idParameter = mRemoteAmbassador.mRTIAmb->getParameterHandle(interactionClassHandle, FOMInteractions::kBusMessageSubclass_Id);
        filterParameters[idParameter] = rti::HLAinteger32LE(messageId).encode();
        DebugPrintf(SDebug, "%s: unsubscribe isRequest=%d busType=%d networkId=%d messageId=%d\n", __FUNCTION__, true, GetBusType(eventType), networkId.c_str(), messageId);
      }
      catch (const rti::NameNotFound&)
      {
        DebugPrintf(SDebug, "%s: unsubscribe isRequest=%d busType=%d networkId=%d\n", __FUNCTION__, true, GetBusType(eventType), networkId.c_str());
        // ignore
      }
      mRemoteAmbassador.unsubscribeInteractionClassWithFilter(interactionClassHandle, filterParameters, this);

      mBusMessageSubscriptionsByID[eventType][networkId].erase(messageId);
      if (mBusMessageSubscriptionsByID[eventType][networkId].empty() && mBusMessageSubscriptions[eventType].find(networkId) == mBusMessageSubscriptions[eventType].end())
      {
        UnsubscribeNetwork(eventType, networkId, isRequest);
      }
    }
  }
}

bool BusService::IsEventPublished(VEventType eventType)
{
  return mBusMessagePublications.find(eventType) != mBusMessagePublications.end();
}

bool BusService::IsNetworkSubscribed(VEventType eventType, NetworkId networkId)
{
  auto networkSubscriptionsByEventTypeIter = mBusMessageSubscriptions.find(eventType);
  if (networkSubscriptionsByEventTypeIter != mBusMessageSubscriptions.end())
  {
    auto& networkSubscriptions = networkSubscriptionsByEventTypeIter->second;
    if (networkSubscriptions.find(networkId) != networkSubscriptions.end())
    {
      return true;
    }
  }
  return false;
}

bool BusService::IsNetworkSubscribedAny(VEventType eventType, NetworkId networkId)
{
  auto networkSubscriptionsByEventTypeIter = mBusMessageSubscriptions.find(eventType);
  if (networkSubscriptionsByEventTypeIter != mBusMessageSubscriptions.end())
  {
    auto& networkSubscriptions = networkSubscriptionsByEventTypeIter->second;
    if (networkSubscriptions.find(networkId) != networkSubscriptions.end())
    {
      return true;
    }
  }
  auto messageIdSubscriptionsByEventTypeIter = mBusMessageSubscriptionsByID.find(eventType);
  if (messageIdSubscriptionsByEventTypeIter != mBusMessageSubscriptionsByID.end())
  {
    auto& messageIdSubscriptionsByNetwork = messageIdSubscriptionsByEventTypeIter->second;
    auto messageIdSubscriptionsByNetworkIter = messageIdSubscriptionsByNetwork.find(networkId);
    if (messageIdSubscriptionsByNetworkIter != messageIdSubscriptionsByNetwork.end())
    {
      if (!messageIdSubscriptionsByNetworkIter->second.empty())
      {
        return true;
      }
    }
  }
  return false;
}

bool BusService::IsMessageSubscribed(VEventType eventType, NetworkId networkId, MessageId messageId)
{
  auto messageIdSubscriptionsByEventTypeIter = mBusMessageSubscriptionsByID.find(eventType);
  if (messageIdSubscriptionsByEventTypeIter != mBusMessageSubscriptionsByID.end())
  {
    auto& messageIdSubscriptionsByNetwork = messageIdSubscriptionsByEventTypeIter->second;
    auto messageIdSubscriptionsByNetworkIter = messageIdSubscriptionsByNetwork.find(networkId);
    if (messageIdSubscriptionsByNetworkIter != messageIdSubscriptionsByNetwork.end())
    {
      auto& messageIdSubscriptions = messageIdSubscriptionsByNetworkIter->second;
      return messageIdSubscriptions.find(messageId) != messageIdSubscriptions.end();
    }
  }
  return false;
}

bool BusService::IsFederationJoined()
{
  return (mRemoteAmbassador.GetState() == RemoteAmbassador::AmbassadorState::Online || 
    mRemoteAmbassador.GetState() == RemoteAmbassador::AmbassadorState::InitFederate ||
    mRemoteAmbassador.GetState() == RemoteAmbassador::AmbassadorState::JoinedFederation);
}

void BusService::UnregisterBusMessageInternal(CallbackHandle handle, VEventType eventType, 
  NetworkId networkId, bool isTxReq, BusMessageHandlerList& msgHandlerList, BusMessageHandlerList::iterator where)
{
  std::shared_ptr<VRTFederateRegisteredBusMsg> busMsg = *where;
  msgHandlerList.erase(where);
  if (msgHandlerList.empty())
  {
    // no more callbacks for this (networkID, isTxReq) -> remove [isTxReq]
    mBusMessageHandler[eventType][networkId].erase(isTxReq);
    UnsubscribeNetwork(eventType, networkId, isTxReq);
    // if there are no more callbacks at all, remove networkID as well
    if (mBusMessageHandler[eventType][networkId].empty())
    {
      mBusMessageHandler[eventType].erase(networkId);
    }
  }
  // FIXME Mth: if (msgHandlerList.empty()) mParent.mRemoteAmbassador.mRTIAmb->unsubscribeWithFilter
  if (mBusMessageHandler[eventType].empty() && mBusMessageHandlerByID[eventType].empty())
  {
    // there is no message receiver left for this message type, unsubscribe the interaction as a whole
    mRemoteAmbassador.mRTIAmb->unsubscribeInteractionClass(GetInteractionClassByEventType(eventType));
  }
}

void BusService::UnregisterBusMessageWithMessageIdInternal(CallbackHandle handle, VEventType eventType, 
  NetworkId networkId, bool isTxReq, MessageId messageId, BusMessageHandlerList& msgHandlerList, BusMessageHandlerList::iterator where)
{
  std::shared_ptr<VRTFederateRegisteredBusMsg> busMsg = *where;
  msgHandlerList.erase(where);
  mUnregisterHandlerMap.erase(handle);
  if (msgHandlerList.empty())
  {
    mBusMessageHandlerByID[eventType][networkId][isTxReq].erase(messageId);
    UnsubscribeMessageId(eventType, networkId, messageId, isTxReq);
    if (mBusMessageHandlerByID[eventType][networkId][isTxReq].empty())
    {
      // if [isTxReq] does not have any callbacks left, then delete it
      mBusMessageHandlerByID[eventType][networkId].erase(isTxReq);
      if (mBusMessageHandlerByID[eventType][networkId].empty())
      {
        // if [networkID] does not have any callbacks left, delete it
        mBusMessageHandlerByID[eventType].erase(networkId);
      }
    }
  }
  if (mBusMessageHandler[eventType].empty() && mBusMessageHandlerByID[eventType].empty())
  {
    // there is no registered subscriber left for the VEventType, unsubscribe the whole interaction
    mRemoteAmbassador.mRTIAmb->unsubscribeInteractionClass(GetInteractionClassByEventType(eventType));
  }
}

void BusService::UnregisterBusManagementInternal(CallbackHandle handle, NetworkId networkId,
  BusManagementHandlerList& mgmtHandlerList, BusManagementHandlerList::iterator where)
{
  std::shared_ptr<VRTFederateRegisteredBusMgmt> busMsg = *where;
  mgmtHandlerList.erase(where);
  if (mgmtHandlerList.empty())
  {
    mBusManagementHandler.erase(networkId);
  }
}

const wchar_t* BusService::GetMessageInteractionClassName(VEventType eventType)
{
  switch (eventType)
  {
    case VEventType::kEvEthernetStatus:
      return EventClassOfEventType<VEventType::kEvEthernetStatus>::kInteractionClassName;
    case VEventType::kEvEthernetPacket:
      return EventClassOfEventType<VEventType::kEvEthernetPacket>::kInteractionClassName;
    case VEventType::kEvEthernetPacketForwarded:
      return EventClassOfEventType<VEventType::kEvEthernetPacketForwarded>::kInteractionClassName;
    case VEventType::kEvEthernetPacketError:
      return EventClassOfEventType<VEventType::kEvEthernetPacketError>::kInteractionClassName;
    case VEventType::kEvEthernetPacketErrorForwarded:
      return EventClassOfEventType<VEventType::kEvEthernetPacketErrorForwarded>::kInteractionClassName;
    case VEventType::kEvCanMessage:
      return EventClassOfEventType<VEventType::kEvCanMessage>::kInteractionClassName;
    case VEventType::kEvErrorFrame:
      return EventClassOfEventType<VEventType::kEvErrorFrame>::kInteractionClassName;
    case VEventType::kEvInvalid: //bkd: Clang error "enumeration value not handled in switch"
    default:
      assert(0);
  }
  return nullptr;
}

NRTFederateEncoding::BusType BusService::GetBusType(VEventType eventType)
{
  switch (eventType)
  {
    case VEventType::kEvEthernetStatus:
      return EventClassOfEventType<VEventType::kEvEthernetStatus>::kBusType;
    case VEventType::kEvEthernetPacket:
      return EventClassOfEventType<VEventType::kEvEthernetPacket>::kBusType;
    case VEventType::kEvEthernetPacketForwarded:
      return EventClassOfEventType<VEventType::kEvEthernetPacketForwarded>::kBusType;
    case VEventType::kEvEthernetPacketError:
      return EventClassOfEventType<VEventType::kEvEthernetPacketError>::kBusType;
    case VEventType::kEvEthernetPacketErrorForwarded:
      return EventClassOfEventType<VEventType::kEvEthernetPacketErrorForwarded>::kBusType;
    case VEventType::kEvCanMessage:
      return EventClassOfEventType<VEventType::kEvCanMessage>::kBusType;
    case VEventType::kEvErrorFrame:
      return EventClassOfEventType<VEventType::kEvErrorFrame>::kBusType;
    case VEventType::kEvInvalid:
    case VEventType::kEvLinMessage:
      return NRTFederateEncoding::BusType::kBtInvalid;
  }
  return NRTFederateEncoding::BusType::kBtInvalid;
}

VEventType BusService::GetEventType(rti::InteractionClassHandle handle) const
{
  assert(handle.isValid());
  auto iter = mEventTypeByInteraction.find(handle);
  if (iter != mEventTypeByInteraction.end())
  {
    return iter->second;
  }
  return VEventType::kEvInvalid;
}

rti::InteractionClassHandle BusService::GetInteractionClassByEventType(VEventType eventType) const
{
  auto iter = mInteractionByEventType.find(eventType);
  if (iter != mInteractionByEventType.end())
  {
    return iter->second;
  }
  assert(false);
  return rti::InteractionClassHandle();
}

/*
template<VEventType eventType>
std::unique_ptr<typename EventClassOfEventType<eventType>::EncoderClass> GetBusEventT()
{
  //return std::make_unique<EventClassOfEventType<eventType>::EncoderClass>();
  EventClassOfEventType<eventType>::EncoderClass* result = new EventClassOfEventType<eventType>::EncoderClass();
  return std::unique_ptr<typename EventClassOfEventType<eventType>::EncoderClass>(result);
}
*/
std::unique_ptr<NRTFederateEncoding::BusEvent> BusService::GetBusMessage(VEventType eventType)
{
  switch (eventType)
  {
    case VEventType::kEvEthernetPacket:
      return std::unique_ptr<NRTFederateEncoding::BusEvent>(new NRTFederateEncoding::EthernetPacketEncoding);
    case VEventType::kEvEthernetStatus:
      return std::unique_ptr<NRTFederateEncoding::BusEvent>(new NRTFederateEncoding::EthernetStatusEncoding);
    case VEventType::kEvEthernetPacketForwarded:
      return std::unique_ptr<NRTFederateEncoding::BusEvent>(new NRTFederateEncoding::EthernetPacketForwardedEncoding);
    case VEventType::kEvEthernetPacketError:
      return std::unique_ptr<NRTFederateEncoding::BusEvent>(new NRTFederateEncoding::EthernetPacketErrorEncoding);
    case VEventType::kEvEthernetPacketErrorForwarded:
      return std::unique_ptr<NRTFederateEncoding::BusEvent>(new NRTFederateEncoding::EthernetPacketErrorForwardedEncoding);
    case VEventType::kEvCanMessage:
      return std::unique_ptr<NRTFederateEncoding::BusEvent>(new NRTFederateEncoding::CANFrameEncodingInternal);
    case VEventType::kEvErrorFrame:
      return std::unique_ptr<NRTFederateEncoding::BusEvent>(new NRTFederateEncoding::CANErrorFrameEncoding);
    case VEventType::kEvLinMessage:
    case VEventType::kEvInvalid:
      return std::unique_ptr<NRTFederateEncoding::BusEvent>();
  }
  return std::unique_ptr<NRTFederateEncoding::BusEvent>();
}

BusService::BusEventEncoder BusService::GetBusMessageEncoder(VEventType eventType)
{
  switch (eventType)
  {
    case VEventType::kEvEthernetStatus:
      return EventClassOfEventType<VEventType::kEvEthernetStatus>::encode;
    case VEventType::kEvEthernetPacket:
      return EventClassOfEventType<VEventType::kEvEthernetPacket>::encode;
    case VEventType::kEvEthernetPacketForwarded:
      return EventClassOfEventType<VEventType::kEvEthernetPacketForwarded>::encode;
    case VEventType::kEvEthernetPacketError:
      return EventClassOfEventType<VEventType::kEvEthernetPacketError>::encode;
    case VEventType::kEvEthernetPacketErrorForwarded:
      return EventClassOfEventType<VEventType::kEvEthernetPacketErrorForwarded>::encode;
    case VEventType::kEvCanMessage:
      return EventClassOfEventType<VEventType::kEvCanMessage>::encode;
    case VEventType::kEvErrorFrame:
      return EventClassOfEventType<VEventType::kEvErrorFrame>::encode;
    case VEventType::kEvLinMessage:
    case VEventType::kEvInvalid:
      return nullptr;
  }
  return nullptr;
}

BusService::BusEventDecoder BusService::GetBusMessageDecoder(VEventType eventType)
{
  switch (eventType)
  {
    case VEventType::kEvEthernetStatus:
      return EventClassOfEventType<VEventType::kEvEthernetStatus>::decode;
    case VEventType::kEvEthernetPacket:
      return EventClassOfEventType<VEventType::kEvEthernetPacket>::decode;
    case VEventType::kEvEthernetPacketForwarded:
      return EventClassOfEventType<VEventType::kEvEthernetPacketForwarded>::decode;
    case VEventType::kEvEthernetPacketError:
      return EventClassOfEventType<VEventType::kEvEthernetPacketError>::decode;
    case VEventType::kEvEthernetPacketErrorForwarded:
      return EventClassOfEventType<VEventType::kEvEthernetPacketErrorForwarded>::decode;
    case VEventType::kEvCanMessage:
      return EventClassOfEventType<VEventType::kEvCanMessage>::decode;
    case VEventType::kEvErrorFrame:
      return EventClassOfEventType<VEventType::kEvErrorFrame>::decode;
    case VEventType::kEvLinMessage:
    case VEventType::kEvInvalid:
      return nullptr;
  }
  return nullptr;
}

std::unique_ptr<NRTFederateEncoding::CANFrame> BusService::CreateCANFrame(uint32_t id, std::vector<uint8_t> data) const
{
  auto msg = std::unique_ptr<NRTFederateEncoding::CANFrameEncodingInternal>(new NRTFederateEncoding::CANFrameEncodingInternal());

  msg->SetId(id);
  msg->SetDataLength((uint8_t)(data.size()));
  msg->SetData(data);
  return msg;
}

std::unique_ptr<NRTFederateEncoding::CANFrame> BusService::CreateCANFrame(const NRTFederateEncoding::BusEvent& message) const
{
  try {
    auto targetType = dynamic_cast<const NRTFederateEncoding::CANFrameEncodingInternal&>(message);
    auto msg = std::unique_ptr<NRTFederateEncoding::CANFrameEncodingInternal>(new NRTFederateEncoding::CANFrameEncodingInternal(targetType));
    return std::move(msg);
  }
  catch (const std::bad_cast& e) 
  {
    // Cast failed
    (void)e;
    Error("%s: Failed to create a can frame with the provided message.", __FUNCTION__);
    return nullptr;
  }
}

std::shared_ptr<NRTFederateEncoding::IBusController> BusService::CreateCanBusController(NetworkId channelName)
{
  return CreateCanBusController(channelName, mRemoteAmbassador.mConfig.FederateName, toString(UUIDT4::generate()));
}

std::shared_ptr<NRTFederateEncoding::IBusController> BusService::CreateCanBusController(NetworkId channelName, std::string deviceName, std::string controllerName)
{
  std::shared_ptr<NRTFederateEncoding::CanBusControllerEncoding> controller = std::make_shared<NRTFederateEncoding::CanBusControllerEncoding>();
  controller->SetSelfReference(controller);
  controller->SetNetworkId("CAN::" + channelName);
  controller->SetDeviceId(deviceName + "-" + controllerName);
  controller->SetBusService(this);

  return controller;
}

std::shared_ptr<NRTFederateEncoding::ICanBusManagement> BusService::CreateCanBusManagement(NetworkId channelName)
{
  auto mgmt = std::make_shared<NRTFederateEncoding::CanBusManagementInternal>();
  mgmt->SetNetworkId("CAN::" + channelName);
  return mgmt;
}

BusService::CANMessageSender::CANMessageSender(BusService &parent, NetworkId networkId) : mParent(parent), mNetworkId(networkId)
{
  mParent.AddPublisher(VEventType::kEvCanMessage);
}

BusService::CANMessageSender::~CANMessageSender()
{
  // TODO: this causes a crash in ~BusService, check if needed
  //mParent.RemovePublisher(VEventType::kEvCanMessage);
}

#define STRING2(x) #x
#define STRING(x) STRING2(x)

bool BusService::CANMessageSender::send(std::unique_ptr<NRTFederateEncoding::BusEvent> event, const rti::ObjectInstanceHandle& senderObjectHandle, bool isTxRequest = true)
{
  // #pragma message(__FILE__ ":" STRING(__LINE__) ": CRASHES" )
  //const std::lock_guard<std::mutex> lock(mParent.mRemoteAmbassador.mMutEvokeCallbacks);
  if (mParent.mRemoteAmbassador.GetState() == RemoteAmbassador::AmbassadorState::Online)
  {
    /*auto d = static_cast<Derived*>(p.release());
    return std::unique_ptr<Derived, Del>(d, std::move(p.get_deleter()));*/

    //NRTFederateEncoding::CANFrameEncodingInternal* tmpPtr = static_cast<NRTFederateEncoding::CANFrameEncodingInternal*>(event.get());

    /*auto canFrame = std::unique_ptr<NRTFederateEncoding::CANFrameEncodingInternal>(
      std::move(tmpPtr));*/

    auto canFrame = dynamic_cast<NRTFederateEncoding::CANFrameEncodingInternal*>(event.get());
    rti::InteractionClassHandle interactionClass = mParent.GetInteractionClassByEventType(VEventType::kEvCanMessage);
    rti::ParameterHandle frameParameter = mParent.mRemoteAmbassador.mRTIAmb->getParameterHandle(interactionClass, FOMInteractions::kBusMessageSubclass_Frame);
    rti::ParameterHandle messageIdParameter = mParent.mRemoteAmbassador.mRTIAmb->getParameterHandle(interactionClass, FOMInteractions::kBusMessageSubclass_Id);

    // check for active trivial simulator on this channel
    auto find = mParent.mBusManagementByIdentifier.find(mNetworkId);
    if (find == mParent.mBusManagementByIdentifier.end())
    {
      // no bus management found -> do not send anything
      Error("No bus simulator found for channel '%s' -> message discarded.\n", mNetworkId.c_str());
      return false;
    }
    else
    {
      std::shared_ptr<NRTFederateEncoding::IBusManagement> mgmt = find->second;
      auto specialMgmt = std::dynamic_pointer_cast<NRTFederateEncoding::IBusManagementInternal>(mgmt);
      // bus management found -> is it initialized?
      if (!specialMgmt->GetIsInitialized())
      {
        // distributed bus management detected, but its configuration is not yet known
        // -> send message as if there was no bus management and show error message
        Error("distributed bus simulator on channel '%s' is not ready yet and will be ignored\n", mNetworkId.c_str());
      }
      else
      {
        // management is initialized -> is it a trivial simulator?
        auto busType = mgmt->GetBusType();
        if (busType == NRTFederateEncoding::BusType::kBtCAN)
        {
          auto derivedMgmt = std::dynamic_pointer_cast<NRTFederateEncoding::ICanBusManagement>(mgmt);
          if (derivedMgmt->GetSendMessagesAsRx())
          {
            // trivial simulator detected -> changing frame direction flag
            isTxRequest = false;
            canFrame->SetDir(NRTFederateEncoding::kMskRx);
          }
        }
        // Extension-hint: add more bus managements here once they are available
      }
    }

    // 2020-10-19, Mth: alternatively:
    //auto encoder = GetBusMessageEncoder(VEventType::kEvCanMessage);
    //assert(encoder != nullptr);
    rti::ParameterHandleValueMap parameters;
    parameters[frameParameter] = canFrame->encode();
    parameters[messageIdParameter] = rti::HLAinteger32LE(canFrame->GetId()).encode();
    parameters[mParent.mBusMessageBusTypeParameter] = rti::HLAinteger32LE(NDistSimIB::NRTFederateEncoding::kBtCAN).encode();
    parameters[mParent.mBusMessageChannelNameParameter] = rti::HLAASCIIstring(mNetworkId).encode();
    parameters[mParent.mBusMessageIsRequestParameter] = rti::HLAboolean(isTxRequest).encode();
    assert(mParent.mRemoteAmbassador.mFederateHandle.isValid());
    parameters[mParent.mBusMessageRequestingFederateParameter] = mParent.mRemoteAmbassador.mFederateHandle.encode();
    parameters[mParent.mBusMessageSenderParameter] = senderObjectHandle.encode();
    rti::VariableLengthData tag;
    mParent.mRemoteAmbassador.mRTIAmb->sendInteraction(interactionClass, parameters, tag);
    return true;
  }
  else
  {
    return false;
  }
}

std::unique_ptr<IBusMessageService::IBusMessageSender> BusService::GetMessageRequestSender(VEventType eventType, NetworkId networkId)
{
  if (eventType == VEventType::kEvCanMessage)
  {
    //return std::make_unique<CANMessageSender>(*this, networkId);
    return std::unique_ptr <CANMessageSender>(new CANMessageSender(*this, networkId));
  }
  else
    return std::unique_ptr<IBusMessageService::IBusMessageSender>();
}

// Bus Management methods

void BusService::OnBusManagementObjectReservationCompleted(
  const std::string& instanceName,
  bool wasInstanceReservationSuccessful, 
  std::shared_ptr<NRTFederateEncoding::IBusManagement> busManagement,
  std::shared_ptr<IBusManagementHandler> mgmtHandler)
{
  // assert, if instance name is already reserved
  if (!wasInstanceReservationSuccessful)
  {
    Error("Error: Bus Simulator '%s' already exists!\n", instanceName.c_str());
    return;
  }

  // update all attribute values
  rti::AttributeHandleValueMap attributeValues;

  // figure out what kind of control we actually have
  NRTFederateEncoding::BusType busType = busManagement->GetBusType();
  if (busType == NRTFederateEncoding::BusType::kBtCAN)
  {
    const auto canControl = std::dynamic_pointer_cast<NRTFederateEncoding::ICanBusManagement>(busManagement);
    attributeValues[mBusManagementNetworkId] = rti::HLAASCIIstring(canControl->GetNetworkId()).encode();
    attributeValues[mBusManagementCanBusState] = rti::HLAinteger32LE(canControl->GetBusState()).encode();
    attributeValues[mBusManagementCanTxErrorCount] = rti::HLAinteger32LE(canControl->GetTxErrorCount()).encode();
    attributeValues[mBusManagementCanRxErrorCount] = rti::HLAinteger32LE(canControl->GetTxErrorCount()).encode();
    attributeValues[mBusManagementCanSendMessagesAsRx] = rti::HLAboolean(canControl->GetSendMessagesAsRx()).encode();
  }
  // Extension-hint: if more bus managements are available, then try next cast - otherwise print error and cancel 
  else
  {
    Error("unknown Bus Management detected - aborting!");
    return;
  }

  DebugPrintf(SInfo, "Registering bus management with instance name '%s'\n", instanceName.c_str());
  auto handle = mRemoteAmbassador.mRTIAmb->registerObjectInstance(mBusManagementCanObjClassHandle, string2wstring(instanceName));

  // add object handle to busManagement
  auto internalBusManagement = std::dynamic_pointer_cast<NRTFederateEncoding::IBusManagementInternal>(busManagement);
  internalBusManagement->SetControllerObjectInstanceHandle(handle);
  internalBusManagement->SetIsInitialized(true);
  // store object instance handle
  mBusManagementByIdentifier.insert(std::make_pair(instanceName, busManagement));
  mNetworkNameByObjectHandle[handle] = instanceName;


  rti::HLAinteger64Time sendTime = mRemoteAmbassador.mInternalTimingService->GetEarliestSendTime();
  mRemoteAmbassador.mRTIAmb->updateAttributeValues(handle, attributeValues, rti::VariableLengthData(), sendTime);
}


IBusMessageService::CallbackHandle BusService::RegisterBusManagement(
  std::shared_ptr<NRTFederateEncoding::IBusManagement> busManagement, std::shared_ptr<IBusManagementHandler> mgmtHandler)
{
  if (!IsFederationJoined())
  {
    Error("This federate is not part of a federation yet.");
    return -1;
  }
  uint32_t handle = ++mCurrentCallbackHandle;

  auto busType = busManagement->GetBusType();
  if (busType == NRTFederateEncoding::kBtCAN)
  {
    const auto canControl = std::dynamic_pointer_cast<NRTFederateEncoding::CanBusManagementEncoding>(busManagement);
    auto networkId = canControl->GetNetworkId();

    auto reg = std::make_shared<VRTFederateRegisteredBusMgmt>(networkId, mgmtHandler);
    auto& mgmtHandlerList = mBusManagementHandler[networkId];
    auto it1 = mgmtHandlerList.insert(mgmtHandlerList.end(), reg);

    mUnregisterHandlerMap[handle] = [this, handle, networkId, &mgmtHandlerList, it1]()
    {
      UnregisterBusManagementInternal(handle, networkId, mgmtHandlerList, it1);
    };
  }
  // Reserve name
  mRemoteAmbassador.reserveObjectInstanceName(
    string2wstring(busManagement->GetNetworkId()),
    [this, busManagement, mgmtHandler](const std::wstring& theObjectInstanceName, bool succeeded)
    {
      OnBusManagementObjectReservationCompleted(wstring2string(theObjectInstanceName), succeeded, busManagement, mgmtHandler);
    });
  return handle;
}

void BusService::UnregisterBusManagement(CallbackHandle handle)
{
  auto iter = mUnregisterHandlerMap.find(handle);
  if (iter != mUnregisterHandlerMap.end())
  {
    mUnregisterHandlerMap[handle]();
  }
  else
  {
    DebugPrintf(SWarn, "%s: unknown callback handle %d\n", __FUNCTION__, handle);
  }
}
} // namespace NDistSimIB