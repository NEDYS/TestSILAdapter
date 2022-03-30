
#pragma once

#include <list>
#include <map>
#include <functional>

#include "MomTypes.h"
#include "MomServer.h"
#include "encoding/BasicDataElements.h"
#include "encoding/HLAhandle.h"
#include "encoding/HLAvariableArray.h"
#include "MomFederateMetrics.h"

namespace OpenRTI {

class MomManager;

class OPENRTI_LOCAL MomFederate : public std::enable_shared_from_this<MomFederate>
{
  public:
    MomFederate(std::shared_ptr<MomManager> mom, FederateHandle federateHandle, ObjectInstanceHandle federateObjectInstance);
    ~MomFederate() noexcept;
    // return AttributeHandleValueMap fully prepared for sending attribute update, with all attributes given in parameter
    AttributeHandleValueMap GetAttributeValues(AttributeHandleSet attributes);
    void SetAttributeValues(const AttributeHandleValueMap& attributes);
    // return all attributes for which a response is currently implemented
    AttributeHandleSet GetKnownAttributes() const;

    // These public methods get/set the actual 'native' values of the respective attributes.
    FederateHandle GetFederateHandle() const { return mFederateHandle.getFederateHandle(); }
    void SetFederateHandle(FederateHandle handle) { mFederateHandle.set(handle); }
    std::wstring GetFederateName() const { return mFederateName.get(); }
    void SetFederateName(const std::wstring& value) { mFederateName.set(value); }
    std::wstring GetFederateType() const { return mFederateType.get(); }
    void SetFederateType(const std::wstring& val) { mFederateType.set(val); }
    std::wstring GetFederateHost() const { return mFederateName.get(); }
    void SetFederateHost(const std::wstring& value) { mFederateHost.set(value); }
    std::wstring GetFederateRTIversion() const { return mFederateRTIversion.get(); }
    void SetFederateRTIversion(const std::wstring& val) { mFederateRTIversion.set(val); }
    std::vector<std::wstring> GetFOMmoduleDesignatorList() const;
    void SetFOMmoduleDesignatorList(const std::vector<std::wstring>& fomDesignatorList);
    bool GetTimeConstrained() const { return mTimeConstrained.get(); }
    void SetTimeConstrained(bool value) { mTimeConstrained.set(value); }
    bool GetTimeRegulating() const { return mTimeRegulating.get(); }
    void SetTimeRegulating(bool value) { mTimeRegulating.set(value); }
    bool GetAsynchronousDelivery() const { return mAsynchronousDelivery.get(); }
    void SetAsynchronousDelivery(bool val) { mAsynchronousDelivery.set(val); }

    // basic statistics data, collected by MomFederateMetrics
    Integer32 GetReflectionsReceived() const { return mReflectionsReceived.get(); }
    void SetReflectionsReceived(Integer32 value) { mReflectionsReceived.set(value); }
    Integer32 GetUpdatesSent() const { return mUpdatesSent.get(); }
    void SetUpdatesSent(Integer32 value) { mUpdatesSent.set(value); }
    Integer32 GetInteractionsReceived() const { return mInteractionsReceived.get(); }
    void SetInteractionsReceived(Integer32 val) { mInteractionsReceived.set(val); }
    Integer32 GetInteractionsSent() const { return mInteractionsSent.get(); }
    void SetInteractionsSent(Integer32 val) { mInteractionsSent.set(val); }


    FederateHandle GetInternalFederateHandle() const { return mInternalFederateHandle; }
    ObjectClassHandle GetInternalFederateObjectClass() const { return mFederateClassHandle; }
    ObjectInstanceHandle GetInternalFederateObjectInstance() const { return mFederateObjectInstance; }
    VariableLengthData GetEncodedFederateHandle() const;
    void insertFOMModule(const FOMModule& fomModule);
  private:
    std::map<AttributeHandle, std::function<VariableLengthData()>> mAttributeGetters;
    std::map<AttributeHandle, std::function<void(const VariableLengthData&)>> mAttributeSetters;

    FederateHandle mInternalFederateHandle;
    ObjectClassHandle mFederateClassHandle;
    ObjectInstanceHandle mFederateObjectInstance;

    HLAhandle mFederateHandle;
    HLAunicodeString mFederateName;
    HLAunicodeString mFederateType;
    HLAunicodeString mFederateHost;
    HLAunicodeString mFederateRTIversion;
    HLAboolean mTimeConstrained;
    HLAboolean mTimeRegulating;
    HLAboolean mAsynchronousDelivery;
    HLAinteger32BE mReflectionsReceived;
    HLAinteger32BE mUpdatesSent;
    HLAinteger32BE mInteractionsReceived;
    HLAinteger32BE mInteractionsSent;
    HLAinteger32BE mObjectInstancesThatCanBeDeleted;
    HLAinteger32BE mObjectInstancesUpdated;
    HLAinteger32BE mObjectInstancesReflected;
    HLAinteger32BE mObjectInstancesDeleted;
    HLAinteger32BE mObjectInstancesRemoved;
    HLAinteger32BE mObjectInstancesRegistered;
    HLAinteger32BE mObjectInstancesDiscovered;

    HLAinteger32BE mFederateTimeGrantedTime;
    HLAinteger32BE mFederateTimeAdvancingTime;

    HLAinteger32BE mNumberOfClasses;
    HLAvariableArray mFOMmoduleDesignatorList;

    // still missing:
    //.attribute(L"HLAFOMmoduleDesignatorList", L"HLAmoduleDesignatorList")
    //.attribute(L"HLAfederateState", L"HLAfederateState")
    //.attribute(L"HLAtimeManagerState", L"HLAtimeState")
    //.attribute(L"HLAlogicalTime", L"HLAlogicalTime")
    //.attribute(L"HLAlookahead", L"HLAtimeInterval")
    //.attribute(L"HLAGALT", L"HLAlogicalTime")
    //.attribute(L"HLALITS", L"HLAlogicalTime")
    //.attribute(L"HLAROlength", L"HLAcount")
    //.attribute(L"HLATSOlength", L"HLAcount")
    //.attribute(L"HLAconveyRegionDesignatorSets", L"HLAswitch")
    //.attribute(L"HLAconveyProducingFederate", L"HLAswitch")
};

}