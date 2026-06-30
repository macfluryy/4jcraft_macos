#include <cstdio>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "app/common/src/JavaEdition/JavaAddressParser.h"

namespace {

constexpr unsigned int kSeed = 0x4A11Au;
constexpr int kIterations = 2000;
constexpr uint16_t kTestDefaultPort = 25565;

int g_failures = 0;
int g_checks = 0;

void check(bool condition, const char* property, const std::string& detail) {
    ++g_checks;
    if (!condition) {
        ++g_failures;
        std::printf("  FAIL [%s] %s\n", property, detail.c_str());
    }
}

std::wstring widen(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}
std::string randomHost(std::mt19937& rng) {
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789.-";
    std::uniform_int_distribution<int> lenDist(1, 24);
    std::uniform_int_distribution<int> chDist(0, static_cast<int>(sizeof(alphabet) - 2));
    const int len = lenDist(rng);
    std::string host;
    host.reserve(static_cast<size_t>(len));
    for (int i = 0; i < len; ++i) host.push_back(alphabet[chDist(rng)]);
    return host;
}

std::string randomIpv6(std::mt19937& rng) {
    static const char alphabet[] = "0123456789abcdef:";
    std::uniform_int_distribution<int> lenDist(2, 30);
    std::uniform_int_distribution<int> chDist(0, static_cast<int>(sizeof(alphabet) - 2));
    const int len = lenDist(rng);
    std::string s;
    s.reserve(static_cast<size_t>(len));
    for (int i = 0; i < len; ++i) s.push_back(alphabet[chDist(rng)]);
    return s;
}

void checkBareHost(const std::string& host) {
    const ParsedAddress r = parseAddress(widen(host));
    check(r.host == host, "bare-host-value",
          "input='" + host + "' -> host='" + r.host + "' (expected '" + host + "')");
    check(r.port == kTestDefaultPort, "bare-host-default-port",
          "input='" + host + "' -> port=" + std::to_string(r.port) + " (expected 25565)");
    check(r.hostEmpty == host.empty(), "bare-host-empty-flag",
          "input='" + host + "' -> hostEmpty=" + (r.hostEmpty ? "true" : "false"));
}

void checkHostPort(const std::string& host, uint16_t port) {
    const std::string input = host + ":" + std::to_string(port);
    const ParsedAddress r = parseAddress(widen(input));
    check(r.host == host, "hostport-host-value",
          "input='" + input + "' -> host='" + r.host + "' (expected '" + host + "')");
    check(r.port == port, "hostport-port-roundtrip",
          "input='" + input + "' -> port=" + std::to_string(r.port) +
              " (expected " + std::to_string(port) + ")");
    check(!r.hostEmpty, "hostport-host-not-empty",
          "input='" + input + "' flagged host empty unexpectedly");
}

void checkInvalidPort(const std::string& host, const std::string& portText) {
    const std::string input = host + ":" + portText;
    const ParsedAddress r = parseAddress(widen(input));
    check(r.port == kTestDefaultPort, "invalid-port-default",
          "input='" + input + "' -> port=" + std::to_string(r.port) + " (expected 25565)");
    check(!r.hostEmpty, "invalid-port-host-not-empty",
          "input='" + input + "' flagged host empty (host was '" + host + "')");
    check(r.host == host, "invalid-port-host-value",
          "input='" + input + "' -> host='" + r.host + "' (expected '" + host + "')");
}

void checkBracketedIpv6(const std::string& inner, uint16_t port, bool withPort) {
    const std::string input =
        withPort ? "[" + inner + "]:" + std::to_string(port) : "[" + inner + "]";
    const ParsedAddress r = parseAddress(widen(input));
    check(r.host == inner, "ipv6-host-value",
          "input='" + input + "' -> host='" + r.host + "' (expected '" + inner + "')");
    const uint16_t expectedPort = withPort ? port : kTestDefaultPort;
    check(r.port == expectedPort, "ipv6-port",
          "input='" + input + "' -> port=" + std::to_string(r.port) +
              " (expected " + std::to_string(expectedPort) + ")");
    check(r.hostEmpty == inner.empty(), "ipv6-host-empty-flag",
          "input='" + input + "' -> hostEmpty=" + (r.hostEmpty ? "true" : "false"));
}

}

int main() {
    std::printf("JavaAddressParser property tests (seed=0x%X, iters=%d)\n", kSeed, kIterations);
    {
        ParsedAddress r = parseAddress(L"");
        check(r.hostEmpty && r.host.empty() && r.port == kTestDefaultPort, "edge-empty",
              "empty input host='" + r.host + "' port=" + std::to_string(r.port));
        r = parseAddress(L"    ");
        check(r.hostEmpty && r.host.empty() && r.port == kTestDefaultPort, "edge-whitespace",
              "whitespace input host='" + r.host + "' port=" + std::to_string(r.port));

        r = parseAddress(L"  example.com  ");
        check(r.host == "example.com" && r.port == kTestDefaultPort && !r.hostEmpty,
              "edge-trim", "host='" + r.host + "' port=" + std::to_string(r.port));
    }
    checkBareHost("localhost");
    checkBareHost("mc.hypixel.net");
    checkHostPort("example.com", 1);
    checkHostPort("example.com", 25565);
    checkHostPort("example.com", 65535);
    checkHostPort("127.0.0.1", 25577);
    checkInvalidPort("example.com", "0");
    checkInvalidPort("example.com", "65536");
    checkInvalidPort("example.com", "99999");
    checkInvalidPort("example.com", "-5");
    checkInvalidPort("example.com", "abc");
    checkInvalidPort("example.com", "");
    checkBracketedIpv6("::1", 25565, false);
    checkBracketedIpv6("::1", 8080, true);
    checkBracketedIpv6("2001:db8::1", 25565, true);

    std::mt19937 rng(kSeed);
    std::uniform_int_distribution<int> portDist(1, 65535);
    std::uniform_int_distribution<int> badKind(0, 3);

    for (int i = 0; i < kIterations; ++i) {
        checkBareHost(randomHost(rng));
        checkHostPort(randomHost(rng), static_cast<uint16_t>(portDist(rng)));

        std::string bad;
        switch (badKind(rng)) {
            case 0: bad = "0"; break;
            case 1: bad = std::to_string(65536 + (rng() % 100000)); break;
            case 2: bad = "-" + std::to_string(1 + (rng() % 65535)); break;
            default: {
                static const char letters[] = "ghijklmnopqrstuvwxyz";
                int len = 1 + static_cast<int>(rng() % 5);
                for (int k = 0; k < len; ++k) bad.push_back(letters[rng() % 20]);
                break;
            }
        }
        checkInvalidPort(randomHost(rng), bad);

        const bool withPort = (i % 2) == 0;
        checkBracketedIpv6(randomIpv6(rng), static_cast<uint16_t>(portDist(rng)), withPort);
    }

    std::printf("Ran %d property checks.\n", g_checks);
    if (g_failures == 0) {
        std::printf("ALL PROPERTIES PASSED\n");
        return 0;
    }
    std::printf("%d PROPERTY CHECK(S) FAILED\n", g_failures);
    return 1;
}