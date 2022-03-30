/***********************************************************************
  IEEE 1516.1 High Level Architecture Interface Specification C++ API
  File: RTI/NullFederateAmbassador.h
***********************************************************************/

#ifndef RTI_NullFederateAmbassador_h
#define RTI_NullFederateAmbassador_h

#include <RTI/Exception.h>
#include <RTI/FederateAmbassador.h>

#pragma warning( disable : 4100 )

namespace rti1516
{
  class RTI_EXPORT NullFederateAmbassador : public FederateAmbassador
  {
  public:
    NullFederateAmbassador() {}

    virtual
    ~NullFederateAmbassador() {}

    // 4.7
    virtual
    void
    synchronizationPointRegistrationSucceeded(std::wstring const & label) {}

    virtual
    void
    synchronizationPointRegistrationFailed(std::wstring const & label,
                                           SynchronizationFailureReason reason) {}

    // 4.8
    virtual
    void
    announceSynchronizationPoint(std::wstring  const & label,
                                 VariableLengthData const & theUserSuppliedTag) {}

    // 4.10
    virtual
    void
    federationSynchronized(std::wstring const & label) {}

    // 4.12
    virtual
    void
    initiateFederateSave(std::wstring const & label) {}

    virtual
    void
    initiateFederateSave(std::wstring const & label,
                         LogicalTime const & theTime) {}

    // 4.15
    virtual
    void
    federationSaved() {}

    virtual
    void
    federationNotSaved(SaveFailureReason theSaveFailureReason) {}


    // 4.17
    virtual
    void
    federationSaveStatusResponse(
      FederateHandleSaveStatusPairVector const & 
      theFederateStatusVector) {}

    // 4.19
    virtual
    void
    requestFederationRestoreSucceeded(std::wstring const & label) {}

    virtual
    void
    requestFederationRestoreFailed(std::wstring const & label) {}

    // 4.20
    virtual
    void
    federationRestoreBegun() {}

    // 4.21
    virtual
    void
    initiateFederateRestore(std::wstring const & label,
                            FederateHandle handle) {}

    // 4.23
    virtual
    void
    federationRestored() {}

    virtual
    void
    federationNotRestored(RestoreFailureReason theRestoreFailureReason) {}

    // 4.25
    virtual
    void
    federationRestoreStatusResponse(
      FederateHandleRestoreStatusPairVector  const & 
      theFederateStatusVector) {}

    /////////////////////////////////////
    // Declaration Management Services //
    /////////////////////////////////////
  
    // 5.10
    virtual
    void
    startRegistrationForObjectClass(ObjectClassHandle theClass) {}

    // 5.11
    virtual
    void
    stopRegistrationForObjectClass(ObjectClassHandle theClass) {}

    // 5.12
    virtual
    void
    turnInteractionsOn(InteractionClassHandle theHandle) {}

    // 5.13
    virtual
    void
    turnInteractionsOff(InteractionClassHandle theHandle) {}

    ////////////////////////////////
    // Object Management Services //
    ////////////////////////////////
  
    // 6.3
    virtual
    void
    objectInstanceNameReservationSucceeded(std::wstring const &
                                           theObjectInstanceName) {}

    virtual
    void
    objectInstanceNameReservationFailed(std::wstring const &
                                        theObjectInstanceName) {}

  
    // 6.5
    virtual
    void
    discoverObjectInstance(ObjectInstanceHandle theObject,
                           ObjectClassHandle theObjectClass,
                           std::wstring const & theObjectInstanceName) {}

    // 6.7
    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType) {}

    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     RegionHandleSet const & theSentRegionHandleSet) {}

    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder) {}
  
    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     RegionHandleSet const & theSentRegionHandleSet) {}
  
    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     MessageRetractionHandle theHandle) {}

    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     MessageRetractionHandle theHandle,
     RegionHandleSet const & theSentRegionHandleSet) {}

    // 6.9
    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType) {}

    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     RegionHandleSet const & theSentRegionHandleSet) {}

    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder) {}

    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     RegionHandleSet const & theSentRegionHandleSet) {}

    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     MessageRetractionHandle theHandle) {}

    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     MessageRetractionHandle theHandle,
     RegionHandleSet const & theSentRegionHandleSet) {}

    // 6.11
    virtual
    void
    removeObjectInstance(ObjectInstanceHandle theObject,
                         VariableLengthData const & theUserSuppliedTag,
                         OrderType sentOrder) {}

    virtual
    void
    removeObjectInstance(ObjectInstanceHandle theObject,
                         VariableLengthData const & theUserSuppliedTag,
                         OrderType sentOrder,
                         LogicalTime const & theTime,
                         OrderType receivedOrder) {}

    virtual
    void
    removeObjectInstance(ObjectInstanceHandle theObject,
                         VariableLengthData const & theUserSuppliedTag,
                         OrderType sentOrder,
                         LogicalTime const & theTime,
                         OrderType receivedOrder,
                         MessageRetractionHandle theHandle) {}

    // 6.15
    virtual
    void
    attributesInScope
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes) {}

    // 6.16
    virtual
    void
    attributesOutOfScope
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes) {}

    // 6.18
    virtual
    void
    provideAttributeValueUpdate
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     VariableLengthData const & theUserSuppliedTag) {}

    // 6.19
    virtual
    void
    turnUpdatesOnForObjectInstance
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes) {}

    // 6.20
    virtual
    void
    turnUpdatesOffForObjectInstance
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes) {}

    ///////////////////////////////////
    // Ownership Management Services //
    ///////////////////////////////////
  
    // 7.4
    virtual
    void
    requestAttributeOwnershipAssumption
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & offeredAttributes,
     VariableLengthData const & theUserSuppliedTag) {}

    // 7.5
    virtual
    void
    requestDivestitureConfirmation
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & releasedAttributes) {}

    // 7.7
    virtual
    void
    attributeOwnershipAcquisitionNotification
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & securedAttributes,
     VariableLengthData const & theUserSuppliedTag) {}

    // 7.10
    virtual
    void
    attributeOwnershipUnavailable
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes) {}

    // 7.11
    virtual
    void
    requestAttributeOwnershipRelease
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & candidateAttributes,
     VariableLengthData const & theUserSuppliedTag) {}

    // 7.15
    virtual
    void
    confirmAttributeOwnershipAcquisitionCancellation
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes) {}

    // 7.17
    virtual
    void
    informAttributeOwnership(ObjectInstanceHandle theObject,
                             AttributeHandle theAttribute,
                             FederateHandle theOwner) {}

    virtual
    void
    attributeIsNotOwned(ObjectInstanceHandle theObject,
                        AttributeHandle theAttribute) {}

    virtual
    void
    attributeIsOwnedByRTI(ObjectInstanceHandle theObject,
                          AttributeHandle theAttribute) {}

    //////////////////////////////
    // Time Management Services //
    //////////////////////////////
  
    // 8.3
    virtual
    void
    timeRegulationEnabled(LogicalTime const & theFederateTime) {}

    // 8.6
    virtual
    void
    timeConstrainedEnabled(LogicalTime const & theFederateTime) {}

    // 8.13
    virtual
    void
    timeAdvanceGrant(LogicalTime const & theTime) {}

    // 8.22
    virtual
    void
    requestRetraction(MessageRetractionHandle theHandle) {}
  };
}

#endif // RTI_NullFederateAmbassador_h
