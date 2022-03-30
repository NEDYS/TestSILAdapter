// HLA 1.3 Header "RTIambServices.hh"

typedef FederateAmbassador *FederateAmbassadorPtr ;

// Federation Management -------------------

void createFederationExecution(const char *executionName, const char *FEDid);

void destroyFederationExecution(const char *);

FederateHandle joinFederationExecution(const char *, const char *, FederateAmbassadorPtr);

void resignFederationExecution(ResignAction);

void registerFederationSynchronizationPoint(const char *, const char *);

void registerFederationSynchronizationPoint(const char *, const char *, const FederateHandleSet &);

void synchronizationPointAchieved(const char *);

void requestFederationSave(const char *, const FedTime &);

void requestFederationSave(const char *);

void federateSaveBegun();

void federateSaveComplete();

void federateSaveNotComplete();

void requestFederationRestore(const char *);

void federateRestoreComplete();

void federateRestoreNotComplete();

// Declaration Management -------------------

void publishObjectClass(ObjectClassHandle, const AttributeHandleSet &);

void unpublishObjectClass(ObjectClassHandle);

void publishInteractionClass(InteractionClassHandle);

void unpublishInteractionClass(InteractionClassHandle);

void subscribeObjectClassAttributes(ObjectClassHandle, const AttributeHandleSet &, Boolean = RTI_TRUE);

void unsubscribeObjectClass(ObjectClassHandle);

void subscribeInteractionClass(InteractionClassHandle, Boolean = RTI_TRUE);

void unsubscribeInteractionClass(InteractionClassHandle);

// Object Management -------------------

ObjectHandle registerObjectInstance(ObjectClassHandle, const char *);

ObjectHandle registerObjectInstance(ObjectClassHandle);

EventRetractionHandle updateAttributeValues(ObjectHandle, const AttributeHandleValuePairSet &,
					    const FedTime &, const char *);

void updateAttributeValues(ObjectHandle, const AttributeHandleValuePairSet &, const char *);

EventRetractionHandle sendInteraction(InteractionClassHandle, const ParameterHandleValuePairSet &,
				      const FedTime &, const char *);

void sendInteraction(InteractionClassHandle, const ParameterHandleValuePairSet &, const char *);

EventRetractionHandle deleteObjectInstance(ObjectHandle, const FedTime &, const char *);

void deleteObjectInstance(ObjectHandle, const char *);

void localDeleteObjectInstance(ObjectHandle);

void changeAttributeTransportationType(ObjectHandle, const AttributeHandleSet &, TransportationHandle);

void changeInteractionTransportationType(InteractionClassHandle, TransportationHandle);

void requestObjectAttributeValueUpdate(ObjectHandle, const AttributeHandleSet &);

void requestClassAttributeValueUpdate(ObjectClassHandle, const AttributeHandleSet &);

// Ownership Management -------------------

void unconditionalAttributeOwnershipDivestiture(ObjectHandle, const AttributeHandleSet &);

void negotiatedAttributeOwnershipDivestiture(ObjectHandle, const AttributeHandleSet &, const char *);

void attributeOwnershipAcquisition(ObjectHandle, const AttributeHandleSet &desiredAttributes, const char *);

void attributeOwnershipAcquisitionIfAvailable(ObjectHandle, const AttributeHandleSet &);

AttributeHandleSet *attributeOwnershipReleaseResponse(ObjectHandle, const AttributeHandleSet &);

void cancelNegotiatedAttributeOwnershipDivestiture(ObjectHandle, const AttributeHandleSet &);

void cancelAttributeOwnershipAcquisition(ObjectHandle, const AttributeHandleSet &);

void queryAttributeOwnership(ObjectHandle, AttributeHandle);

Boolean isAttributeOwnedByFederate(ObjectHandle, AttributeHandle);

// Time Management -------------------

void enableTimeRegulation(const FedTime &, const FedTime &);

void disableTimeRegulation();

void enableTimeConstrained();

void disableTimeConstrained();

void timeAdvanceRequest(const FedTime &);

void timeAdvanceRequestAvailable(const FedTime &);

void nextEventRequest(const FedTime &);

void nextEventRequestAvailable(const FedTime &);

void flushQueueRequest(const FedTime &);

void enableAsynchronousDelivery();

void disableAsynchronousDelivery();

void queryLBTS(FedTime &);

void queryFederateTime(FedTime &);

void queryMinNextEventTime(FedTime &);

void modifyLookahead(const FedTime &);

void queryLookahead(FedTime &);

void retract(EventRetractionHandle theHandle);

void changeAttributeOrderType(ObjectHandle, const AttributeHandleSet &, OrderingHandle);

void changeInteractionOrderType(InteractionClassHandle, OrderingHandle);

// Data Distribution Management -------------------

Region *createRegion(SpaceHandle, ULong);

void notifyAboutRegionModification(Region &theRegion);

void deleteRegion(Region *);

ObjectHandle registerObjectInstanceWithRegion(ObjectClassHandle, const char *, AttributeHandle [],
					      Region *theRegions[], ULong);

ObjectHandle registerObjectInstanceWithRegion(ObjectClassHandle, AttributeHandle [], Region *regions[], ULong);

void associateRegionForUpdates(Region &, ObjectHandle, const AttributeHandleSet &);

void unassociateRegionForUpdates(Region &, ObjectHandle);

void subscribeObjectClassAttributesWithRegion(ObjectClassHandle, Region &, const AttributeHandleSet &,
					      Boolean = RTI_TRUE);

void unsubscribeObjectClassWithRegion(ObjectClassHandle, Region &);

void subscribeInteractionClassWithRegion(InteractionClassHandle, Region &, Boolean = RTI_TRUE);

void unsubscribeInteractionClassWithRegion(InteractionClassHandle, Region &);

EventRetractionHandle sendInteractionWithRegion(InteractionClassHandle, const ParameterHandleValuePairSet &,
						const FedTime &, const char *, const Region &);

void sendInteractionWithRegion(InteractionClassHandle, const ParameterHandleValuePairSet &,
			       const char *, const Region &);

void requestClassAttributeValueUpdateWithRegion(ObjectClassHandle, const AttributeHandleSet &, const Region &);

// Support Services -------------------

ObjectClassHandle getObjectClassHandle(const char *)
    ;

char *getObjectClassName(ObjectClassHandle)
    ;

AttributeHandle getAttributeHandle(const char *, ObjectClassHandle);

char *getAttributeName(AttributeHandle, ObjectClassHandle);

InteractionClassHandle getInteractionClassHandle(const char *)
    ;

char *getInteractionClassName(InteractionClassHandle)
    ;

ParameterHandle getParameterHandle(const char *, InteractionClassHandle);

char *getParameterName(ParameterHandle, InteractionClassHandle);

ObjectHandle getObjectInstanceHandle(const char *)
    ;

char *getObjectInstanceName(ObjectHandle)
    ;

SpaceHandle getRoutingSpaceHandle(const char *)
    ;

char *getRoutingSpaceName(SpaceHandle)
    ;

DimensionHandle getDimensionHandle(const char *, SpaceHandle);

char *getDimensionName(DimensionHandle, SpaceHandle);

SpaceHandle getAttributeRoutingSpaceHandle(AttributeHandle, ObjectClassHandle);

ObjectClassHandle getObjectClass(ObjectHandle)
    ;

SpaceHandle getInteractionRoutingSpaceHandle(InteractionClassHandle)
    ;

TransportationHandle getTransportationHandle(const char *)
    ;

char *getTransportationName(TransportationHandle)
    ;

OrderingHandle getOrderingHandle(const char *)
    ;

char *getOrderingName(OrderingHandle)
    ;

void enableClassRelevanceAdvisorySwitch();

void disableClassRelevanceAdvisorySwitch();

void enableAttributeRelevanceAdvisorySwitch();

void disableAttributeRelevanceAdvisorySwitch();

void enableAttributeScopeAdvisorySwitch();

void disableAttributeScopeAdvisorySwitch();

void enableInteractionRelevanceAdvisorySwitch();

void disableInteractionRelevanceAdvisorySwitch();

Boolean tick()
    ;

Boolean tick(TickTime, TickTime)
    ;

RegionToken getRegionToken(Region *)
    ;

Region *getRegion(RegionToken)
    ;

RTIambassador()
    ;

~RTIambassador()
    ;
