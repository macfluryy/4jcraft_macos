#include "RemoteNetworkPlayer.h"

#include <stdio.h>

#include <algorithm>
#include <vector>

#include "Socket.h"

std::mutex RemoteNetworkPlayer::s_mapLock;
std::unordered_map<unsigned char, RemoteNetworkPlayer*>
    RemoteNetworkPlayer::s_byId;
unsigned char RemoteNetworkPlayer::s_nextSmallId = 2;

RemoteNetworkPlayer::RemoteNetworkPlayer(unsigned char smallId, bool isHost,
                                         const std::wstring& name)
    : m_smallId(smallId),
      m_isHost(isHost),
      m_socket(nullptr),
      m_onlineName(name),
      m_uid((PlayerUID)(0x100000000ULL | smallId)) {
    std::lock_guard<std::mutex> lock(s_mapLock);
    s_byId[smallId] = this;
}

RemoteNetworkPlayer::~RemoteNetworkPlayer() {
    std::lock_guard<std::mutex> lock(s_mapLock);
    auto it = s_byId.find(m_smallId);
    if (it != s_byId.end() && it->second == this) {
        s_byId.erase(it);
    }
}

RemoteNetworkPlayer* RemoteNetworkPlayer::CreateForIncoming(const char* peerIp,
                                                            int peerPort) {
    unsigned char id;
    {
        std::lock_guard<std::mutex> lock(s_mapLock);
        id = s_nextSmallId++;
        if (s_nextSmallId == 0) s_nextSmallId = 2;  
    }
    wchar_t buf[64];
    swprintf(buf, 64, L"player-%s-%d", peerIp ? peerIp : "?", peerPort);
    return new RemoteNetworkPlayer(id, false, std::wstring(buf));
}

RemoteNetworkPlayer* RemoteNetworkPlayer::CreateForOutgoing(
    const std::string& name) {
    
    
    
    unsigned char id = 1;
    std::wstring wname(name.begin(), name.end());
    return new RemoteNetworkPlayer(id, true, wname);
}

INetworkPlayer* RemoteNetworkPlayer::LookupBySmallId(unsigned char smallId) {
    std::lock_guard<std::mutex> lock(s_mapLock);
    auto it = s_byId.find(smallId);
    if (it != s_byId.end()) return it->second;
    return nullptr;
}

int RemoteNetworkPlayer::GetActiveCount() {
    std::lock_guard<std::mutex> lock(s_mapLock);
    return (int)s_byId.size();
}

INetworkPlayer* RemoteNetworkPlayer::GetByActiveIndex(int activeIndex) {
    std::lock_guard<std::mutex> lock(s_mapLock);
    if (activeIndex < 0 || (size_t)activeIndex >= s_byId.size()) {
        return nullptr;
    }
    
    
    
    std::vector<unsigned char> ids;
    ids.reserve(s_byId.size());
    for (auto& kv : s_byId) ids.push_back(kv.first);
    std::sort(ids.begin(), ids.end());
    return s_byId[ids[activeIndex]];
}

void RemoteNetworkPlayer::SendData(INetworkPlayer* player, const void* pvData,
                                   int dataSize, bool lowPriority, bool ack) {
    
    
    
    
    if (m_socket == nullptr || pvData == nullptr || dataSize <= 0) return;

    
    
    
    
    
    fprintf(stderr,
            "[RemoteNetworkPlayer] SendData fallback invoked for player %d "
            "(%d bytes) - ignored\n",
            (int)m_smallId, dataSize);
}
