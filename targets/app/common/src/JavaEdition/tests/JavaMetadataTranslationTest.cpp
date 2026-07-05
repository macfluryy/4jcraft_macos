// Regression test: Java 1.8 -> LCE entity metadata translation.
//
// Runs the real JavaServerProxy + JavaConnection between a scripted Java
// protocol-47 server and a scripted LCE client, both on loopback sockets
// inside this process. No rendering, no GL, no game loop. Deterministic:
// the LCE stream is consumed with blocking reads and every SetEntityData
// packet is asserted byte-exact; "no packet" cases are proven with a
// sentinel update that must be the next SetEntityData on the wire.
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

constexpr uint16_t kJavaPort = 25961;

[[noreturn]] void fail(const char* what) {
    fprintf(stderr, "FAIL: %s\n", what);
    exit(1);
}

void check(bool ok, const char* what) {
    if (!ok) fail(what);
    fprintf(stderr, "ok: %s\n", what);
}

// ---------------- raw socket helpers ----------------

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

void putBE16(std::vector<uint8_t>& out, uint16_t v) {
    out.push_back(v >> 8);
    out.push_back(v & 0xFF);
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

// One Java metadata entry appended to a body under construction.
struct JMeta {
    uint8_t index;
    uint8_t type;      // 0 byte, 1 short, 2 int, 3 float, 4 string
    int32_t i = 0;
    float f = 0;
    std::string s;
};

void putJavaMeta(std::vector<uint8_t>& body, const std::vector<JMeta>& items) {
    for (const JMeta& m : items) {
        body.push_back(static_cast<uint8_t>((m.type << 5) | m.index));
        switch (m.type) {
            case 0: body.push_back(static_cast<uint8_t>(m.i)); break;
            case 1: putBE16(body, static_cast<uint16_t>(m.i)); break;
            case 2: putBE32(body, static_cast<uint32_t>(m.i)); break;
            case 3: putF32(body, m.f); break;
            case 4: putStr(body, m.s); break;
        }
    }
    body.push_back(0x7F);
}

void javaSpawnMob(int fd, int eid, uint8_t type,
                  const std::vector<JMeta>& meta) {
    std::vector<uint8_t> b;
    putVarInt(b, static_cast<uint32_t>(eid));
    b.push_back(type);
    putBE32(b, 5 * 32);
    putBE32(b, 65 * 32);
    putBE32(b, 5 * 32);
    b.push_back(0);
    b.push_back(0);
    b.push_back(0);
    putBE16(b, 0);
    putBE16(b, 0);
    putBE16(b, 0);
    putJavaMeta(b, meta);
    sendJavaPacket(fd, 0x0F, b);
}

void javaEntityMeta(int fd, int eid, const std::vector<JMeta>& meta) {
    std::vector<uint8_t> b;
    putVarInt(b, static_cast<uint32_t>(eid));
    putJavaMeta(b, meta);
    sendJavaPacket(fd, 0x1C, b);
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

// Accepts the JavaConnection, performs login + join + first position, then
// drains serverbound traffic forever. Test steps send clientbound packets on
// g_javaFd from the main thread once this signals readiness.
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
    sendJavaPacket(fd, 0x02, b);  // login success -> play state

    b.clear();  // JoinGame
    putBE32(b, 999);
    b.push_back(0);  // gamemode
    b.push_back(0);  // dimension
    b.push_back(1);  // difficulty
    b.push_back(20); // max players
    putStr(b, "default");
    b.push_back(0);  // reduced debug info
    sendJavaPacket(fd, 0x01, b);

    b.clear();  // PlayerPositionAndLook -> triggers the LCE spawn handshake
    putF64(b, 0.0);
    putF64(b, 65.0);
    putF64(b, 0.0);
    putF32(b, 0.0);
    putF32(b, 0.0);
    b.push_back(0);
    sendJavaPacket(fd, 0x08, b);

    g_javaFd = fd;
    ready->store(true);
    // Drain serverbound traffic so the proxy never blocks on writes.
    while (true) {
        uint8_t buf[4096];
        ssize_t k = ::recv(fd, buf, sizeof(buf), 0);
        if (k <= 0) return;
    }
}

// ---------------- scripted LCE client ----------------

struct LceMetaItem {
    uint8_t index;
    uint8_t type;
    int32_t i = 0;
    float f = 0;
    std::wstring s;
};

struct LceEntityData {
    int32_t entityId = 0;
    std::vector<LceMetaItem> items;
    std::vector<uint8_t> raw;  // full packet bytes including id 40
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
    void skipUtf() { skip(2ull * u16()); }

    // Reads LCE packets until the requested id has been consumed (skipping
    // every other packet the proxy emits in this scenario) - used to wait for
    // MovePlayer (13), which marks the end of the spawn handshake. Unknown
    // id = failure.
    void waitFor(uint8_t wanted) {
        while (consumeOne().first != wanted) {
        }
    }

    // Reads LCE packets until the next SetEntityData (40).
    LceEntityData nextEntityData() {
        while (true) {
            auto [id, data] = consumeOne();
            if (id == 40) return data;
        }
    }

    std::pair<uint8_t, LceEntityData> consumeOne() {
        {
            const uint8_t id = u8();
            switch (id) {
                case 1:  // Login
                    skip(4);
                    skipUtf();
                    skipUtf();
                    skip(8 + 4 + 1 + 1 + 1);
                    skip(8 + 8 + 1 + 4 + 1 + 4 + 1 + 4 + 4 + 1 + 1 + 4);
                    skip(2 + 1 + 4);
                    break;
                case 6:   skip(12); break;   // SpawnPosition
                case 202: skip(9);  break;   // PlayerAbilities
                case 13:  skip(41); break;   // MovePlayerPosRot
                case 4:   skip(16); break;   // SetTime
                case 51: {                    // BlockRegionUpdate
                    skip(1 + 4 + 2 + 4 + 1 + 1 + 1);
                    const uint32_t size = u32() & 0x3FFFFFFF;
                    skip(size);
                    break;
                }
                case 20:  // AddPlayer
                    skip(4);
                    skipUtf();
                    skip(12 + 3 + 2 + 16 + 1 + 4 + 4 + 4 + 3);
                    break;
                case 35:  skip(5);  break;   // RotateHead
                case 24:  skip(27); break;   // AddMob
                case 34:  skip(18); break;   // TeleportEntity
                case 40: {                    // SetEntityData
                    LceEntityData out;
                    out.raw.push_back(40);
                    uint8_t b4[4];
                    recvAll(fd, b4, 4);
                    out.raw.insert(out.raw.end(), b4, b4 + 4);
                    out.entityId =
                        static_cast<int32_t>((b4[0] << 24) | (b4[1] << 16) |
                                             (b4[2] << 8) | b4[3]);
                    while (true) {
                        const uint8_t h = u8();
                        out.raw.push_back(h);
                        if (h == 0x7F) return {40, std::move(out)};
                        LceMetaItem item;
                        item.type = h >> 5;
                        item.index = h & 0x1F;
                        switch (item.type) {
                            case 0: {
                                uint8_t v = u8();
                                out.raw.push_back(v);
                                item.i = static_cast<int8_t>(v);
                                break;
                            }
                            case 1: {
                                uint8_t b[2];
                                recvAll(fd, b, 2);
                                out.raw.insert(out.raw.end(), b, b + 2);
                                item.i = static_cast<int16_t>((b[0] << 8) |
                                                              b[1]);
                                break;
                            }
                            case 2: {
                                uint8_t b[4];
                                recvAll(fd, b, 4);
                                out.raw.insert(out.raw.end(), b, b + 4);
                                item.i = static_cast<int32_t>(
                                    (b[0] << 24) | (b[1] << 16) | (b[2] << 8) |
                                    b[3]);
                                break;
                            }
                            case 3: {
                                uint8_t b[4];
                                recvAll(fd, b, 4);
                                out.raw.insert(out.raw.end(), b, b + 4);
                                uint32_t u = (b[0] << 24) | (b[1] << 16) |
                                             (b[2] << 8) | b[3];
                                memcpy(&item.f, &u, 4);
                                break;
                            }
                            case 4: {
                                uint8_t lb[2];
                                recvAll(fd, lb, 2);
                                out.raw.insert(out.raw.end(), lb, lb + 2);
                                const uint16_t n =
                                    static_cast<uint16_t>((lb[0] << 8) |
                                                          lb[1]);
                                for (uint16_t k = 0; k < n; ++k) {
                                    uint8_t cb[2];
                                    recvAll(fd, cb, 2);
                                    out.raw.insert(out.raw.end(), cb, cb + 2);
                                    item.s.push_back(static_cast<wchar_t>(
                                        (cb[0] << 8) | cb[1]));
                                }
                                break;
                            }
                            default:
                                fail("SetEntityData: unexpected item type");
                        }
                        out.items.push_back(std::move(item));
                    }
                }
                default:
                    fprintf(stderr, "unknown LCE packet id=%u\n", id);
                    fail("unknown LCE packet id");
            }
            return {id, {}};
        }
    }
};

// Expected-vs-actual assertion for one SetEntityData packet.
void expectEntityData(LceReader& lce, int expectId,
                      const std::vector<LceMetaItem>& expect,
                      const char* what) {
    const LceEntityData got = lce.nextEntityData();
    if (got.entityId != expectId) {
        fprintf(stderr, "FAIL: %s: entity %d, expected %d\n", what,
                got.entityId, expectId);
        exit(1);
    }
    if (got.items.size() != expect.size()) {
        fprintf(stderr, "FAIL: %s: %zu items, expected %zu\n", what,
                got.items.size(), expect.size());
        exit(1);
    }
    // Byte-exact: rebuild the expected packet and compare with the raw bytes.
    std::vector<uint8_t> raw;
    raw.push_back(40);
    putBE32(raw, static_cast<uint32_t>(expectId));
    for (const LceMetaItem& e : expect) {
        raw.push_back(static_cast<uint8_t>((e.type << 5) | (e.index & 0x1F)));
        switch (e.type) {
            case 0: raw.push_back(static_cast<uint8_t>(e.i)); break;
            case 1: putBE16(raw, static_cast<uint16_t>(e.i)); break;
            case 2: putBE32(raw, static_cast<uint32_t>(e.i)); break;
            case 3: putF32(raw, e.f); break;
            case 4:
                putBE16(raw, static_cast<uint16_t>(e.s.size()));
                for (wchar_t c : e.s) putBE16(raw, static_cast<uint16_t>(c));
                break;
        }
    }
    raw.push_back(0x7F);
    if (raw != got.raw) {
        fprintf(stderr, "FAIL: %s: byte mismatch (%zu vs %zu bytes)\n", what,
                got.raw.size(), raw.size());
        exit(1);
    }
    fprintf(stderr, "ok: %s\n", what);
}

}  // namespace

int main() {
    // Java-side listener the JavaConnection will dial.
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

    // LCE client: connect + login handshake per JavaServerProxy::runWorker.
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
        pre.push_back(2);           // PreLogin
        putBE16(pre, 3);            // netcode version
        putBE16(pre, 0);            // login key length
        pre.push_back(0);           // friends only
        putBE32(pre, 0);            // ugc version
        pre.push_back(0);           // player count
        pre.insert(pre.end(), 14, 0);  // save name
        putBE32(pre, 0);            // server settings
        pre.push_back(0);           // host index
        putBE32(pre, 0);            // texture pack id
        sendAll(lceFd, pre.data(), pre.size());
    }

    LceReader lce{lceFd};
    check(lce.u8() == 2, "PreLogin response id");
    lce.skip(2);
    lce.skipUtf();
    lce.skip(1 + 4 + 1 + 14 + 4 + 1 + 4);

    {
        std::vector<uint8_t> login;
        login.push_back(1);
        putBE32(login, 78);  // client version
        const std::wstring name = L"Tester", type = L"DEFAULT";
        putBE16(login, static_cast<uint16_t>(name.size()));
        for (wchar_t c : name) putBE16(login, static_cast<uint16_t>(c));
        putBE16(login, static_cast<uint16_t>(type.size()));
        for (wchar_t c : type) putBE16(login, static_cast<uint16_t>(c));
        putBE64(login, 0);   // seed
        putBE32(login, 0);   // game type
        login.push_back(0);  // dimension
        login.push_back(255);  // map height
        login.push_back(8);    // max players
        putBE64(login, 0);   // offline xuid
        putBE64(login, 0);   // online xuid
        login.push_back(0);  // friends only ugc
        putBE32(login, 0);   // ugc players version
        login.push_back(1);  // difficulty
        putBE32(login, 0);   // multiplayer instance id
        login.push_back(0);  // player index
        putBE32(login, 0);   // skin id
        putBE32(login, 0);   // cape id
        login.push_back(0);  // is guest
        login.push_back(0);  // new sea level
        putBE32(login, 0);   // ui game privileges
        putBE16(login, 54);  // xz size
        login.push_back(3);  // hell scale
        putBE32(login, 10);  // view distance
        sendAll(lceFd, login.data(), login.size());
    }

    while (!javaReady.load()) std::this_thread::yield();
    const int fd = g_javaFd;

    // Wait until the proxy finished the spawn handshake (it ends with the
    // MovePlayerPosRot packet). Entity packets sent before that point would
    // race the handshake's event drain.
    lce.waitFor(13);
    check(true, "spawn handshake complete (MovePlayer received)");

    const uint8_t uuid[16] = {0, 1, 2,  3,  4,  5,  6,  7,
                              8, 9, 10, 11, 12, 13, 14, 15};

    // Tab list entry so SpawnPlayer resolves a name.
    {
        std::vector<uint8_t> b;
        putVarInt(b, 0);  // action add
        putVarInt(b, 1);  // count
        b.insert(b.end(), uuid, uuid + 16);
        putStr(b, "Steve");
        putVarInt(b, 0);  // properties
        putVarInt(b, 0);  // gamemode
        putVarInt(b, 0);  // ping
        b.push_back(0);   // no display name
        sendJavaPacket(fd, 0x38, b);
    }

    // ---- Players: sneak at spawn, then sprint+invisible, fire, using. ----
    {
        std::vector<uint8_t> b;
        putVarInt(b, 100);
        b.insert(b.end(), uuid, uuid + 16);
        putBE32(b, 5 * 32);
        putBE32(b, 65 * 32);
        putBE32(b, 5 * 32);
        b.push_back(0);
        b.push_back(0);
        putBE16(b, 0);  // current item
        putJavaMeta(b, {{0, 0, 0x02}});
        sendJavaPacket(fd, 0x0C, b);
    }
    expectEntityData(lce, 1000, {{0, 0, 0x02}}, "player sneak (spawn meta)");

    javaEntityMeta(fd, 100, {{0, 0, 0x28}});
    expectEntityData(lce, 1000, {{0, 0, 0x28}}, "player sprint+invisible");

    javaEntityMeta(fd, 100, {{0, 0, 0x01}});
    expectEntityData(lce, 1000, {{0, 0, 0x01}}, "player on fire");

    javaEntityMeta(fd, 100, {{0, 0, 0x10}});
    expectEntityData(lce, 1000, {{0, 0, 0x10}}, "player using item");

    // ---- LivingEntity: health / potion colour / arrows (on a sheep). ----
    javaSpawnMob(fd, 200, 91, {{16, 0, 11}, {12, 0, -1}});
    expectEntityData(lce, 1001, {{16, 0, 11}, {12, 2, -1}},
                     "sheep colour + baby (age byte -> int)");

    javaEntityMeta(fd, 200, {{6, 3, 0, 7.5f}, {7, 2, 0x2200AA}, {9, 0, 3}});
    expectEntityData(lce, 1001,
                     {{6, 3, 0, 7.5f}, {7, 2, 0x2200AA}, {9, 0, 3}},
                     "living health/potion/arrows");

    javaEntityMeta(fd, 200, {{16, 0, 0x1B}});
    expectEntityData(lce, 1001, {{16, 0, 0x1B}}, "sheep sheared+blue");

    // ---- Ageable adult flip. ----
    javaEntityMeta(fd, 200, {{12, 0, 0}});
    expectEntityData(lce, 1001, {{12, 2, 0}}, "ageable adult");

    // ---- Zombie: baby / villager / converting. ----
    javaSpawnMob(fd, 201, 54, {{12, 0, 1}, {13, 0, 1}, {14, 0, 1}});
    expectEntityData(lce, 1002, {{12, 0, 1}, {13, 0, 1}, {14, 0, 1}},
                     "zombie baby/villager/converting");

    // ---- Wolf: sit+angry+tame flags, collar. ----
    javaSpawnMob(fd, 202, 95, {{16, 0, 0x07}, {20, 0, 5}});
    expectEntityData(lce, 1003, {{16, 0, 0x07}, {20, 0, 5}},
                     "wolf flags + collar");

    // ---- Villager profession. ----
    javaSpawnMob(fd, 203, 120, {{16, 2, 3}});
    expectEntityData(lce, 1004, {{16, 2, 3}}, "villager profession");

    // ---- Slime size. ----
    javaSpawnMob(fd, 204, 55, {{16, 0, 4}});
    expectEntityData(lce, 1005, {{16, 0, 4}}, "slime size");

    // ---- Horse: flags/variant/armor -> cached, entity unspawned, so the
    // next SetEntityData on the wire must belong to the sentinel below. ----
    javaSpawnMob(fd, 205, 100, {{16, 2, 6}, {19, 0, 1}, {22, 2, 2}});

    // ---- Unsupported: armor stand meta must produce nothing. ----
    javaSpawnMob(fd, 206, 30, {{10, 0, 1}});

    // ---- Unsupported index on a supported mob: ignored. ----
    javaEntityMeta(fd, 200, {{15, 0, 1}});

    // ---- Duplicate update suppressed: same flags as before. ----
    javaEntityMeta(fd, 100, {{0, 0, 0x10}});

    // Sentinel: this must be the very next SetEntityData, proving the four
    // cases above emitted no packets.
    javaEntityMeta(fd, 204, {{16, 0, 7}});
    expectEntityData(lce, 1005, {{16, 0, 7}},
                     "horse/armorstand/unknown-index/duplicate all suppressed");

    fprintf(stderr, "all metadata translation tests passed\n");
    proxy.requestStop();
    ::shutdown(fd, SHUT_RDWR);
    ::close(lceFd);
    ::close(listenFd);
    javaThread.detach();
    _exit(0);
}
