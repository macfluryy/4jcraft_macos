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

class XSOCIAL_IMAGEPOSTPARAMS {};

class XSOCIAL_LINKPOSTPARAMS {};

typedef struct _XSESSION_VIEW_PROPERTIES {
    uint32_t dwViewId;
    uint32_t dwNumProperties;
    XUSER_PROPERTY* pProperties;
} XSESSION_VIEW_PROPERTIES;

#define XUSER_STATS_ATTRS_IN_SPEC 1

typedef struct _XUSER_STATS_SPEC {
    uint32_t dwViewId;
    uint32_t dwNumColumnIds;
    uint16_t rgwColumnIds[XUSER_STATS_ATTRS_IN_SPEC];
} XUSER_STATS_SPEC, *PXUSER_STATS_SPEC;

typedef struct _XUSER_STATS_COLUMN {
    uint16_t wColumnId;
    XUSER_DATA Value;
} XUSER_STATS_COLUMN, *PXUSER_STATS_COLUMN;

typedef struct _XUSER_STATS_ROW {
    PlayerUID xuid;
    uint32_t dwRank;
    int64_t i64Rating;
    char szGamertag[XUSER_NAME_SIZE];
    uint32_t dwNumColumns;
    PXUSER_STATS_COLUMN pColumns;
} XUSER_STATS_ROW, *PXUSER_STATS_ROW;

typedef struct _XUSER_STATS_VIEW {
    uint32_t dwViewId;
    uint32_t dwTotalViewRows;
    uint32_t dwNumRows;
    PXUSER_STATS_ROW pRows;
} XUSER_STATS_VIEW, *PXUSER_STATS_VIEW;

typedef struct _XUSER_STATS_READ_RESULTS {
    uint32_t dwNumViews;
    PXUSER_STATS_VIEW pViews;
} XUSER_STATS_READ_RESULTS, *PXUSER_STATS_READ_RESULTS;
