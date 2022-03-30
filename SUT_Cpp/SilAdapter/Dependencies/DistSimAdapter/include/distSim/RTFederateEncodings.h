
// Code automatically generated from D:\vfs\CANoe\release\15\Projects_Source\CANoe\Source\RTEVENT\DistSim\RTFederate.xml, do not edit

#pragma once

#include <vector>
#include <cstring>

#include "RTI/encoding/BasicDataElements.h"
#include "RTI/encoding/HLAfixedRecord.h"
#include "RTI/encoding/HLAfixedArray.h"
#include "RTI/encoding/HLAvariableArray.h"
#include "RTFederateDataTypes.h"


namespace NDistSimIB {
namespace NRTFederateEncoding {

// 
// Bytes carried in a CAN frame. Note that the actual number of bytes is carried in
// a distinct member field of the CANFrame.
//         

class CANMessageDataEncoding : public rti1516e::HLAfixedArray
{
  public:
    CANMessageDataEncoding()
      : HLAfixedArray(rti1516e::HLAbyte(), 64)
      , mData(64)
    {
      for (int i=0; i < 64; i++)
      {
        mEncoding[i].setDataPointer(&mData[i]);
        setElementPointer(i, &mEncoding[i]);
      }
    }
    CANMessageDataEncoding(const std::vector<uint8_t>& data)
      : HLAfixedArray(rti1516e::HLAbyte(), 64)
      , mData(64)
    {
      set(data.data(), data.size());
      for (int i=0;i<64;i++)
      {
        mEncoding[i].setDataPointer(&mData[i]);
        setElementPointer(i, &mEncoding[i]);
      }
    }
    void set(const std::vector<uint8_t>& data) {
      set(data.data(), data.size());
    }
    void set(const uint8_t* data, size_t size) 
    {
      if (size <= 64) {
        memcpy(mData.data(), data, size);
      } else {
        memcpy(mData.data(), data, 64);
      }
    }
    const std::vector<uint8_t>& get() const { return mData; }
  private:
    std::vector<uint8_t> mData;
    rti1516e::HLAbyte mEncoding[64];
}; // class CANMessageData

// 
// Bytes carried in a Ethernet Packet. The data is variable-sized and carries the
// actual number of bytes used.
//         

class EthernetPacketDataEncoding : public rti1516e::HLAvariableArray
{
  public:
    EthernetPacketDataEncoding()
      : HLAvariableArray(rti1516e::HLAbyte())
      , mData()
    {
    }
    EthernetPacketDataEncoding(const std::vector<uint8_t>& data)
      : HLAvariableArray(rti1516e::HLAbyte())
      , mData(data)
    {
      update();
    }
    void set(const std::vector<uint8_t>& data)
    {
      if (mData.size() != data.size()) {
        mData = data;
        update();
      } else {
        memcpy(mData.data(), data.data(), mData.size());
      }
    }
    void set(const uint8_t* data, size_t size)
    {
      if (mData.size() != size) {
        mData.resize(size);
        update();
      }
      memcpy(mData.data(), data, size);
    }
    const std::vector<uint8_t>& get() const { return mData; }
    void resize(size_t size)
    {
      mData.resize(size);
      update();
    }
    // resize buffer and encoder array before actually decoding 
    virtual size_t decodeFrom(const rti1516e::Octet* buffer, size_t bufferSize, size_t index) override
    {
      size_t newSize = decodedSize(buffer, bufferSize, index);
      resize(newSize);
      return HLAvariableArray::decodeFrom(buffer, bufferSize, index);
    }
  private:
    void update()
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
    std::vector<uint8_t> mData;
    std::vector<rti1516e::HLAbyte> mEncoding;
}; // class EthernetPacketData

// CAN Frame

class CANFrameEncoding : public CANFrame, public rti1516e::HLAfixedRecord
{
  public:
    CANFrameEncoding() : rti1516e::HLAfixedRecord(1)
    {
      rti1516e::HLAfixedRecord::appendElementPointer(&mClientIndex, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mDir, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mOriginalTimeStamp, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mSimulated, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mId, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mFlags, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mDataLength, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mData, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mFrameLengthNS, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mBitCount, 1);
    }
    virtual ~CANFrameEncoding()
    {
    }
    uint32_t getVersion() const override { return rti1516e::HLAfixedRecord::getVersion(); }
    void SetClientIndex(int16_t value) override
    {
	    mClientIndex.set(value);
    }
    int16_t GetClientIndex() const override
    {
      return mClientIndex.get();
    }
    void SetDir(DirMask value) override
    {
	    mDir.set(value);
    }
    DirMask GetDir() const override
    {
      return static_cast<DirMask>(mDir.get());
    }
    void SetOriginalTimeStamp(VTimeNS value) override
    {
	    mOriginalTimeStamp.set(value);
    }
    VTimeNS GetOriginalTimeStamp() const override
    {
      return mOriginalTimeStamp.get();
    }
    void SetSimulated(SimulatedFlag value) override
    {
	    mSimulated.set(value);
    }
    SimulatedFlag GetSimulated() const override
    {
      return static_cast<SimulatedFlag>(mSimulated.get());
    }
    void SetId(VMessageId value) override
    {
	    mId.set(value);
    }
    VMessageId GetId() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"Id\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mId.get();
    }
    void SetFlags(int32_t value) override
    {
	    mFlags.set(value);
    }
    int32_t GetFlags() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"Flags\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mFlags.get();
    }
    void SetDataLength(uint8_t value) override
    {
	    mDataLength.set(value);
    }
    uint8_t GetDataLength() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"DataLength\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mDataLength.get();
    }
    void SetData(const std::vector<uint8_t>& value) override
    {
	    mData.set(value);
    }
    void SetData(const uint8_t* value, size_t size) override
    {
	    mData.set(value, size);
    }
    const std::vector<uint8_t>& GetData() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"Data\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mData.get();
    }
    void SetFrameLengthNS(int32_t value) override
    {
	    mFrameLengthNS.set(value);
    }
    int32_t GetFrameLengthNS() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"FrameLengthNS\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mFrameLengthNS.get();
    }
    void SetBitCount(int16_t value) override
    {
	    mBitCount.set(value);
    }
    int16_t GetBitCount() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"BitCount\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mBitCount.get();
    }

  private:
    rti1516e::HLAinteger16LE mClientIndex;
    rti1516e::HLAoctet mDir;
    rti1516e::HLAinteger64LE mOriginalTimeStamp;
    rti1516e::HLAoctet mSimulated;
    rti1516e::HLAinteger32LE mId;
    rti1516e::HLAinteger32LE mFlags;
    rti1516e::HLAoctet mDataLength;
    CANMessageDataEncoding mData;
    rti1516e::HLAinteger32LE mFrameLengthNS;
    rti1516e::HLAinteger16LE mBitCount;
}; // class CANFrameEncoding

// CAN Error Frame

class CANErrorFrameEncoding : public CANErrorFrame, public rti1516e::HLAfixedRecord
{
  public:
    CANErrorFrameEncoding() : rti1516e::HLAfixedRecord(1)
    {
      rti1516e::HLAfixedRecord::appendElementPointer(&mErrorBitPosition, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mFrameLengthNS, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mClientIndex, 1);
    }
    virtual ~CANErrorFrameEncoding()
    {
    }
    uint32_t getVersion() const override { return rti1516e::HLAfixedRecord::getVersion(); }
    void SetErrorBitPosition(int16_t value) override
    {
	    mErrorBitPosition.set(value);
    }
    int16_t GetErrorBitPosition() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"ErrorBitPosition\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mErrorBitPosition.get();
    }
    void SetFrameLengthNS(int32_t value) override
    {
	    mFrameLengthNS.set(value);
    }
    int32_t GetFrameLengthNS() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"FrameLengthNS\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mFrameLengthNS.get();
    }
    void SetClientIndex(int16_t value) override
    {
	    mClientIndex.set(value);
    }
    int16_t GetClientIndex() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"ClientIndex\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mClientIndex.get();
    }

  private:
    rti1516e::HLAinteger16LE mErrorBitPosition;
    rti1516e::HLAinteger32LE mFrameLengthNS;
    rti1516e::HLAinteger16LE mClientIndex;
}; // class CANErrorFrameEncoding

// Ethernet Status Event

class EthernetStatusEncoding : public EthernetStatus, public rti1516e::HLAfixedRecord
{
  public:
    EthernetStatusEncoding() : rti1516e::HLAfixedRecord(1)
    {
      rti1516e::HLAfixedRecord::appendElementPointer(&mBusType, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mHardwareChannel, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mLinkStatus, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mBitrate, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mEthernetPhy, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mDuplex, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mMdiType, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mConnector, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mClockMode, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mBrPair, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mDeviceOperationMode, 1);
    }
    virtual ~EthernetStatusEncoding()
    {
    }
    uint32_t getVersion() const override { return rti1516e::HLAfixedRecord::getVersion(); }
    void SetBusType(int16_t value) override
    {
	    mBusType.set(value);
    }
    int16_t GetBusType() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"BusType\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mBusType.get();
    }
    void SetHardwareChannel(int64_t value) override
    {
	    mHardwareChannel.set(value);
    }
    int64_t GetHardwareChannel() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"HardwareChannel\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mHardwareChannel.get();
    }
    void SetLinkStatus(LinkStatus value) override
    {
	    mLinkStatus.set(value);
    }
    LinkStatus GetLinkStatus() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"LinkStatus\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return static_cast<LinkStatus>(mLinkStatus.get());
    }
    void SetBitrate(int32_t value) override
    {
	    mBitrate.set(value);
    }
    int32_t GetBitrate() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"Bitrate\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mBitrate.get();
    }
    void SetEthernetPhy(EthernetPhy value) override
    {
	    mEthernetPhy.set(value);
    }
    EthernetPhy GetEthernetPhy() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"EthernetPhy\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return static_cast<EthernetPhy>(mEthernetPhy.get());
    }
    void SetDuplex(Duplex value) override
    {
	    mDuplex.set(value);
    }
    Duplex GetDuplex() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"Duplex\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return static_cast<Duplex>(mDuplex.get());
    }
    void SetMdiType(MdiType value) override
    {
	    mMdiType.set(value);
    }
    MdiType GetMdiType() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"MdiType\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return static_cast<MdiType>(mMdiType.get());
    }
    void SetConnector(Connector value) override
    {
	    mConnector.set(value);
    }
    Connector GetConnector() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"Connector\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return static_cast<Connector>(mConnector.get());
    }
    void SetClockMode(ClockMode value) override
    {
	    mClockMode.set(value);
    }
    ClockMode GetClockMode() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"ClockMode\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return static_cast<ClockMode>(mClockMode.get());
    }
    void SetBrPair(BrPair value) override
    {
	    mBrPair.set(value);
    }
    BrPair GetBrPair() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"BrPair\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return static_cast<BrPair>(mBrPair.get());
    }
    void SetDeviceOperationMode(int32_t value) override
    {
	    mDeviceOperationMode.set(value);
    }
    int32_t GetDeviceOperationMode() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"DeviceOperationMode\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mDeviceOperationMode.get();
    }

  private:
    rti1516e::HLAinteger16LE mBusType;
    rti1516e::HLAinteger64LE mHardwareChannel;
    rti1516e::HLAinteger32LE mLinkStatus;
    rti1516e::HLAinteger32LE mBitrate;
    rti1516e::HLAinteger32LE mEthernetPhy;
    rti1516e::HLAinteger32LE mDuplex;
    rti1516e::HLAinteger32LE mMdiType;
    rti1516e::HLAinteger32LE mConnector;
    rti1516e::HLAinteger32LE mClockMode;
    rti1516e::HLAinteger32LE mBrPair;
    rti1516e::HLAinteger32LE mDeviceOperationMode;
}; // class EthernetStatusEncoding

// Ethernet Packet

class EthernetPacketEncoding : public EthernetPacket, public rti1516e::HLAfixedRecord
{
  public:
    EthernetPacketEncoding() : rti1516e::HLAfixedRecord(1)
    {
      rti1516e::HLAfixedRecord::appendElementPointer(&mClientIndex, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mDir, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mOriginalTimeStamp, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mSimulated, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mBusType, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mHardwareChannel, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mFrameDuration, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mEthernetChecksum, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mPacketData, 1);
    }
    virtual ~EthernetPacketEncoding()
    {
    }
    uint32_t getVersion() const override { return rti1516e::HLAfixedRecord::getVersion(); }
    void SetClientIndex(int16_t value) override
    {
	    mClientIndex.set(value);
    }
    int16_t GetClientIndex() const override
    {
      return mClientIndex.get();
    }
    void SetDir(DirMask value) override
    {
	    mDir.set(value);
    }
    DirMask GetDir() const override
    {
      return static_cast<DirMask>(mDir.get());
    }
    void SetOriginalTimeStamp(VTimeNS value) override
    {
	    mOriginalTimeStamp.set(value);
    }
    VTimeNS GetOriginalTimeStamp() const override
    {
      return mOriginalTimeStamp.get();
    }
    void SetSimulated(SimulatedFlag value) override
    {
	    mSimulated.set(value);
    }
    SimulatedFlag GetSimulated() const override
    {
      return static_cast<SimulatedFlag>(mSimulated.get());
    }
    void SetBusType(int16_t value) override
    {
	    mBusType.set(value);
    }
    int16_t GetBusType() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"BusType\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mBusType.get();
    }
    void SetHardwareChannel(int64_t value) override
    {
	    mHardwareChannel.set(value);
    }
    int64_t GetHardwareChannel() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"HardwareChannel\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mHardwareChannel.get();
    }
    void SetFrameDuration(VTimeNS value) override
    {
	    mFrameDuration.set(value);
    }
    VTimeNS GetFrameDuration() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"FrameDuration\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mFrameDuration.get();
    }
    void SetEthernetChecksum(int32_t value) override
    {
	    mEthernetChecksum.set(value);
    }
    int32_t GetEthernetChecksum() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"EthernetChecksum\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mEthernetChecksum.get();
    }
    void SetPacketData(const std::vector<uint8_t>& value) override
    {
	    mPacketData.set(value);
    }
    void SetPacketData(const uint8_t* value, size_t size) override
    {
	    mPacketData.set(value, size);
    }
    const std::vector<uint8_t>& GetPacketData() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"PacketData\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mPacketData.get();
    }

  private:
    rti1516e::HLAinteger16LE mClientIndex;
    rti1516e::HLAoctet mDir;
    rti1516e::HLAinteger64LE mOriginalTimeStamp;
    rti1516e::HLAoctet mSimulated;
    rti1516e::HLAinteger16LE mBusType;
    rti1516e::HLAinteger64LE mHardwareChannel;
    rti1516e::HLAinteger64LE mFrameDuration;
    rti1516e::HLAinteger32LE mEthernetChecksum;
    EthernetPacketDataEncoding mPacketData;
}; // class EthernetPacketEncoding

class EthernetPacketForwardedEncoding : public EthernetPacketForwarded, public rti1516e::HLAfixedRecord
{
  public:
    EthernetPacketForwardedEncoding() : rti1516e::HLAfixedRecord(1)
    {
      rti1516e::HLAfixedRecord::appendElementPointer(&mClientIndex, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mDir, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mOriginalTimeStamp, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mSimulated, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mBusType, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mHardwareChannel, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mFrameDuration, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mEthernetChecksum, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mPacketData, 1);
    }
    virtual ~EthernetPacketForwardedEncoding()
    {
    }
    uint32_t getVersion() const override { return rti1516e::HLAfixedRecord::getVersion(); }
    void SetClientIndex(int16_t value) override
    {
	    mClientIndex.set(value);
    }
    int16_t GetClientIndex() const override
    {
      return mClientIndex.get();
    }
    void SetDir(DirMask value) override
    {
	    mDir.set(value);
    }
    DirMask GetDir() const override
    {
      return static_cast<DirMask>(mDir.get());
    }
    void SetOriginalTimeStamp(VTimeNS value) override
    {
	    mOriginalTimeStamp.set(value);
    }
    VTimeNS GetOriginalTimeStamp() const override
    {
      return mOriginalTimeStamp.get();
    }
    void SetSimulated(SimulatedFlag value) override
    {
	    mSimulated.set(value);
    }
    SimulatedFlag GetSimulated() const override
    {
      return static_cast<SimulatedFlag>(mSimulated.get());
    }
    void SetBusType(int16_t value) override
    {
	    mBusType.set(value);
    }
    int16_t GetBusType() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"BusType\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mBusType.get();
    }
    void SetHardwareChannel(int64_t value) override
    {
	    mHardwareChannel.set(value);
    }
    int64_t GetHardwareChannel() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"HardwareChannel\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mHardwareChannel.get();
    }
    void SetFrameDuration(VTimeNS value) override
    {
	    mFrameDuration.set(value);
    }
    VTimeNS GetFrameDuration() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"FrameDuration\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mFrameDuration.get();
    }
    void SetEthernetChecksum(int32_t value) override
    {
	    mEthernetChecksum.set(value);
    }
    int32_t GetEthernetChecksum() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"EthernetChecksum\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mEthernetChecksum.get();
    }
    void SetPacketData(const std::vector<uint8_t>& value) override
    {
	    mPacketData.set(value);
    }
    void SetPacketData(const uint8_t* value, size_t size) override
    {
	    mPacketData.set(value, size);
    }
    const std::vector<uint8_t>& GetPacketData() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"PacketData\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mPacketData.get();
    }

  private:
    rti1516e::HLAinteger16LE mClientIndex;
    rti1516e::HLAoctet mDir;
    rti1516e::HLAinteger64LE mOriginalTimeStamp;
    rti1516e::HLAoctet mSimulated;
    rti1516e::HLAinteger16LE mBusType;
    rti1516e::HLAinteger64LE mHardwareChannel;
    rti1516e::HLAinteger64LE mFrameDuration;
    rti1516e::HLAinteger32LE mEthernetChecksum;
    EthernetPacketDataEncoding mPacketData;
}; // class EthernetPacketForwardedEncoding

class EthernetPacketErrorEncoding : public EthernetPacketError, public rti1516e::HLAfixedRecord
{
  public:
    EthernetPacketErrorEncoding() : rti1516e::HLAfixedRecord(1)
    {
      rti1516e::HLAfixedRecord::appendElementPointer(&mClientIndex, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mDir, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mOriginalTimeStamp, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mSimulated, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mBusType, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mHardwareChannel, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mFrameDuration, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mErrorCode, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mEthernetChecksum, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mPacketData, 1);
    }
    virtual ~EthernetPacketErrorEncoding()
    {
    }
    uint32_t getVersion() const override { return rti1516e::HLAfixedRecord::getVersion(); }
    void SetClientIndex(int16_t value) override
    {
	    mClientIndex.set(value);
    }
    int16_t GetClientIndex() const override
    {
      return mClientIndex.get();
    }
    void SetDir(DirMask value) override
    {
	    mDir.set(value);
    }
    DirMask GetDir() const override
    {
      return static_cast<DirMask>(mDir.get());
    }
    void SetOriginalTimeStamp(VTimeNS value) override
    {
	    mOriginalTimeStamp.set(value);
    }
    VTimeNS GetOriginalTimeStamp() const override
    {
      return mOriginalTimeStamp.get();
    }
    void SetSimulated(SimulatedFlag value) override
    {
	    mSimulated.set(value);
    }
    SimulatedFlag GetSimulated() const override
    {
      return static_cast<SimulatedFlag>(mSimulated.get());
    }
    void SetBusType(int16_t value) override
    {
	    mBusType.set(value);
    }
    int16_t GetBusType() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"BusType\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mBusType.get();
    }
    void SetHardwareChannel(int64_t value) override
    {
	    mHardwareChannel.set(value);
    }
    int64_t GetHardwareChannel() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"HardwareChannel\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mHardwareChannel.get();
    }
    void SetFrameDuration(VTimeNS value) override
    {
	    mFrameDuration.set(value);
    }
    VTimeNS GetFrameDuration() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"FrameDuration\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mFrameDuration.get();
    }
    void SetErrorCode(int32_t value) override
    {
	    mErrorCode.set(value);
    }
    int32_t GetErrorCode() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"ErrorCode\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mErrorCode.get();
    }
    void SetEthernetChecksum(int32_t value) override
    {
	    mEthernetChecksum.set(value);
    }
    int32_t GetEthernetChecksum() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"EthernetChecksum\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mEthernetChecksum.get();
    }
    void SetPacketData(const std::vector<uint8_t>& value) override
    {
	    mPacketData.set(value);
    }
    void SetPacketData(const uint8_t* value, size_t size) override
    {
	    mPacketData.set(value, size);
    }
    const std::vector<uint8_t>& GetPacketData() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"PacketData\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mPacketData.get();
    }

  private:
    rti1516e::HLAinteger16LE mClientIndex;
    rti1516e::HLAoctet mDir;
    rti1516e::HLAinteger64LE mOriginalTimeStamp;
    rti1516e::HLAoctet mSimulated;
    rti1516e::HLAinteger16LE mBusType;
    rti1516e::HLAinteger64LE mHardwareChannel;
    rti1516e::HLAinteger64LE mFrameDuration;
    rti1516e::HLAinteger32LE mErrorCode;
    rti1516e::HLAinteger32LE mEthernetChecksum;
    EthernetPacketDataEncoding mPacketData;
}; // class EthernetPacketErrorEncoding

class EthernetPacketErrorForwardedEncoding : public EthernetPacketErrorForwarded, public rti1516e::HLAfixedRecord
{
  public:
    EthernetPacketErrorForwardedEncoding() : rti1516e::HLAfixedRecord(1)
    {
      rti1516e::HLAfixedRecord::appendElementPointer(&mClientIndex, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mDir, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mOriginalTimeStamp, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mSimulated, 0);
      rti1516e::HLAfixedRecord::appendElementPointer(&mBusType, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mHardwareChannel, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mFrameDuration, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mErrorCode, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mEthernetChecksum, 1);
      rti1516e::HLAfixedRecord::appendElementPointer(&mPacketData, 1);
    }
    virtual ~EthernetPacketErrorForwardedEncoding()
    {
    }
    uint32_t getVersion() const override { return rti1516e::HLAfixedRecord::getVersion(); }
    void SetClientIndex(int16_t value) override
    {
	    mClientIndex.set(value);
    }
    int16_t GetClientIndex() const override
    {
      return mClientIndex.get();
    }
    void SetDir(DirMask value) override
    {
	    mDir.set(value);
    }
    DirMask GetDir() const override
    {
      return static_cast<DirMask>(mDir.get());
    }
    void SetOriginalTimeStamp(VTimeNS value) override
    {
	    mOriginalTimeStamp.set(value);
    }
    VTimeNS GetOriginalTimeStamp() const override
    {
      return mOriginalTimeStamp.get();
    }
    void SetSimulated(SimulatedFlag value) override
    {
	    mSimulated.set(value);
    }
    SimulatedFlag GetSimulated() const override
    {
      return static_cast<SimulatedFlag>(mSimulated.get());
    }
    void SetBusType(int16_t value) override
    {
	    mBusType.set(value);
    }
    int16_t GetBusType() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"BusType\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mBusType.get();
    }
    void SetHardwareChannel(int64_t value) override
    {
	    mHardwareChannel.set(value);
    }
    int64_t GetHardwareChannel() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"HardwareChannel\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mHardwareChannel.get();
    }
    void SetFrameDuration(VTimeNS value) override
    {
	    mFrameDuration.set(value);
    }
    VTimeNS GetFrameDuration() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"FrameDuration\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mFrameDuration.get();
    }
    void SetErrorCode(int32_t value) override
    {
	    mErrorCode.set(value);
    }
    int32_t GetErrorCode() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"ErrorCode\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mErrorCode.get();
    }
    void SetEthernetChecksum(int32_t value) override
    {
	    mEthernetChecksum.set(value);
    }
    int32_t GetEthernetChecksum() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"EthernetChecksum\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mEthernetChecksum.get();
    }
    void SetPacketData(const std::vector<uint8_t>& value) override
    {
	    mPacketData.set(value);
    }
    void SetPacketData(const uint8_t* value, size_t size) override
    {
	    mPacketData.set(value, size);
    }
    const std::vector<uint8_t>& GetPacketData() const override
    {
      if (getVersion() < 1)
      {
        throw rti1516e::EncoderException(L"field \"PacketData\" is not available in decoded record version " + std::to_wstring(getVersion()));
      }
      return mPacketData.get();
    }

  private:
    rti1516e::HLAinteger16LE mClientIndex;
    rti1516e::HLAoctet mDir;
    rti1516e::HLAinteger64LE mOriginalTimeStamp;
    rti1516e::HLAoctet mSimulated;
    rti1516e::HLAinteger16LE mBusType;
    rti1516e::HLAinteger64LE mHardwareChannel;
    rti1516e::HLAinteger64LE mFrameDuration;
    rti1516e::HLAinteger32LE mErrorCode;
    rti1516e::HLAinteger32LE mEthernetChecksum;
    EthernetPacketDataEncoding mPacketData;
}; // class EthernetPacketErrorForwardedEncoding

} // namespace NDistributedSimulation
} // namespace NRTFederateEncoding

