#pragma once

#include <cstdint>
#include <limits>
#include <mutex>
#include <vector>

#include "platform/PlatformTypes.h"

inline constexpr int MINECRAFT_NET_MAX_PLAYERS = 8;

static_assert(
    MINECRAFT_NET_MAX_PLAYERS <= std::numeric_limits<std::uint8_t>::max(),
    "MINECRAFT_NET_MAX_PLAYERS must fit in the 8-bit network protocol");

using SessionID = uint64_t;
using GameSessionUID = PlayerUID;
class INVITE_INFO;

inline constexpr int QNET_SENDDATA_LOW_PRIORITY = 0;
inline constexpr int QNET_SENDDATA_SECONDARY = 0;
inline constexpr int QNET_SENDDATA_RELIABLE = 0;
inline constexpr int QNET_SENDDATA_SEQUENTIAL = 0;
inline constexpr int QNET_GETSENDQUEUESIZE_SECONDARY_TYPE = 0;
inline constexpr int QNET_GETSENDQUEUESIZE_MESSAGES = 0;
inline constexpr int QNET_GETSENDQUEUESIZE_BYTES = 0;

#define QNET_E_SESSION_FULL 0
#define QNET_USER_MASK_USER0 1
#define QNET_USER_MASK_USER1 2
#define QNET_USER_MASK_USER2 4
#define QNET_USER_MASK_USER3 8

struct XRNM_SEND_BUFFER {
    uint32_t dwDataSize;
    uint8_t* pbyData;
};

template <typename T>
class XLockFreeStack {
    std::vector<T*> intStack;
    std::mutex m_cs;

public:
    XLockFreeStack() = default;
    ~XLockFreeStack() = default;
    void Initialize() {}
    void Push(T* data) {
        std::lock_guard<std::mutex> lock(m_cs);
        intStack.push_back(data);
    }
    T* Pop() {
        std::lock_guard<std::mutex> lock(m_cs);
        if (intStack.size()) {
            T* ret = intStack.back();
            intStack.pop_back();
            return ret;
        }
        return nullptr;
    }
};

class IQNetPlayer {
public:
    uint8_t GetSmallId();
    void SendData(IQNetPlayer* player, const void* pvData, uint32_t dwDataSize,
                  uint32_t dwFlags);
    bool IsSameSystem(IQNetPlayer* player);
    uint32_t GetSendQueueSize(IQNetPlayer* player, uint32_t dwFlags);
    uint32_t GetCurrentRtt();
    bool IsHost();
    bool IsGuest();
    bool IsLocal();
    PlayerUID GetXuid();
    const wchar_t* GetGamertag();
    int GetSessionIndex();
    bool IsTalking();
    bool IsMutedByLocalUser(uint32_t dwUserIndex);
    bool HasVoice();
    bool HasCamera();
    int GetUserIndex();
    void SetCustomDataValue(uintptr_t ulpCustomDataValue);
    uintptr_t GetCustomDataValue();

private:
    uintptr_t m_customData;
};

enum QNET_STATE {
    QNET_STATE_IDLE,
    QNET_STATE_SESSION_HOSTING,
    QNET_STATE_SESSION_JOINING,
    QNET_STATE_GAME_LOBBY,
    QNET_STATE_SESSION_REGISTERING,
    QNET_STATE_SESSION_STARTING,
    QNET_STATE_GAME_PLAY,
    QNET_STATE_SESSION_ENDING,
    QNET_STATE_SESSION_LEAVING,
    QNET_STATE_SESSION_DELETING
};

class IQNet {
public:
    int32_t AddLocalPlayerByUserIndex(uint32_t dwUserIndex);
    IQNetPlayer* GetHostPlayer();
    IQNetPlayer* GetLocalPlayerByUserIndex(uint32_t dwUserIndex);
    IQNetPlayer* GetPlayerByIndex(uint32_t dwPlayerIndex);
    IQNetPlayer* GetPlayerBySmallId(uint8_t SmallId);
    IQNetPlayer* GetPlayerByXuid(PlayerUID xuid);
    uint32_t GetPlayerCount();
    QNET_STATE GetState();
    bool IsHost();
    int32_t JoinGameFromInviteInfo(uint32_t dwUserIndex, uint32_t dwUserMask,
                                   const INVITE_INFO* pInviteInfo);
    void HostGame();
    void EndGame();

    static IQNetPlayer m_player[4];
};

class IQNetCallbacks {};
class IQNetGameSearch {};

struct XNQOSINFO {
    uint8_t bFlags;
    uint8_t bReserved;
    uint16_t cProbesXmit;
    uint16_t cProbesRecv;
    uint16_t cbData;
    uint8_t* pbData;
    uint16_t wRttMinInMsecs;
    uint16_t wRttMedInMsecs;
    uint32_t dwUpBitsPerSec;
    uint32_t dwDnBitsPerSec;
};

struct XNQOS {
    uint32_t cxnqos;
    uint32_t cxnqosPending;
    XNQOSINFO axnqosinfo[1];
};

struct XOVERLAPPED {};

struct XSESSION_SEARCHRESULT {};

struct XUSER_CONTEXT {
    uint32_t dwContextId;
    uint32_t dwValue;
};

struct XSESSION_SEARCHRESULT_HEADER {
    uint32_t dwSearchResults;
    XSESSION_SEARCHRESULT* pResults;
};
