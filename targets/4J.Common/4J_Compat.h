#pragma once

#include <cstdint>

// The extracted 4J public headers only need the generic Linux/Windows-style
// compatibility surface. Console backends still own their platform-specific
// identity/content definitions elsewhere.
#if !defined(__ORBIS__) && !defined(__PS3__) && !defined(__PSVITA__) && \
    !defined(_DURANGO)
#ifndef XUSER_INDEX_ANY
inline constexpr int XUSER_INDEX_ANY = 255;
#endif

#ifndef XUSER_MAX_COUNT
inline constexpr int XUSER_MAX_COUNT = 4;
#endif

#ifndef XUSER_NAME_SIZE
inline constexpr int XUSER_NAME_SIZE = 32;
#endif

#ifndef XUSER_INDEX_FOCUS
inline constexpr int XUSER_INDEX_FOCUS = 254;
#endif

#ifndef FOURJ_COMMON_PLAYER_UID_DEFINED
#define FOURJ_COMMON_PLAYER_UID_DEFINED
using PlayerUID = unsigned long long;
using PPlayerUID = PlayerUID*;
inline constexpr PlayerUID INVALID_XUID = 0;
#endif
#endif

class CXuiStringTable;

#if !defined(__ORBIS__) && !defined(__PS3__) && !defined(__PSVITA__) && \
    !defined(_DURANGO)
#ifndef XCONTENT_MAX_DISPLAYNAME_LENGTH
inline constexpr int XCONTENT_MAX_DISPLAYNAME_LENGTH = 256;
#endif

#ifndef XCONTENT_MAX_FILENAME_LENGTH
inline constexpr int XCONTENT_MAX_FILENAME_LENGTH = 256;
#endif

#ifndef FOURJ_COMMON_XCONTENT_DATA_DEFINED
#define FOURJ_COMMON_XCONTENT_DATA_DEFINED
using XCONTENTDEVICEID = int;

struct XCONTENT_DATA {
    XCONTENTDEVICEID DeviceID;
    std::uint32_t dwContentType;
    wchar_t szDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
    char szFileName[XCONTENT_MAX_FILENAME_LENGTH];
};
using PXCONTENT_DATA = XCONTENT_DATA*;
#endif

#ifndef XMARKETPLACE_CONTENT_ID_LEN
inline constexpr int XMARKETPLACE_CONTENT_ID_LEN = 4;
#endif

#ifndef FOURJ_COMMON_XMARKETPLACE_DEFINED
#define FOURJ_COMMON_XMARKETPLACE_DEFINED
struct XMARKETPLACE_CONTENTOFFER_INFO {
    std::uint64_t qwOfferID;
    std::uint64_t qwPreviewOfferID;
    std::uint32_t dwOfferNameLength;
    wchar_t* wszOfferName;
    std::uint32_t dwOfferType;
    std::uint8_t contentId[XMARKETPLACE_CONTENT_ID_LEN];
    bool fIsUnrestrictedLicense;
    std::uint32_t dwLicenseMask;
    std::uint32_t dwTitleID;
    std::uint32_t dwContentCategory;
    std::uint32_t dwTitleNameLength;
    wchar_t* wszTitleName;
    bool fUserHasPurchased;
    std::uint32_t dwPackageSize;
    std::uint32_t dwInstallSize;
    std::uint32_t dwSellTextLength;
    wchar_t* wszSellText;
    std::uint32_t dwAssetID;
    std::uint32_t dwPurchaseQuantity;
    std::uint32_t dwPointsPrice;
};
using PXMARKETPLACE_CONTENTOFFER_INFO = XMARKETPLACE_CONTENTOFFER_INFO*;
#endif

#ifndef XMARKETPLACE_OFFERING_TYPE_CONTENT
inline constexpr std::uint32_t XMARKETPLACE_OFFERING_TYPE_CONTENT = 0x00000002;
#endif

#ifndef XMARKETPLACE_OFFERING_TYPE_GAME_DEMO
inline constexpr std::uint32_t XMARKETPLACE_OFFERING_TYPE_GAME_DEMO =
    0x00000020;
#endif

#ifndef XMARKETPLACE_OFFERING_TYPE_GAME_TRAILER
inline constexpr std::uint32_t XMARKETPLACE_OFFERING_TYPE_GAME_TRAILER =
    0x00000040;
#endif

#ifndef XMARKETPLACE_OFFERING_TYPE_THEME
inline constexpr std::uint32_t XMARKETPLACE_OFFERING_TYPE_THEME = 0x00000080;
#endif

#ifndef XMARKETPLACE_OFFERING_TYPE_TILE
inline constexpr std::uint32_t XMARKETPLACE_OFFERING_TYPE_TILE = 0x00000800;
#endif

#ifndef XMARKETPLACE_OFFERING_TYPE_ARCADE
inline constexpr std::uint32_t XMARKETPLACE_OFFERING_TYPE_ARCADE = 0x00002000;
#endif

#ifndef XMARKETPLACE_OFFERING_TYPE_VIDEO
inline constexpr std::uint32_t XMARKETPLACE_OFFERING_TYPE_VIDEO = 0x00004000;
#endif

#ifndef XMARKETPLACE_OFFERING_TYPE_CONSUMABLE
inline constexpr std::uint32_t XMARKETPLACE_OFFERING_TYPE_CONSUMABLE =
    0x00010000;
#endif

#ifndef XMARKETPLACE_OFFERING_TYPE_AVATARITEM
inline constexpr std::uint32_t XMARKETPLACE_OFFERING_TYPE_AVATARITEM =
    0x00100000;
#endif
#endif
