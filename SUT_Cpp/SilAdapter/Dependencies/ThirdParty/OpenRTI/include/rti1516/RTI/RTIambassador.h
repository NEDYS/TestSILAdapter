/***********************************************************************
  IEEE 1516.1 High Level Architecture Interface Specification C++ API
  File: RTI/RTIambassador.h
***********************************************************************/

// This interface is used to access the services of the RTI. 

#ifndef RTI_RTIambassador_h
#define RTI_RTIambassador_h

namespace rti1516
{
  class FederateAmbassador;
  class LogicalTime;
  class LogicalTimeFactory;
  class LogicalTimeInterval;
  class RangeBounds;
}

#include <RTI/SpecificConfig.h>
#include <string>
#include <RTI/Typedefs.h>
#include <RTI/Exception.h>

namespace rti1516
{
  class RTI_EXPORT RTIambassador
  {
  protected:
    RTIambassador();

  public:
    virtual
      ~RTIambassador();

    // 4.2
    virtual void createFederationExecution
    (std::wstring const & federationExecutionName,
     std::wstring const & fullPathNameToTheFDDfile,
     std::wstring const & logicalTimeImplementationName = L"") = 0;

    // 4.3
    virtual void destroyFederationExecution 
    (std::wstring const & federationExecutionName) = 0;

    // 4.4
    virtual FederateHandle joinFederationExecution 
    (std::wstring const & federateType,
     std::wstring const & federationExecutionName,
     FederateAmbassador & federateAmbassador) = 0;

    // 4.5
    virtual void resignFederationExecution
    (ResignAction resignAction) = 0;

    // 4.6
    virtual void registerFederationSynchronizationPoint
    (std::wstring const & label,
     VariableLengthData const & theUserSuppliedTag) = 0;

    virtual void registerFederationSynchronizationPoint
    (std::wstring const & label,
     VariableLengthData const & theUserSuppliedTag,
     FederateHandleSet const & syncSet) = 0;

    // 4.9
    virtual void synchronizationPointAchieved
    (std::wstring const & label) = 0;

    // 4.11
    virtual void requestFederationSave
    (std::wstring const & label) = 0;

    virtual void requestFederationSave
    (std::wstring const & label,
     LogicalTime const & theTime) = 0;

    // 4.13
    virtual void federateSaveBegun () = 0;

    // 4.14
    virtual void federateSaveComplete () = 0;

    virtual void federateSaveNotComplete() = 0;

    // 4.16
    virtual void queryFederationSaveStatus () = 0;

    // 4.18
    virtual void requestFederationRestore
    (std::wstring const & label) = 0;

    // 4.22
    virtual void federateRestoreComplete () = 0;

    virtual void federateRestoreNotComplete () = 0;

    // 4.24
    virtual void queryFederationRestoreStatus () = 0;

    /////////////////////////////////////
    // Declaration Management Services //
    /////////////////////////////////////
  
    // 5.2
    virtual void publishObjectClassAttributes
    (ObjectClassHandle theClass,
     AttributeHandleSet const & attributeList) = 0;

    // 5.3
    virtual void unpublishObjectClass
    (ObjectClassHandle theClass) = 0;

    virtual void unpublishObjectClassAttributes
    (ObjectClassHandle theClass,
     AttributeHandleSet const & attributeList) = 0;

    // 5.4
    virtual void publishInteractionClass
    (InteractionClassHandle theInteraction) = 0;

    // 5.5
    virtual void unpublishInteractionClass
    (InteractionClassHandle theInteraction) = 0;

    // 5.6
    virtual void subscribeObjectClassAttributes
    (ObjectClassHandle theClass,
     AttributeHandleSet const & attributeList,
     bool active = true) = 0;

    // 5.7
    virtual void unsubscribeObjectClass
    (ObjectClassHandle theClass) = 0;

    virtual void unsubscribeObjectClassAttributes
    (ObjectClassHandle theClass,
     AttributeHandleSet const & attributeList) = 0;

    // 5.8
    virtual void subscribeInteractionClass
    (InteractionClassHandle theClass,
     bool active = true) = 0;

    // 5.9
    virtual void unsubscribeInteractionClass
    (InteractionClassHandle theClass) = 0;

    ////////////////////////////////
    // Object Management Services //
    ////////////////////////////////
  
    // 6.2
    virtual void reserveObjectInstanceName
    (std::wstring const & theObjectInstanceName) = 0;

    // 6.4
    virtual ObjectInstanceHandle registerObjectInstance
    (ObjectClassHandle theClass) = 0;

    virtual ObjectInstanceHandle registerObjectInstance
    (ObjectClassHandle theClass,
     std::wstring const & theObjectInstanceName) = 0;

    // 6.6
    virtual void updateAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag) = 0;

    virtual MessageRetractionHandle updateAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     LogicalTime const & theTime) = 0;

    // 6.8
    virtual void sendInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag) = 0;

    virtual MessageRetractionHandle sendInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     LogicalTime const & theTime) = 0;

    // 6.10
    virtual void deleteObjectInstance
    (ObjectInstanceHandle theObject,
     VariableLengthData const & theUserSuppliedTag) = 0;

    virtual MessageRetractionHandle deleteObjectInstance
    (ObjectInstanceHandle theObject,
     VariableLengthData const & theUserSuppliedTag,
     LogicalTime  const & theTime) = 0;

    // 6.12
    virtual void localDeleteObjectInstance
    (ObjectInstanceHandle theObject) = 0;

    // 6.13
    virtual void changeAttributeTransportationType
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     TransportationType theType) = 0;

    // 6.14
    virtual void changeInteractionTransportationType
    (InteractionClassHandle theClass,
     TransportationType theType) = 0;
  
    // 6.17
    virtual void requestAttributeValueUpdate
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     VariableLengthData const & theUserSuppliedTag) = 0;

    virtual void requestAttributeValueUpdate
    (ObjectClassHandle theClass,
     AttributeHandleSet const & theAttributes,
     VariableLengthData const & theUserSuppliedTag) = 0;

    ///////////////////////////////////
    // Ownership Management Services //
    ///////////////////////////////////
    // 7.2
    virtual void unconditionalAttributeOwnershipDivestiture
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes) = 0;

    // 7.3
    virtual void negotiatedAttributeOwnershipDivestiture
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     VariableLengthData const & theUserSuppliedTag) = 0;

    // 7.6
    virtual void confirmDivestiture
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & confirmedAttributes,
     VariableLengthData const & theUserSuppliedTag) = 0;

    // 7.8
    virtual void attributeOwnershipAcquisition
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & desiredAttributes,
     VariableLengthData const & theUserSuppliedTag) = 0;

    // 7.9
    virtual void attributeOwnershipAcquisitionIfAvailable
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & desiredAttributes) = 0;

    // 7.12
    virtual void attributeOwnershipDivestitureIfWanted
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     AttributeHandleSet & theDivestedAttributes) = 0;

    // 7.13
    virtual void cancelNegotiatedAttributeOwnershipDivestiture
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes) = 0;

    // 7.14
    virtual void cancelAttributeOwnershipAcquisition
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes) = 0;

    // 7.16
    virtual void queryAttributeOwnership
    (ObjectInstanceHandle theObject,
     AttributeHandle theAttribute) = 0;

    // 7.18
    virtual bool isAttributeOwnedByFederate
    (ObjectInstanceHandle theObject,
     AttributeHandle theAttribute) = 0;

    //////////////////////////////
    // Time Management Services //
    //////////////////////////////
  
    // 8.2
    virtual void enableTimeRegulation
    (LogicalTimeInterval const & theLookahead) = 0;

    // 8.4
    virtual void disableTimeRegulation () = 0;

    // 8.5
    virtual void enableTimeConstrained () = 0;

    // 8.7
    virtual void disableTimeConstrained () = 0;

    // 8.8
    virtual void timeAdvanceRequest
    (LogicalTime const & theTime) = 0;

    // 8.9
    virtual void timeAdvanceRequestAvailable
    (LogicalTime const & theTime) = 0;

    // 8.10
    virtual void nextMessageRequest
    (LogicalTime const & theTime) = 0;

    // 8.11
    virtual void nextMessageRequestAvailable
    (LogicalTime const & theTime) = 0;

    // 8.12
    virtual void flushQueueRequest
    (LogicalTime const & theTime) = 0;

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
    virtual void modifyLookahead
    (LogicalTimeInterval const & theLookahead) = 0;

    // 8.20
    virtual void queryLookahead (LogicalTimeInterval & interval) = 0;

    // 8.21
    virtual void retract
    (MessageRetractionHandle theHandle) = 0;

    // 8.23
    virtual void changeAttributeOrderType
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     OrderType theType) = 0;

    // 8.24
    virtual void changeInteractionOrderType
    (InteractionClassHandle theClass,
     OrderType theType) = 0;

    //////////////////////////////////
    // Data Distribution Management //
    //////////////////////////////////
  
    // 9.2
    virtual RegionHandle createRegion
    (DimensionHandleSet const & theDimensions) = 0;

    // 9.3
    virtual void commitRegionModifications
    (RegionHandleSet const & theRegionHandleSet) = 0;

    // 9.4
    virtual void deleteRegion
    (RegionHandle theRegion) = 0;

    // 9.5
    virtual ObjectInstanceHandle registerObjectInstanceWithRegions
    (ObjectClassHandle theClass,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector) = 0;

    virtual ObjectInstanceHandle registerObjectInstanceWithRegions
    (ObjectClassHandle theClass,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector,
     std::wstring const & theObjectInstanceName) = 0;

    // 9.6
    virtual void associateRegionsForUpdates
    (ObjectInstanceHandle theObject,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector) = 0;

    // 9.7
    virtual void unassociateRegionsForUpdates
    (ObjectInstanceHandle theObject,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector) = 0;

    // 9.8
    virtual void subscribeObjectClassAttributesWithRegions
    (ObjectClassHandle theClass,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector,
     bool active = true) = 0;

    // 9.9
    virtual void unsubscribeObjectClassAttributesWithRegions
    (ObjectClassHandle theClass,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector) = 0;

    // 9.10
    virtual void subscribeInteractionClassWithRegions
    (InteractionClassHandle theClass,
     RegionHandleSet const & theRegionHandleSet,
     bool active = true) = 0;

    // 9.11
    virtual void unsubscribeInteractionClassWithRegions
    (InteractionClassHandle theClass,
     RegionHandleSet const & theRegionHandleSet) = 0;

    // 9.12
    virtual void sendInteractionWithRegions
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     RegionHandleSet const & theRegionHandleSet,
     VariableLengthData const & theUserSuppliedTag) = 0;

    virtual MessageRetractionHandle sendInteractionWithRegions
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     RegionHandleSet const & theRegionHandleSet,
     VariableLengthData const & theUserSuppliedTag,
     LogicalTime const & theTime) = 0;

    // 9.13
    virtual void requestAttributeValueUpdateWithRegions
    (ObjectClassHandle theClass,
     AttributeHandleSetRegionHandleSetPairVector const & theSet,
     VariableLengthData const & theUserSuppliedTag) = 0;

    //////////////////////////
    // RTI Support Services //
    //////////////////////////
  
    // 10.2
    virtual ObjectClassHandle getObjectClassHandle
    (std::wstring const & theName) = 0;

    // 10.3
    virtual std::wstring getObjectClassName
    (ObjectClassHandle theHandle) = 0;

    // 10.4
    virtual AttributeHandle getAttributeHandle
    (ObjectClassHandle whichClass,
     std::wstring const & theAttributeName) = 0;

    // 10.5
    virtual std::wstring getAttributeName
    (ObjectClassHandle whichClass,
     AttributeHandle theHandle)    = 0;

    // 10.6
    virtual InteractionClassHandle getInteractionClassHandle
    (std::wstring const & theName) = 0;

    // 10.7
    virtual std::wstring getInteractionClassName
    (InteractionClassHandle theHandle) = 0;

    // 10.8
    virtual ParameterHandle getParameterHandle
    (InteractionClassHandle whichClass,
     std::wstring const & theName) = 0;

    // 10.9
    virtual std::wstring getParameterName
    (InteractionClassHandle whichClass,
     ParameterHandle theHandle)    = 0;

    // 10.10
    virtual ObjectInstanceHandle getObjectInstanceHandle
    (std::wstring const & theName) = 0;

    // 10.11
    virtual std::wstring getObjectInstanceName
    (ObjectInstanceHandle theHandle) = 0;

    // 10.12
    virtual DimensionHandle getDimensionHandle
    (std::wstring const & theName) = 0;

    // 10.13
    virtual std::wstring getDimensionName
    (DimensionHandle theHandle) = 0;

    // 10.14
    virtual unsigned long getDimensionUpperBound
    (DimensionHandle theHandle)    = 0;

    // 10.15
    virtual DimensionHandleSet getAvailableDimensionsForClassAttribute
    (ObjectClassHandle theClass,
     AttributeHandle theHandle)    = 0;

    // 10.16
    virtual ObjectClassHandle getKnownObjectClassHandle
    (ObjectInstanceHandle theObject) = 0;

    // 10.17
    virtual DimensionHandleSet getAvailableDimensionsForInteractionClass
    (InteractionClassHandle theClass) = 0;

    // 10.18
    virtual TransportationType getTransportationType
    (std::wstring const & transportationName) = 0;

    // 10.19
    virtual std::wstring getTransportationName
    (TransportationType transportationType) = 0;

    // 10.20
    virtual OrderType getOrderType
    (std::wstring const & orderName) = 0;

    // 10.21
    virtual std::wstring getOrderName
    (OrderType orderType) = 0;

    // 10.22
    virtual void enableObjectClassRelevanceAdvisorySwitch () = 0;

    // 10.23
    virtual void disableObjectClassRelevanceAdvisorySwitch () = 0;

    // 10.24
    virtual void enableAttributeRelevanceAdvisorySwitch () = 0;

    // 10.25
    virtual void disableAttributeRelevanceAdvisorySwitch () = 0;

    // 10.26
    virtual void enableAttributeScopeAdvisorySwitch () = 0;

    // 10.27
    virtual void disableAttributeScopeAdvisorySwitch () = 0;

    // 10.28
    virtual void enableInteractionRelevanceAdvisorySwitch () = 0;

    // 10.29
    virtual void disableInteractionRelevanceAdvisorySwitch () = 0;

    // 10.30
    virtual
    DimensionHandleSet getDimensionHandleSet
    (RegionHandle theRegionHandle) = 0;

    // 10.31
    virtual
    RangeBounds getRangeBounds
    (RegionHandle theRegionHandle,
     DimensionHandle theDimensionHandle) = 0;

    // 10.32
    virtual void setRangeBounds
    (RegionHandle theRegionHandle,
     DimensionHandle theDimensionHandle,
     RangeBounds const & theRangeBounds) = 0;

    // 10.33
    virtual unsigned long normalizeFederateHandle
    (FederateHandle theFederateHandle) = 0;

    // 10.34
    virtual unsigned long normalizeServiceGroup
    (ServiceGroupIndicator theServiceGroup) = 0;

    // 10.37
    virtual bool evokeCallback(double approximateMinimumTimeInSeconds) = 0;

    // 10.38
    virtual bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                                        double approximateMaximumTimeInSeconds) = 0;

    // 10.39
    virtual void enableCallbacks () = 0;

    // 10.40
    virtual void disableCallbacks () = 0;

    virtual FederateHandle decodeFederateHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual ObjectClassHandle decodeObjectClassHandle(
       VariableLengthData const & encodedValue) const = 0;
    
    virtual InteractionClassHandle decodeInteractionClassHandle(
       VariableLengthData const & encodedValue) const = 0;
 
    virtual ObjectInstanceHandle decodeObjectInstanceHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual AttributeHandle decodeAttributeHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual ParameterHandle decodeParameterHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual DimensionHandle decodeDimensionHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual MessageRetractionHandle decodeMessageRetractionHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual RegionHandle decodeRegionHandle(
       VariableLengthData const & encodedValue) const = 0;

  };
}

#endif // RTI_RTIambassador_h
