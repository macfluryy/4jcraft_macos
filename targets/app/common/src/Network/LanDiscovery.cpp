#include "LanDiscovery.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <vector>

namespace LanDiscovery {

namespace {

struct BeaconPayload {
    uint16_t tcpPort = 0;
    uint16_t netVersion = 0;
    uint8_t playerCount = 0;
    uint8_t maxPlayers = 0;
    uint8_t gameMode = 0;
    bool privateGame = false;
    std::wstring worldName;
    std::wstring motd;
    bool valid = false;
};

std::atomic<bool> g_listenerRunning{false};
std::atomic<bool> g_beaconRunning{false};
std::atomic<bool> g_shouldStop{false};

int g_listenSock = -1;
int g_broadcastSock = -1;
uint16_t g_port = kDefaultPort;

std::thread g_listenThread;
std::thread g_beaconThread;
std::mutex g_payloadMutex;
BeaconPayload g_payload;

std::mutex g_serversMutex;
std::vector<DiscoveredServer> g_servers;

// Wakes the beacon thread out of its sleep so Stop() returns quickly.
std::mutex g_wakeMutex;
std::condition_variable g_wakeCv;

uint16_t readU16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}
uint32_t readU32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0] | (p[1] << 8) | (p[2] << 16) |
                                 (p[3] << 24));
}
void writeU16(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xff);
    p[1] = (uint8_t)((v >> 8) & 0xff);
}
void writeU32(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xff);
    p[1] = (uint8_t)((v >> 8) & 0xff);
    p[2] = (uint8_t)((v >> 16) & 0xff);
    p[3] = (uint8_t)((v >> 24) & 0xff);
}

// Returns the encoded byte length, or 0 on failure (e.g. payload not set).
size_t encodeBeacon(const BeaconPayload& p, uint8_t* out, size_t cap) {
    if (!p.valid) return 0;
    size_t nameChars = std::min<size_t>(p.worldName.size(), kMaxWorldNameChars);
    size_t motdChars = std::min<size_t>(p.motd.size(), kMaxMotdChars);
    size_t needed = 4 + 2 + 2 + 2 + 1 + 1 + 1 + 1 + 2 +
                    nameChars * sizeof(wchar_t) + 2 +
                    motdChars * sizeof(wchar_t);
    if (needed > cap) return 0;
    uint8_t* w = out;
    writeU32(w, kBeaconMagic);
    w += 4;
    writeU16(w, kProtoVersion);
    w += 2;
    writeU16(w, p.netVersion);
    w += 2;
    writeU16(w, p.tcpPort);
    w += 2;
    *w++ = p.playerCount;
    *w++ = p.maxPlayers;
    *w++ = p.gameMode;
    *w++ = p.privateGame ? 1 : 0;
    writeU16(w, (uint16_t)nameChars);
    w += 2;
    if (nameChars > 0) {
        memcpy(w, p.worldName.data(), nameChars * sizeof(wchar_t));
        w += nameChars * sizeof(wchar_t);
    }
    // proto >= 2: MOTD string.
    writeU16(w, (uint16_t)motdChars);
    w += 2;
    if (motdChars > 0) {
        memcpy(w, p.motd.data(), motdChars * sizeof(wchar_t));
        w += motdChars * sizeof(wchar_t);
    }
    return (size_t)(w - out);
}

// Returns true on a well-formed beacon, false if the buffer is bogus.
bool decodeBeacon(const uint8_t* in, size_t len, DiscoveredServer& out) {
    if (len < 4 + 2 + 2 + 2 + 1 + 1 + 1 + 1 + 2) return false;
    const uint8_t* r = in;
    uint32_t magic = readU32(r);
    r += 4;
    if (magic != kBeaconMagic) return false;
    uint16_t proto = readU16(r);
    r += 2;
    // Accept proto 1 (no MOTD) and proto 2 (with MOTD). Reject anything
    // newer/unknown.
    if (proto != 1 && proto != kProtoVersion) return false;
    out.netVersion = readU16(r);
    r += 2;
    out.tcpPort = readU16(r);
    r += 2;
    out.playerCount = *r++;
    out.maxPlayers = *r++;
    out.gameMode = *r++;
    uint8_t flags = *r++;
    out.privateGame = (flags & 1) != 0;
    uint16_t nameChars = readU16(r);
    r += 2;
    if (nameChars > kMaxWorldNameChars) return false;
    size_t nameBytes = (size_t)nameChars * sizeof(wchar_t);
    if ((size_t)((r - in) + nameBytes) > len) return false;
    out.worldName.assign((const wchar_t*)r, nameChars);
    r += nameBytes;
    // proto >= 2: optional MOTD. Tolerate truncated/missing for proto 1.
    out.motd.clear();
    if (proto >= 2) {
        if ((size_t)((r - in) + 2) > len) return true;  // no MOTD field
        uint16_t motdChars = readU16(r);
        r += 2;
        if (motdChars > kMaxMotdChars) return true;  // ignore bad MOTD
        size_t motdBytes = (size_t)motdChars * sizeof(wchar_t);
        if ((size_t)((r - in) + motdBytes) > len) return true;
        out.motd.assign((const wchar_t*)r, motdChars);
        r += motdBytes;
    }
    return true;
}

void mergeOrInsertServer(const DiscoveredServer& s) {
    std::lock_guard<std::mutex> lk(g_serversMutex);
    for (auto& e : g_servers) {
        if (e.host == s.host && e.tcpPort == s.tcpPort) {
            // Update in place so the entry stays at its current slot.
            DiscoveredServer copy = s;
            copy.lastSeen = std::chrono::steady_clock::now();
            e = copy;
            return;
        }
    }
    DiscoveredServer copy = s;
    copy.lastSeen = std::chrono::steady_clock::now();
    g_servers.push_back(copy);
}

void pruneExpiredServers() {
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lk(g_serversMutex);
    g_servers.erase(
        std::remove_if(
            g_servers.begin(), g_servers.end(),
            [&](const DiscoveredServer& e) {
                auto age =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - e.lastSeen)
                        .count();
                return age > kEntryTimeoutMs;
            }),
        g_servers.end());
}

void listenLoop() {
    uint8_t buf[kMaxBeaconBytes];
    while (!g_shouldStop.load()) {
        sockaddr_in src{};
        socklen_t srcLen = sizeof(src);
        ssize_t n = recvfrom(g_listenSock, buf, sizeof(buf), 0,
                             (sockaddr*)&src, &srcLen);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                pruneExpiredServers();
                continue;
            }
            // Unexpected error: surface and bail out.
            fprintf(stderr,
                    "[LAN] recvfrom failed: %s. Stopping listener.\n",
                    strerror(errno));
            break;
        }
        if (n == 0) continue;

        DiscoveredServer s;
        if (!decodeBeacon(buf, (size_t)n, s)) continue;

        char ipbuf[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &src.sin_addr, ipbuf, sizeof(ipbuf));
        s.host = ipbuf;
        mergeOrInsertServer(s);
    }
}

// 4J macOS - collect the per-interface directed broadcast addresses
// (e.g. 192.168.0.255) plus the global 255.255.255.255. Directed
// broadcasts are far more reliably delivered across machines on the
// same subnet than the limited broadcast, which some routers / the
// macOS stack drop. Skips loopback and down interfaces.
std::vector<uint32_t> collectBroadcastTargets() {
    std::vector<uint32_t> targets;
    // Always include the limited broadcast as a fallback.
    targets.push_back(INADDR_BROADCAST);

    ifaddrs* ifap = nullptr;
    if (getifaddrs(&ifap) == 0 && ifap != nullptr) {
        for (ifaddrs* ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) continue;
            if (ifa->ifa_addr->sa_family != AF_INET) continue;
            if ((ifa->ifa_flags & IFF_UP) == 0) continue;
            if (ifa->ifa_flags & IFF_LOOPBACK) continue;
            if ((ifa->ifa_flags & IFF_BROADCAST) == 0) continue;
            if (ifa->ifa_dstaddr == nullptr) continue;  // broadaddr union
            sockaddr_in* bcast = (sockaddr_in*)ifa->ifa_dstaddr;
            uint32_t addr = bcast->sin_addr.s_addr;
            if (addr == 0) continue;
            if (std::find(targets.begin(), targets.end(), addr) ==
                targets.end()) {
                targets.push_back(addr);
            }
        }
        freeifaddrs(ifap);
    }
    return targets;
}

void beaconLoop() {
    uint8_t buf[kMaxBeaconBytes];
    int refreshCounter = 0;
    std::vector<uint32_t> targets = collectBroadcastTargets();
    while (!g_shouldStop.load() && g_beaconRunning.load()) {
        BeaconPayload snapshot;
        {
            std::lock_guard<std::mutex> lk(g_payloadMutex);
            snapshot = g_payload;
        }
        // Re-enumerate interfaces every ~15 s in case the machine changes
        // network (Wi-Fi <-> ethernet, new VPN, etc).
        if (++refreshCounter >= 10) {
            refreshCounter = 0;
            targets = collectBroadcastTargets();
        }
        size_t len = encodeBeacon(snapshot, buf, sizeof(buf));
        if (len > 0 && g_broadcastSock >= 0) {
            for (uint32_t addr : targets) {
                sockaddr_in dst{};
                dst.sin_family = AF_INET;
                dst.sin_port = htons(g_port);
                dst.sin_addr.s_addr = addr;
                ssize_t sent = sendto(g_broadcastSock, buf, len, 0,
                                      (sockaddr*)&dst, sizeof(dst));
                if (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK &&
                    errno != EINTR) {
                    static int s_warnCount = 0;
                    if (s_warnCount++ < 5) {
                        char ipbuf[INET_ADDRSTRLEN] = {0};
                        inet_ntop(AF_INET, &dst.sin_addr, ipbuf,
                                  sizeof(ipbuf));
                        fprintf(stderr,
                                "[LAN] broadcast sendto %s failed: %s\n",
                                ipbuf, strerror(errno));
                    }
                }
            }
        }
        std::unique_lock<std::mutex> wakeLk(g_wakeMutex);
        g_wakeCv.wait_for(wakeLk,
                          std::chrono::milliseconds(kBroadcastIntervalMs),
                          [] { return g_shouldStop.load(); });
    }
}

bool openListenSocket() {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        fprintf(stderr, "[LAN] listen socket() failed: %s\n", strerror(errno));
        return false;
    }
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#ifdef SO_REUSEPORT
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
#endif
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(g_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr,
                "[LAN] bind(%u) failed: %s. LAN browsing disabled.\n",
                (unsigned)g_port, strerror(errno));
        close(fd);
        return false;
    }
    // Non-blocking with a tiny timeout so the loop can poll g_shouldStop.
    timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 250 * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    g_listenSock = fd;
    return true;
}

bool openBroadcastSocket() {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        fprintf(stderr, "[LAN] broadcast socket() failed: %s\n",
                strerror(errno));
        return false;
    }
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) < 0) {
        fprintf(stderr, "[LAN] SO_BROADCAST failed: %s\n", strerror(errno));
        close(fd);
        return false;
    }
    g_broadcastSock = fd;
    return true;
}

}  // namespace

bool Start(uint16_t port) {
    if (g_listenerRunning.load()) return true;
    if (port != 0) g_port = port;
    g_shouldStop.store(false);
    if (!openListenSocket()) return false;
    g_listenerRunning.store(true);
    g_listenThread = std::thread(listenLoop);
    fprintf(stderr, "[LAN] discovery listener up on UDP %u\n",
            (unsigned)g_port);

    // 4J macOS - register std::exit handler so a graceful Exit-Game (which
    // hits std::exit -> static destructors) tears the listener down before
    // ~thread runs. Without this, the std::thread destructor sees a still-
    // joinable thread and calls std::terminate, aborting the process.
    static bool s_atexitRegistered = false;
    if (!s_atexitRegistered) {
        s_atexitRegistered = true;
        std::atexit([] { Stop(); });
    }
    return true;
}

void Stop() {
    if (!g_listenerRunning.load() && !g_beaconRunning.load()) return;
    g_shouldStop.store(true);
    {
        std::lock_guard<std::mutex> lk(g_wakeMutex);
        g_wakeCv.notify_all();
    }
    if (g_beaconThread.joinable()) g_beaconThread.join();
    if (g_listenThread.joinable()) g_listenThread.join();
    if (g_listenSock >= 0) {
        close(g_listenSock);
        g_listenSock = -1;
    }
    if (g_broadcastSock >= 0) {
        close(g_broadcastSock);
        g_broadcastSock = -1;
    }
    g_listenerRunning.store(false);
    g_beaconRunning.store(false);
    {
        std::lock_guard<std::mutex> lk(g_serversMutex);
        g_servers.clear();
    }
}

void StartHostBeacon() {
    if (g_beaconRunning.load()) return;
    if (g_broadcastSock < 0 && !openBroadcastSocket()) return;
    g_beaconRunning.store(true);
    g_beaconThread = std::thread(beaconLoop);
    fprintf(stderr, "[LAN] beaconing started on UDP %u\n", (unsigned)g_port);
}

void StopHostBeacon() {
    if (!g_beaconRunning.load()) return;
    g_beaconRunning.store(false);
    {
        std::lock_guard<std::mutex> lk(g_wakeMutex);
        g_wakeCv.notify_all();
    }
    if (g_beaconThread.joinable()) g_beaconThread.join();
    if (g_broadcastSock >= 0) {
        close(g_broadcastSock);
        g_broadcastSock = -1;
    }
}

void SetBeaconPayload(uint16_t tcpPort,
                      uint16_t netVersion,
                      uint8_t playerCount,
                      uint8_t maxPlayers,
                      uint8_t gameMode,
                      bool privateGame,
                      const std::wstring& worldName,
                      const std::wstring& motd) {
    std::lock_guard<std::mutex> lk(g_payloadMutex);
    g_payload.tcpPort = tcpPort;
    g_payload.netVersion = netVersion;
    g_payload.playerCount = playerCount;
    g_payload.maxPlayers = maxPlayers;
    g_payload.gameMode = gameMode;
    g_payload.privateGame = privateGame;
    g_payload.worldName = worldName;
    g_payload.motd = motd;
    g_payload.valid = true;
}

std::vector<DiscoveredServer> GetActiveServers() {
    pruneExpiredServers();
    std::lock_guard<std::mutex> lk(g_serversMutex);
    std::vector<DiscoveredServer> out = g_servers;
    // Fill in the freshness (ms since last beacon) at query time.
    auto now = std::chrono::steady_clock::now();
    for (auto& s : out) {
        s.lastSeenMs =
            (int)std::chrono::duration_cast<std::chrono::milliseconds>(
                now - s.lastSeen)
                .count();
    }
    return out;
}

}  // namespace LanDiscovery
