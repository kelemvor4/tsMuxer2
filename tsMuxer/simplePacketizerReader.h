#ifndef SIMPLE_PACKETIZER_READER_H_
#define SIMPLE_PACKETIZER_READER_H_

#include "abstractStreamReader.h"
#include "avCodecs.h"
#include "avPacket.h"
#include "tsPacket.h"

class SimplePacketizerReader : public AbstractStreamReader
{
   public:
    SimplePacketizerReader();
    ~SimplePacketizerReader() override = default;

    int readPacket(AVPacket& avPacket) override;
    int flushPacket(AVPacket& avPacket) override;
    void setBuffer(uint8_t* data, uint32_t dataLen, bool lastBlock = false) override;
    int64_t getProcessedSize() override;
    virtual CheckStreamRez checkStream(uint8_t* buffer, int len, ContainerType containerType, int containerDataType,
                                       int containerStreamIndex);

    /// Enhanced detection: runs checkStream(), then extracts codec-specific
    /// properties (channels, sample rate, resolution, etc.) into a
    /// StreamDiscoveryData struct.  Subclasses override fillDiscoveryData()
    /// to add codec-specific fields.
    StreamDiscoveryData probeStream(uint8_t* buffer, int len, ContainerType containerType, int containerDataType,
                                    int containerStreamIndex);

    virtual int getFreq() = 0;
    virtual int getAltFreq() { return getFreq(); }
    virtual uint8_t getChannels() = 0;
    void setStretch(const double value) { m_stretch = value; }
    void setMPLSInfo(const std::vector<MPLSPlayItem>& mplsInfo)
    {
        m_mplsInfo = mplsInfo;
        if (!m_mplsInfo.empty())
        {
            m_curMplsIndex = 0;
            m_lastMplsTime = (m_mplsInfo[0].OUT_time - m_mplsInfo[0].IN_time) * (INTERNAL_PTS_FREQ / 45000.0);
        }
        else
            m_curMplsIndex = -1;
    }

    // split point can be on any frame
    virtual bool isIFrame(AVPacket* packet) { return true; }

    /// Maximum frame size (bytes) that readPacket will accept.
    /// Override for codecs whose frames can exceed MAX_AV_PACKET_SIZE (e.g. FLAC).
    virtual int getMaxFrameSize() { return MAX_AV_PACKET_SIZE; }

   protected:
    virtual int getHeaderLen() = 0;  // return fixed frame header size at bytes
    virtual int decodeFrame(uint8_t* buff, uint8_t* end, int& skipBytes,
                            int& skipBeforeBytes) = 0;  // decode frame parameters. bitrate, channels for audio e.t.c
    virtual uint8_t* findFrame(uint8_t* buff, uint8_t* end) = 0;  // find forawrd nearest frame
    virtual double getFrameDuration() = 0;                        // frame duration at nano seconds
    virtual const std::string getStreamInfo() = 0;
    virtual void setTestMode(bool value) {}
    [[nodiscard]] virtual bool needMPLSCorrection() const { return true; }

    /// Called by probeStream() after a successful checkStream() to let each
    /// codec fill codec-specific fields in the discovery data.  Override in
    /// subclasses.  The base implementation fills sampleRate and channels.
    virtual void fillDiscoveryData(StreamDiscoveryData& data);

    virtual bool needSkipFrame(const AVPacket& packet) { return false; }

    // uint8_t* m_tmpBuffer;
    int m_curMplsIndex;
    double m_stretch;
    std::vector<uint8_t> m_tmpBuffer;
    int64_t m_processedBytes;
    uint64_t m_frameNum;
    bool m_needSync;
    double m_curPts;
    double m_lastMplsTime;
    double m_mplsOffset;
    double m_halfFrameLen;

    int m_containerDataType;
    int m_containerStreamIndex;
    std::vector<MPLSPlayItem> m_mplsInfo;
    void doMplsCorrection();
};

#endif
