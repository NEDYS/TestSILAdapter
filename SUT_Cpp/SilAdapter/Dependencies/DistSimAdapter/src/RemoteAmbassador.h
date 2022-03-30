/*-------------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#pragma once

#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <mutex>
#include <cassert>

#include "RTI/NullFederateAmbassador.h"
#include "RTI/RTIambassador.h"
#include "RTI/RTIambassadorFactory.h"
#include "RTI/encoding/BasicDataElements.h"
#include "RTI/time/HLAinteger64Time.h"

#include "AbstractService.h"
#include "distSim/IServices.h"
#include "distSim/IDistSimIB.h"
#include "EventNotification.h"
#include "FOMDefinitions.h"
#include "Utils.h"
#include "TimingService.h"
#include "DistSimIBConfig.h"

namespace NDistSimIB {

const VTimeNS kEndOfTime = 0x7fffffffffffffffLL;

// =========================================
// RemoteAmbassador handles the RTI connection and redirects incoming Interactions/Objects to specialized Services. 
// Publicly accessible API methods are defined in IRemoteAmbassador
// Forward declaration

class TimingService;

class RemoteAmbassador : 
  public rti::NullFederateAmbassador
{
public:
  // =========================================
  // Ctor Dtor
  RemoteAmbassador(const IBConfig& config);
  ~RemoteAmbassador();

  // =========================================
  // States of the RemoteAmbassador
  enum class AmbassadorState : uint8_t {
    Offline = 0,
    AmbassadorAvailable,
    ScanForFederationPrefix,
    CreateFederation,
    JoinFederation,
    FoundFederation,
    JoinedFederation,
    InitFederate,
    WaitForMeasurementInit,
    Online,
    ReceivedMeasurementStop,
    ReceivedMeasurementInit,
    FederationSynchronized,
    ConnectionLost
  };

  enum class CreateFederationResult : uint8_t {
    CreatedFederation = 0,
    FederationAlreadyExists,
    Error
  };
   
// =========================================
// OpenRTI methods
protected:

  // List existing federation names
  virtual void reportFederationExecutions(const rti::FederationExecutionInformationVector& federationExecutionInformationVector) override;
  virtual void connectionLost(std::wstring const& faultDescription) override;
  // Synchronization point methods
  virtual void synchronizationPointRegistrationSucceeded(std::wstring const& label) override;
  virtual void synchronizationPointRegistrationFailed(std::wstring const& label, rti::SynchronizationPointFailureReason reason) override;
  virtual void announceSynchronizationPoint(std::wstring const& label, rti::VariableLengthData const& theUserSuppliedTag) override;
  virtual void federationSynchronized(std::wstring const& label, rti::FederateHandleSet const& failedToSyncSet) override;

  // Object reservation
  virtual void objectInstanceNameReservationSucceeded(std::wstring const& theObjectInstanceName) override;
  virtual void objectInstanceNameReservationFailed(std::wstring const& theObjectInstanceName) override;
  // Object discovery
  virtual void discoverObjectInstance(rti::ObjectInstanceHandle theObject, rti::ObjectClassHandle theObjectClass, std::wstring const& theObjectInstanceName) override;
  virtual void discoverObjectInstance(rti::ObjectInstanceHandle theObject, rti::ObjectClassHandle theObjectClass, std::wstring const& theObjectInstanceName, rti::FederateHandle producingFederate) override;

  // Object attribute updates control
  virtual void turnUpdatesOnForObjectInstance(rti::ObjectInstanceHandle theObject, rti::AttributeHandleSet const& theAttributes) override;
  virtual void turnUpdatesOnForObjectInstance(rti::ObjectInstanceHandle theObject, rti::AttributeHandleSet const& theAttributes, std::wstring const& updateRateDesignator) override;

  virtual void turnUpdatesOffForObjectInstance(rti::ObjectInstanceHandle theObject, rti::AttributeHandleSet const& theAttributes) override;
  // Object removal
  virtual void removeObjectInstance(rti::ObjectInstanceHandle theObject,
    rti::VariableLengthData const& theUserSuppliedTag,
    rti::OrderType sentOrder,
    rti::SupplementalRemoveInfo theRemoveInfo) override;
  virtual void removeObjectInstance(rti::ObjectInstanceHandle theObject,
    rti::VariableLengthData const& theUserSuppliedTag,
    rti::OrderType sentOrder,
    rti::LogicalTime const& theTime,
    rti::OrderType receivedOrder,
    rti::SupplementalRemoveInfo theRemoveInfo) override;
  virtual void removeObjectInstance(rti::ObjectInstanceHandle theObject,
    rti::VariableLengthData const& theUserSuppliedTag,
    rti::OrderType sentOrder,
    rti::LogicalTime const& theTime,
    rti::OrderType receivedOrder,
    rti::MessageRetractionHandle theHandle,
    rti::SupplementalRemoveInfo theRemoveInfo) override;
  // Object update
  virtual void reflectAttributeValues(
    rti::ObjectInstanceHandle theObject, 
    rti::AttributeHandleValueMap const& theAttributeValues, 
    rti::VariableLengthData const& theUserSuppliedTag, 
    rti::OrderType sentOrder, 
    rti::TransportationType theType, 
    rti::LogicalTime const& theTime, 
    rti::OrderType receivedOrder, 
    rti::SupplementalReflectInfo theReflectInfo) override;
  virtual void reflectAttributeValues(
    rti::ObjectInstanceHandle theObject,
    rti::AttributeHandleValueMap const& theAttributeValues,
    rti::VariableLengthData const& theUserSuppliedTag,
    rti::OrderType sentOrder,
    rti::TransportationType theType,
    rti::SupplementalReflectInfo theReflectInfo) override;
  virtual void reflectAttributeValues(
    rti::ObjectInstanceHandle theObject,
    rti::AttributeHandleValueMap const& theAttributeValues,
    rti::VariableLengthData const& theUserSuppliedTag,
    rti::OrderType sentOrder,
    rti::TransportationType theType,
    rti::LogicalTime const& theTime,
    rti::OrderType receivedOrder,
    rti::MessageRetractionHandle theHandle,
    rti::SupplementalReflectInfo theReflectInfo) override;
  // Object update request response
  virtual void provideAttributeValueUpdate(
    rti::ObjectInstanceHandle theObject,
    rti::AttributeHandleSet const& theAttributes,
    rti::VariableLengthData const& theUserSuppliedTag) override;

  // Incoming Interations
  virtual void receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& theParameterValues, rti::VariableLengthData const& theUserSuppliedTag, rti::OrderType sentOrder, rti::TransportationType theType, rti::SupplementalReceiveInfo theReceiveInfo) override;
  virtual void receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& theParameterValues, rti::VariableLengthData const& theUserSuppliedTag, rti::OrderType sentOrder, rti::TransportationType theType, rti::LogicalTime const& theTime, rti::OrderType receivedOrder, rti::SupplementalReceiveInfo theReceiveInfo) override;
  virtual void receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& theParameterValues, rti::VariableLengthData const& theUserSuppliedTag, rti::OrderType sentOrder, rti::TransportationType theType, rti::LogicalTime const& theTime, rti::OrderType receivedOrder, rti::MessageRetractionHandle theHandle, rti::SupplementalReceiveInfo theReceiveInfo) override;

  // Time related methods
  virtual void timeRegulationEnabled(rti::LogicalTime const& theFederateTime)  override;
  virtual void timeConstrainedEnabled(rti::LogicalTime const& theFederateTime)  override;
  virtual void timeAdvanceGrant(rti::LogicalTime const& theTime) override;

  // Federation restore
  virtual void federationRestoreStatusResponse(rti::FederateRestoreStatusVector const& theFederateRestoreStatusVector) override {};

// =========================================
// Services abstraction and registration
public:

  // =========================================
  // General service registry
  void RegisterService(Service service);
  void UnregisterService(Service service);

  // =========================================
  // Registered services map
  std::unordered_map<Service, AbstractService*> mRegisteredServices;

  // =========================================
  // RTI communication
  struct AbstractInteractionClassSubscriber
  {
    // Services that use RTI Interactions implement these methods (with and without time stamp)
    virtual void receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& theParameterValues, rti::VariableLengthData const& theUserSuppliedTag, rti::SupplementalReceiveInfo theReceiveInfo) = 0;
    virtual void receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& theParameterValues, rti::VariableLengthData const& theUserSuppliedTag, rti::LogicalTime const& theTime, rti::SupplementalReceiveInfo theReceiveInfo) = 0;
  };
  // Services subscribe on RTI Interactions with these methods and RemoteAmbassador redirects the incoming IA
  void subscribeInteractionClass(rti::InteractionClassHandle theInteraction, AbstractInteractionClassSubscriber* subscriber);
  void subscribeInteractionClassWithFilter(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& filterValues, AbstractInteractionClassSubscriber* subscriber);
  void unsubscribeInteractionClass(rti::InteractionClassHandle theInteraction, AbstractInteractionClassSubscriber* subscriber);
  void unsubscribeInteractionClassWithFilter(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& filterValues, AbstractInteractionClassSubscriber* subscriber);

  struct AbstractObjectClassSubscriber
  {
    // Services that use RTI Objects implement these methods
    virtual void discoverObjectInstance(rti::ObjectInstanceHandle theObject, rti::ObjectClassHandle theObjectClass, std::wstring const& theObjectInstanceName) = 0;
    virtual void removeObjectInstance(rti::ObjectInstanceHandle theObject, rti::VariableLengthData const& theUserSuppliedTag, rti::LogicalTime const& theTime, rti::SupplementalRemoveInfo theRemoveInfo) = 0;
    virtual void reflectAttributeValues(rti::ObjectInstanceHandle& theObject, rti::ObjectClassHandle theObjectClass, const rti::AttributeHandleValueMap& theAttributeValues, const rti::VariableLengthData& theUserSuppliedTag, rti::LogicalTime const& theTime, rti::FederateHandle producingFederateHandle) = 0;
  };
  // Services subscribe on RTI Objects with these methods and RemoteAmbassador redirects the incoming value change
  void subscribeObjectClassAttributes(rti::ObjectClassHandle theClass, rti::AttributeHandleSet const& attributeList, AbstractObjectClassSubscriber* subscriber);
  void unsubscribeObjectClass(rti::ObjectClassHandle theClass, AbstractObjectClassSubscriber* subscriber);

  struct AbstractAttributeRequestProvider
  {
    virtual void provideAttributeValueUpdate(rti::ObjectClassHandle theObjectClass, rti::ObjectInstanceHandle theObject, rti::AttributeHandleSet const& theAttributes, rti::VariableLengthData const& theUserSuppliedTag) = 0;
  };
  void registerAttributeRequestProvider(rti::ObjectClassHandle theClass, AbstractAttributeRequestProvider* provider);
  void unregisterAttributeRequestProvider(rti::ObjectClassHandle theClass, AbstractAttributeRequestProvider* provider);

  // =========================================
  // Mapping of ClassHandle -> Service
  std::map<rti::InteractionClassHandle, AbstractInteractionClassSubscriber*> mInteractionClassSubscriber;
  std::map<rti::ObjectClassHandle, AbstractObjectClassSubscriber*> mObjectClassSubscriber;
  std::map<rti::ObjectClassHandle, AbstractAttributeRequestProvider*> mAttributeRequestProvider;

// =========================================
// Guarded getters for attributes/parameters

  rti::ParameterHandle getParameterHandle(rti::InteractionClassHandle interactionClassHandle, std::wstring const& parameterName);
  rti::AttributeHandle getAttributeHandle(rti::ObjectClassHandle objectClassHandle, std::wstring const& attributeName);

// =========================================
// Guarded attribute/parameter access

  //const rti::VariableLengthData& ReadMandatoryAttribute(
  //  const rti::ObjectClassHandle& objectClass,
  //  const rti::AttributeHandleValueMap& attributeValues,
  //  const rti::AttributeHandle& handle);

  bool ReadAttribute(
    rti::VariableLengthData& dataReference,
    const rti::ObjectClassHandle& objectClass,
    const rti::AttributeHandleValueMap& attributeValues,
    const rti::AttributeHandle& handle);

  //const rti::VariableLengthData& ReadMandatoryParameter(
  //  const rti::InteractionClassHandle& interactionClass,
  //  const rti::ParameterHandleValueMap& parameterValues,
  //  const rti::ParameterHandle& handle);

  bool ReadParameter(
    rti::VariableLengthData& dataReference,
    const rti::InteractionClassHandle& interactionClass,
    const rti::ParameterHandleValueMap& parameterValues,
    const rti::ParameterHandle& handle);

// =========================================
// Service object reservation
  typedef std::function<void(const std::wstring & name, bool success)> InstanceNameReservationCallback;
  void reserveObjectInstanceName(const std::wstring& name, InstanceNameReservationCallback callback);

// =========================================
// Services for Distributed Objects (DOs)
// Publicly accessible methods are in IDOService
public:
  friend class DOService;
  class AbstractDOService :
    public IDOService,
    public AbstractService,
    public AbstractObjectClassSubscriber, 
    public AbstractAttributeRequestProvider
  {
  public:
    virtual ~AbstractDOService() {}

  };
  // Creates an instance of the Service Class
  friend std::unique_ptr<AbstractDOService> CreateDOService(RemoteAmbassador& rtFederate);
  std::unique_ptr<AbstractDOService> mInternalDOService;

// =========================================
// Services for Bus Messages
// Publicly accessible methods are in IBusService
public:
  friend class BusService;
  class AbstractBusService :
    public IBusMessageService,
    public AbstractService,
    public AbstractObjectClassSubscriber, 
    public AbstractInteractionClassSubscriber,
    public AbstractAttributeRequestProvider
  {
  public:
    virtual ~AbstractBusService() {}

  };
  // Creates an instance of the Service Class
  friend std::unique_ptr<AbstractBusService> CreateBusService(RemoteAmbassador& rtFederate);
  std::unique_ptr<AbstractBusService> mInternalBusService;

//  // =========================================
//  // TimingService 
//  // Publicly accessible methods are in IDOService
//public:
//  friend class TimingService;
//  class AbstractTimingService :
//    public ITimingService,
//    public AbstractService
//  {
//  public:
//    virtual ~AbstractTimingService() {}
//
//    virtual void timeRegulationEnabled(rti::LogicalTime const& theFederateTime) = 0;
//    virtual void timeConstrainedEnabled(rti::LogicalTime const& theFederateTime) = 0;
//    virtual void timeAdvanceGrant(rti::LogicalTime const& theTime) = 0;
//
//    virtual void NextMessageRequest() = 0;
//
//  };
//  // Creates an instance of the Service Class
//  friend std::unique_ptr<TimingService> CreateTimingService(RemoteAmbassador& rtFederate);
//  std::unique_ptr<TimingService> mInternalTimingService;
  public:
    friend class TimingService;
    std::shared_ptr<TimingService> mInternalTimingService;
    //TimingService mInternalTimingService;

// =========================================
// TextLogService for logging into CANoe
public:
  friend class TextLogService;
  class AbstractTextLogService :
    public ITextLogService,
    public AbstractService,
    public AbstractInteractionClassSubscriber
  {
  public:
    virtual ~AbstractTextLogService() {}

  };
  // Creates an instance of the Service Class
  friend std::unique_ptr<AbstractTextLogService> CreateTextLogService(RemoteAmbassador& rtFederate);
  std::unique_ptr<AbstractTextLogService> mInternalTextLogService;


// =========================================
// NEW SERVICE REQUIRES IMPLEMENTATION HERE

// ...

public:
  // =========================================
  // Methods for DistSimIB object
  void Connect();
  void Disconnect();
  void DisconnectRTINode();
  void EvokeCallbacksPolled(double seconds);
  void EvokeCallbacksOnEvent();
  void EvokeCallbacks(double seconds);

  void SendMeasurementInit();
  void SendMeasurementStop();
  
  AmbassadorState GetState() const;
  // Event notification (for thread-based mode)

  void SetNotificationHandle();
  void UnSetNotificationHandle();

  // To leave the thread polling for notification events, a manual signal is needed 
  void SendManualSignal();

  // Thread safety
  // Locks evokeCallbacks() in polling thread against unbuffered HLA calls through API
  std::mutex mMutEvokeCallbacks;

  // Config
  IBConfig mConfig;

  // Simulation control 
  void ProcessSimulationControlCallbacks(IDistSimIB::SimulationControlEvent simulationEvent);
  typedef std::unordered_map<IDistSimIB::SimulationControlEvent, std::unordered_map<IDistSimIB::SimulationControlCallbackHandle, IDistSimIB::SimulationControlCallback>> SimulationControlRegister;
  SimulationControlRegister mSimulationControlCallbacks;
  IDistSimIB::SimulationControlCallbackHandle mSimulationControlCallbackHandle;

  // Federation control 
  void ProcessFederationControlCallbacks(IDistSimIB::FederationControlEvent federationEvent, const std::string& federationName);
  typedef std::unordered_map<IDistSimIB::FederationControlEvent, std::unordered_map<IDistSimIB::FederationControlCallbackHandle, IDistSimIB::FederationControlCallback>> FederationControlRegister;
  FederationControlRegister mFederationControlCallbacks;
  IDistSimIB::FederationControlCallbackHandle mFederationControlCallbackHandle;

private:

  // =========================================
  // Private RemoteAmbassador methods

  void HandleConnectionLost();
  bool CreateRTIAmbassador();
  bool ConnectToRTINode();
  std::string GetMeasurementIndexOfFederation(const std::string federationName);
  bool GetLatestFederation(std::string& federationName);
  void UpdateFederationNameAndIndex(bool foundFederation, std::string latestFederationName);

  CreateFederationResult CreateFederation();
  bool JoinFederation();
  void SetupRTIAmbassador();
  void RegisterSyncPoints();
  void InitializeHandles();
  void PublishSubscribe();
  void InitializeServices();
  bool ResignAndDestroyFederation();
  void ResetServices();
  void ResetOnMeasurementStop();
  void ResetRemoteAmbassador();
  void ResetFederationMemory();

  void ProcessServiceBuffers();

  void ChangeAmbassadorState(AmbassadorState newState);

  void OnReceiveIAMeasurementInit();
  void OnReceiveIAMeasurementStop(const rti::ParameterHandleValueMap& theParameterValues);

  bool HandleLateJoining();
  bool HandleDeadFederation();

  // =========================================
  // Private RemoteAmbassador data types
  std::unique_ptr<rti::RTIambassador>     mRTIAmb;
  
  // Federation identification
  int32_t                                 mFederationIndex;
  std::string                             mCurrentFederationName;
  std::string                             mCurrentFederateName;
  
  // Runtime states
  AmbassadorState                         mState = AmbassadorState::Offline;

  // RTI Handles
  rti::FederateHandle                     mFederateHandle;

  rti::InteractionClassHandle             mIAHandleMeasurementInit;
  rti::ParameterHandle                    mMeasurementInitParameter;
  rti::InteractionClassHandle             mIAHandleMeasurementStop;
  rti::ParameterHandle                    mMeasurementStopParameter;
  rti::InteractionClassHandle             mIAHandleSystemVariableUpdate;

  // Object reservation
  typedef std::map<std::wstring, InstanceNameReservationCallback> InstanceNameReservations;
  InstanceNameReservations                mInstanceNameReservations;
  
  // Poll federations
  bool mIsListOfFederationExecutionsAvailable;
  rti::FederationExecutionInformationVector mFederationExecutionInformationVector;

  // Wakeup Event notification
  std::unique_ptr<EventNotification> mEventNotification;
  bool mEventNotificationSet;

  // Event notification stats
  uint32_t mManualSignalCounter;
  uint32_t mSignaledWithMultipleCallback;
  uint32_t mSignaledWithSingleCallback;
  uint32_t mEvokeLoopCounter;
  uint32_t mTotalCallbacks;

  // Notification state of resolvable warnings
  std::unordered_map<ResolvableWarning, bool> mAlreadyNotified;

  // Control callbacks
  std::set<IDistSimIB::SimulationControlEvent> mSimulationEventToProcess;
  //std::set<IDistSimIB::FederationControlEvent> mFederationEventToProcess;

  // Flag if federate needs a synced simulation start
  bool mWaitForMeasurementInit;
  // Flag to signal if sync point 'initialized' was announced
  bool mIsAnnouncedInitializedSyncPoint;
  // Flag to signal if sync point 'synchronized' was announced
  bool mIsAnnouncedSynchronizedSyncPoint;
  // Flag to signal if sync point 'stopped' was announced
  bool mIsAnnouncedStoppedSyncPoint;

  // Allow only once per measurement
  bool mMeasurementInitSent, mMeasurementStopSent;

  // Load balancing:
  bool mStartTimeUserCodeAvailable = false;
  std::chrono::time_point<std::chrono::steady_clock> mStartTimeUserCode;

  // The whole FOM as wstring var
  std::wstring mFomModule;

  // The result of CreateFederation
  CreateFederationResult mCreateFederationResult;
};

std::string to_string(RemoteAmbassador::AmbassadorState state);

} // namespace NDistSimIB