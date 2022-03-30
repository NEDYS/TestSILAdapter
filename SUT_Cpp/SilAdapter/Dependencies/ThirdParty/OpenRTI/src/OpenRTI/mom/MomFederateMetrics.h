
#pragma once


#include <list>
#include <map>
#include <set>
#include <algorithm>

//#include "RTI/RTIAmbassador.h"
#include "encoding/BasicDataElements.h"
#include "encoding/HLAhandle.h"
#include "encoding/HLAfixedRecord.h"
#include "encoding/Encoding.h"
#include <functional>
#include "AbstractFederateMetrics.h"

namespace OpenRTI {

// This is the basis for the data provided by the Mom InteractionHandlers.
// The data for the HLAfederate and HLAfederation object is maintained
// in MomFederate and MomFederation, respectively.
// The data collected here relates to the executing federate.
class OPENRTI_LOCAL MomFederateMetrics : public AbstractFederateMetrics
{
  public:
    MomFederateMetrics(MomManager* mom);
    ~MomFederateMetrics() noexcept;

    // counting callbacks (triggered by the appropriate accept methods in the server node)
    void reflectionReceived(ObjectClassHandle classId, ObjectInstanceHandle instanceId) override;
    void sentUpdate(ObjectClassHandle classId, ObjectInstanceHandle instanceId) override;
    void interactionReceived(InteractionClassHandle classId) override;
    void interactionSent(InteractionClassHandle classId) override;
    void objectRegistered(ObjectInstanceHandle instanceId) override;

    void objectClassSubscribed(ObjectClassHandle classId, const AttributeHandleVector& attributes, bool active) override;
    void objectClassUnsubscribed(ObjectClassHandle classId, const AttributeHandleVector& attributes) override;
    void objectClassPublished(ObjectClassHandle classId, const AttributeHandleVector& attributes) override;
    void objectClassUnpublished(ObjectClassHandle classId, const AttributeHandleVector& attributes) override;

    void interactionSubscribed(InteractionClassHandle classId, bool active) override;
    void interactionUnsubscribed(InteractionClassHandle classId) override;
    void interactionPublished(InteractionClassHandle classId) override;
    void interactionUnpublished(InteractionClassHandle classId) override;

    // getter methods, used by the MOM interaction handlers

    // returns how many unique instances (per class) have been reflected
    VariableLengthData getObjectInstancesReflected() const override;
    // returns how many unique instances (per class) have sent updates
    VariableLengthData getObjectInstancesUpdated() const override;
    // returns how many updates, per class, have been sent (total count)
    VariableLengthData getObjectClassUpdatesSent() const override;
    // returns how many reflections, per class, have been received (total count)
    VariableLengthData getObjectClassReflectionsReceived() const override;

    VariableLengthData getInteractionsReceived() const override;
    VariableLengthData getInteractionsSent() const override;
    VariableLengthData getInteractionSubscriptions() const override;
    VariableLengthData getInteractionPublications() const override;

    VariableLengthData getNumberOfPublishedClasses() const override;
    VariableLengthData getNumberOfSubscribedClasses() const override;

    void setFederate(std::shared_ptr<MomFederate> federate);
    const std::map<ObjectClassHandle, std::pair<AttributeHandleSet, bool>>& getObjectClassSubscriptions() const
    {
      return objectClassSubscriptions;
    }
    const std::map<ObjectClassHandle, AttributeHandleSet>& getObjectClassPublications() const
    {
      return objectClassPublications;
    }
private:
    // HLArequestInteractionsReceived => HLAreportInteractionsReceived
    std::map<InteractionClassHandle, HLAinteractionCount> interactionsReceived;
    // HLArequestInteractionsSent => HLAreportInteractionsSent
    std::map<InteractionClassHandle, HLAinteractionCount> interactionsSent;

    // HLAreportInteractionPublication
    std::set<InteractionClassHandle> interactionPublications;
    std::map<InteractionClassHandle, bool> interactionSubcriptions;
    // HLAreportObjectClassPublication
    std::map<ObjectClassHandle, AttributeHandleSet> objectClassPublications;
    // HLAreportObjectClassSubscription
    std::map<ObjectClassHandle, std::pair<AttributeHandleSet, bool>> objectClassSubscriptions;

    std::set<ObjectInstanceHandle> objectsOwned;
    // NOTE: these counts work by class, not by instance
    // HLArequestObjectInstancesUpdated => HLAreportObjectInstancesUpdated
    std::map<ObjectClassHandle, OCMetricTracker> mObjectInstancesUpdated;
    // HLArequestObjectInstancesReflected => HLAreportObjectInstancesReflected
    std::map<ObjectClassHandle, OCMetricTracker> mObjectInstancesReflected;

    //HLAcount numberOfClasses;
    //HLAcount objectsDeleted;
    //HLAcount objectsRemoved;
    //HLAcount objectsRegistered;
    //HLAcount objectsDiscovered;
    //HLAcount serviceInvocations;

    std::shared_ptr<MomFederate> mFederate;
};

}