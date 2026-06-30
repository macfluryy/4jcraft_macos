#include <cstdio>
#include <random>
#include <string>
#include <vector>

#include "app/common/src/JavaEdition/StatusJsonExtractor.h"

namespace {

constexpr unsigned int kSeed = 0x57A705u;
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

std::string narrow(const std::wstring& w) {
    std::string s;
    s.reserve(w.size());
    for (wchar_t c : w) s.push_back((c >= 32 && c < 127) ? static_cast<char>(c) : '?');
    return s;
}

std::string randomSafeMotd(std::mt19937& rng) {
    std::uniform_int_distribution<int> lenDist(0, 48);
    const int len = lenDist(rng);
    std::string s;
    s.reserve(static_cast<size_t>(len));
    for (int i = 0; i < len; ++i) {
        char c;
        do {
            c = static_cast<char>(std::uniform_int_distribution<int>(32, 126)(rng));
        } while (c == '"' || c == '\\');
        s.push_back(c);
    }
    return s;
}

std::string buildStatusJson(const std::string& motd, int online, int max,
                            bool withVersion, const std::string& version) {
    std::string json = "{";
    if (withVersion) {
        json += "\"version\":{\"name\":\"" + version + "\",\"protocol\":47},";
    }
    json += "\"players\":{\"max\":" + std::to_string(max) +
            ",\"online\":" + std::to_string(online) + "},";
    json += "\"description\":\"" + motd + "\"";
    json += "}";
    return json;
}

void checkModelRoundTrip(const std::string& motd, int online, int max, bool withVersion,
                         const std::string& version) {
    const std::string json = buildStatusJson(motd, online, max, withVersion, version);
    const StatusInfo info = extractStatus(json);

    check(info.valid, "p11-valid", "canonical JSON reported invalid: " + json);

    const std::string gotMotd = narrow(info.motd);
    check(gotMotd == motd, "p11-description",
          "motd mismatch: got '" + gotMotd + "' expected '" + motd + "' json=" + json);

    check(info.online == online, "p11-players-online",
          "online got " + std::to_string(info.online) + " expected " + std::to_string(online));
    check(info.max == max, "p11-players-max",
          "max got " + std::to_string(info.max) + " expected " + std::to_string(max));
}

StatusInfo runRobust(const std::string& input, const char* label) {
    const StatusInfo info = extractStatus(input);
    check(true, label, "");
    return info;
}

}

int main() {
    std::printf("StatusJsonExtractor property tests (seed=0x%X, iters=%d)\n", kSeed, kIterations);

    std::mt19937 rng(kSeed);

    checkModelRoundTrip("A Minecraft Server", 0, 20, false, "");
    checkModelRoundTrip("", 0, 0, false, "");
    checkModelRoundTrip("Hello World", 12345, 100000, true, "1.8.9");
    checkModelRoundTrip("spaces   and   tabs kept", 1, 1, true, "Paper 1.8");
    checkModelRoundTrip("symbols !#$%&()*+,-./:;<=>?@[]^_`{|}~", 7, 50, false, "");

    {
        const std::string json =
            "{\"description\":{\"text\":\"Welcome\"},\"players\":{\"online\":3,\"max\":10}}";
        const StatusInfo info = extractStatus(json);
        check(info.valid, "p11-object-desc-valid", json);
        check(narrow(info.motd) == "Welcome", "p11-object-desc-text",
              "got '" + narrow(info.motd) + "'");
        check(info.online == 3 && info.max == 10, "p11-object-desc-players",
              "online=" + std::to_string(info.online) + " max=" + std::to_string(info.max));
    }
    {
        const std::string json =
            "{\"description\":[{\"text\":\"foo\"},{\"text\":\"bar\"}],\"players\":{\"online\":1,\"max\":2}}";
        const StatusInfo info = extractStatus(json);
        check(info.valid && narrow(info.motd) == "foobar", "p11-array-desc",
              "got '" + narrow(info.motd) + "' valid=" + (info.valid ? "true" : "false"));
    }
    {
        const std::string json = "{\"description\":\"only motd\"}";
        const StatusInfo info = extractStatus(json);
        check(info.valid, "p11-desc-only-valid", json);
        check(info.online == -1 && info.max == -1, "p11-desc-only-defaults",
              "online=" + std::to_string(info.online) + " max=" + std::to_string(info.max));
    }

    {
        StatusInfo info = runRobust("", "p12-empty");
        check(!info.valid, "p12-empty-invalid", "empty input reported valid");

        const std::vector<std::string> invalids = {
            "not json at all",
            "[1,2,3]",
            "12345",
            "\"a string\"",
            "true",
            "null",
            "{",
            "{\"description\":",
            "{\"description\":\"unterminated",
            "{\"players\":{\"online\":}}",
            "{\"players\":{\"online\":1,}}",
            "{}extra",
            "{\"description\":\"x\"} junk",
            std::string("{\"description\":\"") + std::string(10, '\\'),
        };
        for (size_t i = 0; i < invalids.size(); ++i) {
            StatusInfo r = runRobust(invalids[i], "p12-invalid-no-crash");
            check(!r.valid, "p12-invalid-reported-invalid",
                  "input #" + std::to_string(i) + " '" + invalids[i] + "' reported valid");
        }

        check(!extractStatus("{}").valid, "p12-empty-object-invalid", "{} reported valid");
        {
            std::string deep = "{\"description\":";
            const int depth = kMaxStatusJsonDepth + 50;
            for (int i = 0; i < depth; ++i) deep += "[";
            for (int i = 0; i < depth; ++i) deep += "]";
            deep += "}";
            StatusInfo r = runRobust(deep, "p12-deep-nesting-no-crash");
            check(!r.valid, "p12-deep-nesting-invalid", "over-deep document reported valid");
        }
    }
    std::uniform_int_distribution<int> countDist(0, 100000);
    for (int i = 0; i < kIterations; ++i) {
        const std::string motd = randomSafeMotd(rng);
        const int online = countDist(rng);
        const int max = countDist(rng);
        const bool withVersion = (rng() & 1) != 0;
        std::string version;
        if (withVersion) {
            const int vlen = static_cast<int>(rng() % 12);
            for (int k = 0; k < vlen; ++k) {
                char c;
                do {
                    c = static_cast<char>(std::uniform_int_distribution<int>(32, 126)(rng));
                } while (c == '"' || c == '\\');
                version.push_back(c);
            }
        }
        checkModelRoundTrip(motd, online, max, withVersion, version);
        {
            std::string garbage;
            const int glen = static_cast<int>(rng() % 64);
            for (int k = 0; k < glen; ++k) garbage.push_back(static_cast<char>(rng() & 0xFF));
            runRobust(garbage, "p12-random-bytes-no-crash");
        }

        {
            const std::string full = buildStatusJson(motd, online, max, withVersion, version);
            if (full.size() > 1) {
                const size_t cut = 1 + (rng() % (full.size() - 1));
                runRobust(full.substr(0, cut), "p12-truncated-no-crash");
            }
        }

        {
            std::string corrupt = buildStatusJson(motd, online, max, withVersion, version);
            if (!corrupt.empty()) {
                const int hits = 1 + static_cast<int>(rng() % 4);
                for (int h = 0; h < hits; ++h) {
                    corrupt[rng() % corrupt.size()] = static_cast<char>(rng() & 0xFF);
                }
                runRobust(corrupt, "p12-corrupted-no-crash");
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