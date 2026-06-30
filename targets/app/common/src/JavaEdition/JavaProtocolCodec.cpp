#include "app/common/src/JavaEdition/JavaProtocolCodec.h"

#include <zlib.h>

#include "app/common/src/JavaEdition/JavaPacketBuffer.h"

namespace {

constexpr int kCodecMaxVarIntBytes = 5;

enum class VarIntScan { Ok, Incomplete, Malformed };

VarIntScan scanVarInt(const uint8_t* data, size_t len, int32_t& outValue, size_t& outBytes) {
    uint32_t result = 0;
    for (int i = 0; i < kCodecMaxVarIntBytes; ++i) {
        if (static_cast<size_t>(i) >= len) {
            return VarIntScan::Incomplete;
        }
        const uint8_t byte = data[i];
        result |= static_cast<uint32_t>(byte & 0x7F) << (7 * i);
        if ((byte & 0x80) == 0) {
            outValue = static_cast<int32_t>(result);
            outBytes = static_cast<size_t>(i) + 1;
            return VarIntScan::Ok;
        }
    }
    return VarIntScan::Malformed;
}

void appendVarInt(std::vector<uint8_t>& out, int32_t v) {
    uint32_t value = static_cast<uint32_t>(v);
    do {
        uint8_t temp = static_cast<uint8_t>(value & 0x7F);
        value >>= 7;
        if (value != 0) temp |= 0x80;
        out.push_back(temp);
    } while (value != 0);
}

size_t varIntSize(int32_t v) {
    uint32_t value = static_cast<uint32_t>(v);
    size_t n = 0;
    do {
        ++n;
        value >>= 7;
    } while (value != 0);
    return n;
}

}

void JavaProtocolCodec::enableCompression(int threshold) {
    if (threshold <= 0) {
        m_compressionEnabled = false;
        m_threshold = 0;
    } else {
        m_compressionEnabled = true;
        m_threshold = threshold;
    }
}

void JavaProtocolCodec::encodeFrame(int32_t packetId, const std::vector<uint8_t>& body,
                                    std::vector<uint8_t>& out) const {
    out.clear();

    std::vector<uint8_t> payload;
    payload.reserve(varIntSize(packetId) + body.size());
    appendVarInt(payload, packetId);
    payload.insert(payload.end(), body.begin(), body.end());

    if (!m_compressionEnabled) {
        appendVarInt(out, static_cast<int32_t>(payload.size()));
        out.insert(out.end(), payload.begin(), payload.end());
        return;
    }

    if (static_cast<int>(payload.size()) < m_threshold) {
        std::vector<uint8_t> inner;
        appendVarInt(inner, 0);
        inner.insert(inner.end(), payload.begin(), payload.end());

        appendVarInt(out, static_cast<int32_t>(inner.size()));
        out.insert(out.end(), inner.begin(), inner.end());
        return;
    }

    uLong bound = compressBound(static_cast<uLong>(payload.size()));
    std::vector<uint8_t> compressed(bound);
    uLongf compressedLen = bound;
    const int rc = compress(compressed.data(), &compressedLen, payload.data(),
                            static_cast<uLong>(payload.size()));
    if (rc != Z_OK) {
        std::vector<uint8_t> inner;
        appendVarInt(inner, 0);
        inner.insert(inner.end(), payload.begin(), payload.end());

        appendVarInt(out, static_cast<int32_t>(inner.size()));
        out.insert(out.end(), inner.begin(), inner.end());
        return;
    }
    compressed.resize(compressedLen);

    std::vector<uint8_t> inner;
    appendVarInt(inner, static_cast<int32_t>(payload.size()));
    inner.insert(inner.end(), compressed.begin(), compressed.end());

    appendVarInt(out, static_cast<int32_t>(inner.size()));
    out.insert(out.end(), inner.begin(), inner.end());
}

bool JavaProtocolCodec::splitPayload(const uint8_t* data, size_t len,
                                     DecodedFrame& outFrame) const {
    int32_t packetId = 0;
    size_t idBytes = 0;
    const VarIntScan scan = scanVarInt(data, len, packetId, idBytes);
    if (scan != VarIntScan::Ok) {
        return false;
    }
    outFrame.packetId = packetId;
    outFrame.body.assign(data + idBytes, data + len);
    return true;
}

JavaProtocolCodec::DecodeStatus JavaProtocolCodec::terminate(DecodeStatus status) {
    if (m_state != State::Terminated) {
        m_state = State::Terminated;
        m_terminalStatus = status;
    }
    return m_terminalStatus;
}

JavaProtocolCodec::DecodeStatus JavaProtocolCodec::tryDecodeFrame(std::vector<uint8_t>& inBuf,
                                                                  DecodedFrame& outFrame) {
    if (m_state == State::Terminated) {
        return m_terminalStatus;
    }

    int32_t declaredLen = 0;
    size_t headerBytes = 0;
    const VarIntScan lenScan = scanVarInt(inBuf.data(), inBuf.size(), declaredLen, headerBytes);
    if (lenScan == VarIntScan::Incomplete) {
        return DecodeStatus::NeedMoreData;
    }
    if (lenScan == VarIntScan::Malformed) {
        return terminate(DecodeStatus::ProtocolError);
    }

    if (declaredLen < 0 || static_cast<size_t>(declaredLen) > kMaxFrameLength) {
        return terminate(DecodeStatus::ProtocolError);
    }
    const size_t frameLen = static_cast<size_t>(declaredLen);

    if (inBuf.size() - headerBytes < frameLen) {
        return DecodeStatus::NeedMoreData;
    }

    const uint8_t* content = inBuf.data() + headerBytes;
    bool ok = false;

    if (!m_compressionEnabled) {
        ok = splitPayload(content, frameLen, outFrame);
    } else {
        int32_t dataLen = 0;
        size_t dataLenBytes = 0;
        const VarIntScan dScan = scanVarInt(content, frameLen, dataLen, dataLenBytes);
        if (dScan != VarIntScan::Ok) {
            return terminate(DecodeStatus::ProtocolError);
        }

        const uint8_t* payload = content + dataLenBytes;
        const size_t payloadLen = frameLen - dataLenBytes;

        if (dataLen == 0) {
            ok = splitPayload(payload, payloadLen, outFrame);
        } else {
            if (dataLen < 0 || static_cast<size_t>(dataLen) > kMaxFrameLength) {
                return terminate(DecodeStatus::ProtocolError);
            }
            const size_t inflatedSize = static_cast<size_t>(dataLen);

            std::vector<uint8_t> inflated(inflatedSize);
            uLongf actual = static_cast<uLongf>(inflatedSize);
            const int rc = uncompress(inflated.data(), &actual, payload,
                                      static_cast<uLong>(payloadLen));
            if (rc != Z_OK || static_cast<size_t>(actual) != inflatedSize) {
                return terminate(DecodeStatus::ProtocolError);
            }
            ok = splitPayload(inflated.data(), inflated.size(), outFrame);
        }
    }

    if (!ok) {
        return terminate(DecodeStatus::ProtocolError);
    }

    inBuf.erase(inBuf.begin(), inBuf.begin() + static_cast<std::ptrdiff_t>(headerBytes + frameLen));
    return DecodeStatus::FrameReady;
}

JavaProtocolCodec::DecodeStatus JavaProtocolCodec::notifyConnectionClosed(
    const std::vector<uint8_t>& inBuf) {
    (void)inBuf;
    return terminate(DecodeStatus::ConnectionBroken);
}