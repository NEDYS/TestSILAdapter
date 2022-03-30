/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/
#pragma once

#include "RemoteAmbassador.h"

namespace NDistSimIB {

class TextLogService : 
  public RemoteAmbassador::AbstractTextLogService
{
public:
  TextLogService(RemoteAmbassador& federate);
  ~TextLogService();

  // =========================================
  // Public DistSimAPI methods
  void LogText(const std::string& text) override;
  CallbackHandle RegisterReceiveCallback(TextLogCallback callback) override;
  void UnRegisterReceiveCallback(CallbackHandle handle) override;

  // =========================================
  // OpenRTI callbacks

  // =========================================
  // AbstractInteractionClassSubscriber
  void receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& theParameterValues, rti::VariableLengthData const& theUserSuppliedTag, rti::SupplementalReceiveInfo theReceiveInfo) override;
  void receiveInteraction(rti::InteractionClassHandle theInteraction, rti::ParameterHandleValueMap const& theParameterValues, rti::VariableLengthData const& theUserSuppliedTag, rti::LogicalTime const& theTime, rti::SupplementalReceiveInfo theReceiveInfo) override;

  //=========================================
  // Service Methods called by RemoteAbmassador
  void OnJoined() override;
  void Reset() override;
  bool ProcessBuffers() override;

private:
  // =========================================
  //The parent federate
  RemoteAmbassador& mRemoteAmbassador;

  // Init
  void InitializeHandles();
  void PublishSubscribe();

  //Receive
  void HandleTextLogInteraction(const rti::ParameterHandleValueMap& theParameterValues, const rti::LogicalTime& theTime, bool sentBySelf);

  CallbackHandle                            mNextReceiveCallbackHandle;
  std::map<CallbackHandle, TextLogCallback> mReceiveCallbacks;

  rti::InteractionClassHandle               mTextLogIAHandle;
  rti::ParameterHandle                      mTextLogIATextParameter;
  rti::ParameterHandle                      mTextLogIASenderParameter;
};

} // namespace NDistSimIB