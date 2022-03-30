
// Code automatically generated from RTFederate.xml, do not edit

#pragma once

#include <vector>
#include <cstring>

#include "RTI/encoding/BasicDataElements.h"
#include "RTI/encoding/HLAfixedRecord.h"
#include "RTI/encoding/HLAfixedArray.h"
#include "RTI/encoding/HLAvariableArray.h"

#include "RTFederateDataTypes.h"
#include "DistSimIBConfig.h"

namespace NDistSimIB {
namespace NRTFederateEncoding {

// 
// Bytes carried in a CAN frame. Note that the actual number of bytes is carried in
// a distinct member field of the CANFrame.
//         
// fixed array of simple type HLAbyte
class CANMessageDataEncoding : public rti::HLAfixedArray
{
  public:
    CANMessageDataEncoding();
    CANMessageDataEncoding(const std::vector<uint8_t>& data);
    using rti::HLAfixedArray::set;
    void set(const std::vector<uint8_t>& data);
    void set(const uint8_t* data, size_t size);
    using rti::HLAfixedArray::get;
    const std::vector<uint8_t>& get() const;
  private:
    std::vector<uint8_t> mData;
    rti::HLAbyte mEncoding[64];
}; // class CANMessageData
// 
// Bytes carried in a Ethernet Packet. The data is variable-sized and carries the
// actual number of bytes used.
//         
// variable array of simple type HLAbyte
class EthernetPacketDataEncoding : public rti::HLAvariableArray
{
  public:
    EthernetPacketDataEncoding();
    EthernetPacketDataEncoding(const std::vector<uint8_t>& data);
    using rti::HLAvariableArray::set;
    void set(const std::vector<uint8_t>& data);
    void set(const uint8_t* data, size_t size);
    using rti::HLAvariableArray::get;
    const std::vector<uint8_t>& get() const;
    void resize(size_t size);
    using rti::HLAvariableArray::decodeFrom;
    virtual size_t decodeFrom(const rti::Octet* buffer, size_t bufferSize, size_t index) override;
  private:
    void update();
    std::vector<uint8_t> mData;
    std::vector<rti::HLAbyte> mEncoding;
}; // class EthernetPacketData

// CAN Frame
class CANFrameEncoding : public CANFrame, public rti::HLAfixedRecord
{
  public:
    CANFrameEncoding();
    virtual ~CANFrameEncoding();
    uint32_t getVersion() const override;
    void SetClientIndex(int16_t value) override;
    int16_t GetClientIndex() const override;
    bool IsClientIndexAvailable() const override;
    void SetDir(DirMask value) override;
    DirMask GetDir() const override;
    bool IsDirAvailable() const override;
    void SetOriginalTimeStamp(VTimeNS value) override;
    VTimeNS GetOriginalTimeStamp() const override;
    bool IsOriginalTimeStampAvailable() const override;
    void SetSimulated(SimulatedFlag value) override;
    SimulatedFlag GetSimulated() const override;
    bool IsSimulatedAvailable() const override;
    void SetId(VMessageId value) override;
    VMessageId GetId() const override;
    bool IsIdAvailable() const override;
    void SetFlags(int32_t value) override;
    int32_t GetFlags() const override;
    bool IsFlagsAvailable() const override;
    void SetDataLength(uint8_t value) override;
    uint8_t GetDataLength() const override;
    bool IsDataLengthAvailable() const override;
    void SetData(const std::vector<uint8_t>& value) override;
    void SetData(const uint8_t* value, size_t size) override;
    const std::vector<uint8_t>& GetData() const override;
    bool IsDataAvailable() const override;
    void SetFrameLengthNS(int32_t value) override;
    int32_t GetFrameLengthNS() const override;
    bool IsFrameLengthNSAvailable() const override;
    void SetBitCount(int16_t value) override;
    int16_t GetBitCount() const override;
    bool IsBitCountAvailable() const override;
  private:
    rti::HLAinteger16LE mClientIndex;
    rti::HLAoctet mDir;
    rti::HLAinteger64LE mOriginalTimeStamp;
    rti::HLAoctet mSimulated;
    rti::HLAinteger32LE mId;
    rti::HLAinteger32LE mFlags;
    rti::HLAoctet mDataLength;
    CANMessageDataEncoding mData;
    rti::HLAinteger32LE mFrameLengthNS;
    rti::HLAinteger16LE mBitCount;
}; // class CANFrameEncoding

// CAN Error Frame
class CANErrorFrameEncoding : public CANErrorFrame, public rti::HLAfixedRecord
{
  public:
    CANErrorFrameEncoding();
    virtual ~CANErrorFrameEncoding();
    uint32_t getVersion() const override;
    void SetErrorBitPosition(int16_t value) override;
    int16_t GetErrorBitPosition() const override;
    bool IsErrorBitPositionAvailable() const override;
    void SetFrameLengthNS(int32_t value) override;
    int32_t GetFrameLengthNS() const override;
    bool IsFrameLengthNSAvailable() const override;
    void SetClientIndex(int16_t value) override;
    int16_t GetClientIndex() const override;
    bool IsClientIndexAvailable() const override;
  private:
    rti::HLAinteger16LE mErrorBitPosition;
    rti::HLAinteger32LE mFrameLengthNS;
    rti::HLAinteger16LE mClientIndex;
}; // class CANErrorFrameEncoding

// Ethernet Status Event
class EthernetStatusEncoding : public EthernetStatus, public rti::HLAfixedRecord
{
  public:
    EthernetStatusEncoding();
    virtual ~EthernetStatusEncoding();
    uint32_t getVersion() const override;
    void SetBusType(int16_t value) override;
    int16_t GetBusType() const override;
    bool IsBusTypeAvailable() const override;
    void SetHardwareChannel(int64_t value) override;
    int64_t GetHardwareChannel() const override;
    bool IsHardwareChannelAvailable() const override;
    void SetLinkStatus(LinkStatus value) override;
    LinkStatus GetLinkStatus() const override;
    bool IsLinkStatusAvailable() const override;
    void SetBitrate(int32_t value) override;
    int32_t GetBitrate() const override;
    bool IsBitrateAvailable() const override;
    void SetEthernetPhy(EthernetPhy value) override;
    EthernetPhy GetEthernetPhy() const override;
    bool IsEthernetPhyAvailable() const override;
    void SetDuplex(Duplex value) override;
    Duplex GetDuplex() const override;
    bool IsDuplexAvailable() const override;
    void SetMdiType(MdiType value) override;
    MdiType GetMdiType() const override;
    bool IsMdiTypeAvailable() const override;
    void SetConnector(Connector value) override;
    Connector GetConnector() const override;
    bool IsConnectorAvailable() const override;
    void SetClockMode(ClockMode value) override;
    ClockMode GetClockMode() const override;
    bool IsClockModeAvailable() const override;
    void SetBrPair(BrPair value) override;
    BrPair GetBrPair() const override;
    bool IsBrPairAvailable() const override;
    void SetDeviceOperationMode(int32_t value) override;
    int32_t GetDeviceOperationMode() const override;
    bool IsDeviceOperationModeAvailable() const override;
  private:
    rti::HLAinteger16LE mBusType;
    rti::HLAinteger64LE mHardwareChannel;
    rti::HLAinteger32LE mLinkStatus;
    rti::HLAinteger32LE mBitrate;
    rti::HLAinteger32LE mEthernetPhy;
    rti::HLAinteger32LE mDuplex;
    rti::HLAinteger32LE mMdiType;
    rti::HLAinteger32LE mConnector;
    rti::HLAinteger32LE mClockMode;
    rti::HLAinteger32LE mBrPair;
    rti::HLAinteger32LE mDeviceOperationMode;
}; // class EthernetStatusEncoding

// Ethernet Packet
class EthernetPacketEncoding : public EthernetPacket, public rti::HLAfixedRecord
{
  public:
    EthernetPacketEncoding();
    virtual ~EthernetPacketEncoding();
    uint32_t getVersion() const override;
    void SetClientIndex(int16_t value) override;
    int16_t GetClientIndex() const override;
    bool IsClientIndexAvailable() const override;
    void SetDir(DirMask value) override;
    DirMask GetDir() const override;
    bool IsDirAvailable() const override;
    void SetOriginalTimeStamp(VTimeNS value) override;
    VTimeNS GetOriginalTimeStamp() const override;
    bool IsOriginalTimeStampAvailable() const override;
    void SetSimulated(SimulatedFlag value) override;
    SimulatedFlag GetSimulated() const override;
    bool IsSimulatedAvailable() const override;
    void SetBusType(int16_t value) override;
    int16_t GetBusType() const override;
    bool IsBusTypeAvailable() const override;
    void SetHardwareChannel(int64_t value) override;
    int64_t GetHardwareChannel() const override;
    bool IsHardwareChannelAvailable() const override;
    void SetFrameDuration(VTimeNS value) override;
    VTimeNS GetFrameDuration() const override;
    bool IsFrameDurationAvailable() const override;
    void SetEthernetChecksum(int32_t value) override;
    int32_t GetEthernetChecksum() const override;
    bool IsEthernetChecksumAvailable() const override;
    void SetPacketData(const std::vector<uint8_t>& value) override;
    void SetPacketData(const uint8_t* value, size_t size) override;
    const std::vector<uint8_t>& GetPacketData() const override;
    bool IsPacketDataAvailable() const override;
  private:
    rti::HLAinteger16LE mClientIndex;
    rti::HLAoctet mDir;
    rti::HLAinteger64LE mOriginalTimeStamp;
    rti::HLAoctet mSimulated;
    rti::HLAinteger16LE mBusType;
    rti::HLAinteger64LE mHardwareChannel;
    rti::HLAinteger64LE mFrameDuration;
    rti::HLAinteger32LE mEthernetChecksum;
    EthernetPacketDataEncoding mPacketData;
}; // class EthernetPacketEncoding

class EthernetPacketForwardedEncoding : public EthernetPacketForwarded, public rti::HLAfixedRecord
{
  public:
    EthernetPacketForwardedEncoding();
    virtual ~EthernetPacketForwardedEncoding();
    uint32_t getVersion() const override;
    void SetClientIndex(int16_t value) override;
    int16_t GetClientIndex() const override;
    bool IsClientIndexAvailable() const override;
    void SetDir(DirMask value) override;
    DirMask GetDir() const override;
    bool IsDirAvailable() const override;
    void SetOriginalTimeStamp(VTimeNS value) override;
    VTimeNS GetOriginalTimeStamp() const override;
    bool IsOriginalTimeStampAvailable() const override;
    void SetSimulated(SimulatedFlag value) override;
    SimulatedFlag GetSimulated() const override;
    bool IsSimulatedAvailable() const override;
    void SetBusType(int16_t value) override;
    int16_t GetBusType() const override;
    bool IsBusTypeAvailable() const override;
    void SetHardwareChannel(int64_t value) override;
    int64_t GetHardwareChannel() const override;
    bool IsHardwareChannelAvailable() const override;
    void SetFrameDuration(VTimeNS value) override;
    VTimeNS GetFrameDuration() const override;
    bool IsFrameDurationAvailable() const override;
    void SetEthernetChecksum(int32_t value) override;
    int32_t GetEthernetChecksum() const override;
    bool IsEthernetChecksumAvailable() const override;
    void SetPacketData(const std::vector<uint8_t>& value) override;
    void SetPacketData(const uint8_t* value, size_t size) override;
    const std::vector<uint8_t>& GetPacketData() const override;
    bool IsPacketDataAvailable() const override;
  private:
    rti::HLAinteger16LE mClientIndex;
    rti::HLAoctet mDir;
    rti::HLAinteger64LE mOriginalTimeStamp;
    rti::HLAoctet mSimulated;
    rti::HLAinteger16LE mBusType;
    rti::HLAinteger64LE mHardwareChannel;
    rti::HLAinteger64LE mFrameDuration;
    rti::HLAinteger32LE mEthernetChecksum;
    EthernetPacketDataEncoding mPacketData;
}; // class EthernetPacketForwardedEncoding

class EthernetPacketErrorEncoding : public EthernetPacketError, public rti::HLAfixedRecord
{
  public:
    EthernetPacketErrorEncoding();
    virtual ~EthernetPacketErrorEncoding();
    uint32_t getVersion() const override;
    void SetClientIndex(int16_t value) override;
    int16_t GetClientIndex() const override;
    bool IsClientIndexAvailable() const override;
    void SetDir(DirMask value) override;
    DirMask GetDir() const override;
    bool IsDirAvailable() const override;
    void SetOriginalTimeStamp(VTimeNS value) override;
    VTimeNS GetOriginalTimeStamp() const override;
    bool IsOriginalTimeStampAvailable() const override;
    void SetSimulated(SimulatedFlag value) override;
    SimulatedFlag GetSimulated() const override;
    bool IsSimulatedAvailable() const override;
    void SetBusType(int16_t value) override;
    int16_t GetBusType() const override;
    bool IsBusTypeAvailable() const override;
    void SetHardwareChannel(int64_t value) override;
    int64_t GetHardwareChannel() const override;
    bool IsHardwareChannelAvailable() const override;
    void SetFrameDuration(VTimeNS value) override;
    VTimeNS GetFrameDuration() const override;
    bool IsFrameDurationAvailable() const override;
    void SetErrorCode(int32_t value) override;
    int32_t GetErrorCode() const override;
    bool IsErrorCodeAvailable() const override;
    void SetEthernetChecksum(int32_t value) override;
    int32_t GetEthernetChecksum() const override;
    bool IsEthernetChecksumAvailable() const override;
    void SetPacketData(const std::vector<uint8_t>& value) override;
    void SetPacketData(const uint8_t* value, size_t size) override;
    const std::vector<uint8_t>& GetPacketData() const override;
    bool IsPacketDataAvailable() const override;
  private:
    rti::HLAinteger16LE mClientIndex;
    rti::HLAoctet mDir;
    rti::HLAinteger64LE mOriginalTimeStamp;
    rti::HLAoctet mSimulated;
    rti::HLAinteger16LE mBusType;
    rti::HLAinteger64LE mHardwareChannel;
    rti::HLAinteger64LE mFrameDuration;
    rti::HLAinteger32LE mErrorCode;
    rti::HLAinteger32LE mEthernetChecksum;
    EthernetPacketDataEncoding mPacketData;
}; // class EthernetPacketErrorEncoding

class EthernetPacketErrorForwardedEncoding : public EthernetPacketErrorForwarded, public rti::HLAfixedRecord
{
  public:
    EthernetPacketErrorForwardedEncoding();
    virtual ~EthernetPacketErrorForwardedEncoding();
    uint32_t getVersion() const override;
    void SetClientIndex(int16_t value) override;
    int16_t GetClientIndex() const override;
    bool IsClientIndexAvailable() const override;
    void SetDir(DirMask value) override;
    DirMask GetDir() const override;
    bool IsDirAvailable() const override;
    void SetOriginalTimeStamp(VTimeNS value) override;
    VTimeNS GetOriginalTimeStamp() const override;
    bool IsOriginalTimeStampAvailable() const override;
    void SetSimulated(SimulatedFlag value) override;
    SimulatedFlag GetSimulated() const override;
    bool IsSimulatedAvailable() const override;
    void SetBusType(int16_t value) override;
    int16_t GetBusType() const override;
    bool IsBusTypeAvailable() const override;
    void SetHardwareChannel(int64_t value) override;
    int64_t GetHardwareChannel() const override;
    bool IsHardwareChannelAvailable() const override;
    void SetFrameDuration(VTimeNS value) override;
    VTimeNS GetFrameDuration() const override;
    bool IsFrameDurationAvailable() const override;
    void SetErrorCode(int32_t value) override;
    int32_t GetErrorCode() const override;
    bool IsErrorCodeAvailable() const override;
    void SetEthernetChecksum(int32_t value) override;
    int32_t GetEthernetChecksum() const override;
    bool IsEthernetChecksumAvailable() const override;
    void SetPacketData(const std::vector<uint8_t>& value) override;
    void SetPacketData(const uint8_t* value, size_t size) override;
    const std::vector<uint8_t>& GetPacketData() const override;
    bool IsPacketDataAvailable() const override;
  private:
    rti::HLAinteger16LE mClientIndex;
    rti::HLAoctet mDir;
    rti::HLAinteger64LE mOriginalTimeStamp;
    rti::HLAoctet mSimulated;
    rti::HLAinteger16LE mBusType;
    rti::HLAinteger64LE mHardwareChannel;
    rti::HLAinteger64LE mFrameDuration;
    rti::HLAinteger32LE mErrorCode;
    rti::HLAinteger32LE mEthernetChecksum;
    EthernetPacketDataEncoding mPacketData;
}; // class EthernetPacketErrorForwardedEncoding

} // namespace NDistSimIB
} // namespace NRTFederateEncoding

