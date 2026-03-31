#pragma once

typedef enum _XBACKGROUND_DOWNLOAD_MODE {
    XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW = 1,
    XBACKGROUND_DOWNLOAD_MODE_AUTO = 2
} XBACKGROUND_DOWNLOAD_MODE,
    *PXBACKGROUND_DOWNLOAD_MODE;

uint32_t XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE Mode);

#define XONLINE_S_LOGON_CONNECTION_ESTABLISHED 0
#define XPARTY_E_NOT_IN_PARTY 0
#define XPARTY_MAX_USERS 4
typedef struct _XPARTY_USER_INFO {
    PlayerUID Xuid;
    char GamerTag[XUSER_NAME_SIZE];
    uint32_t dwUserIndex;
    //   XONLINE_NAT_TYPE NatType;
    uint32_t dwTitleId;
    uint32_t dwFlags;
    //    XSESSION_INFO SessionInfo;
    //    XPARTY_CUSTOM_DATA CustomData;
} XPARTY_USER_INFO;

typedef struct _XPARTY_USER_LIST {
    uint32_t dwUserCount;
    XPARTY_USER_INFO Users[XPARTY_MAX_USERS];
} XPARTY_USER_LIST;

uint32_t XShowPartyUI(uint32_t dwUserIndex);
uint32_t XShowFriendsUI(uint32_t dwUserIndex);
int32_t XPartyGetUserList(XPARTY_USER_LIST* pUserList);

uint32_t XContentGetThumbnail(uint32_t dwUserIndex,
                              const XCONTENT_DATA* pContentData,
                              uint8_t* pbThumbnail, uint32_t* pcbThumbnail,
                              PXOVERLAPPED* pOverlapped);

void XShowAchievementsUI(int i);

uint32_t XUserAreUsersFriends(uint32_t dwUserIndex, PPlayerUID pXuids,
                              uint32_t dwXuidCount, bool* pfResult,
                              void* pOverlapped);

