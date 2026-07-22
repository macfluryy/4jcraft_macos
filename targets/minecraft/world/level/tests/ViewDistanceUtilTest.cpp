


























#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

#include "minecraft/server/level/PlayerChunkMap.h"
#include "minecraft/world/level/ViewDistanceUtil.h"

namespace {

constexpr int kMin = PlayerChunkMap::MIN_VIEW_DISTANCE;
constexpr int kMax = PlayerChunkMap::MAX_VIEW_DISTANCE;


constexpr unsigned int kSeed = 0xC0FFEEu;
constexpr int kRandomSamples = 50000;

int g_failures = 0;
int g_checks = 0;



void check(bool condition, const char* property, const char* detail) {
    ++g_checks;
    if (!condition) {
        ++g_failures;
        std::printf("  FAIL [%s] %s\n", property, detail);
    }
}



void checkClamp(int x) {
    const int c = clampViewDistance(x);

    char buf[128];
    std::snprintf(buf, sizeof(buf), "input=%d -> clamp=%d (expected in [%d,%d])",
                  x, c, kMin, kMax);
    check(c >= kMin && c <= kMax, "range-invariant", buf);

    const int cc = clampViewDistance(c);
    std::snprintf(buf, sizeof(buf), "input=%d clamp=%d clamp(clamp)=%d", x, c, cc);
    check(cc == c, "idempotency", buf);

    
    
    if (x >= kMin && x <= kMax) {
        std::snprintf(buf, sizeof(buf), "in-range input=%d should be identity, got %d", x, c);
        check(c == x, "identity-in-range", buf);
    } else if (x < kMin) {
        std::snprintf(buf, sizeof(buf), "below-min input=%d should clamp to %d, got %d", x, kMin, c);
        check(c == kMin, "clamp-to-min", buf);
    } else {
        std::snprintf(buf, sizeof(buf), "above-max input=%d should clamp to %d, got %d", x, kMax, c);
        check(c == kMax, "clamp-to-max", buf);
    }
}



void checkOptionRange(int option) {
    const int chunks = viewDistanceOptionToChunks(option);
    char buf[128];
    std::snprintf(buf, sizeof(buf), "option=%d -> chunks=%d (expected in [%d,%d])",
                  option, chunks, kMin, kMax);
    check(chunks >= kMin && chunks <= kMax, "option-range", buf);
}

void checkOptionMonotonic(int a, int b) {
    
    if (a > b) std::swap(a, b);
    const int fa = viewDistanceOptionToChunks(a);
    const int fb = viewDistanceOptionToChunks(b);
    char buf[160];
    std::snprintf(buf, sizeof(buf),
                  "a=%d b=%d -> f(a)=%d f(b)=%d (expected f(a) >= f(b))",
                  a, b, fa, fb);
    
    
    check(fa >= fb, "option-monotonic", buf);
}


void checkEffective(int server, int client) {
    const int eff = effectiveViewDistance(server, client);
    const int cs = clampViewDistance(server);
    const int cc = clampViewDistance(client);
    const int upper = std::min(cs, cc);

    char buf[200];
    std::snprintf(buf, sizeof(buf),
                  "srv=%d cli=%d -> eff=%d (expected %d <= eff <= min(%d,%d)=%d <= %d)",
                  server, client, eff, kMin, cs, cc, upper, kMax);
    check(eff >= kMin && eff <= upper && upper <= kMax, "effective-bounds", buf);

    
    
    const int eff2 = effectiveViewDistance(eff, eff);
    std::snprintf(buf, sizeof(buf), "eff=%d effectiveViewDistance(eff,eff)=%d", eff, eff2);
    check(eff2 == eff, "effective-idempotency", buf);
}

}  

int main() {
    std::printf("ViewDistanceUtil property tests (MIN=%d, MAX=%d, seed=0x%X)\n",
                kMin, kMax, kSeed);

    
    const std::vector<int> edgeCases = {
        INT_MIN, INT_MIN + 1, -100000, -1, 0, 1, 2,
        kMin - 1, kMin, kMin + 1,
        kMax - 1, kMax, kMax + 1,
        100000, INT_MAX - 1, INT_MAX,
    };

    
    for (int x : edgeCases) checkClamp(x);

    
    for (int s : edgeCases)
        for (int c : edgeCases) checkEffective(s, c);

    
    const std::vector<int> optionEdges = {
        INT_MIN, -5, -1, 0, 1, 2, 3, 4, 5, 10, 30, 31, 32, 64, INT_MAX,
    };
    for (int o : optionEdges) checkOptionRange(o);
    for (int a : optionEdges)
        for (int b : optionEdges) checkOptionMonotonic(a, b);

    
    std::mt19937 rng(kSeed);
    std::uniform_int_distribution<int> fullRange(INT_MIN, INT_MAX);
    
    std::uniform_int_distribution<int> nearRange(kMin - 8, kMax + 8);
    
    std::uniform_int_distribution<int> optRange(-4, 40);

    for (int i = 0; i < kRandomSamples; ++i) {
        const int x = fullRange(rng);
        checkClamp(x);
        checkClamp(nearRange(rng));

        const int s = (i % 2 == 0) ? fullRange(rng) : nearRange(rng);
        const int c = (i % 3 == 0) ? fullRange(rng) : nearRange(rng);
        checkEffective(s, c);

        const int oa = optRange(rng);
        const int ob = optRange(rng);
        checkOptionRange(oa);
        checkOptionMonotonic(oa, ob);
    }

    std::printf("Ran %d property checks.\n", g_checks);
    if (g_failures == 0) {
        std::printf("ALL PROPERTIES PASSED\n");
        return 0;
    }
    std::printf("%d PROPERTY CHECK(S) FAILED\n", g_failures);
    return 1;
}
