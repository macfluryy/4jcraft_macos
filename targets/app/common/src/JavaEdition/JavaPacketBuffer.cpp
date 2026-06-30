#include "app/common/src/JavaEdition/JavaPacketBuffer.h"

#include <cstring>

namespace {

constexpr int kMaxVarIntBytes = 5;
constexpr int kMaxVarLongBytes = 10;

}

void JavaPacketWriter::writeVarInt(int32_t v) {
    uint32_t value = static_cast<uint32_t>(v);
    do {
        uint8_t temp = static_cast<uint8_t>(value & 0x7F);
        value >>= 7;
        if (value != 0) temp |= 0x80;
        m_buf.push_back(temp);
    } while (value != 0);
}

void JavaPacketWriter::writeVarLong(int64_t v) {
    uint64_t value = static_cast<uint64_t>(v);
    do {
        uint8_t temp = static_cast<uint8_t>(value & 0x7F);
        value >>= 7;
        if (value != 0) temp |= 0x80;
        m_buf.push_back(temp);
    } while (value != 0);
}

void JavaPacketWriter::writeU8(uint8_t v) {
    m_buf.push_back(v);
}

void JavaPacketWriter::writeU16(uint16_t v) {
    m_buf.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    m_buf.push_back(static_cast<uint8_t>(v & 0xFF));
}

void JavaPacketWriter::writeI32(int32_t v) {
    uint32_t u = static_cast<uint32_t>(v);
    m_buf.push_back(static_cast<uint8_t>((u >> 24) & 0xFF));
    m_buf.push_back(static_cast<uint8_t>((u >> 16) & 0xFF));
    m_buf.push_back(static_cast<uint8_t>((u >> 8) & 0xFF));
    m_buf.push_back(static_cast<uint8_t>(u & 0xFF));
}

void JavaPacketWriter::writeI64(int64_t v) {
    uint64_t u = static_cast<uint64_t>(v);
    for (int shift = 56; shift >= 0; shift -= 8) {
        m_buf.push_back(static_cast<uint8_t>((u >> shift) & 0xFF));
    }
}

void JavaPacketWriter::writeUtf8(const std::string& s) {
    writeVarInt(static_cast<int32_t>(s.size()));
    writeBytes(reinterpret_cast<const uint8_t*>(s.data()), s.size());
}

void JavaPacketWriter::writeBytes(const uint8_t* p, size_t n) {
    if (n == 0) return;
    m_buf.insert(m_buf.end(), p, p + n);
}

JavaPacketReader::JavaPacketReader(const uint8_t* data, size_t len)
    : m_p(data), m_len(len), m_pos(0) {}

uint8_t JavaPacketReader::readByte() {
    if (m_pos >= m_len) {
        throw JavaProtocolError("JavaPacketReader: read past end of buffer");
    }
    return m_p[m_pos++];
}

int32_t JavaPacketReader::readVarInt() {
    uint32_t result = 0;
    for (int i = 0; i < kMaxVarIntBytes; ++i) {
        uint8_t byte = readByte();
        result |= static_cast<uint32_t>(byte & 0x7F) << (7 * i);
        if ((byte & 0x80) == 0) {
            return static_cast<int32_t>(result);
        }
    }
    throw JavaProtocolError("JavaPacketReader: VarInt exceeds 5 bytes");
}

int64_t JavaPacketReader::readVarLong() {
    uint64_t result = 0;
    for (int i = 0; i < kMaxVarLongBytes; ++i) {
        uint8_t byte = readByte();
        result |= static_cast<uint64_t>(byte & 0x7F) << (7 * i);
        if ((byte & 0x80) == 0) {
            return static_cast<int64_t>(result);
        }
    }
    throw JavaProtocolError("JavaPacketReader: VarLong exceeds 10 bytes");
}

uint8_t JavaPacketReader::readU8() {
    return readByte();
}

uint16_t JavaPacketReader::readU16() {
    uint16_t hi = readByte();
    uint16_t lo = readByte();
    return static_cast<uint16_t>((hi << 8) | lo);
}

int32_t JavaPacketReader::readI32() {
    uint32_t b0 = readByte();
    uint32_t b1 = readByte();
    uint32_t b2 = readByte();
    uint32_t b3 = readByte();
    return static_cast<int32_t>((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);
}

int64_t JavaPacketReader::readI64() {
    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result = (result << 8) | static_cast<uint64_t>(readByte());
    }
    return static_cast<int64_t>(result);
}

std::string JavaPacketReader::readUtf8(size_t maxLen) {
    const size_t cap = maxLen < kMaxStringBytes ? maxLen : kMaxStringBytes;

    const int32_t declared = readVarInt();
    if (declared < 0) {
        throw JavaProtocolError("JavaPacketReader: negative string length");
    }
    const size_t length = static_cast<size_t>(declared);
    if (length > cap) {
        throw JavaProtocolError("JavaPacketReader: string length exceeds cap");
    }
    if (length > remaining()) {
        throw JavaProtocolError("JavaPacketReader: string runs past end of buffer");
    }
    std::string out(reinterpret_cast<const char*>(m_p + m_pos), length);
    m_pos += length;
    return out;
}

void JavaPacketReader::readBytes(uint8_t* buf, size_t n) {
    if (n > remaining()) {
        throw JavaProtocolError("JavaPacketReader: readBytes past end of buffer");
    }
    if (n > 0 && buf != nullptr) {
        memcpy(buf, m_p + m_pos, n);
    }
    m_pos += n;
}

void JavaPacketReader::skipBytes(size_t n) {
    if (n > remaining()) {
        throw JavaProtocolError("JavaPacketReader: skipBytes past end of buffer");
    }
    m_pos += n;
}

VarIntReadResult readVarIntFromStream(const std::function<int()>& nextByte) {
    uint32_t result = 0;
    for (int i = 0; i < kMaxVarIntBytes; ++i) {
        const int next = nextByte();
        if (next < 0) {
            return VarIntReadResult{0, 0, false};
        }
        const uint8_t byte = static_cast<uint8_t>(next & 0xFF);
        result |= static_cast<uint32_t>(byte & 0x7F) << (7 * i);
        if ((byte & 0x80) == 0) {
            return VarIntReadResult{static_cast<int32_t>(result), i + 1, true};
        }
    }
    return VarIntReadResult{0, 0, false};
}