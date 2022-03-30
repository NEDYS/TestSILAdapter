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
#include <RTI/NullFederateAmbassador.h>
#include "dprintf.h"

namespace rti1516e
{


NullFederateAmbassador::NullFederateAmbassador()
{

}

NullFederateAmbassador::~NullFederateAmbassador() noexcept
{

}

void NullFederateAmbassador::connectionLost(std::wstring const & faultDescription)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::reportFederationExecutions(FederationExecutionInformationVector const & theFederationExecutionInformationList)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::synchronizationPointRegistrationSucceeded(std::wstring const & label)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::synchronizationPointRegistrationFailed(std::wstring const & label, SynchronizationPointFailureReason reason)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::announceSynchronizationPoint(std::wstring const & label, VariableLengthData const & theUserSuppliedTag)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::federationSynchronized(std::wstring const & label, FederateHandleSet const& failedToSyncSet)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }


  void NullFederateAmbassador::initiateFederateSave(std::wstring const & label)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }


  void NullFederateAmbassador::initiateFederateSave(std::wstring const & label, LogicalTime const & theTime)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }


  void NullFederateAmbassador::federationSaved()
  {

  }


  void NullFederateAmbassador::federationNotSaved(SaveFailureReason theSaveFailureReason)
  {

  }

  void NullFederateAmbassador::startRegistrationForObjectClass(ObjectClassHandle theClass)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::stopRegistrationForObjectClass(ObjectClassHandle theClass)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::turnInteractionsOn(InteractionClassHandle theHandle)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::turnInteractionsOff(InteractionClassHandle theHandle)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::objectInstanceNameReservationSucceeded(std::wstring const & theObjectInstanceName)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::objectInstanceNameReservationFailed(std::wstring const & theObjectInstanceName)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::multipleObjectInstanceNameReservationSucceeded(std::set<std::wstring> const & theObjectInstanceNames)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::multipleObjectInstanceNameReservationFailed(std::set<std::wstring> const & theObjectInstanceNames)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::discoverObjectInstance(ObjectInstanceHandle theObject, ObjectClassHandle theObjectClass, std::wstring const & theObjectInstanceName)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::discoverObjectInstance(ObjectInstanceHandle theObject, ObjectClassHandle theObjectClass, std::wstring const & theObjectInstanceName, FederateHandle producingFederate)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::reflectAttributeValues(ObjectInstanceHandle theObject, AttributeHandleValueMap const & theAttributeValues, VariableLengthData const & theUserSuppliedTag, OrderType sentOrder, TransportationType theType, SupplementalReflectInfo theReflectInfo)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::reflectAttributeValues(ObjectInstanceHandle theObject, AttributeHandleValueMap const & theAttributeValues, VariableLengthData const & theUserSuppliedTag, OrderType sentOrder, TransportationType theType, LogicalTime const & theTime, OrderType receivedOrder, SupplementalReflectInfo theReflectInfo)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::reflectAttributeValues(ObjectInstanceHandle theObject, AttributeHandleValueMap const & theAttributeValues, VariableLengthData const & theUserSuppliedTag, OrderType sentOrder, TransportationType theType, LogicalTime const & theTime, OrderType receivedOrder, MessageRetractionHandle theHandle, SupplementalReflectInfo theReflectInfo)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::receiveInteraction(InteractionClassHandle theInteraction, ParameterHandleValueMap const & theParameterValues, VariableLengthData const & theUserSuppliedTag, OrderType sentOrder, TransportationType theType, SupplementalReceiveInfo theReceiveInfo)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::receiveInteraction(InteractionClassHandle theInteraction, ParameterHandleValueMap const & theParameterValues, VariableLengthData const & theUserSuppliedTag, OrderType sentOrder, TransportationType theType, LogicalTime const & theTime, OrderType receivedOrder, SupplementalReceiveInfo theReceiveInfo)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }


  void NullFederateAmbassador::receiveInteraction(InteractionClassHandle theInteraction, ParameterHandleValueMap const & theParameterValues, VariableLengthData const & theUserSuppliedTag, OrderType sentOrder, TransportationType theType, LogicalTime const & theTime, OrderType receivedOrder, MessageRetractionHandle theHandle, SupplementalReceiveInfo theReceiveInfo)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::removeObjectInstance(ObjectInstanceHandle theObject, VariableLengthData const & theUserSuppliedTag, OrderType sentOrder, SupplementalRemoveInfo theRemoveInfo)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::removeObjectInstance(ObjectInstanceHandle theObject, VariableLengthData const & theUserSuppliedTag, OrderType sentOrder, LogicalTime const & theTime, OrderType receivedOrder, SupplementalRemoveInfo theRemoveInfo)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::removeObjectInstance(ObjectInstanceHandle theObject, VariableLengthData const & theUserSuppliedTag, OrderType sentOrder, LogicalTime const & theTime, OrderType receivedOrder, MessageRetractionHandle theHandle, SupplementalRemoveInfo theRemoveInfo)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::attributesInScope(ObjectInstanceHandle theObject, AttributeHandleSet const & theAttributes)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::attributesOutOfScope(ObjectInstanceHandle theObject, AttributeHandleSet const & theAttributes)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::provideAttributeValueUpdate(ObjectInstanceHandle theObject, AttributeHandleSet const & theAttributes, VariableLengthData const & theUserSuppliedTag)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::turnUpdatesOnForObjectInstance(ObjectInstanceHandle theObject, AttributeHandleSet const & theAttributes)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::turnUpdatesOnForObjectInstance(ObjectInstanceHandle theObject, AttributeHandleSet const & theAttributes, std::wstring const & updateRateDesignator)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::turnUpdatesOffForObjectInstance(ObjectInstanceHandle theObject, AttributeHandleSet const & theAttributes)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::confirmAttributeTransportationTypeChange(ObjectInstanceHandle theObject, AttributeHandleSet theAttributes, TransportationType theTransportation)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::reportAttributeTransportationType(ObjectInstanceHandle theObject, AttributeHandle theAttribute, TransportationType theTransportation)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }


  void NullFederateAmbassador::confirmInteractionTransportationTypeChange(InteractionClassHandle theInteraction, TransportationType theTransportation)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::reportInteractionTransportationType(FederateHandle federateHandle, InteractionClassHandle theInteraction, TransportationType theTransportation)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }


  void NullFederateAmbassador::informAttributeOwnership(ObjectInstanceHandle theObject, AttributeHandle theAttribute, FederateHandle theOwner)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::attributeIsNotOwned(ObjectInstanceHandle theObject, AttributeHandle theAttribute)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::attributeIsOwnedByRTI(ObjectInstanceHandle theObject, AttributeHandle theAttribute)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::timeRegulationEnabled(LogicalTime const & theFederateTime)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::timeConstrainedEnabled(LogicalTime const & theFederateTime)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::timeAdvanceGrant(LogicalTime const & theTime)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::requestRetraction(MessageRetractionHandle theHandle)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::requestDivestitureConfirmation(ObjectInstanceHandle theObject, AttributeHandleSet const & releasedAttributes)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::requestAttributeOwnershipAssumption(ObjectInstanceHandle theObject, AttributeHandleSet const & offeredAttributes, VariableLengthData const & theUserSuppliedTag)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::confirmAttributeOwnershipAcquisitionCancellation(ObjectInstanceHandle theObject, AttributeHandleSet const & theAttributes)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

  void NullFederateAmbassador::requestAttributeOwnershipRelease(ObjectInstanceHandle theObject, AttributeHandleSet const & candidateAttributes, VariableLengthData const & theUserSuppliedTag)
  {
    DebugPrintf("%s\n", __FUNCSIG__);
  }

}
