

#pragma once

//#include "RTIambassadorImplementation.h"
//#include "RTI/encoding/BasicDataElements.h"
//#include "RTI/encoding/HLAhandle.h"
//#include "RTI/encoding/HLAvariableArray.h"

#include <list>
#include <map>
#include <functional>
#include <cassert>
#include "Handle.h"
#include "MomServer.h"
#include "MomTypes.h"

//template<>
//struct std::hash<OpenRTI::InteractionClassHandle>
//{
//  size_t operator()(const OpenRTI::InteractionClassHandle& _Keyval) const
//  {
//    return _Keyval.hash();
//  }
//};

namespace OpenRTI {

class MomFederateMetrics;
class MomInteractionHandler;
struct AbstractFederateMetrics;

class MomFederation;
class MomFederate;

class OPENRTI_LOCAL MomManager : public std::enable_shared_from_this<MomManager>
{
  private:
    typedef std::map<InteractionClassHandle, std::shared_ptr<MomInteractionHandler>> MomInteractionHandlerMap;
    struct MomTreeNode;
    struct MomHandleTree;

  public:

    //----------------------------------------------------------
    //                      CONSTRUCTORS
    //----------------------------------------------------------
    MomManager() noexcept = delete;
    MomManager(MomServer* rtiAmb, FederateHandle ownerFederateHandle);
    ~MomManager() noexcept;
    MomManager& operator=(const MomManager&) = delete;
    MomManager& operator=(MomManager&&) = delete;
    //----------------------------------------------------------
    //                    INSTANCE METHODS
    //----------------------------------------------------------
    /**
     * Builds the internal representation of metadata for standard MOM types.
     */
    void initialize();
    void notifyFederationCreated(const std::string& federationName);
    void notifyFederationJoined(FederateHandle federateHandle, const std::string& federateName, const std::string& federateType);
    void notifyOwnerFederateJoined(FederateHandle federateHandle, const std::string& federateName, const std::string& federateType);
    void notifyFederateResigned(FederateHandle federate);
    void setFOMModuleList(const FOMModuleList& moduleList, const std::string& currentFDD);

    void discoverObjectInstance(ObjectInstanceHandle objectInstance, ObjectClassHandle objectClass, const std::string& name);
    void removeObjectInstance(ObjectInstanceHandle objectInstance);
    bool provideAttributeValueUpdate(ObjectInstanceHandle theObject, AttributeHandleSet const& theAttributes);

    void reflectAttributeValues(
      ObjectInstanceHandle theObject,
      AttributeHandleValueMap const& theAttributeValues,
      VariableLengthData const& theUserSuppliedTag);

    // handles all Mom-related interactions
    bool receiveInteraction(InteractionClassHandle theInteraction, ParameterHandleValueMap const& theParameterValues);

    void publishSubscribeObjects();
    void publishSubscribeInteractions();

    std::shared_ptr<MomFederation> getFederation() const { return mFederation; }
    std::shared_ptr<MomFederate> getFederate() const { return mFederate; }
    std::shared_ptr<MomFederate> getFederate(FederateHandle federateHandle) const;
    std::shared_ptr<MomFederateMetrics> getFederateMetrics() const;
    VariableLengthData getEncodedFederateHandle() const;

    struct AttributesAccessor;
    struct ObjectClassAccessor;

    struct AttributesAccessor
    {
      AttributesAccessor() : mRoot(nullptr) {}
      AttributesAccessor(MomTreeNode* root) : mRoot(root) {}
      AttributesAccessor(const AttributesAccessor& ref)
      {
        mRoot = ref.mRoot;
      }
      AttributesAccessor(AttributesAccessor&& ref)
      {
        mRoot = std::move(ref.mRoot);
      }
      AttributesAccessor& operator=(AttributesAccessor&& ref)
      {
        if (this != &ref)
        {
          mRoot = std::move(ref.mRoot);
        }
        return *this;
      }
      AttributesAccessor& operator=(const AttributesAccessor& ref)
      {
        if (this != &ref)
        {
          mRoot = ref.mRoot;
        }
        return *this;
      }
      AttributesAccessor operator [](const std::string& name)
      {
        if (mRoot != nullptr)
        {
          if (mRoot->getType() == MomType::Attribute)
          {
            if (name == mRoot->getName())
            {
              return *this;
            }
          }
          else
          {
            auto* currentNode = mRoot;
            while (currentNode != nullptr && currentNode->getType() == MomType::Object)
            {
              for (auto* childNode : currentNode->children)
              {
                if (childNode->getType() == MomType::Attribute && childNode->getName() == name)
                {
                  return AttributesAccessor(childNode);
                }
              }
              currentNode = currentNode->getParent();
            }
          }
        }
        return AttributesAccessor();
      }
      operator AttributeHandle() const { return (mRoot != nullptr && mRoot->getType() == MomType::Attribute) ? mRoot->attributeHandle : AttributeHandle(); }
      operator AttributeHandleSet() const
      {
        AttributeHandleSet result;
        MomTreeNode* currentNode = mRoot;
        while (currentNode != nullptr && currentNode->getType() == MomType::Object)
        {
          for (MomTreeNode const* child : currentNode->children)
          {
            if (child->getType() == MomType::Attribute)
            {
              assert(child->attributeHandle.valid());
              result.insert(child->attributeHandle);
            }
          }
          currentNode = currentNode->getParent();
        }
        return result;
      }
      MomTreeNode* mRoot;
    };

    struct ObjectClassAccessor
    {
      ObjectClassAccessor() : mRoot(nullptr) {}
      ObjectClassAccessor(const ObjectClassAccessor& ref)
      {
        mRoot = ref.mRoot;
        attributes.mRoot = mRoot;
      }
      ObjectClassAccessor(ObjectClassAccessor&& ref)
      {
        mRoot = std::move(ref.mRoot);
        attributes.mRoot = mRoot;
      }
      ObjectClassAccessor& operator=(ObjectClassAccessor&& ref)
      {
        if (this != &ref)
        {
          mRoot = std::move(ref.mRoot);
          attributes.mRoot = mRoot;
        }
        return *this;
      }
      ObjectClassAccessor& operator=(const ObjectClassAccessor& ref)
      {
        if (this != &ref)
        {
          mRoot = ref.mRoot;
          attributes.mRoot = mRoot;
        }
        return *this;
      }
      ObjectClassAccessor(MomTreeNode* root) : mRoot(root) { attributes = AttributesAccessor(mRoot); }
      ObjectClassAccessor operator [](const std::string& name)
      {
        if (mRoot != nullptr)
        {
          if (mRoot->getType() == MomType::Object && mRoot->getParent() == nullptr)
          {
            if (name == mRoot->getName())
            {
              return *this;
            }
          }
          for (auto* childNode : mRoot->children)
          {
            if (childNode->getType() == MomType::Object && childNode->getName() == name)
            {
              return ObjectClassAccessor(childNode);
            }
          }
        }
        return ObjectClassAccessor();
      }
      operator ObjectClassHandle() const { return mRoot != nullptr ? mRoot->objectClassHandle : ObjectClassHandle(); }
      AttributesAccessor attributes;
      MomTreeNode* mRoot;
    };

    struct ParametersAccessor
    {
      ParametersAccessor() : mRoot(nullptr) {}
      ParametersAccessor(MomTreeNode* root) : mRoot(root) {}
      ParametersAccessor(const ParametersAccessor& ref)
      {
        mRoot = ref.mRoot;
      }
      ParametersAccessor(ParametersAccessor&& ref)
      {
        mRoot = std::move(ref.mRoot);
      }
      ParametersAccessor& operator=(ParametersAccessor&& ref)
      {
        if (this != &ref)
        {
          mRoot = std::move(ref.mRoot);
        }
        return *this;
      }
      ParametersAccessor& operator=(const ParametersAccessor& ref)
      {
        if (this != &ref)
        {
          mRoot = ref.mRoot;
        }
        return *this;
      }
      ParametersAccessor operator [](const std::string& name)
      {
        if (mRoot != nullptr)
        {
          if (mRoot->getType() == MomType::Parameter)
          {
            if (name == mRoot->getName())
            {
              return *this;
            }
          }
          else
          {
            auto* currentNode = mRoot;
            while (currentNode != nullptr && currentNode->getType() == MomType::Interaction)
            {
              for (auto* childNode : currentNode->children)
              {
                if (childNode->getType() == MomType::Parameter && childNode->getName() == name)
                {
                  return ParametersAccessor(childNode);
                }
              }
              currentNode = currentNode->getParent();
            }
          }
        }
        return ParametersAccessor();
      }
      operator ParameterHandle() const { return mRoot != nullptr ? mRoot->parameterHandle : ParameterHandle(); }
      operator ParameterHandleSet() const
      {
        ParameterHandleSet result;
        const MomTreeNode* currentNode = mRoot;
        while (currentNode != nullptr && currentNode->getType() == MomType::Interaction)
        {
          for (const MomTreeNode* child : currentNode->children)
          {
            if (child->getType() == MomType::Parameter)
            {
              assert(child->parameterHandle.valid());
              result.insert(child->parameterHandle);
            }
          }
          currentNode = currentNode->getParent();
        }
        return result;
      }
      MomTreeNode* mRoot;
    };

    struct InteractionClassAccessor
    {
      InteractionClassAccessor() : mRoot(nullptr) {}
      InteractionClassAccessor(MomTreeNode* root) : mRoot(root) { parameters = ParametersAccessor(mRoot); }
      InteractionClassAccessor(const InteractionClassAccessor& ref)
      {
        mRoot = ref.mRoot;
        parameters.mRoot = mRoot;
      }
      InteractionClassAccessor(InteractionClassAccessor&& ref)
      {
        mRoot = std::move(ref.mRoot);
        parameters = ParametersAccessor(mRoot);
      }
      InteractionClassAccessor& operator=(InteractionClassAccessor&& ref)
      {
        if (this != &ref)
        {
          mRoot = std::move(ref.mRoot);
          parameters = ParametersAccessor(mRoot);
        }
        return *this;
      }
      InteractionClassAccessor& operator=(const InteractionClassAccessor& ref)
      {
        if (this != &ref)
        {
          mRoot = ref.mRoot;
          parameters = ParametersAccessor(mRoot);
        }
        return *this;
      }
      InteractionClassAccessor operator [](const std::string& name)
      {
        if (mRoot != nullptr)
        {
          if (mRoot->getType() == MomType::Interaction && mRoot->getParent() == nullptr)
          {
            if (name == mRoot->getName())
            {
              return *this;
            }
          }
          for (auto* childNode : mRoot->children)
          {
            if (childNode->getName() == name)
            {
              return InteractionClassAccessor(childNode);
            }
          }
        }
        return InteractionClassAccessor();
      }
      operator InteractionClassHandle() const { return mRoot != nullptr ? mRoot->interactionClassHandle : InteractionClassHandle(); }
      ParametersAccessor parameters;
      MomTreeNode* mRoot;
    };

    ObjectClassAccessor objects;
    InteractionClassAccessor interactions;
    std::map<ParameterHandle, std::function<VariableLengthData()>> mParameterGetters;

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////// MOM Hierarchy Generation Methods ////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
  private:
    std::shared_ptr<MomFederation> RegisterFederation(ObjectInstanceHandle federateObject);
    std::shared_ptr<MomFederate> RegisterFederate(FederateHandle federateHandle, ObjectInstanceHandle theObject);

    struct MomTreeNodeBuilder
    {
      std::list<MomTreeNode*> mContextStack;
      MomServer* mMomServer;
      MomTreeNodeBuilder(MomServer* server) : mMomServer(server)
      {
      }
      ~MomTreeNodeBuilder()
      {
        assert(mContextStack.size() == 0);
      }

      /**
       * Adds a node to the context current node.
       * <p/>
       * <b>Note:</b> This method does not affect the context stack.
       *
       * @param newNode the node to add to the current context node
       * @throws IllegalStateException if the context stack is currently empty
       */
      void add(MomTreeNode* newNode);

      /**
       * Adds the specified node to the context current node and pushes it onto the context stack.
       *
       * @param newNode the node to add to the current context node
       */
      void addAndPush(MomTreeNode* newNode);

      /**
       * Creates an object {@link MomTreeNode} for the specified HLA Versions and adds it to the current
       * context node.
       * <p/>
       * <b>Note:</b> The object's name for each HLA version naming scheme will be automatically
       * generated from the <code>name</code> parameter via the simple pattern rule.
       * <p/>
       * The created node is pushed onto the context stack to become the new context node.
       *
       * @param name the name of the object
       * @param supportedVersions the HLA Versions that the object is supported in
       * @return this
       */
      MomTreeNodeBuilder&  object(std::string name);

      /**
       * Creates an attribute {@link MomTreeNode} for the specified HLA Versions and adds it to the
       * current context node.
       *
       * @param name the name of the attribute under each HLA version
       * @param datatype the name of this attribute's datatype
       * @param supportedVersions the HLA Versions that support this attribute
       * @return this
       * @throws IllegalStateException if the current context is not an Object node
       */
      MomTreeNodeBuilder&  attribute(std::string name, std::string datatype);


      /**
       * Creates an interaction {@link MomTreeNode} for the specified HLA Versions and adds it to the
       * current context node.
       * <p/>
       * <b>Note:</b> The interaction's name for each HLA version naming scheme will be automatically
       * generated from the <code>name</code> parameter via the simple pattern rule.
       * <p/>
       * The created node is pushed onto the context stack to become the new context node.
       *
       * @param name the name of the interaction under each HLA version
       * @param supportedVersions the HLA Versions that support this attribute
       * @return this
       */
      MomTreeNodeBuilder&  interaction(std::string name);

      /**
       * Creates a parameter {@link MomTreeNode} for the specified HLA Versions and adds it to the
       * current context node.
       *
       * @param name the name of the parameter under each HLA version
       * @param datatype the name of this parameter's datatype
       * @param supportedVersions the HLA Versions that support this parameter
       * @return this
       * @throws IllegalStateException if the current context is not an Parameter node
       */
      MomTreeNodeBuilder&  parameter(std::string name, std::string datatype);

      /**
       * Marks the end of the current context node, popping it from the stack.
       *
       * @return the previous node on the context stack
       */
      MomTreeNodeBuilder&  end()
      {
        mContextStack.pop_front();
        return *this;
      }

      /**
       * @return the current context node
       */
      MomTreeNode* getContext()
      {
        return mContextStack.front();
      }
    };

    enum MomType
    {
      Object,
      Attribute,
      Interaction,
      Parameter
    };

    struct MomTreeNode
    {
      MomTreeNode* mParent;
      std::list<MomTreeNode*> children;
      std::string mName;
      ObjectClassHandle objectClassHandle;
      AttributeHandle attributeHandle;
      InteractionClassHandle interactionClassHandle;
      ParameterHandle parameterHandle;
      MomType mType;
      std::string mDatatype; // For attributes and parameters, only for 1516+ (all mom types are strings in 1.3)


      MomTreeNode(std::string name,
                  MomType type,
                  ObjectClassHandle handle)
      {
        mName = name;
        mParent = nullptr;
        objectClassHandle = handle;
        mType = type;
        mDatatype = "HLAasciiString";
      }

      MomTreeNode(std::string name,
                  MomType type,
                  AttributeHandle handle,
                  std::string datatype)
      {
        mName = name;
        mParent = nullptr;
        attributeHandle = handle;
        mType = type;
        datatype = "HLAasciiString";
      }

      MomTreeNode(std::string name,
                  MomType type,
                  InteractionClassHandle handle)
      {
        mName = name;
        mParent = nullptr;
        interactionClassHandle = handle;
        mType = type;
      }

      MomTreeNode(std::string name,
                  MomType type,
                  ParameterHandle handle,
                  std::string datatype)
      {
        mName = name;
        mParent = nullptr;
        parameterHandle = handle;
        mType = type;
        mDatatype = datatype;
      }

      ~MomTreeNode() noexcept;
      const MomTreeNode* getParent() const { return mParent; }
      MomTreeNode* getParent() { return mParent; }

      MomType getType() const noexcept
      {
        return mType;
      }

      //std::list<MomTreeNode*>& getChildren()
      //{
      //  return children;
      //}

      const std::list<MomTreeNode*>& getChildren() const noexcept
      {
        return children;
      }

      /**
       * Returns the version-specific name of this node.
       *
       * @param version a HLA specification version
       * @return the name of the node for the specified HLA version
       *
       * @see #getQualifiedName()
       */
      const std::string getName() const
      {
        return mName;
      }

      /**
       * Returns the fully qualified version-specific name of this node.
       *
       * @param version a HLA specification version
       * @return the name of the node for the specified HLA version
       *
       * @see #getName()
       */
      const std::string getQualifiedName() const;

      const std::string& getDatatype() const
      {
        return mDatatype;
      }

      void addChild(MomTreeNode* node);
    };

  private:

    // Our entry point into HLA messaging. Note the MomServer owns us.
    MomServer* _momServer;

    ObjectClassHandle mFederationObjectClass;
    // the object instance corresponding to the federation (HLAfederation).
    // It will be created in a root server, and discovered by all leaf servers who joined the same federation.
    ObjectInstanceHandle mFederationObject;
    std::shared_ptr<MomFederation> mFederation;

    ObjectClassHandle mFederateObjectClass;
    // the federate who originally triggered our initialization. 
    // This is only set in a leaf server!
    FederateHandle _ownerFederateHandle;
    // the object corresponding to the federate who originally created us.
    // This is also only used in a leaf server!
    ObjectInstanceHandle mFederateObject;
    // ... and the corresponding MomFederate
    std::shared_ptr<MomFederate> mFederate;
    // ... and it's data collector.
    std::shared_ptr<MomFederateMetrics> mFederateMetrics;

    // The following two maps will remain required, in case other federates are getting discovered before the
    // federation has been discovered (we need to keep track of the already discovered MomFederates in that case)
    // map of (global) federate handles to MomFederates
    std::map<FederateHandle, std::shared_ptr<MomFederate>> mFederatesByHandle;
    // map of (global, discovered or own) federate objects to MomFederates
    std::map<ObjectInstanceHandle, std::shared_ptr<MomFederate>> mFederatesByObject;
    MomInteractionHandlerMap mMomInteractionHandlers;

};

}