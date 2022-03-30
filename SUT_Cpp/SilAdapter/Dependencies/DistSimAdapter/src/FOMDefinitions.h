/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

// =========================================
// Names of HLA interactions + parameters, HLA objects + attributes.
// Must match the Federate obejct model
namespace NDistSimIB {

namespace FOMInteractions {

  // Simulation control
  const wchar_t kMeasurementInit[] = L"HLAinteractionRoot.MeasurementInit";
  const wchar_t kMeasurementInit_Param[] = L"Dummy";                                                      // Optional: Ignored

  const wchar_t kMeasurementStop[] = L"HLAinteractionRoot.MeasurementStop";
  const wchar_t kMeasurementStop_Param_NextFederationSuffix[] = L"NextFederationSuffix";                  // Optional: Ignored

  // Bus messages
  const wchar_t kBusMessage[] = L"HLAinteractionRoot.BusMessage";                                         
  const wchar_t kBusMessage_IsRequest[] = L"IsRequest";                                                   // Mandatory: If Missing: Abort reception
  const wchar_t kBusMessage_ChannelName[] = L"ChannelName";                                               // Mandatory: If Missing: Abort reception
  const wchar_t kBusMessage_BusType[] = L"BusType";                                                       // Optional: No reception: Used for subscription
  const wchar_t kBusMessage_RequestingFederate[] = L"RequestingFederate";                                 // TODO: Unused in IB, but in CANoe
  const wchar_t kBusMessage_Sender[] = L"Sender";                                                         // Optional:  If Missing: Assume not sentBySelf
  const wchar_t kBusMessage_Receiver[] = L"Receiver";                                                     // TODO: Currently Unused
  
  // named identical for all BusMessage subclasses, but must be fetched 
  // from each individual subclass, and has different data type
  const wchar_t kBusMessageSubclass_Frame[] = L"Frame";                                                   // Mandatory: If Missing: Ignore reception
  const wchar_t kBusMessageSubclass_Id[] = L"Id";                                                         // Optional: If Missing: Ignore reception
  const wchar_t kBusMessageCANMessage[] = L"HLAinteractionRoot.BusMessage.CANMessage"; 
  const wchar_t kBusMessageCANErrorFrame[] = L"HLAinteractionRoot.BusMessage.CANErrorFrame";

  // TextLog
  const wchar_t kTextLog[] = L"TextLog";
  const wchar_t kTextLog_Param_Text[] = L"Text";                                                          // Optional:  If Missing: Use IBDefaults::TextLog::Text
  const wchar_t kTextLog_Param_Sender[] = L"Sender";                                                      // Optional:  If Missing: Use IBDefaults::TextLog::Sender


} // namespace FOMInteractions

namespace FOMObjects {
  // DO Sources
  const wchar_t kDOMemberSource[] = L"HLAobjectRoot.DOMemberSource";
  const wchar_t kDOMemberSource_Attr_DOMemberName[] = L"DOSourceMemberName";                              // Mandatory: If Missing: Ignore DO
  const wchar_t kDOMemberSource_Attr_DOMemberConnectionType[] = L"DOSourceMemberConnectionType";          // Optional:  If Missing: Use IBDefaults::DOs::ConnectionType
  const wchar_t kDOMemberSource_Attr_DOMemberLastDataBytes[] = L"DOSourceMemberDataBytes";                // Optional:  If Missing: Don't execute reception callbacks
  
  // DO Targets
  const wchar_t kDOMemberTarget[] = L"HLAobjectRoot.DOMemberTarget";
  const wchar_t kDOMemberTarget_Attr_DOMemberName[] = L"DOTargetMemberName";                              // Mandatory: If Missing: Ignore DO
  const wchar_t kDOMemberTarget_Attr_DOMemberConnectionType[] = L"DOTargetMemberConnectionType";          // Optional:  If Missing: Use IBDefaults::DOs::ConnectionType

  // Abstract Bus Management
  const wchar_t kBusManagement[] = L"HLAobjectRoot.BusManagement";
  const wchar_t kBusManagement_Attr_NetworkID[] = L"NetworkID";                                           // Mandatory: If Missing: BusManager not ready
                                                                                                           
  // CAN Bus Management
  const wchar_t kBusManagement_CAN[] = L"HLAobjectRoot.BusManagement.BusManagementCan";
  const wchar_t kBusManagement_CAN_Attr_BusState[] = L"BusState";                                         // Mandatory: If Missing: BusManager not ready
  const wchar_t kBusManagement_CAN_Attr_TxErrorCount[] = L"TxErrorCount";                                 // Mandatory: If Missing: BusManager not ready
  const wchar_t kBusManagement_CAN_Attr_RxErrorCount[] = L"RxErrorCount";                                 // Mandatory: If Missing: BusManager not ready
  const wchar_t kBusManagement_CAN_Attr_SendMessagesAsRx[] = L"SendMessagesAsRx";                         // Mandatory: If Missing: BusManager not ready

  // Abstract Bus Controller
  const wchar_t kBusController[] = L"HLAobjectRoot.BusController";
  const wchar_t kBusController_Attr_NetworkID[] = L"NetworkID";                                           // TODO: Distribute (via controller object update) for non-trivial BusManager
  const wchar_t kBusController_Attr_DeviceID[] = L"DeviceID";                                             // TODO: Distribute (via controller object update) for non-trivial BusManager
  
  // CAN Bus Controller
  const wchar_t kBusController_CAN[] = L"HLAobjectRoot.BusController.BusControllerCan";                   // TODO: Distribute (via controller object update) for non-trivial BusManager
  const wchar_t kBusController_CAN_Attr_BaudRate[] = L"BaudRate";                                         // TODO: Distribute (via controller object update) for non-trivial BusManager
  const wchar_t kBusController_CAN_Attr_DataBaudRate[] = L"DataBaudRate";                                 // TODO: Distribute (via controller object update) for non-trivial BusManager
  const wchar_t kBusController_CAN_Attr_PreScaler[] = L"PreScaler";                                       // TODO: Distribute (via controller object update) for non-trivial BusManager
  const wchar_t kBusController_CAN_Attr_OperationMode[] = L"OperationMode";                               // TODO: Distribute (via controller object update) for non-trivial BusManager
  const wchar_t kBusController_CAN_Attr_Sync_Seg[] = L"Sync_Seg";                                         // TODO: Distribute (via controller object update) for non-trivial BusManager
  const wchar_t kBusController_CAN_Attr_Prop_Seg[] = L"Prop_Seg";                                         // TODO: Distribute (via controller object update) for non-trivial BusManager
  const wchar_t kBusController_CAN_Attr_Phase_Seg1[] = L"Phase_Seg1";                                     // TODO: Distribute (via controller object update) for non-trivial BusManager
  const wchar_t kBusController_CAN_Attr_Phase_Seg2[] = L"Phase_Seg2";                                     // TODO: Distribute (via controller object update) for non-trivial BusManager
  const wchar_t kBusController_CAN_Attr_SamplingMode[] = L"SamplingMode";                                 // TODO: Distribute (via controller object update) for non-trivial BusManager

} // namespace FOMObjects

namespace IBDefaults {
  namespace DOs {
    const char ConnectionType[] = "";
  } // namespace DOs
  namespace TextLog {
    const char Text[] = "EMPTY";
    const char Sender[] = "EMPTY";
  }
} // namespace IBDefaults


namespace RTISyncPointNames {
  const wchar_t kInitializedSyncPointName[] = L"Initialized";
  const wchar_t kSynchronizeSysvarsSyncPointName[] = L"Synchronized";
  const wchar_t kStoppedSyncPointName[] = L"Stopped";
} // namespace RTISyncPointNames

namespace RTIConstants {
  static const double kDefaultEvokeTimeout = 0.1;
  static const double kDefaultWaitForSyncPointsTimeout = 1.0;
}

} // namespace NDistSimIB

