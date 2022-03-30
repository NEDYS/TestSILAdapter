#pragma once

#include "MomServer.h"
#include "MomTypes.h"
#include "encoding/BasicDataElements.h"
#include "encoding/HLAhandle.h"
#include "encoding/HLAvariableArray.h"

#include <list>
#include <map>
#include <functional>
#include "Handle.h"
#include "Message.h"

namespace OpenRTI {

class MomManager;
class MomFederate;

class OPENRTI_LOCAL MomFederation : public std::enable_shared_from_this<MomFederation>
{
  public:
    MomFederation(std::shared_ptr<MomManager> mom, std::wstring federationName, ObjectInstanceHandle federationObjectInstance);
    // return AttributeHandleValueMap fully prepared for sending attribute update, with all attributes given in parameter
    AttributeHandleValueMap GetAttributeValues(AttributeHandleSet attributes);
    void SetAttributeValues(const AttributeHandleValueMap& attributes);
    // return all attributes for which a response is currently implemented
    AttributeHandleSet GetKnownAttributes() const;
    // return the internal object instance representing this federation
    ObjectInstanceHandle GetObjectInstance() const { return mFederationObjectInstance; }
    // .. and its class
    ObjectClassHandle GetObjectClass() const { return mFederationClassHandle; }

    // add/remove a federate
    void AddFederate(std::shared_ptr<MomFederate> federate);
    void RemoveFederate(std::shared_ptr<MomFederate> federate);
    std::shared_ptr<MomFederate> GetFederate(ObjectInstanceHandle federateObjectInstance) const;
    std::shared_ptr<MomFederate> GetFederate(FederateHandle federateHandle) const;

    // attribute getters/setters
    std::wstring GetFederationName() const { return mFederationName.get(); }
    void SetFederationName(const std::wstring& value) { mFederationName.set(value); }

    std::vector<FederateHandle> GetFederatesInFederation() const;
    void SetFederatesInFederation(const std::vector<FederateHandle>& value);

    std::wstring GetRTIversion() const { return mRTIversion.get(); }
    void SetRTIversion(const std::wstring& value) { mRTIversion.set(value); }

    std::wstring GetMIMdesignator() const { return mMIMdesignator.get(); }
    void SetMIMdesignator(const std::wstring& value) { mMIMdesignator.set(value); }

    std::vector<std::wstring> GetFOMmoduleDesignatorList() const;
    void SetFOMmoduleDesignatorList(const std::vector<std::wstring>& fomDesignatorList);

    std::wstring GetCurrentFDD() const { return mCurrentFDD.get(); }
    void SetCurrentFDD(const std::wstring& value) { mCurrentFDD.set(value); }

  private:
    std::map<AttributeHandle, std::function<VariableLengthData()>> mAttributeGetters;
    std::map<AttributeHandle, std::function<void(const VariableLengthData&)>> mAttributeSetters;

    ObjectClassHandle mFederationClassHandle;
    ObjectInstanceHandle mFederationObjectInstance;

    HLAunicodeString mFederationName;
    HLAvariableArray mFederatesInFederation;
    HLAunicodeString mRTIversion;
    HLAunicodeString mMIMdesignator;
    HLAvariableArray mFOMmoduleDesignatorList;
    HLAunicodeString mCurrentFDD;
    std::set<std::shared_ptr<MomFederate>> mFederates;
    // map of (global) federate handles to MomFederates
    std::map<FederateHandle, std::shared_ptr<MomFederate>> mFederatesByHandle;
    // map of (global, discovered or own) federate objects to MomFederates
    std::map<ObjectInstanceHandle, std::shared_ptr<MomFederate>> mFederatesByObject;
};

}