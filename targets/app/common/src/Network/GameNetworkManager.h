#pragma once
#include <stdint.h>

#include <format>
#include <string>
#include <vector>
#if !defined(__linux__) && !defined(__APPLE__)
#include <qnet.h>
#endif
#include "platform/PlatformTypes.h"
#include "platform/IPlatformNetwork.h"
#include "app/include/NetTypes.h"
#include "NetworkPlayerInterface.h"
#include "PlatformNetworkManagerStub.h"
#include "SessionInfo.h"
#include "platform/C4JThread.h"

class ClientConnection;
class Minecraft;
class FriendSessionInfo;
class INVITE_INFO;
class INetworkPlayer;

const int NON_QNET_SENDDATA_ACK_REQUIRED = 1;







class CGameNetworkManager {
    friend class IPlatformNetworkStub;

public:
    CGameNetworkManager();
    

    typedef enum {
        JOINGAME_SUCCESS,
        JOINGAME_FAIL_GENERAL,
        JOINGAME_FAIL_SERVER_FULL
    } eJoinGameResult;

    void Initialise();
    void Terminate();
    void DoWork();
    bool _RunNetworkGame(void* lpParameter);
    bool StartNetworkGame(Minecraft* minecraft, void* lpParameter);
    int CorrectErrorIDS(int IDS);

    

    static int GetLocalPlayerMask(int playerIndex);
    int GetPlayerCount();
    int GetOnlinePlayerCount();
    bool AddLocalPlayerByUserIndex(int userIndex);
    bool RemoveLocalPlayerByUserIndex(int userIndex);
    INetworkPlayer* GetLocalPlayerByUserIndex(int userIndex);
    INetworkPlayer* GetPlayerByIndex(int playerIndex);
    INetworkPlayer* GetPlayerByXuid(PlayerUID xuid);
    INetworkPlayer* GetPlayerBySmallId(unsigned char smallId);
    std::wstring GetDisplayNameByGamertag(std::wstring gamertag);
    INetworkPlayer* GetHostPlayer();
    void RegisterPlayerChangedCallback(
        int iPad,
        std::function<void(INetworkPlayer* pPlayer, bool leaving)> callback);
    void UnRegisterPlayerChangedCallback(int iPad);
    void HandleSignInChange();
    bool ShouldMessageForFullSession();

    

    bool IsInSession();
    bool IsInGameplay();
    bool IsLeavingGame();
    bool IsReadyToPlayOrIdle();

    

    bool SetLocalGame(bool isLocal);
    bool IsLocalGame();
    void SetPrivateGame(bool isPrivate);
    bool IsPrivateGame();
    void HostGame(int localUsersMask, bool bOnlineGame, bool bIsPrivate,
                  unsigned char publicSlots = MINECRAFT_NET_MAX_PLAYERS,
                  unsigned char privateSlots = 0);
    bool IsHost();
    bool IsInStatsEnabledSession();

    

    bool SessionHasSpace(unsigned int spaceRequired = 1);
    std::vector<FriendSessionInfo*>* GetSessionList(int iPad, int localPlayers,
                                                    bool partyOnly);
    bool GetGameSessionInfo(int iPad, SessionID sessionId,
                            FriendSessionInfo* foundSession);
    void SetSessionsUpdatedCallback(std::function<void()> callback);
    void GetFullFriendSessionInfo(
        FriendSessionInfo* foundSession,
        std::function<void(bool success)> callback);
    void ForceFriendsSessionRefresh();

    

    bool JoinGameFromInviteInfo(int userIndex, int userMask,
                                const INVITE_INFO* pInviteInfo);
    eJoinGameResult JoinGame(FriendSessionInfo* searchResult,
                             int localUsersMask);
    static void CancelJoinGame(
        void* lpParam);  
    bool LeaveGame(bool bMigrateHost);
    static int JoinFromInvite_SignInReturned(void* pParam, bool bContinue,
                                             int iPad);
    void UpdateAndSetGameSessionData(
        INetworkPlayer* pNetworkPlayerLeaving = nullptr);
    void SendInviteGUI(int iPad);
    void ResetLeavingGame();

    

    bool IsNetworkThreadRunning();
    static int RunNetworkGameThreadProc(void* lpParameter);
    static int ServerThreadProc(void* lpParameter);
    static int ExitAndJoinFromInviteThreadProc(void* lpParam);

    static void _LeaveGame();
    static int ChangeSessionTypeThreadProc(void* lpParam);

    

    void SystemFlagSet(INetworkPlayer* pNetworkPlayer, int index);
    bool SystemFlagGet(INetworkPlayer* pNetworkPlayer, int index);
    void SystemFlagClear(INetworkPlayer* pNetworkPlayer, int index);

    

    void ServerReadyCreate(
        bool create);           
    void ServerReady();         
    void ServerReadyWait();     
    void ServerReadyDestroy();  
    bool ServerReadyValid();    

    void ServerStoppedCreate(bool create);  
    void ServerStopped();                   
    void ServerStoppedWait();               
    void ServerStoppedDestroy();            
    bool ServerStoppedValid();              

    

    std::wstring GatherStats();
    void renderQueueMeter();
    std::wstring GatherRTTStats();

    

    
    static const int messageQueue_length = 512;
    static int64_t messageQueue[messageQueue_length];
    static const int byteQueue_length = 512;
    static int64_t byteQueue[byteQueue_length];
    static int messageQueuePos;

    
private:
    void StateChange_AnyToHosting();
    void StateChange_AnyToJoining();
    void StateChange_JoiningToIdle(
        IPlatformNetwork::eJoinFailedReason reason);
    void StateChange_AnyToStarting();
    void StateChange_AnyToEnding(bool bStateWasPlaying);
    void StateChange_AnyToIdle();
    void CreateSocket(INetworkPlayer* pNetworkPlayer, bool localPlayer);
    void CloseConnection(INetworkPlayer* pNetworkPlayer);
    void PlayerJoining(INetworkPlayer* pNetworkPlayer);
    void PlayerLeaving(INetworkPlayer* pNetworkPlayer);
    void HostChanged();
    void WriteStats(INetworkPlayer* pNetworkPlayer);
    void GameInviteReceived(int userIndex, const INVITE_INFO* pInviteInfo);
    void HandleInviteWhenInMenus(int userIndex, const INVITE_INFO* pInviteInfo);
    void AddLocalPlayerFailed(int idx, bool serverFull = false);
    void HandleDisconnect(bool bLostRoomOnly);

    int GetPrimaryPad();
    int GetLockedProfile();
    bool IsSignedInLive(int playerIdx);
    bool AllowedToPlayMultiplayer(int playerIdx);
    char* GetOnlineName(int playerIdx);

    C4JThread::Event* m_hServerStoppedEvent;
    C4JThread::Event* m_hServerReadyEvent;
    bool m_bInitialised;

private:
    float m_lastPlayerEventTimeStart;  
    static IPlatformNetwork* s_pPlatformNetworkManager;
    bool m_bNetworkThreadRunning;
    int GetJoiningReadyPercentage();
    bool m_bLastDisconnectWasLostRoomOnly;
    bool m_bFullSessionMessageOnNextSessionChange;

public:
    void FakeLocalPlayerJoined();  
                                   
    void DirectConnectPlayerJoining(INetworkPlayer* pNetworkPlayer) {
        PlayerJoining(pNetworkPlayer);
    }
};

extern CGameNetworkManager g_NetworkManager;
