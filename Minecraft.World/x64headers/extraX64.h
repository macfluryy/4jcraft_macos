#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <functional>
#include <cstdint>
#include <limits>
#include <mutex>
#include "../../../4J.Common/4J_Compat.h"

#include "../../../Minecraft.Client/Header Files/SkinBox.h"

#include <vector>

#define MULTITHREAD_ENABLE

constexpr int MINECRAFT_NET_MAX_PLAYERS = 8;

static_assert(
    MINECRAFT_NET_MAX_PLAYERS <= std::numeric_limits<std::uint8_t>::max(),
    "MINECRAFT_NET_MAX_PLAYERS must fit in the 8-bit network protocol");

typedef uint64_t SessionID;
typedef PlayerUID GameSessionUID;
class INVITE_INFO;

typedef struct _XUIOBJ* HXUIOBJ;
typedef struct _XUICLASS* HXUICLASS;
typedef struct _XUIBRUSH* HXUIBRUSH;
typedef struct _XUIDC* HXUIDC;

bool IsEqualXUID(PlayerUID a, PlayerUID b);

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

void XMemCpy(void* a, const void* b, size_t s);
void XMemSet(void* a, int t, size_t s);
void XMemSet128(void* a, int t, size_t s);
void* XPhysicalAlloc(size_t a, uintptr_t b, uintptr_t c, uint32_t d);
void XPhysicalFree(void* a);

class DLCManager;
class LevelRuleset;
class ModelPart;
class LevelChunk;
class IXACT3Engine;
class XACT_NOTIFICATION;
class ConsoleSchematicFile;

const int XZP_ICON_SHANK_01 = 1;
const int XZP_ICON_SHANK_02 = 2;
const int XZP_ICON_SHANK_03 = 3;

const int XN_SYS_SIGNINCHANGED = 0;
const int XN_SYS_INPUTDEVICESCHANGED = 1;
const int XN_LIVE_CONTENT_INSTALLED = 2;
const int XN_SYS_STORAGEDEVICESCHANGED = 3;

#define VK_PAD_A 0x5800
#define VK_PAD_B 0x5801
#define VK_PAD_X 0x5802
#define VK_PAD_Y 0x5803
#define VK_PAD_RSHOULDER 0x5804
#define VK_PAD_LSHOULDER 0x5805
#define VK_PAD_LTRIGGER 0x5806
#define VK_PAD_RTRIGGER 0x5807

#define VK_PAD_DPAD_UP 0x5810
#define VK_PAD_DPAD_DOWN 0x5811
#define VK_PAD_DPAD_LEFT 0x5812
#define VK_PAD_DPAD_RIGHT 0x5813
#define VK_PAD_START 0x5814
#define VK_PAD_BACK 0x5815
#define VK_PAD_LTHUMB_PRESS 0x5816
#define VK_PAD_RTHUMB_PRESS 0x5817

#define VK_PAD_LTHUMB_UP 0x5820
#define VK_PAD_LTHUMB_DOWN 0x5821
#define VK_PAD_LTHUMB_RIGHT 0x5822
#define VK_PAD_LTHUMB_LEFT 0x5823
#define VK_PAD_LTHUMB_UPLEFT 0x5824
#define VK_PAD_LTHUMB_UPRIGHT 0x5825
#define VK_PAD_LTHUMB_DOWNRIGHT 0x5826
#define VK_PAD_LTHUMB_DOWNLEFT 0x5827

#define VK_PAD_RTHUMB_UP 0x5830
#define VK_PAD_RTHUMB_DOWN 0x5831
#define VK_PAD_RTHUMB_RIGHT 0x5832
#define VK_PAD_RTHUMB_LEFT 0x5833
#define VK_PAD_RTHUMB_UPLEFT 0x5834
#define VK_PAD_RTHUMB_UPRIGHT 0x5835
#define VK_PAD_RTHUMB_DOWNRIGHT 0x5836
#define VK_PAD_RTHUMB_DOWNLEFT 0x5837

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

const int QNET_GETSENDQUEUESIZE_SECONDARY_TYPE = 0;
const int QNET_GETSENDQUEUESIZE_MESSAGES = 0;
const int QNET_GETSENDQUEUESIZE_BYTES = 0;

typedef struct {
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
} XNQOSINFO;

typedef struct {
    uint32_t cxnqos;
    uint32_t cxnqosPending;
    XNQOSINFO axnqosinfo[1];
} XNQOS;

typedef struct _XOVERLAPPED {
} XOVERLAPPED, *PXOVERLAPPED;

typedef struct _XSESSION_SEARCHRESULT {
} XSESSION_SEARCHRESULT, *PXSESSION_SEARCHRESULT;

typedef struct {
    uint32_t dwContextId;
    uint32_t dwValue;
} XUSER_CONTEXT, *PXUSER_CONTEXT;

typedef struct _XSESSION_SEARCHRESULT_HEADER {
    uint32_t dwSearchResults;
    XSESSION_SEARCHRESULT* pResults;
} XSESSION_SEARCHRESULT_HEADER, *PXSESSION_SEARCHRESULT_HEADER;

typedef struct _XONLINE_FRIEND {
    PlayerUID xuid;
    char szGamertag[XUSER_NAME_SIZE];
    uint32_t dwFriendState;
    SessionID sessionID;
    uint32_t dwTitleID;
    FILETIME ftUserTime;
    SessionID xnkidInvite;
    FILETIME gameinviteTime;
    uint32_t cchRichPresence;
} XONLINE_FRIEND, *PXONLINE_FRIEND;

class IQNetCallbacks {};
class IQNetGameSearch {};

typedef enum _QNET_STATE {
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
} QNET_STATE,
    *PQNET_STATE;

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

void PIXAddNamedCounter(int a, const char* b, ...);
void PIXBeginNamedEvent(int a, const char* b, ...);
void PIXEndNamedEvent();
void PIXSetMarkerDeprecated(int a, const char* b, ...);

void XSetThreadProcessor(void* a, int b);

const int QNET_SENDDATA_LOW_PRIORITY = 0;
const int QNET_SENDDATA_SECONDARY = 0;
constexpr PlayerUID INVALID_XUID = 0;

const int QNET_SENDDATA_RELIABLE = 0;
const int QNET_SENDDATA_SEQUENTIAL = 0;

struct XRNM_SEND_BUFFER {
    uint32_t dwDataSize;
    uint8_t* pbyData;
};

const int D3DBLEND_CONSTANTALPHA = 0;
const int D3DBLEND_INVCONSTANTALPHA = 0;
const int D3DPT_QUADLIST = 0;

typedef struct _XUSER_SIGNIN_INFO {
    PlayerUID xuid;
    uint32_t dwGuestNumber;
} XUSER_SIGNIN_INFO, *PXUSER_SIGNIN_INFO;

#define XUSER_GET_SIGNIN_INFO_ONLINE_XUID_ONLY 0x00000001
#define XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY 0x00000002

uint32_t XUserGetSigninInfo(uint32_t dwUserIndex, uint32_t dwFlags,
                            PXUSER_SIGNIN_INFO pSigninInfo);

class CXuiStringTable {
public:
    const wchar_t* Lookup(const wchar_t* szId);
    const wchar_t* Lookup(uint32_t nIndex);
    void Clear();
    int32_t Load(const wchar_t* szId);
};

typedef void* XMEMDECOMPRESSION_CONTEXT;
typedef void* XMEMCOMPRESSION_CONTEXT;

typedef enum _XMEMCODEC_TYPE {
    XMEMCODEC_DEFAULT = 0,
    XMEMCODEC_LZX = 1
} XMEMCODEC_TYPE;

int32_t XMemDecompress(XMEMDECOMPRESSION_CONTEXT Context, void* pDestination,
                       size_t* pDestSize, void* pSource, size_t SrcSize);

int32_t XMemCompress(XMEMCOMPRESSION_CONTEXT Context, void* pDestination,
                     size_t* pDestSize, void* pSource, size_t SrcSize);

int32_t XMemCreateCompressionContext(XMEMCODEC_TYPE CodecType,
                                     const void* pCodecParams, uint32_t Flags,
                                     XMEMCOMPRESSION_CONTEXT* pContext);

int32_t XMemCreateDecompressionContext(XMEMCODEC_TYPE CodecType,
                                       const void* pCodecParams, uint32_t Flags,
                                       XMEMDECOMPRESSION_CONTEXT* pContext);

typedef struct _XMEMCODEC_PARAMETERS_LZX {
    uint32_t Flags;
    uint32_t WindowSize;
    uint32_t CompressionPartitionSize;
} XMEMCODEC_PARAMETERS_LZX;

void XMemDestroyCompressionContext(XMEMCOMPRESSION_CONTEXT Context);
void XMemDestroyDecompressionContext(XMEMDECOMPRESSION_CONTEXT Context);

typedef struct {
    uint8_t type;
    union {
        int32_t nData;
        int64_t i64Data;
        double dblData;
        struct {
            uint32_t cbData;
            wchar_t* pwszData;
        } string;
        float fData;
        struct {
            uint32_t cbData;
            uint8_t* pbData;
        } binary;
        FILETIME ftData;
    };
} XUSER_DATA, *PXUSER_DATA;

typedef struct {
    uint32_t dwPropertyId;
    XUSER_DATA value;
} XUSER_PROPERTY, *PXUSER_PROPERTY;

const int XC_LANGUAGE_ENGLISH = 0x01;
const int XC_LANGUAGE_JAPANESE = 0x02;
const int XC_LANGUAGE_GERMAN = 0x03;
const int XC_LANGUAGE_FRENCH = 0x04;
const int XC_LANGUAGE_SPANISH = 0x05;
const int XC_LANGUAGE_ITALIAN = 0x06;
const int XC_LANGUAGE_KOREAN = 0x07;
const int XC_LANGUAGE_TCHINESE = 0x08;
const int XC_LANGUAGE_PORTUGUESE = 0x09;
const int XC_LANGUAGE_POLISH = 0x0B;
const int XC_LANGUAGE_RUSSIAN = 0x0C;
const int XC_LANGUAGE_SWEDISH = 0x0D;
const int XC_LANGUAGE_TURKISH = 0x0E;
const int XC_LANGUAGE_BNORWEGIAN = 0x0F;
const int XC_LANGUAGE_DUTCH = 0x10;
const int XC_LANGUAGE_SCHINESE = 0x11;

const int XC_LANGUAGE_LATINAMERICANSPANISH = 0xF0;
const int XC_LANGUAGE_FINISH = 0xF1;
const int XC_LANGUAGE_GREEK = 0xF2;
const int XC_LANGUAGE_DANISH = 0xF3;
const int XC_LANGUAGE_CZECH = 0xF4;
const int XC_LANGUAGE_SLOVAK = 0xF5;

const int XC_LOCALE_AUSTRALIA = 1;
const int XC_LOCALE_AUSTRIA = 2;
const int XC_LOCALE_BELGIUM = 3;
const int XC_LOCALE_BRAZIL = 4;
const int XC_LOCALE_CANADA = 5;
const int XC_LOCALE_CHILE = 6;
const int XC_LOCALE_CHINA = 7;
const int XC_LOCALE_COLOMBIA = 8;
const int XC_LOCALE_CZECH_REPUBLIC = 9;
const int XC_LOCALE_DENMARK = 10;
const int XC_LOCALE_FINLAND = 11;
const int XC_LOCALE_FRANCE = 12;
const int XC_LOCALE_GERMANY = 13;
const int XC_LOCALE_GREECE = 14;
const int XC_LOCALE_HONG_KONG = 15;
const int XC_LOCALE_HUNGARY = 16;
const int XC_LOCALE_INDIA = 17;
const int XC_LOCALE_IRELAND = 18;
const int XC_LOCALE_ITALY = 19;
const int XC_LOCALE_JAPAN = 20;
const int XC_LOCALE_KOREA = 21;
const int XC_LOCALE_MEXICO = 22;
const int XC_LOCALE_NETHERLANDS = 23;
const int XC_LOCALE_NEW_ZEALAND = 24;
const int XC_LOCALE_NORWAY = 25;
const int XC_LOCALE_POLAND = 26;
const int XC_LOCALE_PORTUGAL = 27;
const int XC_LOCALE_SINGAPORE = 28;
const int XC_LOCALE_SLOVAK_REPUBLIC = 29;
const int XC_LOCALE_SOUTH_AFRICA = 30;
const int XC_LOCALE_SPAIN = 31;
const int XC_LOCALE_SWEDEN = 32;
const int XC_LOCALE_SWITZERLAND = 33;
const int XC_LOCALE_TAIWAN = 34;
const int XC_LOCALE_GREAT_BRITAIN = 35;
const int XC_LOCALE_UNITED_STATES = 36;
const int XC_LOCALE_RUSSIAN_FEDERATION = 37;
const int XC_LOCALE_WORLD_WIDE = 38;
const int XC_LOCALE_TURKEY = 39;
const int XC_LOCALE_ARGENTINA = 40;
const int XC_LOCALE_SAUDI_ARABIA = 41;
const int XC_LOCALE_ISRAEL = 42;
const int XC_LOCALE_UNITED_ARAB_EMIRATES = 43;
const int XC_LOCALE_LATIN_AMERICA = 240;

uint32_t XGetLanguage();
uint32_t XGetLocale();
uint32_t XEnableGuestSignin(bool fEnable);

class D3DXVECTOR3 {
public:
    D3DXVECTOR3();
    D3DXVECTOR3(float, float, float);
    float x, y, z, pad;
    D3DXVECTOR3& operator+=(const D3DXVECTOR3& add);
};

#define QNET_E_SESSION_FULL 0
#define QNET_USER_MASK_USER0 1
#define QNET_USER_MASK_USER1 2
#define QNET_USER_MASK_USER2 4
#define QNET_USER_MASK_USER3 8
