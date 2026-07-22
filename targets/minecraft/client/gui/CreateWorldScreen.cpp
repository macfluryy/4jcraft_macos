#include "CreateWorldScreen.h"

#include <stddef.h>
#include <stdint.h>

#include <filesystem>
#include <string>
#include <vector>

#include "platform/sdl2/Storage.h"
#include "Button.h"
#include "EditBox.h"
#include "MessageScreen.h"
#include "app/common/App_Defines.h"
#include "app/common/App_enums.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "app/include/NetTypes.h"
#include "app/include/stubs.h"
#include "util/StringHelpers.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/User.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/locale/Language.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/chunk/ChunkSource.h"

CreateWorldScreen::CreateWorldScreen(Screen* lastScreen)
    : lastScreen(lastScreen),
      nameEdit(nullptr),
      seedEdit(nullptr),
      done(false),
      moreOptions(false),
      gameMode(L"survival"),
      generateStructures(true),
      bonusChest(false),
      cheatsEnabled(false),
      worldType(e_levelType_Normal),
      gameModeButton(nullptr),
      moreWorldOptionsButton(nullptr),
      generateStructuresButton(nullptr),
      bonusChestButton(nullptr),
      worldTypeButton(nullptr),
      cheatsEnabledButton(nullptr),
      m_iGameModeId(GameType::SURVIVAL->getId()),
      m_bGameModeCreative(false) {}

void CreateWorldScreen::tick() {
    if (nameEdit != nullptr) nameEdit->tick();
    if (seedEdit != nullptr) seedEdit->tick();
}

void CreateWorldScreen::init() {
    Language* language = Language::getInstance();

    Keyboard::enableRepeatEvents(true);
    buttons.clear();
    buttons.push_back(new Button(0, width / 2 - 155, height - 28, 150, 20,
                                 language->getElement(L"selectWorld.create")));
    buttons.push_back(new Button(1, width / 2 + 5, height - 28, 150, 20,
                                 language->getElement(L"gui.cancel")));

    nameEdit = new EditBox(this, font, width / 2 - 100, 60, 200, 20,
                           language->getElement(L"selectWorld.newWorld"));
    nameEdit->setMaxLength(32);

    seedEdit = new EditBox(this, font, width / 2 - 100, 60, 200, 20, L"");
    seedEdit->setMaxLength(60);

    buttons.push_back(gameModeButton = new Button(
                          2, width / 2 - 75, 100, 150, 20,
                          language->getElement(L"selectWorld.gameMode")));
    buttons.push_back(
        moreWorldOptionsButton =
            new Button(3, width / 2 - 75, 172, 150, 20,
                       language->getElement(L"selectWorld.moreWorldOptions")));
    buttons.push_back(generateStructuresButton = new Button(
                          4, width / 2 - 155, 100, 150, 20,
                          language->getElement(L"selectWorld.mapFeatures")));
    generateStructuresButton->visible = false;
    generateStructuresButton->active = false;
    buttons.push_back(bonusChestButton = new Button(
                          7, width / 2 + 5, 136, 150, 20,
                          language->getElement(L"selectWorld.bonusItems")));
    bonusChestButton->visible = false;
    bonusChestButton->active = false;
    buttons.push_back(worldTypeButton = new Button(
                          5, width / 2 + 5, 100, 150, 20,
                          language->getElement(L"selectWorld.mapType")));
    worldTypeButton->visible = false;
    worldTypeButton->active = false;
    buttons.push_back(cheatsEnabledButton = new Button(
                          6, width / 2 - 155, 136, 150, 20,
                          language->getElement(L"selectWorld.allowCommands")));

    setMoreOptionsVisible(false);
    updateStrings();
    updateResultFolder();
    updateCreateButtonState();
}


void CreateWorldScreen::updateStrings() {
    Language* language = Language::getInstance();

    gameModeButton->msg =
        language->getElement(L"selectWorld.gameMode") + L" " +
        language->getElement(L"selectWorld.gameMode." + gameMode);

    std::wstring line1Key = L"selectWorld.gameMode." + gameMode + L".line1";
    std::wstring line2Key = L"selectWorld.gameMode." + gameMode + L".line2";
    gameModeDescriptionLine1 = language->getElement(line1Key);
    gameModeDescriptionLine2 = language->getElement(line2Key);

    generateStructuresButton->msg =
        language->getElement(L"selectWorld.mapFeatures") + L" " +
        (generateStructures ? language->getElement(L"options.on")
                            : language->getElement(L"options.off"));

    bonusChestButton->msg = language->getElement(L"selectWorld.bonusItems") +
                            L" " +
                            (bonusChest ? language->getElement(L"options.on")
                                        : language->getElement(L"options.off"));

    const wchar_t* worldTypeKeys[] = {
        L"selectWorld.mapType.normal", 
        L"selectWorld.mapType.flat",
        L"selectWorld.mapType.largeBiomes",
        L"selectWorld.mapType.amplified",
        L"selectWorld.mapType.triple"
    };
    worldTypeButton->msg =
        language->getElement(L"selectWorld.mapType") + L" " +
        language->getElement(worldTypeKeys[worldType]);

    cheatsEnabledButton->msg =
        language->getElement(L"selectWorld.allowCommands") + L" " +
        (cheatsEnabled ? language->getElement(L"options.on")
                       : language->getElement(L"options.off"));
}

void CreateWorldScreen::updateResultFolder() {
    resultFolder = trimString(nameEdit->getValue());

    for (int i = 0; i < SharedConstants::ILLEGAL_FILE_CHARACTERS_LENGTH; i++) {
        size_t pos;
        while ((pos = resultFolder.find(
                    SharedConstants::ILLEGAL_FILE_CHARACTERS[i])) !=
               std::wstring::npos) {
            resultFolder[pos] = L'_';
        }
    }

    if (resultFolder.length() == 0) {
        resultFolder = L"World";
    }
    resultFolder = CreateWorldScreen::findAvailableFolderName(
        minecraft->getLevelSource(), resultFolder);
}

EditBox* CreateWorldScreen::getVisibleEditBox() const {
    return moreOptions ? seedEdit : nameEdit;
}

void CreateWorldScreen::updateTextBoxFocus() {
    if (nameEdit != nullptr) {
        nameEdit->focus(!moreOptions);
    }
    if (seedEdit != nullptr) {
        seedEdit->focus(moreOptions);
    }
}

void CreateWorldScreen::updateCreateButtonState() {
    if (!buttons.empty()) {
        buttons[0]->active = trimString(nameEdit->getValue()).length() > 0;
    }
}

void CreateWorldScreen::setMoreOptionsVisible(bool visible) {
    moreOptions = visible;
    gameModeButton->visible = !visible;
    gameModeButton->active = !visible;
    generateStructuresButton->visible = visible;
    generateStructuresButton->active = visible;
    bonusChestButton->visible = visible;
    bonusChestButton->active = visible;
    worldTypeButton->visible = visible;
    worldTypeButton->active = visible;
    cheatsEnabledButton->visible = visible;
    cheatsEnabledButton->active = visible;

    Language* language = Language::getInstance();
    moreWorldOptionsButton->msg = visible
                                      ? language->getElement(L"gui.done")
                                      : language->getElement(
                                            L"selectWorld.moreWorldOptions");
    updateTextBoxFocus();
}

std::wstring CreateWorldScreen::findAvailableFolderName(
    LevelStorageSource* levelSource, const std::wstring& folder) {
    std::wstring folder2 = folder;  

    return folder2;
}

void CreateWorldScreen::removed() { Keyboard::enableRepeatEvents(false); }

void CreateWorldScreen::buttonClicked(Button* button) {
    app.DebugPrintf("CreateWorldScreen::buttonClicked START\n");
    if (!button->active) return;
    if (button->id == 1) {
        app.DebugPrintf(
            "CreateWorldScreen::buttonClicked 'Cancel' "
            "minecraft->setScreen(lastScreen)\n");
        minecraft->setScreen(lastScreen);
    } else if (button->id == 0) {
        minecraft->setScreen(
            new Screen());  
        if (done) return;
        done = true;

        MoreOptionsParams* moreOptionsParams = new MoreOptionsParams();

        
        
        
        moreOptionsParams->bGenerateOptions = true;
        moreOptionsParams->bStructures = generateStructures;
        moreOptionsParams->bFlatWorld = worldType == e_levelType_Superflat;
        moreOptionsParams->iLevelType = worldType;
        moreOptionsParams->bBonusChest = bonusChest;
        moreOptionsParams->bPVP = true;
        moreOptionsParams->bTrust = true;
        moreOptionsParams->bFireSpreads = true;
        moreOptionsParams->bTNT = true;
        moreOptionsParams->bHostPrivileges = false;
        moreOptionsParams->bOnlineGame = false;
        moreOptionsParams->bInviteOnly = false;
        moreOptionsParams->bAllowFriendsOfFriends = false;
        moreOptionsParams->bOnlineSettingChangedBySystem = false;
        moreOptionsParams->bCheatsEnabled = cheatsEnabled;
        moreOptionsParams->iPad = 0;

        moreOptionsParams->worldName = nameEdit->getValue();
        moreOptionsParams->seed = seedEdit->getValue();

        moreOptionsParams->dwTexturePack = 0;

        std::wstring worldName = nameEdit->getValue();
        if (worldName.empty()) {
            worldName = L"2slimey";
        }

        {
            File savesDir = Minecraft::getSavesDirectory();
            std::filesystem::path savesPath(savesDir.getPath());
            std::wstring base = worldName;
            std::wstring candidate = base;
            int suffix = 2;
            while (std::filesystem::exists(savesPath / candidate)) {
                candidate = base + L" " + std::to_wstring(suffix);
                ++suffix;
                if (suffix > 9999) break;  
            }
            worldName = candidate;
        }

        StorageManager.ResetSaveData();
        StorageManager.SetSaveTitle((wchar_t*)worldName.c_str());

        std::wstring seedString = seedEdit->getValue();

        int64_t seedValue = 0;
        NetworkGameInitData* param = new NetworkGameInitData();

        if (seedString.length() != 0) {
            int64_t value = fromWString<int64_t>(seedString);

            bool isNumber = true;
            for (unsigned int i = 0; i < seedString.length(); ++i) {
                if (seedString.at(i) < L'0' || seedString.at(i) > L'9') {
                    if (!(i == 0 && seedString.at(i) == L'-')) {
                        isNumber = false;
                        break;
                    }
                }
            }

            if (isNumber) value = fromWString<int64_t>(seedString);

            if (value != 0) {
                seedValue = value;
            } else {
                int hashValue = 0;
                for (unsigned int i = 0; i < seedString.length(); ++i)
                    hashValue = 31 * hashValue + seedString.at(i);
                seedValue = hashValue;
            }
            
            
            
            
        } else {
            param->findSeed = true;
        }

        param->seed = seedValue;
        param->saveData = nullptr;
        param->texturePackId = 0;
        param->settings = 0;

        app.SetGameHostOption(eGameHostOption_Difficulty,
                              minecraft->options->difficulty);
        app.SetGameHostOption(eGameHostOption_FriendsOfFriends,
                              moreOptionsParams->bAllowFriendsOfFriends);
        app.SetGameHostOption(eGameHostOption_Gamertags, 1);
        app.SetGameHostOption(eGameHostOption_BedrockFog, 0);
        app.SetGameHostOption(eGameHostOption_GameType,
                              (gameMode == L"survival")
                                  ? GameType::SURVIVAL->getId()
                                  : GameType::CREATIVE->getId());
        app.SetGameHostOption(eGameHostOption_LevelType,
                              moreOptionsParams->iLevelType);
        app.SetGameHostOption(eGameHostOption_Structures,
                              moreOptionsParams->bStructures);
        app.SetGameHostOption(eGameHostOption_BonusChest,
                              moreOptionsParams->bBonusChest);
        app.SetGameHostOption(eGameHostOption_PvP, moreOptionsParams->bPVP);
        app.SetGameHostOption(eGameHostOption_TrustPlayers,
                              moreOptionsParams->bTrust);
        app.SetGameHostOption(eGameHostOption_FireSpreads,
                              moreOptionsParams->bFireSpreads);
        app.SetGameHostOption(eGameHostOption_TNT, moreOptionsParams->bTNT);
        app.SetGameHostOption(eGameHostOption_HostCanFly,
                              moreOptionsParams->bHostPrivileges);
        app.SetGameHostOption(eGameHostOption_HostCanChangeHunger,
                              moreOptionsParams->bHostPrivileges);
        app.SetGameHostOption(eGameHostOption_HostCanBeInvisible,
                              moreOptionsParams->bHostPrivileges);
        app.SetGameHostOption(eGameHostOption_CheatsEnabled,
                              moreOptionsParams->bHostPrivileges);

        param->settings = app.GetGameHostOption(eGameHostOption_All);
        param->xzSize = LEVEL_MAX_WIDTH;
        param->hellScale = HELL_LEVEL_MAX_SCALE;
        app.ClearTerrainFeaturePosition();

        if (minecraft->user != nullptr && minecraft->options != nullptr &&
            !minecraft->options->lastMpNickname.empty()) {
            minecraft->user->name = minecraft->options->lastMpNickname;
        }

        g_NetworkManager.HostGame(0, false, false, MINECRAFT_NET_MAX_PLAYERS,
                                  0);

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
            new MessageScreen(language->getElement(L"menu.generatingLevel")));
    } else if (button->id == 2) {
        if (gameMode == L"survival")
            gameMode = L"creative";
        else
            gameMode = L"survival";
        updateStrings();
    } else if (button->id == 3) {
        setMoreOptionsVisible(!moreOptions);
    } else if (button->id == 4) {
        generateStructures = !generateStructures;
        updateStrings();
    } else if (button->id == 7) {
        bonusChest = !bonusChest;
        updateStrings();
    } else if (button->id == 5) {
        worldType = (worldType + 1) % 5;
        updateStrings();
    } else if (button->id == 6) {
        cheatsEnabled = !cheatsEnabled;
        updateStrings();
    }
}

void CreateWorldScreen::keyPressed(wchar_t ch, int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        Screen::keyPressed(ch, eventKey);
        return;
    }

    if (eventKey == Keyboard::KEY_TAB) {
        tabPressed();
        return;
    }

    EditBox* visibleEdit = getVisibleEditBox();
    if (visibleEdit != nullptr) {
        visibleEdit->keyPressed(ch, eventKey);
    }

    updateCreateButtonState();

    updateResultFolder();

    if ((eventKey == Keyboard::KEY_RETURN || ch == 13) && buttons[0]->active) {
        buttonClicked(buttons[0]);
    }
}

void CreateWorldScreen::mouseClicked(int x, int y, int buttonNum) {
    Screen::mouseClicked(x, y, buttonNum);

    EditBox* visibleEdit = getVisibleEditBox();
    if (visibleEdit != nullptr) {
        visibleEdit->mouseClicked(x, y, buttonNum);
    }
}

void CreateWorldScreen::render(int xm, int ym, float a) {
    Language* language = Language::getInstance();

    
    renderBackground();

    drawCenteredString(font, language->getElement(L"selectWorld.create"),
                       width / 2, 20, 0xffffff);
    if (!moreOptions) {
        drawString(font, language->getElement(L"selectWorld.enterName"),
                   width / 2 - 100, 47, 0xa0a0a0);
        drawString(font,
                   language->getElement(L"selectWorld.resultFolder") + L" " +
                       resultFolder,
                   width / 2 - 100, 85, 0xa0a0a0);

        nameEdit->render();

        drawString(font, gameModeDescriptionLine1, width / 2 - 100, 122,
                   0xa0a0a0);
        drawString(font, gameModeDescriptionLine2, width / 2 - 100, 134,
                   0xa0a0a0);
    } else {
        drawString(font, language->getElement(L"selectWorld.enterSeed"),
                   width / 2 - 100, 47, 0xa0a0a0);
        drawString(font, language->getElement(L"selectWorld.seedInfo"),
                   width / 2 - 100, 85, 0xa0a0a0);
        drawString(font, language->getElement(L"selectWorld.mapFeatures.info"),
                   width / 2 - 150, 122, 0xa0a0a0);
        drawString(font,
                   language->getElement(L"selectWorld.allowCommands.info"),
                   width / 2 - 150, 157, 0xa0a0a0);

        seedEdit->render();
    }

    Screen::render(xm, ym, a);
}

void CreateWorldScreen::tabPressed() { updateTextBoxFocus(); }
