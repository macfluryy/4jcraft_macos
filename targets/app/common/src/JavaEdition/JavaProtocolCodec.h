#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

class JavaProtocolCodec {
public:
    enum class DecodeStatus {
        FrameReady,
        NeedMoreData,
        ProtocolError,
        ConnectionBroken,
    };

    struct DecodedFrame {
        int32_t packetId = 0;
        std::vector<uint8_t> body;
    };

    JavaProtocolCodec() = default;

    void encodeFrame(int32_t packetId, const std::vector<uint8_t>& body,
                     std::vector<uint8_t>& out) const;

    DecodeStatus tryDecodeFrame(std::vector<uint8_t>& inBuf, DecodedFrame& outFrame);
    DecodeStatus notifyConnectionClosed(const std::vector<uint8_t>& inBuf);

    void enableCompression(int threshold);

    bool compressionEnabled() const { return m_compressionEnabled; }
    int compressionThreshold() const { return m_threshold; }
    bool terminated() const { return m_state == State::Terminated; }

private:
    enum class State { Active, Terminated };

    DecodeStatus terminate(DecodeStatus status);

    bool splitPayload(const uint8_t* data, size_t len, DecodedFrame& outFrame) const;

    State m_state = State::Active;
    DecodeStatus m_terminalStatus = DecodeStatus::ProtocolError;
    bool m_compressionEnabled = false;
    int m_threshold = 0;
};