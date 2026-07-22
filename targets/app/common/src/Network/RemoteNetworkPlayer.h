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
    
    
    
    
    static RemoteNetworkPlayer* CreateForIncoming(const char* peerIp,
                                                  int peerPort);
    static RemoteNetworkPlayer* CreateForOutgoing(const std::string& name);

    
    
    static INetworkPlayer* LookupBySmallId(unsigned char smallId);

    
    
    static int GetActiveCount();
    static INetworkPlayer* GetByActiveIndex(int activeIndex);

    ~RemoteNetworkPlayer();

    
    unsigned char GetSmallId() override { return m_smallId; }
    void SendData(INetworkPlayer* player, const void* pvData, int dataSize,
                  bool lowPriority, bool ack) override;
    bool IsSameSystem(INetworkPlayer* player) override {
        return player == this ||
               (player != nullptr && player->GetUID() == m_uid);
    }
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
    
    
    
    
    
    
    int GetSessionIndex() override { return (int)m_smallId - 1; }
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

    
    static std::mutex s_mapLock;
    static std::unordered_map<unsigned char, RemoteNetworkPlayer*> s_byId;
    
    static unsigned char s_nextSmallId;
};
