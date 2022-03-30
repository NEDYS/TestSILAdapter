/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
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

#ifndef RTIambassadorImplementation_h
#define RTIambassadorImplementation_h

#include <Export.h>
#include <RTI/RTIambassador.h>

namespace OpenRTI {

class OPENRTI_LOCAL RTIambassadorImplementation : public rti1516::RTIambassador {
public:
  RTIambassadorImplementation(const std::vector<std::wstring>& args);
  virtual ~RTIambassadorImplementation();

  // 4.2
  virtual void
  createFederationExecution(std::wstring const & federationExecutionName,
                            std::wstring const & fullPathNameToTheFDDfile,
                            std::wstring const & logicalTimeImplementationName);

  // 4.3
  virtual void
  destroyFederationExecution(std::wstring const & federationExecutionName);

  // 4.4
  virtual rti1516::FederateHandle
  joinFederationExecution(std::wstring const & federateType,
                          std::wstring const & federationExecutionName,
                          rti1516::FederateAmbassador & federateAmbassador);

  // 4.5
  virtual void
  resignFederationExecution(rti1516::ResignAction rti1516ResignAction);

  // 4.6
  virtual void
  registerFederationSynchronizationPoint(std::wstring const & label,
                                         rti1516::VariableLengthData const & rti1516Tag);

  virtual void
  registerFederationSynchronizationPoint(std::wstring const & label,
                                         rti1516::VariableLengthData const & rti1516Tag,
                                         rti1516::FederateHandleSet const & rti1516FederateHandleSet);

  // 4.9
  virtual void
  synchronizationPointAchieved(std::wstring const & label);

  // 4.11
  virtual void
  requestFederationSave(std::wstring const & label);

  virtual void
  requestFederationSave(const std::wstring& label,
                        const rti1516::LogicalTime& rti1516LogicalTime);

  // 4.13
  virtual void
  federateSaveBegun();

  // 4.14
  virtual void
  federateSaveComplete();

  virtual void
  federateSaveNotComplete();

  // 4.16
  virtual void
  queryFederationSaveStatus ();

  // 4.18
  virtual void
  requestFederationRestore(std::wstring const & label);

  // 4.22
  virtual void
  federateRestoreComplete();

  virtual void
  federateRestoreNotComplete();

  // 4.24
  virtual void
  queryFederationRestoreStatus();

  /////////////////////////////////////
  // Declaration Management Services //
  /////////////////////////////////////

  // 5.2
  virtual void
  publishObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                               rti1516::AttributeHandleSet const & rti1516AttributeHandleSet);

  // 5.3
  virtual void
  unpublishObjectClass(rti1516::ObjectClassHandle rti1516ObjectClassHandle);

  virtual void
  unpublishObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                 rti1516::AttributeHandleSet const & rti1516AttributeHandleSet);

  // 5.4
  virtual void
  publishInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle);

  // 5.5
  virtual void
  unpublishInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle);

  // 5.6
  virtual void
  subscribeObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                 rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                 bool active);

  // 5.7
  virtual void
  unsubscribeObjectClass(rti1516::ObjectClassHandle rti1516ObjectClassHandle);

  virtual void
  unsubscribeObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                   rti1516::AttributeHandleSet const & rti1516AttributeHandleSet);

  // 5.8
  virtual void
  subscribeInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                            bool active);

  // 5.9
  virtual void
  unsubscribeInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle);

  ////////////////////////////////
  // Object Management Services //
  ////////////////////////////////

  // 6.2
  virtual void
  reserveObjectInstanceName(std::wstring const & objectInstanceName);

  // 6.4
  virtual rti1516::ObjectInstanceHandle
  registerObjectInstance(rti1516::ObjectClassHandle rti1516ObjectClassHandle);

  virtual rti1516::ObjectInstanceHandle
  registerObjectInstance(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                         std::wstring const & objectInstanceName);

  // 6.6
  virtual void
  updateAttributeValues(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                        const rti1516::AttributeHandleValueMap& rti1516AttributeValues,
                        const rti1516::VariableLengthData& rti1516Tag);

  virtual rti1516::MessageRetractionHandle
  updateAttributeValues(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                        const rti1516::AttributeHandleValueMap& rti1516AttributeValues,
                        const rti1516::VariableLengthData& rti1516Tag,
                        const rti1516::LogicalTime& rti1516LogicalTime);

  // 6.8
  virtual void
  sendInteraction(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                  const rti1516::ParameterHandleValueMap& rti1516ParameterValues,
                  const rti1516::VariableLengthData& rti1516Tag);

  virtual rti1516::MessageRetractionHandle
  sendInteraction(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                  const rti1516::ParameterHandleValueMap& rti1516ParameterValues,
                  const rti1516::VariableLengthData& rti1516Tag,
                  const rti1516::LogicalTime& rti1516LogicalTime);

  // 6.10
  virtual void
  deleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                       const rti1516::VariableLengthData& rti1516Tag);

  virtual rti1516::MessageRetractionHandle
  deleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                       const rti1516::VariableLengthData& rti1516Tag,
                       const rti1516::LogicalTime& rti1516LogicalTime);

  // 6.12
  virtual void
  localDeleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle);

  // 6.13
  virtual void
  changeAttributeTransportationType(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                    rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                    rti1516::TransportationType rti1516TransportationType);

  // 6.14
  virtual void
  changeInteractionTransportationType(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                      rti1516::TransportationType rti1516TransportationType);

  // 6.17
  virtual void
  requestAttributeValueUpdate(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                              rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                              rti1516::VariableLengthData const & rti1516Tag);

  virtual void
  requestAttributeValueUpdate(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                              rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                              rti1516::VariableLengthData const & rti1516Tag);

  ///////////////////////////////////
  // Ownership Management Services //
  ///////////////////////////////////
  // 7.2
  virtual void
  unconditionalAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                             rti1516::AttributeHandleSet const & rti1516AttributeHandleSet);

  // 7.3
  virtual void
  negotiatedAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                          rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                          rti1516::VariableLengthData const & rti1516Tag);

  // 7.6
  virtual void
  confirmDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                     rti1516::AttributeHandleSet const& rti1516AttributeHandleSet,
                     rti1516::VariableLengthData const& rti1516Tag);

  // 7.8
  virtual void
  attributeOwnershipAcquisition(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                rti1516::VariableLengthData const & rti1516Tag);

  // 7.9
  virtual void
  attributeOwnershipAcquisitionIfAvailable(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                           rti1516::AttributeHandleSet const & rti1516AttributeHandleSet);

  // 7.12
  virtual void
  attributeOwnershipDivestitureIfWanted(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                        rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                        rti1516::AttributeHandleSet & rti1516DivestedAttributeSet);

  // 7.13
  virtual void
  cancelNegotiatedAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                rti1516::AttributeHandleSet const & rti1516AttributeHandleSet);

  // 7.14
  virtual void
  cancelAttributeOwnershipAcquisition(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                      rti1516::AttributeHandleSet const & rti1516AttributeHandleSet);

  // 7.16
  virtual void
  queryAttributeOwnership(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                          rti1516::AttributeHandle rti1516AttributeHandle);

  // 7.18
  virtual bool
  isAttributeOwnedByFederate(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                             rti1516::AttributeHandle rti1516AttributeHandle);

  //////////////////////////////
  // Time Management Services //
  //////////////////////////////

  // 8.2
  virtual void
  enableTimeRegulation(const rti1516::LogicalTimeInterval& rti1516Lookahead);

  // 8.4
  virtual void
  disableTimeRegulation();

  // 8.5
  virtual void
  enableTimeConstrained();

  // 8.7
  virtual void
  disableTimeConstrained();

  // 8.8
  virtual void
  timeAdvanceRequest(const rti1516::LogicalTime& logicalTime);

  // 8.9
  virtual void
  timeAdvanceRequestAvailable(const rti1516::LogicalTime& logicalTime);

  // 8.10
  virtual void
  nextMessageRequest(const rti1516::LogicalTime& logicalTime);

  // 8.11
  virtual void
  nextMessageRequestAvailable(const rti1516::LogicalTime& logicalTime);

  // 8.12
  virtual void
  flushQueueRequest(const rti1516::LogicalTime& logicalTime);

  // 8.14
  virtual void
  enableAsynchronousDelivery();

  // 8.15
  virtual void
  disableAsynchronousDelivery();

  // 8.16
  virtual bool
  queryGALT(rti1516::LogicalTime& logicalTime);

  // 8.17
  virtual void
  queryLogicalTime(rti1516::LogicalTime& logicalTime);

  // 8.18
  virtual bool
  queryLITS(rti1516::LogicalTime& logicalTime);

  // 8.19
  virtual void
  modifyLookahead(const rti1516::LogicalTimeInterval& lookahead);

  // 8.20
  virtual void
  queryLookahead(rti1516::LogicalTimeInterval& lookahead);

  // 8.21
  virtual void
  retract(rti1516::MessageRetractionHandle rti1516MessageRetractionHandle);

  // 8.23
  virtual void
  changeAttributeOrderType(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                           rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                           rti1516::OrderType rti1516OrderType);

  // 8.24
  virtual void
  changeInteractionOrderType(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                             rti1516::OrderType rti1516OrderType);

  //////////////////////////////////
  // Data Distribution Management //
  //////////////////////////////////

  // 9.2
  virtual rti1516::RegionHandle
  createRegion(rti1516::DimensionHandleSet const & rti1516DimensionHandleSet);

  // 9.3
  virtual void
  commitRegionModifications(rti1516::RegionHandleSet const & rti1516RegionHandleSet);

  // 9.4
  virtual void
  deleteRegion(rti1516::RegionHandle rti1516RegionHandle);

  // 9.5
  virtual rti1516::ObjectInstanceHandle
  registerObjectInstanceWithRegions(rti1516::ObjectClassHandle theClass,
                                    rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                    theAttributeHandleSetRegionHandleSetPairVector);

  virtual rti1516::ObjectInstanceHandle
  registerObjectInstanceWithRegions(rti1516::ObjectClassHandle theClass,
                                    rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                    theAttributeHandleSetRegionHandleSetPairVector,
                                    std::wstring const & theObjectInstanceName);

  // 9.6
  virtual void
  associateRegionsForUpdates(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                             rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                             theAttributeHandleSetRegionHandleSetPairVector);

  // 9.7
  virtual void
  unassociateRegionsForUpdates(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                               rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                               theAttributeHandleSetRegionHandleSetPairVector);

  // 9.8
  virtual void
  subscribeObjectClassAttributesWithRegions(rti1516::ObjectClassHandle theClass,
                                            rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                            theAttributeHandleSetRegionHandleSetPairVector,
                                            bool active);

  // 9.9
  virtual void
  unsubscribeObjectClassAttributesWithRegions(rti1516::ObjectClassHandle theClass,
                                              rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                              theAttributeHandleSetRegionHandleSetPairVector);

  // 9.10
  virtual void
  subscribeInteractionClassWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                       rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                                       bool active);

  // 9.11
  virtual void
  unsubscribeInteractionClassWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                         rti1516::RegionHandleSet const & rti1516RegionHandleSet);

  // 9.12
  virtual void
  sendInteractionWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                             rti1516::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                             rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                             rti1516::VariableLengthData const & rti1516Tag);

  virtual rti1516::MessageRetractionHandle
  sendInteractionWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                             rti1516::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                             rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                             rti1516::VariableLengthData const & rti1516Tag,
                             rti1516::LogicalTime const & rti1516LogicalTime);

  // 9.13
  virtual void
  requestAttributeValueUpdateWithRegions(rti1516::ObjectClassHandle theClass,
                                         rti1516::AttributeHandleSetRegionHandleSetPairVector const & theSet,
                                         rti1516::VariableLengthData const & rti1516Tag);

  //////////////////////////
  // RTI Support Services //
  //////////////////////////

  // 10.2
  virtual rti1516::ObjectClassHandle
  getObjectClassHandle(std::wstring const & name);

  // 10.3
  virtual std::wstring
  getObjectClassName(rti1516::ObjectClassHandle rti1516ObjectClassHandle);

  // 10.4
  virtual rti1516::AttributeHandle
  getAttributeHandle(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                     std::wstring const & attributeName);

  // 10.5
  virtual std::wstring
  getAttributeName(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                   rti1516::AttributeHandle rti1516AttributeHandle);

  // 10.6
  virtual rti1516::InteractionClassHandle
  getInteractionClassHandle(std::wstring const & name);

  // 10.7
  virtual std::wstring
  getInteractionClassName(rti1516::InteractionClassHandle rti1516InteractionClassHandle);

  // 10.8
  virtual rti1516::ParameterHandle
  getParameterHandle(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                     std::wstring const & parameterName);

  // 10.9
  virtual std::wstring
  getParameterName(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                   rti1516::ParameterHandle rti1516ParameterHandle);

  // 10.10
  virtual rti1516::ObjectInstanceHandle
  getObjectInstanceHandle(std::wstring const & name);

  // 10.11
  virtual std::wstring
  getObjectInstanceName(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle);

  // 10.12
  virtual rti1516::DimensionHandle
  getDimensionHandle(std::wstring const & name);

  // 10.13
  virtual std::wstring
  getDimensionName(rti1516::DimensionHandle rti1516DimensionHandle);

  // 10.14
  virtual unsigned long
  getDimensionUpperBound(rti1516::DimensionHandle rti1516DimensionHandle);

  // 10.15
  virtual rti1516::DimensionHandleSet
  getAvailableDimensionsForClassAttribute(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                          rti1516::AttributeHandle rti1516AttributeHandle);

  // 10.16
  virtual rti1516::ObjectClassHandle
  getKnownObjectClassHandle(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle);

  // 10.17
  virtual rti1516::DimensionHandleSet
  getAvailableDimensionsForInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle);

  // 10.18
  virtual rti1516::TransportationType
  getTransportationType(std::wstring const & transportationName);

  // 10.19
  virtual std::wstring
  getTransportationName(rti1516::TransportationType transportationType);

  // 10.20
  virtual rti1516::OrderType
  getOrderType(std::wstring const & orderName);

  // 10.21
  virtual std::wstring
  getOrderName(rti1516::OrderType orderType);

  // 10.22
  virtual void enableObjectClassRelevanceAdvisorySwitch();

  // 10.23
  virtual void disableObjectClassRelevanceAdvisorySwitch();

  // 10.24
  virtual void enableAttributeRelevanceAdvisorySwitch ();

  // 10.25
  virtual void disableAttributeRelevanceAdvisorySwitch ();

  // 10.26
  virtual void enableAttributeScopeAdvisorySwitch ();

  // 10.27
  virtual void disableAttributeScopeAdvisorySwitch ();

  // 10.28
  virtual void enableInteractionRelevanceAdvisorySwitch ();

  // 10.29
  virtual void disableInteractionRelevanceAdvisorySwitch ();

  // 10.30
  virtual
  rti1516::DimensionHandleSet getDimensionHandleSet(rti1516::RegionHandle rti1516RegionHandle);

  // 10.31
  virtual rti1516::RangeBounds
  getRangeBounds(rti1516::RegionHandle theRegionHandle,
                 rti1516::DimensionHandle theDimensionHandle);

  // 10.32
  virtual void
  setRangeBounds(rti1516::RegionHandle theRegionHandle,
                 rti1516::DimensionHandle theDimensionHandle,
                 rti1516::RangeBounds const & theRangeBounds);

  // 10.33
  virtual unsigned long normalizeFederateHandle(rti1516::FederateHandle rti1516FederateHandle);

  // 10.34
  virtual unsigned long normalizeServiceGroup(rti1516::ServiceGroupIndicator rti1516ServiceGroup);

  // 10.37
  virtual bool evokeCallback(double approximateMinimumTimeInSeconds);

  // 10.38
  virtual bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                                      double approximateMaximumTimeInSeconds);

  // 10.39
  virtual void enableCallbacks ();

  // 10.40
  virtual void disableCallbacks ();

  virtual rti1516::FederateHandle
  decodeFederateHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::ObjectClassHandle
  decodeObjectClassHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::InteractionClassHandle
  decodeInteractionClassHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::ObjectInstanceHandle
  decodeObjectInstanceHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::AttributeHandle
  decodeAttributeHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::ParameterHandle
  decodeParameterHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::DimensionHandle
  decodeDimensionHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::MessageRetractionHandle
  decodeMessageRetractionHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::RegionHandle
  decodeRegionHandle(rti1516::VariableLengthData const & encodedValue) const;

private:
  class RTI1516AmbassadorInterface;
  RTI1516AmbassadorInterface* _ambassadorInterface;
};

}

#endif
