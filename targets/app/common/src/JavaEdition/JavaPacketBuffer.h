#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

static constexpr size_t kMaxFrameLength = 2097151;
static constexpr size_t kMaxStringBytes = 1u << 20;

class JavaProtocolError : public std::runtime_error {
public:
    explicit JavaProtocolError(const std::string& what) : std::runtime_error(what) {}
};

class JavaPacketWriter {
public:
    void writeVarInt(int32_t v);
    void writeVarLong(int64_t v);
    void writeU8(uint8_t v);
    void writeU16(uint16_t v);
    void writeI32(int32_t v);
    void writeI64(int64_t v);
    void writeUtf8(const std::string& s);
    void writeBytes(const uint8_t* p, size_t n);

    const std::vector<uint8_t>& bytes() const { return m_buf; }

private:
    std::vector<uint8_t> m_buf;
};


class JavaPacketReader {
public:
    JavaPacketReader(const uint8_t* data, size_t len);

    int32_t readVarInt();
    int64_t readVarLong();
    uint8_t readU8();
    uint16_t readU16();
    int32_t readI32();
    int64_t readI64();

    std::string readUtf8(size_t maxLen);

    void readBytes(uint8_t* buf, size_t n);
    void skipBytes(size_t n);

    size_t remaining() const { return m_len - m_pos; }

private:
    uint8_t readByte();

    const uint8_t* m_p;
    size_t m_len;
    size_t m_pos;
};


struct VarIntReadResult {
    int32_t value;
    int bytesRead;
    bool ok;
};

VarIntReadResult readVarIntFromStream(const std::function<int()>& nextByte);