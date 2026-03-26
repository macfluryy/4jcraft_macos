#include "../../Platform/stdafx.h"
#include "TexturePack.h"

std::wstring TexturePack::getPath(bool bTitleUpdateTexture /*= false*/,
                                  const char* pchBDPatchFileName /*= NULL*/) {
    std::wstring wDrive;
#if 0
    if (bTitleUpdateTexture) {
        // Make the content package point to to the UPDATE: drive is needed
#ifdef _TU_BUILD
        wDrive = L"UPDATE:\\";
#else

        wDrive = L"GAME:\\res\\TitleUpdate\\";
#endif
    } else {
        wDrive = L"GAME:\\";
    }
#else

#if 0

    // 4J-PB - we need to check for a BD patch - this is going to be an issue
    // for full DLC texture packs (Halloween)
    char* pchUsrDir = NULL;
    if (app.GetBootedFromDiscPatch() && pchBDPatchFileName != NULL) {
        pchUsrDir = app.GetBDUsrDirPath(pchBDPatchFileName);
        std::wstring wstr(pchUsrDir, pchUsrDir + strlen(pchUsrDir));

        if (bTitleUpdateTexture) {
            wDrive = wstr + L"\\Common\\res\\TitleUpdate\\";

        } else {
            wDrive = wstr + L"/Common/";
        }
    } else {
        pchUsrDir = getUsrDirPath();

        std::wstring wstr(pchUsrDir, pchUsrDir + strlen(pchUsrDir));

        if (bTitleUpdateTexture) {
            // Make the content package point to to the UPDATE: drive is needed
            wDrive = wstr + L"\\Common\\res\\TitleUpdate\\";
        } else {
            wDrive = wstr + L"/Common/";
        }
    }

#elif 0
    char* pchUsrDir = "";  // getUsrDirPath();
    std::wstring wstr(pchUsrDir, pchUsrDir + strlen(pchUsrDir));

    if (bTitleUpdateTexture) {
        // Make the content package point to to the UPDATE: drive is needed
        wDrive = wstr + L"Common\\res\\TitleUpdate\\";
    } else {
        wDrive = wstr + L"Common\\";
    }
#else
    if (bTitleUpdateTexture) {
        // Make the content package point to to the UPDATE: drive is needed
        wDrive = L"Common\\res\\TitleUpdate\\";
    } else {
        wDrive = L"Common/";
    }
#endif
#endif

    return wDrive;
}
