#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../4J.Common/4J_Compat.h"

class INetworkPlayer;
class ClientConnection;
class Minecraft;
class CGameNetworkManager;
struct FriendSessionInfo;
using SessionID = uint64_t;

class IPlatformNetwork {
public:
    enum eJoinFailedReason {
        JOIN_FAILED_SERVER_FULL,
        JOIN_FAILED_INSUFFICIENT_PRIVILEGES,
        JOIN_FAILED_NONSPECIFIC,
    };

    virtual ~IPlatformNetwork() = default;

    virtual bool Initialise(CGameNetworkManager* pGameNetworkManager,
                            int flagIndexSize) = 0;
    virtual void Terminate() = 0;
    virtual void DoWork() = 0;

    // Players
    virtual int GetPlayerCount() = 0;
    virtual int GetOnlinePlayerCount() = 0;
    virtual int GetLocalPlayerMask(int playerIndex) = 0;
    virtual bool AddLocalPlayerByUserIndex(int userIndex) = 0;
    virtual bool RemoveLocalPlayerByUserIndex(int userIndex) = 0;
    virtual INetworkPlayer* GetLocalPlayerByUserIndex(int userIndex) = 0;
    virtual INetworkPlayer* GetPlayerByIndex(int playerIndex) = 0;
    virtual INetworkPlayer* GetPlayerByXuid(PlayerUID xuid) = 0;
    virtual INetworkPlayer* GetPlayerBySmallId(unsigned char smallId) = 0;
    virtual INetworkPlayer* GetHostPlayer() = 0;
    virtual bool ShouldMessageForFullSession() = 0;

    // Session
    virtual bool IsHost() = 0;
    virtual bool IsInSession() = 0;
    virtual bool IsInGameplay() = 0;
    virtual bool IsReadyToPlayOrIdle() = 0;
    virtual bool IsInStatsEnabledSession() = 0;
    virtual bool SessionHasSpace(unsigned int spaceRequired = 1) = 0;
    virtual bool IsAddingPlayer() = 0;
    virtual bool IsLocalGame() = 0;
    virtual void SetLocalGame(bool isLocal) = 0;
    virtual bool IsPrivateGame() = 0;
    virtual void SetPrivateGame(bool isPrivate) = 0;
    virtual bool IsLeavingGame() = 0;
    virtual void ResetLeavingGame() = 0;

    // Hosting / joining
    virtual void HostGame(int localUsersMask, bool bOnlineGame, bool bIsPrivate,
                          unsigned char publicSlots, unsigned char privateSlots) = 0;
    virtual int JoinGame(FriendSessionInfo* searchResult, int dwLocalUsersMask,
                         int dwPrimaryUserIndex) = 0;
    virtual void CancelJoinGame() {}
    virtual bool LeaveGame(bool bMigrateHost) = 0;
    virtual void SendInviteGUI(int quadrant) = 0;
    virtual int GetJoiningReadyPercentage() = 0;

    // Callbacks
    virtual void RegisterPlayerChangedCallback(
        int iPad,
        void (*callback)(void*, INetworkPlayer*, bool),
        void* callbackParam) = 0;
    virtual void UnRegisterPlayerChangedCallback(
        int iPad,
        void (*callback)(void*, INetworkPlayer*, bool),
        void* callbackParam) = 0;

    virtual void HandleSignInChange() = 0;

    // Stats
    virtual std::wstring GatherStats() = 0;
    virtual std::wstring GatherRTTStats() = 0;

    // Session list
    virtual std::vector<FriendSessionInfo*>* GetSessionList(int iPad,
                                                            int localPlayers,
                                                            bool partyOnly) = 0;
    virtual void SetSessionsUpdatedCallback(
        void (*SessionsUpdatedCallback)(void*), void* pSearchParam) = 0;
    virtual void ForceFriendsSessionRefresh() = 0;
};
