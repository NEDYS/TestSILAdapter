
// Code automatically generated from RTFederate.xml, do not edit

#include <vector>
#include <cstring>

#include "RTI/encoding/BasicDataElements.h"
#include "RTI/encoding/HLAfixedRecord.h"
#include "RTI/encoding/HLAfixedArray.h"
#include "RTI/encoding/HLAvariableArray.h"

#include "RTFederateEncodings.h"

namespace NDistSimIB {
namespace NRTFederateEncoding {

// 
// Bytes carried in a CAN frame. Note that the actual number of bytes is carried in
// a distinct member field of the CANFrame.
//         
// fixed array of simple type HLAbyte
CANMessageDataEncoding::CANMessageDataEncoding()
  : HLAfixedArray(rti::HLAbyte(), 64)
  , mData(64)
{
  for (int i=0; i < 64; i++)
  {
    mEncoding[i].setDataPointer(&mData[i]);
    setElementPointer(i, &mEncoding[i]);
  }
}
CANMessageDataEncoding::CANMessageDataEncoding(const std::vector<uint8_t>& data)
  : HLAfixedArray(rti::HLAbyte(), 64)
  , mData(64)
{
  set(data.data(), data.size());
  for (int i=0;i<64;i++)
  {
    mEncoding[i].setDataPointer(&mData[i]);
    setElementPointer(i, &mEncoding[i]);
  }
}
void CANMessageDataEncoding::set(const std::vector<uint8_t>& data) {
  set(data.data(), data.size());
}
void CANMessageDataEncoding::set(const uint8_t* data, size_t size) 
{
  if (size <= 64) {
    memcpy(mData.data(), data, size);
  } else {
    memcpy(mData.data(), data, 64);
  }
}
const std::vector<uint8_t>& CANMessageDataEncoding::get() const
{
  return mData;
}
// 
// Bytes carried in a Ethernet Packet. The data is variable-sized and carries the
// actual number of bytes used.
//         
// variable array of simple type HLAbyte
EthernetPacketDataEncoding::EthernetPacketDataEncoding()
  : HLAvariableArray(rti::HLAbyte())
  , mData()
{
}
EthernetPacketDataEncoding::EthernetPacketDataEncoding(const std::vector<uint8_t>& data)
  : HLAvariableArray(rti::HLAbyte())
  , mData(data)
{
  update();
}
void EthernetPacketDataEncoding::set(const std::vector<uint8_t>& data)
{
  if (mData.size() != data.size()) {
    mData = data;
    update();
  } else {
    memcpy(mData.data(), data.data(), mData.size());
  }
}
void EthernetPacketDataEncoding::set(const uint8_t* data, size_t size)
{
  if (mData.size() != size) {
    mData.resize(size);
    update();
  }
  memcpy(mData.data(), data, size);
}
const std::vector<uint8_t>& EthernetPacketDataEncoding::get() const { return mData; }
void EthernetPacketDataEncoding::resize(size_t size)
{
  mData.resize(size);
  update();
}
// resize buffer and encoder array before actually decoding 
size_t EthernetPacketDataEncoding::decodeFrom(const rti::Octet* buffer, size_t bufferSize, size_t index)
{
  size_t newSize = decodedSize(buffer, bufferSize, index);
  resize(newSize);
  return HLAvariableArray::decodeFrom(buffer, bufferSize, index);
}
void EthernetPacketDataEncoding::update()
{
  size_t size = mData.size();
  mEncoding.resize(size);
  for (size_t i=0; i<size; i++)
  {
    mEncoding[i].setDataPointer(&mData.data()[i]);
    if (i < HLAvariableArray::size()) {
      setElementPointer(i, &mEncoding[i]);
    } else {
      addElementPointer(&mEncoding[i]);
    }
  }
}

// CAN Frame
CANFrameEncoding::CANFrameEncoding() : rti::HLAfixedRecord(1)
{
  rti::HLAfixedRecord::appendElementPointer(&mClientIndex, 0);
  rti::HLAfixedRecord::appendElementPointer(&mDir, 0);
  rti::HLAfixedRecord::appendElementPointer(&mOriginalTimeStamp, 0);
  rti::HLAfixedRecord::appendElementPointer(&mSimulated, 0);
  rti::HLAfixedRecord::appendElementPointer(&mId, 1);
  rti::HLAfixedRecord::appendElementPointer(&mFlags, 1);
  rti::HLAfixedRecord::appendElementPointer(&mDataLength, 1);
  rti::HLAfixedRecord::appendElementPointer(&mData, 1);
  rti::HLAfixedRecord::appendElementPointer(&mFrameLengthNS, 1);
  rti::HLAfixedRecord::appendElementPointer(&mBitCount, 1);
}
CANFrameEncoding::~CANFrameEncoding()
{
}
uint32_t CANFrameEncoding::getVersion() const { return rti::HLAfixedRecord::getVersion(); }
void CANFrameEncoding::SetClientIndex(int16_t value)
{
  mClientIndex.set(value);
}
int16_t CANFrameEncoding::GetClientIndex() const
{
  return mClientIndex.get();
}
bool CANFrameEncoding::IsClientIndexAvailable() const
{
  return true;
}
void CANFrameEncoding::SetDir(DirMask value)
{
  mDir = value;
}
DirMask CANFrameEncoding::GetDir() const
{
  return static_cast<DirMask>(mDir.get());
}
bool CANFrameEncoding::IsDirAvailable() const
{
  return true;
}
void CANFrameEncoding::SetOriginalTimeStamp(VTimeNS value)
{
  mOriginalTimeStamp.set(value);
}
VTimeNS CANFrameEncoding::GetOriginalTimeStamp() const
{
  return mOriginalTimeStamp.get();
}
bool CANFrameEncoding::IsOriginalTimeStampAvailable() const
{
  return true;
}
void CANFrameEncoding::SetSimulated(SimulatedFlag value)
{
  mSimulated = value;
}
SimulatedFlag CANFrameEncoding::GetSimulated() const
{
  return static_cast<SimulatedFlag>(mSimulated.get());
}
bool CANFrameEncoding::IsSimulatedAvailable() const
{
  return true;
}
void CANFrameEncoding::SetId(VMessageId value)
{
  mId.set(value);
}
VMessageId CANFrameEncoding::GetId() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"Id\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mId.get();
}
bool CANFrameEncoding::IsIdAvailable() const
{
  return (getVersion() >= 1);
}
void CANFrameEncoding::SetFlags(int32_t value)
{
  mFlags.set(value);
}
int32_t CANFrameEncoding::GetFlags() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"Flags\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mFlags.get();
}
bool CANFrameEncoding::IsFlagsAvailable() const
{
  return (getVersion() >= 1);
}
void CANFrameEncoding::SetDataLength(uint8_t value)
{
  mDataLength.set(value);
}
uint8_t CANFrameEncoding::GetDataLength() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"DataLength\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mDataLength.get();
}
bool CANFrameEncoding::IsDataLengthAvailable() const
{
  return (getVersion() >= 1);
}
void CANFrameEncoding::SetData(const std::vector<uint8_t>& value)
{
  mData.set(value);
}
void CANFrameEncoding::SetData(const uint8_t* value, size_t size)
{
	mData.set(value, size);
}
const std::vector<uint8_t>& CANFrameEncoding::GetData() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"Data\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mData.get();
}
bool CANFrameEncoding::IsDataAvailable() const
{
  return (getVersion() >= 1);
}
void CANFrameEncoding::SetFrameLengthNS(int32_t value)
{
  mFrameLengthNS.set(value);
}
int32_t CANFrameEncoding::GetFrameLengthNS() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"FrameLengthNS\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mFrameLengthNS.get();
}
bool CANFrameEncoding::IsFrameLengthNSAvailable() const
{
  return (getVersion() >= 1);
}
void CANFrameEncoding::SetBitCount(int16_t value)
{
  mBitCount.set(value);
}
int16_t CANFrameEncoding::GetBitCount() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"BitCount\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mBitCount.get();
}
bool CANFrameEncoding::IsBitCountAvailable() const
{
  return (getVersion() >= 1);
}
// CAN Error Frame
CANErrorFrameEncoding::CANErrorFrameEncoding() : rti::HLAfixedRecord(1)
{
  rti::HLAfixedRecord::appendElementPointer(&mErrorBitPosition, 1);
  rti::HLAfixedRecord::appendElementPointer(&mFrameLengthNS, 1);
  rti::HLAfixedRecord::appendElementPointer(&mClientIndex, 1);
}
CANErrorFrameEncoding::~CANErrorFrameEncoding()
{
}
uint32_t CANErrorFrameEncoding::getVersion() const { return rti::HLAfixedRecord::getVersion(); }
void CANErrorFrameEncoding::SetErrorBitPosition(int16_t value)
{
  mErrorBitPosition.set(value);
}
int16_t CANErrorFrameEncoding::GetErrorBitPosition() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"ErrorBitPosition\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mErrorBitPosition.get();
}
bool CANErrorFrameEncoding::IsErrorBitPositionAvailable() const
{
  return (getVersion() >= 1);
}
void CANErrorFrameEncoding::SetFrameLengthNS(int32_t value)
{
  mFrameLengthNS.set(value);
}
int32_t CANErrorFrameEncoding::GetFrameLengthNS() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"FrameLengthNS\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mFrameLengthNS.get();
}
bool CANErrorFrameEncoding::IsFrameLengthNSAvailable() const
{
  return (getVersion() >= 1);
}
void CANErrorFrameEncoding::SetClientIndex(int16_t value)
{
  mClientIndex.set(value);
}
int16_t CANErrorFrameEncoding::GetClientIndex() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"ClientIndex\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mClientIndex.get();
}
bool CANErrorFrameEncoding::IsClientIndexAvailable() const
{
  return (getVersion() >= 1);
}
// Ethernet Status Event
EthernetStatusEncoding::EthernetStatusEncoding() : rti::HLAfixedRecord(1)
{
  rti::HLAfixedRecord::appendElementPointer(&mBusType, 1);
  rti::HLAfixedRecord::appendElementPointer(&mHardwareChannel, 1);
  rti::HLAfixedRecord::appendElementPointer(&mLinkStatus, 1);
  rti::HLAfixedRecord::appendElementPointer(&mBitrate, 1);
  rti::HLAfixedRecord::appendElementPointer(&mEthernetPhy, 1);
  rti::HLAfixedRecord::appendElementPointer(&mDuplex, 1);
  rti::HLAfixedRecord::appendElementPointer(&mMdiType, 1);
  rti::HLAfixedRecord::appendElementPointer(&mConnector, 1);
  rti::HLAfixedRecord::appendElementPointer(&mClockMode, 1);
  rti::HLAfixedRecord::appendElementPointer(&mBrPair, 1);
  rti::HLAfixedRecord::appendElementPointer(&mDeviceOperationMode, 1);
}
EthernetStatusEncoding::~EthernetStatusEncoding()
{
}
uint32_t EthernetStatusEncoding::getVersion() const { return rti::HLAfixedRecord::getVersion(); }
void EthernetStatusEncoding::SetBusType(int16_t value)
{
  mBusType.set(value);
}
int16_t EthernetStatusEncoding::GetBusType() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"BusType\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mBusType.get();
}
bool EthernetStatusEncoding::IsBusTypeAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetStatusEncoding::SetHardwareChannel(int64_t value)
{
  mHardwareChannel.set(value);
}
int64_t EthernetStatusEncoding::GetHardwareChannel() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"HardwareChannel\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mHardwareChannel.get();
}
bool EthernetStatusEncoding::IsHardwareChannelAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetStatusEncoding::SetLinkStatus(LinkStatus value)
{
  mLinkStatus = value;
}
LinkStatus EthernetStatusEncoding::GetLinkStatus() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"LinkStatus\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return static_cast<LinkStatus>(mLinkStatus.get());
}
bool EthernetStatusEncoding::IsLinkStatusAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetStatusEncoding::SetBitrate(int32_t value)
{
  mBitrate.set(value);
}
int32_t EthernetStatusEncoding::GetBitrate() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"Bitrate\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mBitrate.get();
}
bool EthernetStatusEncoding::IsBitrateAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetStatusEncoding::SetEthernetPhy(EthernetPhy value)
{
  mEthernetPhy = value;
}
EthernetPhy EthernetStatusEncoding::GetEthernetPhy() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"EthernetPhy\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return static_cast<EthernetPhy>(mEthernetPhy.get());
}
bool EthernetStatusEncoding::IsEthernetPhyAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetStatusEncoding::SetDuplex(Duplex value)
{
  mDuplex = value;
}
Duplex EthernetStatusEncoding::GetDuplex() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"Duplex\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return static_cast<Duplex>(mDuplex.get());
}
bool EthernetStatusEncoding::IsDuplexAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetStatusEncoding::SetMdiType(MdiType value)
{
  mMdiType = value;
}
MdiType EthernetStatusEncoding::GetMdiType() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"MdiType\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return static_cast<MdiType>(mMdiType.get());
}
bool EthernetStatusEncoding::IsMdiTypeAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetStatusEncoding::SetConnector(Connector value)
{
  mConnector = value;
}
Connector EthernetStatusEncoding::GetConnector() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"Connector\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return static_cast<Connector>(mConnector.get());
}
bool EthernetStatusEncoding::IsConnectorAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetStatusEncoding::SetClockMode(ClockMode value)
{
  mClockMode = value;
}
ClockMode EthernetStatusEncoding::GetClockMode() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"ClockMode\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return static_cast<ClockMode>(mClockMode.get());
}
bool EthernetStatusEncoding::IsClockModeAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetStatusEncoding::SetBrPair(BrPair value)
{
  mBrPair = value;
}
BrPair EthernetStatusEncoding::GetBrPair() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"BrPair\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return static_cast<BrPair>(mBrPair.get());
}
bool EthernetStatusEncoding::IsBrPairAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetStatusEncoding::SetDeviceOperationMode(int32_t value)
{
  mDeviceOperationMode.set(value);
}
int32_t EthernetStatusEncoding::GetDeviceOperationMode() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"DeviceOperationMode\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mDeviceOperationMode.get();
}
bool EthernetStatusEncoding::IsDeviceOperationModeAvailable() const
{
  return (getVersion() >= 1);
}
// Ethernet Packet
EthernetPacketEncoding::EthernetPacketEncoding() : rti::HLAfixedRecord(1)
{
  rti::HLAfixedRecord::appendElementPointer(&mClientIndex, 0);
  rti::HLAfixedRecord::appendElementPointer(&mDir, 0);
  rti::HLAfixedRecord::appendElementPointer(&mOriginalTimeStamp, 0);
  rti::HLAfixedRecord::appendElementPointer(&mSimulated, 0);
  rti::HLAfixedRecord::appendElementPointer(&mBusType, 1);
  rti::HLAfixedRecord::appendElementPointer(&mHardwareChannel, 1);
  rti::HLAfixedRecord::appendElementPointer(&mFrameDuration, 1);
  rti::HLAfixedRecord::appendElementPointer(&mEthernetChecksum, 1);
  rti::HLAfixedRecord::appendElementPointer(&mPacketData, 1);
}
EthernetPacketEncoding::~EthernetPacketEncoding()
{
}
uint32_t EthernetPacketEncoding::getVersion() const { return rti::HLAfixedRecord::getVersion(); }
void EthernetPacketEncoding::SetClientIndex(int16_t value)
{
  mClientIndex.set(value);
}
int16_t EthernetPacketEncoding::GetClientIndex() const
{
  return mClientIndex.get();
}
bool EthernetPacketEncoding::IsClientIndexAvailable() const
{
  return true;
}
void EthernetPacketEncoding::SetDir(DirMask value)
{
  mDir = value;
}
DirMask EthernetPacketEncoding::GetDir() const
{
  return static_cast<DirMask>(mDir.get());
}
bool EthernetPacketEncoding::IsDirAvailable() const
{
  return true;
}
void EthernetPacketEncoding::SetOriginalTimeStamp(VTimeNS value)
{
  mOriginalTimeStamp.set(value);
}
VTimeNS EthernetPacketEncoding::GetOriginalTimeStamp() const
{
  return mOriginalTimeStamp.get();
}
bool EthernetPacketEncoding::IsOriginalTimeStampAvailable() const
{
  return true;
}
void EthernetPacketEncoding::SetSimulated(SimulatedFlag value)
{
  mSimulated = value;
}
SimulatedFlag EthernetPacketEncoding::GetSimulated() const
{
  return static_cast<SimulatedFlag>(mSimulated.get());
}
bool EthernetPacketEncoding::IsSimulatedAvailable() const
{
  return true;
}
void EthernetPacketEncoding::SetBusType(int16_t value)
{
  mBusType.set(value);
}
int16_t EthernetPacketEncoding::GetBusType() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"BusType\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mBusType.get();
}
bool EthernetPacketEncoding::IsBusTypeAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketEncoding::SetHardwareChannel(int64_t value)
{
  mHardwareChannel.set(value);
}
int64_t EthernetPacketEncoding::GetHardwareChannel() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"HardwareChannel\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mHardwareChannel.get();
}
bool EthernetPacketEncoding::IsHardwareChannelAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketEncoding::SetFrameDuration(VTimeNS value)
{
  mFrameDuration.set(value);
}
VTimeNS EthernetPacketEncoding::GetFrameDuration() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"FrameDuration\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mFrameDuration.get();
}
bool EthernetPacketEncoding::IsFrameDurationAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketEncoding::SetEthernetChecksum(int32_t value)
{
  mEthernetChecksum.set(value);
}
int32_t EthernetPacketEncoding::GetEthernetChecksum() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"EthernetChecksum\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mEthernetChecksum.get();
}
bool EthernetPacketEncoding::IsEthernetChecksumAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketEncoding::SetPacketData(const std::vector<uint8_t>& value)
{
  mPacketData.set(value);
}
void EthernetPacketEncoding::SetPacketData(const uint8_t* value, size_t size)
{
	mPacketData.set(value, size);
}
const std::vector<uint8_t>& EthernetPacketEncoding::GetPacketData() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"PacketData\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mPacketData.get();
}
bool EthernetPacketEncoding::IsPacketDataAvailable() const
{
  return (getVersion() >= 1);
}
EthernetPacketForwardedEncoding::EthernetPacketForwardedEncoding() : rti::HLAfixedRecord(1)
{
  rti::HLAfixedRecord::appendElementPointer(&mClientIndex, 0);
  rti::HLAfixedRecord::appendElementPointer(&mDir, 0);
  rti::HLAfixedRecord::appendElementPointer(&mOriginalTimeStamp, 0);
  rti::HLAfixedRecord::appendElementPointer(&mSimulated, 0);
  rti::HLAfixedRecord::appendElementPointer(&mBusType, 1);
  rti::HLAfixedRecord::appendElementPointer(&mHardwareChannel, 1);
  rti::HLAfixedRecord::appendElementPointer(&mFrameDuration, 1);
  rti::HLAfixedRecord::appendElementPointer(&mEthernetChecksum, 1);
  rti::HLAfixedRecord::appendElementPointer(&mPacketData, 1);
}
EthernetPacketForwardedEncoding::~EthernetPacketForwardedEncoding()
{
}
uint32_t EthernetPacketForwardedEncoding::getVersion() const { return rti::HLAfixedRecord::getVersion(); }
void EthernetPacketForwardedEncoding::SetClientIndex(int16_t value)
{
  mClientIndex.set(value);
}
int16_t EthernetPacketForwardedEncoding::GetClientIndex() const
{
  return mClientIndex.get();
}
bool EthernetPacketForwardedEncoding::IsClientIndexAvailable() const
{
  return true;
}
void EthernetPacketForwardedEncoding::SetDir(DirMask value)
{
  mDir = value;
}
DirMask EthernetPacketForwardedEncoding::GetDir() const
{
  return static_cast<DirMask>(mDir.get());
}
bool EthernetPacketForwardedEncoding::IsDirAvailable() const
{
  return true;
}
void EthernetPacketForwardedEncoding::SetOriginalTimeStamp(VTimeNS value)
{
  mOriginalTimeStamp.set(value);
}
VTimeNS EthernetPacketForwardedEncoding::GetOriginalTimeStamp() const
{
  return mOriginalTimeStamp.get();
}
bool EthernetPacketForwardedEncoding::IsOriginalTimeStampAvailable() const
{
  return true;
}
void EthernetPacketForwardedEncoding::SetSimulated(SimulatedFlag value)
{
  mSimulated = value;
}
SimulatedFlag EthernetPacketForwardedEncoding::GetSimulated() const
{
  return static_cast<SimulatedFlag>(mSimulated.get());
}
bool EthernetPacketForwardedEncoding::IsSimulatedAvailable() const
{
  return true;
}
void EthernetPacketForwardedEncoding::SetBusType(int16_t value)
{
  mBusType.set(value);
}
int16_t EthernetPacketForwardedEncoding::GetBusType() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"BusType\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mBusType.get();
}
bool EthernetPacketForwardedEncoding::IsBusTypeAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketForwardedEncoding::SetHardwareChannel(int64_t value)
{
  mHardwareChannel.set(value);
}
int64_t EthernetPacketForwardedEncoding::GetHardwareChannel() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"HardwareChannel\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mHardwareChannel.get();
}
bool EthernetPacketForwardedEncoding::IsHardwareChannelAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketForwardedEncoding::SetFrameDuration(VTimeNS value)
{
  mFrameDuration.set(value);
}
VTimeNS EthernetPacketForwardedEncoding::GetFrameDuration() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"FrameDuration\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mFrameDuration.get();
}
bool EthernetPacketForwardedEncoding::IsFrameDurationAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketForwardedEncoding::SetEthernetChecksum(int32_t value)
{
  mEthernetChecksum.set(value);
}
int32_t EthernetPacketForwardedEncoding::GetEthernetChecksum() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"EthernetChecksum\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mEthernetChecksum.get();
}
bool EthernetPacketForwardedEncoding::IsEthernetChecksumAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketForwardedEncoding::SetPacketData(const std::vector<uint8_t>& value)
{
  mPacketData.set(value);
}
void EthernetPacketForwardedEncoding::SetPacketData(const uint8_t* value, size_t size)
{
	mPacketData.set(value, size);
}
const std::vector<uint8_t>& EthernetPacketForwardedEncoding::GetPacketData() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"PacketData\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mPacketData.get();
}
bool EthernetPacketForwardedEncoding::IsPacketDataAvailable() const
{
  return (getVersion() >= 1);
}
EthernetPacketErrorEncoding::EthernetPacketErrorEncoding() : rti::HLAfixedRecord(1)
{
  rti::HLAfixedRecord::appendElementPointer(&mClientIndex, 0);
  rti::HLAfixedRecord::appendElementPointer(&mDir, 0);
  rti::HLAfixedRecord::appendElementPointer(&mOriginalTimeStamp, 0);
  rti::HLAfixedRecord::appendElementPointer(&mSimulated, 0);
  rti::HLAfixedRecord::appendElementPointer(&mBusType, 1);
  rti::HLAfixedRecord::appendElementPointer(&mHardwareChannel, 1);
  rti::HLAfixedRecord::appendElementPointer(&mFrameDuration, 1);
  rti::HLAfixedRecord::appendElementPointer(&mErrorCode, 1);
  rti::HLAfixedRecord::appendElementPointer(&mEthernetChecksum, 1);
  rti::HLAfixedRecord::appendElementPointer(&mPacketData, 1);
}
EthernetPacketErrorEncoding::~EthernetPacketErrorEncoding()
{
}
uint32_t EthernetPacketErrorEncoding::getVersion() const { return rti::HLAfixedRecord::getVersion(); }
void EthernetPacketErrorEncoding::SetClientIndex(int16_t value)
{
  mClientIndex.set(value);
}
int16_t EthernetPacketErrorEncoding::GetClientIndex() const
{
  return mClientIndex.get();
}
bool EthernetPacketErrorEncoding::IsClientIndexAvailable() const
{
  return true;
}
void EthernetPacketErrorEncoding::SetDir(DirMask value)
{
  mDir = value;
}
DirMask EthernetPacketErrorEncoding::GetDir() const
{
  return static_cast<DirMask>(mDir.get());
}
bool EthernetPacketErrorEncoding::IsDirAvailable() const
{
  return true;
}
void EthernetPacketErrorEncoding::SetOriginalTimeStamp(VTimeNS value)
{
  mOriginalTimeStamp.set(value);
}
VTimeNS EthernetPacketErrorEncoding::GetOriginalTimeStamp() const
{
  return mOriginalTimeStamp.get();
}
bool EthernetPacketErrorEncoding::IsOriginalTimeStampAvailable() const
{
  return true;
}
void EthernetPacketErrorEncoding::SetSimulated(SimulatedFlag value)
{
  mSimulated = value;
}
SimulatedFlag EthernetPacketErrorEncoding::GetSimulated() const
{
  return static_cast<SimulatedFlag>(mSimulated.get());
}
bool EthernetPacketErrorEncoding::IsSimulatedAvailable() const
{
  return true;
}
void EthernetPacketErrorEncoding::SetBusType(int16_t value)
{
  mBusType.set(value);
}
int16_t EthernetPacketErrorEncoding::GetBusType() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"BusType\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mBusType.get();
}
bool EthernetPacketErrorEncoding::IsBusTypeAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketErrorEncoding::SetHardwareChannel(int64_t value)
{
  mHardwareChannel.set(value);
}
int64_t EthernetPacketErrorEncoding::GetHardwareChannel() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"HardwareChannel\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mHardwareChannel.get();
}
bool EthernetPacketErrorEncoding::IsHardwareChannelAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketErrorEncoding::SetFrameDuration(VTimeNS value)
{
  mFrameDuration.set(value);
}
VTimeNS EthernetPacketErrorEncoding::GetFrameDuration() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"FrameDuration\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mFrameDuration.get();
}
bool EthernetPacketErrorEncoding::IsFrameDurationAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketErrorEncoding::SetErrorCode(int32_t value)
{
  mErrorCode.set(value);
}
int32_t EthernetPacketErrorEncoding::GetErrorCode() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"ErrorCode\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mErrorCode.get();
}
bool EthernetPacketErrorEncoding::IsErrorCodeAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketErrorEncoding::SetEthernetChecksum(int32_t value)
{
  mEthernetChecksum.set(value);
}
int32_t EthernetPacketErrorEncoding::GetEthernetChecksum() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"EthernetChecksum\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mEthernetChecksum.get();
}
bool EthernetPacketErrorEncoding::IsEthernetChecksumAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketErrorEncoding::SetPacketData(const std::vector<uint8_t>& value)
{
  mPacketData.set(value);
}
void EthernetPacketErrorEncoding::SetPacketData(const uint8_t* value, size_t size)
{
	mPacketData.set(value, size);
}
const std::vector<uint8_t>& EthernetPacketErrorEncoding::GetPacketData() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"PacketData\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mPacketData.get();
}
bool EthernetPacketErrorEncoding::IsPacketDataAvailable() const
{
  return (getVersion() >= 1);
}
EthernetPacketErrorForwardedEncoding::EthernetPacketErrorForwardedEncoding() : rti::HLAfixedRecord(1)
{
  rti::HLAfixedRecord::appendElementPointer(&mClientIndex, 0);
  rti::HLAfixedRecord::appendElementPointer(&mDir, 0);
  rti::HLAfixedRecord::appendElementPointer(&mOriginalTimeStamp, 0);
  rti::HLAfixedRecord::appendElementPointer(&mSimulated, 0);
  rti::HLAfixedRecord::appendElementPointer(&mBusType, 1);
  rti::HLAfixedRecord::appendElementPointer(&mHardwareChannel, 1);
  rti::HLAfixedRecord::appendElementPointer(&mFrameDuration, 1);
  rti::HLAfixedRecord::appendElementPointer(&mErrorCode, 1);
  rti::HLAfixedRecord::appendElementPointer(&mEthernetChecksum, 1);
  rti::HLAfixedRecord::appendElementPointer(&mPacketData, 1);
}
EthernetPacketErrorForwardedEncoding::~EthernetPacketErrorForwardedEncoding()
{
}
uint32_t EthernetPacketErrorForwardedEncoding::getVersion() const { return rti::HLAfixedRecord::getVersion(); }
void EthernetPacketErrorForwardedEncoding::SetClientIndex(int16_t value)
{
  mClientIndex.set(value);
}
int16_t EthernetPacketErrorForwardedEncoding::GetClientIndex() const
{
  return mClientIndex.get();
}
bool EthernetPacketErrorForwardedEncoding::IsClientIndexAvailable() const
{
  return true;
}
void EthernetPacketErrorForwardedEncoding::SetDir(DirMask value)
{
  mDir = value;
}
DirMask EthernetPacketErrorForwardedEncoding::GetDir() const
{
  return static_cast<DirMask>(mDir.get());
}
bool EthernetPacketErrorForwardedEncoding::IsDirAvailable() const
{
  return true;
}
void EthernetPacketErrorForwardedEncoding::SetOriginalTimeStamp(VTimeNS value)
{
  mOriginalTimeStamp.set(value);
}
VTimeNS EthernetPacketErrorForwardedEncoding::GetOriginalTimeStamp() const
{
  return mOriginalTimeStamp.get();
}
bool EthernetPacketErrorForwardedEncoding::IsOriginalTimeStampAvailable() const
{
  return true;
}
void EthernetPacketErrorForwardedEncoding::SetSimulated(SimulatedFlag value)
{
  mSimulated = value;
}
SimulatedFlag EthernetPacketErrorForwardedEncoding::GetSimulated() const
{
  return static_cast<SimulatedFlag>(mSimulated.get());
}
bool EthernetPacketErrorForwardedEncoding::IsSimulatedAvailable() const
{
  return true;
}
void EthernetPacketErrorForwardedEncoding::SetBusType(int16_t value)
{
  mBusType.set(value);
}
int16_t EthernetPacketErrorForwardedEncoding::GetBusType() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"BusType\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mBusType.get();
}
bool EthernetPacketErrorForwardedEncoding::IsBusTypeAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketErrorForwardedEncoding::SetHardwareChannel(int64_t value)
{
  mHardwareChannel.set(value);
}
int64_t EthernetPacketErrorForwardedEncoding::GetHardwareChannel() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"HardwareChannel\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mHardwareChannel.get();
}
bool EthernetPacketErrorForwardedEncoding::IsHardwareChannelAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketErrorForwardedEncoding::SetFrameDuration(VTimeNS value)
{
  mFrameDuration.set(value);
}
VTimeNS EthernetPacketErrorForwardedEncoding::GetFrameDuration() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"FrameDuration\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mFrameDuration.get();
}
bool EthernetPacketErrorForwardedEncoding::IsFrameDurationAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketErrorForwardedEncoding::SetErrorCode(int32_t value)
{
  mErrorCode.set(value);
}
int32_t EthernetPacketErrorForwardedEncoding::GetErrorCode() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"ErrorCode\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mErrorCode.get();
}
bool EthernetPacketErrorForwardedEncoding::IsErrorCodeAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketErrorForwardedEncoding::SetEthernetChecksum(int32_t value)
{
  mEthernetChecksum.set(value);
}
int32_t EthernetPacketErrorForwardedEncoding::GetEthernetChecksum() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"EthernetChecksum\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mEthernetChecksum.get();
}
bool EthernetPacketErrorForwardedEncoding::IsEthernetChecksumAvailable() const
{
  return (getVersion() >= 1);
}
void EthernetPacketErrorForwardedEncoding::SetPacketData(const std::vector<uint8_t>& value)
{
  mPacketData.set(value);
}
void EthernetPacketErrorForwardedEncoding::SetPacketData(const uint8_t* value, size_t size)
{
	mPacketData.set(value, size);
}
const std::vector<uint8_t>& EthernetPacketErrorForwardedEncoding::GetPacketData() const
{
  if (getVersion() < 1)
  {
    throw rti::EncoderException(L"field \"PacketData\" is not available in decoded record version " + std::to_wstring(getVersion()));
  }
  return mPacketData.get();
}
bool EthernetPacketErrorForwardedEncoding::IsPacketDataAvailable() const
{
  return (getVersion() >= 1);
}
} // namespace NDistSimIB
} // namespace NRTFederateEncoding

