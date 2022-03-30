/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#include "TextLogService.h"

namespace NDistSimIB {

// =========================================
// Creates an instance of the Service
std::unique_ptr<RemoteAmbassador::AbstractTextLogService> CreateTextLogService(RemoteAmbassador& remoteAmb)
{
  return std::unique_ptr<RemoteAmbassador::AbstractTextLogService>(new TextLogService(remoteAmb));
}

TextLogService::TextLogService(RemoteAmbassador& remoteAmb) :
  mRemoteAmbassador(remoteAmb),
  mNextReceiveCallbackHandle(1)
{

}

TextLogService::~TextLogService()
{
  DebugPrintf(SDebug,"~TextLogService\n");
}

//=========================================
// Methods called by RemoteAbmassador
void TextLogService::OnJoined()
{
  InitializeHandles();
  PublishSubscribe();
}

void TextLogService::Reset()
{
  mReceiveCallbacks.clear();
  mNextReceiveCallbackHandle = 1;
}

bool TextLogService::ProcessBuffers()
{
  return false;
}

//=========================================
// InteractionSubscriber implementation

// Signature with time
void TextLogService::receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& theParameterValues, rti::VariableLengthData const& theUserSuppliedTag, rti::LogicalTime const& theTime, rti::SupplementalReceiveInfo theReceiveInfo)
{
  if (theInteraction == mTextLogIAHandle)
  {
    HandleTextLogInteraction(theParameterValues, theTime, mRemoteAmbassador.mFederateHandle == theReceiveInfo.producingFederate);
  }
}
// Signature without time
void TextLogService::receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& theParameterValues, rti::VariableLengthData const& theUserSuppliedTag, rti::SupplementalReceiveInfo theReceiveInfo)
{
  rti::HLAinteger64Time theTime(0); // dummy time
  receiveInteraction(theInteraction, theParameterValues, theUserSuppliedTag, theTime, theReceiveInfo);
}


// =========================================
// DO internal methods

void TextLogService::InitializeHandles()
{
  // TransmitData Interaction
  mTextLogIAHandle = mRemoteAmbassador.mRTIAmb->getInteractionClassHandle(FOMInteractions::kTextLog);

  mTextLogIATextParameter = mRemoteAmbassador.getParameterHandle(mTextLogIAHandle, FOMInteractions::kTextLog_Param_Text);
  mTextLogIASenderParameter = mRemoteAmbassador.getParameterHandle(mTextLogIAHandle, FOMInteractions::kTextLog_Param_Sender);
}

void TextLogService::PublishSubscribe()
{
  // Interactions
  mRemoteAmbassador.subscribeInteractionClass(mTextLogIAHandle, this);
  //mRemoteAmbassador.mRTIAmb->setInteractionClassDeliverToSelf(mTextLogIAHandle, true);
  mRemoteAmbassador.mRTIAmb->publishInteractionClass(mTextLogIAHandle);
}

void TextLogService::HandleTextLogInteraction(const rti::ParameterHandleValueMap& theParameterValues, const rti::LogicalTime& theTime, bool sentBySelf)
{
  std::string msg_s = IBDefaults::TextLog::Text;
  std::string sender_s = IBDefaults::TextLog::Sender;

  rti::VariableLengthData msg_vld;
  if (mTextLogIATextParameter.isValid() && // isValid for FOM merge CTest
      mRemoteAmbassador.ReadParameter(/*out*/ msg_vld, mTextLogIAHandle, theParameterValues, mTextLogIATextParameter))
  {
    rti::HLAunicodeString msg;
    msg.decode(msg_vld);
    msg_s = wstring2string(msg.get());
  }

  rti::VariableLengthData sender_vld;
  if (mRemoteAmbassador.ReadParameter(/*out*/ sender_vld, mTextLogIAHandle, theParameterValues, mTextLogIASenderParameter))
  {
    rti::HLAunicodeString sender;
    sender.decode(sender_vld);
    sender_s = wstring2string(sender.get());
  }

  for (auto callback : mReceiveCallbacks)
  {
    callback.second(msg_s, sender_s);
  }

  //DebugPrintf(SDebug, "%s: Received msg '%s' from '%s' \n", __FUNCTION__, msg_s.c_str(), sender_s.c_str());
}

// =========================================
// DO public API methods

// Targets
void TextLogService::LogText(const std::string& text)
{
  if (mRemoteAmbassador.GetState() != RemoteAmbassador::AmbassadorState::Online)
    return;

  rti::ParameterHandleValueMap parameters;
  std::wstring textws = string2wstring(text);
  std::wstring senderws = string2wstring(mRemoteAmbassador.mCurrentFederateName);
  if (mTextLogIATextParameter.isValid())
    parameters[mTextLogIATextParameter] = rti::HLAunicodeString(textws.c_str()).encode();
  if (mTextLogIASenderParameter.isValid())
    parameters[mTextLogIASenderParameter] = rti::HLAunicodeString(senderws.c_str()).encode();

  mRemoteAmbassador.mRTIAmb->sendInteraction(mTextLogIAHandle, parameters, rti::VariableLengthData());
}

NDistSimIB::ITextLogService::CallbackHandle TextLogService::RegisterReceiveCallback(TextLogCallback callback)
{
  mReceiveCallbacks[mNextReceiveCallbackHandle] = callback;
  return mNextReceiveCallbackHandle++;
}

void TextLogService::UnRegisterReceiveCallback(CallbackHandle handle)
{
  if (mReceiveCallbacks.find(handle) != mReceiveCallbacks.end())
  {
    mReceiveCallbacks.erase(handle);
  }
  else
  {
    Error("Could not unregister the TextLog receive callback: Handle '%d' does not exist.\n");
  }
}

} // namespace NDistSimIB