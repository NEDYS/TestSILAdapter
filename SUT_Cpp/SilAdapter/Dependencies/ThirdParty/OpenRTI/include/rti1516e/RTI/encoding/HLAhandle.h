#pragma once

#include <RTI/encoding/DataElement.h>
#include <RTI/encoding/EncodingConfig.h>
#include <RTI/SpecificConfig.h>
#include "RTI/Handle.h"

namespace rti1516e
{

// This is fake as it currently only supports federate handles.
// It should be reworked to support all kinds of handles
// 1
class RTI_EXPORT HLAhandle : public DataElement
{
  enum HandleKind {
    eFederateHandle,
    eObjectClassHandle,
    eObjectInstanceHandle,
    eInteractionClassHandle,
    eAttributeHandle,
    eParameterHandle,
    eInvalid
  };
public:
  HLAhandle();
  HLAhandle(const HLAhandle& ref);
  HLAhandle(const FederateHandle& handle);
  HLAhandle(const ObjectClassHandle& handle);
  HLAhandle(const ObjectInstanceHandle& handle);
  HLAhandle(const AttributeHandle& handle);
  HLAhandle(const InteractionClassHandle& handle);
  HLAhandle(const ParameterHandle& handle);
  ~HLAhandle() noexcept;
  HLAhandle& operator=(const HLAhandle& ref);

  FederateHandle getFederateHandle() const;
  ObjectClassHandle getObjectClassHandle() const;
  ObjectInstanceHandle getObjectInstanceHandle() const;
  AttributeHandle getAttributeHandle() const;
  InteractionClassHandle getInteractionClassHandle() const;
  ParameterHandle getParameterHandle() const;

  void set(FederateHandle handle);
  void set(ObjectClassHandle handle);
  void set(ObjectInstanceHandle handle);
  void set(AttributeHandle handle);
  void set(InteractionClassHandle handle);
  void set(ParameterHandle handle);

  std::unique_ptr<DataElement> clone() const override
  {
    return std::unique_ptr<DataElement>(new HLAhandle(*this));
  }

  VariableLengthData encode() const override;
  void encode(VariableLengthData& inData) const override;
  void encodeInto(std::vector<Octet>& buffer) const override;

  // Encode this element into a preallocated buffer
  virtual size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const override;

  void decode(VariableLengthData const & inData) override;
  size_t decodeFrom(std::vector<Octet> const & buffer, size_t index) override;
  size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index) override;

  size_t getEncodedLength() const override;
  unsigned int getOctetBoundary() const override;
private:
  class Implementation;
  std::unique_ptr<Implementation> mImpl;
};

}