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

std::vector<uint8_t> writeClientInfoWire(int viewDistance) {
    ByteArrayOutputStream baos;
    DataOutputStream dos(&baos);
    dos.writeInt(viewDistance);
    return baos.toByteArray();
}

int readClientInfoWire(std::vector<uint8_t>& bytes) {
    ByteArrayInputStream bais(bytes);
    DataInputStream dis(&bais);
    return dis.readInt();
}

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

}

int main() {
    std::printf(
        "ClientInformationPacket viewDistance round-trip property test "
        "(seed=0x%X)\n",
        kSeed);

    const std::vector<int> edgeCases = {
        INT_MIN, INT_MIN + 1, -100000, -31,    -3,      -1,      0,
        1,       2,           3,       30,      31,      100,     1000,
        100000,  INT_MAX - 1, INT_MAX,
    };
    for (int x : edgeCases) checkRoundTrip(x);

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