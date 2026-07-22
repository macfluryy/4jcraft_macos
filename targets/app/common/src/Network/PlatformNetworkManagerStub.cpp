#include "PlatformNetworkManagerStub.h"

#include <string.h>
#include <wchar.h>

#include <compare>

#include "app/common/src/BuildVer/BuildVer.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/LanDiscovery.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/mac/MacGame.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "app/include/NetTypes.h"
#include "NetworkPlayerQNet.h"
#include "RemoteNetworkPlayer.h"
#include "Socket.h"
#include "platform/C4JThread.h"

IPlatformNetworkStub* g_pPlatformNetworkManager;
INetworkPlayer* IPlatformNetworkStub::s_pRemoteHostOverride = nullptr;

void IPlatformNetworkStub::NotifyPlayerJoined(IQNetPlayer* pQNetPlayer) {
    const char* pszDescription;

    
    
    
    
    bool createFakeSocket = false;
    bool localPlayer = false;

    NetworkPlayerQNet* networkPlayer =
        (NetworkPlayerQNet*)addNetworkPlayer(pQNetPlayer);

    if (pQNetPlayer->IsLocal()) {
        localPlayer = true;
        if (pQNetPlayer->IsHost()) {
            pszDescription = "local host";
            
            

            m_machineQNetPrimaryPlayers.push_back(pQNetPlayer);
        } else {
            pszDescription = "local";

            
            
            createFakeSocket = true;
        }
    } else {
        if (pQNetPlayer->IsHost()) {
            pszDescription = "remote host";
        } else {
            pszDescription = "remote";

            
            
            if (m_pIQNet->IsHost()) {
                createFakeSocket = true;
            }
        }

        if (m_pIQNet->IsHost() && !m_bHostChanged) {
            
            bool systemHasPrimaryPlayer = false;
            for (auto it = m_machineQNetPrimaryPlayers.begin();
                 it < m_machineQNetPrimaryPlayers.end(); ++it) {
                IQNetPlayer* pQNetPrimaryPlayer = *it;
                if (pQNetPlayer->IsSameSystem(pQNetPrimaryPlayer)) {
                    systemHasPrimaryPlayer = true;
                    break;
                }
            }
            if (!systemHasPrimaryPlayer)
                m_machineQNetPrimaryPlayers.push_back(pQNetPlayer);
        }
    }
    g_NetworkManager.PlayerJoining(networkPlayer);

    if (createFakeSocket == true && !m_bHostChanged) {
        g_NetworkManager.CreateSocket(networkPlayer, localPlayer);
    }

    app.DebugPrintf("Player 0x%p \"%ls\" joined; %s; voice %i; camera %i.\n",
                    pQNetPlayer, pQNetPlayer->GetGamertag(), pszDescription,
                    (int)pQNetPlayer->HasVoice(),
                    (int)pQNetPlayer->HasCamera());

    if (m_pIQNet->IsHost()) {
        
        
        SystemFlagAddPlayer(networkPlayer);
    }

    for (int idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
        if (playerChangedCallback[idx])
            playerChangedCallback[idx](networkPlayer, false);
    }

    if (m_pIQNet->GetState() == QNET_STATE_GAME_PLAY) {
        int localPlayerCount = 0;
        for (unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
            if (m_pIQNet->GetLocalPlayerByUserIndex(idx) != nullptr)
                ++localPlayerCount;
        }

        float appTime = app.getAppTime();

        
        m_lastPlayerEventTimeStart = appTime;
    }
}

bool IPlatformNetworkStub::Initialise(
    CGameNetworkManager* pGameNetworkManager, int flagIndexSize) {
    m_pGameNetworkManager = pGameNetworkManager;
    m_flagIndexSize = flagIndexSize;
    g_pPlatformNetworkManager = this;
    
    m_pIQNet = new IQNet();
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        playerChangedCallback[i] = nullptr;
    }

    m_bLeavingGame = false;
    m_bLeaveGameOnTick = false;
    m_bHostChanged = false;

    m_bSearchResultsReady = false;
    m_bSearchPending = false;

    m_bIsOfflineGame = false;
    m_SessionsUpdatedCallback = nullptr;

    for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
        m_searchResultsCount[i] = 0;
        m_lastSearchStartTime[i] = 0;

        
        m_pSearchResults[i] = nullptr;
        m_pQoSResult[i] = nullptr;
        m_pCurrentSearchResults[i] = nullptr;
        m_pCurrentQoSResult[i] = nullptr;
        m_currentSearchResultsCount[i] = 0;
    }

    LanDiscovery::Start();

    
    return true;
}

void IPlatformNetworkStub::Terminate() {
    LanDiscovery::Stop();
    
}

int IPlatformNetworkStub::GetJoiningReadyPercentage() { return 100; }

int IPlatformNetworkStub::CorrectErrorIDS(int IDS) { return IDS; }

bool IPlatformNetworkStub::isSystemPrimaryPlayer(
    IQNetPlayer* pQNetPlayer) {
    return true;
}



void IPlatformNetworkStub::DoWork() {}

int IPlatformNetworkStub::GetPlayerCount() {
    int count = (int)m_pIQNet->GetPlayerCount();
    count += RemoteNetworkPlayer::GetActiveCount();
    return count;
}

bool IPlatformNetworkStub::ShouldMessageForFullSession() {
    return false;
}

int IPlatformNetworkStub::GetOnlinePlayerCount() { return 1; }

int IPlatformNetworkStub::GetLocalPlayerMask(int playerIndex) {
    return 1 << playerIndex;
}

bool IPlatformNetworkStub::AddLocalPlayerByUserIndex(int userIndex) {
    NotifyPlayerJoined(m_pIQNet->GetLocalPlayerByUserIndex(userIndex));
    return (m_pIQNet->AddLocalPlayerByUserIndex(userIndex) == 0);
}

bool IPlatformNetworkStub::RemoveLocalPlayerByUserIndex(int userIndex) {
    return true;
}

bool IPlatformNetworkStub::IsInStatsEnabledSession() { return true; }

bool IPlatformNetworkStub::SessionHasSpace(
    unsigned int spaceRequired ) {
    return true;
}

void IPlatformNetworkStub::SendInviteGUI(int quadrant) {}

bool IPlatformNetworkStub::IsAddingPlayer() { return false; }

bool IPlatformNetworkStub::LeaveGame(bool bMigrateHost) {
    if (m_bLeavingGame) return true;

    m_bLeavingGame = true;

    
    if (m_pIQNet->IsHost() && g_NetworkManager.ServerStoppedValid()) {
        m_pIQNet->EndGame();
        g_NetworkManager.ServerStoppedWait();
        g_NetworkManager.ServerStoppedDestroy();
    } else {
        m_pIQNet->EndGame();
        _LeaveGame(bMigrateHost,  true);
        m_bLeavingGame = false;
    }
    return true;
}

bool IPlatformNetworkStub::_LeaveGame(bool bMigrateHost,
                                             bool bLeaveRoom) {
    LanDiscovery::StopHostBeacon();
    if (Socket::IsTcpListenerRunning()) {
        fprintf(stderr, "[TCP] Stopping listener on leave-game.\n");
        Socket::StopTcpListener();
    }
    s_pRemoteHostOverride = nullptr;
    return true;
}

void IPlatformNetworkStub::HostGame(
    int localUsersMask, bool bOnlineGame, bool bIsPrivate,
    unsigned char publicSlots ,
    unsigned char privateSlots ) {
    
    
    SetLocalGame(!bOnlineGame);
    SetPrivateGame(bIsPrivate);
    SystemFlagReset();

    
    localUsersMask |= GetLocalPlayerMask(g_NetworkManager.GetPrimaryPad());

    m_bLeavingGame = false;

    m_pIQNet->HostGame();

    _HostGame(localUsersMask, publicSlots, privateSlots);
    
}

void IPlatformNetworkStub::_HostGame(
    int usersMask, unsigned char publicSlots ,
    unsigned char privateSlots ) {
    if (std::getenv("MC_NO_LISTEN") != nullptr) return;

    int port = 25565;
    if (const char* env = std::getenv("MC_LISTEN_PORT")) {
        int p = atoi(env);
        if (p > 0 && p < 65536) port = p;
    }
    if (!Socket::StartTcpListener(port)) {
        fprintf(stderr,
                "[TCP] Failed to start listener on port %d - multiplayer "
                "will be local-only.\n",
                port);
    } else {
        fprintf(stderr,
                "[TCP] Direct-connect listener ready on port %d. Tell "
                "clients to set MC_DIRECT_CONNECT=<your-ip>:%d\n",
                port, port);
        LanDiscovery::SetBeaconPayload(
            (uint16_t)port, (uint16_t)VER_NETWORK,
             1,  MINECRAFT_NET_MAX_PLAYERS,
             0,  m_bIsPrivateGame, L"");
        LanDiscovery::StartHostBeacon();
    }
}

bool IPlatformNetworkStub::_StartGame() { return true; }

int IPlatformNetworkStub::JoinGame(FriendSessionInfo* searchResult,
                                          int localUsersMask,
                                          int primaryUserIndex) {
    return CGameNetworkManager::JOINGAME_SUCCESS;
}

bool IPlatformNetworkStub::SetLocalGame(bool isLocal) {
    m_bIsOfflineGame = isLocal;

    return true;
}

void IPlatformNetworkStub::SetPrivateGame(bool isPrivate) {
    app.DebugPrintf("Setting as private game: %s\n", isPrivate ? "yes" : "no");
    m_bIsPrivateGame = isPrivate;
}

void IPlatformNetworkStub::RegisterPlayerChangedCallback(
    int iPad,
    std::function<void(INetworkPlayer* pPlayer, bool leaving)> callback) {
    playerChangedCallback[iPad] = std::move(callback);
}

void IPlatformNetworkStub::UnRegisterPlayerChangedCallback(int iPad) {
    playerChangedCallback[iPad] = nullptr;
}

void IPlatformNetworkStub::HandleSignInChange() { return; }

bool IPlatformNetworkStub::_RunNetworkGame() { return true; }

void IPlatformNetworkStub::UpdateAndSetGameSessionData(
    INetworkPlayer* pNetworkPlayerLeaving ) {
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
}

int IPlatformNetworkStub::RemovePlayerOnSocketClosedThreadProc(
    void* lpParam) {
    INetworkPlayer* pNetworkPlayer = (INetworkPlayer*)lpParam;

    Socket* socket = pNetworkPlayer->GetSocket();

    if (socket != nullptr) {
        
        socket->m_socketClosedEvent->waitForSignal(C4JThread::kInfiniteTimeout);

        
        
        pNetworkPlayer->SetSocket(nullptr);
        delete socket;
    }

    return g_pPlatformNetworkManager->RemoveLocalPlayer(pNetworkPlayer);
}

bool IPlatformNetworkStub::RemoveLocalPlayer(
    INetworkPlayer* pNetworkPlayer) {
    return true;
}

IPlatformNetworkStub::PlayerFlags::PlayerFlags(
    INetworkPlayer* pNetworkPlayer, unsigned int count) {
    
    
    
    count = (count + 8 - 1) & ~(8 - 1);
    
    this->m_pNetworkPlayer = pNetworkPlayer;
    this->flags = new unsigned char[count / 8];
    memset(this->flags, 0, count / 8);
    this->count = count;
}
IPlatformNetworkStub::PlayerFlags::~PlayerFlags() { delete[] flags; }



void IPlatformNetworkStub::SystemFlagAddPlayer(
    INetworkPlayer* pNetworkPlayer) {
    PlayerFlags* newPlayerFlags =
        new PlayerFlags(pNetworkPlayer, m_flagIndexSize);
    
    
    for (unsigned int i = 0; i < m_playerFlags.size(); i++) {
        if (pNetworkPlayer->IsSameSystem(m_playerFlags[i]->m_pNetworkPlayer)) {
            memcpy(newPlayerFlags->flags, m_playerFlags[i]->flags,
                   m_playerFlags[i]->count / 8);
            break;
        }
    }
    m_playerFlags.push_back(newPlayerFlags);
}



void IPlatformNetworkStub::SystemFlagRemovePlayer(
    INetworkPlayer* pNetworkPlayer) {
    for (unsigned int i = 0; i < m_playerFlags.size(); i++) {
        if (m_playerFlags[i]->m_pNetworkPlayer == pNetworkPlayer) {
            delete m_playerFlags[i];
            m_playerFlags[i] = m_playerFlags.back();
            m_playerFlags.pop_back();
            return;
        }
    }
}

void IPlatformNetworkStub::SystemFlagReset() {
    for (unsigned int i = 0; i < m_playerFlags.size(); i++) {
        delete m_playerFlags[i];
    }
    m_playerFlags.clear();
}



void IPlatformNetworkStub::SystemFlagSet(INetworkPlayer* pNetworkPlayer,
                                                int index) {
    if ((index < 0) || (index >= m_flagIndexSize)) return;
    if (pNetworkPlayer == nullptr) return;

    bool foundExactPlayer = false;
    for (unsigned int i = 0; i < m_playerFlags.size(); i++) {
        if (m_playerFlags[i]->m_pNetworkPlayer == pNetworkPlayer) {
            foundExactPlayer = true;
        }
        if (m_playerFlags[i]->m_pNetworkPlayer == pNetworkPlayer ||
            pNetworkPlayer->IsSameSystem(m_playerFlags[i]->m_pNetworkPlayer)) {
            m_playerFlags[i]->flags[index / 8] |= (128 >> (index % 8));
        }
    }
    if (!foundExactPlayer) {
        SystemFlagAddPlayer(pNetworkPlayer);
        for (unsigned int i = 0; i < m_playerFlags.size(); i++) {
            if (m_playerFlags[i]->m_pNetworkPlayer == pNetworkPlayer ||
                pNetworkPlayer->IsSameSystem(
                    m_playerFlags[i]->m_pNetworkPlayer)) {
                m_playerFlags[i]->flags[index / 8] |= (128 >> (index % 8));
            }
        }
    }
}


void IPlatformNetworkStub::SystemFlagClear(INetworkPlayer* pNetworkPlayer,
                                                  int index) {
    if ((index < 0) || (index >= m_flagIndexSize)) return;
    if (pNetworkPlayer == nullptr) return;

    for (unsigned int i = 0; i < m_playerFlags.size(); i++) {
        if (m_playerFlags[i]->m_pNetworkPlayer == pNetworkPlayer ||
            pNetworkPlayer->IsSameSystem(m_playerFlags[i]->m_pNetworkPlayer)) {
            m_playerFlags[i]->flags[index / 8] &= ~(128 >> (index % 8));
        }
    }
}




bool IPlatformNetworkStub::SystemFlagGet(INetworkPlayer* pNetworkPlayer,
                                                int index) {
    if ((index < 0) || (index >= m_flagIndexSize)) return false;
    if (pNetworkPlayer == nullptr) {
        return false;
    }

    for (unsigned int i = 0; i < m_playerFlags.size(); i++) {
        if (m_playerFlags[i]->m_pNetworkPlayer == pNetworkPlayer) {
            return ((m_playerFlags[i]->flags[index / 8] &
                     (128 >> (index % 8))) != 0);
        }
    }
    return false;
}

std::wstring IPlatformNetworkStub::GatherStats() { return L""; }

std::wstring IPlatformNetworkStub::GatherRTTStats() {
    std::wstring stats(L"Rtt: ");

    wchar_t stat[32];

    for (unsigned int i = 0; i < GetPlayerCount(); ++i) {
        IQNetPlayer* pQNetPlayer =
            ((NetworkPlayerQNet*)GetPlayerByIndex(i))->GetQNetPlayer();

        if (!pQNetPlayer->IsLocal()) {
            memset(stat, 0, 32 * sizeof(wchar_t));
            swprintf(stat, 32, L"%d: %d/", i, pQNetPlayer->GetCurrentRtt());
            stats.append(stat);
        }
    }
    return stats;
}

void IPlatformNetworkStub::TickSearch() {}

void IPlatformNetworkStub::SearchForGames() {}

int IPlatformNetworkStub::SearchForGamesThreadProc(void* lpParameter) {
    return 0;
}

void IPlatformNetworkStub::SetSearchResultsReady(int resultCount) {
    m_bSearchResultsReady = true;
    m_searchResultsCount[m_lastSearchPad] = resultCount;
}

std::vector<FriendSessionInfo*>* IPlatformNetworkStub::GetSessionList(
    int iPad, int localPlayers, bool partyOnly) {
    std::vector<FriendSessionInfo*>* filteredList =
        new std::vector<FriendSessionInfo*>();

    if (partyOnly) return filteredList;

    auto found = LanDiscovery::GetActiveServers();
    int idx = 0;
    for (const auto& s : found) {
        FriendSessionInfo* info = new FriendSessionInfo();

        
        
        
        
        
        SessionID id = 0;
        sockaddr_in tmp{};
        if (inet_pton(AF_INET, s.host.c_str(), &tmp.sin_addr) == 1) {
            id = ((SessionID)ntohl(tmp.sin_addr.s_addr) << 16) |
                 (SessionID)s.tcpPort;
        } else {
            id = (SessionID)(0xDEADBEEF00000000ULL | (uint32_t)idx);
        }
        info->sessionId = id;

        
        
        wchar_t buf[160] = {0};
        std::wstring worldName =
            s.worldName.empty() ? std::wstring(L"4jcraft") : s.worldName;
        std::wstring hostW;
        for (char c : s.host) hostW.push_back((wchar_t)c);
        swprintf(buf, 160, L"%ls (%ls:%u)  %u/%u", worldName.c_str(),
                 hostW.c_str(), (unsigned)s.tcpPort, (unsigned)s.playerCount,
                 (unsigned)s.maxPlayers);
        size_t labelLen = wcslen(buf);
        info->displayLabel = new wchar_t[labelLen + 1];
        wmemcpy(info->displayLabel, buf, labelLen);
        info->displayLabel[labelLen] = 0;
        info->displayLabelLength = (unsigned char)std::min<size_t>(labelLen, 255);
        info->displayLabelViewableStartIndex = 0;

        info->data.netVersion = s.netVersion;
        info->data.m_uiGameHostSettings = (unsigned int)s.gameMode;
        info->data.texturePackParentId = 0;
        info->data.subTexturePackId = 0;
        info->data.isReadyToJoin = true;
        info->hasPartyMember = false;

        filteredList->push_back(info);
        ++idx;
    }
    return filteredList;
}

bool IPlatformNetworkStub::GetGameSessionInfo(
    int iPad, SessionID sessionId, FriendSessionInfo* foundSessionInfo) {
    return false;
}

void IPlatformNetworkStub::SetSessionsUpdatedCallback(
    std::function<void()> callback) {
    m_SessionsUpdatedCallback = std::move(callback);
}

void IPlatformNetworkStub::GetFullFriendSessionInfo(
    FriendSessionInfo* foundSession,
    std::function<void(bool success)> callback) {
    callback(true);
}

void IPlatformNetworkStub::ForceFriendsSessionRefresh() {
    app.DebugPrintf("Resetting friends session search data\n");

    for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
        m_searchResultsCount[i] = 0;
        m_lastSearchStartTime[i] = 0;
        delete m_pSearchResults[i];
        m_pSearchResults[i] = nullptr;
    }
}

INetworkPlayer* IPlatformNetworkStub::addNetworkPlayer(
    IQNetPlayer* pQNetPlayer) {
    NetworkPlayerQNet* pNetworkPlayer = new NetworkPlayerQNet(pQNetPlayer);
    pQNetPlayer->SetCustomDataValue((uintptr_t)pNetworkPlayer);
    currentNetworkPlayers.push_back(pNetworkPlayer);
    return pNetworkPlayer;
}

void IPlatformNetworkStub::removeNetworkPlayer(
    IQNetPlayer* pQNetPlayer) {
    INetworkPlayer* pNetworkPlayer = getNetworkPlayer(pQNetPlayer);
    for (auto it = currentNetworkPlayers.begin();
         it != currentNetworkPlayers.end(); it++) {
        if (*it == pNetworkPlayer) {
            currentNetworkPlayers.erase(it);
            return;
        }
    }
}

INetworkPlayer* IPlatformNetworkStub::getNetworkPlayer(
    IQNetPlayer* pQNetPlayer) {
    return pQNetPlayer ? (INetworkPlayer*)(pQNetPlayer->GetCustomDataValue())
                       : nullptr;
}

INetworkPlayer* IPlatformNetworkStub::GetLocalPlayerByUserIndex(
    int userIndex) {
    return getNetworkPlayer(m_pIQNet->GetLocalPlayerByUserIndex(userIndex));
}

INetworkPlayer* IPlatformNetworkStub::GetPlayerByIndex(int playerIndex) {
    int localCount = (int)m_pIQNet->GetPlayerCount();
    if (playerIndex < localCount) {
        return getNetworkPlayer(m_pIQNet->GetPlayerByIndex(playerIndex));
    }
    return RemoteNetworkPlayer::GetByActiveIndex(playerIndex - localCount);
}

INetworkPlayer* IPlatformNetworkStub::GetPlayerByXuid(PlayerUID xuid) {
    return getNetworkPlayer(m_pIQNet->GetPlayerByXuid(xuid));
}

INetworkPlayer* IPlatformNetworkStub::GetPlayerBySmallId(
    unsigned char smallId) {
    if (INetworkPlayer* remote =
            RemoteNetworkPlayer::LookupBySmallId(smallId)) {
        return remote;
    }
    return getNetworkPlayer(m_pIQNet->GetPlayerBySmallId(smallId));
}

INetworkPlayer* IPlatformNetworkStub::GetHostPlayer() {
    if (s_pRemoteHostOverride != nullptr) {
        return s_pRemoteHostOverride;
    }
    return getNetworkPlayer(m_pIQNet->GetHostPlayer());
}

bool IPlatformNetworkStub::IsHost() {
    return m_pIQNet->IsHost() && !m_bHostChanged;
}

bool IPlatformNetworkStub::JoinGameFromInviteInfo(
    int userIndex, int userMask, const INVITE_INFO* pInviteInfo) {
    return (m_pIQNet->JoinGameFromInviteInfo(userIndex, userMask,
                                             pInviteInfo) == 0);
}

void IPlatformNetworkStub::SetSessionTexturePackParentId(int id) {
    m_hostGameSessionData.texturePackParentId = id;
}

void IPlatformNetworkStub::SetSessionSubTexturePackId(int id) {
    m_hostGameSessionData.subTexturePackId = id;
}

void IPlatformNetworkStub::Notify(int ID, uintptr_t Param) {}

bool IPlatformNetworkStub::IsInSession() {
    return m_pIQNet->GetState() != QNET_STATE_IDLE;
}

bool IPlatformNetworkStub::IsInGameplay() {
    return m_pIQNet->GetState() == QNET_STATE_GAME_PLAY;
}

bool IPlatformNetworkStub::IsReadyToPlayOrIdle() { return true; }