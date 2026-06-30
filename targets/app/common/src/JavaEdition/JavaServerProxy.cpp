#include "app/common/src/JavaEdition/JavaServerProxy.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "app/common/src/JavaEdition/JavaBlockIdMap.h"
#include "app/common/src/JavaEdition/JavaItemIdMap.h"
#include "app/common/src/JavaEdition/JavaSoundIdMap.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"

JavaServerProxy* g_activeJavaProxy = nullptr;

namespace {

constexpr int kLceProtocolVersion = 78;
constexpr int kLceNetcodeVersion  = 560;
constexpr int kLceXzSize          = 5 * 64;
constexpr int kLceHellScale       = 8;
constexpr int kLceMaxPlayers      = 20;
constexpr int kLceViewDistance    = 10;

constexpr uint8_t kLceIdPreLogin      = 2;
constexpr uint8_t kLceIdLogin         = 1;
constexpr uint8_t kLceIdChat          = 3;
constexpr uint8_t kLceIdBRUP          = 51;
constexpr uint8_t kLceIdAddMob        = 24;
constexpr uint8_t kLceIdAddEntity     = 23;
constexpr uint8_t kLceIdEntityData    = 40;
constexpr uint8_t kLceIdRemoveEntity  = 29;
constexpr uint8_t kLceIdMoveEntity_PR = 33;
constexpr uint8_t kLceIdTeleportEnt   = 34;
constexpr uint8_t kLceIdMovePlayerBase= 10;
constexpr uint8_t kLceIdMovePlayerPos = 11;
constexpr uint8_t kLceIdMovePlayerRot = 12;
constexpr uint8_t kLceIdMovePlayerPR  = 13;
constexpr uint8_t kLceIdAnimate       = 18;
constexpr uint8_t kLceIdEntityEvent   = 38;
constexpr uint8_t kLceIdLevelSound    = 62;
constexpr uint8_t kLceIdSetTime       = 4;
constexpr uint8_t kLceIdRespawn       = 9;
constexpr uint8_t kLceIdSetExperience = 43;
constexpr uint8_t kLceIdGameEvent     = 70;
constexpr uint8_t kLceIdTileEvent     = 54;
constexpr uint8_t kLceIdExplode       = 60;
constexpr uint8_t kLceIdSetEntityLink = 39;
constexpr uint8_t kLceIdSignUpdate    = 130;
constexpr uint8_t kLceIdTileEditorOpen= 133;
constexpr uint8_t kLceIdContainerData = 105;
constexpr uint8_t kLceIdDisconnect    = 255;

constexpr int kLceDisconnectKicked     = 8;
constexpr int kLceDisconnectServerFull = 12;
constexpr int kLceDisconnectOutdated   = 14;
constexpr int kLceDisconnectBanned     = 25;
constexpr int kLceDisconnectCustomText = 28;

static int jsp_mapDisconnectReason(const std::wstring& reason) {
    std::wstring r;
    r.reserve(reason.size());
    for (wchar_t c : reason)
        r.push_back((c >= L'A' && c <= L'Z') ? static_cast<wchar_t>(c + 32) : c);
    auto has = [&](const wchar_t* s) { return r.find(s) != std::wstring::npos; };
    if (has(L"ban")) return kLceDisconnectBanned;
    if (has(L"full") || has(L"queue")) return kLceDisconnectServerFull;
    if (has(L"outdated") || has(L"protocol")) return kLceDisconnectOutdated;
    return kLceDisconnectKicked;
}

constexpr double kLceFeetOffset = 1.65;
constexpr double kLceEyeOffset  = 1.62;

static const char* jsp_lceName(uint8_t id) {
    switch (id) {
        case kLceIdPreLogin:      return "PreLogin";
        case kLceIdLogin:         return "Login";
        case kLceIdChat:          return "Chat";
        case kLceIdBRUP:          return "BlockRegionUpdate";
        case kLceIdAddMob:        return "AddMob";
        case kLceIdAddEntity:     return "AddEntity";
        case kLceIdEntityData:    return "EntityData";
        case kLceIdRemoveEntity:  return "RemoveEntity";
        case kLceIdMoveEntity_PR: return "MoveEntity";
        case kLceIdTeleportEnt:   return "TeleportEntity";
        case kLceIdMovePlayerBase: return "MovePlayerBase";
        case kLceIdMovePlayerPos: return "MovePlayerPos";
        case kLceIdMovePlayerRot: return "MovePlayerRot";
        case kLceIdMovePlayerPR:  return "MovePlayerPosRot";
        case kLceIdAnimate:       return "Animate";
        case kLceIdEntityEvent:   return "EntityEvent";
        case kLceIdLevelSound:    return "LevelSound";
        case kLceIdSetTime:       return "SetTime";
        case kLceIdRespawn:       return "Respawn";
        case kLceIdSetExperience: return "SetExperience";
        case kLceIdGameEvent:     return "GameEvent";
        case kLceIdTileEvent:     return "TileEvent";
        case kLceIdExplode:       return "Explode";
        case kLceIdSetEntityLink: return "SetEntityLink";
        case kLceIdSignUpdate:    return "SignUpdate";
        case kLceIdTileEditorOpen: return "TileEditorOpen";
        case kLceIdContainerData: return "ContainerData";
        case 20:                  return "AddPlayer";
        case 0x64:                return "ContainerOpen";
        case 0x67:                return "ContainerSetSlot";
        case 0x68:                return "ContainerSetContent";
        case kLceIdDisconnect:    return "Disconnect";
        default:                  return "?";
    }
}

constexpr size_t kLzxRleScratchFactor = 2;

int waitReadable(int fd, int timeoutMs) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    int n = poll(&pfd, 1, timeoutMs);
    if (n < 0) return -1;
    if (n == 0) return 0;
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) return -1;
    return (pfd.revents & POLLIN) ? 1 : 0;
}

inline void packBE16(uint8_t* p, uint16_t v) {
    p[0] = static_cast<uint8_t>(v >> 8);
    p[1] = static_cast<uint8_t>(v);
}
inline void packBE32(uint8_t* p, uint32_t v) {
    p[0] = static_cast<uint8_t>(v >> 24);
    p[1] = static_cast<uint8_t>(v >> 16);
    p[2] = static_cast<uint8_t>(v >> 8);
    p[3] = static_cast<uint8_t>(v);
}
inline void packBE64(uint8_t* p, uint64_t v) {
    packBE32(p,     static_cast<uint32_t>(v >> 32));
    packBE32(p + 4, static_cast<uint32_t>(v));
}
inline uint16_t unpackBE16(const uint8_t* p) {
    return (static_cast<uint16_t>(p[0]) << 8) | p[1];
}
inline uint32_t unpackBE32(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) |
            static_cast<uint32_t>(p[3]);
}
inline uint64_t unpackBE64(const uint8_t* p) {
    return (static_cast<uint64_t>(unpackBE32(p)) << 32) | unpackBE32(p + 4);
}

namespace lce_str {
constexpr size_t kPreLoginName  = 32;
constexpr size_t kPlayerName    = 20;
constexpr size_t kChat          = 123;
constexpr size_t kSignLine      = 15;
constexpr size_t kWindowTitle   = 64;
constexpr size_t kLevelType     = 16;
[[maybe_unused]] constexpr size_t kObjectiveName = 16;
[[maybe_unused]] constexpr size_t kObjectiveDisp = 32;
[[maybe_unused]] constexpr size_t kTeamName      = 16;
[[maybe_unused]] constexpr size_t kTeamDisplay   = 32;
[[maybe_unused]] constexpr size_t kTeamAffix     = 16;
[[maybe_unused]] constexpr size_t kCustomPayload = 20;
[[maybe_unused]] constexpr size_t kParticleName  = 64;
constexpr size_t kDisconnectText = 256;
}

inline void appendLceUtf(std::vector<uint8_t>& buf, const std::wstring& s,
                         size_t maxLen) {
    const size_t n = s.size() < maxLen ? s.size() : maxLen;
    uint8_t tmp[2];
    packBE16(tmp, static_cast<uint16_t>(n));
    buf.insert(buf.end(), tmp, tmp + 2);
    for (size_t i = 0; i < n; ++i) {
        packBE16(tmp, static_cast<uint16_t>(s[i]));
        buf.insert(buf.end(), tmp, tmp + 2);
    }
}

}


JavaServerProxy::JavaServerProxy() {
    static bool s_ignored = false;
    if (!s_ignored) {
        signal(SIGPIPE, SIG_IGN);
        s_ignored = true;
    }
}

JavaServerProxy::~JavaServerProxy() {
    requestStop();
    if (m_worker.joinable()) m_worker.join();
    if (m_listenFd >= 0) ::close(m_listenFd);
    if (m_clientFd >= 0) ::close(m_clientFd);
    if (g_activeJavaProxy == this) g_activeJavaProxy = nullptr;
}


int JavaServerProxy::startListening() {
    m_listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd < 0) {
        fprintf(stderr, "[JEProxy] socket() failed: %s\n", strerror(errno));
        return -1;
    }
    int yes = 1;
    setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    if (::bind(m_listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        fprintf(stderr, "[JEProxy] bind() failed: %s\n", strerror(errno));
        ::close(m_listenFd);
        m_listenFd = -1;
        return -1;
    }
    socklen_t alen = sizeof(addr);
    if (::getsockname(m_listenFd, reinterpret_cast<sockaddr*>(&addr), &alen) != 0) {
        fprintf(stderr, "[JEProxy] getsockname() failed\n");
        ::close(m_listenFd);
        m_listenFd = -1;
        return -1;
    }
    m_listenPort = ntohs(addr.sin_port);
    if (::listen(m_listenFd, 1) != 0) {
        fprintf(stderr, "[JEProxy] listen() failed: %s\n", strerror(errno));
        ::close(m_listenFd);
        m_listenFd = -1;
        return -1;
    }
    return m_listenPort;
}


void JavaServerProxy::startWorker(const std::string& javaHost, uint16_t javaPort,
                                  const std::string& nicknameUtf8,
                                  const std::wstring& displayName) {
    m_javaHost = javaHost;
    m_javaPort = javaPort;
    m_nicknameUtf8 = nicknameUtf8;
    m_displayName = displayName;
    m_worker = std::thread([this]() { runWorker(); });
}

void JavaServerProxy::requestStop() {
    m_stopRequested.store(true);
    if (m_listenFd >= 0) {
        ::shutdown(m_listenFd, SHUT_RDWR);
    }
    if (m_clientFd >= 0) {
        ::shutdown(m_clientFd, SHUT_RDWR);
    }
    if (m_java) m_java->requestDisconnect();
}


bool JavaServerProxy::readAll(uint8_t* buf, size_t n) {
    size_t got = 0;
    while (got < n) {
        if (m_stopRequested.load()) return false;
        int r = waitReadable(m_clientFd, 100);
        if (r < 0) return false;
        if (r == 0) continue;
        ssize_t k = ::recv(m_clientFd, buf + got, n - got, 0);
        if (k <= 0) {
            if (k < 0 && (errno == EINTR || errno == EAGAIN)) continue;
            return false;
        }
        got += static_cast<size_t>(k);
    }
    return true;
}

bool JavaServerProxy::writeAll(const uint8_t* buf, size_t n) {
    std::lock_guard<std::mutex> lock(m_sendMutex);
    if (m_clientFd < 0) return false;
    static const bool s_txlog = (getenv("JEPROXY_TXLOG") != nullptr);
    if (s_txlog && n > 0) {
        fprintf(stderr, "[JTX] -> LCE id=%u (%s) len=%zu\n",
                static_cast<unsigned>(buf[0]), jsp_lceName(buf[0]), n);
    }
    size_t sent = 0;
    while (sent < n) {
        ssize_t k = ::send(m_clientFd, buf + sent, n - sent, 0);
        if (k <= 0) {
            if (k < 0 && (errno == EINTR)) continue;
            return false;
        }
        sent += static_cast<size_t>(k);
    }
    return true;
}

bool JavaServerProxy::readU8(uint8_t& v) {
    return readAll(&v, 1);
}
bool JavaServerProxy::readU16(uint16_t& v) {
    uint8_t b[2];
    if (!readAll(b, 2)) return false;
    v = unpackBE16(b);
    return true;
}
bool JavaServerProxy::readI16(int16_t& v) {
    uint16_t u;
    if (!readU16(u)) return false;
    v = static_cast<int16_t>(u);
    return true;
}
bool JavaServerProxy::readI32(int32_t& v) {
    uint8_t b[4];
    if (!readAll(b, 4)) return false;
    v = static_cast<int32_t>(unpackBE32(b));
    return true;
}
bool JavaServerProxy::readI64(int64_t& v) {
    uint8_t b[8];
    if (!readAll(b, 8)) return false;
    v = static_cast<int64_t>(unpackBE64(b));
    return true;
}
bool JavaServerProxy::readF32(float& v) {
    uint8_t b[4];
    if (!readAll(b, 4)) return false;
    uint32_t u = unpackBE32(b);
    memcpy(&v, &u, 4);
    return true;
}
bool JavaServerProxy::readF64(double& v) {
    uint8_t b[8];
    if (!readAll(b, 8)) return false;
    uint64_t u = unpackBE64(b);
    memcpy(&v, &u, 8);
    return true;
}
bool JavaServerProxy::readUtf16(std::wstring& out, int maxChars) {
    uint16_t len;
    if (!readU16(len)) return false;
    if (len > maxChars) return false;
    out.clear();
    out.reserve(len);
    for (int i = 0; i < len; ++i) {
        uint16_t c;
        if (!readU16(c)) return false;
        out.push_back(static_cast<wchar_t>(c));
    }
    return true;
}


bool JavaServerProxy::sendPreLoginPacket(const std::wstring& username) {
    std::vector<uint8_t> buf;
    buf.reserve(64 + username.size() * 2);
    buf.push_back(kLceIdPreLogin);
    uint8_t tmp[2];
    packBE16(tmp, static_cast<uint16_t>(kLceNetcodeVersion));
    buf.insert(buf.end(), tmp, tmp + 2);
    appendLceUtf(buf, username, lce_str::kPreLoginName);
    buf.push_back(0);
    uint8_t z4[4] = {0,0,0,0};
    buf.insert(buf.end(), z4, z4 + 4);
    buf.push_back(0);
    for (int i = 0; i < 14; ++i) buf.push_back(0);
    buf.insert(buf.end(), z4, z4 + 4);
    buf.push_back(0);
    buf.insert(buf.end(), z4, z4 + 4);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendLoginPacket(const std::wstring& username) {
    std::vector<uint8_t> buf;
    buf.reserve(96 + username.size() * 2);
    buf.push_back(kLceIdLogin);
    uint8_t tmp[8];
    packBE32(tmp, static_cast<uint32_t>(kLceProtocolVersion));
    buf.insert(buf.end(), tmp, tmp + 4);
    appendLceUtf(buf, username, lce_str::kPlayerName);
    appendLceUtf(buf, L"default", lce_str::kLevelType);
    packBE64(tmp, 0);
    buf.insert(buf.end(), tmp, tmp + 8);
    packBE32(tmp, static_cast<uint32_t>(m_javaGameMode));
    buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(static_cast<uint8_t>(m_javaDimension & 0xFF));
    const int loginMapHeight = 256;
    buf.push_back(static_cast<uint8_t>(loginMapHeight - 1));
    buf.push_back(kLceMaxPlayers);
    packBE64(tmp, 0); buf.insert(buf.end(), tmp, tmp + 8);
    packBE64(tmp, 0); buf.insert(buf.end(), tmp, tmp + 8);
    buf.push_back(0);
    packBE32(tmp, 0); buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(1);
    packBE32(tmp, 1); buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(0);
    packBE32(tmp, 0); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, 0); buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(0);
    buf.push_back(0);
    packBE32(tmp, 0u);
    buf.insert(buf.end(), tmp, tmp + 4);
    packBE16(tmp, static_cast<uint16_t>(kLceXzSize));
    buf.insert(buf.end(), tmp, tmp + 2);
    buf.push_back(kLceHellScale);
    packBE32(tmp, static_cast<uint32_t>(kLceViewDistance));
    buf.insert(buf.end(), tmp, tmp + 4);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendBlockRegionUpdatePacket(int32_t chunkX, int32_t chunkZ,
                                                  const std::vector<uint8_t>& lceBuf) {
    static thread_local bool s_compInit = false;
    if (!s_compInit) {
        Compression::CreateNewThreadStorage();
        s_compInit = true;
    }
    const unsigned int srcSize = static_cast<unsigned int>(lceBuf.size());
    unsigned int destSize = srcSize * kLzxRleScratchFactor;
    std::vector<uint8_t> compressed(destSize);
    int rc = Compression::getCompression()->CompressLZXRLE(
        compressed.data(), &destSize,
        const_cast<uint8_t*>(lceBuf.data()), srcSize);
    if (rc != 0) {
        fprintf(stderr, "[JEProxy] CompressLZXRLE failed rc=%d\n", rc);
        return false;
    }
    if (destSize > 0x3fffffff) {
        fprintf(stderr, "[JEProxy] compressed chunk > 1GiB?? (%u)\n", destSize);
        return false;
    }
    std::vector<uint8_t> buf;
    buf.reserve(20 + destSize);
    buf.push_back(kLceIdBRUP);
    buf.push_back(0x01);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(chunkX * 16));
    buf.insert(buf.end(), tmp, tmp + 4);
    packBE16(tmp, 0);
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE32(tmp, static_cast<uint32_t>(chunkZ * 16));
    buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(15);
    buf.push_back(255);
    buf.push_back(15);
    packBE32(tmp, destSize & 0x3fffffffu);
    buf.insert(buf.end(), tmp, tmp + 4);
    buf.insert(buf.end(), compressed.begin(), compressed.begin() + destSize);
    return writeAll(buf.data(), buf.size());
}

void JavaServerProxy::emitFlatFallback() {
    std::vector<uint8_t> buf(164096, 0);
    for (int i = 131072; i < 163840; ++i) buf[i] = 0xFF;
    for (int i = 163840; i < 164096; ++i) buf[i] = 1;
    int floorY = static_cast<int>(std::floor(m_spawnY)) - 2;
    if (floorY < 4) floorY = 4;
    if (floorY > 250) floorY = 250;
    for (int y = floorY - 3; y <= floorY; ++y) {
        for (int z = 0; z < 16; ++z) {
            for (int x = 0; x < 16; ++x) {
                buf[(y << 8) | (z << 4) | x] = 1;
            }
        }
    }
    const int scx = static_cast<int>(std::floor(m_spawnX / 16.0));
    const int scz = static_cast<int>(std::floor(m_spawnZ / 16.0));
    for (int cx = scx - 2; cx <= scx + 2; ++cx) {
        for (int cz = scz - 2; cz <= scz + 2; ++cz) {
            if (sendBlockRegionUpdatePacket(cx, cz, buf)) {
                m_chunksSent.fetch_add(1);
            }
        }
    }
}

bool JavaServerProxy::sendChatPacket(const std::wstring& text) {
    std::vector<uint8_t> buf;
    buf.reserve(8 + text.size() * 2);
    buf.push_back(kLceIdChat);
    uint8_t tmp[2];
    packBE16(tmp, 0);
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, (1u << 4) | 0u);
    buf.insert(buf.end(), tmp, tmp + 2);
    appendLceUtf(buf, text, lce_str::kChat);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendMovePlayerPosRot(double x, double y, double z,
                                           double yView, float yRot, float xRot,
                                           bool onGround, bool isFlying) {
    std::vector<uint8_t> buf;
    buf.reserve(50);
    buf.push_back(kLceIdMovePlayerPR);
    uint8_t tmp[8];
    auto putF64 = [&](double d) {
        uint64_t u; memcpy(&u, &d, 8); packBE64(tmp, u);
        buf.insert(buf.end(), tmp, tmp + 8);
    };
    auto putF32 = [&](float f) {
        uint32_t u; memcpy(&u, &f, 4); packBE32(tmp, u);
        buf.insert(buf.end(), tmp, tmp + 4);
    };
    putF64(x); putF64(y); putF64(yView); putF64(z);
    putF32(yRot); putF32(xRot);
    uint8_t flags = (onGround ? 0x01 : 0) | (isFlying ? 0x02 : 0);
    buf.push_back(flags);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendAddMobPacket(int id, uint8_t type,
                                       int x, int y, int z,
                                       int8_t yaw, int8_t pitch, int8_t headYaw) {
    std::vector<uint8_t> buf;
    buf.reserve(32);
    buf.push_back(kLceIdAddMob);
    uint8_t tmp[4];
    packBE16(tmp, static_cast<uint16_t>(id));
    buf.insert(buf.end(), tmp, tmp + 2);
    buf.push_back(type);
    packBE32(tmp, static_cast<uint32_t>(x)); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(y)); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(z)); buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(static_cast<uint8_t>(yaw));
    buf.push_back(static_cast<uint8_t>(pitch));
    buf.push_back(static_cast<uint8_t>(headYaw));
    packBE16(tmp, 0); buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, 0); buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, 0); buf.insert(buf.end(), tmp, tmp + 2);
    buf.push_back(0x00);
    buf.push_back(0x00);
    buf.push_back(0x7F);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendAddEntityPacket(int id, uint8_t type,
                                          int x, int y, int z,
                                          int8_t yawByte, int8_t pitchByte,
                                          int32_t data,
                                          int16_t vx, int16_t vy, int16_t vz) {
    std::vector<uint8_t> buf;
    buf.reserve(28);
    buf.push_back(kLceIdAddEntity);
    uint8_t tmp[4];
    packBE16(tmp, static_cast<uint16_t>(id));
    buf.insert(buf.end(), tmp, tmp + 2);
    buf.push_back(type);
    packBE32(tmp, static_cast<uint32_t>(x)); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(y)); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(z)); buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(static_cast<uint8_t>(yawByte));
    buf.push_back(static_cast<uint8_t>(pitchByte));
    packBE32(tmp, static_cast<uint32_t>(data));
    buf.insert(buf.end(), tmp, tmp + 4);
    if (data > -1) {
        packBE16(tmp, static_cast<uint16_t>(vx)); buf.insert(buf.end(), tmp, tmp + 2);
        packBE16(tmp, static_cast<uint16_t>(vy)); buf.insert(buf.end(), tmp, tmp + 2);
        packBE16(tmp, static_cast<uint16_t>(vz)); buf.insert(buf.end(), tmp, tmp + 2);
    }
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendRemoveEntitiesPacket(const std::vector<int>& ids) {
    if (ids.empty()) return true;
    std::vector<uint8_t> buf;
    buf.reserve(2 + ids.size() * 4);
    buf.push_back(kLceIdRemoveEntity);
    buf.push_back(static_cast<uint8_t>(std::min<size_t>(ids.size(), 255u)));
    uint8_t tmp[4];
    for (size_t i = 0; i < ids.size() && i < 255; ++i) {
        packBE32(tmp, static_cast<uint32_t>(ids[i]));
        buf.insert(buf.end(), tmp, tmp + 4);
    }
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendTeleportEntityPacket(int id, int x, int y, int z,
                                               int8_t yaw, int8_t pitch) {
    std::vector<uint8_t> buf;
    buf.reserve(20);
    buf.push_back(kLceIdTeleportEnt);
    uint8_t tmp[4];
    packBE16(tmp, static_cast<uint16_t>(id));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE32(tmp, static_cast<uint32_t>(x)); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(y)); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(z)); buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(static_cast<uint8_t>(yaw));
    buf.push_back(static_cast<uint8_t>(pitch));
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendMoveEntityPosRot(int id, int8_t dx, int8_t dy, int8_t dz,
                                           int8_t yaw, int8_t pitch) {
    std::vector<uint8_t> buf;
    buf.reserve(9);
    buf.push_back(kLceIdMoveEntity_PR);
    uint8_t tmp[2];
    packBE16(tmp, static_cast<uint16_t>(id));
    buf.insert(buf.end(), tmp, tmp + 2);
    buf.push_back(static_cast<uint8_t>(dx));
    buf.push_back(static_cast<uint8_t>(dy));
    buf.push_back(static_cast<uint8_t>(dz));
    buf.push_back(static_cast<uint8_t>(yaw));
    buf.push_back(static_cast<uint8_t>(pitch));
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendMoveEntityRot(int id, int8_t yaw, int8_t pitch) {
    std::vector<uint8_t> buf;
    buf.reserve(5);
    buf.push_back(32);
    uint8_t tmp[2];
    packBE16(tmp, static_cast<uint16_t>(id));
    buf.insert(buf.end(), tmp, tmp + 2);
    buf.push_back(static_cast<uint8_t>(yaw));
    buf.push_back(static_cast<uint8_t>(pitch));
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendRotateHeadPacket(int id, int8_t yHeadRot) {
    std::vector<uint8_t> buf;
    buf.reserve(6);
    buf.push_back(35);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(id));
    buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(static_cast<uint8_t>(yHeadRot));
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendSetEntityMotionPacket(int id, int16_t vx,
                                                int16_t vy, int16_t vz) {
    std::vector<uint8_t> buf;
    buf.reserve(9);
    buf.push_back(28);
    uint8_t tmp[2];
    packBE16(tmp, static_cast<uint16_t>(id));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, static_cast<uint16_t>(vx));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, static_cast<uint16_t>(vy));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, static_cast<uint16_t>(vz));
    buf.insert(buf.end(), tmp, tmp + 2);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendAnimatePacket(int entityId, uint8_t animation) {
    std::vector<uint8_t> buf;
    buf.reserve(6);
    buf.push_back(kLceIdAnimate);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(entityId));
    buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(animation);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendEntityEventPacket(int entityId, uint8_t eventId) {
    std::vector<uint8_t> buf;
    buf.reserve(6);
    buf.push_back(kLceIdEntityEvent);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(entityId));
    buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(eventId);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendLevelSoundPacket(int soundId, double x, double y,
                                           double z, float volume, float pitch) {
    std::vector<uint8_t> buf;
    buf.reserve(25);
    buf.push_back(kLceIdLevelSound);
    uint8_t tmp[4];
    auto putI32 = [&](int32_t v) {
        packBE32(tmp, static_cast<uint32_t>(v));
        buf.insert(buf.end(), tmp, tmp + 4);
    };
    auto putF32 = [&](float f) {
        uint32_t u;
        std::memcpy(&u, &f, 4);
        packBE32(tmp, u);
        buf.insert(buf.end(), tmp, tmp + 4);
    };
    putI32(soundId);
    putI32(static_cast<int32_t>(std::floor(x * 8.0)));
    putI32(static_cast<int32_t>(std::floor(y * 8.0)));
    putI32(static_cast<int32_t>(std::floor(z * 8.0)));
    putF32(volume);
    putF32(pitch);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendSetTimePacket(int64_t gameTime, int64_t dayTime) {
    std::vector<uint8_t> buf;
    buf.reserve(17);
    buf.push_back(kLceIdSetTime);
    uint8_t tmp[8];
    packBE64(tmp, static_cast<uint64_t>(gameTime));
    buf.insert(buf.end(), tmp, tmp + 8);
    packBE64(tmp, static_cast<uint64_t>(dayTime));
    buf.insert(buf.end(), tmp, tmp + 8);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendSetExperiencePacket(float progress, int16_t level,
                                              int16_t total) {
    std::vector<uint8_t> buf;
    buf.reserve(9);
    buf.push_back(kLceIdSetExperience);
    uint8_t tmp[4];
    uint32_t u;
    std::memcpy(&u, &progress, 4);
    packBE32(tmp, u);
    buf.insert(buf.end(), tmp, tmp + 4);
    packBE16(tmp, static_cast<uint16_t>(level));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, static_cast<uint16_t>(total));
    buf.insert(buf.end(), tmp, tmp + 2);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendGameEventPacket(uint8_t event, uint8_t param) {
    std::vector<uint8_t> buf;
    buf.reserve(3);
    buf.push_back(kLceIdGameEvent);
    buf.push_back(event);
    buf.push_back(param);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendRespawnPacket(int dimension, int gameMode,
                                        uint8_t difficulty) {
    std::vector<uint8_t> buf;
    buf.reserve(40);
    buf.push_back(kLceIdRespawn);
    buf.push_back(static_cast<uint8_t>(dimension & 0xFF));
    buf.push_back(static_cast<uint8_t>(gameMode & 0xFF));
    uint8_t tmp[8];
    const int kRespawnMapHeight = 256;
    packBE16(tmp, static_cast<uint16_t>(kRespawnMapHeight));
    buf.insert(buf.end(), tmp, tmp + 2);
    appendLceUtf(buf, L"default", lce_str::kLevelType);
    packBE64(tmp, 0);
    buf.insert(buf.end(), tmp, tmp + 8);
    buf.push_back(difficulty ? difficulty : 1);
    buf.push_back(0);
    packBE16(tmp, 0);
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, static_cast<uint16_t>(kLceXzSize));
    buf.insert(buf.end(), tmp, tmp + 2);
    buf.push_back(8);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendTileEventPacket(int x, int y, int z, uint8_t b0,
                                          uint8_t b1, int16_t lceBlock) {
    std::vector<uint8_t> buf;
    buf.reserve(14);
    buf.push_back(kLceIdTileEvent);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(x));
    buf.insert(buf.end(), tmp, tmp + 4);
    packBE16(tmp, static_cast<uint16_t>(y));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE32(tmp, static_cast<uint32_t>(z));
    buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(b0);
    buf.push_back(b1);
    packBE16(tmp, static_cast<uint16_t>(lceBlock));
    buf.insert(buf.end(), tmp, tmp + 2);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendExplodePacket(double x, double y, double z,
                                        float radius,
                                        const std::vector<int8_t>& offsets,
                                        float kbX, float kbY, float kbZ) {
    std::vector<uint8_t> buf;
    buf.reserve(40 + offsets.size());
    buf.push_back(kLceIdExplode);
    uint8_t tmp[8];
    auto putF64 = [&](double d) {
        uint64_t u; std::memcpy(&u, &d, 8); packBE64(tmp, u);
        buf.insert(buf.end(), tmp, tmp + 8);
    };
    auto putF32 = [&](float f) {
        uint32_t u; std::memcpy(&u, &f, 4); packBE32(tmp, u);
        buf.insert(buf.end(), tmp, tmp + 4);
    };
    buf.push_back(0);
    putF64(x); putF64(y); putF64(z);
    putF32(radius);
    int32_t count = static_cast<int32_t>(offsets.size() / 3);
    packBE32(tmp, static_cast<uint32_t>(count));
    buf.insert(buf.end(), tmp, tmp + 4);
    for (int8_t o : offsets) buf.push_back(static_cast<uint8_t>(o));
    putF32(kbX); putF32(kbY); putF32(kbZ);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendSetEntityLinkPacket(int sourceId, int destId,
                                              uint8_t linkType) {
    std::vector<uint8_t> buf;
    buf.reserve(10);
    buf.push_back(kLceIdSetEntityLink);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(sourceId));
    buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(destId));
    buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(linkType);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendSignUpdatePacket(
    int x, int y, int z, const std::vector<std::wstring>& lines) {
    std::vector<uint8_t> buf;
    buf.reserve(32);
    buf.push_back(kLceIdSignUpdate);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(x));
    buf.insert(buf.end(), tmp, tmp + 4);
    packBE16(tmp, static_cast<uint16_t>(y));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE32(tmp, static_cast<uint32_t>(z));
    buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(1);
    buf.push_back(0);
    for (int i = 0; i < 4; ++i) {
        const std::wstring& line =
            i < (int)lines.size() ? lines[i] : std::wstring();
        appendLceUtf(buf, line, lce_str::kSignLine);
    }
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendTileEditorOpenPacket(uint8_t editorType, int x, int y,
                                               int z) {
    std::vector<uint8_t> buf;
    buf.reserve(14);
    buf.push_back(kLceIdTileEditorOpen);
    buf.push_back(editorType);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(x));
    buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(y));
    buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(z));
    buf.insert(buf.end(), tmp, tmp + 4);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendContainerSetDataPacket(uint8_t windowId,
                                                 int16_t property,
                                                 int16_t value) {
    std::vector<uint8_t> buf;
    buf.reserve(6);
    buf.push_back(kLceIdContainerData);
    buf.push_back(windowId);
    uint8_t tmp[2];
    packBE16(tmp, static_cast<uint16_t>(property));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, static_cast<uint16_t>(value));
    buf.insert(buf.end(), tmp, tmp + 2);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendSetEquippedItemPacket(int entityId, int slot,
                                                int16_t itemId, uint8_t count,
                                                int16_t damage) {
    if (itemId >= 0) {
        int lce = JavaItemIdMap::toLce(itemId);
        if (lce <= 0 || lce > 2267) lce = 1;
        itemId = static_cast<int16_t>(lce);
    }
    std::vector<uint8_t> buf;
    buf.reserve(16);
    buf.push_back(5);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(entityId));
    buf.insert(buf.end(), tmp, tmp + 4);
    packBE16(tmp, static_cast<uint16_t>(slot));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, static_cast<uint16_t>(itemId));
    buf.insert(buf.end(), tmp, tmp + 2);
    if (itemId >= 0) {
        buf.push_back(count);
        packBE16(tmp, static_cast<uint16_t>(damage));
        buf.insert(buf.end(), tmp, tmp + 2);
        packBE16(tmp, 0xFFFF);
        buf.insert(buf.end(), tmp, tmp + 2);
    }
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendAddPlayerPacket(int id, const std::wstring& name,
                                          int x, int y, int z,
                                          int8_t yaw, int8_t pitch,
                                          int8_t headYaw,
                                          int16_t carriedItem) {
    if (carriedItem >= 0) {
        int lce = JavaItemIdMap::toLce(carriedItem);
        if (lce <= 0 || lce > 2267) lce = 1;
        carriedItem = static_cast<int16_t>(lce);
    }
    std::vector<uint8_t> buf;
    buf.reserve(64 + name.size() * 2);
    buf.push_back(20);
    uint8_t tmp[8];
    packBE32(tmp, static_cast<uint32_t>(id));
    buf.insert(buf.end(), tmp, tmp + 4);
    appendLceUtf(buf, name, lce_str::kPlayerName);
    packBE32(tmp, static_cast<uint32_t>(x)); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(y)); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(z)); buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(static_cast<uint8_t>(yaw));
    buf.push_back(static_cast<uint8_t>(pitch));
    buf.push_back(static_cast<uint8_t>(headYaw));
    packBE16(tmp, static_cast<uint16_t>(carriedItem));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE64(tmp, 0); buf.insert(buf.end(), tmp, tmp + 8);
    packBE64(tmp, 0); buf.insert(buf.end(), tmp, tmp + 8);
    buf.push_back(0);
    packBE32(tmp, 0); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, 0); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, 0u); buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(0x00);
    buf.push_back(0x00);
    buf.push_back(0x7F);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendTileUpdatePacket(int x, int y, int z,
                                           uint8_t block, uint8_t meta) {
    std::vector<uint8_t> buf;
    buf.reserve(16);
    buf.push_back(53);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(x)); buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(static_cast<uint8_t>(y));
    packBE32(tmp, static_cast<uint32_t>(z)); buf.insert(buf.end(), tmp, tmp + 4);
    packBE16(tmp, block);
    buf.insert(buf.end(), tmp, tmp + 2);
    buf.push_back(static_cast<uint8_t>((0 << 4) | (meta & 0xF)));
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendSetHealthPacket(float health, int16_t food,
                                          float saturation) {
    std::vector<uint8_t> buf;
    buf.reserve(12);
    buf.push_back(8);
    uint8_t tmp[4];
    uint32_t u;
    memcpy(&u, &health, 4);     packBE32(tmp, u);
    buf.insert(buf.end(), tmp, tmp + 4);
    packBE16(tmp, static_cast<uint16_t>(food));
    buf.insert(buf.end(), tmp, tmp + 2);
    memcpy(&u, &saturation, 4); packBE32(tmp, u);
    buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(0);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendSpawnPositionPacket(int x, int y, int z) {
    std::vector<uint8_t> buf;
    buf.reserve(13);
    buf.push_back(6);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(x)); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(y)); buf.insert(buf.end(), tmp, tmp + 4);
    packBE32(tmp, static_cast<uint32_t>(z)); buf.insert(buf.end(), tmp, tmp + 4);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendPlayerAbilitiesPacket(bool invulnerable, bool flying,
                                                bool canFly, bool instabuild) {
    uint8_t flags = 0;
    if (invulnerable) flags |= 0x01;
    if (flying)       flags |= 0x02;
    if (canFly)       flags |= 0x04;
    if (instabuild)   flags |= 0x08;
    std::vector<uint8_t> buf;
    buf.reserve(10);
    buf.push_back(202);
    buf.push_back(flags);
    uint8_t tmp[4];
    uint32_t u;
    float flySpeed = 0.05f, walkSpeed = 0.1f;
    memcpy(&u, &flySpeed, 4);  packBE32(tmp, u); buf.insert(buf.end(), tmp, tmp + 4);
    memcpy(&u, &walkSpeed, 4); packBE32(tmp, u); buf.insert(buf.end(), tmp, tmp + 4);
    return writeAll(buf.data(), buf.size());
}

void JavaServerProxy::appendLceItem(std::vector<uint8_t>& buf,
                                    const JavaInvSlot& item) {
    uint8_t tmp[2];
    if (item.id < 0) {
        packBE16(tmp, static_cast<uint16_t>(static_cast<int16_t>(-1)));
        buf.insert(buf.end(), tmp, tmp + 2);
        return;
    }
    int lceId = JavaItemIdMap::toLce(item.id);
    if (lceId <= 0 || lceId > 2267) lceId = 1;
    int count = item.count < 1 ? 1 : (item.count > 255 ? 255 : item.count);
    int damage = item.damage < 0 ? 0 : (item.damage > 32767 ? 32767 : item.damage);
    packBE16(tmp, static_cast<uint16_t>(lceId));
    buf.insert(buf.end(), tmp, tmp + 2);
    buf.push_back(static_cast<uint8_t>(count));
    packBE16(tmp, static_cast<uint16_t>(damage));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, static_cast<uint16_t>(static_cast<int16_t>(-1)));
    buf.insert(buf.end(), tmp, tmp + 2);
}

bool JavaServerProxy::sendContainerSetContentPacket(
    uint8_t windowId, const std::vector<JavaInvSlot>& slots) {
    std::vector<uint8_t> buf;
    buf.reserve(4 + slots.size() * 8);
    buf.push_back(0x68);
    buf.push_back(windowId);
    uint8_t tmp[2];
    packBE16(tmp, static_cast<uint16_t>(slots.size()));
    buf.insert(buf.end(), tmp, tmp + 2);
    for (const auto& s : slots) appendLceItem(buf, s);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendContainerSetSlotPacket(uint8_t windowId, int16_t slot,
                                                 const JavaInvSlot& item) {
    std::vector<uint8_t> buf;
    buf.reserve(12);
    buf.push_back(0x67);
    buf.push_back(windowId);
    uint8_t tmp[2];
    packBE16(tmp, static_cast<uint16_t>(slot));
    buf.insert(buf.end(), tmp, tmp + 2);
    appendLceItem(buf, item);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendContainerOpenPacket(uint8_t windowId, uint8_t type,
                                              uint8_t slotCount, int32_t entityId,
                                              const std::wstring& title) {
    std::vector<uint8_t> buf;
    buf.reserve(16 + title.size() * 2);
    buf.push_back(0x64);
    buf.push_back(windowId);
    buf.push_back(type);
    buf.push_back(slotCount);
    const bool customName = !title.empty();
    buf.push_back(customName ? 1 : 0);
    uint8_t tmp[4];
    if (type == 11) {
        packBE32(tmp, static_cast<uint32_t>(entityId));
        buf.insert(buf.end(), tmp, tmp + 4);
    }
    if (customName) {
        appendLceUtf(buf, title, lce_str::kWindowTitle);
    }
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendSetItemDataPacket(int entityId,
                                            const JavaInvSlot& item) {
    int lceId = (item.id >= 0) ? JavaItemIdMap::toLce(item.id) : -1;
    if (lceId <= 0 || lceId > 2267) return true;
    int count = item.count < 1 ? 1 : (item.count > 255 ? 255 : item.count);
    int damage = item.damage < 0 ? 0 : (item.damage > 32767 ? 32767 : item.damage);
    std::vector<uint8_t> buf;
    buf.reserve(16);
    buf.push_back(kLceIdEntityData);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(entityId));
    buf.insert(buf.end(), tmp, tmp + 4);
    buf.push_back(static_cast<uint8_t>((5 << 5) | 10));
    packBE16(tmp, static_cast<uint16_t>(lceId));
    buf.insert(buf.end(), tmp, tmp + 2);
    buf.push_back(static_cast<uint8_t>(count));
    packBE16(tmp, static_cast<uint16_t>(damage));
    buf.insert(buf.end(), tmp, tmp + 2);
    packBE16(tmp, static_cast<uint16_t>(static_cast<int16_t>(-1)));
    buf.insert(buf.end(), tmp, tmp + 2);
    buf.push_back(0x7F);
    return writeAll(buf.data(), buf.size());
}

bool JavaServerProxy::sendContainerClosePacket(uint8_t windowId) {
    uint8_t buf[2] = {0x65, windowId};
    return writeAll(buf, 2);
}

bool JavaServerProxy::sendContainerAckPacket(uint8_t windowId, int16_t action,
                                             bool accepted) {
    uint8_t buf[5];
    buf[0] = 0x6A;
    buf[1] = windowId;
    packBE16(buf + 2, static_cast<uint16_t>(action));
    buf[4] = accepted ? 1 : 0;
    return writeAll(buf, 5);
}

bool JavaServerProxy::sendDisconnectPacket(int reason) {
    uint8_t buf[5];
    buf[0] = kLceIdDisconnect;
    packBE32(buf + 1, static_cast<uint32_t>(reason));
    return writeAll(buf, 5);
}

bool JavaServerProxy::sendDisconnectWithTextPacket(const std::wstring& text) {
    std::vector<uint8_t> buf;
    buf.reserve(8 + text.size() * 2);
    buf.push_back(kLceIdDisconnect);
    uint8_t tmp[4];
    packBE32(tmp, static_cast<uint32_t>(kLceDisconnectCustomText));
    buf.insert(buf.end(), tmp, tmp + 4);
    appendLceUtf(buf, text, lce_str::kDisconnectText);
    return writeAll(buf.data(), buf.size());
}

void JavaServerProxy::sendGracefulDisconnect(const std::wstring& reason) {
    if (!reason.empty()) {
        sendDisconnectWithTextPacket(reason);
    } else {
        sendDisconnectPacket(jsp_mapDisconnectReason(reason));
    }
    m_stopRequested.store(true);
}


bool JavaServerProxy::handleLceMovePlayer(uint8_t pktId) {
    double newX = 0, newFeetY = 0, newViewY = 0, newZ = 0;
    float newYaw = 0, newPitch = 0;
    uint8_t flags = 0;
    bool gotPos = false, gotRot = false;

    switch (pktId) {
        case kLceIdMovePlayerBase:
            if (!readU8(flags)) return false;
            break;
        case kLceIdMovePlayerPos:
            if (!readF64(newX)) return false;
            if (!readF64(newFeetY)) return false;
            if (!readF64(newViewY)) return false;
            if (!readF64(newZ)) return false;
            if (!readU8(flags)) return false;
            gotPos = true;
            break;
        case kLceIdMovePlayerRot:
            if (!readF32(newYaw)) return false;
            if (!readF32(newPitch)) return false;
            if (!readU8(flags)) return false;
            gotRot = true;
            break;
        case kLceIdMovePlayerPR:
            if (!readF64(newX)) return false;
            if (!readF64(newFeetY)) return false;
            if (!readF64(newViewY)) return false;
            if (!readF64(newZ)) return false;
            if (!readF32(newYaw)) return false;
            if (!readF32(newPitch)) return false;
            if (!readU8(flags)) return false;
            gotPos = true;
            gotRot = true;
            break;
        default:
            return false;
    }

    if (!m_java || !m_haveLastPos) return true;

    if (gotPos) {
        m_lastFeetX = newX;
        m_lastFeetY = newFeetY;
        m_lastFeetZ = newZ;
    }
    if (gotRot) {
        m_lastYaw = newYaw;
        m_lastPitch = newPitch;
    }
    m_lastOnGround = (flags & 0x01) != 0;

    m_java->sendPlayerPosition(m_lastFeetX, m_lastFeetY, m_lastFeetZ,
                               m_lastYaw, m_lastPitch, m_lastOnGround);
    return true;
}

bool JavaServerProxy::handleLceChat() {
    int16_t msgType;
    if (!readI16(msgType)) return false;
    int16_t counts;
    if (!readI16(counts)) return false;
    const int stringCount = (counts >> 4) & 0xF;
    const int intCount = counts & 0xF;
    std::wstring firstText;
    for (int i = 0; i < stringCount; ++i) {
        std::wstring s;
        if (!readUtf16(s, 256)) return false;
        if (i == 0) firstText = s;
    }
    for (int i = 0; i < intCount; ++i) {
        int32_t junk;
        if (!readI32(junk)) return false;
    }
    if (m_java && !firstText.empty()) {
        std::string utf8;
        utf8.reserve(firstText.size());
        for (wchar_t wc : firstText) {
            unsigned int cp = static_cast<unsigned int>(wc);
            if (cp < 0x80) {
                utf8.push_back(static_cast<char>(cp));
            } else if (cp < 0x800) {
                utf8.push_back(static_cast<char>(0xC0 | (cp >> 6)));
                utf8.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            } else {
                utf8.push_back(static_cast<char>(0xE0 | (cp >> 12)));
                utf8.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                utf8.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            }
        }
        m_java->sendChat(utf8);
    }
    return true;
}

bool JavaServerProxy::skipNBytes(size_t n) {
    uint8_t scratch[256];
    while (n > 0) {
        size_t take = std::min(n, sizeof(scratch));
        if (!readAll(scratch, take)) return false;
        n -= take;
    }
    return true;
}

bool JavaServerProxy::skipLceItem() {
    int16_t itemId;
    if (!readI16(itemId)) return false;
    if (itemId < 0) return true;
    uint8_t count; int16_t damage;
    if (!readU8(count) || !readI16(damage)) return false;
    int16_t nbtSize;
    if (!readI16(nbtSize)) return false;
    if (nbtSize <= 0) return true;
    return skipNBytes(static_cast<size_t>(nbtSize));
}

bool JavaServerProxy::consumeLceBody(uint8_t pktId, bool& consumed) {
    consumed = true;
    switch (pktId) {
        case 0: {
            int32_t kid;
            if (!readI32(kid)) return false;
            return true;
        }
        case kLceIdMovePlayerBase:
        case kLceIdMovePlayerPos:
        case kLceIdMovePlayerRot:
        case kLceIdMovePlayerPR:
            return handleLceMovePlayer(pktId);
        case kLceIdChat:
            return handleLceChat();
        case 7: {
            int32_t s, t; uint8_t a;
            if (!readI32(s) || !readI32(t) || !readU8(a)) return false;
            if (m_java) {
                int32_t javaTarget = -1;
                for (auto& p : m_entityMap) {
                    if (p.second == t) { javaTarget = p.first; break; }
                }
                if (javaTarget >= 0) {
                    const uint8_t javaAction = (a == 1) ? 1u : 0u;
                    m_java->sendUseEntity(javaTarget, javaAction);
                }
            }
            return true;
        }
        case 14: {
            uint8_t action, y, face;
            int32_t x, z;
            if (!readU8(action) || !readI32(x) || !readU8(y) ||
                !readI32(z) || !readU8(face)) return false;
            if (m_java) {
                m_java->sendBlockDig(x, static_cast<int>(y), z, action, face);
            }
            return true;
        }
        case 15: {
            int32_t x, z; uint8_t y, face;
            if (!readI32(x) || !readU8(y) || !readI32(z) || !readU8(face)) {
                return false;
            }
            int16_t itemId;
            if (!readI16(itemId)) return false;
            uint8_t itemCount = 0;
            int16_t itemDamage = 0;
            if (itemId >= 0) {
                if (!readU8(itemCount) || !readI16(itemDamage)) return false;
                int16_t nbtSize;
                if (!readI16(nbtSize)) return false;
                if (nbtSize > 0 && !skipNBytes(static_cast<size_t>(nbtSize))) {
                    return false;
                }
            }
            uint8_t cX, cY, cZ;
            if (!readU8(cX) || !readU8(cY) || !readU8(cZ)) return false;
            if (m_java) {
                auto toJavaCursor = [](uint8_t b) -> uint8_t {
                    int v = b / 2;
                    return static_cast<uint8_t>(v > 15 ? 15 : v);
                };
                int16_t javaItem = -1;
                if (itemId >= 0) {
                    const int j = JavaItemIdMap::toJava(itemId);
                    javaItem = (j >= 0) ? static_cast<int16_t>(j) : -1;
                }
                const bool isBlockPlace = (face <= 5) && (x != -1) && (z != -1);
                if (isBlockPlace) {
                    m_java->sendBlockPlace(x, static_cast<int>(y), z, face,
                                           javaItem, itemCount, itemDamage,
                                           toJavaCursor(cX), toJavaCursor(cY),
                                           toJavaCursor(cZ));
                } else if (javaItem >= 0) {
                    m_java->sendBlockPlace(-1, 255, -1, 0xFF,
                                           javaItem, itemCount, itemDamage,
                                           0, 0, 0);
                }
            }
            return true;
        }
        case 16: {
            int16_t slot;
            if (!readI16(slot)) return false;
            if (m_java && slot >= 0 && slot <= 8) {
                m_java->sendHeldItem(slot);
            }
            return true;
        }
        case 18: {
            int32_t aid; uint8_t action;
            if (!readI32(aid) || !readU8(action)) return false;
            if (m_java && action == 0) {
                m_java->sendAnimation();
            }
            return true;
        }
        case 19: {
            int32_t pid; uint8_t paction; int32_t pdata;
            if (!readI32(pid) || !readU8(paction) || !readI32(pdata)) {
                return false;
            }
            if (m_java) {
                int32_t javaAction = -1;
                switch (paction) {
                    case 1: javaAction = 0; break;
                    case 2: javaAction = 1; break;
                    case 3: javaAction = 2; break;
                    case 4: javaAction = 3; break;
                    case 5: javaAction = 4; break;
                    case 8: javaAction = 5; break;
                    case 9: javaAction = 6; break;
                    default: javaAction = -1; break;
                }
                if (javaAction >= 0) {
                    m_java->sendEntityAction(pid, javaAction, pdata);
                }
            }
            return true;
        }
        case 27: {
            float xxa, yya; uint8_t jump, sneak;
            if (!readF32(xxa) || !readF32(yya) ||
                !readU8(jump) || !readU8(sneak)) return false;
            return true;
        }
        case 35: {
            int32_t rid; uint8_t y;
            if (!readI32(rid) || !readU8(y)) return false;
            return true;
        }
        case 101: {
            uint8_t cid;
            if (!readU8(cid)) return false;
            if (m_java) m_java->sendCloseWindow(cid);
            return true;
        }
        case 102: {
            uint8_t cid; int16_t slot; uint8_t btn; int16_t uid; uint8_t ct;
            if (!readU8(cid) || !readI16(slot) || !readU8(btn) ||
                !readI16(uid) || !readU8(ct)) return false;
            int16_t itemId;
            if (!readI16(itemId)) return false;
            uint8_t count = 0;
            int16_t damage = 0;
            if (itemId >= 0) {
                if (!readU8(count) || !readI16(damage)) return false;
                int16_t nbtSize;
                if (!readI16(nbtSize)) return false;
                if (nbtSize > 0 && !skipNBytes(static_cast<size_t>(nbtSize))) {
                    return false;
                }
            }
            if (m_java) {
                int16_t javaItem = -1;
                if (itemId >= 0) {
                    const int j = JavaItemIdMap::toJava(itemId);
                    javaItem = (j >= 0) ? static_cast<int16_t>(j) : -1;
                }
                m_java->sendWindowClick(cid, slot, btn, uid, ct,
                                        javaItem, count, damage);
            }
            return true;
        }
        case 106: {
            uint8_t cid; int16_t uid; uint8_t accepted;
            if (!readU8(cid) || !readI16(uid) || !readU8(accepted)) {
                return false;
            }
            return true;
        }
        case 107: {
            int16_t slot;
            if (!readI16(slot)) return false;
            int16_t itemId;
            if (!readI16(itemId)) return false;
            uint8_t count = 0;
            int16_t damage = 0;
            if (itemId >= 0) {
                if (!readU8(count) || !readI16(damage)) return false;
                int16_t nbtSize;
                if (!readI16(nbtSize)) return false;
                if (nbtSize > 0 && !skipNBytes(static_cast<size_t>(nbtSize))) {
                    return false;
                }
            }
            if (m_java) {
                int16_t javaItem = -1;
                if (itemId >= 0) {
                    const int j = JavaItemIdMap::toJava(itemId);
                    javaItem = (j >= 0) ? static_cast<int16_t>(j) : -1;
                }
                m_java->sendCreativeSlot(slot, javaItem, count, damage);
            }
            return true;
        }
        case 204: {
            int32_t vd;
            return readI32(vd);
        }
        case 205: {
            uint8_t a;
            if (!readU8(a)) return false;
            if (m_java && a == 1) m_java->sendClientStatus(0);
            return true;
        }
        case 250: {
            std::wstring ident;
            if (!readUtf16(ident, 32)) return false;
            int16_t len;
            if (!readI16(len)) return false;
            if (len > 0) return skipNBytes(static_cast<size_t>(len));
            return true;
        }
        case 9: {
            uint8_t dim, gameType, difficulty, newSeaLevel, hellScale;
            int16_t mapHeight, newEntityId, xzSize;
            int64_t seed;
            std::wstring typeName;
            if (!readU8(dim) || !readU8(gameType) || !readI16(mapHeight) ||
                !readUtf16(typeName, 16) || !readI64(seed) ||
                !readU8(difficulty) || !readU8(newSeaLevel) ||
                !readI16(newEntityId) || !readI16(xzSize) ||
                !readU8(hellScale)) {
                return false;
            }
            return true;
        }
        case 103: {
            uint8_t cid; int16_t slot;
            if (!readU8(cid) || !readI16(slot)) return false;
            return skipLceItem();
        }
        case 108: {
            uint8_t cid, btn;
            if (!readU8(cid) || !readU8(btn)) return false;
            return true;
        }
        case 130: {
            int32_t x, z; int16_t y; uint8_t v, c;
            if (!readI32(x) || !readI16(y) || !readI32(z) ||
                !readU8(v) || !readU8(c)) return false;
            for (int i = 0; i < 4; ++i) {
                std::wstring line;
                if (!readUtf16(line, 64)) return false;
            }
            return true;
        }
        case 150: {
            int16_t uid; int32_t recipe;
            return readI16(uid) && readI32(recipe);
        }
        case 151: {
            int32_t cid, offer;
            return readI32(cid) && readI32(offer);
        }
        case 152: {
            int32_t v;
            return readI32(v);
        }
        case 153: {
            uint8_t a; int32_t d;
            return readU8(a) && readI32(d);
        }
        case 154: {
            std::wstring name;
            if (!readUtf16(name, 64)) return false;
            int16_t db;
            if (!readI16(db)) return false;
            if (db > 0) return skipNBytes(static_cast<size_t>(db));
            return true;
        }
        case 157: {
            int32_t id; uint8_t a;
            if (!readI32(id) || !readU8(a)) return false;
            std::wstring path;
            return readUtf16(path, 256);
        }
        case 159: {
            uint8_t v;
            return readU8(v);
        }
        case 166: {
            uint8_t a; int32_t x, z;
            return readU8(a) && readI32(x) && readI32(z);
        }
        case 167: {
            int32_t cmd; int16_t len;
            if (!readI32(cmd) || !readI16(len)) return false;
            if (len > 0) return skipNBytes(static_cast<size_t>(len));
            return true;
        }
        case 201: {
            uint8_t s; int16_t c; int32_t p, e;
            return readU8(s) && readI16(c) && readI32(p) && readI32(e);
        }
        case 202: {
            uint8_t flags; float fly, walk;
            return readU8(flags) && readF32(fly) && readF32(walk);
        }
        case 254: {
            return true;
        }
        case kLceIdDisconnect: {
            int32_t reason;
            if (!readI32(reason)) {}
            return true;
        }
        default:
            consumed = false;
            return true;
    }
}


int JavaServerProxy::allocateLceEntityId(int32_t javaId) {
    int existing = lookupLceEntityId(javaId);
    if (existing > 0) return existing;
    int id = m_nextLceEntityId++;
    if (id == kLceProtocolVersion) id = m_nextLceEntityId++;
    if (id >= 32760) {
        return -1;
    }
    m_entityMap.emplace_back(javaId, id);
    return id;
}

int JavaServerProxy::lookupLceEntityId(int32_t javaId) {
    for (auto& p : m_entityMap) if (p.first == javaId) return p.second;
    return -1;
}

void JavaServerProxy::releaseJavaEntityId(int32_t javaId) {
    for (auto it = m_entityMap.begin(); it != m_entityMap.end(); ++it) {
        if (it->first == javaId) {
            m_entityMap.erase(it);
            return;
        }
    }
}


bool JavaServerProxy::runSpawnHandshake(
    const std::wstring& username,
    std::vector<JavaConnectionEvent>& pendingChunks) {
    bool clientSettingsSent = false;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(20);
    while (!m_stopRequested.load() &&
           std::chrono::steady_clock::now() < deadline && !m_haveSpawn) {
        std::vector<JavaConnectionEvent> events;
        m_java->drainEvents(events);
        for (auto& ev : events) {
            switch (ev.type) {
                case JavaConnectionEventType::JoinGame:
                    m_javaGameMode = ev.joinGameMode;
                    m_javaDimension = ev.joinDimension;
                    if (m_java && !clientSettingsSent) {
                        m_java->sendClientSettings();
                        m_java->sendBrand();
                        m_java->sendHeldItem(0);
                        clientSettingsSent = true;
                    }
                    break;
                case JavaConnectionEventType::ChunkData:
                case JavaConnectionEventType::WindowItems:
                case JavaConnectionEventType::SetSlot:
                    pendingChunks.push_back(std::move(ev));
                    break;
                case JavaConnectionEventType::PlayerTeleport:
                    m_spawnX = ev.posX;
                    m_spawnY = ev.posY;
                    m_spawnZ = ev.posZ;
                    m_spawnYaw = ev.posYaw;
                    m_spawnPitch = ev.posPitch;
                    m_haveSpawn = true;
                    break;
                case JavaConnectionEventType::Disconnected:
                    fprintf(stderr,
                            "[JLOGIN] handshake aborting: Java side reported "
                            "Disconnected -> reason: %ls\n",
                            ev.text.c_str());
                    m_pendingDisconnectReason = ev.text;
                    return false;
                default:
                    break;
            }
            if (m_haveSpawn) break;
        }
        if (m_java->state() == JavaConnectionState::Disconnected && !m_haveSpawn) {
            fprintf(stderr,
                    "[JLOGIN] handshake aborting: Java connection entered "
                    "Disconnected state before spawn (no PlayerPositionAndLook "
                    "received)\n");
            return false;
        }
        if (!m_haveSpawn) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
    if (!m_haveSpawn) {
        fprintf(stderr,
                "[JLOGIN] handshake aborting: 20s deadline reached without a "
                "PlayerPositionAndLook (Java state=%d). Server reached login but "
                "never streamed a spawn.\n",
                static_cast<int>(m_java ? m_java->state()
                                        : JavaConnectionState::Disconnected));
        return false;
    }

    if (m_java && !clientSettingsSent) {
        m_java->sendClientSettings();
        m_java->sendBrand();
        m_java->sendHeldItem(0);
    }
    if (!sendLoginPacket(username)) return false;
    m_loginSent.store(true);

    sendSpawnPositionPacket(static_cast<int>(std::floor(m_spawnX)),
                            static_cast<int>(std::floor(m_spawnY)),
                            static_cast<int>(std::floor(m_spawnZ)));

    const bool canFly = (m_javaGameMode == 1 || m_javaGameMode == 3);
    const bool instabuild = (m_javaGameMode == 1);
    sendPlayerAbilitiesPacket(false, false, canFly, instabuild);

    const double lceY = m_spawnY + kLceFeetOffset;
    sendMovePlayerPosRot(m_spawnX, lceY, m_spawnZ, lceY + kLceEyeOffset,
                         m_spawnYaw, m_spawnPitch, true, false);

    m_lastFeetX = m_spawnX;
    m_lastFeetY = m_spawnY;
    m_lastFeetZ = m_spawnZ;
    m_lastYaw = m_spawnYaw;
    m_lastPitch = m_spawnPitch;
    m_lastOnGround = true;
    m_haveLastPos = true;

    if (m_java) {
        m_java->sendPlayerPosition(m_spawnX, m_spawnY, m_spawnZ,
                                   m_spawnYaw, m_spawnPitch, true);
    }
    return true;
}


void JavaServerProxy::runWorker() {
    struct sockaddr_in cli;
    socklen_t clen = sizeof(cli);
    m_clientFd = ::accept(m_listenFd, reinterpret_cast<sockaddr*>(&cli), &clen);
    if (m_clientFd < 0) {
        fprintf(stderr, "[JEProxy] accept() failed: %s\n", strerror(errno));
        m_stopped.store(true);
        return;
    }
    int one = 1;
    setsockopt(m_clientFd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

    uint8_t id;
    if (!readU8(id) || id != kLceIdPreLogin) {
        fprintf(stderr, "[JEProxy] expected PreLoginPacket, got id=%d\n", id);
        m_stopped.store(true);
        return;
    }
    int16_t netcode; uint16_t loginKeyLen;
    if (!readI16(netcode) || !readU16(loginKeyLen)) {
        m_stopped.store(true);
        return;
    }
    std::wstring loginKey;
    for (int i = 0; i < loginKeyLen; ++i) {
        uint16_t c;
        if (!readU16(c)) { m_stopped.store(true); return; }
        loginKey.push_back(static_cast<wchar_t>(c));
    }
    uint8_t friendsOnly; int32_t ugcVer; uint8_t playerCount;
    if (!readU8(friendsOnly) || !readI32(ugcVer) || !readU8(playerCount)) {
        m_stopped.store(true); return;
    }
    for (int i = 0; i < playerCount; ++i) {
        int64_t xuid;
        if (!readI64(xuid)) { m_stopped.store(true); return; }
    }
    uint8_t saveName[14];
    if (!readAll(saveName, 14)) { m_stopped.store(true); return; }
    int32_t serverSettings; uint8_t hostIndex; int32_t texturePackId;
    if (!readI32(serverSettings) || !readU8(hostIndex) ||
        !readI32(texturePackId)) {
        m_stopped.store(true); return;
    }

    m_java = std::make_unique<JavaConnection>(m_javaHost, m_javaPort,
                                              m_nicknameUtf8);
    m_java->start();

    if (!sendPreLoginPacket(loginKey)) {
        fprintf(stderr, "[JEProxy] PreLogin response write failed\n");
        m_stopped.store(true); return;
    }

    if (!readU8(id) || id != kLceIdLogin) {
        fprintf(stderr, "[JEProxy] expected LoginPacket (id=1), got id=%d\n",
                id);
        m_stopped.store(true); return;
    }
    int32_t clientVer;
    if (!readI32(clientVer)) { m_stopped.store(true); return; }
    std::wstring userName;
    if (!readUtf16(userName, 128)) { m_stopped.store(true); return; }
    std::wstring levelTypeName;
    if (!readUtf16(levelTypeName, 16)) { m_stopped.store(true); return; }
    int64_t seed;
    int32_t gameType;
    uint8_t dimension, mapHeight, maxPlayers;
    if (!readI64(seed) || !readI32(gameType) ||
        !readU8(dimension) || !readU8(mapHeight) || !readU8(maxPlayers)) {
        m_stopped.store(true); return;
    }
    int64_t offlineXuid, onlineXuid;
    uint8_t friendsOnlyUGC;
    int32_t ugcPlayersVersion;
    uint8_t difficulty;
    int32_t multiplayerInstanceId;
    uint8_t playerIndex;
    int32_t playerSkinId, playerCapeId;
    uint8_t isGuest, newSeaLevel;
    int32_t uiGamePrivileges;
    if (!readI64(offlineXuid) || !readI64(onlineXuid) ||
        !readU8(friendsOnlyUGC) || !readI32(ugcPlayersVersion) ||
        !readU8(difficulty) || !readI32(multiplayerInstanceId) ||
        !readU8(playerIndex) || !readI32(playerSkinId) ||
        !readI32(playerCapeId) || !readU8(isGuest) || !readU8(newSeaLevel) ||
        !readI32(uiGamePrivileges)) {
        m_stopped.store(true); return;
    }
    int16_t xzSize;
    uint8_t hellScale;
    if (!readI16(xzSize) || !readU8(hellScale)) {
        m_stopped.store(true); return;
    }
    int32_t serverViewDistance;
    if (!readI32(serverViewDistance)) { m_stopped.store(true); return; }

    std::vector<JavaConnectionEvent> pendingChunks;
    if (!runSpawnHandshake(userName, pendingChunks)) {
        fprintf(stderr, "[JEProxy] spawn handshake failed; disconnecting\n");
        if (!m_pendingDisconnectReason.empty()) {
            sendDisconnectWithTextPacket(m_pendingDisconnectReason);
        } else {
            sendDisconnectPacket(
                jsp_mapDisconnectReason(m_pendingDisconnectReason));
        }
        if (m_java) m_java->requestDisconnect();
        m_stopped.store(true);
        return;
    }

    for (auto& ev : pendingChunks) {
        switch (ev.type) {
            case JavaConnectionEventType::ChunkData: {
                bool ok = sendBlockRegionUpdatePacket(ev.chunkX, ev.chunkZ,
                                                      ev.chunkBuffer);
                if (ok) {
                    m_chunksSent.fetch_add(1);
                    m_realChunkSeen = true;
                }
                break;
            }
            case JavaConnectionEventType::WindowItems:
                sendContainerSetContentPacket(
                    static_cast<uint8_t>(ev.winId), ev.invSlots);
                break;
            case JavaConnectionEventType::SetSlot:
                if (!ev.invSlots.empty()) {
                    sendContainerSetSlotPacket(
                        static_cast<uint8_t>(ev.winId),
                        static_cast<int16_t>(ev.slotIndex),
                        ev.invSlots.front());
                }
                break;
            default:
                break;
        }
    }


    m_spawnDoneTime = std::chrono::steady_clock::now();
    while (!m_stopRequested.load()) {
        if (false && !m_realChunkSeen && !m_flatFallbackSent &&
            std::chrono::steady_clock::now() - m_spawnDoneTime >
                std::chrono::milliseconds(kFlatFallbackMs)) {
            emitFlatFallback();
            m_flatFallbackSent = true;
        }
        std::vector<JavaConnectionEvent> events;
        m_java->drainEvents(events);
        for (auto& ev : events) {
            switch (ev.type) {
                case JavaConnectionEventType::JoinGame: {
                    m_javaGameMode = ev.joinGameMode;
                    m_javaDimension = ev.joinDimension;
                    const bool canFly =
                        (m_javaGameMode == 1 || m_javaGameMode == 3);
                    sendPlayerAbilitiesPacket(false, false, canFly,
                                              m_javaGameMode == 1);
                    break;
                }
                case JavaConnectionEventType::Chat: {
                    if (m_chunksSent.load() < kChunksBeforeChat) {
                        m_chatBacklog.push_back(ev.text);
                    } else {
                        sendChatPacket(ev.text);
                    }
                    break;
                }
                case JavaConnectionEventType::SystemMessage: {
                    if (m_chunksSent.load() < kChunksBeforeChat) {
                        m_chatBacklog.push_back(L"§7" + ev.text);
                    } else {
                        sendChatPacket(L"§7" + ev.text);
                    }
                    break;
                }
                case JavaConnectionEventType::Disconnected: {
                    sendGracefulDisconnect(ev.text);
                    break;
                }
                case JavaConnectionEventType::ChunkData: {
                    if (sendBlockRegionUpdatePacket(ev.chunkX, ev.chunkZ,
                                                    ev.chunkBuffer)) {
                        m_realChunkSeen = true;
                        const int n = m_chunksSent.fetch_add(1) + 1;
                        if (n == kChunksBeforeChat && !m_chatBacklog.empty()) {
                            for (auto& line : m_chatBacklog) {
                                sendChatPacket(line);
                            }
                            m_chatBacklog.clear();
                        }
                    }
                    break;
                }
                case JavaConnectionEventType::PlayerTeleport: {
                    const double lceY = ev.posY + kLceFeetOffset;
                    sendMovePlayerPosRot(ev.posX, lceY, ev.posZ,
                                         lceY + kLceEyeOffset,
                                         ev.posYaw, ev.posPitch,
                                         true, false);
                    m_lastFeetX = ev.posX;
                    m_lastFeetY = ev.posY;
                    m_lastFeetZ = ev.posZ;
                    m_lastYaw = ev.posYaw;
                    m_lastPitch = ev.posPitch;
                    m_lastOnGround = true;
                    m_haveLastPos = true;
                    if (m_java) {
                        m_java->sendPlayerPosition(ev.posX, ev.posY, ev.posZ,
                                                   ev.posYaw, ev.posPitch,
                                                   true);
                    }
                    break;
                }
                case JavaConnectionEventType::SpawnMob: {
                    int lceId = allocateLceEntityId(ev.entity.id);
                    if (lceId > 0 && ev.entity.entityType > 0 &&
                        ev.entity.entityType < 256) {
                        EntityState st;
                        st.x = ev.entity.x; st.y = ev.entity.y;
                        st.z = ev.entity.z;
                        st.yaw = ev.entity.yaw; st.pitch = ev.entity.pitch;
                        st.headYaw = ev.entity.headYaw;
                        m_entityState[ev.entity.id] = st;
                        sendAddMobPacket(
                            lceId,
                            static_cast<uint8_t>(ev.entity.entityType),
                            ev.entity.x, ev.entity.y, ev.entity.z,
                            ev.entity.yaw, ev.entity.pitch, ev.entity.headYaw);
                    }
                    break;
                }
                case JavaConnectionEventType::SpawnObject: {
                    int lceId = allocateLceEntityId(ev.entity.id);
                    if (lceId > 0 && ev.entity.entityType >= 0 &&
                        ev.entity.entityType < 256) {
                        EntityState st;
                        st.x = ev.entity.x; st.y = ev.entity.y;
                        st.z = ev.entity.z;
                        st.yaw = ev.entity.yaw; st.pitch = ev.entity.pitch;
                        st.headYaw = ev.entity.yaw;
                        m_entityState[ev.entity.id] = st;
                        sendAddEntityPacket(
                            lceId,
                            static_cast<uint8_t>(ev.entity.entityType),
                            ev.entity.x, ev.entity.y, ev.entity.z,
                            ev.entity.yaw, ev.entity.pitch,
                            ev.entity.objectData,
                            ev.entity.vx, ev.entity.vy, ev.entity.vz);
                    }
                    break;
                }
                case JavaConnectionEventType::EntityDestroy: {
                    std::vector<int> lceIds;
                    lceIds.reserve(ev.destroyIds.size());
                    for (int32_t jid : ev.destroyIds) {
                        int lceId = lookupLceEntityId(jid);
                        if (lceId > 0) lceIds.push_back(lceId);
                        releaseJavaEntityId(jid);
                        m_entityState.erase(jid);
                    }
                    if (!lceIds.empty()) sendRemoveEntitiesPacket(lceIds);
                    break;
                }
                case JavaConnectionEventType::EntityMove: {
                    auto it = m_entityState.find(ev.entity.id);
                    int lceId = lookupLceEntityId(ev.entity.id);
                    if (it != m_entityState.end() && lceId > 0) {
                        EntityState& st = it->second;
                        st.x += ev.entity.dx;
                        st.y += ev.entity.dy;
                        st.z += ev.entity.dz;
                        if (ev.entityHasRot) {
                            st.yaw = ev.entity.yaw;
                            st.pitch = ev.entity.pitch;
                            st.headYaw = ev.entity.yaw;
                        }
                        sendTeleportEntityPacket(lceId, st.x, st.y, st.z,
                                                 st.yaw, st.pitch);
                        if (ev.entityHasRot) {
                            sendRotateHeadPacket(lceId, st.headYaw);
                        }
                    }
                    break;
                }
                case JavaConnectionEventType::EntityTeleport: {
                    auto it = m_entityState.find(ev.entity.id);
                    int lceId = lookupLceEntityId(ev.entity.id);
                    if (lceId > 0) {
                        if (it != m_entityState.end()) {
                            it->second.x = ev.entity.x;
                            it->second.y = ev.entity.y;
                            it->second.z = ev.entity.z;
                            it->second.yaw = ev.entity.yaw;
                            it->second.pitch = ev.entity.pitch;
                            it->second.headYaw = ev.entity.yaw;
                        }
                        sendTeleportEntityPacket(lceId, ev.entity.x,
                                                  ev.entity.y, ev.entity.z,
                                                  ev.entity.yaw,
                                                  ev.entity.pitch);
                        sendRotateHeadPacket(lceId, ev.entity.yaw);
                    }
                    break;
                }
                case JavaConnectionEventType::SpawnPlayer: {
                    int lceId = allocateLceEntityId(ev.entity.id);
                    if (lceId > 0) {
                        std::wstring name = ev.entity.playerName.empty()
                                                ? L"Player"
                                                : ev.entity.playerName;
                        EntityState st;
                        st.x = ev.entity.x; st.y = ev.entity.y;
                        st.z = ev.entity.z;
                        st.yaw = ev.entity.yaw; st.pitch = ev.entity.pitch;
                        st.headYaw = ev.entity.headYaw;
                        m_entityState[ev.entity.id] = st;
                        sendAddPlayerPacket(lceId, name,
                                            ev.entity.x, ev.entity.y,
                                            ev.entity.z,
                                            ev.entity.yaw, ev.entity.pitch,
                                            ev.entity.headYaw,
                                            ev.equippedItemId);
                        sendRotateHeadPacket(lceId, ev.entity.headYaw);
                    }
                    break;
                }
                case JavaConnectionEventType::EntityLook: {
                    auto it = m_entityState.find(ev.entity.id);
                    int lceId = lookupLceEntityId(ev.entity.id);
                    if (it != m_entityState.end() && lceId > 0) {
                        EntityState& st = it->second;
                        st.yaw = ev.entity.yaw;
                        st.pitch = ev.entity.pitch;
                        st.headYaw = ev.entity.yaw;
                        sendTeleportEntityPacket(lceId, st.x, st.y, st.z,
                                                 st.yaw, st.pitch);
                        sendRotateHeadPacket(lceId, st.headYaw);
                    }
                    break;
                }
                case JavaConnectionEventType::EntityHeadLook: {
                    int lceId = lookupLceEntityId(ev.entity.id);
                    if (lceId > 0) {
                        auto it = m_entityState.find(ev.entity.id);
                        if (it != m_entityState.end()) {
                            it->second.headYaw = ev.entity.headYaw;
                        }
                        sendRotateHeadPacket(lceId, ev.entity.headYaw);
                    }
                    break;
                }
                case JavaConnectionEventType::EntityVelocity: {
                    int lceId = lookupLceEntityId(ev.entity.id);
                    if (lceId > 0) {
                        sendSetEntityMotionPacket(lceId, ev.entity.vx,
                                                  ev.entity.vy, ev.entity.vz);
                    }
                    break;
                }
                case JavaConnectionEventType::EntityEquipment: {
                    int lceId = lookupLceEntityId(ev.entity.id);
                    if (lceId > 0) {
                        sendSetEquippedItemPacket(lceId, ev.equippedSlot,
                                                  ev.equippedItemId,
                                                  ev.equippedItemCount,
                                                  ev.equippedItemDamage);
                    }
                    break;
                }
                case JavaConnectionEventType::EntityItemData: {
                    int lceId = lookupLceEntityId(ev.entity.id);
                    if (lceId > 0 && !ev.invSlots.empty()) {
                        sendSetItemDataPacket(lceId, ev.invSlots.front());
                    }
                    break;
                }
                case JavaConnectionEventType::BlockChange: {
                    const uint8_t lceBlock =
                        JavaBlockIdMap::toLce(static_cast<uint8_t>(
                            ev.blockId & 0xFF));
                    sendTileUpdatePacket(ev.blockX, ev.blockY, ev.blockZ,
                                         lceBlock, ev.blockMeta);
                    break;
                }
                case JavaConnectionEventType::UpdateHealth: {
                    sendSetHealthPacket(ev.healthVal,
                                        static_cast<int16_t>(ev.foodVal),
                                        ev.saturationVal);
                    break;
                }
                case JavaConnectionEventType::WindowItems: {
                    sendContainerSetContentPacket(
                        static_cast<uint8_t>(ev.winId), ev.invSlots);
                    break;
                }
                case JavaConnectionEventType::SetSlot: {
                    if (!ev.invSlots.empty()) {
                        sendContainerSetSlotPacket(
                            static_cast<uint8_t>(ev.winId),
                            static_cast<int16_t>(ev.slotIndex),
                            ev.invSlots.front());
                    }
                    break;
                }
                case JavaConnectionEventType::OpenWindow: {
                    m_openWindowId = ev.winId;
                    m_openWindowIsFurnace = ev.winIsFurnace;
                    sendContainerOpenPacket(static_cast<uint8_t>(ev.winId),
                                            static_cast<uint8_t>(ev.winType),
                                            static_cast<uint8_t>(ev.winSlotCount),
                                            ev.winEntityId, ev.text);
                    break;
                }
                case JavaConnectionEventType::CloseWindow: {
                    m_openWindowId = -1;
                    m_openWindowIsFurnace = false;
                    sendContainerClosePacket(static_cast<uint8_t>(ev.winId));
                    break;
                }
                case JavaConnectionEventType::CraftProgress: {
                    if (ev.winId != m_openWindowId) break;
                    int property = ev.craftProperty;
                    if (m_openWindowIsFurnace) {
                        if (property == 0) property = 1;
                        else if (property == 1) property = 2;
                        else if (property == 2) property = 0;
                    }
                    sendContainerSetDataPacket(
                        static_cast<uint8_t>(ev.winId),
                        static_cast<int16_t>(property),
                        static_cast<int16_t>(ev.craftValue));
                    break;
                }
                case JavaConnectionEventType::ConfirmTransaction: {
                    sendContainerAckPacket(static_cast<uint8_t>(ev.winId),
                                           static_cast<int16_t>(ev.slotIndex),
                                           ev.winAccepted);
                    break;
                }
                case JavaConnectionEventType::HeldItemChange:
                    break;
                case JavaConnectionEventType::Animation: {
                    if (ev.entityIsSelf) break;
                    int lceId = lookupLceEntityId(ev.entity.id);
                    if (lceId <= 0) break;
                    uint8_t lceAnim;
                    switch (ev.animAction) {
                        case 1: lceAnim = 2; break;
                        case 2: lceAnim = 3; break;
                        case 4: lceAnim = 6; break;
                        case 5: lceAnim = 7; break;
                        case 0:
                        case 3:
                        default: lceAnim = 1; break;
                    }
                    sendAnimatePacket(lceId, lceAnim);
                    break;
                }
                case JavaConnectionEventType::EntityStatus: {
                    if (ev.entityIsSelf) {
                        if (ev.entityStatus == 2 || ev.entityStatus == 3) {
                            sendEntityEventPacket(kLceProtocolVersion,
                                                  ev.entityStatus);
                        }
                        break;
                    }
                    int lceId = lookupLceEntityId(ev.entity.id);
                    if (lceId <= 0) break;
                    sendEntityEventPacket(lceId, ev.entityStatus);
                    break;
                }
                case JavaConnectionEventType::SoundEffect: {
                    int soundId = JavaSoundIdMap::mapJavaSoundToLce(ev.soundName);
                    if (soundId < 0) break;
                    sendLevelSoundPacket(soundId, ev.soundX / 8.0,
                                         ev.soundY / 8.0, ev.soundZ / 8.0,
                                         ev.soundVolume,
                                         ev.soundPitch / 63.0f);
                    break;
                }
                case JavaConnectionEventType::Respawn: {
                    m_javaDimension = ev.joinDimension;
                    m_javaGameMode = ev.joinGameMode;
                    if (!m_entityMap.empty()) {
                        std::vector<int> ids;
                        ids.reserve(m_entityMap.size());
                        for (auto& p : m_entityMap) ids.push_back(p.second);
                        sendRemoveEntitiesPacket(ids);
                        m_entityMap.clear();
                        m_entityState.clear();
                    }
                    sendRespawnPacket(ev.joinDimension, ev.joinGameMode,
                                      ev.respawnDifficulty);
                    sendGameEventPacket(3,
                                        static_cast<uint8_t>(ev.joinGameMode));
                    bool canFly = (ev.joinGameMode == 1 || ev.joinGameMode == 3);
                    bool instabuild = (ev.joinGameMode == 1);
                    sendPlayerAbilitiesPacket(false, false, canFly, instabuild);
                    break;
                }
                case JavaConnectionEventType::GameStateChange: {
                    if (ev.gameStateReason == 1) {
                        sendGameEventPacket(1, 0);
                    } else if (ev.gameStateReason == 2) {
                        sendGameEventPacket(2, 0);
                    } else if (ev.gameStateReason == 3) {
                        m_javaGameMode = ev.gameStateGameMode;
                        sendGameEventPacket(
                            3, static_cast<uint8_t>(ev.gameStateGameMode));
                    }
                    break;
                }
                case JavaConnectionEventType::PlayerAbilities: {
                    bool invuln = (ev.abilityFlags & 0x01) != 0;
                    bool flying = (ev.abilityFlags & 0x02) != 0;
                    bool canFly = (ev.abilityFlags & 0x04) != 0;
                    bool instabuild = (m_javaGameMode == 1);
                    sendPlayerAbilitiesPacket(invuln, flying, canFly,
                                              instabuild);
                    break;
                }
                case JavaConnectionEventType::SetExperience:
                    sendSetExperiencePacket(ev.xpBar,
                                            static_cast<int16_t>(ev.xpLevel),
                                            static_cast<int16_t>(ev.xpTotal));
                    break;
                case JavaConnectionEventType::UpdateTime: {
                    int64_t day = ev.timeDay < 0 ? -ev.timeDay : ev.timeDay;
                    sendSetTimePacket(ev.timeAge, day);
                    break;
                }
                case JavaConnectionEventType::BlockAction: {
                    int lceBlock = JavaBlockIdMap::toLce(
                        static_cast<uint8_t>(ev.blockId & 0xFF));
                    sendTileEventPacket(ev.blockX, ev.blockY, ev.blockZ,
                                        ev.blockActionB0, ev.blockActionB1,
                                        static_cast<int16_t>(lceBlock));
                    break;
                }
                case JavaConnectionEventType::Explosion:
                    sendExplodePacket(ev.explodeX, ev.explodeY, ev.explodeZ,
                                      ev.explodeRadius, ev.explodeOffsets,
                                      ev.explodeMotX, ev.explodeMotY,
                                      ev.explodeMotZ);
                    break;
                case JavaConnectionEventType::AttachEntity: {
                    int riderLce = ev.entityIsSelf
                                       ? static_cast<int>(kLceProtocolVersion)
                                       : lookupLceEntityId(ev.entity.id);
                    if (riderLce <= 0) break;
                    int vehicleLce = -1;
                    if (ev.vehicleId != -1) {
                        vehicleLce = lookupLceEntityId(ev.vehicleId);
                        if (vehicleLce <= 0) vehicleLce = -1;
                    }
                    sendSetEntityLinkPacket(riderLce, vehicleLce,
                                            ev.attachLeash ? 1 : 0);
                    break;
                }
                case JavaConnectionEventType::UpdateSign:
                    sendSignUpdatePacket(ev.blockX, ev.blockY, ev.blockZ,
                                         ev.signLines);
                    break;
                case JavaConnectionEventType::OpenSignEditor:
                    sendTileEditorOpenPacket(0, ev.blockX, ev.blockY,
                                             ev.blockZ);
                    break;
                case JavaConnectionEventType::StateChanged:
                case JavaConnectionEventType::TabListReplaced:
                case JavaConnectionEventType::TabHeaderFooter:
                    break;
            }
        }

        bool lceClosed = false;
        for (int budget = 0; budget < 256; ++budget) {
            int rr = waitReadable(m_clientFd, budget == 0 ? 25 : 0);
            if (rr < 0) {
                fprintf(stderr, "[JEProxy] LCE socket error/closed\n");
                lceClosed = true;
                break;
            }
            if (rr == 0) break;
            uint8_t pktId;
            if (!readU8(pktId)) { lceClosed = true; break; }
            bool consumed = false;
            if (!consumeLceBody(pktId, consumed)) {
                fprintf(stderr,
                        "[JEProxy] socket error reading LCE body for id=%d\n",
                        pktId);
                m_stopRequested.store(true);
                lceClosed = true;
                break;
            }
            if (!consumed) {
                fprintf(stderr, "[JEProxy] dropping unknown LCE pkt id=%d\n",
                        pktId);
                m_stopRequested.store(true);
                lceClosed = true;
                break;
            }
            if (pktId == kLceIdDisconnect) {
                m_stopRequested.store(true);
                lceClosed = true;
                break;
            }
        }
        if (lceClosed) break;

        if (m_java->isFinished()) {
            sendDisconnectPacket(kLceDisconnectKicked);
            break;
        }
    }

    if (m_java) m_java->requestDisconnect();
    m_stopped.store(true);
}