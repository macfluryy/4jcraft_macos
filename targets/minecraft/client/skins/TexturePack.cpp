#include "TexturePack.h"

std::wstring TexturePack::getPath(
    bool bTitleUpdateTexture ,
    const char* pchBDPatchFileName ) {
    std::wstring wDrive;

    if (bTitleUpdateTexture) {
        
        wDrive = L"Common\\res\\TitleUpdate\\";
    } else {
        wDrive = L"app/common/";
    }

    return wDrive;
}
