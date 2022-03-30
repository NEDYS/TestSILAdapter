
#include "DebugNew.h"
#include "StringUtils.h"
#include "RTI/encoding/BasicDataElements.h"
#include "RTI/encoding/HLAhandle.h"
#include "Encoding.h"
#include "HandleImplementation.h"
#include "VariableLengthDataImplementation.h"
#include "encoding/HLAhandle.h"
#include "StringUtils.h"

#include <cassert>

namespace rti1516e {

class HLAhandle::Implementation : public OpenRTI::HLAhandle
{
  public:
    Implementation(const OpenRTI::HLAhandle& ref) : OpenRTI::HLAhandle(ref) {}
    Implementation() : OpenRTI::HLAhandle() {}
};

HLAhandle::HLAhandle() : mImpl()
{
  mImpl = std::unique_ptr<HLAhandle::Implementation>(new HLAhandle::Implementation());
}

HLAhandle::HLAhandle(const HLAhandle& ref) : mImpl()
{
  mImpl = std::unique_ptr<HLAhandle::Implementation>(new HLAhandle::Implementation(*ref.mImpl.get()));
}

HLAhandle::HLAhandle(const FederateHandle& handle) : mImpl(std::unique_ptr<HLAhandle::Implementation>(new HLAhandle::Implementation()))
{
  set(handle);
}

HLAhandle::HLAhandle(const ObjectClassHandle& handle) : mImpl(std::unique_ptr<HLAhandle::Implementation>(new HLAhandle::Implementation()))
{
  set(handle);
}

HLAhandle::HLAhandle(const ObjectInstanceHandle& handle) : mImpl(std::unique_ptr<HLAhandle::Implementation>(new HLAhandle::Implementation()))
{
  set(handle);
}

HLAhandle::HLAhandle(const AttributeHandle& handle) : mImpl(std::unique_ptr<HLAhandle::Implementation>(new HLAhandle::Implementation()))
{
  set(handle);
}


HLAhandle::HLAhandle(const InteractionClassHandle& handle) : mImpl(std::unique_ptr<HLAhandle::Implementation>(new HLAhandle::Implementation()))
{
  set(handle);
}

HLAhandle::HLAhandle(const ParameterHandle& handle) : mImpl(std::unique_ptr<HLAhandle::Implementation>(new HLAhandle::Implementation()))
{
  set(handle);
}

HLAhandle::~HLAhandle() noexcept
{
}

HLAhandle& HLAhandle::operator=(const HLAhandle& ref)
{
  mImpl = std::unique_ptr<HLAhandle::Implementation>(new HLAhandle::Implementation(*ref.mImpl.get()));
  return *this;
}

FederateHandle HLAhandle::getFederateHandle() const
{
  FederateHandle rti1516eHandle;
  FederateHandleFriend::copy(rti1516eHandle,  mImpl->getFederateHandle());
  return rti1516eHandle;
}

ObjectClassHandle HLAhandle::getObjectClassHandle() const
{
  ObjectClassHandle rti1516eHandle;
  ObjectClassHandleFriend::copy(rti1516eHandle,  mImpl->getObjectClassHandle());
  return rti1516eHandle;
}

ObjectInstanceHandle HLAhandle::getObjectInstanceHandle() const
{
  ObjectInstanceHandle rti1516eHandle;
  ObjectInstanceHandleFriend::copy(rti1516eHandle,  mImpl->getObjectInstanceHandle());
  return rti1516eHandle;
}

AttributeHandle HLAhandle::getAttributeHandle() const
{
  AttributeHandle rti1516eHandle;
  AttributeHandleFriend::copy(rti1516eHandle,  mImpl->getAttributeHandle());
  return rti1516eHandle;
}

InteractionClassHandle HLAhandle::getInteractionClassHandle() const
{
  InteractionClassHandle rti1516eHandle;
  InteractionClassHandleFriend::copy(rti1516eHandle,  mImpl->getInteractionClassHandle());
  return rti1516eHandle;
}

ParameterHandle HLAhandle::getParameterHandle() const
{
  ParameterHandle rti1516eHandle;
  ParameterHandleFriend::copy(rti1516eHandle,  mImpl->getParameterHandle());
  return rti1516eHandle;
}

void HLAhandle::set(FederateHandle handle)
{
  OpenRTI::FederateHandle nativeHandle;
  FederateHandleFriend::copy(nativeHandle, handle);
  mImpl->set(nativeHandle);
}

void HLAhandle::set(ObjectClassHandle handle)
{
  OpenRTI::ObjectClassHandle nativeHandle;
  ObjectClassHandleFriend::copy(nativeHandle, handle);
  mImpl->set(nativeHandle);
}

void HLAhandle::set(ObjectInstanceHandle handle)
{
  OpenRTI::ObjectInstanceHandle nativeHandle;
  ObjectInstanceHandleFriend::copy(nativeHandle, handle);
  mImpl->set(nativeHandle);
}

void HLAhandle::set(AttributeHandle handle)
{
  OpenRTI::AttributeHandle nativeHandle;
  AttributeHandleFriend::copy(nativeHandle, handle);
  mImpl->set(nativeHandle);
}

void HLAhandle::set(InteractionClassHandle handle)
{
  OpenRTI::InteractionClassHandle nativeHandle;
  InteractionClassHandleFriend::copy(nativeHandle, handle);
  mImpl->set(nativeHandle);
}

void HLAhandle::set(ParameterHandle handle)
{
  OpenRTI::ParameterHandle nativeHandle;
  ParameterHandleFriend::copy(nativeHandle, handle);
  mImpl->set(nativeHandle);
}

VariableLengthData HLAhandle::encode() const
{
  VariableLengthData result;
  encode(result);
  return result;
}

void HLAhandle::encode(VariableLengthData& outData) const
{
  OpenRTI::VariableLengthData data;
  mImpl->encode(data);
  outData = VariableLengthDataFriend::create(data);
}

void HLAhandle::encodeInto(std::vector<Octet>& buffer) const
{
  mImpl->encodeInto(buffer);
}

size_t HLAhandle::encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return mImpl->encodeInto(buffer, bufferSize, offset);
}

void HLAhandle::decode(VariableLengthData const& inData)
{
  try
  {
    mImpl->decode(OpenRTI::VariableLengthData(rti1516e::VariableLengthDataFriend::readPointer(inData)));
  }
  catch (const OpenRTI::EncoderException& e)
  {
    throw rti1516e::EncoderException(OpenRTI::utf8ToUcs(e.what()));
  }
}

size_t HLAhandle::decodeFrom(std::vector<Octet> const& buffer, size_t index)
{
  try
  {
    return mImpl->decodeFrom(buffer, index);
  }
  catch (const OpenRTI::EncoderException& e)
  {
    throw rti1516e::EncoderException(OpenRTI::utf8ToUcs(e.what()));
  }
}

size_t HLAhandle::decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  try
  {
    return mImpl->decodeFrom(buffer, bufferSize, index);
  }
  catch (const OpenRTI::EncoderException& e)
  {
    throw rti1516e::EncoderException(OpenRTI::utf8ToUcs(e.what()));
  }
}

size_t HLAhandle::getEncodedLength() const
{
  return mImpl->getEncodedLength();
}

unsigned int HLAhandle::getOctetBoundary() const
{
  return mImpl->getOctetBoundary();
}

}