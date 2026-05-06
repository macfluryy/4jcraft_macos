#pragma once

#include <cstdint>
#include "app/common/App_enums.h"
#include "app/common/Game.h"

class C4JStringTable;

class MacGame : public Game {
public:
    MacGame();

    void SetRichPresenceContext(int iPad, int contextId) override;
    void StoreLaunchData() override;
    void ExitGame() override;
    void FatalLoadError() override;
    void CaptureSaveThumbnail() override;

    void GetSaveThumbnail(std::uint8_t** thumbnailData,
                          unsigned int*  thumbnailSize) override;

    void ReleaseSaveThumbnail() override;

    void GetScreenshot(int iPad, std::uint8_t** screenshotData,
                       unsigned int* screenshotSize) override;

    int  LoadLocalTMSFile(wchar_t* wchTMSFile) override;
    int  LoadLocalTMSFile(wchar_t* wchTMSFile,
                          eFileExtensionType eExt) override;

    void FreeLocalTMSFiles(eTMSFileType eType) override;

    int  GetLocalTMSFileIndex(wchar_t* wchTMSFile,
                              bool     bFilenameIncludesExtension,
                              eFileExtensionType eEXT =
                                      eFileExtensionType_PNG) override;

    // ReadBannedList is a no-op on desktop platforms
    void ReadBannedList(int iPad, eTMSAction action = (eTMSAction)0,
                        bool bCallback = false) override {}

    C4JStringTable* GetStringTable() { return nullptr; }

    // Temporary game-start helper (mirrors original Linux version)
    virtual void TemporaryCreateGameStart();
    bool TemporaryDirectConnectStart(const char* host, int port);
};

extern MacGame app;