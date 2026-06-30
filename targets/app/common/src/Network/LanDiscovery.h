#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace LanDiscovery {

inline constexpr uint16_t kDefaultPort = 25566;
inline constexpr uint32_t kBeaconMagic = 0x4E4C4A34u;
inline constexpr uint16_t kProtoVersion = 2;
inline constexpr int kEntryTimeoutMs = 5000;
inline constexpr int kBroadcastIntervalMs = 1500;
inline constexpr int kMaxWorldNameChars = 64;
inline constexpr int kMaxMotdChars = 64;
inline constexpr int kMaxBeaconBytes = 512;

struct DiscoveredServer {
    std::string host;
    uint16_t tcpPort = 0;
    uint16_t netVersion = 0;
    uint8_t playerCount = 0;
    uint8_t maxPlayers = 0;
    uint8_t gameMode = 0;
    bool privateGame = false;
    std::wstring worldName;
    std::wstring motd;
    std::chrono::steady_clock::time_point lastSeen;
    int lastSeenMs = 0;
};

bool Start(uint16_t port = 0);
void Stop();
void StartHostBeacon();
void StopHostBeacon();
void SetBeaconPayload(uint16_t tcpPort,
                      uint16_t netVersion,
                      uint8_t playerCount,
                      uint8_t maxPlayers,
                      uint8_t gameMode,
                      bool privateGame,
                      const std::wstring& worldName,
                      const std::wstring& motd = std::wstring());
std::vector<DiscoveredServer> GetActiveServers();

}