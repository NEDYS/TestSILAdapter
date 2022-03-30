/* -*-c++-*- OpenRTI - Copyright (C) 2009-2015 Mathias Froehlich
 *
 * This file is part of OpenRTI.
 *
 * OpenRTI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * OpenRTI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenRTI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "DebugNew.h"
#include "FDD1516EFileReader.h"

#include "DefaultErrorHandler.h"
#include "Exception.h"
#include "ExpatXMLReader.h"
#include "FOMStringModuleBuilder.h"
#include "Message.h"
#include "StringUtils.h"
#include "TranslateTypes.h"

#include <functional>
#include <iostream>

namespace OpenRTI {

struct XmlSchemaNodeBase : std::enable_shared_from_this<XmlSchemaNodeBase>
{
  XmlSchemaNodeBase(const std::string& name, bool recurse, bool ignoreChildren)
    : _name(name), _recurse(recurse), _ignoreChildren(ignoreChildren)
  {
  }
  XmlSchemaNodeBase(const XmlSchemaNodeBase& rhs) : _name(rhs._name), _recurse(rhs._recurse), _ignoreChildren(rhs._ignoreChildren)
  {
  }
  virtual ~XmlSchemaNodeBase() {}
  const std::string getName() const
  {
    return _name;
  }
  const std::list<std::shared_ptr<XmlSchemaNodeBase>>& getChildren() const noexcept
  {
    return _children;
  }
  void addChild(std::shared_ptr<XmlSchemaNodeBase> node)
  {
    if (node->_parent == nullptr)
    {
      node->_parent = shared_from_this();
      _children.push_back(node);
    }
    else
    {
      throw RTIinternalError(node->getName() + " already has a parent");
    }
  }
  // note: may return 'this' if _recurse is true and 'name' equals the name of this node
  std::shared_ptr<XmlSchemaNodeBase> getChild(const std::string& name)
  {
    for (auto child : _children)
    {
      if (child->getName() == name)
        return child;
    }
    if (_recurse && name == getName())
    {
      return shared_from_this();
    }
    return std::shared_ptr<XmlSchemaNodeBase>();
  }
  std::string getPath() const
  {
    if (_parent != nullptr)
    {
      return _parent->getPath() + "/" + _name;
    }
    else
    {
      return "/" + _name;
    }
  }

  bool ignoreChildren() const { return _ignoreChildren; }
  std::shared_ptr<XmlSchemaNodeBase> _parent;
  std::string _name;
  bool _recurse;
  bool _ignoreChildren;
  std::list<std::shared_ptr<XmlSchemaNodeBase>> _children;
};

struct XmlSchemaNode : public XmlSchemaNodeBase
{
  using NodeCallbackType = std::function<void()>;
  XmlSchemaNode(const std::string& name, bool recurse, bool ignoreChildren, NodeCallbackType startCallback = nullptr, NodeCallbackType endCallback = nullptr)
    : XmlSchemaNodeBase(name, recurse, ignoreChildren)
    , _startCallback(startCallback)
    , _endCallback(endCallback)
  {
  }
  XmlSchemaNode(const XmlSchemaNode& rhs) 
    : XmlSchemaNodeBase(rhs._name, rhs._recurse, rhs._ignoreChildren)
    , _startCallback(rhs._startCallback)
    , _endCallback(rhs._endCallback)
  {
  }
  NodeCallbackType _startCallback;
  NodeCallbackType _endCallback;
};

struct XmlSchemaLeafNode : public XmlSchemaNodeBase
{
  using LeafCallbackType = std::function<void(const std::string&)>;

  XmlSchemaLeafNode(const std::string& name, LeafCallbackType callback = nullptr)
    : XmlSchemaNodeBase(name, false, false)
    , _callback(callback)
  {
  }
  XmlSchemaLeafNode(const XmlSchemaLeafNode& rhs)
    : XmlSchemaNodeBase(rhs._name, rhs._recurse, rhs._ignoreChildren)
    , _callback(rhs._callback)
  {
  }
  LeafCallbackType _callback;
};

struct XmlSchemaTreeBuilder
{
  XmlSchemaTreeBuilder& node(std::string name, XmlSchemaNode::NodeCallbackType startCallback = nullptr)
  {
    if (name[name.size()-1] == '*')
    {
      addAndPush(std::make_shared<XmlSchemaNode>(name.substr(0, name.size() - 1), true, false, startCallback));
    }
    else
    {
      addAndPush(std::make_shared<XmlSchemaNode>(name, false, false, startCallback));
    }
    return *this;
  }
  XmlSchemaTreeBuilder& ignore_node(std::string name)
  {
    addAndPush(std::make_shared<XmlSchemaNode>(name, false, true));
    return *this;
  }
  XmlSchemaTreeBuilder& leaf(std::string name, XmlSchemaLeafNode::LeafCallbackType callback = nullptr)
  {
    add(std::make_shared<XmlSchemaLeafNode>(name, callback));
    return *this;
  }
  void addAndPush(std::shared_ptr<XmlSchemaNodeBase> newNode)
  {
    if (!mContextStack.empty())
    {
      auto currentContext = mContextStack.front();
      currentContext->addChild(newNode);
    }
    mContextStack.push_front(newNode);
  }
  void add(std::shared_ptr<XmlSchemaNodeBase> newNode)
  {
    if (!mContextStack.empty())
    {
      auto currentContext = mContextStack.front();
      currentContext->addChild(newNode);
    }
    else
    {
      throw new RTIinternalError("no context to add to");
    }
  }
  XmlSchemaTreeBuilder& end(XmlSchemaNode::NodeCallbackType endCallback = nullptr)
  {
    assert(mContextStack.size() >= 1);
    auto currentContextNode = std::dynamic_pointer_cast<XmlSchemaNode>(mContextStack.front());
    if (mContextStack.size() == 1)
    {
      _root = currentContextNode;
      assert(_root != nullptr);
    }
    if (currentContextNode != nullptr)
    {
      currentContextNode->_endCallback = endCallback;
    }
    mContextStack.pop_front();
    return *this;
  }

  std::list<std::shared_ptr<XmlSchemaNodeBase>> mContextStack;
  std::shared_ptr<XmlSchemaNode> _root;
};

class OPENRTI_LOCAL FDD1516EContentHandler final : public XML::ContentHandler {
public:
  FDD1516EContentHandler() noexcept;
  virtual ~FDD1516EContentHandler() noexcept;



  void startDocument() override;
  void endDocument() override;
  void startElement(const char* uri, const char* name,
                            const char* qName, const XML::Attributes* atts) override;
  void endElement(const char* uri, const char* name, const char* qName) override;

  void characters(const char* data, unsigned length) override;

  // Character data content. Is only accumulated in the leaf elements.
  std::string _characterData;

  // Helper to build up the data
  FOMStringModuleBuilder _fomStringModuleBuilder;

  std::shared_ptr<XmlSchemaNode> _rootNode;
  std::list<std::shared_ptr<XmlSchemaNodeBase>> _contextStack;
};

FDD1516EContentHandler::FDD1516EContentHandler() noexcept
{
  XmlSchemaTreeBuilder b;
  b.node("objectModel")
    .ignore_node("modelIdentification")
    .end() // modelIdentification
    .node("objects")
      .node("objectClass*", [this]() { _fomStringModuleBuilder.pushObjectClass(); })
        .leaf("name", [this](const std::string& name) { _fomStringModuleBuilder.getCurrentObjectClass().getName().push_back(name); })
        .leaf("sharing")
        .leaf("semantics")
        .node("attribute", [this]() { _fomStringModuleBuilder.addAttribute(); })
          .leaf("name", [this](const std::string& name) { _fomStringModuleBuilder.getCurrentObjectClassAttribute().setName(name); })
          .leaf("order", [this](const std::string& s) { _fomStringModuleBuilder.getCurrentObjectClassAttribute().setOrderType(s); })
          .leaf("transportation", [this](const std::string& name) { _fomStringModuleBuilder.getCurrentObjectClassAttribute().setTransportationType(name); })
          .leaf("sharing")
          .leaf("ownership")
          .leaf("dataType", [this](const std::string& s) { _fomStringModuleBuilder.getCurrentObjectClassAttribute().setDataType(s); })
          .leaf("updateType")
          .leaf("updateCondition")
          .leaf("semantics")
          .node("dimensions")
            .leaf("dimension", [this](const std::string& s) { _fomStringModuleBuilder.addAttributeDimension(s); })
          .end()
        .end() // attribute
      .end([this]() { _fomStringModuleBuilder.popObjectClass(); }) // objectClass*
    .end() // objects
    .node("interactions")
      .node("interactionClass*", [this]() { _fomStringModuleBuilder.pushInteractionClass(); })
        .leaf("name", [this](const std::string& name) { _fomStringModuleBuilder.getCurrentInteractionClass().getName().push_back(name); })
        .leaf("order", [this](const std::string& s) { _fomStringModuleBuilder.getCurrentInteractionClass().setOrderType(s); })
        .leaf("sharing")
        .leaf("transportation", [this](const std::string& name) { _fomStringModuleBuilder.getCurrentInteractionClass().setTransportationType(name); })
        .leaf("semantics")
        .node("dimensions")
          .leaf("dimension", [this](const std::string& s) { _fomStringModuleBuilder.addInteractionDimension(s); })
        .end()
        .node("parameter", [this]() { _fomStringModuleBuilder.addParameter(); })
          .leaf("name", [this](const std::string& name) { _fomStringModuleBuilder.getCurrentInteractionClassParameter().setName(name); })
          .leaf("dataType", [this](const std::string& s) { _fomStringModuleBuilder.getCurrentInteractionClassParameter().setDataType(s); })
          .leaf("semantics")
        .end([]() {  }) // parameter
      .end([this]() { _fomStringModuleBuilder.popInteractionClass(); }) // interactionClass
    .end() // interactions
    .node("dimensions")
      .node("dimension", [this]() { _fomStringModuleBuilder.addDimension();})
        .leaf("name", [this](const std::string& name) { _fomStringModuleBuilder.getCurrentDimension().setName(name); })
        .leaf("dataType")
        .leaf("normalization")
        .leaf("value")
        .leaf("upperBound")
      .end()
    .end()
    .node("transportations")
      .node("transportation", [this]() { _fomStringModuleBuilder.addTransportationType();})
        .leaf("name", [this](const std::string& name) {_fomStringModuleBuilder.getCurrentTransportationType().setName(name); })
        .leaf("reliable")
        .leaf("semantics")
      .end()
    .end()
    .node("updateRates")
      .node("updateRate", [this]() { _fomStringModuleBuilder.addUpdateRate();})
        .leaf("name", [this](const std::string& name) {_fomStringModuleBuilder.getCurrentUpdateRate().setName(name); })
        .leaf("rate", [this](const std::string& rate) {_fomStringModuleBuilder.getCurrentUpdateRate().setRate(from_string<double>(rate)); })
      .end()
    .end()
    .node("dataTypes")
      .node("basicDataRepresentations")
        .node("basicData", [this]() { _fomStringModuleBuilder.addBasicDataType();})
          .leaf("name", [this](const std::string& name) {_fomStringModuleBuilder.getCurrentBasicDataType().setName(name); })
          .leaf("size", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentBasicDataType().setSize(from_string<uint32_t>(s)); })
          .leaf("interpretation")
          .leaf("endian", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentBasicDataType().setEndian(from_string<Endianness>(s)); })
          .leaf("encoding")
          .leaf("nocode")
        .end() // basicData
      .end() // basicDataRepresentations
      .node("simpleDataTypes")
        .node("simpleData", [this]() { _fomStringModuleBuilder.addSimpleDataType();})
          .leaf("name", [this](const std::string& name) {_fomStringModuleBuilder.getCurrentSimpleDataType().setName(name); })
          .leaf("representation", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentSimpleDataType().setRepresentation(s); })
          .leaf("units")
          .leaf("resolution")
          .leaf("accuracy")
          .leaf("semantics")
          .leaf("nocode")
        .end() // simpleData
      .end() // simpleDataTypes
      .node("enumeratedDataTypes")
        .node("enumeratedData", [this]() { _fomStringModuleBuilder.addEnumeratedDataType();})
          .leaf("name", [this](const std::string& name) {_fomStringModuleBuilder.getCurrentEnumeratedDataType().setName(name); })
          .leaf("representation", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentEnumeratedDataType().setRepresentation(s); })
          .leaf("semantics")
          .leaf("nocode")
          .node("enumerator", [this]() { _fomStringModuleBuilder.getCurrentEnumeratedDataType().getEnumerators().push_back(FOMStringEnumerator()); })
            .leaf("name", [this](const std::string& name) {_fomStringModuleBuilder.getCurrentEnumeratedDataType().getEnumerators().back().setName(name); })
            .leaf("value", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentEnumeratedDataType().getEnumerators().back().setValue(from_string<uint32_t>(s)); })
          .end() // enumerator
        .end() // enumeratedData
      .end() // enumeratedDataTypes
      .node("arrayDataTypes")
        .node("arrayData", [this]() { _fomStringModuleBuilder.addArrayDataType();})
          .leaf("name", [this](const std::string& name) {_fomStringModuleBuilder.getCurrentArrayDataType().setName(name); })
          .leaf("dataType", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentArrayDataType().setDataType(s); })
          .leaf("cardinality", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentArrayDataType().setCardinality(s); })
          .leaf("encoding", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentArrayDataType().setEncoding(from_string<ArrayDataTypeEncoding>(s)); })
          .leaf("semantics")
          .leaf("nocode")
        .end() // arrayData
      .end() // arrayDataTypes
      .node("fixedRecordDataTypes")
        .node("fixedRecordData", [this]() { _fomStringModuleBuilder.addFixedRecordDataType();})
          .leaf("name", [this](const std::string& name) {_fomStringModuleBuilder.getCurrentFixedRecordDataType().setName(name); })
          .leaf("include", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentFixedRecordDataType().setInclude(s); })
          .leaf("version", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentFixedRecordDataType().setVersion(from_string<uint32_t>(s)); })
          .leaf("encoding", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentFixedRecordDataType().setEncoding(s); })
          .leaf("semantics")
          .leaf("nocode")
          .node("field", [this]() {_fomStringModuleBuilder.getCurrentFixedRecordDataType().getFields().push_back(FOMStringFixedRecordField2()); })
            .leaf("name", [this](const std::string& name) {_fomStringModuleBuilder.getCurrentFixedRecordDataType().getFields().back().setName(name); })
            .leaf("dataType", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentFixedRecordDataType().getFields().back().setDataType(s); })
            .leaf("version", [this](const std::string& s) {_fomStringModuleBuilder.getCurrentFixedRecordDataType().getFields().back().setVersion(from_string<uint32_t>(s)); })
            .leaf("semantics")
          .end() // field
        .end() // fixedRecordData
      .end() // fixedRecordDataTypes
      .ignore_node("variantRecordDataTypes")
      .end() // fixedRecordData
    .end() // dataTypes
    .ignore_node("notes")
    .end() // notes
  .end(); // objectModel
  _rootNode = b._root;
}

FDD1516EContentHandler::~FDD1516EContentHandler() noexcept
{
}

void
FDD1516EContentHandler::startDocument()
{
}

void
FDD1516EContentHandler::endDocument()
{
  _fomStringModuleBuilder.validate();
}

void
FDD1516EContentHandler::startElement(const char* /*uri*/, const char* name,
                                     const char* /*qName*/, const XML::Attributes* /*atts*/)
{
  if (_contextStack.empty())
  {
    if (name == _rootNode->getName())
    {
      _contextStack.push_front(_rootNode);
      return;
    }
    else
    {
      throw ErrorReadingFDD(std::string("unexpected tag ") + name + " at document root");
    }
  }
  std::shared_ptr<XmlSchemaNodeBase> currentContext = _contextStack.front();
  if (currentContext->ignoreChildren())
  {
    return;
  }
  auto childNode = currentContext->getChild(name);
  if (childNode == nullptr)
  {
    throw ErrorReadingFDD(std::string("unexpected tag ") + name + " at path " + currentContext->getPath());
  }
  _contextStack.push_front(childNode);

  auto childContextNode = std::dynamic_pointer_cast<XmlSchemaNode>(childNode);
  if (childContextNode != nullptr)
  {
    if (childContextNode->_startCallback != nullptr)
    {
      childContextNode->_startCallback();
    }
  }
}

void
FDD1516EContentHandler::endElement(const char* /*uri*/, const char* name, const char* /*qName*/)
{
  auto currentContext = _contextStack.front();
  std::string characters = _characterData;
  _characterData.clear();

  if (name != currentContext->getName())
  {
    if (currentContext->ignoreChildren())
    {
      return;
    }
    throw ErrorReadingFDD(std::string("unexpected end tag ") + name + " at path " + currentContext->getPath());
  }
  auto leafNode = std::dynamic_pointer_cast<XmlSchemaLeafNode>(currentContext);
  if (leafNode != nullptr)
  {
    if (leafNode->_callback != nullptr)
    {
      leafNode->_callback(characters);
    }
  }
  else
  {
    auto currentContextNode = std::dynamic_pointer_cast<XmlSchemaNode>(currentContext);
    if (currentContextNode != nullptr)
    {
      if (currentContextNode->_endCallback != nullptr)
      {
        currentContextNode->_endCallback();
      }
    }
    else
    {
      throw ErrorReadingFDD(std::string("internal error, tag ") + name + " at path " + currentContext->getPath());
    }
  }
  _contextStack.pop_front();
}

void
FDD1516EContentHandler::characters(const char* data, unsigned length)
{
  //_characterData.append(data, length);
  _characterData = std::string(data, length);
}

FOMStringModule2
FDD1516EFileReader::read(std::istream& stream, const std::string& encoding)
{
  // Set up the fdd parser
  SharedPtr<XML::XMLReader> reader;
  reader = MakeShared<XML::ExpatXMLReader>();

  SharedPtr<FDD1516EContentHandler> contentHandler = MakeShared<FDD1516EContentHandler>();
  reader->setContentHandler(contentHandler);
  SharedPtr<DefaultErrorHandler> errorHandler = MakeShared<DefaultErrorHandler>();
  reader->setErrorHandler(errorHandler);

  reader->parse(stream, encoding);

  std::string errorMessage = errorHandler->getMessages();
  if (!errorMessage.empty())
    throw ErrorReadingFDD(errorMessage);

  return contentHandler->_fomStringModuleBuilder.getFOMStringModule();
}

} // namespace OpenRTI
