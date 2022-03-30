/***********************************************************************
   The IEEE hereby grants a general, royalty-free license to copy, distribute,
   display and make derivative works from this material, for all purposes,
   provided that any use of the material contains the following
   attribution: "Reprinted with permission from IEEE 1516.1(TM)-2010".
   Should you require additional information, contact the Manager, Standards
   Intellectual Property, IEEE Standards Association (stds-ipr@ieee.org).
***********************************************************************/
/***********************************************************************
   IEEE 1516.1 High Level Architecture Interface Specification C++ API
   File: RTI/FederateAmbassador.h
***********************************************************************/

// This is a pure abstract interface that must be implemented by the
// federate to receive callbacks from the RTI.

#ifndef RTI_FederateAmbassador_h
#define RTI_FederateAmbassador_h

namespace rti1516e
{
   class LogicalTime;
}

#include <RTI/SpecificConfig.h>
#include <RTI/Exception.h>
#include <RTI/Typedefs.h>
#include <RTI/Enums.h>

namespace rti1516e
{
   class RTI_EXPORT FederateAmbassador
   {
   protected:
      FederateAmbassador ();

   public:
      virtual ~FederateAmbassador () = 0;

      // 4.4
      virtual void connectionLost (
         std::wstring const & faultDescription) = 0;

      // 4.8
      virtual void reportFederationExecutions (
         FederationExecutionInformationVector const &
         theFederationExecutionInformationList) = 0;

      // 4.12
      virtual void synchronizationPointRegistrationSucceeded (
         std::wstring const & label) = 0;

      virtual void synchronizationPointRegistrationFailed (
         std::wstring const & label,
         SynchronizationPointFailureReason reason) = 0;

      // 4.13
      virtual void announceSynchronizationPoint (
         std::wstring  const & label,
         VariableLengthData const & theUserSuppliedTag) = 0;

      // 4.15
      virtual void federationSynchronized (
         std::wstring const & label,
         FederateHandleSet const& failedToSyncSet) = 0;

      // 4.17
      virtual void initiateFederateSave (
         std::wstring const & label) = 0;

      virtual void initiateFederateSave (
         std::wstring const & label,
         LogicalTime const & theTime) = 0;

      // 4.20
      virtual void federationSaved () = 0;

      virtual void federationNotSaved (
         SaveFailureReason theSaveFailureReason) = 0;


      // 4.23
      virtual void federationSaveStatusResponse (
         FederateHandleSaveStatusPairVector const &
         theFederateStatusVector) = 0;

      // 4.25
      virtual void requestFederationRestoreSucceeded (
         std::wstring const & label) = 0;

      virtual void requestFederationRestoreFailed (
         std::wstring const & label) = 0;

      // 4.26
      virtual void federationRestoreBegun () = 0;

      // 4.27
      virtual void initiateFederateRestore (
         std::wstring const & label,
         std::wstring const & federateName,
         FederateHandle handle) = 0;

      // 4.29
      virtual void federationRestored () = 0;

      virtual void federationNotRestored (
         RestoreFailureReason theRestoreFailureReason) = 0;

      // 4.32
      virtual void federationRestoreStatusResponse (
         FederateRestoreStatusVector const &
         theFederateRestoreStatusVector) = 0;

      /////////////////////////////////////
      // Declaration Management Services //
      /////////////////////////////////////

      // 5.10
      virtual void startRegistrationForObjectClass (
         ObjectClassHandle theClass) = 0;

      // 5.11
      virtual void stopRegistrationForObjectClass (
         ObjectClassHandle theClass) = 0;

      // 5.12
      virtual void turnInteractionsOn (
         InteractionClassHandle theHandle) = 0;

      // 5.13
      virtual void turnInteractionsOff (
         InteractionClassHandle theHandle) = 0;

      ////////////////////////////////
      // Object Management Services //
      ////////////////////////////////

      // 6.3
      virtual void objectInstanceNameReservationSucceeded (
         std::wstring const & theObjectInstanceName) = 0;

      virtual void objectInstanceNameReservationFailed (
         std::wstring const & theObjectInstanceName) = 0;

      // 6.6
      virtual void multipleObjectInstanceNameReservationSucceeded (
         std::set<std::wstring> const & theObjectInstanceNames) = 0;

      virtual void multipleObjectInstanceNameReservationFailed (
         std::set<std::wstring> const & theObjectInstanceNames) = 0;


      // 6.9
      virtual void discoverObjectInstance (
         ObjectInstanceHandle theObject,
         ObjectClassHandle theObjectClass,
         std::wstring const & theObjectInstanceName) = 0;

      virtual void discoverObjectInstance (
         ObjectInstanceHandle theObject,
         ObjectClassHandle theObjectClass,
         std::wstring const & theObjectInstanceName,
         FederateHandle producingFederate) = 0;

      // 6.11
      virtual void reflectAttributeValues (
         ObjectInstanceHandle theObject,
         AttributeHandleValueMap const & theAttributeValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         SupplementalReflectInfo theReflectInfo) = 0;

      virtual void reflectAttributeValues (
         ObjectInstanceHandle theObject,
         AttributeHandleValueMap const & theAttributeValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         SupplementalReflectInfo theReflectInfo) = 0;

      virtual void reflectAttributeValues (
         ObjectInstanceHandle theObject,
         AttributeHandleValueMap const & theAttributeValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         MessageRetractionHandle theHandle,
         SupplementalReflectInfo theReflectInfo) = 0;

      // 6.13
      virtual void receiveInteraction (
         InteractionClassHandle theInteraction,
         ParameterHandleValueMap const & theParameterValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         SupplementalReceiveInfo theReceiveInfo) = 0;

      virtual void receiveInteraction (
         InteractionClassHandle theInteraction,
         ParameterHandleValueMap const & theParameterValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         SupplementalReceiveInfo theReceiveInfo) = 0;

      virtual void receiveInteraction (
         InteractionClassHandle theInteraction,
         ParameterHandleValueMap const & theParameterValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         MessageRetractionHandle theHandle,
         SupplementalReceiveInfo theReceiveInfo) = 0;

      // 6.15
      virtual void removeObjectInstance (
         ObjectInstanceHandle theObject,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         SupplementalRemoveInfo theRemoveInfo) = 0;

      virtual void removeObjectInstance (
         ObjectInstanceHandle theObject,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         SupplementalRemoveInfo theRemoveInfo) = 0;

      virtual void removeObjectInstance (
         ObjectInstanceHandle theObject,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         MessageRetractionHandle theHandle,
         SupplementalRemoveInfo theRemoveInfo) = 0;

      // 6.17
      virtual void attributesInScope (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes) = 0;

      // 6.18
      virtual void attributesOutOfScope (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes) = 0;

      // 6.20
      virtual void provideAttributeValueUpdate (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes,
         VariableLengthData const & theUserSuppliedTag) = 0;

      // 6.21
      virtual void turnUpdatesOnForObjectInstance (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes) = 0;

      virtual void turnUpdatesOnForObjectInstance (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes,
         std::wstring const & updateRateDesignator) = 0;

      // 6.22
      virtual void turnUpdatesOffForObjectInstance (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes) = 0;

      // 6.24
      virtual void confirmAttributeTransportationTypeChange (
         ObjectInstanceHandle theObject,
         AttributeHandleSet theAttributes,
         TransportationType theTransportation) = 0;

      // 6.26
      virtual void reportAttributeTransportationType (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute,
         TransportationType theTransportation) = 0;

      // 6.28
      virtual void confirmInteractionTransportationTypeChange (
         InteractionClassHandle theInteraction,
         TransportationType theTransportation) = 0;

      // 6.30
      virtual void reportInteractionTransportationType (
         FederateHandle federateHandle,
         InteractionClassHandle theInteraction,
         TransportationType  theTransportation) = 0;


      ///////////////////////////////////
      // Ownership Management Services //
      ///////////////////////////////////

      // 7.4
      virtual void requestAttributeOwnershipAssumption (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & offeredAttributes,
         VariableLengthData const & theUserSuppliedTag) = 0;

      // 7.5
      virtual void requestDivestitureConfirmation (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & releasedAttributes) = 0;

      // 7.7
      virtual void attributeOwnershipAcquisitionNotification (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & securedAttributes,
         VariableLengthData const & theUserSuppliedTag) = 0;

      // 7.10
      virtual void attributeOwnershipUnavailable (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes) = 0;

      // 7.11
      virtual void requestAttributeOwnershipRelease (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & candidateAttributes,
         VariableLengthData const & theUserSuppliedTag) = 0;

      // 7.16
      virtual void confirmAttributeOwnershipAcquisitionCancellation (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes) = 0;

      // 7.18
      virtual void informAttributeOwnership (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute,
         FederateHandle theOwner) = 0;

      virtual void attributeIsNotOwned (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute) = 0;

      virtual void attributeIsOwnedByRTI (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute) = 0;

      //////////////////////////////
      // Time Management Services //
      //////////////////////////////

      // 8.3
      virtual void timeRegulationEnabled (
         LogicalTime const & theFederateTime) = 0;

      // 8.6
      virtual void timeConstrainedEnabled (
         LogicalTime const & theFederateTime) = 0;

      // 8.13
      virtual void timeAdvanceGrant (
         LogicalTime const & theTime) = 0;

      // 8.22
      virtual void requestRetraction (
         MessageRetractionHandle theHandle) = 0;
   };
}

#endif // RTI_FederateAmbassador_h
