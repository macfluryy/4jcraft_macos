#include "SelectWorldScreen.h"

#include <stdint.h>
#include <time.h>
#include <wchar.h>

#include <vector>

#include "Button.h"
#include "ConfirmScreen.h"
#include "CreateWorldScreen.h"
#include "MessageScreen.h"
#include "app/common/App_enums.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/include/NetTypes.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "platform/sdl2/Storage.h"
#include "RenameWorldScreen.h"
#include "util/StringHelpers.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/client/gui/ScrolledSelectionList.h"
#include "minecraft/locale/Language.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/storage/LevelStorageSource.h"
#include "minecraft/world/level/storage/LevelSummary.h"

SelectWorldScreen::SelectWorldScreen(Screen* lastScreen) {
    // 4J - added initialisers
    title = L"Select world";
    done = false;
    selectedWorld = 0;
    worldSelectionList = nullptr;
    isDeleting = false;
    deleteButton = nullptr;
    selectButton = nullptr;
    renameButton = nullptr;

    this->lastScreen = lastScreen;
}

void SelectWorldScreen::init() {
    app.DebugPrintf("SelectWorldScreen::init() START\n");
    Language* language = Language::getInstance();
    title = language->getElement(L"selectWorld.title");

    worldLang = language->getElement(L"selectWorld.world");
    conversionLang = language->getElement(L"selectWorld.conversion");
    loadLevelList();

    worldSelectionList = new WorldSelectionList(this);
    worldSelectionList->init(&buttons, BUTTON_UP_ID, BUTTON_DOWN_ID);

    postInit();
}

void SelectWorldScreen::loadLevelList() {
    LevelStorageSource* levelSource = minecraft->getLevelSource();
    levelList = levelSource->getLevelList();
    //	Collections.sort(levelList);	// 4J - TODO - get sort functor etc.
    selectedWorld = -1;
}

std::wstring SelectWorldScreen::getWorldId(int id) {
    return levelList->at(id)->getLevelId();
}

std::wstring SelectWorldScreen::getWorldName(int id) {
    std::wstring levelName = levelList->at(id)->getLevelName();

    if (levelName.length() == 0) {
        Language* language = Language::getInstance();
        levelName = language->getElement(L"selectWorld.world") + L" " +
                    toWString<int>(id + 1);
    }

    return levelName;
}

void SelectWorldScreen::postInit() {
    Language* language = Language::getInstance();

    buttons.push_back(selectButton = new Button(
                          BUTTON_SELECT_ID, width / 2 - 154, height - 52, 150,
                          20, language->getElement(L"selectWorld.select")));
    buttons.push_back(deleteButton = new Button(
                          BUTTON_RENAME_ID, width / 2 - 154, height - 28, 70,
                          20, language->getElement(L"selectWorld.rename")));
    buttons.push_back(renameButton = new Button(
                          BUTTON_DELETE_ID, width / 2 - 74, height - 28, 70, 20,
                          language->getElement(L"selectWorld.delete")));
    buttons.push_back(new Button(BUTTON_CREATE_ID, width / 2 + 4, height - 52,
                                 150, 20,
                                 language->getElement(L"selectWorld.create")));
    buttons.push_back(new Button(BUTTON_CANCEL_ID, width / 2 + 4, height - 28,
                                 150, 20, language->getElement(L"gui.cancel")));

    selectButton->active = false;
    deleteButton->active = false;
    renameButton->active = false;
}

void SelectWorldScreen::buttonClicked(Button* button) {
    app.DebugPrintf("SelectWorldScreen::buttonClicked START\n");
    if (!button->active) return;
    if (button->id == BUTTON_DELETE_ID) {
        std::wstring worldName = getWorldName(selectedWorld);
        if (worldName != L"") {
            isDeleting = true;

            Language* language = Language::getInstance();
            std::wstring title =
                language->getElement(L"selectWorld.deleteQuestion");
            std::wstring warning =
                L"'" + worldName + L"' " +
                language->getElement(L"selectWorld.deleteWarning");
            std::wstring yes =
                language->getElement(L"selectWorld.deleteButton");
            std::wstring no = language->getElement(L"gui.cancel");

            ConfirmScreen* confirmScreen =
                new ConfirmScreen(this, title, warning, yes, no, selectedWorld);
            minecraft->setScreen(confirmScreen);
        }
    } else if (button->id == BUTTON_SELECT_ID) {
        worldSelected(selectedWorld);
    } else if (button->id == BUTTON_CREATE_ID) {
        minecraft->setScreen(new CreateWorldScreen(this));
    } else if (button->id == BUTTON_RENAME_ID) {
        minecraft->setScreen(
            new RenameWorldScreen(this, getWorldId(selectedWorld)));
    } else if (button->id == BUTTON_CANCEL_ID) {
        app.DebugPrintf(
            "SelectWorldScreen::buttonClicked 'Cancel' "
            "minecraft->setScreen(lastScreen)\n");
        minecraft->setScreen(lastScreen);
    } else {
        worldSelectionList->buttonClicked(button);
    }
}

void SelectWorldScreen::mouseClicked(int x, int y, int buttonNum) {
    if (worldSelectionList != nullptr &&
        worldSelectionList->mouseClicked(x, y, buttonNum)) {
        return;
    }
    Screen::mouseClicked(x, y, buttonNum);
}

void SelectWorldScreen::worldSelected(int id) {
    if (done) return;
    if (id < 0 || id >= (int)levelList->size()) {
        minecraft->setScreen(lastScreen);
        return;
    }

    done = true;
    minecraft->gameMode = nullptr;

    LevelSummary* summary = levelList->at(id);

    std::wstring worldFolderName = getWorldId(id);
    if (worldFolderName.empty()) {
        worldFolderName = L"World" + toWString<int>(id);
    }

    std::wstring worldName = summary->getLevelName();
    if (worldName.empty()) worldName = worldFolderName;

    minecraft->setScreen(new Screen());  // blank screen while world loads

    // Point the StorageManager at the save slot named after this world so any
    // autosave writes land in the matching .mcs file under ~/Library/
    // Application Support/4jcraft/Saves/.
    StorageManager.ResetSaveData();
    StorageManager.SetSaveTitle((wchar_t*)worldName.c_str());

    NetworkGameInitData* param = new NetworkGameInitData();
    param->seed = 0;
    param->findSeed = true;        // regenerate if no seed info available
    param->saveData = nullptr;     // no raw save blob to hand off
    param->texturePackId = 0;
    param->settings = 0;

    GameType* gameMode = summary->getGameMode();
    int gameModeId = (gameMode != nullptr) ? gameMode->getId()
                                           : GameType::SURVIVAL->getId();

    const bool bFlatWorld = false;
    const bool bStructures = true;
    const bool bBonusChest = false;
    const bool bPVP = true;
    const bool bTrust = true;
    const bool bFireSpreads = true;
    const bool bTNT = true;
    const bool bHostPrivileges = false;
    const bool bCheatsEnabled = summary->hasCheats();

    app.SetGameHostOption(eGameHostOption_Difficulty,
                          minecraft->options->difficulty);
    app.SetGameHostOption(eGameHostOption_FriendsOfFriends, 0);
    app.SetGameHostOption(eGameHostOption_Gamertags, 1);
    app.SetGameHostOption(eGameHostOption_BedrockFog, 0);
    app.SetGameHostOption(eGameHostOption_GameType, gameModeId);
    app.SetGameHostOption(eGameHostOption_LevelType, bFlatWorld);
    app.SetGameHostOption(eGameHostOption_Structures, bStructures);
    app.SetGameHostOption(eGameHostOption_BonusChest, bBonusChest);
    app.SetGameHostOption(eGameHostOption_PvP, bPVP);
    app.SetGameHostOption(eGameHostOption_TrustPlayers, bTrust);
    app.SetGameHostOption(eGameHostOption_FireSpreads, bFireSpreads);
    app.SetGameHostOption(eGameHostOption_TNT, bTNT);
    app.SetGameHostOption(eGameHostOption_HostCanFly, bHostPrivileges);
    app.SetGameHostOption(eGameHostOption_HostCanChangeHunger, bHostPrivileges);
    app.SetGameHostOption(eGameHostOption_HostCanBeInvisible, bHostPrivileges);
    app.SetGameHostOption(eGameHostOption_CheatsEnabled, bCheatsEnabled);

    param->settings = app.GetGameHostOption(eGameHostOption_All);
    param->xzSize = LEVEL_MAX_WIDTH;
    param->hellScale = HELL_LEVEL_MAX_SCALE;

    g_NetworkManager.HostGame(0, false, false, MINECRAFT_NET_MAX_PLAYERS, 0);
    g_NetworkManager.FakeLocalPlayerJoined();

    LoadingInputParams* loadingParams = new LoadingInputParams();
    loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
    loadingParams->lpParam = param;

    app.SetAutosaveTimerTime();

    UIFullscreenProgressCompletionData* completionData =
        new UIFullscreenProgressCompletionData();
    completionData->bShowBackground = true;
    completionData->bShowLogo = true;
    completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
    completionData->iPad = 0;
    loadingParams->completionData = completionData;

    ui.NavigateToScene(0, eUIScene_FullscreenProgress, loadingParams);

    Language* language = Language::getInstance();
    minecraft->setScreen(
        new MessageScreen(language->getElement(L"menu.loadingLevel")));
}

void SelectWorldScreen::confirmResult(bool result, int id) {
    if (isDeleting) {
        isDeleting = false;
        if (result) {
            LevelStorageSource* levelSource = minecraft->getLevelSource();
            levelSource->clearAll();
            levelSource->deleteLevel(getWorldId(id));

            loadLevelList();
        }
        minecraft->setScreen(this);
    }
}

void SelectWorldScreen::render(int xm, int ym, float a) {
    // fill(0, 0, width, height, 0x40000000);
    renderDirtBackground(0);
    worldSelectionList->render(xm, ym, a);

    drawCenteredString(font, title, width / 2, 20, 0xffffff);

    Screen::render(xm, ym, a);

    // 4J - debug code - remove
    if (0) {
        static int count = 0;
        static bool forceCreateLevel = false;
        if (count++ >= 100) {
            if (!forceCreateLevel && levelList->size() > 0) {
                // 4J Stu - For some obscures reason the "delete" button is
                // called "renameButton" and vice versa. if( levelList->size() >
                // 2 && deleteButton->active )
                //{
                //	this->selectedWorld = 2;
                //	count = 0;
                //	buttonClicked(deleteButton);
                //}
                // else
                if (levelList->size() > 1 && renameButton->active) {
                    this->selectedWorld = 1;
                    count = 0;
                    buttonClicked(renameButton);
                } else if (selectButton->active == true) {
                    this->selectedWorld = 0;
                    buttonClicked(selectButton);
                    // this->worldSelected( 0 );
                } else {
                    selectButton->active = true;
                    deleteButton->active = true;
                    renameButton->active = true;
                    count = 0;
                }
            } else {
                app.DebugPrintf(
                    "SelectWorldScreen::render minecraft->setScreen(new "
                    "CreateWorldScreen(this))\n");
                minecraft->setScreen(new CreateWorldScreen(this));
            }
        }
    }
}

SelectWorldScreen::WorldSelectionList::WorldSelectionList(
    SelectWorldScreen* sws)
    : ScrolledSelectionList(sws->minecraft, sws->width, sws->height, 32,
                            sws->height - 64, 36) {
    parent = sws;
}

int SelectWorldScreen::WorldSelectionList::getNumberOfItems() {
    return (int)this->parent->levelList->size();
}

void SelectWorldScreen::WorldSelectionList::selectItem(int item,
                                                       bool doubleClick) {
    parent->selectedWorld = item;
    bool active = (this->parent->selectedWorld >= 0 &&
                   this->parent->selectedWorld < getNumberOfItems());
    parent->selectButton->active = active;
    parent->deleteButton->active = active;
    parent->renameButton->active = active;

    if (doubleClick && active) {
        parent->worldSelected(item);
    }
}

bool SelectWorldScreen::WorldSelectionList::isSelectedItem(int item) {
    return item == parent->selectedWorld;
}

int SelectWorldScreen::WorldSelectionList::getMaxPosition() {
    return (int)parent->levelList->size() * 36;
}

void SelectWorldScreen::WorldSelectionList::renderBackground() {
    parent->renderBackground();  // 4J - was
                                 // SelectWorldScreen.this.renderBackground();
}

void SelectWorldScreen::WorldSelectionList::renderItem(int i, int x, int y,
                                                       int h, Tesselator* t) {
    LevelSummary* levelSummary = parent->levelList->at(i);

    std::wstring name = levelSummary->getLevelName();
    if (name.length() == 0) {
        name = parent->worldLang + L" " + toWString<int>(i + 1);
    }

    std::wstring id = levelSummary->getLevelId();

    int64_t lastPlayedMs = levelSummary->getLastPlayed();
    time_t lastPlayedSec = (time_t)(lastPlayedMs / 1000);
    struct tm local{};
#if defined(_WIN32)
    localtime_s(&local, &lastPlayedSec);
#else
    localtime_r(&lastPlayedSec, &local);
#endif

    wchar_t buffer[32];
    swprintf(buffer, 32, L"%d/%d/%d %d:%02d", local.tm_mday, local.tm_mon + 1,
             local.tm_year + 1900, local.tm_hour, local.tm_min);
    id = id + L" (" + buffer;

    int64_t size = levelSummary->getSizeOnDisk();
    wchar_t sizeBuf[32];
    if (size >= 1024 * 1024) {
        swprintf(sizeBuf, 32, L"%.1f MB", (double)size / (1024.0 * 1024.0));
    } else if (size >= 1024) {
        swprintf(sizeBuf, 32, L"%.1f KB", (double)size / 1024.0);
    } else {
        swprintf(sizeBuf, 32, L"%lld B", (long long)size);
    }
    id = id + L", " + sizeBuf + L")";
    std::wstring info;

    if (levelSummary->isRequiresConversion()) {
        info = parent->conversionLang + L" " + info;
    }

    parent->drawString(parent->font, name, x + 2, y + 1, 0xffffff);
    parent->drawString(parent->font, id, x + 2, y + 12, 0x808080);
    parent->drawString(parent->font, info, x + 2, y + 12 + 10, 0x808080);
}
