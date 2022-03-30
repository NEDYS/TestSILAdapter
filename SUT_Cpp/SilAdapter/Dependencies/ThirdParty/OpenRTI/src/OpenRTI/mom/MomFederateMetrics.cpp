


#include "DebugNew.h"
#include "MomFederateMetrics.h"
#include "MomManager.h"
#include "MomFederate.h"
#include "dprintf.h"


namespace OpenRTI {

MomFederateMetrics::MomFederateMetrics(MomManager* mom)
{
  mFederate = mom->getFederate();
}


MomFederateMetrics::~MomFederateMetrics() noexcept
{}

void MomFederateMetrics::reflectionReceived(ObjectClassHandle classId, ObjectInstanceHandle instanceId)
{
  if (mObjectInstancesReflected.find(classId) == mObjectInstancesReflected.end())
  {
    mObjectInstancesReflected.insert(std::make_pair(classId, OCMetricTracker(classId)));
  }
  mObjectInstancesReflected[classId].increment(instanceId);
  if (mFederate != nullptr) mFederate->SetReflectionsReceived(mFederate->GetReflectionsReceived() + 1);
}

void MomFederateMetrics::sentUpdate(ObjectClassHandle classId, ObjectInstanceHandle instanceId)
{
  if (mObjectInstancesUpdated.find(classId) == mObjectInstancesUpdated.end())
  {
    mObjectInstancesUpdated.insert(std::make_pair(classId, OCMetricTracker(classId)));
  }
  mObjectInstancesUpdated[classId].increment(instanceId);
  if (mFederate != nullptr) mFederate->SetUpdatesSent(mFederate->GetUpdatesSent() + 1);
}


void MomFederateMetrics::interactionReceived(InteractionClassHandle classId)
{
  if (interactionsReceived.find(classId) == interactionsReceived.end())
  {
    interactionsReceived.insert(std::make_pair(classId, HLAinteractionCount(classId, 0)));
  }
  interactionsReceived[classId].increment();
  if (mFederate != nullptr) mFederate->SetInteractionsReceived(mFederate->GetInteractionsReceived() + 1);
}

void MomFederateMetrics::interactionSent(InteractionClassHandle classId)
{
  if (interactionsSent.find(classId) == interactionsSent.end())
  {
    interactionsSent.insert(std::make_pair(classId, HLAinteractionCount(classId, 0)));
  }
  interactionsSent[classId].increment();
  if (mFederate != nullptr) mFederate->SetInteractionsSent(mFederate->GetInteractionsSent() + 1);
}

void MomFederateMetrics::objectRegistered(ObjectInstanceHandle instanceId)
{
  // When a federate registers an object, they hold privilege to delete until such time as they
  // yield it, or remove it
  objectsOwned.insert(instanceId);
}

void MomFederateMetrics::objectClassSubscribed(ObjectClassHandle classId, const AttributeHandleVector& attributes, bool active)
{
  auto iter = objectClassSubscriptions.find(classId);
  if (iter == objectClassSubscriptions.end())
  {
    objectClassSubscriptions.insert(std::make_pair(classId, std::make_pair(AttributeHandleSet(attributes.begin(), attributes.end()), active)));
  }
  else
  {
    auto& existingSubscription = iter->second;
    existingSubscription.first.insert(attributes.begin(), attributes.end());
  }
}

void MomFederateMetrics::objectClassUnsubscribed(ObjectClassHandle classId, const AttributeHandleVector& attributes)
{
  auto iter = objectClassSubscriptions.find(classId);
  if (iter != objectClassSubscriptions.end())
  {
    auto& existingSubscription = iter->second;
    for (auto attribute : attributes)
    {
      existingSubscription.first.erase(attribute);
    }
    if (existingSubscription.first.empty())
    {
      objectClassSubscriptions.erase(classId);
    }
  }
}

void MomFederateMetrics::objectClassPublished(ObjectClassHandle classId, const AttributeHandleVector& attributes)
{
  auto iter = objectClassPublications.find(classId);
  if (iter == objectClassPublications.end())
  {
    objectClassPublications.insert(std::make_pair(classId, AttributeHandleSet(attributes.begin(), attributes.end())));
  }
  else
  {
    AttributeHandleSet& existingAttributes = iter->second;
    existingAttributes.insert(attributes.begin(), attributes.end());
  }
}

void MomFederateMetrics::objectClassUnpublished(ObjectClassHandle classId, const AttributeHandleVector& attributes)
{
  auto iter = objectClassPublications.find(classId);
  if (iter != objectClassPublications.end())
  {
    auto& existingPublication= iter->second;
    for (auto attribute : attributes)
    {
      existingPublication.erase(attribute);
    }
    if (existingPublication.empty())
    {
      objectClassPublications.erase(classId);
    }
  }
}

void MomFederateMetrics::interactionSubscribed(InteractionClassHandle classId, bool active)
{
  interactionSubcriptions.insert(std::make_pair(classId, active));
}

void MomFederateMetrics::interactionUnsubscribed(InteractionClassHandle classId)
{
  interactionSubcriptions.erase(classId);
}

void MomFederateMetrics::interactionPublished(InteractionClassHandle classId)
{
  interactionPublications.insert(classId);
}

void MomFederateMetrics::interactionUnpublished(InteractionClassHandle classId)
{
  interactionPublications.erase(classId);
}

VariableLengthData MomFederateMetrics::getNumberOfPublishedClasses() const
{
  HLAcount count(static_cast<int32_t>(objectClassPublications.size()));
  return count.encode();
}

VariableLengthData MomFederateMetrics::getNumberOfSubscribedClasses() const
{
  HLAcount count(static_cast<int32_t>(objectClassSubscriptions.size()));
  return count.encode();
}

void MomFederateMetrics::setFederate(std::shared_ptr<MomFederate> federate)
{
  mFederate = federate;
}

VariableLengthData MomFederateMetrics::getObjectInstancesReflected() const
{
  HLAvariableArray counts = HLAvariableArray(HLAobjectClassBasedCount(ObjectClassHandle(), HLAcount()));
  for (auto& entry : mObjectInstancesReflected)
  {
    counts.addElement(entry.second.getTotalUniqueInstances());
  }
  return counts.encode();
}

VariableLengthData MomFederateMetrics::getObjectInstancesUpdated() const
{
  HLAvariableArray counts = HLAvariableArray(HLAobjectClassBasedCount(ObjectClassHandle(), HLAcount()));
  for (auto& entry : mObjectInstancesUpdated)
  {
    counts.addElement(entry.second.getTotalUniqueInstances());
  }
  return counts.encode();
}

VariableLengthData MomFederateMetrics::getObjectClassUpdatesSent() const
{
  HLAvariableArray counts = HLAvariableArray(HLAobjectClassBasedCount(ObjectClassHandle(), HLAcount()));
  for (auto& entry : mObjectInstancesUpdated)
  {
    counts.addElement(entry.second.getTotal());
  }
  return counts.encode();
}

VariableLengthData MomFederateMetrics::getObjectClassReflectionsReceived() const
{
  HLAvariableArray counts = HLAvariableArray(HLAobjectClassBasedCount(ObjectClassHandle(), HLAcount()));
  for (auto& entry : mObjectInstancesReflected)
  {
    counts.addElement(entry.second.getTotal());
  }
  return counts.encode();
}

VariableLengthData MomFederateMetrics::getInteractionsReceived() const
{
  HLAvariableArray counts = HLAvariableArray(HLAinteractionCount(InteractionClassHandle(), HLAcount()));
  for (auto& entry : interactionsReceived)
  {
    counts.addElement(entry.second);
  }
  return counts.encode();
}

VariableLengthData MomFederateMetrics::getInteractionsSent() const
{
  HLAvariableArray counts = HLAvariableArray(HLAinteractionCount(InteractionClassHandle(), HLAcount()));
  for (auto& entry : interactionsSent)
  {
    counts.addElement(entry.second);
  }
  return counts.encode();
}

VariableLengthData MomFederateMetrics::getInteractionSubscriptions() const
{
  HLAvariableArray subscriptions = HLAvariableArray(HLAinteractionSubscription(InteractionClassHandle(), HLAboolean()));
  for (auto& entry : interactionSubcriptions)
  {
    subscriptions.addElement(HLAinteractionSubscription(entry.first, entry.second));
  }
  return subscriptions.encode();
}

VariableLengthData MomFederateMetrics::getInteractionPublications() const
{
  HLAvariableArray subscriptions = HLAvariableArray(HLAhandle(InteractionClassHandle()));
  for (auto& interactionClass : interactionPublications)
  {
    subscriptions.addElement(HLAhandle(interactionClass));
  }
  return subscriptions.encode();
}

}