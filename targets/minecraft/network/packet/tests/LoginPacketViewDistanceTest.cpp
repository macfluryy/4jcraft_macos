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
}

IPlatformFileIO& PlatformFileIO = g_stubFileIO;

namespace {

constexpr int kMin = PlayerChunkMap::MIN_VIEW_DISTANCE;
constexpr int kMax = PlayerChunkMap::MAX_VIEW_DISTANCE;
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

void writeUtfLike(DataOutputStream* dos, const std::wstring& value) {
    dos->writeShort(static_cast<short>(value.length()));
    dos->writeChars(value);
}

std::wstring readUtfLike(DataInputStream* dis) {
    short len = dis->readShort();
    std::wstring out;
    out.reserve(len < 0 ? 0 : static_cast<size_t>(len));
    for (short i = 0; i < len; ++i) {
        out.push_back(dis->readChar());
    }
    return out;
}

struct LoginWireFields {
    int clientVersion = 17;
    std::wstring userName = L"Steve";
    std::wstring generatorName = L"";
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
    short xzSize = 320;
    int hellScale = 8;
    int serverViewDistance = 0;
};

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
    dos.write(static_cast<unsigned int>(f.hellScale));
#endif
    dos.writeInt(f.serverViewDistance);

    return baos.toByteArray();
}

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
    f.hellScale = dis.read();
#endif
    f.serverViewDistance = dis.readInt();
    return f;
}

void checkRoundTrip(int rawValue) {
    const int expected = clampViewDistance(rawValue);

    LoginWireFields in;
    in.serverViewDistance = expected;

    std::vector<uint8_t> bytes = writeLoginWire(in);
    LoginWireFields out = readLoginWire(bytes);

    char buf[200];
    std::snprintf(buf, sizeof(buf),
                  "raw=%d clamp=%d -> read serverViewDistance=%d", rawValue,
                  expected, out.serverViewDistance);
    check(out.serverViewDistance == expected, "viewdistance-roundtrip", buf);

    std::snprintf(buf, sizeof(buf), "raw=%d clamp=%d not in [%d,%d]", rawValue,
                  expected, kMin, kMax);
    check(expected >= kMin && expected <= kMax, "clamp-in-range", buf);

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

}

int main() {
    std::printf(
        "LoginPacket serverViewDistance round-trip property test "
        "(MIN=%d, MAX=%d)\n",
        kMin, kMax);

    const std::vector<int> edgeCases = {
        INT_MIN,  INT_MIN + 1, -100000, -1,       0,
        1,        2,
        kMin - 1, kMin,        kMin + 1,
        kMax - 1, kMax,        kMax + 1,
        100000,   INT_MAX - 1, INT_MAX,
    };
    for (int x : edgeCases) checkRoundTrip(x);

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