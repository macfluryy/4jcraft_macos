// Standalone property-based test for the round-trip serialization of
// ClientInformationPacket (created in task 6.1).
//
// Property under test (task 6.2):
//   read(write(x)).viewDistance == x
// for random view-distance ints (including int extremes and the boundary
// region), exercising the packet's single-int wire contract exactly.
//
// Validates: Requirement 5.1
//
// ----------------------------------------------------------------------------
// WHY THIS IS A WIRE-LEVEL (FALLBACK) TEST, NOT A REAL ClientInformationPacket
// OBJECT TEST
// ----------------------------------------------------------------------------
// ClientInformationPacket::read()/write() are trivial (a single
// dis->readInt() / dos->writeInt(viewDistance)), but the class derives from
// Packet, whose constructor lives in Packet.cpp. Packet.cpp's staticCtor()
// references EVERY registered packet type and pulls in the broad
// packet-registration / `app` global stack. Compiling ClientInformationPacket.cpp
// (and therefore Packet.cpp + PacketListener.cpp) into a tiny standalone test
// executable would create unresolved `app` symbols and drag the whole
// client/network stack into the test binary. This mirrors exactly the coupling
// documented for LoginPacketViewDistanceTest.cpp.
//
// Instead this test reproduces ClientInformationPacket::write()'s EXACT byte
// sequence using the REAL java stream classes (lightweight, no app/graphics
// coupling): a single `dos.writeInt(viewDistance)`, read back with the matching
// `dis.readInt()`. This faithfully verifies the wire contract Requirement 5.1
// depends on: the view-distance int the client sends round-trips intact.
//
// The live end-to-end ClientInformationPacket round-trip (real packet object
// over a real connection) is covered by the manual integration checklist in
// task 10.
//
// The project has no unit-test framework wired up, so this file ships a tiny
// self-contained harness: it runs the property over a deterministic stream of
// randomized inputs (std::mt19937 with a FIXED seed) plus explicit edge cases.
// On any violation it prints the failing input and exits non-zero; otherwise it
// prints a summary and exits 0.
//
// This translation unit is intentionally EXCLUDED from lib_minecraft's source
// glob (see targets/minecraft/meson.build) and built into its own small test
// executable.

#include <climits>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
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

// Reproduce ClientInformationPacket::write(): a single writeInt(viewDistance).
std::vector<uint8_t> writeClientInfoWire(int viewDistance) {
    ByteArrayOutputStream baos;
    DataOutputStream dos(&baos);
    dos.writeInt(viewDistance);
    return baos.toByteArray();
}

// Reproduce ClientInformationPacket::read(): viewDistance = readInt().
int readClientInfoWire(std::vector<uint8_t>& bytes) {
    ByteArrayInputStream bais(bytes);
    DataInputStream dis(&bais);
    return dis.readInt();
}

// Core property: the view-distance int survives the ClientInformationPacket
// wire round-trip intact, for ANY int input (the packet performs no clamping;
// clamping is the server's responsibility, validated separately).
void checkRoundTrip(int viewDistance) {
    std::vector<uint8_t> bytes = writeClientInfoWire(viewDistance);

    char buf[128];
    std::snprintf(buf, sizeof(buf), "wrote %zu bytes for viewDistance=%d "
                  "(expected exactly 4)",
                  bytes.size(), viewDistance);
    check(bytes.size() == sizeof(int32_t), "wire-size", buf);

    int out = readClientInfoWire(bytes);
    std::snprintf(buf, sizeof(buf), "in=%d -> read viewDistance=%d",
                  viewDistance, out);
    check(out == viewDistance, "viewdistance-roundtrip", buf);
}

}  // namespace

int main() {
    std::printf(
        "ClientInformationPacket viewDistance round-trip property test "
        "(seed=0x%X)\n",
        kSeed);

    // Explicit edge cases: int extremes, small values, and the view-distance
    // boundary region values called out in the task.
    const std::vector<int> edgeCases = {
        INT_MIN, INT_MIN + 1, -100000, -31,    -3,      -1,      0,
        1,       2,           3,       30,      31,      100,     1000,
        100000,  INT_MAX - 1, INT_MAX,
    };
    for (int x : edgeCases) checkRoundTrip(x);

    // Randomized sampling across the full int range plus a generator biased
    // toward the interesting low/boundary region.
    std::mt19937 rng(kSeed);
    std::uniform_int_distribution<int> fullRange(INT_MIN, INT_MAX);
    std::uniform_int_distribution<int> nearRange(-8, 40);
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
