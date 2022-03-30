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
   File: RTI/RTIambassador.h
***********************************************************************/

// This interface is used to access the services of the RTI.

#ifndef RTI_RTIambassador_h
#define RTI_RTIambassador_h

namespace rti1516e
{
   class FederateAmbassador;
   class LogicalTime;
   class LogicalTimeFactory;
   class LogicalTimeInterval;
   class RangeBounds;
}

#include <RTI/SpecificConfig.h>
#include <string>
#include <memory>
#include <RTI/Typedefs.h>
#include <RTI/Exception.h>
#include <memory>

namespace rti1516e
{
    struct RTI_EXPORT RTInotificationHandle
    {
      virtual void Signal() = 0;
    };


   class RTI_EXPORT RTIambassador
   {
   protected:
      RTIambassador ();

   public:
      virtual ~RTIambassador () noexcept;

      // 4.2
      virtual void connect (
         FederateAmbassador & federateAmbassador,
         CallbackModel theCallbackModel,
         std::wstring const & localSettingsDesignator=L"") = 0;

      // 4.3
      virtual void disconnect () = 0;

      virtual void shutdown() = 0;

      // 4.5
      virtual void createFederationExecution (
         std::wstring const & federationExecutionName,
         std::wstring const & fomModule,
         std::wstring const & logicalTimeImplementationName = L"") = 0;

      virtual void createFederationExecution (
         std::wstring const & federationExecutionName,
         std::vector<std::wstring> const & fomModules,
         std::wstring const & logicalTimeImplementationName = L"") = 0;

      virtual void createFederationExecutionWithMIM (
         std::wstring const & federationExecutionName,
         std::vector<std::wstring> const & fomModules,
         std::wstring const & mimModule,
         std::wstring const & logicalTimeImplementationName = L"") = 0;

      // 4.6
      virtual void destroyFederationExecution (
         std::wstring const & federationExecutionName) = 0;

      // 4.7
      virtual void listFederationExecutions () = 0;

      // 4.9
      virtual FederateHandle joinFederationExecution (
         std::wstring const & federateType,
         std::wstring const & federationExecutionName,
         std::vector<std::wstring> const & additionalFomModules=std::vector<std::wstring>()) = 0;

      virtual FederateHandle joinFederationExecution (
         std::wstring const & federateName,
         std::wstring const & federateType,
         std::wstring const & federationExecutionName,
         std::vector<std::wstring> const & additionalFomModules=std::vector<std::wstring>()) = 0;

      // 4.10
      virtual void resignFederationExecution (
         ResignAction resignAction) = 0;

      // 4.11
      virtual void registerFederationSynchronizationPoint (
         std::wstring const & label,
         VariableLengthData const & theUserSuppliedTag) = 0;

      virtual void registerFederationSynchronizationPoint (
         std::wstring const & label,
         VariableLengthData const & theUserSuppliedTag,
         FederateHandleSet const & synchronizationSet) = 0;

      // 4.14
      virtual void synchronizationPointAchieved (
         std::wstring const & label,
         bool successfully = true) = 0;

      // 4.16
      virtual void requestFederationSave (
         std::wstring const & label) = 0;

      virtual void requestFederationSave (
         std::wstring const & label,
         LogicalTime const & theTime) = 0;

      // 4.18
      virtual void federateSaveBegun () = 0;

      // 4.19
      virtual void federateSaveComplete () = 0;

      virtual void federateSaveNotComplete () = 0;

      // 4.21
      virtual void abortFederationSave () = 0;

      // 4.22
      virtual void queryFederationSaveStatus () = 0;

      // 4.24
      virtual void requestFederationRestore (
         std::wstring const & label) = 0;

      // 4.28
      virtual void federateRestoreComplete () = 0;

      virtual void federateRestoreNotComplete () = 0;

      // 4.30
      virtual void abortFederationRestore () = 0;

      // 4.31
      virtual void queryFederationRestoreStatus () = 0;

      /////////////////////////////////////
      // Declaration Management Services //
      /////////////////////////////////////

      // 5.2
      virtual void publishObjectClassAttributes (
         ObjectClassHandle theClass,
         AttributeHandleSet const & attributeList) = 0;

      // 5.3
      virtual void unpublishObjectClass (
         ObjectClassHandle theClass) = 0;

      virtual void unpublishObjectClassAttributes (
         ObjectClassHandle theClass,
         AttributeHandleSet const & attributeList) = 0;

      // 5.4
      virtual void publishInteractionClass (
         InteractionClassHandle theInteraction) = 0;

      // 5.5
      virtual void unpublishInteractionClass (
         InteractionClassHandle theInteraction) = 0;

      // 5.6
      virtual void subscribeObjectClassAttributes (
         ObjectClassHandle theClass,
         AttributeHandleSet const & attributeList,
         bool active = true,
         std::wstring const & updateRateDesignator = L"") = 0;

      // 5.7
      virtual void unsubscribeObjectClass (
         ObjectClassHandle theClass) = 0;

      virtual void unsubscribeObjectClassAttributes (
         ObjectClassHandle theClass,
         AttributeHandleSet const & attributeList) = 0;

      // 5.8
      virtual void subscribeInteractionClass (
         InteractionClassHandle theClass,
         bool active = true) = 0;

      // 5.9
      virtual void unsubscribeInteractionClass (
         InteractionClassHandle theClass) = 0;

      ////////////////////////////////
      // Object Management Services //
      ////////////////////////////////

      // 6.2
      virtual void reserveObjectInstanceName (
         std::wstring const & theObjectInstanceName) = 0;

      // 6.4
      virtual void releaseObjectInstanceName (
         std::wstring const & theObjectInstanceName) = 0;

      // 6.5
      virtual void reserveMultipleObjectInstanceName (
         std::set<std::wstring> const & theObjectInstanceNames) = 0;

      // 6.7
      virtual void releaseMultipleObjectInstanceName (
         std::set<std::wstring> const & theObjectInstanceNames) = 0;

      // 6.8
      virtual ObjectInstanceHandle registerObjectInstance (
         ObjectClassHandle theClass) = 0;

      virtual ObjectInstanceHandle registerObjectInstance (
         ObjectClassHandle theClass,
         std::wstring const & theObjectInstanceName) = 0;

      // >>> Vector extension
      virtual rti1516e::ObjectInstanceHandle
      registerObjectInstance(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
        std::wstring const & objectInstanceName,
        bool allowUnreservedObjectNames) = 0;
      // <<<

      // 6.10
      virtual void updateAttributeValues (
         ObjectInstanceHandle theObject,
         AttributeHandleValueMap const & theAttributeValues,
         VariableLengthData const & theUserSuppliedTag) = 0;

      virtual MessageRetractionHandle updateAttributeValues (
         ObjectInstanceHandle theObject,
         AttributeHandleValueMap const & theAttributeValues,
         VariableLengthData const & theUserSuppliedTag,
         LogicalTime const & theTime) = 0;

      // 6.12
      virtual void sendInteraction (
         InteractionClassHandle theInteraction,
         ParameterHandleValueMap const & theParameterValues,
         VariableLengthData const & theUserSuppliedTag) = 0;

      virtual MessageRetractionHandle sendInteraction (
         InteractionClassHandle theInteraction,
         ParameterHandleValueMap const & theParameterValues,
         VariableLengthData const & theUserSuppliedTag,
         LogicalTime const & theTime) = 0;

      // 6.14
      virtual void deleteObjectInstance (
         ObjectInstanceHandle theObject,
         VariableLengthData const & theUserSuppliedTag) = 0;

      virtual MessageRetractionHandle deleteObjectInstance (
         ObjectInstanceHandle theObject,
         VariableLengthData const & theUserSuppliedTag,
         LogicalTime  const & theTime) = 0;

      // 6.16
      virtual void localDeleteObjectInstance (
         ObjectInstanceHandle theObject) = 0;

      // 6.19
      virtual void requestAttributeValueUpdate (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes,
         VariableLengthData const & theUserSuppliedTag) = 0;

      virtual void requestAttributeValueUpdate (
         ObjectClassHandle theClass,
         AttributeHandleSet const & theAttributes,
         VariableLengthData const & theUserSuppliedTag) = 0;

      // 6.23
      virtual void requestAttributeTransportationTypeChange (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes,
         TransportationType theType) = 0;

      // 6.25
      virtual void queryAttributeTransportationType (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute) = 0;

      // 6.27
      virtual void requestInteractionTransportationTypeChange (
         InteractionClassHandle theClass,
         TransportationType theType) = 0;

      // 6.29
      virtual void queryInteractionTransportationType (
         FederateHandle theFederate,
         InteractionClassHandle theInteraction) = 0;


      ///////////////////////////////////
      // Ownership Management Services //
      ///////////////////////////////////

      // 7.2
      virtual void unconditionalAttributeOwnershipDivestiture (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes) = 0;

      // 7.3
      virtual void negotiatedAttributeOwnershipDivestiture (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes,
         VariableLengthData const & theUserSuppliedTag) = 0;

      // 7.6
      virtual void confirmDivestiture (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & confirmedAttributes,
         VariableLengthData const & theUserSuppliedTag) = 0;

      // 7.8
      virtual void attributeOwnershipAcquisition (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & desiredAttributes,
         VariableLengthData const & theUserSuppliedTag) = 0;

      // 7.9
      virtual void attributeOwnershipAcquisitionIfAvailable (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & desiredAttributes) = 0;

      // 7.12
      virtual void attributeOwnershipReleaseDenied (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes) = 0;

      // 7.13
      virtual void attributeOwnershipDivestitureIfWanted (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes,
         AttributeHandleSet & theDivestedAttributes) = 0;

      // 7.14
      virtual void cancelNegotiatedAttributeOwnershipDivestiture (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes) = 0;

      // 7.15
      virtual void cancelAttributeOwnershipAcquisition (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes) = 0;

      // 7.17
      virtual void queryAttributeOwnership (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute) = 0;

      // 7.19
      virtual bool isAttributeOwnedByFederate (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute) = 0;

      //////////////////////////////
      // Time Management Services //
      //////////////////////////////

      // 8.2
      virtual void enableTimeRegulation (
         LogicalTimeInterval const & theLookahead) = 0;

      // 8.4
      virtual void disableTimeRegulation () = 0;

      // 8.5
      virtual void enableTimeConstrained () = 0;

      // 8.7
      virtual void disableTimeConstrained () = 0;

      // 8.8
      virtual void timeAdvanceRequest (
         LogicalTime const & theTime) = 0;

      // 8.9
      virtual void timeAdvanceRequestAvailable (
         LogicalTime const & theTime) = 0;

      // 8.10
      virtual void nextMessageRequest (
         LogicalTime const & theTime) = 0;

      // 8.11
      virtual void nextMessageRequestAvailable (
         LogicalTime const & theTime) = 0;

      // 8.12
      virtual void flushQueueRequest (
         LogicalTime const & theTime) = 0;

      // 8.14
      virtual void enableAsynchronousDelivery () = 0;

      // 8.15
      virtual void disableAsynchronousDelivery () = 0;

      // 8.16
      virtual bool queryGALT (LogicalTime & theTime) = 0;

      // 8.17
      virtual void queryLogicalTime (LogicalTime & theTime) = 0;

      // 8.18
      virtual bool queryLITS (LogicalTime & theTime) = 0;

      // 8.19
      virtual void modifyLookahead (
         LogicalTimeInterval const & theLookahead) = 0;

      // 8.20
      virtual void queryLookahead (LogicalTimeInterval & interval) = 0;

      // 8.21
      virtual void retract (
         MessageRetractionHandle theHandle) = 0;

      // 8.23
      virtual void changeAttributeOrderType (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes,
         OrderType theType) = 0;

      // 8.24
      virtual void changeInteractionOrderType (
         InteractionClassHandle theClass,
         OrderType theType) = 0;

      //////////////////////////////////
      // Data Distribution Management //
      //////////////////////////////////

      // 9.2
      virtual RegionHandle createRegion (
         DimensionHandleSet const & theDimensions) = 0;

      // 9.3
      virtual void commitRegionModifications (
         RegionHandleSet const & theRegionHandleSet) = 0;

      // 9.4
      virtual void deleteRegion (
         RegionHandle const & theRegion) = 0;

      // 9.5
      virtual ObjectInstanceHandle registerObjectInstanceWithRegions (
         ObjectClassHandle theClass,
         AttributeHandleSetRegionHandleSetPairVector const &
         theAttributeHandleSetRegionHandleSetPairVector) = 0;

      virtual ObjectInstanceHandle registerObjectInstanceWithRegions (
         ObjectClassHandle theClass,
         AttributeHandleSetRegionHandleSetPairVector const &
         theAttributeHandleSetRegionHandleSetPairVector,
         std::wstring const & theObjectInstanceName) = 0;

      // 9.6
      virtual void associateRegionsForUpdates (
         ObjectInstanceHandle theObject,
         AttributeHandleSetRegionHandleSetPairVector const &
         theAttributeHandleSetRegionHandleSetPairVector) = 0;

      // 9.7
      virtual void unassociateRegionsForUpdates (
         ObjectInstanceHandle theObject,
         AttributeHandleSetRegionHandleSetPairVector const &
         theAttributeHandleSetRegionHandleSetPairVector) = 0;

      // 9.8
      virtual void subscribeObjectClassAttributesWithRegions (
         ObjectClassHandle theClass,
         AttributeHandleSetRegionHandleSetPairVector const &
         theAttributeHandleSetRegionHandleSetPairVector,
         bool active = true,
         std::wstring const & updateRateDesignator = L"") = 0;


      // 9.9
      virtual void unsubscribeObjectClassAttributesWithRegions (
         ObjectClassHandle theClass,
         AttributeHandleSetRegionHandleSetPairVector const &
         theAttributeHandleSetRegionHandleSetPairVector) = 0;

      // 9.10
      virtual void subscribeInteractionClassWithRegions (
         InteractionClassHandle theClass,
         RegionHandleSet const & theRegionHandleSet,
         bool active = true) = 0;

      // 9.11
      virtual void unsubscribeInteractionClassWithRegions (
         InteractionClassHandle theClass,
         RegionHandleSet const & theRegionHandleSet) = 0;

      // 9.12
      virtual void sendInteractionWithRegions (
         InteractionClassHandle theInteraction,
         ParameterHandleValueMap const & theParameterValues,
         RegionHandleSet const & theRegionHandleSet,
         VariableLengthData const & theUserSuppliedTag) = 0;

      virtual MessageRetractionHandle sendInteractionWithRegions (
         InteractionClassHandle theInteraction,
         ParameterHandleValueMap const & theParameterValues,
         RegionHandleSet const & theRegionHandleSet,
         VariableLengthData const & theUserSuppliedTag,
         LogicalTime const & theTime) = 0;

      // 9.13
      virtual void requestAttributeValueUpdateWithRegions (
         ObjectClassHandle theClass,
         AttributeHandleSetRegionHandleSetPairVector const & theSet,
         VariableLengthData const & theUserSuppliedTag) = 0;

      //////////////////////////
      // RTI Support Services //
      //////////////////////////

      // 10.2
      virtual ResignAction getAutomaticResignDirective () = 0;

      // 10.3
      virtual void setAutomaticResignDirective (
         ResignAction resignAction) = 0;

      // 10.4
      virtual FederateHandle getFederateHandle (
         std::wstring const & theName) = 0;

      // 10.5
      virtual std::wstring getFederateName (
         FederateHandle theHandle) = 0;

      // 10.6
      virtual ObjectClassHandle getObjectClassHandle (
         std::wstring const & theName) = 0;

      // 10.7
      virtual std::wstring getObjectClassName (
         ObjectClassHandle theHandle) = 0;

      // 10.8
      virtual ObjectClassHandle getKnownObjectClassHandle (
         ObjectInstanceHandle theObject) = 0;

      // 10.9
      virtual ObjectInstanceHandle getObjectInstanceHandle (
         std::wstring const & theName) = 0;

      // 10.10
      virtual std::wstring getObjectInstanceName (
         ObjectInstanceHandle theHandle) = 0;

      // 10.11
      virtual AttributeHandle getAttributeHandle (
         ObjectClassHandle whichClass,
         std::wstring const & theAttributeName) = 0;

      // 10.12
      virtual std::wstring getAttributeName (
         ObjectClassHandle whichClass,
         AttributeHandle theHandle) = 0;

      // 10.13
      virtual double getUpdateRateValue (
         std::wstring const & updateRateDesignator) = 0;

      // 10.14
      virtual double getUpdateRateValueForAttribute (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute) = 0;

      // 10.15
      virtual InteractionClassHandle getInteractionClassHandle (
         std::wstring const & theName) = 0;

      // 10.16
      virtual std::wstring getInteractionClassName (
         InteractionClassHandle theHandle) = 0;

      // 10.17
      virtual ParameterHandle getParameterHandle (
         InteractionClassHandle whichClass,
         std::wstring const & theName) = 0;

      // 10.18
      virtual std::wstring getParameterName (
         InteractionClassHandle whichClass,
         ParameterHandle theHandle) = 0;

      // 10.19
      virtual OrderType getOrderType (
         std::wstring const & orderName) = 0;

      // 10.20
      virtual std::wstring getOrderName (
         OrderType orderType) = 0;

      // 10.21
      virtual TransportationType getTransportationType (
         std::wstring const & transportationName) = 0;

      // 10.22
      virtual std::wstring getTransportationName (
         TransportationType transportationType) = 0;

      // 10.23
      virtual DimensionHandleSet getAvailableDimensionsForClassAttribute (
         ObjectClassHandle theClass,
         AttributeHandle theHandle) = 0;

      // 10.24
      virtual DimensionHandleSet getAvailableDimensionsForInteractionClass (
         InteractionClassHandle theClass) = 0;

      // 10.25
      virtual DimensionHandle getDimensionHandle (
         std::wstring const & theName) = 0;

      // 10.26
      virtual std::wstring getDimensionName (
         DimensionHandle theHandle) = 0;

      // 10.27
      virtual unsigned long getDimensionUpperBound (
         DimensionHandle theHandle) = 0;

      // 10.28
      virtual DimensionHandleSet getDimensionHandleSet (
         RegionHandle theRegionHandle) = 0;

      // 10.29
      virtual RangeBounds getRangeBounds (
         RegionHandle theRegionHandle,
         DimensionHandle theDimensionHandle) = 0;

      // 10.30
      virtual void setRangeBounds (
         RegionHandle theRegionHandle,
         DimensionHandle theDimensionHandle,
         RangeBounds const & theRangeBounds) = 0;

      // 10.31
      virtual unsigned long normalizeFederateHandle (
         FederateHandle theFederateHandle) = 0;

      // 10.32
      virtual unsigned long normalizeServiceGroup (
         ServiceGroup theServiceGroup) = 0;

      // 10.33
      virtual void enableObjectClassRelevanceAdvisorySwitch () = 0;

      // 10.34
      virtual void disableObjectClassRelevanceAdvisorySwitch () = 0;

      // 10.35
      virtual void enableAttributeRelevanceAdvisorySwitch () = 0;

      // 10.36
      virtual void disableAttributeRelevanceAdvisorySwitch () = 0;

      // 10.37
      virtual void enableAttributeScopeAdvisorySwitch () = 0;

      // 10.38
      virtual void disableAttributeScopeAdvisorySwitch () = 0;

      // 10.39
      virtual void enableInteractionRelevanceAdvisorySwitch () = 0;

      // 10.40
      virtual void disableInteractionRelevanceAdvisorySwitch () = 0;

      // 10.41
      virtual bool evokeCallback (
         double approximateMinimumTimeInSeconds) = 0;

      // 10.42
      virtual bool evokeMultipleCallbacks (
         double approximateMinimumTimeInSeconds,
         double approximateMaximumTimeInSeconds) = 0;

      // 10.43
      virtual void enableCallbacks () = 0;

      // 10.44
      virtual void disableCallbacks () = 0;

      // API-specific services

      // Return instance of time factory being used by the federation
      virtual std::unique_ptr<LogicalTimeFactory> getTimeFactory () const = 0;

      // Decode handles
      virtual FederateHandle decodeFederateHandle (
         VariableLengthData const & encodedValue) const = 0;

      virtual ObjectClassHandle decodeObjectClassHandle (
         VariableLengthData const & encodedValue) const = 0;

      virtual InteractionClassHandle decodeInteractionClassHandle (
         VariableLengthData const & encodedValue) const = 0;

      virtual ObjectInstanceHandle decodeObjectInstanceHandle (
         VariableLengthData const & encodedValue) const = 0;

      virtual AttributeHandle decodeAttributeHandle (
         VariableLengthData const & encodedValue) const = 0;

      virtual ParameterHandle decodeParameterHandle (
         VariableLengthData const & encodedValue) const = 0;

      virtual DimensionHandle decodeDimensionHandle (
         VariableLengthData const & encodedValue) const = 0;

      virtual MessageRetractionHandle decodeMessageRetractionHandle (
         VariableLengthData const & encodedValue) const = 0;

      virtual RegionHandle decodeRegionHandle (
         VariableLengthData const & encodedValue) const = 0;

      // Vector extensions
      virtual void setNotificationHandle(RTInotificationHandle* h) = 0;
      
      virtual void unsubscribeObjectInstance(ObjectInstanceHandle theClass) = 0;
      virtual void subscribeInteractionClassWithFilter(InteractionClassHandle theClass, const ParameterHandleValueMap& filterValues, bool active = true) = 0;
      virtual void unsubscribeInteractionClassWithFilter(InteractionClassHandle theClass, const ParameterHandleValueMap& filterValues) = 0;

      virtual void setInteractionClassDeliverToSelf(InteractionClassHandle interactionClassHandle, bool enable) = 0;
      virtual void setObjectClassDeliverToSelf(ObjectClassHandle objectClassHandle, bool enable) = 0;

      virtual void StartNetworkStatistics(bool startTimer) = 0;
      virtual void StopNetworkStatistics() = 0;
      virtual void ResetNetworkStatistics() = 0;
      virtual void setConnectWaitTimeout(uint32_t timeoutMilliSeconds) = 0;
      virtual void setOperationWaitTimeout(uint32_t timeoutMilliSeconds) = 0;

      virtual std::wstring getParameterDataType(rti1516e::InteractionClassHandle interactionClassHandle, rti1516e::ParameterHandle paramterHandle) = 0;
      virtual std::wstring getAttributeDataType(rti1516e::ObjectClassHandle objectClassHandle, rti1516e::AttributeHandle attributeHandle) = 0;
};
}

#endif // RTI_RTIambassador_h
