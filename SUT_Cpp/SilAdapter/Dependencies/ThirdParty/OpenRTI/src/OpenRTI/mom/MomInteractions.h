
#pragma once

#include <list>
#include <map>
#include <functional>

//#include "RTI/RTIAmbassador.h"
//#include "RTI/encoding/BasicDataElements.h"
//#include "RTI/encoding/HLAhandle.h"
//#include "RTI/encoding/HLAvariableArray.h"
#include "MomManager.h"

namespace OpenRTI {

class OPENRTI_LOCAL MomInteractionHandler
{
public:
  typedef std::pair<InteractionClassHandle, ParameterHandleValueMap> ResponseType;
  typedef std::list<ResponseType> ResponseList;

  MomInteractionHandler(std::shared_ptr<MomManager> mom);
  virtual ~MomInteractionHandler() {}
  FederateHandle getFederateHandleFromParameters(const ParameterHandleValueMap& requestParameters);
  FederateHandle getFederateHandleFromResponse(const ParameterHandleValueMap& responseParameters);
  std::shared_ptr<MomFederate> getFederateFromParameters(const ParameterHandleValueMap& requestParameters);
  virtual ResponseList operator()(const ParameterHandleValueMap& requestParameters);

  InteractionClassHandle getRequestInteractionClass() const { return mRequestInteractionHandle; }
  std::set<InteractionClassHandle> getResponseInteractionClasses() const {
    std::set<InteractionClassHandle> result;
    for (auto& entry : mResponseInteractions)
    {
      result.insert(entry.first);
    }
    return result;
  }
protected:
  const VariableLengthData kEmptyValue;

  InteractionClassHandle mRequestInteractionHandle;
  std::vector<ResponseType> mResponseInteractions;
  ParameterHandle mResponseFederateParameter;
  ParameterHandle mRequestFederateParameter;
  std::map<ParameterHandle, std::function<VariableLengthData()>> mParameterGetters;
  MomManager* mMomManager; // borrowed pointer, must not create circular reference
};

class OPENRTI_LOCAL InteractionHandlerFederateSetServiceReporting : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateSetServiceReporting(std::shared_ptr<MomManager> mom);
    ResponseList operator()(const ParameterHandleValueMap& requestParameters) override;

};

class OPENRTI_LOCAL InteractionHandlerFederateSetExceptionReporting : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateSetExceptionReporting(std::shared_ptr<MomManager> mom);
    ResponseList operator()(const ParameterHandleValueMap& requestParameters) override;
};

class OPENRTI_LOCAL InteractionHandlerFederateRequestPublications : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateRequestPublications(std::shared_ptr<MomManager> mom);
    ResponseList operator()(const ParameterHandleValueMap& requestParameters) override;
  private:
    ParameterHandle _classHandleParameter;
    ParameterHandle _attributeListParameter;
};

class OPENRTI_LOCAL InteractionHandlerFederateRequestSubscriptions : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateRequestSubscriptions(std::shared_ptr<MomManager> mom);
    ResponseList operator()(const ParameterHandleValueMap& requestParameters) override;
  private:
    ParameterHandle _classHandleParameter;
    ParameterHandle _activeSubscriptionParameter;
    ParameterHandle _attributeListParameter;
};

class OPENRTI_LOCAL InteractionHandlerFederateRequestObjectInstancesThatCanBeDeleted : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateRequestObjectInstancesThatCanBeDeleted(std::shared_ptr<MomManager> mom);
    ResponseList operator()(const ParameterHandleValueMap& requestParameters) override;
};

class OPENRTI_LOCAL InteractionHandlerFederateRequestObjectInstancesUpdated : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateRequestObjectInstancesUpdated(std::shared_ptr<MomManager> mom);
    // default: ResponseList operator()(const ParameterHandleValueMap& parameters) override;
};

class OPENRTI_LOCAL InteractionHandlerFederateRequestObjectInstancesReflected : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateRequestObjectInstancesReflected(std::shared_ptr<MomManager> mom);
    // default: ResponseList operator()(const ParameterHandleValueMap& parameters) override;
};

class OPENRTI_LOCAL InteractionHandlerFederateRequestUpdatesSent : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateRequestUpdatesSent(std::shared_ptr<MomManager> mom);
    // default: ResponseList operator()(const ParameterHandleValueMap& parameters) override;
};

class OPENRTI_LOCAL InteractionHandlerFederateRequestReflectionsReceived : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateRequestReflectionsReceived(std::shared_ptr<MomManager> mom);
    // default: ResponseList operator()(const ParameterHandleValueMap& parameters) override;
};

class OPENRTI_LOCAL InteractionHandlerFederateRequestInteractionsSent : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateRequestInteractionsSent(std::shared_ptr<MomManager> mom);
    // default: ResponseList operator()(const ParameterHandleValueMap& parameters) override;
};

class OPENRTI_LOCAL InteractionHandlerFederateRequestInteractionsReceived: public MomInteractionHandler
{
  public:
    InteractionHandlerFederateRequestInteractionsReceived(std::shared_ptr<MomManager> mom);
    // default: ResponseList operator()(const ParameterHandleValueMap& parameters) override;
};

class OPENRTI_LOCAL InteractionHandlerFederateRequestObjectInstanceInformation : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateRequestObjectInstanceInformation(std::shared_ptr<MomManager> mom);
};

class OPENRTI_LOCAL InteractionHandlerFederateRequestFomModuleData : public MomInteractionHandler
{
  public:
    InteractionHandlerFederateRequestFomModuleData(std::shared_ptr<MomManager> mom);
};

class OPENRTI_LOCAL InteractionHandlerFederationRequestSynchronizationPoints : public MomInteractionHandler
{
  public:
    InteractionHandlerFederationRequestSynchronizationPoints(std::shared_ptr<MomManager> mom);
};

class OPENRTI_LOCAL InteractionHandlerFederationRequestSynchronizationPointStatus : public MomInteractionHandler
{
  public:
    InteractionHandlerFederationRequestSynchronizationPointStatus(std::shared_ptr<MomManager> mom);
};

class OPENRTI_LOCAL InteractionHandlerFederationRequestFomModuleData : public MomInteractionHandler
{
  public:
    InteractionHandlerFederationRequestFomModuleData(std::shared_ptr<MomManager> mom);
};

class OPENRTI_LOCAL InteractionHandlerFederationRequestMIMData : public MomInteractionHandler
{
  public:
    InteractionHandlerFederationRequestMIMData(std::shared_ptr<MomManager> mom);
};

}