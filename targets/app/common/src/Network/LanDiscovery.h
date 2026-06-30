// 4J macOS - LAN game discovery via UDP broadcast.
//
// The original codebase had a Xbox Live - based session search ( XSESSION_*,
// XNQOS) that's all stubbed out on macOS. To bring back something useful we
// run a tiny UDP broadcaster / listener on a dedicated port (default 25566).
//
//   * The host periodically (~ 1500 ms) sends a small UDP packet to
//     255.255.255.255:25566 advertising the world.
//   * Each running client listens on the same port and caches the most
//     recently seen beacons keyed by source IP + port.
//
// Entries that haven't been refreshed for kEntryTimeoutMs are dropped so a
// host that quit shows up gone within a few seconds.
//
// Beacon wire format (little-endian):
//   uint32  magic        = '4JLN'  (0x4E4C4A34)
//   uint16  protoVersion = 2
//   uint16  netVersion          // VER_NETWORK
//   uint16  tcpPort             // where to TCP connect
//   uint8   playerCount
//   uint8   maxPlayers
//   uint8   gameMode            // eGameType
//   uint8   flags               // bit0 = privateGame
//   uint16  worldNameLen        // wchar count, capped at 64
//   wchar_t worldName[worldNameLen]   // wchar_t = 4 bytes on macOS
//   uint16  motdLen             // wchar count, capped at 64 (proto >= 2)
//   wchar_t motd[motdLen]
//
// We keep the wchar_t-on-the-wire choice because both sides are macOS, and
// it matches how the rest of the codebase represents level names.

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
inline constexpr uint32_t kBeaconMagic = 0x4E4C4A34u;  // '4JLN'
inline constexpr uint16_t kProtoVersion = 2;
inline constexpr int kEntryTimeoutMs = 5000;
inline constexpr int kBroadcastIntervalMs = 1500;
inline constexpr int kMaxWorldNameChars = 64;
inline constexpr int kMaxMotdChars = 64;
inline constexpr int kMaxBeaconBytes = 512;

struct DiscoveredServer {
    std::string host;          // dotted-quad IPv4 (or IPv6 textual) of sender
    uint16_t tcpPort = 0;
    uint16_t netVersion = 0;
    uint8_t playerCount = 0;
    uint8_t maxPlayers = 0;
    uint8_t gameMode = 0;
    bool privateGame = false;
    std::wstring worldName;
    std::wstring motd;
    std::chrono::steady_clock::time_point lastSeen;
    // 4J macOS - rough freshness indicator (ms since this server's beacon
    // was last received). Not a true network RTT - the host broadcasts
    // every kBroadcastIntervalMs, so a healthy LAN server sits well under
    // that. A climbing value means we're missing beacons (congestion /
    // host stalling). GetActiveServers() fills this in at query time.
    int lastSeenMs = 0;
};

// Start the listener. Idempotent. Returns false if the socket couldn't be
// opened (port in use by another instance). 'port' = 0 means use kDefaultPort.
bool Start(uint16_t port = 0);

// Stop and close the listener thread / socket. Idempotent.
void Stop();

// Begin host-mode beaconing. Caller is responsible for re-calling
// SetBeaconPayload() whenever the ad-data changes (new player joined,
// world re-renamed, etc). Once active, we send the current payload
// every ~kBroadcastIntervalMs.
void StartHostBeacon();
void StopHostBeacon();

// Set the payload that will be sent in subsequent broadcasts. Safe to call
// from any thread. Cheap (just copies into a mutex-guarded snapshot).
void SetBeaconPayload(uint16_t tcpPort,
                      uint16_t netVersion,
                      uint8_t playerCount,
                      uint8_t maxPlayers,
                      uint8_t gameMode,
                      bool privateGame,
                      const std::wstring& worldName,
                      const std::wstring& motd = std::wstring());

// Returns a snapshot of all servers seen within the last kEntryTimeoutMs.
// Safe to call from any thread, reasonably cheap (copies a small vector).
std::vector<DiscoveredServer> GetActiveServers();

}  // namespace LanDiscovery
