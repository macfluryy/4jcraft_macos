










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


struct JMeta {
    uint8_t index;
    uint8_t type;      
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
    javaReadPacket(fd, tmp);  
    javaReadPacket(fd, tmp);  

    std::vector<uint8_t> b;
    putStr(b, "11111111-2222-3333-4444-555555555555");
    putStr(b, "Tester");
    sendJavaPacket(fd, 0x02, b);  

    b.clear();  
    putBE32(b, 999);
    b.push_back(0);  
    b.push_back(0);  
    b.push_back(1);  
    b.push_back(20); 
    putStr(b, "default");
    b.push_back(0);  
    sendJavaPacket(fd, 0x01, b);

    b.clear();  
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
    std::vector<uint8_t> raw;  
};

struct LceReader {
    int fd;
    int lastMobId = -1;
    int lastMobType = -1;

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

    
    
    
    
    void waitFor(uint8_t wanted) {
        while (consumeOne().first != wanted) {
        }
    }

    
    LceEntityData nextEntityData() {
        while (true) {
            auto [id, data] = consumeOne();
            if (id == 40) return data;
        }
    }

    
    int nextMobType() {
        while (consumeOne().first != 24) {
        }
        return lastMobType;
    }

    std::pair<uint8_t, LceEntityData> consumeOne() {
        {
            const uint8_t id = u8();
            switch (id) {
                case 1:  
                    skip(4);
                    skipUtf();
                    skipUtf();
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
                case 20:  
                    skip(4);
                    skipUtf();
                    skip(12 + 3 + 2 + 16 + 1 + 4 + 4 + 4 + 3);
                    break;
                case 35:  skip(5);  break;   
                case 24:                     
                    lastMobId = u16();
                    lastMobType = u8();
                    skip(24);
                    break;
                case 34:  skip(18); break;   
                case 40: {                    
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

}  

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
        putBE16(pre, 3);            
        putBE16(pre, 0);            
        pre.push_back(0);           
        putBE32(pre, 0);            
        pre.push_back(0);           
        pre.insert(pre.end(), 14, 0);  
        putBE32(pre, 0);            
        pre.push_back(0);           
        putBE32(pre, 0);            
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
        putBE32(login, 78);  
        const std::wstring name = L"Tester", type = L"DEFAULT";
        putBE16(login, static_cast<uint16_t>(name.size()));
        for (wchar_t c : name) putBE16(login, static_cast<uint16_t>(c));
        putBE16(login, static_cast<uint16_t>(type.size()));
        for (wchar_t c : type) putBE16(login, static_cast<uint16_t>(c));
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
        putBE16(login, 54);  
        login.push_back(3);  
        putBE32(login, 10);  
        sendAll(lceFd, login.data(), login.size());
    }

    while (!javaReady.load()) std::this_thread::yield();
    const int fd = g_javaFd;

    
    
    
    lce.waitFor(13);
    check(true, "spawn handshake complete (MovePlayer received)");

    const uint8_t uuid[16] = {0, 1, 2,  3,  4,  5,  6,  7,
                              8, 9, 10, 11, 12, 13, 14, 15};

    
    {
        std::vector<uint8_t> b;
        putVarInt(b, 0);  
        putVarInt(b, 1);  
        b.insert(b.end(), uuid, uuid + 16);
        putStr(b, "Steve");
        putVarInt(b, 0);  
        putVarInt(b, 0);  
        putVarInt(b, 0);  
        b.push_back(0);   
        sendJavaPacket(fd, 0x38, b);
    }

    
    {
        std::vector<uint8_t> b;
        putVarInt(b, 100);
        b.insert(b.end(), uuid, uuid + 16);
        putBE32(b, 5 * 32);
        putBE32(b, 65 * 32);
        putBE32(b, 5 * 32);
        b.push_back(0);
        b.push_back(0);
        putBE16(b, 0);  
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

    
    javaSpawnMob(fd, 200, 91, {{16, 0, 11}, {12, 0, -1}});
    expectEntityData(lce, 1001, {{16, 0, 11}, {12, 2, -1}},
                     "sheep colour + baby (age byte -> int)");

    javaEntityMeta(fd, 200, {{6, 3, 0, 7.5f}, {7, 2, 0x2200AA}, {9, 0, 3}});
    expectEntityData(lce, 1001,
                     {{6, 3, 0, 7.5f}, {7, 2, 0x2200AA}, {9, 0, 3}},
                     "living health/potion/arrows");

    javaEntityMeta(fd, 200, {{16, 0, 0x1B}});
    expectEntityData(lce, 1001, {{16, 0, 0x1B}}, "sheep sheared+blue");

    
    javaEntityMeta(fd, 200, {{12, 0, 0}});
    expectEntityData(lce, 1001, {{12, 2, 0}}, "ageable adult");

    
    javaSpawnMob(fd, 201, 54, {{12, 0, 1}, {13, 0, 1}, {14, 0, 1}});
    expectEntityData(lce, 1002, {{12, 0, 1}, {13, 0, 1}, {14, 0, 1}},
                     "zombie baby/villager/converting");

    
    javaSpawnMob(fd, 202, 95, {{16, 0, 0x07}, {20, 0, 5}});
    expectEntityData(lce, 1003, {{16, 0, 0x07}, {20, 0, 5}},
                     "wolf flags + collar");

    
    javaSpawnMob(fd, 203, 120, {{16, 2, 3}});
    expectEntityData(lce, 1004, {{16, 2, 3}}, "villager profession");

    
    javaSpawnMob(fd, 204, 55, {{16, 0, 4}});
    expectEntityData(lce, 1005, {{16, 0, 4}}, "slime size");

    
    javaSpawnMob(fd, 205, 100, {{16, 2, 6}, {19, 0, 1}, {22, 2, 2}});

    
    
    
    javaSpawnMob(fd, 206, 30,
                 {{0, 0, 0x20}, {2, 4, 0, 0, "Hologram"}, {3, 0, 1},
                  {10, 0, 0x10}});
    check(lce.nextMobType() == 94, "armor stand spawns as Squid (94)");
    const int asLce = lce.lastMobId;
    expectEntityData(lce, asLce,
                     {{0, 0, 0x20}, {10, 4, 0, 0, L"Hologram"}, {11, 0, 1}},
                     "armor stand invisible+name+namevis forwarded");

    
    javaEntityMeta(fd, 200, {{15, 0, 1}});

    
    javaEntityMeta(fd, 100, {{0, 0, 0x10}});

    
    
    javaEntityMeta(fd, 204, {{16, 0, 7}});
    expectEntityData(lce, 1005, {{16, 0, 7}},
                     "horse/unknown-index/duplicate all suppressed");

    fprintf(stderr, "all metadata translation tests passed\n");
    proxy.requestStop();
    ::shutdown(fd, SHUT_RDWR);
    ::close(lceFd);
    ::close(listenFd);
    javaThread.detach();
    _exit(0);
}
