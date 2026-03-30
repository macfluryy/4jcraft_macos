#include "../../Platform/stdafx.h"
#include "TexturePack.h"

std::wstring TexturePack::getPath(bool bTitleUpdateTexture /*= false*/,
                                  const char* pchBDPatchFileName /*= NULL*/) {
    std::wstring wDrive;

    if (bTitleUpdateTexture) {
        // Make the content package point to to the UPDATE: drive is needed
        wDrive = L"Common\\res\\TitleUpdate\\";
    } else {
        wDrive = L"Common/";
    }

    return wDrive;
}
