#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include "app/common/src/JavaEdition/JavaServerListStore.h"
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

constexpr unsigned int kSeed = 0x5103E5u;
constexpr int kIterations = 200;
constexpr uint16_t kDefaultPort = 25565;

int g_failures = 0;
int g_checks = 0;

void check(bool condition, const char* property, const std::string& detail) {
    ++g_checks;
    if (!condition) {
        ++g_failures;
        std::printf("  FAIL [%s] %s\n", property, detail.c_str());
    }
}

std::filesystem::path makeUniqueTempDir(const std::string& tag) {
    static unsigned long long counter = 0;
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::ostringstream name;
    name << "jsrv_store_test_" << tag << "_" << static_cast<unsigned long long>(now)
         << "_" << counter++;
    std::error_code ec;
    std::filesystem::path dir = std::filesystem::temp_directory_path(ec) / name.str();
    std::filesystem::create_directories(dir, ec);
    return dir;
}

File workingDirFile(const std::filesystem::path& dir) {
    return File(dir.wstring());
}

std::filesystem::path javaServersPath(const std::filesystem::path& dir) {
    return dir / "javaservers.txt";
}

void writeRawFile(const std::filesystem::path& path, const std::string& bytes) {
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    ofs.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

std::string readRawFile(const std::filesystem::path& path) {
    std::ifstream ifs(path, std::ios::binary);
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

std::string toAscii(const std::wstring& s) {
    std::string out;
    out.reserve(s.size());
    for (wchar_t wc : s) out.push_back(static_cast<char>(static_cast<unsigned int>(wc) & 0xff));
    return out;
}

std::wstring randomName(std::mt19937& rng) {
    std::uniform_int_distribution<int> lenDist(0, 24);
    std::uniform_int_distribution<int> chDist(0x20, 0x7E);
    const int len = lenDist(rng);
    std::wstring s;
    s.reserve(static_cast<size_t>(len));
    for (int i = 0; i < len; ++i) s.push_back(static_cast<wchar_t>(chDist(rng)));
    return s;
}

std::string randomHost(std::mt19937& rng) {
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789.-:";
    std::uniform_int_distribution<int> lenDist(1, 32);
    std::uniform_int_distribution<int> chDist(0, static_cast<int>(sizeof(alphabet) - 2));
    const int len = lenDist(rng);
    std::string host;
    host.reserve(static_cast<size_t>(len));
    for (int i = 0; i < len; ++i) host.push_back(alphabet[chDist(rng)]);
    return host;
}

uint16_t randomPort(std::mt19937& rng) {
    std::uniform_int_distribution<int> portDist(1, 65535);
    return static_cast<uint16_t>(portDist(rng));
}

std::vector<JavaServerEntry> randomEntries(std::mt19937& rng) {
    std::uniform_int_distribution<int> countDist(0, 12);
    const int n = countDist(rng);
    std::vector<JavaServerEntry> entries;
    entries.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        JavaServerEntry e;
        e.name = randomName(rng);
        e.host = randomHost(rng);
        e.port = randomPort(rng);
        entries.push_back(e);
    }
    return entries;
}

JavaServerList toList(const std::vector<JavaServerEntry>& entries) {
    JavaServerList list;
    for (const auto& e : entries) list.add(e);
    return list;
}

std::string entryDesc(const JavaServerEntry& e) {
    return "name='" + toAscii(e.name) + "' host='" + e.host + "' port=" +
           std::to_string(e.port);
}

void runProperty6(std::mt19937& rng) {
    const std::filesystem::path dir = makeUniqueTempDir("rt");

    for (int iter = 0; iter < kIterations; ++iter) {
        const std::vector<JavaServerEntry> expected = randomEntries(rng);
        JavaServerList list = toList(expected);

        JavaServerListStore store(workingDirFile(dir));
        const bool saved = store.save(list);
        check(saved, "p6-save-ok", "save() returned false");

        JavaServerListStore::LoadResult loaded = store.load();

        check(!loaded.hadCorruptLines, "p6-no-corruption",
              "round-tripped a clean file but hadCorruptLines was set");
        check(loaded.list.size() == expected.size(), "p6-size",
              "size " + std::to_string(loaded.list.size()) + " != expected " +
                  std::to_string(expected.size()));

        const size_t common = std::min(loaded.list.size(), expected.size());
        for (size_t i = 0; i < common; ++i) {
            const JavaServerEntry& got = loaded.list.at(i);
            const JavaServerEntry& exp = expected[i];
            check(got.name == exp.name && got.host == exp.host &&
                      got.port == exp.port,
                  "p6-entry-roundtrip",
                  "index " + std::to_string(i) + " got {" + entryDesc(got) +
                      "} expected {" + entryDesc(exp) + "}");
        }
    }

    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

void runProperty7(std::mt19937& rng) {
    const std::filesystem::path dir = makeUniqueTempDir("corrupt");
    const std::filesystem::path file = javaServersPath(dir);

    for (int iter = 0; iter < kIterations; ++iter) {
        const std::vector<JavaServerEntry> valid = randomEntries(rng);

        std::vector<std::string> lines;
        lines.push_back("jsrv.count:" + std::to_string(valid.size()));
        for (size_t i = 0; i < valid.size(); ++i) {
            const std::string idx = std::to_string(i);
            lines.push_back("jsrv." + idx + ".name:" + toAscii(valid[i].name));
            lines.push_back("jsrv." + idx + ".host:" + valid[i].host);
            lines.push_back("jsrv." + idx + ".port:" + std::to_string(valid[i].port));
        }

        std::uniform_int_distribution<int> corruptCountDist(1, 5);
        std::uniform_int_distribution<int> kindDist(0, 2);
        const int corruptCount = corruptCountDist(rng);
        std::vector<std::string> corrupt;
        bool guaranteedFlag = false;
        for (int c = 0; c < corruptCount; ++c) {
            int kind = kindDist(rng);
            if (c == corruptCount - 1 && !guaranteedFlag) kind = 0;
            if (kind == 0) {
                static const char* junk[] = {"this is broken garbage",
                                             "not a key value pair",
                                             "###garbage###", "zzz"};
                corrupt.push_back(junk[rng() % 4]);
                guaranteedFlag = true;
            } else if (kind == 1) {
                corrupt.push_back("garbage" + std::to_string(rng() % 1000) +
                                  ":value" + std::to_string(rng() % 1000));
            } else {
                const int bigIdx = 1000 + static_cast<int>(rng() % 8000);
                corrupt.push_back("jsrv." + std::to_string(bigIdx) + ".host:");
                guaranteedFlag = true;
            }
        }

        std::vector<std::string> merged = lines;
        for (const std::string& cl : corrupt) {
            std::uniform_int_distribution<size_t> posDist(0, merged.size());
            merged.insert(merged.begin() + static_cast<std::ptrdiff_t>(posDist(rng)), cl);
        }

        std::string content;
        for (const std::string& l : merged) {
            content += l;
            content += '\n';
        }
        writeRawFile(file, content);

        JavaServerListStore store(workingDirFile(dir));
        JavaServerListStore::LoadResult loaded = store.load();

        check(loaded.hadCorruptLines, "p7-flag",
              "expected hadCorruptLines but it was false (corrupt lines=" +
                  std::to_string(corrupt.size()) + ")");
        check(loaded.list.size() == valid.size(), "p7-recover-all",
              "recovered " + std::to_string(loaded.list.size()) +
                  " entries, expected " + std::to_string(valid.size()));

        const size_t common = std::min(loaded.list.size(), valid.size());
        for (size_t i = 0; i < common; ++i) {
            const JavaServerEntry& got = loaded.list.at(i);
            const JavaServerEntry& exp = valid[i];
            check(got.name == exp.name && got.host == exp.host &&
                      got.port == exp.port,
                  "p7-entry-order",
                  "index " + std::to_string(i) + " got {" + entryDesc(got) +
                      "} expected {" + entryDesc(exp) + "}");
        }
    }

    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

void runProperty8(std::mt19937& rng) {
    const std::filesystem::path dir = makeUniqueTempDir("options");
    const std::filesystem::path optionsFile = dir / "options.txt";

    for (int iter = 0; iter < kIterations; ++iter) {
        std::ostringstream sentinel;
        sentinel << "music:" << (rng() % 100) / 100.0f << "\n";
        sentinel << "lastServer:" << "127.0.0." << (rng() % 256) << "\n";
        sentinel << "lastNickname:Player" << (rng() % 1000) << "\n";
        sentinel << "guiScale:" << (rng() % 4) << "\n";
        const std::string sentinelBytes = sentinel.str();
        writeRawFile(optionsFile, sentinelBytes);

        JavaServerListStore store(workingDirFile(dir));
        std::uniform_int_distribution<int> opCountDist(1, 6);
        const int ops = opCountDist(rng);
        for (int o = 0; o < ops; ++o) {
            if ((rng() & 1) == 0) {
                JavaServerList list = toList(randomEntries(rng));
                store.save(list);
            } else {
                (void)store.load();
            }
        }

        const std::string after = readRawFile(optionsFile);
        check(after == sentinelBytes, "p8-options-untouched",
              "options.txt changed after " + std::to_string(ops) +
                  " store ops (before " + std::to_string(sentinelBytes.size()) +
                  " bytes, after " + std::to_string(after.size()) + " bytes)");
    }

    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

void exampleMissingFile() {
    const std::filesystem::path dir = makeUniqueTempDir("missing");

    JavaServerListStore store(workingDirFile(dir));
    JavaServerListStore::LoadResult loaded = store.load();

    check(loaded.list.size() == 0, "ex-missing-empty",
          "missing file should yield empty list, got size " +
              std::to_string(loaded.list.size()));
    check(!loaded.hadCorruptLines, "ex-missing-no-corruption",
          "missing file should not flag corruption");

    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

void exampleExactKeyForm() {
    const std::filesystem::path dir = makeUniqueTempDir("keys");
    const std::filesystem::path file = javaServersPath(dir);

    JavaServerList list;
    JavaServerEntry e;
    e.name = L"Hypixel";
    e.host = "mc.hypixel.net";
    e.port = 25565;
    list.add(e);

    JavaServerListStore store(workingDirFile(dir));
    store.save(list);

    const std::string content = readRawFile(file);
    auto has = [&](const std::string& needle, const char* tag) {
        check(content.find(needle) != std::string::npos, tag,
              "saved file missing substring '" + needle + "'\n--- file ---\n" +
                  content);
    };
    has("jsrv.count:1", "ex-key-count");
    has("jsrv.0.name:Hypixel", "ex-key-name");
    has("jsrv.0.host:mc.hypixel.net", "ex-key-host");
    has("jsrv.0.port:25565", "ex-key-port");

    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

void exampleBadPortFallsBack() {
    struct Case {
        const char* portText;
        const char* tag;
    };
    const Case cases[] = {
        {"abc", "ex-port-nonnumeric"},
        {"99999", "ex-port-too-high"},
        {"65536", "ex-port-just-over"},
        {"0", "ex-port-zero"},
        {"-5", "ex-port-negative"},
        {"", "ex-port-empty"},
    };

    for (const Case& c : cases) {
        const std::filesystem::path dir = makeUniqueTempDir("badport");
        const std::filesystem::path file = javaServersPath(dir);

        std::string content;
        content += "jsrv.count:1\n";
        content += "jsrv.0.name:Test\n";
        content += "jsrv.0.host:example.com\n";
        content += std::string("jsrv.0.port:") + c.portText + "\n";
        writeRawFile(file, content);

        JavaServerListStore store(workingDirFile(dir));
        JavaServerListStore::LoadResult loaded = store.load();

        check(loaded.list.size() == 1, c.tag,
              std::string("port '") + c.portText +
                  "' should keep the entry, got size " +
                  std::to_string(loaded.list.size()));
        if (loaded.list.size() == 1) {
            const JavaServerEntry& got = loaded.list.at(0);
            check(got.port == kDefaultPort, c.tag,
                  std::string("port '") + c.portText +
                      "' should fall back to 25565, got " +
                      std::to_string(got.port));
            check(got.host == "example.com", c.tag,
                  std::string("port '") + c.portText +
                      "' entry host corrupted: '" + got.host + "'");
        }

        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }
}

}

int main() {
    std::printf("JavaServerListStore property/example tests (seed=0x%X, iters=%d)\n",
                kSeed, kIterations);

    std::mt19937 rng(kSeed);

    runProperty6(rng);
    runProperty7(rng);
    runProperty8(rng);

    exampleMissingFile();
    exampleExactKeyForm();
    exampleBadPortFallsBack();

    std::printf("Ran %d property/example checks.\n", g_checks);
    if (g_failures == 0) {
        std::printf("ALL PROPERTIES PASSED\n");
        return 0;
    }
    std::printf("%d PROPERTY CHECK(S) FAILED\n", g_failures);
    return 1;
}