// Regression test: Java 1.8 scoreboard -> LCE scoreboard translation.
//
// Runs the real JavaServerProxy + JavaConnection between a scripted Java
// protocol-47 server and a scripted LCE client on loopback sockets in this
// process. No rendering, no GL, no game loop. Every emitted LCE scoreboard
// packet (206 SetObjective / 207 SetScore / 208 SetDisplayObjective) is
// asserted field-exact; "no packet" cases are proven with a sentinel update
// that must be the next scoreboard packet on the wire.
//
// Exits non-zero on the first failed assertion.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "app/common/src/JavaEdition/JavaServerProxy.h"
#include "platform/IPlatformFileIO.h"

// File IO stub: pulled in transitively via libjava's File; never used here.
namespace {
class StubFileIO : public IPlatformFileIO {
public:
    ReadResult readFile(const std::filesystem::path&, void*,
                        std::size_t) override {
        return {ReadStatus::NotFound, 0, 0};
    }
    ReadResult readFileSegment(const std::filesystem::path&, std::size_t,
                               void*, std::size_t) override {
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

constexpr uint16_t kJavaPort = 25971;

[[noreturn]] void fail(const char* what) {
    fprintf(stderr, "FAIL: %s\n", what);
    exit(1);
}

void check(bool ok, const char* what) {
    if (!ok) fail(what);
    fprintf(stderr, "ok: %s\n", what);
}

void sendAll(int fd, const void* data, size_t n) {
    const uint8_t* p = static_cast<const uint8_t*>(data);
    size_t off = 0;
    while (off < n) {
        ssize_t k = ::send(fd, p + off, n - off, 0);
        if (k <= 0) fail("socket send");
        off += static_cast<size_t>(k);
    }
}

void recvAll(int fd, void* data, size_t n) {
    uint8_t* p = static_cast<uint8_t*>(data);
    size_t off = 0;
    while (off < n) {
        ssize_t k = ::recv(fd, p + off, n - off, 0);
        if (k <= 0) fail("socket recv (timeout or close)");
        off += static_cast<size_t>(k);
    }
}

void setRecvTimeout(int fd, int seconds) {
    struct timeval tv { seconds, 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

// ---------------- Java protocol 47 encoding ----------------

void putVarInt(std::vector<uint8_t>& out, uint32_t v) {
    while (true) {
        uint8_t b = v & 0x7F;
        v >>= 7;
        if (v) {
            out.push_back(b | 0x80);
        } else {
            out.push_back(b);
            return;
        }
    }
}

void putStr(std::vector<uint8_t>& out, const std::string& s) {
    putVarInt(out, static_cast<uint32_t>(s.size()));
    out.insert(out.end(), s.begin(), s.end());
}

void putBE32(std::vector<uint8_t>& out, uint32_t v) {
    for (int i = 3; i >= 0; --i) out.push_back((v >> (8 * i)) & 0xFF);
}

void putBE64(std::vector<uint8_t>& out, uint64_t v) {
    for (int i = 7; i >= 0; --i) out.push_back((v >> (8 * i)) & 0xFF);
}

void putF32(std::vector<uint8_t>& out, float f) {
    uint32_t u;
    memcpy(&u, &f, 4);
    putBE32(out, u);
}

void putF64(std::vector<uint8_t>& out, double d) {
    uint64_t u;
    memcpy(&u, &d, 8);
    putBE64(out, u);
}

void sendJavaPacket(int fd, int id, const std::vector<uint8_t>& body) {
    std::vector<uint8_t> payload;
    putVarInt(payload, static_cast<uint32_t>(id));
    payload.insert(payload.end(), body.begin(), body.end());
    std::vector<uint8_t> framed;
    putVarInt(framed, static_cast<uint32_t>(payload.size()));
    framed.insert(framed.end(), payload.begin(), payload.end());
    sendAll(fd, framed.data(), framed.size());
}

// Java 0x3B ScoreboardObjective
void javaObjective(int fd, const std::string& name, uint8_t mode,
                   const std::string& display) {
    std::vector<uint8_t> b;
    putStr(b, name);
    b.push_back(mode);
    if (mode == 0 || mode == 2) {
        putStr(b, display);
        putStr(b, "integer");
    }
    sendJavaPacket(fd, 0x3B, b);
}

// Java 0x3C UpdateScore
void javaScore(int fd, const std::string& owner, uint8_t action,
               const std::string& objective, int32_t value) {
    std::vector<uint8_t> b;
    putStr(b, owner);
    b.push_back(action);
    putStr(b, objective);
    if (action != 1) putVarInt(b, static_cast<uint32_t>(value));
    sendJavaPacket(fd, 0x3C, b);
}

// Java 0x3D DisplayScoreboard
void javaDisplay(int fd, uint8_t slot, const std::string& name) {
    std::vector<uint8_t> b;
    b.push_back(slot);
    putStr(b, name);
    sendJavaPacket(fd, 0x3D, b);
}

// Java 0x3E Teams
void javaTeam(int fd, const std::string& name, uint8_t mode,
              const std::string& display, const std::string& prefix,
              const std::string& suffix,
              const std::vector<std::string>& players) {
    std::vector<uint8_t> b;
    putStr(b, name);
    b.push_back(mode);
    if (mode == 0 || mode == 2) {
        putStr(b, display);
        putStr(b, prefix);
        putStr(b, suffix);
        b.push_back(1);          // friendly fire on
        putStr(b, "always");     // name tag visibility
        b.push_back(0);          // color
    }
    if (mode == 0 || mode == 3 || mode == 4) {
        putVarInt(b, static_cast<uint32_t>(players.size()));
        for (const std::string& p : players) putStr(b, p);
    }
    sendJavaPacket(fd, 0x3E, b);
}

// ---------------- scripted Java server ----------------

int g_javaFd = -1;

void javaReadPacket(int fd, std::vector<uint8_t>& out) {
    uint32_t len = 0;
    int shift = 0;
    while (true) {
        uint8_t b;
        recvAll(fd, &b, 1);
        len |= static_cast<uint32_t>(b & 0x7F) << shift;
        if (!(b & 0x80)) break;
        shift += 7;
    }
    out.resize(len);
    if (len) recvAll(fd, out.data(), len);
}

void javaServerThread(int listenFd, std::atomic<bool>* ready) {
    int fd = ::accept(listenFd, nullptr, nullptr);
    if (fd < 0) fail("java accept");
    setRecvTimeout(fd, 10);
    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

    std::vector<uint8_t> tmp;
    javaReadPacket(fd, tmp);  // handshake
    javaReadPacket(fd, tmp);  // login start

    std::vector<uint8_t> b;
    putStr(b, "11111111-2222-3333-4444-555555555555");
    putStr(b, "Tester");
    sendJavaPacket(fd, 0x02, b);  // login success

    b.clear();  // JoinGame
    putBE32(b, 999);
    b.push_back(0);
    b.push_back(0);
    b.push_back(1);
    b.push_back(20);
    putStr(b, "default");
    b.push_back(0);
    sendJavaPacket(fd, 0x01, b);

    b.clear();  // PlayerPositionAndLook
    putF64(b, 0.0);
    putF64(b, 65.0);
    putF64(b, 0.0);
    putF32(b, 0.0);
    putF32(b, 0.0);
    b.push_back(0);
    sendJavaPacket(fd, 0x08, b);

    g_javaFd = fd;
    ready->store(true);
    while (true) {
        uint8_t buf[4096];
        ssize_t k = ::recv(fd, buf, sizeof(buf), 0);
        if (k <= 0) return;
    }
}

// ---------------- scripted LCE client ----------------

struct LceScorePacket {
    uint8_t id = 0;          // 206 / 207 / 208 / 209
    std::wstring a;          // 206/209: name   207: owner       208: name
    std::wstring b;          // 206/209: display  207: objective
    int32_t value = 0;       // 207 value, 209 options
    uint8_t method = 0;      // 206/207/209 method, 208 slot
    std::wstring prefix;     // 209
    std::wstring suffix;     // 209
    std::vector<std::wstring> players;  // 209
};

struct LceReader {
    int fd;

    uint8_t u8() {
        uint8_t v;
        recvAll(fd, &v, 1);
        return v;
    }
    uint16_t u16() {
        uint8_t b[2];
        recvAll(fd, b, 2);
        return static_cast<uint16_t>((b[0] << 8) | b[1]);
    }
    uint32_t u32() {
        uint8_t b[4];
        recvAll(fd, b, 4);
        return (static_cast<uint32_t>(b[0]) << 24) | (b[1] << 16) |
               (b[2] << 8) | b[3];
    }
    void skip(size_t n) {
        std::vector<uint8_t> junk(n);
        if (n) recvAll(fd, junk.data(), n);
    }
    std::wstring utf() {
        const uint16_t n = u16();
        std::wstring s;
        s.reserve(n);
        for (uint16_t i = 0; i < n; ++i)
            s.push_back(static_cast<wchar_t>(u16()));
        return s;
    }

    void waitFor(uint8_t wanted) {
        while (consumeOne().id != wanted) {
        }
    }

    LceScorePacket nextScorePacket() {
        while (true) {
            LceScorePacket p = consumeOne();
            if (p.id >= 206 && p.id <= 209) return p;
        }
    }

    // Consumes one LCE packet; fills LceScorePacket for scoreboard ids, else
    // returns just the id. Unknown id = failure.
    LceScorePacket consumeOne() {
        LceScorePacket out;
        const uint8_t id = u8();
        out.id = id;
        switch (id) {
            case 1:  // Login
                skip(4);
                utf();
                utf();
                skip(8 + 4 + 1 + 1 + 1);
                skip(8 + 8 + 1 + 4 + 1 + 4 + 1 + 4 + 4 + 1 + 1 + 4);
                skip(2 + 1 + 4);
                break;
            case 6:   skip(12); break;
            case 202: skip(9);  break;
            case 13:  skip(41); break;
            case 4:   skip(16); break;
            case 51: {
                skip(1 + 4 + 2 + 4 + 1 + 1 + 1);
                const uint32_t size = u32() & 0x3FFFFFFF;
                skip(size);
                break;
            }
            case 206:
                out.a = utf();
                out.b = utf();
                out.method = u8();
                break;
            case 207:
                out.a = utf();
                out.method = u8();
                if (out.method != 1) {
                    out.b = utf();
                    out.value = static_cast<int32_t>(u32());
                }
                break;
            case 208:
                out.method = u8();  // slot
                out.a = utf();
                break;
            case 209:  // SetPlayerTeamPacket
                out.a = utf();
                out.method = u8();
                if (out.method == 0 || out.method == 2) {
                    out.b = utf();
                    out.prefix = utf();
                    out.suffix = utf();
                    out.value = u8();  // options
                }
                if (out.method == 0 || out.method == 3 || out.method == 4) {
                    const uint16_t n = u16();
                    for (uint16_t i = 0; i < n; ++i)
                        out.players.push_back(utf());
                }
                break;
            default:
                fprintf(stderr, "unknown LCE packet id=%u\n", id);
                fail("unknown LCE packet id");
        }
        return out;
    }
};

void expectObjective(LceReader& lce, const std::wstring& name,
                     const std::wstring& display, uint8_t method,
                     const char* what) {
    const LceScorePacket p = lce.nextScorePacket();
    if (p.id != 206 || p.a != name || p.b != display || p.method != method) {
        fprintf(stderr, "FAIL: %s (id=%u name='%ls' disp='%ls' method=%u)\n",
                what, p.id, p.a.c_str(), p.b.c_str(), p.method);
        exit(1);
    }
    fprintf(stderr, "ok: %s\n", what);
}

void expectScore(LceReader& lce, const std::wstring& owner, uint8_t method,
                 const std::wstring& objective, int32_t value,
                 const char* what) {
    const LceScorePacket p = lce.nextScorePacket();
    if (p.id != 207 || p.a != owner || p.method != method ||
        (method != 1 && (p.b != objective || p.value != value))) {
        fprintf(stderr, "FAIL: %s (id=%u owner='%ls' obj='%ls' v=%d m=%u)\n",
                what, p.id, p.a.c_str(), p.b.c_str(), p.value, p.method);
        exit(1);
    }
    fprintf(stderr, "ok: %s\n", what);
}

void expectTeam(LceReader& lce, const std::wstring& name, uint8_t method,
                const std::wstring& display, const std::wstring& prefix,
                const std::wstring& suffix,
                const std::vector<std::wstring>& players, const char* what) {
    const LceScorePacket p = lce.nextScorePacket();
    bool ok = p.id == 209 && p.a == name && p.method == method;
    if (ok && (method == 0 || method == 2))
        ok = p.b == display && p.prefix == prefix && p.suffix == suffix;
    if (ok && (method == 0 || method == 3 || method == 4))
        ok = p.players == players;
    if (!ok) {
        fprintf(stderr,
                "FAIL: %s (id=%u name='%ls' m=%u disp='%ls' pre='%ls' "
                "suf='%ls' players=%zu)\n",
                what, p.id, p.a.c_str(), p.method, p.b.c_str(),
                p.prefix.c_str(), p.suffix.c_str(), p.players.size());
        exit(1);
    }
    fprintf(stderr, "ok: %s\n", what);
}

void expectDisplay(LceReader& lce, uint8_t slot, const std::wstring& name,
                   const char* what) {
    const LceScorePacket p = lce.nextScorePacket();
    if (p.id != 208 || p.method != slot || p.a != name) {
        fprintf(stderr, "FAIL: %s (id=%u slot=%u name='%ls')\n", what, p.id,
                p.method, p.a.c_str());
        exit(1);
    }
    fprintf(stderr, "ok: %s\n", what);
}

}  // namespace

int main() {
    int listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(kJavaPort);
    if (::bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        fail("java bind (port in use?)");
    if (::listen(listenFd, 1) < 0) fail("java listen");

    JavaServerProxy proxy;
    const int lcePort = proxy.startListening();
    check(lcePort > 0, "proxy listening");
    proxy.startWorker("127.0.0.1", kJavaPort, "Tester", L"Tester");

    std::atomic<bool> javaReady{false};
    std::thread javaThread(javaServerThread, listenFd, &javaReady);

    int lceFd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in paddr{};
    paddr.sin_family = AF_INET;
    paddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    paddr.sin_port = htons(static_cast<uint16_t>(lcePort));
    if (::connect(lceFd, reinterpret_cast<sockaddr*>(&paddr), sizeof(paddr)) <
        0)
        fail("lce connect");
    setRecvTimeout(lceFd, 10);
    setsockopt(lceFd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

    {
        std::vector<uint8_t> pre;
        pre.push_back(2);
        pre.push_back(0);
        pre.push_back(3);   // netcode version (BE16 = 0x0003)
        pre.push_back(0);
        pre.push_back(0);   // login key length 0
        pre.push_back(0);   // friends only
        putBE32(pre, 0);    // ugc version
        pre.push_back(0);   // player count
        pre.insert(pre.end(), 14, 0);
        putBE32(pre, 0);
        pre.push_back(0);
        putBE32(pre, 0);
        sendAll(lceFd, pre.data(), pre.size());
    }

    LceReader lce{lceFd};
    check(lce.u8() == 2, "PreLogin response id");
    lce.skip(2);
    lce.utf();
    lce.skip(1 + 4 + 1 + 14 + 4 + 1 + 4);

    {
        std::vector<uint8_t> login;
        login.push_back(1);
        putBE32(login, 78);
        const std::wstring name = L"Tester", type = L"DEFAULT";
        login.push_back(0);
        login.push_back(static_cast<uint8_t>(name.size()));
        for (wchar_t c : name) {
            login.push_back(static_cast<uint8_t>(c >> 8));
            login.push_back(static_cast<uint8_t>(c & 0xFF));
        }
        login.push_back(0);
        login.push_back(static_cast<uint8_t>(type.size()));
        for (wchar_t c : type) {
            login.push_back(static_cast<uint8_t>(c >> 8));
            login.push_back(static_cast<uint8_t>(c & 0xFF));
        }
        putBE64(login, 0);
        putBE32(login, 0);
        login.push_back(0);
        login.push_back(255);
        login.push_back(8);
        putBE64(login, 0);
        putBE64(login, 0);
        login.push_back(0);
        putBE32(login, 0);
        login.push_back(1);
        putBE32(login, 0);
        login.push_back(0);
        putBE32(login, 0);
        putBE32(login, 0);
        login.push_back(0);
        login.push_back(0);
        putBE32(login, 0);
        login.push_back(0);
        login.push_back(54);  // xz size (BE16 = 0x0036)
        login.push_back(3);   // hell scale
        putBE32(login, 10);   // view distance
        sendAll(lceFd, login.data(), login.size());
    }

    while (!javaReady.load()) std::this_thread::yield();
    const int fd = g_javaFd;

    // Wait for the spawn handshake to finish before driving the scoreboard.
    lce.waitFor(13);
    check(true, "spawn handshake complete (MovePlayer received)");

    // 1. Objective create, with a 1.16-style RGB display name that must be
    //    normalized by the chat translator (#FF5555 -> nearest legacy = §c).
    javaObjective(fd, "obj1", 0,
                  "\xC2\xA7x\xC2\xA7"
                  "F\xC2\xA7"
                  "F\xC2\xA7"
                  "5\xC2\xA7"
                  "5\xC2\xA7"
                  "5\xC2\xA7"
                  "5Stats");
    expectObjective(lce, L"obj1", L"§cStats", 0,
                    "objective create (RGB display normalized)");

    // 2. Display slot sidebar.
    javaDisplay(fd, 1, "obj1");
    expectDisplay(lce, 1, L"obj1", "display slot sidebar");

    // 3. Score ordering: equal values arrive in insertion order.
    javaScore(fd, "alpha", 0, "obj1", 5);
    javaScore(fd, "beta", 0, "obj1", 5);
    expectScore(lce, L"alpha", 0, L"obj1", 5, "score create alpha");
    expectScore(lce, L"beta", 0, L"obj1", 5, "score create beta (order kept)");

    // 4. Suppression: duplicate score, duplicate display slot, score for an
    //    unknown objective, non-sidebar display slot. None may emit; the
    //    sentinel below must be the next scoreboard packet on the wire.
    javaScore(fd, "alpha", 0, "obj1", 5);   // duplicate value
    javaDisplay(fd, 1, "obj1");             // duplicate display
    javaScore(fd, "ghost", 0, "nosuch", 1); // unknown objective
    javaDisplay(fd, 0, "obj1");             // list slot: ignored
    javaDisplay(fd, 2, "obj1");             // below-name slot: ignored
    javaScore(fd, "alpha", 0, "obj1", 6);   // sentinel
    expectScore(lce, L"alpha", 0, L"obj1", 6,
                "duplicates/unknown/non-sidebar all suppressed");

    // 5. Objective display-name update.
    javaObjective(fd, "obj1", 2, "\xC2\xA7" "aNew");
    expectObjective(lce, L"obj1", L"§aNew", 2, "objective update");

    // 6. Score removal.
    javaScore(fd, "beta", 1, "obj1", 0);
    expectScore(lce, L"beta", 1, L"", 0, "score removed");

    // 7. Objective removal.
    javaObjective(fd, "obj1", 1, "");
    expectObjective(lce, L"obj1", L"", 1, "objective removed");

    // 8. Removing it again must be suppressed; sentinel proves it.
    javaObjective(fd, "obj1", 1, "");
    javaObjective(fd, "obj2", 0, "Second");
    expectObjective(lce, L"obj2", L"Second", 0,
                    "double-remove suppressed, next create flows");

    // ---- Teams (Java 0x3E -> LCE 209) ----

    // 9. Team create with RGB prefix (normalized) and two members.
    javaTeam(fd, "red", 0, "Red Team",
             "\xC2\xA7x\xC2\xA7"
             "F\xC2\xA7"
             "F\xC2\xA7"
             "5\xC2\xA7"
             "5\xC2\xA7"
             "5\xC2\xA7"
             "5[R] ",
             "", {"alpha", "beta"});
    expectTeam(lce, L"red", 0, L"Red Team", L"§c[R] ", L"",
               {L"alpha", L"beta"}, "team create (RGB prefix normalized)");

    // 10. Second team.
    javaTeam(fd, "blue", 0, "Blue Team", "\xC2\xA7" "9[B] ", "",
             {"gamma"});
    expectTeam(lce, L"blue", 0, L"Blue Team", L"§9[B] ", L"", {L"gamma"},
               "second team create");

    // 11. Duplicate create with identical info and members: fully suppressed.
    //     Prefix update afterwards is the sentinel.
    javaTeam(fd, "red", 0, "Red Team", "\xC2\xA7" "c[R] ", "",
             {"alpha", "beta"});
    javaTeam(fd, "red", 2, "Red Team", "\xC2\xA7" "4[R] ", "", {});
    expectTeam(lce, L"red", 2, L"Red Team", L"§4[R] ", L"", {},
               "duplicate create suppressed, prefix update flows");

    // 12. Duplicate info update suppressed; join is the sentinel.
    javaTeam(fd, "red", 2, "Red Team", "\xC2\xA7" "4[R] ", "", {});
    javaTeam(fd, "red", 3, "", "", "", {"delta"});
    expectTeam(lce, L"red", 3, L"", L"", L"", {L"delta"},
               "duplicate update suppressed, join flows");

    // 13. Duplicate join suppressed; the next join is the sentinel.
    javaTeam(fd, "red", 3, "", "", "", {"delta"});
    javaTeam(fd, "blue", 3, "", "", "", {"epsilon"});
    expectTeam(lce, L"blue", 3, L"", L"", L"", {L"epsilon"},
               "duplicate join suppressed (idempotent)");

    // 14. Player moves between teams: joining blue removes gamma... alpha
    //     moves from red to blue; only the join is emitted (the client
    //     relocates membership itself).
    javaTeam(fd, "blue", 3, "", "", "", {"alpha"});
    expectTeam(lce, L"blue", 3, L"", L"", L"", {L"alpha"},
               "player moved between teams");
    // Re-joining red afterwards must emit again (state moved).
    javaTeam(fd, "red", 3, "", "", "", {"alpha"});
    expectTeam(lce, L"red", 3, L"", L"", L"", {L"alpha"},
               "move back re-emits (state tracked)");

    // 15. Leave, idempotent: second leave suppressed, team remove sentinel.
    javaTeam(fd, "red", 4, "", "", "", {"beta"});
    expectTeam(lce, L"red", 4, L"", L"", L"", {L"beta"}, "team leave");
    javaTeam(fd, "red", 4, "", "", "", {"beta"});
    javaTeam(fd, "blue", 1, "", "", "", {});
    expectTeam(lce, L"blue", 1, L"", L"", L"", {},
               "duplicate leave suppressed, team remove flows");

    // 16. Duplicate team remove suppressed; new team create sentinel.
    javaTeam(fd, "blue", 1, "", "", "", {});
    javaTeam(fd, "green", 0, "Green", "", "", {});
    expectTeam(lce, L"green", 0, L"Green", L"", L"", {},
               "duplicate team remove suppressed");

    fprintf(stderr, "all scoreboard translation tests passed\n");
    proxy.requestStop();
    ::shutdown(fd, SHUT_RDWR);
    ::close(lceFd);
    ::close(listenFd);
    javaThread.detach();
    _exit(0);
}
