/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/
#pragma once

#include "RemoteAmbassador.h"
#include "UUIDT4.h"

#include <mutex>

namespace NDistSimIB {

class DOService : 
  public RemoteAmbassador::AbstractDOService
{
public:
  DOService(RemoteAmbassador& federate);
  ~DOService();

  // =========================================
  // Public DistSimAPI methods
  
  TransmitResult TransmitDOMember(DOHandle localHandle, const std::vector<uint8_t>& dataVec) override;

  DOHandle RegisterDOMemberSource(const std::string& DOMemberName) override;
  DOHandle RegisterDOMemberSource(const std::string& DOMemberName, IDODiscovererCounterpart* discoverer) override;
  DOHandle RegisterDOMemberSource(const std::string& DOMemberName, IDODiscovererCounterpart* discoverer, const std::string& connectionType) override;
  
  DOHandle RegisterDOMemberTarget(const std::string& DOMemberName, DOMemberReceptionCallback receptionCallback) override;
  DOHandle RegisterDOMemberTarget(const std::string& DOMemberName, DOMemberReceptionCallback receptionCallback, IDODiscovererCounterpart* discoverer) override;
  DOHandle RegisterDOMemberTarget(const std::string& DOMemberName, DOMemberReceptionCallback receptionCallback, IDODiscovererCounterpart* discoverer, const std::string& connectionType) override;

  DOHandle RegisterDOMemberTargetBySourceUUID(const std::string& sourceUUID, DOMemberReceptionCallback receptionCallback) override;
  DOHandle RegisterDOMemberTargetBySourceUUID(const std::string& sourceUUID, DOMemberReceptionCallback receptionCallback, const std::string& connectionType) override;

  void UnregisterDOMemberSource(DOHandle handle) override;
  void UnregisterDOMemberTarget(DOHandle handle) override;

  void RegisterGlobalDODiscovery(IDODiscovererGlobal* discoverer) override;
  void RegisterGlobalDODiscovery(IDODiscovererGlobal* discoverer, const std::string& connectionType) override;
  
  void UnregisterGlobalDODiscovery() override;
  void UnregisterGlobalDODiscovery(const std::string& connectionType) override;

  // =========================================
  // OpenRTI callbacks

  // =========================================
  // AbstractObjectClassSubscriber
  void discoverObjectInstance(rti::ObjectInstanceHandle theObject, rti::ObjectClassHandle theObjectClass, std::wstring const& theObjectInstanceName) override;
  void removeObjectInstance(rti::ObjectInstanceHandle theObject, rti::VariableLengthData const& theUserSuppliedTag, rti::LogicalTime const& theTime, rti::SupplementalRemoveInfo theRemoveInfo) override;
  void reflectAttributeValues(rti::ObjectInstanceHandle& theObject, rti::ObjectClassHandle theObjectClass, const rti::AttributeHandleValueMap& theAttributeValues, const rti::VariableLengthData& theUserSuppliedTag, rti::LogicalTime const& theTime, rti::FederateHandle producingFederateHandle) override;

  // =========================================
  // AbstractAttributeRequestProvider
  void provideAttributeValueUpdate(rti::ObjectClassHandle theObjectClass, rti::ObjectInstanceHandle theObject, rti::AttributeHandleSet const& theAttributes, rti::VariableLengthData const& theUserSuppliedTag) override;

  //=========================================
  // Service Methods called by RemoteAbmassador
  void OnJoined() override;
  void Reset() override;
  bool ProcessBuffers() override;

private:
  // =========================================
  //The parent federate
  RemoteAmbassador& mRemoteAmbassador;

  // =========================================
  // Private DOService members

  // Buffer structs
  struct RegisterBufferDOSourceData {
    std::string DOMemberName;
    std::string ConnectionType;
    DOHandle LocalHandle;
    IDODiscovererCounterpart* Discoverer;
    std::vector<uint8_t> LastDataVec;
    RegisterBufferDOSourceData(std::string n, std::string t, DOHandle h, IDODiscovererCounterpart* d)
      : DOMemberName(std::move(n)), ConnectionType(std::move(t)), LocalHandle(h), Discoverer(d) {}
  };
  struct RegisterBufferDOTargetData {
    std::string DOMemberName;
    std::string ConnectionType;
    DOHandle LocalHandle;
    DOMemberReceptionCallback ReceptionCallback;
    IDODiscovererCounterpart* Discoverer;
    RegisterBufferDOTargetData(std::string n, std::string t, DOHandle h, DOMemberReceptionCallback cb, IDODiscovererCounterpart* d)
      : DOMemberName(std::move(n)), ConnectionType(std::move(t)), LocalHandle(h), ReceptionCallback(std::move(cb)), Discoverer(d) {}
  };
  struct TransmitDOMemberBufferData {
    DOHandle LocalHandle;
    std::vector<uint8_t> DataVec;
    TransmitDOMemberBufferData(DOHandle h, const std::vector<uint8_t>& d)
      : LocalHandle(h), DataVec(d) {}
  };

  // Short term buffers, DO... found here have to be processed 
// Collected in RegisterDOSource if amb is online, rolled out and cleared in ProcessBuffers in event loop
  std::vector<RegisterBufferDOSourceData>       mBufferedRegisterDOSources;
  std::vector<RegisterBufferDOTargetData>       mBufferedRegisterDOTargets;

  // Long term DO vectors that persists during applicaton runtime (over federations and Stop()/Disconnect())
  // Set on RegisterDO... API call, copied to mBufferedRegisterDO... in ProcessPersistentRegistrations on JoinFederation
  std::vector<RegisterBufferDOSourceData>       mPersistentRegisterDOSources;
  std::vector<RegisterBufferDOTargetData>       mPersistentRegisterDOTargets;
  // TODO bkd: Buffer Unregister calls?
  std::vector<std::string>                      mBufferedUnregisterDOSources;
  std::vector<std::string>                      mBufferedUnregisterDOTargets;

  // TransmitDOMember buffer
  std::vector<TransmitDOMemberBufferData>       mBufferedTransmit;

  // Buffer thread safety
  // One mutex per queue: Locks API calls that fill the buffers against ProcessBuffer
  std::mutex mMutTransmitDOMember;
  std::mutex mMutRegisterDOSource;
  std::mutex mMutRegisterDOTarget;

  // DOTarget struct
  struct DOTarget {
    DOTarget(std::string DOMemberName, std::string UUID, std::string connectionType, DOHandle localHandle, rti::ObjectInstanceHandle objectInstanceHandle, DOMemberReceptionCallback callback, IDODiscovererCounterpart* discoverer) :
      mName(std::move(DOMemberName)), mUUID(std::move(UUID)), mConnectionType(std::move(connectionType)), mLocalHandle(localHandle), mObjectInstanceHandle(std::move(objectInstanceHandle)), mCallback(std::move(callback)), mDiscoverer(discoverer) { }

    std::string mName;
    std::string mUUID;
    std::string mConnectionType;
    DOHandle mLocalHandle;
    rti::ObjectInstanceHandle mObjectInstanceHandle;
    DOMemberReceptionCallback mCallback;
    IDODiscovererCounterpart* mDiscoverer;
  };

  // DOSource struct
  struct DOSource {
    DOSource(std::string DOMemberName, std::string UUID, std::string connectionType, DOHandle localHandle, rti::ObjectInstanceHandle objectInstanceHandle, IDODiscovererCounterpart* discoverer) :
      mName(std::move(DOMemberName)), mUUID(std::move(UUID)), mConnectionType(std::move(connectionType)), mLocalHandle(localHandle), mObjectInstanceHandle(std::move(objectInstanceHandle)), mDiscoverer(discoverer) { }

    std::string mName;
    std::string mUUID;
    std::string mConnectionType;
    DOHandle mLocalHandle;
    rti::ObjectInstanceHandle mObjectInstanceHandle;
    IDODiscovererCounterpart* mDiscoverer;
    std::vector<uint8_t> mLastDataVec;
  };

  // Storage vectors
  std::vector<std::shared_ptr<DOSource>> mLocalSources;
  std::vector<DOTarget> mLocalTargets;

  std::vector<std::shared_ptr<DOSource>> mRemoteSources;
  std::vector<std::shared_ptr<DOTarget>> mRemoteTargets;

  // Maps
  using SourceByObjectInstanceHandle = std::unordered_map<rti::ObjectInstanceHandle, std::shared_ptr<DOSource>>;
  using TargetByObjectInstanceHandle = std::unordered_map<rti::ObjectInstanceHandle, std::shared_ptr<DOTarget>>;
  using SourceObjectByLocalHandle = std::unordered_map<DOHandle, std::shared_ptr<DOSource>>;

  using TargetCallbacks = std::unordered_map<std::string, std::unordered_map<std::string, DOMemberReceptionCallback>>;

  // For incoming attribute updates
  SourceByObjectInstanceHandle                  mRemoteSourceByObjectInstanceHandle;
  TargetByObjectInstanceHandle                  mRemoteTargetByObjectInstanceHandle;
  // Local callbacks by connectionType and DOName
  TargetCallbacks                               mLocalTargetCallbacks;
  // To identify the local source on TransmitDOMember/Unregister
  SourceObjectByLocalHandle                     mLocalSourceByLocalHandle;

  // Local DO handles simply increase by one per Register... call 
  DOHandle                                      mNextLocalDOTargetHandle = 1;
  DOHandle                                      mNextLocalDOSourceHandle = 1;

  // Advanced API
  using DOGlobalDiscoverers = std::unordered_map<std::string, std::vector<IDODiscovererGlobal*>>;
  DOGlobalDiscoverers                           mDODiscoverers;

  // RTI Handles
    // HLA Object per DO Source 
  rti::ObjectClassHandle                        mDOSourceObjClassHandle;
  rti::AttributeHandle                          mDOSourceObjectDOMemberNameAttributeHandle;
  rti::AttributeHandle                          mDOSourceObjectConnectionTypeAttributeHandle;
  rti::AttributeHandle                          mDOSourceObjectDataBytesAttributeHandle;

  rti::ObjectClassHandle                        mDOTargetObjClassHandle;
  rti::AttributeHandle                          mDOTargetObjectDOMemberNameAttributeHandle;
  rti::AttributeHandle                          mDOTargetObjectConnectionTypeAttributeHandle;

  // =========================================
  // Private DOService methods

  // Init
  void InitializeHandles();
  void PublishSubscribe();
  // Processed on join federation
  void ProcessPersistentRegistrations();

  // 1. Register buffered sources/targets and reserve the name(uuid)
  void RegisterBufferedDOSource(const RegisterBufferDOSourceData& DOSourceData);
  DOHandle RegisterDOMemberSourceCommon(const std::string& DOMemberName, IDODiscovererCounterpart* discoverer, const std::string& connectionType);
  void RegisterBufferedDOTarget(const RegisterBufferDOTargetData& DOTargetData);
  DOHandle RegisterDOMemberTargetCommon(const std::string& DOMemberName, DOMemberReceptionCallback receptionCallback, IDODiscovererCounterpart* discoverer, const std::string& connectionType);
  DOHandle RegisterDOMemberTargetBySourceUUIDCommon(const std::string& sourceUUID, DOMemberReceptionCallback receptionCallback, const std::string& connectionType);
  // 2. Create the local objects here and get a objectInstnceHandle by HLA
  void CreateDOTarget(const RegisterBufferDOTargetData& DOTargetData);
  void OnDOSourceObjectReservationCompleted(const std::string& DOmemberUUID, bool wasInstanceReservationSuccessful);
  void OnDOTargetObjectReservationCompleted(const std::string& DOmemberUUID, bool wasInstanceReservationSuccessful);
  // 3. Update the objects with name, connectionType
  void UpdateDOTargetObjectName(const DOTarget& target);
  void UpdateDOSourceObjectName(const DOSource& source);
  // 4. In DO data transmit, the source updates its HLA objects 
  enum class BufferedTransmitResult { OK, NotRegistered };
  BufferedTransmitResult BufferedTransmitDOMember(const TransmitDOMemberBufferData& transmitDOData);
  // 5. The outgoing data update (w/wo connectionType)
  void UpdateDOSourceObjectData(const rti::ObjectInstanceHandle& theObject, const std::vector<uint8_t>& dataVec);
  // 6a. Handle incoming name/data updates for sources
  void HandleDOSourceObjectUpdate(const rti::ObjectInstanceHandle& objInstanceHandle, const rti::AttributeHandleValueMap& theAttributeValues, rti::LogicalTime const& theTime, rti::FederateHandle producingFederateHandle);
  // 6b. Handle incoming name updates for targets, used for target discovery callbacks
  void HandleDOTargetObjectUpdate(const rti::ObjectInstanceHandle& objInstanceHandle, const rti::AttributeHandleValueMap& theAttributeValues, rti::LogicalTime const& theTime, rti::FederateHandle producingFederateHandle);
  // 7. Local targets can execute their callbacks on an incoming source data update
  void ExecuteTargetCallbacks(const std::string& DOmemberName, const std::string& DOMemberConnectionType, const std::vector<uint8_t>& dataVec, const rti::LogicalTime& theTime);

  // Global discovery callbacks, no local counterpart needed
  enum class DODiscoveryCallbackType { DiscoverSource, DiscoverTarget, LostSource, LostTarget };
  void ExecuteGlobalDiscoveryCallbacks(DODiscoveryCallbackType discoveryType, const std::string& DOMemberName, const std::string& DOuuid, const std::string& DOMemberConnectionType, rti::FederateHandle producingFederateHandle);

  // Discovery callbacks for local counterparts: Returns if a counterpart was found
  bool ExecuteLocalTargetsDiscoveryCallbacks(DODiscoveryCallbackType discoveryType, const std::string& DOMemberName, const std::string& uuid, const std::string& DOMemberConnectionType, rti::FederateHandle producingFederateHandle);
  bool ExecuteLocalSourcesDiscoveryCallbacks(DODiscoveryCallbackType discoveryType, const std::string& DOMemberName, const std::string& uuid, const std::string& DOMemberConnectionType, rti::FederateHandle producingFederateHandle);

  // Convenience methods
  bool hasLocalTargetCallback(const std::string& DOMemberName, const std::string& DOMemberConnectionType);
  enum class DOMemberType { Value, Call, CallReturn, Unknown };
  DOMemberType IdentifyDOMemberType(const std::string& DOMemberName);

};

} // namespace NDistSimIB