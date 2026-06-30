#include <cstdint>
#include <cstdio>
#include <functional>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "app/common/src/JavaEdition/JavaPacketBuffer.h"

namespace {

constexpr unsigned int kSeed = 0xB0FFECu;
constexpr int kIterations = 2000;

int g_failures = 0;
int g_checks = 0;

void check(bool condition, const char* property, const std::string& detail) {
    ++g_checks;
    if (!condition) {
        ++g_failures;
        std::printf("  FAIL [%s] %s\n", property, detail.c_str());
    }
}

std::vector<uint8_t> encodeVarInt(int32_t v) {
    JavaPacketWriter w;
    w.writeVarInt(v);
    return w.bytes();
}

void checkVarInt(int32_t v) {
    const std::vector<uint8_t> enc = encodeVarInt(v);

    check(!enc.empty() && enc.size() <= 5, "varint-length",
          "v=" + std::to_string(v) + " encoded length=" + std::to_string(enc.size()));

    if (v < 0) {
        check(enc.size() == 5, "varint-negative-5-bytes",
              "negative v=" + std::to_string(v) + " encoded length=" + std::to_string(enc.size()));
    }

    bool bitsOk = true;
    for (size_t i = 0; i + 1 < enc.size(); ++i) {
        if ((enc[i] & 0x80) == 0) bitsOk = false;
    }
    if (!enc.empty() && (enc.back() & 0x80) != 0) bitsOk = false;
    check(bitsOk, "varint-continuation-bits",
          "v=" + std::to_string(v) + " has malformed MSB continuation flags");

    uint32_t acc = 0;
    for (size_t i = 0; i < enc.size(); ++i) {
        acc |= static_cast<uint32_t>(enc[i] & 0x7F) << (7 * i);
    }
    check(static_cast<int32_t>(acc) == v, "varint-7bit-reconstruct",
          "v=" + std::to_string(v) + " reconstructed=" + std::to_string(static_cast<int32_t>(acc)));

    JavaPacketReader r(enc.data(), enc.size());
    int32_t got = 0;
    bool threw = false;
    try {
        got = r.readVarInt();
    } catch (const JavaProtocolError&) {
        threw = true;
    }
    check(!threw && got == v, "varint-roundtrip",
          "v=" + std::to_string(v) + " read=" + std::to_string(got) +
              (threw ? " (threw)" : ""));
}

enum class FieldType { VarInt, U16, I64, Utf8 };

struct Field {
    FieldType type;
    int32_t vi;
    uint16_t u16;
    int64_t i64;
    std::string utf8;
};

std::string randomUtf8(std::mt19937& rng) {
    std::uniform_int_distribution<int> lenDist(0, 40);
    std::uniform_int_distribution<int> chDist(32, 126);
    const int len = lenDist(rng);
    std::string s;
    for (int i = 0; i < len; ++i) s.push_back(static_cast<char>(chDist(rng)));
    return s;
}

void checkMixedRoundTrip(std::mt19937& rng) {
    std::uniform_int_distribution<int> countDist(1, 12);
    std::uniform_int_distribution<int> typeDist(0, 3);
    const int n = countDist(rng);

    std::vector<Field> fields;
    JavaPacketWriter w;
    for (int i = 0; i < n; ++i) {
        Field f{};
        f.type = static_cast<FieldType>(typeDist(rng));
        switch (f.type) {
            case FieldType::VarInt:
                f.vi = static_cast<int32_t>(rng());
                w.writeVarInt(f.vi);
                break;
            case FieldType::U16:
                f.u16 = static_cast<uint16_t>(rng() & 0xFFFF);
                w.writeU16(f.u16);
                break;
            case FieldType::I64:
                f.i64 = static_cast<int64_t>((static_cast<uint64_t>(rng()) << 32) | rng());
                w.writeI64(f.i64);
                break;
            case FieldType::Utf8:
                f.utf8 = randomUtf8(rng);
                w.writeUtf8(f.utf8);
                break;
        }
        fields.push_back(f);
    }

    JavaPacketReader r(w.bytes().data(), w.bytes().size());
    bool ok = true;
    std::string detail;
    try {
        for (const Field& f : fields) {
            switch (f.type) {
                case FieldType::VarInt: {
                    int32_t g = r.readVarInt();
                    if (g != f.vi) { ok = false; detail = "varint mismatch"; }
                    break;
                }
                case FieldType::U16: {
                    uint16_t g = r.readU16();
                    if (g != f.u16) { ok = false; detail = "u16 mismatch"; }
                    break;
                }
                case FieldType::I64: {
                    int64_t g = r.readI64();
                    if (g != f.i64) { ok = false; detail = "i64 mismatch"; }
                    break;
                }
                case FieldType::Utf8: {
                    std::string g = r.readUtf8(kMaxStringBytes);
                    if (g != f.utf8) { ok = false; detail = "utf8 mismatch"; }
                    break;
                }
            }
            if (!ok) break;
        }
    } catch (const JavaProtocolError& e) {
        ok = false;
        detail = std::string("unexpected throw: ") + e.what();
    }
    check(ok && r.remaining() == 0, "mixed-fields-roundtrip",
          detail.empty() ? ("trailing bytes remaining=" + std::to_string(r.remaining())) : detail);
}

void checkFrameRoundTrip(int32_t id, const std::vector<uint8_t>& body) {
    JavaPacketWriter inner;
    inner.writeVarInt(id);
    inner.writeBytes(body.data(), body.size());
    const std::vector<uint8_t>& innerBytes = inner.bytes();

    JavaPacketWriter frame;
    frame.writeVarInt(static_cast<int32_t>(innerBytes.size()));
    frame.writeBytes(innerBytes.data(), innerBytes.size());
    const std::vector<uint8_t> wire = frame.bytes();

    JavaPacketReader r(wire.data(), wire.size());
    bool ok = true;
    std::string detail;
    try {
        const int32_t len = r.readVarInt();
        if (static_cast<size_t>(len) != innerBytes.size()) {
            ok = false;
            detail = "frame length mismatch";
        }
        if (ok && r.remaining() != static_cast<size_t>(len)) {
            ok = false;
            detail = "remaining != declared len";
        }
        const int32_t gotId = r.readVarInt();
        if (gotId != id) {
            ok = false;
            detail = "id mismatch got=" + std::to_string(gotId);
        }
        if (ok && r.remaining() != body.size()) {
            ok = false;
            detail = "body length mismatch remaining=" + std::to_string(r.remaining());
        }
    } catch (const JavaProtocolError& e) {
        ok = false;
        detail = std::string("unexpected throw: ") + e.what();
    }

    if (ok) {
        const size_t lenPrefix = encodeVarInt(static_cast<int32_t>(innerBytes.size())).size();
        const size_t idBytes = encodeVarInt(id).size();
        const size_t bodyStart = lenPrefix + idBytes;
        bool bodyOk = (wire.size() - bodyStart) == body.size();
        for (size_t i = 0; bodyOk && i < body.size(); ++i) {
            if (wire[bodyStart + i] != body[i]) bodyOk = false;
        }
        check(bodyOk, "frame-body-bytes", "decoded body bytes differ for id=" + std::to_string(id));
    }

    check(ok, "frame-roundtrip", detail.empty() ? "id=" + std::to_string(id) : detail);
}

void checkHandshakeFrame(const std::string& host, uint16_t port) {
    JavaPacketWriter body;
    body.writeVarInt(47);
    body.writeUtf8(host);
    body.writeU16(port);
    body.writeVarInt(1);

    JavaPacketWriter inner;
    inner.writeVarInt(0x00);
    inner.writeBytes(body.bytes().data(), body.bytes().size());

    JavaPacketWriter frame;
    frame.writeVarInt(static_cast<int32_t>(inner.bytes().size()));
    frame.writeBytes(inner.bytes().data(), inner.bytes().size());

    const std::vector<uint8_t> wire = frame.bytes();
    JavaPacketReader r(wire.data(), wire.size());
    bool ok = true;
    std::string detail;
    try {
        const int32_t len = r.readVarInt();
        if (r.remaining() != static_cast<size_t>(len)) { ok = false; detail = "len mismatch"; }
        const int32_t id = r.readVarInt();
        if (id != 0x00) { ok = false; detail = "id != 0"; }
        const int32_t proto = r.readVarInt();
        if (proto != 47) { ok = false; detail = "proto != 47"; }
        const std::string gotHost = r.readUtf8(kMaxStringBytes);
        if (gotHost != host) { ok = false; detail = "host mismatch"; }
        const uint16_t gotPort = r.readU16();
        if (gotPort != port) { ok = false; detail = "port mismatch"; }
        const int32_t next = r.readVarInt();
        if (next != 1) { ok = false; detail = "nextState != 1"; }
        if (ok && r.remaining() != 0) { ok = false; detail = "trailing bytes"; }
    } catch (const JavaProtocolError& e) {
        ok = false;
        detail = std::string("unexpected throw: ") + e.what();
    }
    check(ok, "handshake-frame", detail.empty() ? ("host='" + host + "'") : detail);
}

void checkOversizedString(int32_t declaredLen, size_t maxLen, bool expectThrow,
                          const char* label) {
    JavaPacketWriter w;
    w.writeVarInt(declaredLen);
    const std::vector<uint8_t> buf = w.bytes();

    JavaPacketReader r(buf.data(), buf.size());
    bool threw = false;
    try {
        r.readUtf8(maxLen);
    } catch (const JavaProtocolError&) {
        threw = true;
    }
    check(threw == expectThrow, label,
          "declaredLen=" + std::to_string(declaredLen) + " maxLen=" + std::to_string(maxLen) +
              " threw=" + (threw ? "true" : "false"));
}

bool truncatedThrows(const std::vector<uint8_t>& buf, const std::function<void(JavaPacketReader&)>& reads) {
    JavaPacketReader r(buf.data(), buf.size());
    try {
        reads(r);
    } catch (const JavaProtocolError&) {
        return true;
    }
    return false;
}

void checkRandomGarbage(const std::vector<uint8_t>& buf, std::mt19937& rng) {
    JavaPacketReader r(buf.data(), buf.size());
    std::uniform_int_distribution<int> opDist(0, 4);
    const int ops = 1 + static_cast<int>(rng() % 10);
    bool sane = true;
    try {
        for (int i = 0; i < ops; ++i) {
            switch (opDist(rng)) {
                case 0: r.readVarInt(); break;
                case 1: r.readVarLong(); break;
                case 2: r.readU16(); break;
                case 3: r.readI64(); break;
                default: r.readUtf8(kMaxStringBytes); break;
            }
        }
    } catch (const JavaProtocolError&) {
    } catch (...) {
        sane = false;
    }
    check(sane, "garbage-no-foreign-throw", "a non-JavaProtocolError exception escaped the reader");
}

}

int main() {
    std::printf("JavaPacketBuffer property tests (seed=0x%X, iters=%d)\n", kSeed, kIterations);

    std::mt19937 rng(kSeed);

    const std::vector<int32_t> viEdges = {
        0, 1, -1, 2, 127, 128, 255, 256,
        16383, 16384, 2097151, 2097152,
        std::numeric_limits<int32_t>::max(),
        std::numeric_limits<int32_t>::min(),
        -2147483647, 100, -100, 12345, -12345,
    };
    for (int32_t v : viEdges) checkVarInt(v);

    checkOversizedString(static_cast<int32_t>(kMaxStringBytes) + 1, kMaxStringBytes, true,
                         "p13-above-module-cap");
    checkOversizedString(std::numeric_limits<int32_t>::max(), kMaxStringBytes, true,
                         "p13-int-max-length");
    checkOversizedString(-1, kMaxStringBytes, true, "p13-negative-length");
    checkOversizedString(100, 10, true, "p13-above-caller-maxlen");
    checkOversizedString(5, kMaxStringBytes, true, "p13-within-cap-but-truncated");

    {
        JavaPacketWriter w;
        w.writeUtf8("hello");
        JavaPacketReader r(w.bytes().data(), w.bytes().size());
        bool ok = false;
        try {
            ok = (r.readUtf8(kMaxStringBytes) == "hello");
        } catch (const JavaProtocolError&) {
            ok = false;
        }
        check(ok, "p13-positive-control", "small in-bounds string failed to read back");
    }

    {
        std::vector<uint8_t> empty;
        check(truncatedThrows(empty, [](JavaPacketReader& r) { r.readVarInt(); }),
              "p14-empty-varint", "empty buffer readVarInt did not throw");
        check(truncatedThrows(empty, [](JavaPacketReader& r) { r.readU16(); }),
              "p14-empty-u16", "empty buffer readU16 did not throw");
        check(truncatedThrows(empty, [](JavaPacketReader& r) { r.readI64(); }),
              "p14-empty-i64", "empty buffer readI64 did not throw");

        std::vector<uint8_t> one = {0x12};
        check(truncatedThrows(one, [](JavaPacketReader& r) { r.readU16(); }),
              "p14-partial-u16", "1-byte readU16 did not throw");

        std::vector<uint8_t> three = {0x01, 0x02, 0x03};
        check(truncatedThrows(three, [](JavaPacketReader& r) { r.readI64(); }),
              "p14-partial-i64", "3-byte readI64 did not throw");

        std::vector<uint8_t> badVarInt = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
        check(truncatedThrows(badVarInt, [](JavaPacketReader& r) { r.readVarInt(); }),
              "p14-varint-too-long", "6-byte continuation VarInt did not throw");

        std::vector<uint8_t> unterminated = {0x80, 0x80};
        check(truncatedThrows(unterminated, [](JavaPacketReader& r) { r.readVarInt(); }),
              "p14-varint-unterminated", "unterminated VarInt did not throw");
    }

    std::uniform_int_distribution<uint32_t> u32(0, std::numeric_limits<uint32_t>::max());
    std::uniform_int_distribution<int> idDist(0, 0x7FFFFF);
    for (int i = 0; i < kIterations; ++i) {
        checkVarInt(static_cast<int32_t>(u32(rng)));
        checkMixedRoundTrip(rng);

        {
            std::vector<uint8_t> body;
            const int bodyLen = static_cast<int>(rng() % 64);
            for (int k = 0; k < bodyLen; ++k) body.push_back(static_cast<uint8_t>(rng() & 0xFF));
            checkFrameRoundTrip(idDist(rng), body);
        }

        {
            static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789.-";
            std::string host;
            const int hlen = 1 + static_cast<int>(rng() % 30);
            for (int k = 0; k < hlen; ++k) host.push_back(alphabet[rng() % (sizeof(alphabet) - 1)]);
            checkHandshakeFrame(host, static_cast<uint16_t>(rng() & 0xFFFF));
        }

        {
            std::vector<uint8_t> garbage;
            const int glen = static_cast<int>(rng() % 24);
            for (int k = 0; k < glen; ++k) garbage.push_back(static_cast<uint8_t>(rng() & 0xFF));
            checkRandomGarbage(garbage, rng);
        }

        {
            JavaPacketWriter body;
            body.writeVarInt(47);
            body.writeUtf8("example.com");
            body.writeU16(25565);
            body.writeVarInt(1);
            JavaPacketWriter inner;
            inner.writeVarInt(0x00);
            inner.writeBytes(body.bytes().data(), body.bytes().size());
            JavaPacketWriter frame;
            frame.writeVarInt(static_cast<int32_t>(inner.bytes().size()));
            frame.writeBytes(inner.bytes().data(), inner.bytes().size());

            const std::vector<uint8_t> full = frame.bytes();
            if (full.size() > 1) {
                const size_t cut = 1 + (rng() % (full.size() - 1));
                std::vector<uint8_t> truncated(full.begin(), full.begin() + static_cast<std::ptrdiff_t>(cut));
                const bool threw = truncatedThrows(truncated, [](JavaPacketReader& r) {
                    r.readVarInt();
                    r.readVarInt();
                    r.readVarInt();
                    (void)r.readUtf8(kMaxStringBytes);
                    r.readU16();
                    r.readVarInt();
                });
                check(threw, "p14-truncated-frame",
                      "truncated frame (cut=" + std::to_string(cut) + "/" +
                          std::to_string(full.size()) + ") did not throw");
            }
        }
    }

    std::printf("Ran %d property checks.\n", g_checks);
    if (g_failures == 0) {
        std::printf("ALL PROPERTIES PASSED\n");
        return 0;
    }
    std::printf("%d PROPERTY CHECK(S) FAILED\n", g_failures);
    return 1;
}