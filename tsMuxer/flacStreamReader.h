#ifndef FLAC_STREAM_READER_H_
#define FLAC_STREAM_READER_H_

#include <vector>

#include "avPacket.h"
#include "simplePacketizerReader.h"

class FLACStreamReader final : public SimplePacketizerReader
{
    friend class MatroskaMuxer;

   public:
    FLACStreamReader();
    ~FLACStreamReader() override = default;

    int getMaxFrameSize() override
    {
        return DEFAULT_FILE_BLOCK_SIZE;
    }  // FLAC frames can be very large for multichannel hi-res audio
    int getFreq() override { return m_sampleRate; }
    uint8_t getChannels() override { return m_channels; }
    int getBitsPerSample() const { return m_bitsPerSample; }

    /// Store the raw CodecPrivate blob from the MKV container.
    /// Expected layout: "fLaC" (4 bytes) + metadata blocks (STREAMINFO first).
    void setCodecPrivate(const uint8_t* data, int size);

    /// Return the stored CodecPrivate (fLaC + STREAMINFO).
    const std::vector<uint8_t>& getCodecPrivate() const { return m_codecPrivate; }

    void applyDiscoveryData(const StreamDiscoveryData& data) override;

   protected:
    void fillDiscoveryData(StreamDiscoveryData& data) override;
    int getHeaderLen() override { return 4; }
    int decodeFrame(uint8_t* buff, uint8_t* end, int& skipBytes, int& skipBeforeBytes) override;
    uint8_t* findFrame(uint8_t* buff, uint8_t* end) override;
    double getFrameDuration() override;
    const CodecInfo& getCodecInfo() override;
    const std::string getStreamInfo() override;
    void setTestMode(bool value) override { m_testMode = value; }
    int writeAdditionData(uint8_t* dstBuffer, uint8_t* dstEnd, AVPacket& avPacket,
                          PriorityDataInfo* priorityData) override;

   private:
    bool m_testMode;
    bool m_firstFrame;

    int m_sampleRate;
    uint8_t m_channels;
    int m_bitsPerSample;
    int m_minBlockSize;
    int m_maxBlockSize;
    int m_curBlockSize;

    /// Raw CodecPrivate data (fLaC marker + STREAMINFO metadata block, possibly more).
    std::vector<uint8_t> m_codecPrivate;

    /// Parse a 34-byte STREAMINFO block to extract audio parameters.
    bool parseStreamInfo(const uint8_t* data, int size);

    /// Parse the FLAC frame header starting at \p buff and return the block size.
    /// Returns 0 on failure.
    int parseFrameHeader(const uint8_t* buff, const uint8_t* end) const;
};

#endif  // FLAC_STREAM_READER_H_
