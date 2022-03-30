
#pragma once

#include "AbstractConnect.h"
#include "AbstractServer.h"
#include "Export.h"

#include "ThreadServer.h"
#include "ServerNode.h"
#include "SharedPtr.h"
#include "InternalAmbassador.h"
#include "Federate.h"
#include "MomTypes.h"
#include <memory>
#include <functional>

namespace OpenRTI {

class AbstractConnect;
class AbstractServerNode;
class AbstractServer;
class LeafServerThread;
class MomManager;
struct AbstractFederateMetrics;

namespace ServerModel {
class Federation;
}

//class MomTimeManagement;

// This is a fake ambassador, similar to Ambassador<T>.
// It has no TimeManagement, and does (almost) no translation from/to OpenRTI internal parameter types.
class OPENRTI_LOCAL MomServer : public Referenced
{
  public:
    MomServer(ServerModel::Federation* federation, AbstractServer* server, bool isRoot, FederateHandle ownerFederateHandle);
    virtual ~MomServer();

    void connect(AbstractServer* server);
    const FederateHandle& getFederateHandle() const
    {
      return _rtiFederate->getFederateHandle();
    }
    FederateHandle getFederateHandle(const std::string& name);

    const FederationHandle& getFederationHandle() const
    {
      return _rtiFederate->getFederationHandle();
    }
    bool isRoot() const noexcept { return _isRoot; }

    void accept(const AbstractMessage& message);

    void accept(const InsertFederationExecutionMessage& message);
    void accept(const JoinFederationExecutionResponseMessage& message);
    void accept(const JoinFederateNotifyMessage& message);
    void accept(const ResignFederateNotifyMessage& message);
    void accept(const InsertModulesMessage& message);
    void accept(const EraseFederationExecutionMessage& message);

    void accept(const EnableTimeRegulationRequestMessage& message);
    void accept(const EnableTimeConstrainedNotifyMessage& message);
    void accept(const CommitLowerBoundTimeStampMessage& /*message*/)
    {
      // currently ignored (can we make sth. out of this?)
    }

    void accept(const AttributeUpdateMessage& message);
    void accept(const RequestAttributeUpdateMessage& message);
    void accept(const InteractionMessage& message);
    void accept(const ReserveObjectInstanceNameRequestMessage& message);
    void accept(const ReserveObjectInstanceNameResponseMessage& message);
    void accept(const InsertObjectInstanceMessage& message);
    void accept(const DeleteObjectInstanceMessage& message);
    void accept(const ChangeObjectClassSubscriptionMessage& /*message*/)
    {
      // ignore this
    }
    void accept(const ChangeObjectClassPublicationMessage& /*message*/)
    {
      // ignore this
    }
    void accept(const ChangeInteractionClassSubscriptionMessage& /*message*/)
    {
      // ignore this
    }
    void accept(const ChangeInteractionClassPublicationMessage& /*message*/)
    {
      // ignore this
    }

    Federate* getRtiFederate() const
    {
      return _rtiFederate.get();
    }

    void joinFederationExecution(ServerModel::Federation* federation);
    void resignFederationExecution();
    void publishObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeHandleVector);
    void unpublishObjectClass(ObjectClassHandle objectClassHandle);
    void subscribeObjectClassAttributes(ObjectClassHandle objectClassHandle, AttributeHandleSet const& attributes, bool active);

    void reserveObjectInstanceName(const std::string& objectInstanceName, std::function<void()> callback);
    ObjectInstanceHandle registerObjectInstance(ObjectClassHandle objectClassHandle, const std::string& objectInstanceName);
    void deleteObjectInstance(ObjectInstanceHandle objectInstanceHandle);
    void updateAttributeValues(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleValueMap& attributes);
    void requestAttributeValueUpdate(ObjectInstanceHandle objectInstanceHandle, AttributeHandleSet const& attributes);

    void publishInteractionClass(InteractionClassHandle interactionClassHandle);
    void subscribeInteractionClassWithFilter(InteractionClassHandle interactionClassHandle,
        ParameterHandleValueMap filterValues,
        bool active);
    void subscribeInteractionClass(InteractionClassHandle interactionClassHandle, bool active);
    void unsubscribeInteractionClass(InteractionClassHandle InteractionClassHandle);
    void sendInteraction(InteractionClassHandle interactionClassHandle, const ParameterHandleValueMap& parameterValues);


    const std::string& getFederateName(FederateHandle federateHandle);
    const std::string& getFederateType(FederateHandle federateHandle);

    const std::string& getObjectClassName(ObjectClassHandle objectClassHandle);
    ObjectClassHandle getObjectClassHandle(const std::string& name);
    ObjectInstanceHandle getObjectInstanceHandle(const std::string& name);
    const std::string& getObjectInstanceName(ObjectInstanceHandle objectInstanceHandle);
    ObjectClassHandle getKnownObjectClassHandle(ObjectInstanceHandle objectInstanceHandle);
    AttributeHandle getAttributeHandle(ObjectClassHandle objectClassHandle, const std::string& name);

    const std::string& getInteractionClassName(InteractionClassHandle interactionClassHandle);
    InteractionClassHandle getInteractionClassHandle(const std::string& name);
    ParameterHandle getParameterHandle(InteractionClassHandle interactionClassHandle, const std::string& name);

    FederateHandle getOwnerFederate() const { return _ownerFederate; }
    std::shared_ptr<AbstractFederateMetrics> getFederateMetrics();

    // the RTI federate entered the scene ...
    void notifyFederationJoined(FederateHandle federate, const std::string& federateName, const std::string& federateType);
    void notifyFederateResigned(FederateHandle federate);

    void sendRequest(const SharedPtr<const AbstractMessage>& message);

    bool getIsTimeConstrained(FederateHandle federateHandle) const;
    bool getIsTimeRegulating(FederateHandle federateHandle) const;

  private:
    void initializeMomManager();
    void _releaseObjectInstance(const ObjectInstanceHandle& objectInstanceHandle);
  private:

    class _InternalMessageDispatchFunctor;
    //class _EraseFederationExecutionFunctor;
    class _MomMessageDispatcher;

    bool _isRoot;

    // the MomManager actually aggregates all relevant data about the federation and its federates
    std::shared_ptr<MomManager> _momManager;

    // the connection to the server
    SharedPtr<AbstractConnect> _connect;
    // the dispatcher given to the server (which is an instance of NetworkServer) for sending us back stuff
    SharedPtr<_MomMessageDispatcher> _dispatcher;
    // the federation in the server model
    ServerModel::Federation* _federation;
    // This represents the RTI/MOM federate (the federate subscribing and publishing MOM stuff ...)
    // see Ambassador<T>::accept
    SharedPtr<Federate> _rtiFederate;
    // The federate which originally joined the federation and created THIS INSTANCE of the MomServer
    // It's only set in a leaf server.
    // TODO: this should better be factored out somehow, because one day we want more than
    // one federate share the same process (and thus Leaf NetworkServer)
    FederateHandle _ownerFederate;
    // helper for joinFederation
    JoinFederationExecutionResponseType _joinResponse;
    // helper for reserveObjectInstanceName
    std::map<std::string, std::function<void()>> mPendingNameReservations;
};

}


