#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

#include "NetworkPlayerInterface.h"
#include "platform/PlatformTypes.h"

class Socket;

class RemoteNetworkPlayer : public INetworkPlayer {
public:
    // Factory helpers - both allocate a new small id and register the player
    // in the global map. `CreateForIncoming` is used on the host when a
    // remote peer connects; `CreateForOutgoing` is used on the client to
    // represent the host we're connecting to.
    static RemoteNetworkPlayer* CreateForIncoming(const char* peerIp,
                                                  int peerPort);
    static RemoteNetworkPlayer* CreateForOutgoing(const std::string& name);

    // Look up a remote player by small id - consulted by CGameNetworkManager
    // before falling back to IQNet.
    static INetworkPlayer* LookupBySmallId(unsigned char smallId);

    ~RemoteNetworkPlayer();

    // INetworkPlayer interface
    unsigned char GetSmallId() override { return m_smallId; }
    void SendData(INetworkPlayer* player, const void* pvData, int dataSize,
                  bool lowPriority, bool ack) override;
    bool IsSameSystem(INetworkPlayer* player) override { return false; }
    int GetOutstandingAckCount() override { return 0; }
    int GetSendQueueSizeBytes(INetworkPlayer* player,
                              bool lowPriority) override {
        return 0;
    }
    int GetSendQueueSizeMessages(INetworkPlayer* player,
                                 bool lowPriority) override {
        return 0;
    }
    int GetCurrentRtt() override { return 0; }
    bool IsHost() override { return m_isHost; }
    bool IsGuest() override { return !m_isHost; }
    bool IsLocal() override { return false; }
    int GetSessionIndex() override { return m_smallId; }
    bool IsTalking() override { return false; }
    bool IsMutedByLocalUser(int userIndex) override { return false; }
    bool HasVoice() override { return false; }
    bool HasCamera() override { return false; }
    int GetUserIndex() override { return -1; }
    void SetSocket(Socket* pSocket) override { m_socket = pSocket; }
    Socket* GetSocket() override { return m_socket; }
    const wchar_t* GetOnlineName() override { return m_onlineName.c_str(); }
    std::wstring GetDisplayName() override { return m_onlineName; }
    unsigned long long GetUID() override { return (unsigned long long)m_uid; }
    void SentChunkPacket() override {}
    int GetTimeSinceLastChunkPacket_ms() override { return 0; }

private:
    RemoteNetworkPlayer(unsigned char smallId, bool isHost,
                        const std::wstring& name);

    unsigned char m_smallId;
    bool m_isHost;
    Socket* m_socket;
    std::wstring m_onlineName;
    PlayerUID m_uid;

    // Global registry - small id -> player. Used by LookupBySmallId.
    static std::mutex s_mapLock;
    static std::unordered_map<unsigned char, RemoteNetworkPlayer*> s_byId;
    // Next small id to hand out. Starts at 2 (host takes 1).
    static unsigned char s_nextSmallId;
};
