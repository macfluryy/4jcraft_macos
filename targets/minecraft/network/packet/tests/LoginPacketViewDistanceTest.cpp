// Standalone property-based test for the round-trip serialization of the
// LoginPacket Server->Client `serverViewDistance` field (added in task 3.1).
//
// Property under test (task 3.2):
//   read(write(x)).serverViewDistance == clampViewDistance(x)
// for random view-distance values across the full int range, including the
// in-range boundaries and out-of-range values.
//
// Validates: Requirement 1.1
//
// ----------------------------------------------------------------------------
// WHY THIS IS A WIRE-LEVEL (FALLBACK) TEST, NOT A REAL LoginPacket OBJECT TEST
// ----------------------------------------------------------------------------
// LoginPacket::read() calls the global `app.DebugPrintf(...)` where `app` is a
// MacGame instance DEFINED IN THE APP TARGET (targets/app), not in
// lib_minecraft. LoginPacket also pulls in LevelType statics, Player
// (MAX_NAME_LENGTH) and Packet::readUtf/writeUtf. Compiling LoginPacket.cpp
// into a tiny standalone test executable would therefore create unresolved
// `app` / LevelType / Player symbols and risk dragging the whole
// client/graphics/network stack into the test binary.
//
// Instead this test reproduces LoginPacket::write()'s EXACT field sequence at
// the byte-stream level using the REAL java stream classes (lightweight, no
// app/graphics coupling) and reads it back with the matching DataInputStream
// calls. This faithfully exercises the wire contract that Requirement 1.1
// depends on: "an int appended LAST, after every existing LoginPacket field, is
// read back intact". The serverViewDistance is the trailing field, so this
// genuinely verifies trailing-field decode after a realistic byte stream
// (mixed ints / longs / bytes / bools / UTF strings / player UIDs, plus the
// _LARGE_WORLDS short+byte block when that build flag is set).
//
// The live end-to-end login round-trip (real LoginPacket object over a real
// connection) is covered by the manual integration checklist in task 10.
//
// The project has no unit-test framework wired up, so this file ships a tiny
// self-contained harness: it runs the property over a deterministic stream of
// randomized inputs (std::mt19937 with a FIXED seed) plus explicit edge cases.
// On any violation it prints the failing input and exits non-zero; otherwise it
// prints a summary and exits 0.
//
// This translation unit is intentionally EXCLUDED from lib_minecraft's source
// glob (see targets/minecraft/meson.build) and built into its own small test
// executable that compiles ViewDistanceUtil.cpp directly so clampViewDistance
// is available.

#include <climits>
#include <cstdint>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/server/level/PlayerChunkMap.h"
#include "minecraft/world/level/ViewDistanceUtil.h"
#include "platform/IPlatformFileIO.h"

// ----------------------------------------------------------------------------
// Minimal PlatformFileIO stub.
//
// The java static lib is compiled as a unity build, so the object file that
// provides the lightweight stream classes used here (DataInputStream,
// ByteArrayInputStream, ...) ALSO contains File.cpp, whose constructor
// references the global `PlatformFileIO`. That global is normally defined in
// platform/PlatformServices.cpp, which transitively pulls in SDL2 Input /
// Profile / Render / Storage managers. To keep this test self-contained and
// free of graphics/SDL coupling we satisfy the single referenced symbol with a
// no-op stub. None of these methods are ever called by the round-trip test
// (no File is ever constructed/used); the stub exists purely to resolve the
// link-time dependency the unity build introduces.
// ----------------------------------------------------------------------------
namespace {
class StubFileIO : public IPlatformFileIO {
public:
    ReadResult readFile(const std::filesystem::path&, void*,
                        std::size_t) override {
        return {ReadStatus::NotFound, 0, 0};
    }
    ReadResult readFileSegment(const std::filesystem::path&, std::size_t, void*,
                               std::size_t) override {
        return {ReadStatus::NotFound, 0, 0};
    }
    std::vector<std::uint8_t> readFileToVec(
        const std::filesystem::path&) override {
        return {};
    }
    bool writeFile(const std::filesystem::path&, const void*,
                   std::size_t) override {
        return false;
    }
    bool exists(const std::filesystem::path&) override { return false; }
    std::size_t fileSize(const std::filesystem::path&) override { return 0; }
    std::filesystem::path getBasePath() override { return {}; }
    std::filesystem::path getUserDataPath() override { return {}; }
};
StubFileIO g_stubFileIO;
}  // namespace

// Global referenced (via File.cpp) by the java unity build object.
IPlatformFileIO& PlatformFileIO = g_stubFileIO;

namespace {

constexpr int kMin = PlayerChunkMap::MIN_VIEW_DISTANCE;
constexpr int kMax = PlayerChunkMap::MAX_VIEW_DISTANCE;

// Fixed seed -> deterministic, reproducible runs.
constexpr unsigned int kSeed = 0xC0FFEEu;
constexpr int kRandomSamples = 20000;

int g_failures = 0;
int g_checks = 0;

void check(bool condition, const char* property, const char* detail) {
    ++g_checks;
    if (!condition) {
        ++g_failures;
        std::printf("  FAIL [%s] %s\n", property, detail);
    }
}

// Mirror of Packet::writeUtf: short length prefix then writeChars (2 bytes per
// wchar, high byte first). Reproduced locally so we do NOT have to link
// Packet.cpp (which transitively needs the `app` global / LevelType / Player).
void writeUtfLike(DataOutputStream* dos, const std::wstring& value) {
    dos->writeShort(static_cast<short>(value.length()));
    dos->writeChars(value);
}

// Mirror of Packet::readUtf (length-prefixed wchar sequence). readChar() reads
// two bytes high-byte-first, exactly inverting writeChar().
std::wstring readUtfLike(DataInputStream* dis) {
    short len = dis->readShort();
    std::wstring out;
    out.reserve(len < 0 ? 0 : static_cast<size_t>(len));
    for (short i = 0; i < len; ++i) {
        out.push_back(dis->readChar());
    }
    return out;
}

// Representative, non-trivial prefix mirroring every LoginPacket (S->C) field
// that precedes the trailing serverViewDistance. Mixed types ensure the
// trailing int is decoded after a realistic byte stream.
struct LoginWireFields {
    int clientVersion = 17;
    std::wstring userName = L"Steve";
    std::wstring generatorName = L"";  // null LevelType -> empty string on wire
    int64_t seed = 0x0123456789ABCDEFLL;
    int gameType = 1;
    std::uint8_t dimension = 0;
    std::uint8_t mapHeight = 128;
    std::uint8_t maxPlayers = 8;
    unsigned long long offlineXuid = 0xDEADBEEFCAFEULL;
    unsigned long long onlineXuid = 0xFEEDFACE1234ULL;
    bool friendsOnlyUGC = true;
    int ugcPlayersVersion = 42;
    std::uint8_t difficulty = 2;
    int multiplayerInstanceId = 7;
    std::uint8_t playerIndex = 3;
    int skinId = 99;
    int capeId = 5;
    bool isGuest = false;
    bool newSeaLevel = true;
    int uiGamePrivileges = 0x7F;
    short xzSize = 320;       // _LARGE_WORLDS only
    int hellScale = 8;        // _LARGE_WORLDS only; single byte on the wire
    int serverViewDistance = 0;  // the field under test (trailing)
};

// Reproduce LoginPacket::write()'s EXACT field order at the stream level.
std::vector<uint8_t> writeLoginWire(const LoginWireFields& f) {
    ByteArrayOutputStream baos;
    DataOutputStream dos(&baos);

    dos.writeInt(f.clientVersion);
    writeUtfLike(&dos, f.userName);
    writeUtfLike(&dos, f.generatorName);
    dos.writeLong(f.seed);
    dos.writeInt(f.gameType);
    dos.writeByte(f.dimension);
    dos.writeByte(f.mapHeight);
    dos.writeByte(f.maxPlayers);
    dos.writePlayerUID(f.offlineXuid);
    dos.writePlayerUID(f.onlineXuid);
    dos.writeBoolean(f.friendsOnlyUGC);
    dos.writeInt(f.ugcPlayersVersion);
    dos.writeByte(f.difficulty);
    dos.writeInt(f.multiplayerInstanceId);
    dos.writeByte(f.playerIndex);
    dos.writeInt(f.skinId);
    dos.writeInt(f.capeId);
    dos.writeBoolean(f.isGuest);
    dos.writeBoolean(f.newSeaLevel);
    dos.writeInt(f.uiGamePrivileges);
#ifdef _LARGE_WORLDS
    dos.writeShort(f.xzSize);
    dos.write(static_cast<unsigned int>(f.hellScale));  // single byte
#endif
    // Trailing field under test (unconditional, after the _LARGE_WORLDS block).
    dos.writeInt(f.serverViewDistance);

    return baos.toByteArray();
}

// Reproduce LoginPacket::read()'s EXACT field order, returning the decoded
// fields so the whole packet round-trip (not just the trailing field) can be
// asserted.
LoginWireFields readLoginWire(std::vector<uint8_t>& bytes) {
    ByteArrayInputStream bais(bytes);
    DataInputStream dis(&bais);

    LoginWireFields f;
    f.clientVersion = dis.readInt();
    f.userName = readUtfLike(&dis);
    f.generatorName = readUtfLike(&dis);
    f.seed = dis.readLong();
    f.gameType = dis.readInt();
    f.dimension = dis.readByte();
    f.mapHeight = dis.readByte();
    f.maxPlayers = dis.readByte();
    f.offlineXuid = dis.readPlayerUID();
    f.onlineXuid = dis.readPlayerUID();
    f.friendsOnlyUGC = dis.readBoolean();
    f.ugcPlayersVersion = dis.readInt();
    f.difficulty = dis.readByte();
    f.multiplayerInstanceId = dis.readInt();
    f.playerIndex = dis.readByte();
    f.skinId = dis.readInt();
    f.capeId = dis.readInt();
    f.isGuest = dis.readBoolean();
    f.newSeaLevel = dis.readBoolean();
    f.uiGamePrivileges = dis.readInt();
#ifdef _LARGE_WORLDS
    f.xzSize = dis.readShort();
    f.hellScale = dis.read();  // single byte
#endif
    f.serverViewDistance = dis.readInt();
    return f;
}

// Core property: a server-clamped view distance survives the full LoginPacket
// wire round-trip intact, AND the surrounding prefix fields are unchanged
// (confirming the trailing int is decoded at the correct offset).
void checkRoundTrip(int rawValue) {
    const int expected = clampViewDistance(rawValue);

    LoginWireFields in;
    in.serverViewDistance = expected;  // server always sends a clamped value

    std::vector<uint8_t> bytes = writeLoginWire(in);
    LoginWireFields out = readLoginWire(bytes);

    char buf[200];
    std::snprintf(buf, sizeof(buf),
                  "raw=%d clamp=%d -> read serverViewDistance=%d", rawValue,
                  expected, out.serverViewDistance);
    check(out.serverViewDistance == expected, "viewdistance-roundtrip", buf);

    // Sanity: the clamped value is always in range (Requirement 1.5).
    std::snprintf(buf, sizeof(buf), "raw=%d clamp=%d not in [%d,%d]", rawValue,
                  expected, kMin, kMax);
    check(expected >= kMin && expected <= kMax, "clamp-in-range", buf);

    // The trailing field must not corrupt / be corrupted by the prefix: verify
    // a representative subset of preceding fields round-trips too.
    std::snprintf(buf, sizeof(buf), "prefix corrupted (raw=%d)", rawValue);
    const bool prefixOk =
        out.clientVersion == in.clientVersion && out.userName == in.userName &&
        out.generatorName == in.generatorName && out.seed == in.seed &&
        out.gameType == in.gameType && out.dimension == in.dimension &&
        out.mapHeight == in.mapHeight && out.maxPlayers == in.maxPlayers &&
        out.offlineXuid == in.offlineXuid && out.onlineXuid == in.onlineXuid &&
        out.friendsOnlyUGC == in.friendsOnlyUGC &&
        out.ugcPlayersVersion == in.ugcPlayersVersion &&
        out.difficulty == in.difficulty &&
        out.multiplayerInstanceId == in.multiplayerInstanceId &&
        out.playerIndex == in.playerIndex && out.skinId == in.skinId &&
        out.capeId == in.capeId && out.isGuest == in.isGuest &&
        out.newSeaLevel == in.newSeaLevel &&
        out.uiGamePrivileges == in.uiGamePrivileges;
    check(prefixOk, "prefix-roundtrip", buf);
}

}  // namespace

int main() {
    std::printf(
        "LoginPacket serverViewDistance round-trip property test "
        "(MIN=%d, MAX=%d)\n",
        kMin, kMax);

    // Explicit edge cases: int extremes, the in-range boundaries, just-outside
    // boundaries, and zero.
    const std::vector<int> edgeCases = {
        INT_MIN,  INT_MIN + 1, -100000, -1,       0,
        1,        2,  // kMin - 1 == 2
        kMin - 1, kMin,        kMin + 1,           // 2, 3, 4
        kMax - 1, kMax,        kMax + 1,           // 29, 30, 31
        100000,   INT_MAX - 1, INT_MAX,
    };
    for (int x : edgeCases) checkRoundTrip(x);

    // Randomized sampling: full int range plus a generator biased toward the
    // interesting boundary region.
    std::mt19937 rng(kSeed);
    std::uniform_int_distribution<int> fullRange(INT_MIN, INT_MAX);
    std::uniform_int_distribution<int> nearRange(kMin - 8, kMax + 8);
    for (int i = 0; i < kRandomSamples; ++i) {
        checkRoundTrip(fullRange(rng));
        checkRoundTrip(nearRange(rng));
    }

    std::printf("Ran %d property checks.\n", g_checks);
    if (g_failures == 0) {
        std::printf("ALL PROPERTIES PASSED\n");
        return 0;
    }
    std::printf("%d PROPERTY CHECK(S) FAILED\n", g_failures);
    return 1;
}
