










#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <time.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
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
    out.push_back(static_cast<uint8_t>(v >> 8));
    out.push_back(static_cast<uint8_t>(v & 0xFF));
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


void javaScore(int fd, const std::string& owner, uint8_t action,
               const std::string& objective, int32_t value) {
    std::vector<uint8_t> b;
    putStr(b, owner);
    b.push_back(action);
    putStr(b, objective);
    if (action != 1) putVarInt(b, static_cast<uint32_t>(value));
    sendJavaPacket(fd, 0x3C, b);
}


void javaDisplay(int fd, uint8_t slot, const std::string& name) {
    std::vector<uint8_t> b;
    b.push_back(slot);
    putStr(b, name);
    sendJavaPacket(fd, 0x3D, b);
}


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
        b.push_back(1);          
        putStr(b, "always");     
        b.push_back(0);          
    }
    if (mode == 0 || mode == 3 || mode == 4) {
        putVarInt(b, static_cast<uint32_t>(players.size()));
        for (const std::string& p : players) putStr(b, p);
    }
    sendJavaPacket(fd, 0x3E, b);
}





struct NbtCursor {
    const uint8_t* p;
    size_t n;
    size_t i = 0;
    uint8_t u8() { return p[i++]; }
    uint16_t u16() {
        uint16_t v = (p[i] << 8) | p[i + 1];
        i += 2;
        return v;
    }
    int16_t s16() { return static_cast<int16_t>(u16()); }
    int32_t s32() {
        int32_t v = (p[i] << 24) | (p[i + 1] << 16) | (p[i + 2] << 8) | p[i + 3];
        i += 4;
        return v;
    }
    std::string name() {
        uint16_t l = u16();
        std::string s(reinterpret_cast<const char*>(p + i), l);
        i += l;
        return s;
    }
    std::string str() { return name(); }  
};

void nbtSkip(NbtCursor& c, int type);

void nbtSkip(NbtCursor& c, int type) {
    switch (type) {
        case 1: c.i += 1; break;
        case 2: c.i += 2; break;
        case 3: c.i += 4; break;
        case 4: c.i += 8; break;
        case 5: c.i += 4; break;
        case 6: c.i += 8; break;
        case 7: { int32_t l = c.s32(); c.i += l; break; }
        case 8: { uint16_t l = c.u16(); c.i += l; break; }
        case 9: {
            int ct = c.u8();
            int32_t cnt = c.s32();
            for (int32_t k = 0; k < cnt; ++k) nbtSkip(c, ct);
            break;
        }
        case 10:
            for (;;) {
                int t = c.u8();
                if (t == 0) break;
                c.name();
                nbtSkip(c, t);
            }
            break;
        case 11: { int32_t l = c.s32(); c.i += 4 * l; break; }
        default: fail("nbtSkip: bad type");
    }
}

struct ItemNbt {
    std::vector<std::pair<int, int>> ench;    
    std::vector<std::pair<int, int>> stored;
    int hideFlags = -1;
    bool hasDisplay = false;
    bool hasName = false;
    std::string name;                  
    std::vector<std::string> lore;     
};



void nbtReadDisplay(NbtCursor& c, ItemNbt& out) {
    for (;;) {
        int t = c.u8();
        if (t == 0) break;
        std::string nm = c.name();
        if (t == 8 && nm == "Name") { out.name = c.str(); out.hasName = true; }
        else if (t == 9 && nm == "Lore") {
            int ct = c.u8();
            int32_t cnt = c.s32();
            for (int32_t k = 0; k < cnt; ++k) {
                if (ct == 8) out.lore.push_back(c.str());
                else nbtSkip(c, ct);
            }
        } else {
            nbtSkip(c, t);
        }
    }
}

std::vector<std::pair<int, int>> nbtReadEnchList(NbtCursor& c) {
    std::vector<std::pair<int, int>> out;
    int ct = c.u8();
    int32_t cnt = c.s32();
    for (int32_t k = 0; k < cnt; ++k) {
        if (ct != 10) { nbtSkip(c, ct); continue; }
        int id = -1, lvl = 0;
        for (;;) {
            int t = c.u8();
            if (t == 0) break;
            std::string nm = c.name();
            if (t == 2 && nm == "id") id = c.s16();
            else if (t == 2 && nm == "lvl") lvl = c.s16();
            else nbtSkip(c, t);
        }
        out.push_back({id, lvl});
    }
    return out;
}

ItemNbt parseItemNbt(const std::vector<uint8_t>& raw) {
    
    NbtCursor c{raw.data(), raw.size()};
    ItemNbt out;
    if (c.u8() != 10) fail("item nbt: root not compound");
    c.name();  
    for (;;) {
        int t = c.u8();
        if (t == 0) break;
        std::string nm = c.name();
        if (t == 9 && nm == "ench") out.ench = nbtReadEnchList(c);
        else if (t == 9 && nm == "StoredEnchantments")
            out.stored = nbtReadEnchList(c);
        else if (nm == "HideFlags" && t == 3) out.hideFlags = c.s32();
        else if (t == 10 && nm == "display") {
            out.hasDisplay = true;
            nbtReadDisplay(c, out);
        } else nbtSkip(c, t);
    }
    return out;
}

int g_javaFd = -1;
std::mutex g_rpMutex;
std::vector<int> g_rpackStatus;  


void putJavaItem(std::vector<uint8_t>& b, int16_t id, uint8_t count,
                 int16_t dmg,
                 const std::vector<std::pair<int, int>>& ench,
                 const std::vector<std::pair<int, int>>& stored,
                 int hideFlags,
                 const std::string& displayName = "",
                 const std::vector<std::string>& lore = {}) {
    putBE16(b, static_cast<uint16_t>(id));
    if (id == -1) return;
    b.push_back(count);
    putBE16(b, static_cast<uint16_t>(dmg));
    const bool hasDisplay = !displayName.empty() || !lore.empty();
    const bool hasNbt =
        hasDisplay || !ench.empty() || !stored.empty() || hideFlags >= 0;
    if (!hasNbt) { b.push_back(0); return; }  
    b.push_back(10);   
    putBE16(b, 0);     
    auto name = [&](const char* s) {
        putBE16(b, static_cast<uint16_t>(strlen(s)));
        b.insert(b.end(), s, s + strlen(s));
    };
    auto str = [&](const std::string& s) {
        putBE16(b, static_cast<uint16_t>(s.size()));
        b.insert(b.end(), s.begin(), s.end());
    };
    if (hasDisplay) {
        b.push_back(10);  
        name("display");
        if (!displayName.empty()) {
            b.push_back(8); name("Name"); str(displayName);
        }
        if (!lore.empty()) {
            b.push_back(9); name("Lore");
            b.push_back(8);  
            putBE32(b, static_cast<uint32_t>(lore.size()));
            for (const std::string& line : lore) str(line);
        }
        b.push_back(0);   
    }
    auto enchList = [&](const char* key,
                        const std::vector<std::pair<int, int>>& e) {
        b.push_back(9);      
        name(key);
        b.push_back(10);     
        putBE32(b, static_cast<uint32_t>(e.size()));
        for (const auto& pr : e) {
            b.push_back(2); name("id"); putBE16(b, static_cast<uint16_t>(pr.first));
            b.push_back(2); name("lvl"); putBE16(b, static_cast<uint16_t>(pr.second));
            b.push_back(0);  
        }
    };
    if (!ench.empty()) enchList("ench", ench);
    if (!stored.empty()) enchList("StoredEnchantments", stored);
    if (hideFlags >= 0) {
        b.push_back(3); name("HideFlags");
        putBE32(b, static_cast<uint32_t>(hideFlags));
    }
    b.push_back(0);    
}


void javaSetSlot(int fd, int8_t windowId, int16_t slot,
                 const std::vector<uint8_t>& item) {
    std::vector<uint8_t> b;
    b.push_back(static_cast<uint8_t>(windowId));
    putBE16(b, static_cast<uint16_t>(slot));
    b.insert(b.end(), item.begin(), item.end());
    sendJavaPacket(fd, 0x2F, b);
}


void javaResourcePack(int fd, const std::string& url, const std::string& hash) {
    std::vector<uint8_t> b;
    putStr(b, url);
    putStr(b, hash);
    sendJavaPacket(fd, 0x48, b);
}

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
        uint32_t len = 0;
        int shift = 0;
        bool eof = false;
        while (true) {
            uint8_t bb;
            ssize_t k = ::recv(fd, &bb, 1, 0);
            if (k <= 0) { eof = true; break; }
            len |= static_cast<uint32_t>(bb & 0x7F) << shift;
            if (!(bb & 0x80)) break;
            shift += 7;
        }
        if (eof) return;
        std::vector<uint8_t> body(len);
        size_t off = 0;
        while (off < len) {
            ssize_t k = ::recv(fd, body.data() + off, len - off, 0);
            if (k <= 0) return;
            off += static_cast<size_t>(k);
        }
        if (len >= 2 && body[0] == 0x19) {
            std::lock_guard<std::mutex> lk(g_rpMutex);
            g_rpackStatus.push_back(body[1]);
        }
    }
}



struct LceScorePacket {
    uint8_t id = 0;          
    std::wstring a;          
    std::wstring b;          
    int32_t value = 0;       
    uint8_t method = 0;      
    std::wstring prefix;     
    std::wstring suffix;     
    std::vector<std::wstring> players;  
};

struct LceReader {
    int fd;
    bool lastItemHasNbt = false;
    std::vector<uint8_t> lastItemNbt;

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

    
    
    ItemNbt nextItemNbt() {
        while (consumeOne().id != 103) {
        }
        if (!lastItemHasNbt) return ItemNbt{};
        return parseItemNbt(lastItemNbt);
    }

    LceScorePacket nextScorePacket() {
        while (true) {
            LceScorePacket p = consumeOne();
            if (p.id >= 206 && p.id <= 210) return p;
        }
    }

    
    
    LceScorePacket consumeOne() {
        LceScorePacket out;
        const uint8_t id = u8();
        out.id = id;
        switch (id) {
            case 1:  
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
                out.method = u8();  
                out.a = utf();
                break;
            case 209:  
                out.a = utf();
                out.method = u8();
                if (out.method == 0 || out.method == 2) {
                    out.b = utf();
                    out.prefix = utf();
                    out.suffix = utf();
                    out.value = u8();  
                }
                if (out.method == 0 || out.method == 3 || out.method == 4) {
                    const uint16_t n = u16();
                    for (uint16_t i = 0; i < n; ++i)
                        out.players.push_back(utf());
                }
                break;
            case 103: {  
                u8();                                   
                u16();                                  
                const int lceId = static_cast<int16_t>(u16());
                lastItemHasNbt = false;
                lastItemNbt.clear();
                if (lceId != -1) {
                    u8();                               
                    u16();                              
                    const int nbtLen = static_cast<int16_t>(u16());
                    if (nbtLen > 0) {
                        lastItemHasNbt = true;
                        for (int k = 0; k < nbtLen; ++k)
                            lastItemNbt.push_back(u8());
                    }
                }
                break;
            }
            case 210:  
                out.method = u8();  
                if (out.method <= 2) {
                    out.a = utf();
                } else if (out.method == 3) {
                    out.value = static_cast<int32_t>(u32());   
                    out.players.push_back(
                        std::to_wstring(static_cast<int32_t>(u32())));  
                    out.players.push_back(
                        std::to_wstring(static_cast<int32_t>(u32())));  
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
        pre.push_back(0);
        pre.push_back(3);   
        pre.push_back(0);
        pre.push_back(0);   
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
        login.push_back(54);  
        login.push_back(3);   
        putBE32(login, 10);   
        sendAll(lceFd, login.data(), login.size());
    }

    while (!javaReady.load()) std::this_thread::yield();
    const int fd = g_javaFd;

    
    lce.waitFor(13);
    check(true, "spawn handshake complete (MovePlayer received)");

    
    
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

    
    javaDisplay(fd, 1, "obj1");
    expectDisplay(lce, 1, L"obj1", "display slot sidebar");

    
    javaScore(fd, "alpha", 0, "obj1", 5);
    javaScore(fd, "beta", 0, "obj1", 5);
    expectScore(lce, L"alpha", 0, L"obj1", 5, "score create alpha");
    expectScore(lce, L"beta", 0, L"obj1", 5, "score create beta (order kept)");

    
    
    
    javaScore(fd, "alpha", 0, "obj1", 5);   
    javaDisplay(fd, 1, "obj1");             
    javaScore(fd, "ghost", 0, "nosuch", 1); 
    javaDisplay(fd, 0, "obj1");             
    javaDisplay(fd, 2, "obj1");             
    javaScore(fd, "alpha", 0, "obj1", 6);   
    expectScore(lce, L"alpha", 0, L"obj1", 6,
                "duplicates/unknown/non-sidebar all suppressed");

    
    javaObjective(fd, "obj1", 2, "\xC2\xA7" "aNew");
    expectObjective(lce, L"obj1", L"§aNew", 2, "objective update");

    
    javaScore(fd, "beta", 1, "obj1", 0);
    expectScore(lce, L"beta", 1, L"", 0, "score removed");

    
    javaObjective(fd, "obj1", 1, "");
    expectObjective(lce, L"obj1", L"", 1, "objective removed");

    
    javaObjective(fd, "obj1", 1, "");
    javaObjective(fd, "obj2", 0, "Second");
    expectObjective(lce, L"obj2", L"Second", 0,
                    "double-remove suppressed, next create flows");

    

    
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

    
    javaTeam(fd, "blue", 0, "Blue Team", "\xC2\xA7" "9[B] ", "",
             {"gamma"});
    expectTeam(lce, L"blue", 0, L"Blue Team", L"§9[B] ", L"", {L"gamma"},
               "second team create");

    
    
    javaTeam(fd, "red", 0, "Red Team", "\xC2\xA7" "c[R] ", "",
             {"alpha", "beta"});
    javaTeam(fd, "red", 2, "Red Team", "\xC2\xA7" "4[R] ", "", {});
    expectTeam(lce, L"red", 2, L"Red Team", L"§4[R] ", L"", {},
               "duplicate create suppressed, prefix update flows");

    
    javaTeam(fd, "red", 2, "Red Team", "\xC2\xA7" "4[R] ", "", {});
    javaTeam(fd, "red", 3, "", "", "", {"delta"});
    expectTeam(lce, L"red", 3, L"", L"", L"", {L"delta"},
               "duplicate update suppressed, join flows");

    
    javaTeam(fd, "red", 3, "", "", "", {"delta"});
    javaTeam(fd, "blue", 3, "", "", "", {"epsilon"});
    expectTeam(lce, L"blue", 3, L"", L"", L"", {L"epsilon"},
               "duplicate join suppressed (idempotent)");

    
    
    
    javaTeam(fd, "blue", 3, "", "", "", {"alpha"});
    expectTeam(lce, L"blue", 3, L"", L"", L"", {L"alpha"},
               "player moved between teams");
    
    javaTeam(fd, "red", 3, "", "", "", {"alpha"});
    expectTeam(lce, L"red", 3, L"", L"", L"", {L"alpha"},
               "move back re-emits (state tracked)");

    
    javaTeam(fd, "red", 4, "", "", "", {"beta"});
    expectTeam(lce, L"red", 4, L"", L"", L"", {L"beta"}, "team leave");
    javaTeam(fd, "red", 4, "", "", "", {"beta"});
    javaTeam(fd, "blue", 1, "", "", "", {});
    expectTeam(lce, L"blue", 1, L"", L"", L"", {},
               "duplicate leave suppressed, team remove flows");

    
    javaTeam(fd, "blue", 1, "", "", "", {});
    javaTeam(fd, "green", 0, "Green", "", "", {});
    expectTeam(lce, L"green", 0, L"Green", L"", L"", {},
               "duplicate team remove suppressed");

    

    auto expectHud = [&](uint8_t action, const std::wstring& text,
                         const char* what) {
        const LceScorePacket p = lce.nextScorePacket();
        if (p.id != 210 || p.method != action ||
            (action <= 2 && p.a != text)) {
            fprintf(stderr, "FAIL: %s (id=%u action=%u text='%ls')\n", what,
                    p.id, p.method, p.a.c_str());
            exit(1);
        }
        fprintf(stderr, "ok: %s\n", what);
    };

    
    {
        std::vector<uint8_t> b;
        putStr(b, "{\"text\":\"\\u00a7eDeposit +5\"}");
        b.push_back(2);
        sendJavaPacket(fd, 0x02, b);
    }
    expectHud(0, L"§eDeposit +5", "actionbar routed to HUD overlay");

    
    {
        std::vector<uint8_t> b;   
        putVarInt(b, 2);
        putBE32(b, 5);
        putBE32(b, 40);
        putBE32(b, 10);
        sendJavaPacket(fd, 0x45, b);
    }
    {
        const LceScorePacket p = lce.nextScorePacket();
        if (p.id != 210 || p.method != 3 || p.value != 5 ||
            p.players != std::vector<std::wstring>{L"40", L"10"}) {
            fprintf(stderr, "FAIL: title times (v=%d)\n", p.value);
            exit(1);
        }
        fprintf(stderr, "ok: title times\n");
    }
    {
        std::vector<uint8_t> b;   
        putVarInt(b, 0);
        putStr(b, "{\"text\":\"\\u00a76Victory\"}");
        sendJavaPacket(fd, 0x45, b);
    }
    expectHud(1, L"§6Victory", "title text");
    {
        std::vector<uint8_t> b;   
        putVarInt(b, 1);
        putStr(b, "{\"text\":\"gg\"}");
        sendJavaPacket(fd, 0x45, b);
    }
    expectHud(2, L"gg", "subtitle text");
    {
        std::vector<uint8_t> b;   
        putVarInt(b, 3);
        sendJavaPacket(fd, 0x45, b);
    }
    expectHud(4, L"", "title clear");
    {
        std::vector<uint8_t> b;   
        putVarInt(b, 4);
        sendJavaPacket(fd, 0x45, b);
    }
    expectHud(5, L"", "title reset");

    
    javaResourcePack(fd, "http://example.com/pack.zip",
                     "0123456789abcdef0123456789abcdef01234567");
    for (int i = 0; i < 100; ++i) {  
        {
            std::lock_guard<std::mutex> lk(g_rpMutex);
            if (g_rpackStatus.size() >= 2) break;
        }
        struct timespec ts { 0, 50 * 1000 * 1000 };
        nanosleep(&ts, nullptr);
    }
    {
        std::lock_guard<std::mutex> lk(g_rpMutex);
        if (g_rpackStatus.size() != 2 || g_rpackStatus[0] != 3 ||
            g_rpackStatus[1] != 0) {
            fprintf(stderr, "FAIL: resource pack ack (n=%zu)\n",
                    g_rpackStatus.size());
            exit(1);
        }
    }
    fprintf(stderr, "ok: resource pack -> ACCEPTED then SUCCESS\n");

    
    using EL = std::vector<std::pair<int, int>>;
    using SL = std::vector<std::string>;
    auto sendItem = [&](const std::vector<uint8_t>& it) -> ItemNbt {
        javaSetSlot(fd, 0, 36, it);
        return lce.nextItemNbt();
    };

    
    {
        std::vector<uint8_t> it;
        putJavaItem(it, 267, 1, 0, EL{}, EL{}, -1);
        ItemNbt n = sendItem(it);
        if (n.hasDisplay || !n.ench.empty() || !n.stored.empty() ||
            n.hideFlags != -1) {
            fprintf(stderr, "FAIL: plain item carried NBT\n");
            exit(1);
        }
        fprintf(stderr, "ok: plain item (no NBT)\n");
    }

    
    {
        std::vector<uint8_t> it;
        putJavaItem(it, 267, 1, 0, EL{}, EL{}, -1, "Excalibur");
        ItemNbt n = sendItem(it);
        if (!n.hasName || n.name != "Excalibur") {
            fprintf(stderr, "FAIL: renamed item name='%s'\n", n.name.c_str());
            exit(1);
        }
        fprintf(stderr, "ok: renamed item (Name preserved)\n");
    }

    
    {
        std::vector<uint8_t> it;
        const SL lore{"Line one", "Line two"};
        putJavaItem(it, 267, 1, 0, EL{}, EL{}, -1, "", lore);
        ItemNbt n = sendItem(it);
        if (n.lore != lore) {
            fprintf(stderr, "FAIL: lore (n=%zu)\n", n.lore.size());
            exit(1);
        }
        fprintf(stderr, "ok: lore preserved\n");
    }

    
    {
        std::vector<uint8_t> it;
        putJavaItem(it, 267, 1, 0, EL{{16, 5}}, EL{}, -1);
        ItemNbt n = sendItem(it);
        if (n.ench != EL{{16, 5}}) {
            fprintf(stderr, "FAIL: one enchantment\n");
            exit(1);
        }
        fprintf(stderr, "ok: one enchantment (16:5)\n");
    }

    
    {
        std::vector<uint8_t> it;
        putJavaItem(it, 276, 1, 0, EL{{16, 5}, {19, 2}, {34, 3}}, EL{}, -1);
        ItemNbt n = sendItem(it);
        if (n.ench != EL{{16, 5}, {19, 2}, {34, 3}}) {
            fprintf(stderr, "FAIL: multiple enchants\n");
            exit(1);
        }
        fprintf(stderr, "ok: multiple enchantments\n");
    }

    
    {
        std::vector<uint8_t> it;
        putJavaItem(it, 403, 1, 0, EL{}, EL{{33, 1}}, -1);
        ItemNbt n = sendItem(it);
        if (n.stored != EL{{33, 1}} || !n.ench.empty()) {
            fprintf(stderr, "FAIL: enchanted book\n");
            exit(1);
        }
        fprintf(stderr, "ok: enchanted book (StoredEnchantments 33:1)\n");
    }

    
    {
        std::vector<uint8_t> it;
        putJavaItem(it, 267, 1, 0, EL{{16, 1}}, EL{}, 1);
        ItemNbt n = sendItem(it);
        if (n.ench != EL{{16, 1}} || n.hideFlags != 1) {
            fprintf(stderr, "FAIL: HideFlags item\n");
            exit(1);
        }
        fprintf(stderr, "ok: HideFlags=1 preserved, ench intact\n");
    }

    
    {
        std::vector<uint8_t> it;
        putJavaItem(it, 267, 1, 0, EL{{16, 1}, {-5, 2}}, EL{}, -1);
        ItemNbt n = sendItem(it);
        if (n.ench != EL{{16, 1}}) {
            fprintf(stderr, "FAIL: negative id not dropped (n=%zu)\n",
                    n.ench.size());
            exit(1);
        }
        fprintf(stderr, "ok: invalid enchant id (<0) dropped\n");
    }

    
    {
        std::vector<uint8_t> it;
        putJavaItem(it, 267, 1, 0, EL{{16, 1}, {300, 2}}, EL{}, -1);
        ItemNbt n = sendItem(it);
        if (n.ench != EL{{16, 1}}) {
            fprintf(stderr, "FAIL: id>255 not dropped (n=%zu)\n",
                    n.ench.size());
            exit(1);
        }
        fprintf(stderr, "ok: invalid enchant id (>255) dropped\n");
    }

    
    {
        std::vector<uint8_t> it;
        const SL lore{"Sharp blade"};
        putJavaItem(it, 267, 1, 0, EL{{16, 4}}, EL{}, -1, "Named", lore);
        ItemNbt n = sendItem(it);
        if (n.name != "Named" || n.lore != lore || n.ench != EL{{16, 4}}) {
            fprintf(stderr, "FAIL: display+ench together\n");
            exit(1);
        }
        fprintf(stderr, "ok: display identical after enchant forwarding\n");
    }

    fprintf(stderr, "all scoreboard translation tests passed\n");
    proxy.requestStop();
    ::shutdown(fd, SHUT_RDWR);
    ::close(lceFd);
    ::close(listenFd);
    javaThread.detach();
    _exit(0);
}
