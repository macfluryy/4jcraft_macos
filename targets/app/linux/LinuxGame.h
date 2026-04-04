#pragma once

#include <cstdint>

#include "app/common/App_enums.h"
#include "app/common/Game.h"

class C4JStringTable;

class LinuxGame : public Game {
public:
    LinuxGame();

    void SetRichPresenceContext(int iPad, int contextId) override;

    void StoreLaunchData() override;
    void ExitGame() override;
    void FatalLoadError() override;

    void CaptureSaveThumbnail() override;
    void GetSaveThumbnail(std::uint8_t** thumbnailData,
                          unsigned int* thumbnailSize) override;
    void ReleaseSaveThumbnail() override;
    void GetScreenshot(int iPad, std::uint8_t** screenshotData,
                       unsigned int* screenshotSize) override;

    int LoadLocalTMSFile(wchar_t* wchTMSFile) override;
    int LoadLocalTMSFile(wchar_t* wchTMSFile,
                         eFileExtensionType eExt) override;

    void FreeLocalTMSFiles(eTMSFileType eType) override;
    int GetLocalTMSFileIndex(
        wchar_t* wchTMSFile, bool bFilenameIncludesExtension,
        eFileExtensionType eEXT = eFileExtensionType_PNG) override;

    void ReadBannedList(int iPad, eTMSAction action = (eTMSAction)0,
                        bool bCallback = false) override {}

    C4JStringTable* GetStringTable() { return nullptr; }

    // original code
    virtual void TemporaryCreateGameStart();
};

extern LinuxGame app;

