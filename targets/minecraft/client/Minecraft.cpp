#include "Minecraft.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include <assert.h>
#include <stdlib.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <thread>

#include <SDL2/SDL.h>
#include <SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>

#include "platform/InputActions.h"
#include "platform/JavaKeyInput.h"
#include "platform/sdl2/Profile.h"
#include "platform/sdl2/Render.h"
#include "platform/sdl2/Storage.h"
#include "app/common/App_enums.h"
#include "app/common/src/Audio/SoundEngine.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/common/src/Tutorial/Tutorial.h"
#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "app/include/XboxStubs.h"
#include "Options.h"
#include "Pos.h"
#include "ProgressRenderer.h"
#include "SharedConstants.h"
#include "Timer.h"
#include "User.h"
#include "minecraft/world/entity/player/SkinTypes.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "java/Class.h"
#include "java/Random.h"
#include "minecraft/client/gui/DeathScreen.h"
#include "minecraft/client/gui/ErrorScreen.h"
#include "minecraft/client/gui/PauseScreen.h"
#include "minecraft/client/gui/inventory/InventoryScreen.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/client/gui/particle/GuiParticles.h"
#include "minecraft/client/model/HumanoidModel.h"
#include "minecraft/client/multiplayer/MultiPlayerLevel.h"
#include "minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"
#include "minecraft/client/particle/ParticleEngine.h"
#include "minecraft/client/player/LocalPlayer.h"
#include "minecraft/client/renderer/Chunk.h"
#include "minecraft/client/renderer/GameRenderer.h"
#include "minecraft/client/renderer/ItemInHandRenderer.h"
#include "minecraft/client/renderer/LevelRenderer.h"
#include "minecraft/client/renderer/Tesselator.h"
#include "minecraft/client/renderer/Textures.h"
#include "minecraft/client/renderer/TileRenderer.h"
#include "minecraft/client/renderer/entity/EntityRenderDispatcher.h"
#include "minecraft/client/renderer/tileentity/TileEntityRenderDispatcher.h"
#include "minecraft/client/resources/ResourceLocation.h"
#include "minecraft/client/skins/TexturePack.h"
#include "minecraft/client/skins/TexturePackRepository.h"
#include "minecraft/client/title/TitleScreen.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/network/packet/Packet.h"
#include "minecraft/sounds/SoundTypes.h"
#include "minecraft/stats/Stats.h"
#include "minecraft/stats/StatsCounter.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/ItemFrame.h"
#include "minecraft/world/entity/Mob.h"
#include "minecraft/world/entity/animal/Animal.h"
#include "minecraft/world/entity/animal/EntityHorse.h"
#include "minecraft/world/entity/animal/Ocelot.h"
#include "minecraft/world/entity/animal/Pig.h"
#include "minecraft/world/entity/animal/Sheep.h"
#include "minecraft/world/entity/animal/Wolf.h"
#include "minecraft/world/entity/monster/Spider.h"
#include "minecraft/world/entity/monster/Zombie.h"
#include "minecraft/world/entity/player/Abilities.h"
#include "minecraft/world/entity/player/Inventory.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/food/FoodData.h"
#include "minecraft/world/item/DyePowderItem.h"
#include "minecraft/world/item/FoodItem.h"
#include "minecraft/world/item/GoldenAppleItem.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/item/LeashItem.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/chunk/CompressedTileStorage.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/material/Material.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/storage/LevelStorageSource.h"
#include "minecraft/world/level/storage/McRegionLevelStorageSource.h"
#include "minecraft/world/level/tile/ChestTile.h"
#include "minecraft/world/level/tile/ColoredTile.h"
#include "minecraft/world/level/tile/TallGrassPlantTile.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/phys/HitResult.h"
#include "strings.h"
#if defined(ENABLE_JAVA_GUIS)
#include "minecraft/client/gui/inventory/CreativeInventoryScreen.h"
#endif
#include "platform/sdl2/Input.h"
#include "app/common/Minecraft_Macros.h"
#include "app/common/src/Colours/ColourTable.h"
#include "app/common/src/ConsoleGameMode.h"
#include "app/common/src/DLC/DLCPack.h"
#include "app/common/src/Tutorial/FullTutorialMode.h"
#include "app/common/src/UI/All Platforms/IUIScene_CreativeMenu.h"
#include "app/common/src/UI/UIFontData.h"
#include "app/include/stubs.h"
#include "util/StringHelpers.h"
#include "java/File.h"
#include "java/System.h"
#include "minecraft/StaticConstructors.h"
#include "minecraft/client/MemoryTracker.h"
#include "minecraft/client/KeyMapping.h"
#include "minecraft/client/gui/ChatScreen.h"
#include "minecraft/client/gui/Font.h"
#include "minecraft/client/gui/Gui.h"
#include "minecraft/client/gui/InBedChatScreen.h"
#include "minecraft/client/gui/ScreenSizeCalculator.h"
#include "minecraft/client/gui/achievement/AchievementPopup.h"
#include "minecraft/client/multiplayer/ClientConnection.h"
#include "minecraft/client/multiplayer/MultiPlayerGameMode.h"
#include "minecraft/client/player/Input.h"
#include "minecraft/client/renderer/texture/TextureManager.h"
#include "minecraft/client/skins/DLCTexturePack.h"
#include "minecraft/world/entity/npc/Villager.h"
#include "minecraft/world/item/alchemy/PotionMacros.h"
#include "minecraft/world/level/chunk/SparseDataStorage.h"
#include "minecraft/world/level/chunk/SparseLightStorage.h"

class ChunkSource;

namespace {

bool TranslateJavaGuiScancodeToChar(int scancode, bool shift, wchar_t& ch) {
    SDL_Keycode keycode =
        SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(scancode));

    if (keycode >= SDLK_a && keycode <= SDLK_z) {
        ch = static_cast<wchar_t>((shift ? L'A' : L'a') +
                                  (keycode - SDLK_a));
        return true;
    }

    if (keycode >= SDLK_0 && keycode <= SDLK_9) {
        static const wchar_t digits[] = L"0123456789";
        static const wchar_t shiftedDigits[] = L")!@#$%^&*(";
        int idx = static_cast<int>(keycode - SDLK_0);
        ch = shift ? shiftedDigits[idx] : digits[idx];
        return true;
    }

    switch (keycode) {
        case SDLK_SPACE:
            ch = L' ';
            return true;
        case SDLK_MINUS:
            ch = shift ? L'_' : L'-';
            return true;
        case SDLK_EQUALS:
            ch = shift ? L'+' : L'=';
            return true;
        case SDLK_LEFTBRACKET:
            ch = shift ? L'{' : L'[';
            return true;
        case SDLK_RIGHTBRACKET:
            ch = shift ? L'}' : L']';
            return true;
        case SDLK_BACKSLASH:
            ch = shift ? L'|' : L'\\';
            return true;
        case SDLK_SEMICOLON:
            ch = shift ? L':' : L';';
            return true;
        case SDLK_QUOTE:
            ch = shift ? L'"' : L'\'';
            return true;
        case SDLK_COMMA:
            ch = shift ? L'<' : L',';
            return true;
        case SDLK_PERIOD:
            ch = shift ? L'>' : L'.';
            return true;
        case SDLK_SLASH:
            ch = shift ? L'?' : L'/';
            return true;
        case SDLK_BACKQUOTE:
            ch = shift ? L'~' : L'`';
            return true;
        default:
            return false;
    }
}

bool HasAcceptedJavaGuiTextInput() {
    for (wchar_t ch : JavaKeyInput::typedChars) {
        if (SharedConstants::acceptableLetters.find(ch) !=
            std::wstring::npos) {
            return true;
        }
    }
    return false;
}

void DispatchJavaScreenKeyboard(Screen* screen) {
    for (int key : JavaKeyInput::pressedKeys) {
        if (screen != nullptr) screen->keyPressed(0, key);
    }

    for (wchar_t ch : JavaKeyInput::typedChars) {
        if (screen != nullptr) screen->keyPressed(ch, 0);
    }

    if (!HasAcceptedJavaGuiTextInput()) {
        bool shift =
            Keyboard::isKeyDown(Keyboard::KEY_LSHIFT) ||
            Keyboard::isKeyDown(Keyboard::KEY_RSHIFT);
        for (int key : JavaKeyInput::pressedKeys) {
            wchar_t ch;
            if (TranslateJavaGuiScancodeToChar(key, shift, ch) &&
                screen != nullptr) {
                screen->keyPressed(ch, 0);
            }
        }
    }
}

}  










#define DISABLE_LEVELTICK_THREAD

Minecraft* Minecraft::m_instance = nullptr;
int64_t Minecraft::frameTimes[512];
int64_t Minecraft::tickTimes[512];
int Minecraft::frameTimePos = 0;
int64_t Minecraft::warezTime = 0;
File Minecraft::workDir = File(L"");

ResourceLocation Minecraft::DEFAULT_FONT_LOCATION =
    ResourceLocation(TN_DEFAULT_FONT);
ResourceLocation Minecraft::ALT_FONT_LOCATION = ResourceLocation(TN_ALT_FONT);

Minecraft::Minecraft(Component* mouseComponent, Canvas* parent,
                     MinecraftApplet* minecraftApplet, int width, int height,
                     bool fullscreen) {
    
    gameMode = nullptr;
    hasCrashed = false;
    timer = new Timer(SharedConstants::TICKS_PER_SECOND);
    oldLevel = nullptr;  
    level = nullptr;
    levels = std::vector<MultiPlayerLevel*>(3);  
    levelRenderer = nullptr;
    player = nullptr;
    cameraTargetPlayer = nullptr;
    particleEngine = nullptr;
    user = nullptr;
    parent = nullptr;
    pause = false;
    exitingWorldRightNow = false;
    textures = nullptr;
    font = nullptr;
    screen = nullptr;
    localPlayerIdx = 0;
    rightClickDelay = 0;

    
    

    progressRenderer = nullptr;
    gameRenderer = nullptr;
    bgLoader = nullptr;

    ticks = 0;
    
    
    
    
    

    orgWidth = orgHeight = 0;
    achievementPopup = new AchievementPopup(this);
    gui = nullptr;
    noRender = false;
    humanoidModel = new HumanoidModel(0);
    hitResult = 0;
    options = nullptr;
    soundEngine = new SoundEngine();
    mouseHandler = nullptr;
    skins = nullptr;
    workingDirectory = File(L"");
    levelSource = nullptr;
    stats[0] = nullptr;
    stats[1] = nullptr;
    stats[2] = nullptr;
    stats[3] = nullptr;
    connectToPort = 0;
    workDir = File(L"");
    
    
    lastTimer = -1;

    
    
    recheckPlayerIn = 0;
    running = true;
    unoccupiedQuadrant = -1;

    Stats::init();

    orgHeight = height;
    this->fullscreen = fullscreen;
    this->minecraftApplet = nullptr;

    this->parent = parent;
    
    
    
    
    
    if (RenderManager.IsWidescreen()) {
        this->width = width;
    } else {
        this->width = (width * 3) / 4;
    }
    this->height = height;
    this->width_phys = width;
    this->height_phys = height;

    this->fullscreen = fullscreen;

    appletMode = false;

    Minecraft::m_instance = this;
    TextureManager::createInstance();

    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        m_pendingLocalConnections[i] = nullptr;
        m_connectionFailed[i] = false;
        localgameModes[i] = nullptr;
    }

    animateTickLevel = nullptr;  
    m_inFullTutorialBits = 0;    
    reloadTextures = false;

    
    
    

    
    
    this->soundEngine->init(nullptr);

#if !defined(DISABLE_LEVELTICK_THREAD)
    levelTickEventQueue =
        new C4JThread::EventQueue(levelTickUpdateFunc, levelTickThreadInitFunc,
                                  "LevelTick_EventQueuePoll");
    levelTickEventQueue->setPriority(C4JThread::ThreadPriority::Normal);
#endif
}

void Minecraft::clearConnectionFailed() {
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        m_connectionFailed[i] = false;
        m_connectionFailedReason[i] = DisconnectPacket::eDisconnect_None;
    }
    app.SetDisconnectReason(DisconnectPacket::eDisconnect_None);
}

void Minecraft::connectTo(const std::wstring& server, int port) {
    connectToIp = server;
    connectToPort = port;
}

void Minecraft::init() {
    

    workingDirectory = getWorkingDirectory();
    levelSource =
        new McRegionLevelStorageSource(File(workingDirectory, L"saves"));
    
    options = new Options(this, workingDirectory);
    if (soundEngine != nullptr) {
        soundEngine->updateMusicVolume(options->music);
        soundEngine->updateSoundEffectVolume(options->sound);
    }
    skins = new TexturePackRepository(workingDirectory, this);
    skins->addDebugPacks();
    textures = new Textures(skins, options);
    

    font =
        new Font(options, L"font/Default.png", textures, false,
                 &DEFAULT_FONT_LOCATION, 23, 20, 8, 8, SFontData::Codepoints);
    altFont = new Font(options, L"font/alternate.png", textures, false,
                       &ALT_FONT_LOCATION, 16, 16, 8, 8);

    
    
    
    
    
    
    

    
    
    
    

    gameRenderer = new GameRenderer(this);
    EntityRenderDispatcher::instance->itemInHandRenderer =
        new ItemInHandRenderer(this, false);

    for (int i = 0; i < 4; ++i) stats[i] = new StatsCounter();

    








    
    

    
    Mouse::create();

    checkGlError(L"Pre startup");

    
    

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    glCullFace(GL_BACK);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    checkGlError(L"Startup");

    

    levelRenderer = new LevelRenderer(this, textures);
    
    
    
    
    textures->stitch();

    glViewport(0, 0, width, height);

    particleEngine = new ParticleEngine(level, textures);
    
    bgLoader = new BackgroundDownloader(workingDirectory, this);
    bgLoader->start();
    
    

    checkGlError(L"Post startup");
    gui = new Gui(this);

    if (connectToIp != L"")  
    {
        
        
    } else {
        setScreen(new TitleScreen());
    }
    progressRenderer = new ProgressRenderer(this);

    RenderManager.CBuffLockStaticCreations();
}

void Minecraft::renderLoadingScreen() {
    
    
#if defined(ENABLE_JAVA_GUIS)
    ScreenSizeCalculator ssc(options, width, height);

    
    RenderManager.StartFrame();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (float)ssc.rawWidth, (float)ssc.rawHeight, 0, 1000, 3000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -2000);
    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 0);

    Tesselator* t = Tesselator::getInstance();

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_FOG);
    
    glBindTexture(GL_TEXTURE_2D, textures->loadTexture(TN_MOB_PIG));
    t->begin();
    t->color(0xffffff);
    t->vertexUV((float)(0), (float)(height), (float)(0), (float)(0),
                (float)(0));
    t->vertexUV((float)(width), (float)(height), (float)(0), (float)(0),
                (float)(0));
    t->vertexUV((float)(width), (float)(0), (float)(0), (float)(0), (float)(0));
    t->vertexUV((float)(0), (float)(0), (float)(0), (float)(0), (float)(0));
    t->end();

    int lw = 256;
    int lh = 256;
    glColor4f(1, 1, 1, 1);
    t->color(0xffffff);
    blit((ssc.getWidth() - lw) / 2, (ssc.getHeight() - lh) / 2, 0, 0, lw, lh);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    
    
    RenderManager.Present();
#endif
}

void Minecraft::blit(int x, int y, int sx, int sy, int w, int h) {
    float us = 1 / 256.0f;
    float vs = 1 / 256.0f;
    Tesselator* t = Tesselator::getInstance();
    t->begin();
    t->vertexUV((float)(x + 0), (float)(y + h), (float)(0),
                (float)((sx + 0) * us), (float)((sy + h) * vs));
    t->vertexUV((float)(x + w), (float)(y + h), (float)(0),
                (float)((sx + w) * us), (float)((sy + h) * vs));
    t->vertexUV((float)(x + w), (float)(y + 0), (float)(0),
                (float)((sx + w) * us), (float)((sy + 0) * vs));
    t->vertexUV((float)(x + 0), (float)(y + 0), (float)(0),
                (float)((sx + 0) * us), (float)((sy + 0) * vs));
    t->end();
}

File Minecraft::getWorkingDirectory() {
    if (workDir.getPath().empty()) workDir = getWorkingDirectory(L"4jcraft");
    return workDir;
}

File Minecraft::getWorkingDirectory(const std::wstring& applicationName) {
    
    
    std::wstring userHome = convStringToWstring(getenv("HOME"));
    File* workingDirectory;
#if defined(__APPLE__)
    workingDirectory = new File(userHome, L"Library/Application Support/" + applicationName);
#elif defined(_WINDOWS64)
    std::string applicationData = getenv("APPDATA");
    if (!applicationData.empty()) {
        workingDirectory = new File(convStringToWstring(applicationData),
                                    L'.' + applicationName + L'/');
    } else {
        workingDirectory = new File(userHome, L'.' + applicationName + L'/');
    }
#else
    workingDirectory = new File(userHome, applicationName + L'/');
#endif
    if (!workingDirectory->exists()) {
        if (!workingDirectory->mkdirs()) {
            app.DebugPrintf("The working directory could not be created");
            assert(0);
            
            
        }
    }
    return *workingDirectory;
}

File Minecraft::getSavesDirectory() {
    File wd = getWorkingDirectory();
    File savesDir(wd, L"Saves");
    if (!savesDir.exists()) {
        savesDir.mkdirs();
    }
    return savesDir;
}

LevelStorageSource* Minecraft::getLevelSource() { return levelSource; }

void Minecraft::setScreen(Screen* screen) {
    if (dynamic_cast<ErrorScreen*>(this->screen) != nullptr) return;

    if (this->screen != nullptr) {
        this->screen->removed();
    }

    
    





    if (screen == nullptr && level == nullptr) {
        screen = new TitleScreen();
    } else if (player != nullptr &&
               !ui.GetMenuDisplayed(player->GetXboxPad()) &&
               player->getHealth() <= 0) {
#if defined(ENABLE_JAVA_GUIS)
        screen = new DeathScreen();
#else
        
        
        
        
        if (ticks == 0) {
            player->respawn();
        } else {
            ui.NavigateToScene(player->GetXboxPad(), eUIScene_DeathMenu,
                               nullptr);
        }
#endif
    }
    this->screen = screen;

    if (dynamic_cast<TitleScreen*>(screen) != nullptr) {
        options->renderDebug = false;
        gui->clearMessages();
    }

    if (screen != nullptr) {
        
        ScreenSizeCalculator ssc(options, width, height);
        int screenWidth = ssc.getWidth();
        int screenHeight = ssc.getHeight();
        screen->init(this, screenWidth, screenHeight);
        noRender = false;
    } else {
        
    }

    
    
    
#if defined(ENABLE_JAVA_GUIS)
    if (screen != nullptr && player != nullptr) {
        if (player && player->GetXboxPad() != -1) {
            InputManager.SetMenuDisplayed(player->GetXboxPad(), true);
        }
    } else if (player != nullptr) {
        if (player && player->GetXboxPad() != -1) {
            InputManager.SetMenuDisplayed(player->GetXboxPad(), false);
        }
    }
#endif
}

void Minecraft::checkGlError(const std::wstring& string) {
    
}

void Minecraft::destroy() {
    
    


    
    
    if (this->bgLoader != nullptr) {
        bgLoader->halt();
    }
    
    

    
    setLevel(nullptr);
    
    

    if (screen == nullptr && level == nullptr) {
        screen = new TitleScreen();
    } else if (player != nullptr &&
               !ui.GetMenuDisplayed(player->GetXboxPad()) &&
               player->getHealth() <= 0) {
#if defined(ENABLE_JAVA_GUIS)
        screen = new DeathScreen();
#else
        
        
        
        
        if (ticks == 0) {
            player->respawn();
        } else {
            ui.NavigateToScene(player->GetXboxPad(), eUIScene_DeathMenu,
                               nullptr);
        }
#endif
    }

    if (screen != nullptr && dynamic_cast<TitleScreen*>(screen) != nullptr) {
        options->renderDebug = false;
        gui->clearMessages();
    }

    if (screen != nullptr) {
        
        ScreenSizeCalculator ssc(options, width, height);
        int screenWidth = ssc.getWidth();
        int screenHeight = ssc.getHeight();
        screen->init(this, screenWidth, screenHeight);
        noRender = false;
    } else {
        
    }

    
    
#if defined(ENABLE_JAVA_GUIS)
    if (screen != nullptr) {
        if (player && player->GetXboxPad() != -1) {
            InputManager.SetMenuDisplayed(player->GetXboxPad(), true);
        }
    } else {
        if (player && player->GetXboxPad() != -1) {
            InputManager.SetMenuDisplayed(player->GetXboxPad(), false);
        }
    }
#endif
    
    MemoryTracker::release();
    
    

    soundEngine->destroy();
    Mouse::destroy();
    Keyboard::destroy();
    
    Display::destroy();
    
    
    
}




void Minecraft::run() {
    running = true;
    
    init();
    
    
    
    
    
    
}



bool Minecraft::setLocalPlayerIdx(int idx) {
    localPlayerIdx = idx;
    
    
    if (localplayers[idx] == nullptr || localgameModes[idx] == nullptr)
        return false;

    gameMode = localgameModes[idx];
    player = localplayers[idx];
    cameraTargetPlayer = localplayers[idx];
    gameRenderer->itemInHandRenderer = localitemInHandRenderers[idx];
    level = getLevel(localplayers[idx]->dimension);
    particleEngine->setLevel(level);

    return true;
}

int Minecraft::getLocalPlayerIdx() { return localPlayerIdx; }

void Minecraft::updatePlayerViewportAssignments() {
    unoccupiedQuadrant = -1;
    
    int viewportsRequired = 0;
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        if (localplayers[i] != nullptr) viewportsRequired++;
    }
    if (viewportsRequired == 3) viewportsRequired = 4;

    
    if (viewportsRequired == 1) {
        
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            if (localplayers[i] != nullptr)
                localplayers[i]->m_iScreenSection =
                    C4JRender::VIEWPORT_TYPE_FULLSCREEN;
        }
    } else if (viewportsRequired == 2) {
        
        int found = 0;
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            if (localplayers[i] != nullptr) {
                
                if (app.GetGameSettings(InputManager.GetPrimaryPad(),
                                        eGameSetting_SplitScreenVertical)) {
                    localplayers[i]->m_iScreenSection =
                        C4JRender::VIEWPORT_TYPE_SPLIT_LEFT + found;
                } else {
                    localplayers[i]->m_iScreenSection =
                        C4JRender::VIEWPORT_TYPE_SPLIT_TOP + found;
                }
                found++;
            }
        }
    } else if (viewportsRequired >= 3) {
        
        
        
        bool quadrantsAllocated[4] = {false, false, false, false};

        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            if (localplayers[i] != nullptr) {
                
                
                
                
                if (app.GetGameStarted()) {
                    if ((localplayers[i]->m_iScreenSection >=
                         C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT) &&
                        (localplayers[i]->m_iScreenSection <=
                         C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT)) {
                        quadrantsAllocated
                            [localplayers[i]->m_iScreenSection -
                             C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT] = true;
                    }
                } else {
                    
                    
                    localplayers[i]->m_iScreenSection =
                        C4JRender::VIEWPORT_TYPE_FULLSCREEN;
                }
            }
        }

        
        
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            if (localplayers[i] != nullptr) {
                if ((localplayers[i]->m_iScreenSection <
                     C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT) ||
                    (localplayers[i]->m_iScreenSection >
                     C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT)) {
                    for (int j = 0; j < 4; j++) {
                        if (!quadrantsAllocated[j]) {
                            localplayers[i]->m_iScreenSection =
                                C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT + j;
                            quadrantsAllocated[j] = true;
                            break;
                        }
                    }
                }
            }
        }
        
        
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            if (quadrantsAllocated[i] == false) {
                unoccupiedQuadrant = i;
            }
        }
    }

    
    
    
    if (app.GetGameStarted()) ui.UpdatePlayerBasePositions();
}



bool Minecraft::addLocalPlayer(int idx) {
    
    if (m_pendingLocalConnections[idx] != nullptr) {
        
        assert(false);
        m_pendingLocalConnections[idx]->close();
    }
    m_connectionFailed[idx] = false;
    m_pendingLocalConnections[idx] = nullptr;

    bool success = g_NetworkManager.AddLocalPlayerByUserIndex(idx);

    if (success) {
        app.DebugPrintf("Adding temp local player on pad %d\n", idx);
        localplayers[idx] = std::shared_ptr<MultiplayerLocalPlayer>(
            new MultiplayerLocalPlayer(this, level, user, nullptr));
        localgameModes[idx] = nullptr;

        updatePlayerViewportAssignments();

        ConnectionProgressParams* param = new ConnectionProgressParams();
        param->iPad = idx;
        param->stringId = IDS_PROGRESS_CONNECTING;
        param->showTooltips = true;
        param->setFailTimer = true;
        param->timerTime = CONNECTING_PROGRESS_CHECK_TIME;

        
        ui.NavigateToScene(idx, eUIScene_ConnectingProgress, param);

    } else {
        app.DebugPrintf("g_NetworkManager.AddLocalPlayerByUserIndex failed\n");
    }

    return success;
}

void Minecraft::addPendingLocalConnection(int idx,
                                          ClientConnection* connection) {
    m_pendingLocalConnections[idx] = connection;
}

std::shared_ptr<MultiplayerLocalPlayer> Minecraft::createExtraLocalPlayer(
    int idx, const std::wstring& name, int iPad, int iDimension,
    ClientConnection* clientConnection ,
    MultiPlayerLevel* levelpassedin) {
    if (clientConnection == nullptr) return nullptr;

    if (clientConnection == m_pendingLocalConnections[idx]) {
        int tempScreenSection = C4JRender::VIEWPORT_TYPE_FULLSCREEN;
        if (localplayers[idx] != nullptr && localgameModes[idx] == nullptr) {
            
            tempScreenSection = localplayers[idx]->m_iScreenSection;
        }
        std::wstring prevname = user->name;
        user->name = name;

        
        m_pendingLocalConnections[idx] = nullptr;

        
        
        
        
        

        MultiPlayerLevel* mpLevel;

        if (levelpassedin) {
            level = levelpassedin;
            mpLevel = levelpassedin;
        } else {
            level = getLevel(iDimension);
            mpLevel = getLevel(iDimension);
            mpLevel->addClientConnection(clientConnection);
        }

        if (app.GetTutorialMode()) {
            localgameModes[idx] =
                new FullTutorialMode(idx, this, clientConnection);
        } else {
            localgameModes[idx] =
                new ConsoleGameMode(idx, this, clientConnection);
        }

        
        
        
        
        localplayers[idx] = localgameModes[idx]->createPlayer(level);

        PlayerUID playerXUIDOffline = INVALID_XUID;
        PlayerUID playerXUIDOnline = INVALID_XUID;
        ProfileManager.GetXUID(idx, &playerXUIDOffline, false);
        ProfileManager.GetXUID(idx, &playerXUIDOnline, true);
        localplayers[idx]->setXuid(playerXUIDOffline);
        localplayers[idx]->setOnlineXuid(playerXUIDOnline);
        localplayers[idx]->setIsGuest(ProfileManager.IsGuest(idx));

        localplayers[idx]->m_displayName = ProfileManager.GetDisplayName(idx);

        localplayers[idx]->m_iScreenSection = tempScreenSection;

        if (levelpassedin == nullptr)
            level->addEntity(
                localplayers[idx]);  
                                     
                                     
                                     

        localplayers[idx]->SetXboxPad(iPad);

        if (localplayers[idx]->input != nullptr)
            delete localplayers[idx]->input;
        localplayers[idx]->input = new Input();

        localplayers[idx]->resetPos();

        levelRenderer->setLevel(idx, level);
        localplayers[idx]->level = level;

        user->name = prevname;

        updatePlayerViewportAssignments();

        
        
        
        
        
        
        
        
        
        
    }

    return localplayers[idx];
}


void Minecraft::storeExtraLocalPlayer(int idx) {
    localplayers[idx] = player;

    if (localplayers[idx]->input != nullptr) delete localplayers[idx]->input;
    localplayers[idx]->input = new Input();

    if (ProfileManager.IsSignedIn(idx)) {
        localplayers[idx]->name =
            convStringToWstring(ProfileManager.GetGamertag(idx));
    }
}

void Minecraft::removeLocalPlayerIdx(int idx) {
    bool updateXui = true;
    if (localgameModes[idx] != nullptr) {
        if (getLevel(localplayers[idx]->dimension)->isClientSide) {
            std::shared_ptr<MultiplayerLocalPlayer> mplp = localplayers[idx];
            ((MultiPlayerLevel*)getLevel(localplayers[idx]->dimension))
                ->removeClientConnection(mplp->connection, true);
            delete mplp->connection;
            mplp->connection = nullptr;
            g_NetworkManager.RemoveLocalPlayerByUserIndex(idx);
        }
        getLevel(localplayers[idx]->dimension)->removeEntity(localplayers[idx]);

        
        
        
        playerLeftTutorial(idx);

        delete localgameModes[idx];
        localgameModes[idx] = nullptr;
    } else if (m_pendingLocalConnections[idx] != nullptr) {
        m_pendingLocalConnections[idx]->sendAndDisconnect(
            std::shared_ptr<DisconnectPacket>(
                new DisconnectPacket(DisconnectPacket::eDisconnect_Quitting)));
        ;
        delete m_pendingLocalConnections[idx];
        m_pendingLocalConnections[idx] = nullptr;
        g_NetworkManager.RemoveLocalPlayerByUserIndex(idx);
    } else {
        
        
        
        
        
    }
    localplayers[idx] = nullptr;

    if (idx == InputManager.GetPrimaryPad()) {
        
        assert(false);
        










    } else if (updateXui) {
        gameRenderer->DisableUpdateThread();
        levelRenderer->setLevel(idx, nullptr);
        gameRenderer->EnableUpdateThread();
        ui.CloseUIScenes(idx, true);
        updatePlayerViewportAssignments();
    }

    
    
}

void Minecraft::createPrimaryLocalPlayer(int iPad) {
    localgameModes[iPad] = gameMode;
    localplayers[iPad] = player;
    
    
    if (ProfileManager.IsSignedIn(InputManager.GetPrimaryPad())) {
        user->name = convStringToWstring(
            ProfileManager.GetGamertag(InputManager.GetPrimaryPad()));
    }
}

void Minecraft::run_middle() {
    static int64_t lastTime = 0;
    static bool bFirstTimeIntoGame = true;
    static bool bAutosaveTimerSet = false;
    static unsigned int uiAutosaveTimer = 0;
    static int iFirstTimeCountdown = 60;
    if (lastTime == 0) lastTime = System::nanoTime();
    static int frames = 0;

#if defined(ENABLE_JAVA_GUIS)
    
    if (exitingWorldRightNow) {
        screen->render(0, 0, 1);
        return;
    }
#endif

    {
        std::lock_guard<std::recursive_mutex> lock(m_setLevelCS);

        if (running) {
            if (reloadTextures) {
                reloadTextures = false;
                textures->reloadAll();
            }

            
            {
                
                
                
                

                
                
                
                
                

                
                if (level != nullptr && g_NetworkManager.IsHost()) {
                    







                    {
                        
                        
                        
                        
                        if (!StorageManager.GetSaveDisabled() &&
                            (app.GetXuiAction(InputManager.GetPrimaryPad()) ==
                             eAppAction_Idle)) {
                            if (!ui.IsPauseMenuDisplayed(
                                    InputManager.GetPrimaryPad()) &&
                                !ui.IsIgnoreAutosaveMenuDisplayed(
                                    InputManager.GetPrimaryPad())) {
                                
                                
                                unsigned char ucAutosaveVal =
                                    app.GetGameSettings(
                                        InputManager.GetPrimaryPad(),
                                        eGameSetting_Autosave);
                                bool bTrialTexturepack = false;
                                if (!Minecraft::GetInstance()
                                         ->skins->isUsingDefaultSkin()) {
                                    TexturePack* tPack =
                                        Minecraft::GetInstance()
                                            ->skins->getSelected();
                                    DLCTexturePack* pDLCTexPack =
                                        (DLCTexturePack*)tPack;

                                    DLCPack* pDLCPack =
                                        pDLCTexPack->getDLCInfoParentPack();

                                    if (pDLCPack) {
                                        if (!pDLCPack->hasPurchasedFile(
                                                DLCManager::e_DLCType_Texture,
                                                L"")) {
                                            bTrialTexturepack = true;
                                        }
                                    }
                                }

                                
                                
                                
                                if ((ucAutosaveVal != 0) &&
                                    !bTrialTexturepack) {
                                    if (app.AutosaveDue()) {
                                        
                                        ui.ShowAutosaveCountdownTimer(false);

                                        
                                        app.DebugPrintf("+++++++++++\n");
                                        app.DebugPrintf("+++Autosave\n");
                                        app.DebugPrintf("+++++++++++\n");
                                        app.SetAction(
                                            InputManager.GetPrimaryPad(),
                                            eAppAction_AutosaveSaveGame);
                                        
#if !defined(_CONTENT_PACKAGE)
                                        {
                                            
                                            auto now_tp = std::chrono::
                                                system_clock::now();
                                            std::time_t now_tt = std::chrono::
                                                system_clock::to_time_t(now_tp);
                                            std::tm utcTime{};
#if defined(_WIN32)
                                            gmtime_s(&utcTime, &now_tt);
#else
                                            gmtime_r(&now_tt, &utcTime);
#endif

                                            app.DebugPrintf("%02d:%02d:%02d\n",
                                                            utcTime.tm_hour,
                                                            utcTime.tm_min,
                                                            utcTime.tm_sec);
                                        }
#endif
                                    } else {
                                        int64_t uiTimeToAutosave =
                                            app.SecondsToAutosave();

                                        if (uiTimeToAutosave < 6) {
                                            ui.ShowAutosaveCountdownTimer(true);
                                            ui.UpdateAutosaveCountdownTimer(
                                                uiTimeToAutosave);
                                        }
                                    }
                                }
                            } else {
                                
                                ui.ShowAutosaveCountdownTimer(false);
                            }
                        }
                    }
                }

                
                
                
                for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                    if (localplayers[i] && (app.GetBanListCheck(i) == false) &&
                        !Minecraft::GetInstance()->isTutorial() &&
                        ProfileManager.IsSignedInLive(i) &&
                        !ProfileManager.IsGuest(i)) {
                        
                        
                        if (!ProfileManager.IsSystemUIDisplayed()) {
                            app.SetBanListCheck(i, true);
                            
                            
                            
                            INetworkPlayer* pHostPlayer =
                                g_NetworkManager.GetHostPlayer();
                            PlayerUID xuid = pHostPlayer->GetUID();

                            if (app.IsInBannedLevelList(
                                    i, xuid, app.GetUniqueMapName())) {
                                
                                
                                app.DebugPrintf("This level is banned\n");
                                
                                
                                
                                app.SetAction(i, eAppAction_LevelInBanLevelList,
                                              (void*)true);
                            }
                        }
                    }
                }

                if (!ProfileManager.IsSystemUIDisplayed() &&
                    app.DLCInstallProcessCompleted() &&
                    !app.DLCInstallPending() &&
                    app.m_dlcManager.NeedsCorruptCheck()) {
                    app.m_dlcManager.checkForCorruptDLCAndAlert();
                }

                
                
                
                
                if (level != nullptr && bFirstTimeIntoGame &&
                    g_NetworkManager.SessionHasSpace()) {
                    
                    if (iFirstTimeCountdown == 0) {
                        bFirstTimeIntoGame = false;

                        if (app.IsLocalMultiplayerAvailable()) {
                            for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                                if ((localplayers[i] == nullptr) &&
                                    InputManager.IsPadConnected(i)) {
                                    if (!ui.PressStartPlaying(i)) {
                                        ui.ShowPressStart(i);
                                    }
                                }
                            }
                        }
                    } else
                        iFirstTimeCountdown--;
                }
                
                
                

                for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                    if (localplayers[i]) {
                        
                        if (InputManager.ButtonPressed(i,
                                                       MINECRAFT_ACTION_JUMP))
                            localplayers[i]->ullButtonsPressed |=
                                1LL << MINECRAFT_ACTION_JUMP;
                        if (InputManager.ButtonPressed(i, MINECRAFT_ACTION_USE))
                            localplayers[i]->ullButtonsPressed |=
                                1LL << MINECRAFT_ACTION_USE;

                        if (InputManager.ButtonPressed(
                                i, MINECRAFT_ACTION_INVENTORY))
                            localplayers[i]->ullButtonsPressed |=
                                1LL << MINECRAFT_ACTION_INVENTORY;
                        if (InputManager.ButtonPressed(i,
                                                       MINECRAFT_ACTION_ACTION))
                            localplayers[i]->ullButtonsPressed |=
                                1LL << MINECRAFT_ACTION_ACTION;
                        if (InputManager.ButtonPressed(
                                i, MINECRAFT_ACTION_CRAFTING))
                            localplayers[i]->ullButtonsPressed |=
                                1LL << MINECRAFT_ACTION_CRAFTING;
                        if (InputManager.ButtonPressed(
                                i, MINECRAFT_ACTION_PAUSEMENU)) {
                            localplayers[i]->ullButtonsPressed |=
                                1LL << MINECRAFT_ACTION_PAUSEMENU;
                            app.DebugPrintf(
                                "PAUSE PRESSED - ipad = %d, Storing press\n",
                                i);
#if defined(ENABLE_JAVA_GUIS)
                            pauseGame();
#endif
                        }
                        if (InputManager.ButtonPressed(i,
                                                       MINECRAFT_ACTION_DROP))
                            localplayers[i]->ullButtonsPressed |=
                                1LL << MINECRAFT_ACTION_DROP;

                        
                        
                        if (localplayers[i]->abilities.flying) {
                            if (InputManager.ButtonDown(
                                    i, MINECRAFT_ACTION_SNEAK_TOGGLE))
                                localplayers[i]->ullButtonsPressed |=
                                    1LL << MINECRAFT_ACTION_SNEAK_TOGGLE;
                        } else {
                            if (InputManager.ButtonPressed(
                                    i, MINECRAFT_ACTION_SNEAK_TOGGLE))
                                localplayers[i]->ullButtonsPressed |=
                                    1LL << MINECRAFT_ACTION_SNEAK_TOGGLE;
                        }
                        if (InputManager.ButtonPressed(
                                i, MINECRAFT_ACTION_RENDER_THIRD_PERSON))
                            localplayers[i]->ullButtonsPressed |=
                                1LL << MINECRAFT_ACTION_RENDER_THIRD_PERSON;
                        if (InputManager.ButtonPressed(
                                i, MINECRAFT_ACTION_GAME_INFO))
                            localplayers[i]->ullButtonsPressed |=
                                1LL << MINECRAFT_ACTION_GAME_INFO;

#if !defined(_FINAL_BUILD)
                        if (app.DebugSettingsOn() && app.GetUseDPadForDebug()) {
                            localplayers[i]->ullDpad_last = 0;
                            localplayers[i]->ullDpad_this = 0;
                            localplayers[i]->ullDpad_filtered = 0;
                            if (InputManager.ButtonPressed(
                                    i, MINECRAFT_ACTION_DPAD_RIGHT))
                                localplayers[i]->ullButtonsPressed |=
                                    1LL << MINECRAFT_ACTION_CHANGE_SKIN;
                            if (InputManager.ButtonPressed(
                                    i, MINECRAFT_ACTION_DPAD_UP))
                                localplayers[i]->ullButtonsPressed |=
                                    1LL << MINECRAFT_ACTION_FLY_TOGGLE;
                            if (InputManager.ButtonPressed(
                                    i, MINECRAFT_ACTION_DPAD_DOWN))
                                localplayers[i]->ullButtonsPressed |=
                                    1LL << MINECRAFT_ACTION_RENDER_DEBUG;
                            if (InputManager.ButtonPressed(
                                    i, MINECRAFT_ACTION_DPAD_LEFT))
                                localplayers[i]->ullButtonsPressed |=
                                    1LL << MINECRAFT_ACTION_SPAWN_CREEPER;
                        } else
#endif
                        {
                            
                            
                            
                            
                            
                            

                            localplayers[i]->ullDpad_this = 0;
                            int dirCount = 0;

                            if (InputManager.ButtonDown(
                                    i, MINECRAFT_ACTION_DPAD_LEFT)) {
                                localplayers[i]->ullDpad_this |=
                                    1LL << MINECRAFT_ACTION_DPAD_LEFT;
                                dirCount++;
                            }
                            if (InputManager.ButtonDown(
                                    i, MINECRAFT_ACTION_DPAD_RIGHT)) {
                                localplayers[i]->ullDpad_this |=
                                    1LL << MINECRAFT_ACTION_DPAD_RIGHT;
                                dirCount++;
                            }
                            if (InputManager.ButtonDown(
                                    i, MINECRAFT_ACTION_DPAD_UP)) {
                                localplayers[i]->ullDpad_this |=
                                    1LL << MINECRAFT_ACTION_DPAD_UP;
                                dirCount++;
                            }
                            if (InputManager.ButtonDown(
                                    i, MINECRAFT_ACTION_DPAD_DOWN)) {
                                localplayers[i]->ullDpad_this |=
                                    1LL << MINECRAFT_ACTION_DPAD_DOWN;
                                dirCount++;
                            }

                            if (dirCount <= 1) {
                                localplayers[i]->ullDpad_last =
                                    localplayers[i]->ullDpad_this;
                                localplayers[i]->ullDpad_filtered =
                                    localplayers[i]->ullDpad_this;
                            } else {
                                localplayers[i]->ullDpad_filtered =
                                    localplayers[i]->ullDpad_last;
                            }
                        }

                        
                        if (InputManager.ButtonPressed(
                                i, MINECRAFT_ACTION_LEFT_SCROLL) ||
                            InputManager.ButtonPressed(
                                i, MINECRAFT_ACTION_RIGHT_SCROLL))
                        
                        
                        
                        {
                            app.SetOpacityTimer(i);
                        }
                    } else {
                        
                        
                        
                        
                        bool tryJoin = !pause &&
                                       !ui.IsIgnorePlayerJoinMenuDisplayed(
                                           InputManager.GetPrimaryPad()) &&
                                       g_NetworkManager.SessionHasSpace() &&
                                       RenderManager.IsHiDef() &&
                                       InputManager.ButtonPressed(i);
                        if (tryJoin) {
                            if (!ui.PressStartPlaying(i)) {
                                ui.ShowPressStart(i);
                            } else {
                                
                                
                                
                                if (InputManager.ButtonPressed(
                                        i, MINECRAFT_ACTION_PAUSEMENU)) {
                                    

                                    
                                    if (ProfileManager.IsSignedIn(i)) {
                                        
                                        
                                        if (g_NetworkManager.IsLocalGame() ||
                                            (ProfileManager.IsSignedInLive(i) &&
                                             ProfileManager
                                                 .AllowedToPlayMultiplayer(
                                                     i))) {
                                            if (level->isClientSide) {
                                                bool success =
                                                    addLocalPlayer(i);

                                                if (!success) {
                                                    app.DebugPrintf(
                                                        "Bringing up the sign "
                                                        "in "
                                                        "ui\n");
                                                    ProfileManager.RequestSignInUI(
                                                        false,
                                                        g_NetworkManager
                                                            .IsLocalGame(),
                                                        true, false, true,
                                                        [this](bool b, int p) {
                                                            return InGame_SignInReturned(
                                                                this, b, p);
                                                        },
                                                        i);
                                                } else {
                                                }
                                            } else {
                                                
                                                std::shared_ptr<Player> player =
                                                    localplayers[i];
                                                if (player == nullptr) {
                                                    player =
                                                        createExtraLocalPlayer(
                                                            i,
                                                            (convStringToWstring(
                                                                 ProfileManager
                                                                     .GetGamertag(
                                                                         i)))
                                                                .c_str(),
                                                            i,
                                                            level->dimension
                                                                ->id);
                                                }
                                            }
                                        } else {
                                            if (ProfileManager.IsSignedInLive(
                                                    ProfileManager
                                                        .GetPrimaryPad()) &&
                                                !ProfileManager
                                                     .AllowedToPlayMultiplayer(
                                                         i)) {
                                                ProfileManager
                                                    .RequestConvertOfflineToGuestUI(
                                                        [this](bool b, int p) {
                                                            return InGame_SignInReturned(
                                                                this, b, p);
                                                        },
                                                        i);
                                                
                                                
                                                
                                                
                                                
                                                
                                                
                                                
                                                
                                                
                                                
                                                
                                                
                                                

                                                ui.HidePressStart();
                                                {
                                                    uint32_t uiIDA[1];
                                                    uiIDA[0] = IDS_CONFIRM_OK;
                                                    ui.RequestErrorMessage(
                                                        IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE,
                                                        IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT,
                                                        uiIDA, 1, i);
                                                }
                                            }
                                            
                                            {
                                                
                                                
                                                app.DebugPrintf(
                                                    "Bringing up the sign in "
                                                    "ui\n");
                                                ProfileManager.RequestSignInUI(
                                                    false,
                                                    g_NetworkManager
                                                        .IsLocalGame(),
                                                    true, false, true,
                                                    [this](bool b, int p) {
                                                        return InGame_SignInReturned(
                                                            this, b, p);
                                                    },
                                                    i);
                                            }
                                        }
                                    } else {
                                        
                                        app.DebugPrintf(
                                            "Bringing up the sign in ui\n");
                                        ProfileManager.RequestSignInUI(
                                            false,
                                            g_NetworkManager.IsLocalGame(),
                                            true, false, true,
                                            [this](bool b, int p) {
                                                return InGame_SignInReturned(
                                                    this, b, p);
                                            },
                                            i);
                                    }
                                }
                            }
                        }
                    }
                }

                if (pause && level != nullptr) {
                    float lastA = timer->a;
                    timer->advanceTime();
                    timer->a = lastA;
                } else {
                    timer->advanceTime();
                }

                
                for (int i = 0; i < timer->ticks; i++) {
                    bool bLastTimerTick = (i == (timer->ticks - 1));
                    
                    
                    
                    
                    if (i != 0) {
                        InputManager.Tick();
                        app.HandleButtonPresses();
                    }

                    ticks++;
                    
                    bool bFirst = true;
                    for (int idx = 0; idx < XUSER_MAX_COUNT; idx++) {
                        
                        
                        
                        
                        if (m_pendingLocalConnections[idx] != nullptr) {
                            m_pendingLocalConnections[idx]->tick();
                        }

                        
                        if (localplayers[idx] != nullptr) {
                            
                            if ((localplayers[idx]->ullButtonsPressed != 0) ||
                                InputManager.GetJoypadStick_LX(idx, false) !=
                                    0.0f ||
                                InputManager.GetJoypadStick_LY(idx, false) !=
                                    0.0f ||
                                InputManager.GetJoypadStick_RX(idx, false) !=
                                    0.0f ||
                                InputManager.GetJoypadStick_RY(idx, false) !=
                                    0.0f) {
                                localplayers[idx]->ResetInactiveTicks();
                            } else {
                                localplayers[idx]->IncrementInactiveTicks();
                            }

                            if (localplayers[idx]->GetInactiveTicks() > 200) {
                                if (!localplayers[idx]->isIdle() &&
                                    localplayers[idx]->onGround) {
                                    localplayers[idx]->setIsIdle(true);
                                }
                            } else {
                                if (localplayers[idx]->isIdle()) {
                                    localplayers[idx]->setIsIdle(false);
                                }
                            }
                        }

                        if (setLocalPlayerIdx(idx)) {
                            tick(bFirst, bLastTimerTick);
                            bFirst = false;
                            
                            
                            player->ullButtonsPressed = 0LL;
                        } else if (screen != nullptr) {
                            
                            
                            
                            if (!idx) {
                                screen->updateEvents();
                                DispatchJavaScreenKeyboard(screen);
                                if (screen != nullptr &&
                                    screen->particles != nullptr) {
                                    screen->particles->tick();
                                }
                                screen->tick();
                                Keyboard::update();
                            }
                        }
                    }

                    ui.HandleGameTick();

                    setLocalPlayerIdx(InputManager.GetPrimaryPad());

                    
                    

                    for (int l = 0; l < levels.size(); l++) {
                        if (levels[l]) {
                            levels[l]->animateTickDoWork();
                        }
                    }

                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                }
                
                checkGlError(L"Pre render");

                TileRenderer::fancy = options->fancyGraphics;

                

                soundEngine->tick((std::shared_ptr<Mob>*)localplayers,
                                  timer->a);

                
                glEnable(GL_TEXTURE_2D);

                
                

                
                
                
                if (player != nullptr && player->isInWall())
                    player->SetThirdPersonView(0);

                if (!noRender) {
                    bool bFirst = true;
                    int iPrimaryPad = InputManager.GetPrimaryPad();
                    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                        if (setLocalPlayerIdx(i)) {
                            RenderManager.StateSetViewport(
                                (C4JRender::eViewportType)
                                    player->m_iScreenSection);
                            gameRenderer->render(timer->a, bFirst);
                            bFirst = false;

                            if (i == iPrimaryPad) {
                                
                                
                                switch (app.GetXuiAction(i)) {
                                    case eAppAction_ExitWorldCapturedThumbnail:
                                    case eAppAction_SaveGameCapturedThumbnail:
                                    case eAppAction_AutosaveSaveGameCapturedThumbnail:
                                        
                                        app.CaptureSaveThumbnail();
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }

#if !defined(_ENABLEIGGY)
                    
                    
                    
                    if (bFirst) {
                        localPlayerIdx = 0;
                        RenderManager.StateSetViewport(
                            C4JRender::VIEWPORT_TYPE_FULLSCREEN);
                        gameRenderer->render(timer->a, true);
                    }
#endif

                    
                    
                    if (unoccupiedQuadrant > -1) {
                        
                        RenderManager.StateSetViewport((
                            C4JRender::
                                eViewportType)(C4JRender::
                                                   VIEWPORT_TYPE_QUADRANT_TOP_LEFT +
                                               unoccupiedQuadrant));
                        glClearColor(0, 0, 0, 0);
                        glClear(GL_COLOR_BUFFER_BIT);

                        ui.SetEmptyQuadrantLogo(
                            C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT +
                            unoccupiedQuadrant);
                    }
                    setLocalPlayerIdx(iPrimaryPad);
                    RenderManager.StateSetViewport(
                        C4JRender::VIEWPORT_TYPE_FULLSCREEN);
                }
                glFlush();

                










#if PACKET_ENABLE_STAT_TRACKING
                Packet::updatePacketStatsPIX();
#endif

                if (options->renderDebug) {
                    

#if DEBUG_RENDER_SHOWS_PACKETS
                    
                    

                    
                    
                    Packet::renderAllPacketStats();
#else
                    
                    g_NetworkManager.renderQueueMeter();
#endif
                } else {
                    lastTimer = System::nanoTime();
                }

                achievementPopup->render();

                std::this_thread::yield();  
                                            
                                            
                
                

                
                
                Display::update();

                

                













                checkGlError(L"Post render");
                frames++;
                
                
#if defined(ENABLE_JAVA_GUIS)
                pause = g_NetworkManager.IsLocalGame() &&
                        g_NetworkManager.GetPlayerCount() == 1 &&
                        screen != nullptr && screen->isPauseScreen();
#else
                pause = app.IsAppPaused();
#endif

#if !defined(_CONTENT_PACKAGE)
                while (System::nanoTime() >= lastTime + 1000000000) {
                    fpsString = toWString<int>(frames) + L" fps, " +
                                toWString<int>(Chunk::updates) +
                                L" chunk updates";
                    Chunk::updates = 0;
                    lastTime += 1000000000;
                    frames = 0;
                }
#endif
                










            }
            









        }
    }  
}

void Minecraft::run_end() { destroy(); }

void Minecraft::emergencySave() {
    
    levelRenderer->clear();
    setLevel(nullptr);
}

void Minecraft::renderFpsMeter(int64_t tickTime) {
    int nsPer60Fps = 1000000000l / 60;
    if (lastTimer == -1) {
        lastTimer = System::nanoTime();
    }
    int64_t now = System::nanoTime();
    Minecraft::tickTimes[(Minecraft::frameTimePos) &
                         (Minecraft::frameTimes_length - 1)] = tickTime;
    Minecraft::frameTimes[(Minecraft::frameTimePos++) &
                          (Minecraft::frameTimes_length - 1)] = now - lastTimer;
    lastTimer = now;

    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glEnable(GL_COLOR_MATERIAL);
    glLoadIdentity();
    glOrtho(0, (float)width, (float)height, 0, 1000, 3000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -2000);

    glLineWidth(1);
    glDisable(GL_TEXTURE_2D);
    Tesselator* t = Tesselator::getInstance();
    t->begin(GL_QUADS);
    int hh1 = (int)(nsPer60Fps / 200000);
    t->color(0x20000000);
    t->vertex((float)(0), (float)(height - hh1), (float)(0));
    t->vertex((float)(0), (float)(height), (float)(0));
    t->vertex((float)(Minecraft::frameTimes_length), (float)(height),
              (float)(0));
    t->vertex((float)(Minecraft::frameTimes_length), (float)(height - hh1),
              (float)(0));

    t->color(0x20200000);
    t->vertex((float)(0), (float)(height - hh1 * 2), (float)(0));
    t->vertex((float)(0), (float)(height - hh1), (float)(0));
    t->vertex((float)(Minecraft::frameTimes_length), (float)(height - hh1),
              (float)(0));
    t->vertex((float)(Minecraft::frameTimes_length), (float)(height - hh1 * 2),
              (float)(0));

    t->end();
    int64_t totalTime = 0;
    for (int i = 0; i < Minecraft::frameTimes_length; i++) {
        totalTime += Minecraft::frameTimes[i];
    }
    int hh = (int)(totalTime / 200000 / Minecraft::frameTimes_length);
    t->begin(GL_QUADS);
    t->color(0x20400000);
    t->vertex((float)(0), (float)(height - hh), (float)(0));
    t->vertex((float)(0), (float)(height), (float)(0));
    t->vertex((float)(Minecraft::frameTimes_length), (float)(height),
              (float)(0));
    t->vertex((float)(Minecraft::frameTimes_length), (float)(height - hh),
              (float)(0));
    t->end();
    t->begin(GL_LINES);
    for (int i = 0; i < Minecraft::frameTimes_length; i++) {
        int col = ((i - Minecraft::frameTimePos) &
                   (Minecraft::frameTimes_length - 1)) *
                  255 / Minecraft::frameTimes_length;
        int cc = col * col / 255;
        cc = cc * cc / 255;
        int cc2 = cc * cc / 255;
        cc2 = cc2 * cc2 / 255;
        if (Minecraft::frameTimes[i] > nsPer60Fps) {
            t->color(0xff000000 + cc * 65536);
        } else {
            t->color(0xff000000 + cc * 256);
        }

        int64_t time = Minecraft::frameTimes[i] / 200000;
        int64_t time2 = Minecraft::tickTimes[i] / 200000;

        t->vertex((float)(i + 0.5f), (float)(height - time + 0.5f), (float)(0));
        t->vertex((float)(i + 0.5f), (float)(height + 0.5f), (float)(0));

        
        t->color(0xff000000 + cc * 65536 + cc * 256 + cc * 1);
        
        
        
        t->vertex((float)(i + 0.5f), (float)(height - time + 0.5f), (float)(0));
        t->vertex((float)(i + 0.5f), (float)(height - (time - time2) + 0.5f),
                  (float)(0));
    }
    t->end();

    glEnable(GL_TEXTURE_2D);
}

void Minecraft::stop() {
    running = false;
    
}

void Minecraft::pauseGame() {
    if (screen != nullptr) {
        
        
        
        screen->keyPressed(0, Keyboard::KEY_ESCAPE);
        return;
    }
#if defined(ENABLE_JAVA_GUIS)
    setScreen(new PauseScreen());  
#endif
}

bool Minecraft::pollResize() {
    int fbw, fbh;
    RenderManager.GetFramebufferSize(fbw, fbh);
    if (fbw != width_phys || fbh != height_phys) {
        resize(fbw, fbh);
        return true;
    }
    return false;
}

void Minecraft::resize(int width, int height) {
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    
    
    this->width_phys = width;
    this->height_phys = height;
    if (RenderManager.IsWidescreen()) {
        this->width = width;
    } else {
        this->width = (width * 3) / 4;
    }
    this->height = height;

    if (screen != nullptr) {
        
        
        ScreenSizeCalculator ssc(options, this->width, height);
        int screenWidth = ssc.getWidth();
        int screenHeight = ssc.getHeight();
        screen->init(
            this, screenWidth,
            screenHeight);  
                            
    }
}

void Minecraft::verify() {
    















}

void Minecraft::levelTickUpdateFunc(void* pParam) {
    Level* pLevel = (Level*)pParam;
    pLevel->tick();
}

void Minecraft::levelTickThreadInitFunc() {
    Compression::UseDefaultThreadStorage();
}





void Minecraft::tick(bool bFirst, bool bUpdateTextures) {
    if (player == nullptr) return;
    int iPad = player->GetXboxPad();
    

    
    stats[iPad]->tick(iPad);

    
    
    app.TickOpacityTimer(iPad);

    
    if (bFirst) levelRenderer->destroyedTileManager->tick();

    gui->tick();
    gameRenderer->pick(1);

    

    if (!pause && level != nullptr) gameMode->tick();
    glBindTexture(GL_TEXTURE_2D,
                  textures->loadTexture(TN_TERRAIN));  
    if (bFirst) {
        if (!pause) textures->tick(bUpdateTextures);
    }

    






    if (screen == nullptr && player != nullptr) {
        if (player->getHealth() <= 0 && !ui.GetMenuDisplayed(iPad)) {
            setScreen(nullptr);
        } else if (player->isSleeping() && level != nullptr &&
                   level->isClientSide) {
            
            
        }
    } else if (screen != nullptr &&
               (dynamic_cast<InBedChatScreen*>(screen) != nullptr) &&
               !player->isSleeping()) {
        setScreen(nullptr);
    }

    if (screen != nullptr) {
        player->missTime = 10000;
        player->lastClickTick[0] = ticks + 10000;
        player->lastClickTick[1] = ticks + 10000;
    }

    if (screen != nullptr && bFirst) {
        
        
        screen->_performDeferredUIRebuild();
        
        screen->updateEvents();
        DispatchJavaScreenKeyboard(screen);

        if (screen != nullptr) {
            screen->particles->tick();
            screen->tick();
        }

        Keyboard::update();
    }

    if (screen == nullptr && !ui.GetMenuDisplayed(iPad)) {
        
        int iA = -1, iB = -1, iX, iY = IDS_CONTROLS_INVENTORY, iLT = -1,
            iRT = -1, iLB = -1, iRB = -1, iLS = -1, iRS = -1;

        if (player->abilities.instabuild) {
            iX = IDS_TOOLTIPS_CREATIVE;
        } else {
            iX = IDS_CONTROLS_CRAFTING;
        }
        
        
        int* piAction;
        int* piJump;
        int* piUse;
        int* piAlt;

        unsigned int uiAction = InputManager.GetGameJoypadMaps(
            InputManager.GetJoypadMapVal(iPad), MINECRAFT_ACTION_ACTION);
        unsigned int uiJump = InputManager.GetGameJoypadMaps(
            InputManager.GetJoypadMapVal(iPad), MINECRAFT_ACTION_JUMP);
        unsigned int uiUse = InputManager.GetGameJoypadMaps(
            InputManager.GetJoypadMapVal(iPad), MINECRAFT_ACTION_USE);
        unsigned int uiAlt = InputManager.GetGameJoypadMaps(
            InputManager.GetJoypadMapVal(iPad), MINECRAFT_ACTION_SNEAK_TOGGLE);

        
        switch (uiAction) {
            case _360_JOY_BUTTON_RT:
                piAction = &iRT;
                break;
            case _360_JOY_BUTTON_LT:
                piAction = &iLT;
                break;
            case _360_JOY_BUTTON_LB:
                piAction = &iLB;
                break;
            case _360_JOY_BUTTON_RB:
                piAction = &iRB;
                break;
            case _360_JOY_BUTTON_A:
            default:
                piAction = &iA;
                break;
        }

        switch (uiJump) {
            case _360_JOY_BUTTON_LT:
                piJump = &iLT;
                break;
            case _360_JOY_BUTTON_RT:
                piJump = &iRT;
                break;
            case _360_JOY_BUTTON_LB:
                piJump = &iLB;
                break;
            case _360_JOY_BUTTON_RB:
                piJump = &iRB;
                break;
            case _360_JOY_BUTTON_A:
            default:
                piJump = &iA;
                break;
        }

        switch (uiUse) {
            case _360_JOY_BUTTON_LB:
                piUse = &iLB;
                break;
            case _360_JOY_BUTTON_RB:
                piUse = &iRB;
                break;
            case _360_JOY_BUTTON_LT:
                piUse = &iLT;
                break;
            case _360_JOY_BUTTON_RT:
            default:
                piUse = &iRT;
                break;
        }

        switch (uiAlt) {
            default:
            case _360_JOY_BUTTON_LSTICK_RIGHT:
                piAlt = &iRS;
                break;

                
        }

        if (player->isUnderLiquid(Material::water)) {
            *piJump = IDS_TOOLTIPS_SWIMUP;
        } else {
            *piJump = -1;
        }

        *piUse = -1;
        *piAction = -1;
        *piAlt = -1;

        
        if (player->isSleeping() && (level != nullptr) && level->isClientSide) {
            *piUse = IDS_TOOLTIPS_WAKEUP;
        } else {
            if (player->isRiding()) {
                std::shared_ptr<Entity> mount = player->riding;

                if (mount->instanceof(eTYPE_MINECART) ||
                    mount->instanceof(eTYPE_BOAT)) {
                    *piAlt = IDS_TOOLTIPS_EXIT;
                } else {
                    *piAlt = IDS_TOOLTIPS_DISMOUNT;
                }
            }

            
            
            std::shared_ptr<ItemInstance> itemInstance =
                player->inventory->getSelected();

            
            
            if (itemInstance) {
                
                
                bool bUseItem =
                    gameMode->useItem(player, level, itemInstance, true);

                switch (itemInstance->getItem()->id) {
                        
                    case Item::potatoBaked_Id:
                    case Item::potato_Id:
                    case Item::pumpkinPie_Id:
                    case Item::potatoPoisonous_Id:
                    case Item::carrotGolden_Id:
                    case Item::carrots_Id:
                    case Item::mushroomStew_Id:
                    case Item::apple_Id:
                    case Item::bread_Id:
                    case Item::porkChop_raw_Id:
                    case Item::porkChop_cooked_Id:
                    case Item::apple_gold_Id:
                    case Item::fish_raw_Id:
                    case Item::fish_cooked_Id:
                    case Item::cookie_Id:
                    case Item::beef_cooked_Id:
                    case Item::beef_raw_Id:
                    case Item::chicken_cooked_Id:
                    case Item::chicken_raw_Id:
                    case Item::melon_Id:
                    case Item::rotten_flesh_Id:
                    case Item::spiderEye_Id:
                        
                        
                        {
                            FoodItem* food = (FoodItem*)itemInstance->getItem();
                            if (food != nullptr && food->canEat(player)) {
                                *piUse = IDS_TOOLTIPS_EAT;
                            }
                        }
                        break;

                    case Item::bucket_milk_Id:
                        *piUse = IDS_TOOLTIPS_DRINK;
                        break;

                    case Item::fishingRod_Id:  
                    case Item::emptyMap_Id:
                        *piUse = IDS_TOOLTIPS_USE;
                        break;

                    case Item::egg_Id:  
                    case Item::snowBall_Id:
                        *piUse = IDS_TOOLTIPS_THROW;
                        break;

                    case Item::bow_Id:  
                        if (player->abilities.instabuild ||
                            player->inventory->hasResource(Item::arrow_Id)) {
                            if (player->isUsingItem())
                                *piUse = IDS_TOOLTIPS_RELEASE_BOW;
                            else
                                *piUse = IDS_TOOLTIPS_DRAW_BOW;
                        }
                        break;

                    case Item::sword_wood_Id:
                    case Item::sword_stone_Id:
                    case Item::sword_iron_Id:
                    case Item::sword_diamond_Id:
                    case Item::sword_gold_Id:
                        *piUse = IDS_TOOLTIPS_BLOCK;
                        break;

                    case Item::bucket_empty_Id:
                    case Item::glassBottle_Id:
                        if (bUseItem) *piUse = IDS_TOOLTIPS_COLLECT;
                        break;

                    case Item::bucket_lava_Id:
                    case Item::bucket_water_Id:
                        *piUse = IDS_TOOLTIPS_EMPTY;
                        break;

                    case Item::boat_Id:
                    case Tile::waterLily_Id:
                        if (bUseItem) *piUse = IDS_TOOLTIPS_PLACE;
                        break;

                    case Item::potion_Id:
                        if (bUseItem) {
                            if (MACRO_POTION_IS_SPLASH(
                                    itemInstance->getAuxValue()))
                                *piUse = IDS_TOOLTIPS_THROW;
                            else
                                *piUse = IDS_TOOLTIPS_DRINK;
                        }
                        break;

                    case Item::enderPearl_Id:
                        if (bUseItem) *piUse = IDS_TOOLTIPS_THROW;
                        break;

                    case Item::eyeOfEnder_Id:
                        
                        
                        if (bUseItem && (level->dimension->id == 0) &&
                            level->getLevelData()->getHasStronghold()) {
                            *piUse = IDS_TOOLTIPS_THROW;
                        }
                        break;

                    case Item::expBottle_Id:
                        if (bUseItem) *piUse = IDS_TOOLTIPS_THROW;
                        break;
                }
            }

            if (hitResult != nullptr) {
                switch (hitResult->type) {
                    case HitResult::TILE: {
                        int x, y, z;
                        x = hitResult->x;
                        y = hitResult->y;
                        z = hitResult->z;
                        int face = hitResult->f;

                        int iTileID = level->getTile(x, y, z);
                        int iData = level->getData(x, y, z);

                        if (gameMode != nullptr &&
                            gameMode->getTutorial() != nullptr) {
                            
                            
                            
                            gameMode->getTutorial()->onLookAt(iTileID, iData);
                        }

                        
                        bool bUseItemOn = gameMode->useItemOn(
                            player, level, itemInstance, x, y, z, face,
                            &hitResult->pos, true);

                        





                        if (bUseItemOn && itemInstance != nullptr) {
                            switch (itemInstance->getItem()->id) {
                                case Tile::mushroom_brown_Id:
                                case Tile::mushroom_red_Id:
                                case Tile::tallgrass_Id:
                                case Tile::cactus_Id:
                                case Tile::sapling_Id:
                                case Tile::reeds_Id:
                                case Tile::flower_Id:
                                case Tile::rose_Id:
                                    *piUse = IDS_TOOLTIPS_PLANT;
                                    break;

                                    
                                case Item::hoe_wood_Id:
                                case Item::hoe_stone_Id:
                                case Item::hoe_iron_Id:
                                case Item::hoe_diamond_Id:
                                case Item::hoe_gold_Id:
                                    *piUse = IDS_TOOLTIPS_TILL;
                                    break;

                                case Item::seeds_wheat_Id:
                                case Item::netherwart_seeds_Id:
                                    *piUse = IDS_TOOLTIPS_PLANT;
                                    break;

                                case Item::dye_powder_Id:
                                    
                                    if (itemInstance->getAuxValue() ==
                                        DyePowderItem::WHITE) {
                                        switch (iTileID) {
                                            case Tile::sapling_Id:
                                            case Tile::wheat_Id:
                                            case Tile::grass_Id:
                                            case Tile::mushroom_brown_Id:
                                            case Tile::mushroom_red_Id:
                                            case Tile::melonStem_Id:
                                            case Tile::pumpkinStem_Id:
                                            case Tile::carrots_Id:
                                            case Tile::potatoes_Id:
                                                *piUse = IDS_TOOLTIPS_GROW;
                                                break;
                                        }
                                    }
                                    break;

                                case Item::painting_Id:
                                    *piUse = IDS_TOOLTIPS_HANG;
                                    break;

                                case Item::flintAndSteel_Id:
                                case Item::fireball_Id:
                                    *piUse = IDS_TOOLTIPS_IGNITE;
                                    break;

                                case Item::fireworks_Id:
                                    *piUse = IDS_TOOLTIPS_FIREWORK_LAUNCH;
                                    break;

                                case Item::lead_Id:
                                    *piUse = IDS_TOOLTIPS_ATTACH;
                                    break;

                                default:
                                    *piUse = IDS_TOOLTIPS_PLACE;
                                    break;
                            }
                        }

                        switch (iTileID) {
                            case Tile::anvil_Id:
                            case Tile::enchantTable_Id:
                            case Tile::brewingStand_Id:
                            case Tile::workBench_Id:
                            case Tile::furnace_Id:
                            case Tile::furnace_lit_Id:
                            case Tile::door_wood_Id:
                            case Tile::dispenser_Id:
                            case Tile::lever_Id:
                            case Tile::button_stone_Id:
                            case Tile::button_wood_Id:
                            case Tile::trapdoor_Id:
                            case Tile::fenceGate_Id:
                            case Tile::beacon_Id:
                                *piAction = IDS_TOOLTIPS_MINE;
                                *piUse = IDS_TOOLTIPS_USE;
                                break;

                            case Tile::chest_Id:
                                *piAction = IDS_TOOLTIPS_MINE;
                                *piUse = (Tile::chest->getContainer(
                                              level, x, y, z) != nullptr)
                                             ? IDS_TOOLTIPS_OPEN
                                             : -1;
                                break;

                            case Tile::enderChest_Id:
                            case Tile::chest_trap_Id:
                            case Tile::dropper_Id:
                            case Tile::hopper_Id:
                                *piUse = IDS_TOOLTIPS_OPEN;
                                *piAction = IDS_TOOLTIPS_MINE;
                                break;

                            case Tile::activatorRail_Id:
                            case Tile::goldenRail_Id:
                            case Tile::detectorRail_Id:
                            case Tile::rail_Id:
                                if (bUseItemOn) *piUse = IDS_TOOLTIPS_PLACE;
                                *piAction = IDS_TOOLTIPS_MINE;
                                break;

                            case Tile::bed_Id:
                                if (bUseItemOn) *piUse = IDS_TOOLTIPS_SLEEP;
                                *piAction = IDS_TOOLTIPS_MINE;
                                break;

                            case Tile::noteblock_Id:
                                
                                if (player->abilities.instabuild)
                                    *piAction = IDS_TOOLTIPS_MINE;
                                else
                                    *piAction = IDS_TOOLTIPS_PLAY;
                                *piUse = IDS_TOOLTIPS_CHANGEPITCH;
                                break;

                            case Tile::sign_Id:
                                *piAction = IDS_TOOLTIPS_MINE;
                                break;

                            case Tile::cauldron_Id:
                                
                                
                                if (itemInstance) {
                                    int iID = itemInstance->getItem()->id;
                                    int currentData = level->getData(x, y, z);
                                    if ((iID == Item::glassBottle_Id) &&
                                        (currentData > 0)) {
                                        *piUse = IDS_TOOLTIPS_COLLECT;
                                    }
                                }
                                *piAction = IDS_TOOLTIPS_MINE;
                                break;

                            case Tile::cake_Id:
                                if (player->abilities
                                        .instabuild)  
                                                      
                                {
                                    *piAction = IDS_TOOLTIPS_MINE;
                                } else {
                                    if (player->getFoodData()
                                            ->needsFood())  
                                                            
                                                            
                                    {
                                        *piAction = IDS_TOOLTIPS_EAT;
                                        *piUse = IDS_TOOLTIPS_EAT;
                                    } else {
                                        *piAction = IDS_TOOLTIPS_MINE;
                                    }
                                }
                                break;

                            case Tile::jukebox_Id:
                                if (!bUseItemOn && itemInstance != nullptr) {
                                    int iID = itemInstance->getItem()->id;
                                    if ((iID >= Item::record_01_Id) &&
                                        (iID <= Item::record_12_Id)) {
                                        *piUse = IDS_TOOLTIPS_PLAY;
                                    }
                                    *piAction = IDS_TOOLTIPS_MINE;
                                } else {
                                    if (Tile::jukebox->TestUse(
                                            level, x, y, z,
                                            player))  
                                    {
                                        *piUse = IDS_TOOLTIPS_EJECT;
                                    }
                                    *piAction = IDS_TOOLTIPS_MINE;
                                }
                                break;

                            case Tile::flowerPot_Id:
                                if (!bUseItemOn && (itemInstance != nullptr) &&
                                    (iData == 0)) {
                                    int iID = itemInstance->getItem()->id;
                                    if (iID < 256)  
                                    {
                                        switch (iID) {
                                            case Tile::flower_Id:
                                            case Tile::rose_Id:
                                            case Tile::sapling_Id:
                                            case Tile::mushroom_brown_Id:
                                            case Tile::mushroom_red_Id:
                                            case Tile::cactus_Id:
                                            case Tile::deadBush_Id:
                                                *piUse = IDS_TOOLTIPS_PLANT;
                                                break;

                                            case Tile::tallgrass_Id:
                                                if (itemInstance
                                                        ->getAuxValue() !=
                                                    TallGrass::TALL_GRASS)
                                                    *piUse = IDS_TOOLTIPS_PLANT;
                                                break;
                                        }
                                    }
                                }
                                *piAction = IDS_TOOLTIPS_MINE;
                                break;

                            case Tile::comparator_off_Id:
                            case Tile::comparator_on_Id:
                                *piUse = IDS_TOOLTIPS_USE;
                                *piAction = IDS_TOOLTIPS_MINE;
                                break;

                            case Tile::diode_off_Id:
                            case Tile::diode_on_Id:
                                *piUse = IDS_TOOLTIPS_USE;
                                *piAction = IDS_TOOLTIPS_MINE;
                                break;

                            case Tile::redStoneOre_Id:
                                if (bUseItemOn) *piUse = IDS_TOOLTIPS_USE;
                                *piAction = IDS_TOOLTIPS_MINE;
                                break;

                            case Tile::door_iron_Id:
                                if (*piUse == IDS_TOOLTIPS_PLACE) {
                                    *piUse = -1;
                                }
                                *piAction = IDS_TOOLTIPS_MINE;
                                break;

                            default:
                                *piAction = IDS_TOOLTIPS_MINE;
                                break;
                        }
                    } break;

                    case HitResult::ENTITY:
                        eINSTANCEOF entityType = hitResult->entity->GetType();

                        if ((gameMode != nullptr) &&
                            (gameMode->getTutorial() != nullptr)) {
                            
                            
                            
                            gameMode->getTutorial()->onLookAtEntity(
                                hitResult->entity);
                        }

                        std::shared_ptr<ItemInstance> heldItem = nullptr;
                        if (player->inventory->IsHeldItem()) {
                            heldItem = player->inventory->getSelected();
                        }
                        int heldItemId =
                            heldItem != nullptr ? heldItem->getItem()->id : -1;

                        switch (entityType) {
                            case eTYPE_CHICKEN: {
                                if (player->isAllowedToAttackAnimals())
                                    *piAction = IDS_TOOLTIPS_HIT;

                                std::shared_ptr<Animal> animal =
                                    std::dynamic_pointer_cast<Animal>(
                                        hitResult->entity);

                                if (animal->isLeashed() &&
                                    animal->getLeashHolder() == player) {
                                    *piUse = IDS_TOOLTIPS_UNLEASH;
                                    break;
                                }

                                switch (heldItemId) {
                                    case Item::nameTag_Id:
                                        *piUse = IDS_TOOLTIPS_NAME;
                                        break;

                                    case Item::lead_Id:
                                        if (!animal->isLeashed())
                                            *piUse = IDS_TOOLTIPS_LEASH;
                                        break;

                                    default: {
                                        if (!animal->isBaby() &&
                                            !animal->isInLove() &&
                                            (animal->getAge() == 0) &&
                                            animal->isFood(heldItem)) {
                                            *piUse = IDS_TOOLTIPS_LOVEMODE;
                                        }
                                    } break;

                                    case -1:
                                        break;  
                                }
                            } break;

                            case eTYPE_COW: {
                                if (player->isAllowedToAttackAnimals())
                                    *piAction = IDS_TOOLTIPS_HIT;

                                std::shared_ptr<Animal> animal =
                                    std::dynamic_pointer_cast<Animal>(
                                        hitResult->entity);

                                if (animal->isLeashed() &&
                                    animal->getLeashHolder() == player) {
                                    *piUse = IDS_TOOLTIPS_UNLEASH;
                                    break;
                                }

                                switch (heldItemId) {
                                        
                                    case Item::nameTag_Id:
                                        *piUse = IDS_TOOLTIPS_NAME;
                                        break;
                                    case Item::lead_Id:
                                        if (!animal->isLeashed())
                                            *piUse = IDS_TOOLTIPS_LEASH;
                                        break;
                                    case Item::bucket_empty_Id:
                                        *piUse = IDS_TOOLTIPS_MILK;
                                        break;
                                    default: {
                                        if (!animal->isBaby() &&
                                            !animal->isInLove() &&
                                            (animal->getAge() == 0) &&
                                            animal->isFood(heldItem)) {
                                            *piUse = IDS_TOOLTIPS_LOVEMODE;
                                        }
                                    } break;

                                    case -1:
                                        break;  
                                }
                            } break;
                            case eTYPE_MUSHROOMCOW: {
                                
                                
                                
                                if (player->isAllowedToAttackAnimals())
                                    *piAction = IDS_TOOLTIPS_HIT;

                                std::shared_ptr<Animal> animal =
                                    std::dynamic_pointer_cast<Animal>(
                                        hitResult->entity);

                                if (animal->isLeashed() &&
                                    animal->getLeashHolder() == player) {
                                    *piUse = IDS_TOOLTIPS_UNLEASH;
                                    break;
                                }

                                
                                switch (heldItemId) {
                                        
                                    case Item::nameTag_Id:
                                        *piUse = IDS_TOOLTIPS_NAME;
                                        break;

                                    case Item::lead_Id:
                                        if (!animal->isLeashed())
                                            *piUse = IDS_TOOLTIPS_LEASH;
                                        break;

                                    case Item::bowl_Id:
                                    case Item::
                                        bucket_empty_Id:  
                                                          
                                                          
                                                          
                                                          
                                        *piUse = IDS_TOOLTIPS_MILK;
                                        break;
                                    case Item::shears_Id: {
                                        if (player->isAllowedToAttackAnimals())
                                            *piAction = IDS_TOOLTIPS_HIT;
                                        if (!animal->isBaby())
                                            *piUse = IDS_TOOLTIPS_SHEAR;
                                    } break;
                                    default: {
                                        if (!animal->isBaby() &&
                                            !animal->isInLove() &&
                                            (animal->getAge() == 0) &&
                                            animal->isFood(heldItem)) {
                                            *piUse = IDS_TOOLTIPS_LOVEMODE;
                                        }
                                    } break;

                                    case -1:
                                        break;  
                                }
                            } break;

                            case eTYPE_BOAT:
                                *piAction = IDS_TOOLTIPS_MINE;
                                *piUse = IDS_TOOLTIPS_SAIL;
                                break;

                            case eTYPE_MINECART_RIDEABLE:
                                *piAction = IDS_TOOLTIPS_MINE;
                                *piUse =
                                    IDS_TOOLTIPS_RIDE;  
                                                        
                                                        
                                                        
                                break;

                            case eTYPE_MINECART_FURNACE:
                                *piAction = IDS_TOOLTIPS_MINE;

                                
                                
                                if (heldItemId == Item::coal_Id)
                                    *piUse = IDS_TOOLTIPS_USE;
                                break;

                            case eTYPE_MINECART_CHEST:
                            case eTYPE_MINECART_HOPPER:
                                *piAction = IDS_TOOLTIPS_MINE;
                                *piUse = IDS_TOOLTIPS_OPEN;
                                break;

                            case eTYPE_MINECART_SPAWNER:
                            case eTYPE_MINECART_TNT:
                                *piUse = IDS_TOOLTIPS_MINE;
                                break;

                            case eTYPE_SHEEP: {
                                
                                if (player->isAllowedToAttackAnimals())
                                    *piAction = IDS_TOOLTIPS_HIT;

                                std::shared_ptr<Sheep> sheep =
                                    std::dynamic_pointer_cast<Sheep>(
                                        hitResult->entity);

                                if (sheep->isLeashed() &&
                                    sheep->getLeashHolder() == player) {
                                    *piUse = IDS_TOOLTIPS_UNLEASH;
                                    break;
                                }

                                switch (heldItemId) {
                                    case Item::nameTag_Id:
                                        *piUse = IDS_TOOLTIPS_NAME;
                                        break;

                                    case Item::lead_Id:
                                        if (!sheep->isLeashed())
                                            *piUse = IDS_TOOLTIPS_LEASH;
                                        break;

                                    case Item::dye_powder_Id: {
                                        
                                        
                                        int newColor = ColoredTile::
                                            getTileDataForItemAuxValue(
                                                heldItem->getAuxValue());

                                        
                                        
                                        if (!(sheep->isSheared() &&
                                              sheep->getColor() != newColor)) {
                                            *piUse = IDS_TOOLTIPS_DYE;
                                        }
                                    } break;
                                    case Item::shears_Id: {
                                        
                                        
                                        if (!sheep->isBaby() &&
                                            !sheep->isSheared()) {
                                            *piUse = IDS_TOOLTIPS_SHEAR;
                                        }
                                    }

                                    break;
                                    default: {
                                        if (!sheep->isBaby() &&
                                            !sheep->isInLove() &&
                                            (sheep->getAge() == 0) &&
                                            sheep->isFood(heldItem)) {
                                            *piUse = IDS_TOOLTIPS_LOVEMODE;
                                        }
                                    } break;

                                    case -1:
                                        break;  
                                }
                            } break;

                            case eTYPE_PIG: {
                                
                                if (player->isAllowedToAttackAnimals())
                                    *piAction = IDS_TOOLTIPS_HIT;

                                std::shared_ptr<Pig> pig =
                                    std::dynamic_pointer_cast<Pig>(
                                        hitResult->entity);

                                if (pig->isLeashed() &&
                                    pig->getLeashHolder() == player) {
                                    *piUse = IDS_TOOLTIPS_UNLEASH;
                                } else if (heldItemId == Item::lead_Id) {
                                    if (!pig->isLeashed())
                                        *piUse = IDS_TOOLTIPS_LEASH;
                                } else if (heldItemId == Item::nameTag_Id) {
                                    *piUse = IDS_TOOLTIPS_NAME;
                                } else if (pig->hasSaddle())  
                                                              
                                {
                                    *piUse = IDS_TOOLTIPS_MOUNT;
                                } else if (!pig->isBaby()) {
                                    if (player->inventory->IsHeldItem()) {
                                        switch (heldItemId) {
                                            case Item::saddle_Id:
                                                *piUse = IDS_TOOLTIPS_SADDLE;
                                                break;

                                            default: {
                                                if (!pig->isInLove() &&
                                                    (pig->getAge() == 0) &&
                                                    pig->isFood(heldItem)) {
                                                    *piUse =
                                                        IDS_TOOLTIPS_LOVEMODE;
                                                }
                                            } break;
                                        }
                                    }
                                }
                            } break;

                            case eTYPE_WOLF:
                                
                                
                                {
                                    std::shared_ptr<Wolf> wolf =
                                        std::dynamic_pointer_cast<Wolf>(
                                            hitResult->entity);

                                    if (player->isAllowedToAttackAnimals())
                                        *piAction = IDS_TOOLTIPS_HIT;

                                    if (wolf->isLeashed() &&
                                        wolf->getLeashHolder() == player) {
                                        *piUse = IDS_TOOLTIPS_UNLEASH;
                                        break;
                                    }

                                    switch (heldItemId) {
                                        case Item::nameTag_Id:
                                            *piUse = IDS_TOOLTIPS_NAME;
                                            break;

                                        case Item::lead_Id:
                                            if (!wolf->isLeashed())
                                                *piUse = IDS_TOOLTIPS_LEASH;
                                            break;

                                        case Item::bone_Id:
                                            if (!wolf->isAngry() &&
                                                !wolf->isTame()) {
                                                *piUse = IDS_TOOLTIPS_TAME;
                                            } else if (
                                                equalsIgnoreCase(
                                                    player->getUUID(),
                                                    wolf->getOwnerUUID())) {
                                                if (wolf->isSitting()) {
                                                    *piUse =
                                                        IDS_TOOLTIPS_FOLLOWME;
                                                } else {
                                                    *piUse = IDS_TOOLTIPS_SIT;
                                                }
                                            }

                                            break;
                                        case Item::enderPearl_Id:
                                            
                                            
                                            break;
                                        case Item::dye_powder_Id:
                                            if (wolf->isTame()) {
                                                if (ColoredTile::
                                                        getTileDataForItemAuxValue(
                                                            heldItem
                                                                ->getAuxValue()) !=
                                                    wolf->getCollarColor()) {
                                                    *piUse =
                                                        IDS_TOOLTIPS_DYECOLLAR;
                                                } else if (wolf->isSitting()) {
                                                    *piUse =
                                                        IDS_TOOLTIPS_FOLLOWME;
                                                } else {
                                                    *piUse = IDS_TOOLTIPS_SIT;
                                                }
                                            }
                                            break;
                                        default:
                                            if (wolf->isTame()) {
                                                if (wolf->isFood(heldItem)) {
                                                    if (wolf->GetSynchedHealth() <
                                                        wolf->getMaxHealth()) {
                                                        *piUse =
                                                            IDS_TOOLTIPS_HEAL;
                                                    } else {
                                                        if (!wolf->isBaby() &&
                                                            !wolf->isInLove() &&
                                                            (wolf->getAge() ==
                                                             0)) {
                                                            *piUse =
                                                                IDS_TOOLTIPS_LOVEMODE;
                                                        }
                                                    }
                                                    
                                                    break;
                                                }

                                                if (equalsIgnoreCase(
                                                        player->getUUID(),
                                                        wolf->getOwnerUUID())) {
                                                    if (wolf->isSitting()) {
                                                        *piUse =
                                                            IDS_TOOLTIPS_FOLLOWME;
                                                    } else {
                                                        *piUse =
                                                            IDS_TOOLTIPS_SIT;
                                                    }
                                                }
                                            }
                                            break;
                                    }
                                }
                                break;
                            case eTYPE_OCELOT: {
                                std::shared_ptr<Ocelot> ocelot =
                                    std::dynamic_pointer_cast<Ocelot>(
                                        hitResult->entity);

                                if (player->isAllowedToAttackAnimals())
                                    *piAction = IDS_TOOLTIPS_HIT;

                                if (ocelot->isLeashed() &&
                                    ocelot->getLeashHolder() == player) {
                                    *piUse = IDS_TOOLTIPS_UNLEASH;
                                } else if (heldItemId == Item::lead_Id) {
                                    if (!ocelot->isLeashed())
                                        *piUse = IDS_TOOLTIPS_LEASH;
                                } else if (heldItemId == Item::nameTag_Id) {
                                    *piUse = IDS_TOOLTIPS_NAME;
                                } else if (ocelot->isTame()) {
                                    
                                    
                                    
                                    if (ocelot->isFood(heldItem)) {
                                        if (!ocelot->isBaby()) {
                                            if (!ocelot->isInLove()) {
                                                if (ocelot->getAge() == 0) {
                                                    *piUse =
                                                        IDS_TOOLTIPS_LOVEMODE;
                                                }
                                            } else {
                                                *piUse = IDS_TOOLTIPS_FEED;
                                            }
                                        }

                                    } else if (equalsIgnoreCase(
                                                   player->getUUID(),
                                                   ocelot->getOwnerUUID()) &&
                                               !ocelot->isSittingOnTile()) {
                                        if (ocelot->isSitting()) {
                                            *piUse = IDS_TOOLTIPS_FOLLOWME;
                                        } else {
                                            *piUse = IDS_TOOLTIPS_SIT;
                                        }
                                    }
                                } else if (heldItemId >= 0) {
                                    if (ocelot->isFood(heldItem))
                                        *piUse = IDS_TOOLTIPS_TAME;
                                }
                            } break;

                            case eTYPE_PLAYER: {
                                
                                
                                
                                std::shared_ptr<Player> TargetPlayer =
                                    std::dynamic_pointer_cast<Player>(
                                        hitResult->entity);

                                if (!TargetPlayer
                                         ->hasInvisiblePrivilege())  
                                                                     
                                                                     
                                                                     
                                                                     
                                                                     
                                                                     
                                                                     
                                                                     
                                {
                                    if (app.GetGameHostOption(
                                            eGameHostOption_PvP) &&
                                        player->isAllowedToAttackPlayers()) {
                                        *piAction = IDS_TOOLTIPS_HIT;
                                    }
                                }
                            } break;

                            case eTYPE_ITEM_FRAME: {
                                std::shared_ptr<ItemFrame> itemFrame =
                                    std::dynamic_pointer_cast<ItemFrame>(
                                        hitResult->entity);

                                
                                if (itemFrame->getItem() != nullptr) {
                                    
                                    *piUse = IDS_TOOLTIPS_ROTATE;
                                } else {
                                    
                                    if (heldItemId >= 0)
                                        *piUse = IDS_TOOLTIPS_PLACE;
                                }

                                *piAction = IDS_TOOLTIPS_HIT;
                            } break;

                            case eTYPE_VILLAGER: {
                                

                                std::shared_ptr<Villager> villager =
                                    std::dynamic_pointer_cast<Villager>(
                                        hitResult->entity);
                                if (!villager->isBaby()) {
                                    *piUse = IDS_TOOLTIPS_TRADE;
                                }
                                *piAction = IDS_TOOLTIPS_HIT;
                            } break;

                            case eTYPE_ZOMBIE: {
                                std::shared_ptr<Zombie> zomb =
                                    std::dynamic_pointer_cast<Zombie>(
                                        hitResult->entity);
                                static GoldenAppleItem* goldapple =
                                    (GoldenAppleItem*)Item::apple_gold;

                                
                                
                                if (zomb->isVillager() && zomb->isWeakened() &&
                                    (heldItemId == Item::apple_gold_Id) &&
                                    !goldapple->isFoil(heldItem)) {
                                    *piUse = IDS_TOOLTIPS_CURE;
                                }
                                *piAction = IDS_TOOLTIPS_HIT;
                            } break;

                            case eTYPE_HORSE: {
                                std::shared_ptr<EntityHorse> horse =
                                    std::dynamic_pointer_cast<EntityHorse>(
                                        hitResult->entity);

                                bool heldItemIsFood = false,
                                     heldItemIsLove = false,
                                     heldItemIsArmour = false;

                                switch (heldItemId) {
                                    case Item::wheat_Id:
                                    case Item::sugar_Id:
                                    case Item::bread_Id:
                                    case Tile::hayBlock_Id:
                                    case Item::apple_Id:
                                        heldItemIsFood = true;
                                        break;
                                    case Item::carrotGolden_Id:
                                    case Item::apple_gold_Id:
                                        heldItemIsLove = true;
                                        heldItemIsFood = true;
                                        break;
                                    case Item::horseArmorDiamond_Id:
                                    case Item::horseArmorGold_Id:
                                    case Item::horseArmorMetal_Id:
                                        heldItemIsArmour = true;
                                        break;
                                }

                                if (horse->isLeashed() &&
                                    horse->getLeashHolder() == player) {
                                    *piUse = IDS_TOOLTIPS_UNLEASH;
                                } else if (heldItemId == Item::lead_Id) {
                                    if (!horse->isLeashed())
                                        *piUse = IDS_TOOLTIPS_LEASH;
                                } else if (heldItemId == Item::nameTag_Id) {
                                    *piUse = IDS_TOOLTIPS_NAME;
                                } else if (horse->isBaby())  
                                                             
                                                             
                                {
                                    if (heldItemIsFood) {
                                        
                                        *piUse = IDS_TOOLTIPS_FEED;
                                    }
                                } else if (!horse->isTamed()) {
                                    if (heldItemId == -1) {
                                        
                                        
                                        
                                        *piUse = IDS_TOOLTIPS_TAME;
                                    } else if (heldItemIsFood) {
                                        
                                        
                                        *piUse = IDS_TOOLTIPS_FEED;
                                    }
                                } else if (player->isSneaking() ||
                                           (heldItemId == Item::saddle_Id) ||
                                           (horse->canWearArmor() &&
                                            heldItemIsArmour)) {
                                    
                                    if (*piUse == -1)
                                        *piUse = IDS_TOOLTIPS_OPEN;
                                } else if (horse->canWearBags() &&
                                           !horse->isChestedHorse() &&
                                           (heldItemId == Tile::chest_Id)) {
                                    
                                    
                                    *piUse = IDS_TOOLTIPS_ATTACH;
                                } else if (horse->isReadyForParenting() &&
                                           heldItemIsLove) {
                                    
                                    *piUse = IDS_TOOLTIPS_LOVEMODE;
                                } else if (heldItemIsFood &&
                                           (horse->getHealth() <
                                            horse->getMaxHealth())) {
                                    
                                    
                                    *piUse = IDS_TOOLTIPS_HEAL;
                                } else {
                                    
                                    *piUse = IDS_TOOLTIPS_MOUNT;
                                }

                                if (player->isAllowedToAttackAnimals())
                                    *piAction = IDS_TOOLTIPS_HIT;
                            } break;

                            case eTYPE_ENDERDRAGON:
                                
                                *piAction = IDS_TOOLTIPS_HIT;
                                break;

                            case eTYPE_LEASHFENCEKNOT:
                                *piAction = IDS_TOOLTIPS_UNLEASH;
                                if (heldItemId == Item::lead_Id &&
                                    LeashItem::bindPlayerMobsTest(
                                        player, level, player->x, player->y,
                                        player->z)) {
                                    *piUse = IDS_TOOLTIPS_ATTACH;
                                } else {
                                    *piUse = IDS_TOOLTIPS_UNLEASH;
                                }
                                break;

                            default:
                                if (hitResult->entity->instanceof(eTYPE_MOB)) {
                                    std::shared_ptr<Mob> mob =
                                        std::dynamic_pointer_cast<Mob>(
                                            hitResult->entity);
                                    if (mob->isLeashed() &&
                                        mob->getLeashHolder() == player) {
                                        *piUse = IDS_TOOLTIPS_UNLEASH;
                                    } else if (heldItemId == Item::lead_Id) {
                                        if (!mob->isLeashed())
                                            *piUse = IDS_TOOLTIPS_LEASH;
                                    } else if (heldItemId == Item::nameTag_Id) {
                                        *piUse = IDS_TOOLTIPS_NAME;
                                    }
                                }
                                *piAction = IDS_TOOLTIPS_HIT;
                                break;
                        }
                        break;
                }
            }
        }

        
        
        if (!ui.IsReloadingSkin())
            ui.SetTooltips(iPad, iA, iB, iX, iY, iLT, iRT, iLB, iRB, iLS, iRS);

        int wheel = 0;
        unsigned int leftTicks =
            InputManager.GetValue(iPad, MINECRAFT_ACTION_LEFT_SCROLL, true);
        unsigned int rightTicks =
            InputManager.GetValue(iPad, MINECRAFT_ACTION_RIGHT_SCROLL, true);
        if (leftTicks > 0 &&
            gameMode->isInputAllowed(MINECRAFT_ACTION_LEFT_SCROLL)) {
            wheel = (int)leftTicks;  
        } else if (rightTicks > 0 &&
                   gameMode->isInputAllowed(MINECRAFT_ACTION_RIGHT_SCROLL)) {
            wheel = -(int)rightTicks;  
        }
        if (wheel != 0) {
            player->inventory->swapPaint(wheel);

            if (gameMode != nullptr && gameMode->getTutorial() != nullptr) {
                
                
                gameMode->getTutorial()->onSelectedItemChanged(
                    player->inventory->getSelected());
            }

            
            player->updateRichPresence();

            if (options->isFlying) {
                if (wheel > 0) wheel = 1;
                if (wheel < 0) wheel = -1;

                options->flySpeed += wheel * .25f;
            }
        }

        if (gameMode->isInputAllowed(MINECRAFT_ACTION_ACTION)) {
            if ((player->ullButtonsPressed & (1LL << MINECRAFT_ACTION_ACTION)))
            
            {
                
                player->handleMouseClick(0);
                player->lastClickTick[0] = ticks;
            }

            if (InputManager.ButtonDown(iPad, MINECRAFT_ACTION_ACTION) &&
                ticks - player->lastClickTick[0] >= timer->ticksPerSecond / 4) {
                
                player->handleMouseClick(0);
                player->lastClickTick[0] = ticks;
            }

            if (InputManager.ButtonDown(iPad, MINECRAFT_ACTION_ACTION)) {
                player->handleMouseDown(0, true);
            } else {
                player->handleMouseDown(0, false);
            }
        }

        
        
        
        
        







        if (player->isUsingItem()) {
            if (!InputManager.ButtonDown(iPad, MINECRAFT_ACTION_USE))
                gameMode->releaseUsingItem(player);
        } else if (gameMode->isInputAllowed(MINECRAFT_ACTION_USE)) {
            if (player->abilities.instabuild) {
                
                
                bool didClick = player->creativeModeHandleMouseClick(
                    1, InputManager.ButtonDown(iPad, MINECRAFT_ACTION_USE));
                
                
                
                if (player->lastClickState ==
                    LocalPlayer::lastClick_oldRepeat) {
                    
                    
                    
                    if (didClick) {
                        player->lastClickTick[1] = ticks;
                    } else {
                        
                        
                        if (InputManager.ButtonDown(iPad,
                                                    MINECRAFT_ACTION_USE) &&
                            ticks - player->lastClickTick[1] >=
                                timer->ticksPerSecond / 4) {
                            player->handleMouseClick(1);
                            player->lastClickTick[1] = ticks;
                        }
                    }
                }
            } else {
                
                
                
                
                
                
                
                bool firstClick = (player->lastClickTick[1] == 0);
                bool autoRepeat = ticks - player->lastClickTick[1] >=
                                  timer->ticksPerSecond / 4;
                if (player->isRiding() || player->isSprinting() ||
                    player->isSleeping())
                    autoRepeat = false;
                if (InputManager.ButtonDown(iPad, MINECRAFT_ACTION_USE)) {
                    
                    
                    if (player->isSleeping())
                        player->lastClickTick[1] =
                            ticks + (timer->ticksPerSecond * 2);
                    if (firstClick || autoRepeat) {
                        bool wasSleeping = player->isSleeping();

                        player->handleMouseClick(1);

                        
                        
                        if (wasSleeping)
                            player->lastClickTick[1] =
                                ticks + (timer->ticksPerSecond * 2);
                        else
                            player->lastClickTick[1] = ticks;
                    }
                } else {
                    player->lastClickTick[1] = 0;
                }
            }
        }

        if (app.DebugSettingsOn()) {
            if (player->ullButtonsPressed &
                (1LL << MINECRAFT_ACTION_CHANGE_SKIN)) {
                player->ChangePlayerSkin();
            }
        }

        if (player->missTime > 0) player->missTime--;

#if defined(_DEBUG_MENUS_ENABLED)
        if (app.DebugSettingsOn()) {
            
            if (iPad == InputManager.GetPrimaryPad()) {
                if ((player->ullButtonsPressed &
                     (1LL << MINECRAFT_ACTION_RENDER_DEBUG))) {
#if !defined(_CONTENT_PACKAGE)

                    options->renderDebug = !options->renderDebug;
                    
                    
                    ui.NavigateToScene(0, eUIScene_DebugOverlay, nullptr,
                                       eUILayer_Debug);
#endif
                }

                if ((player->ullButtonsPressed &
                     (1LL << MINECRAFT_ACTION_SPAWN_CREEPER)) &&
                    app.GetMobsDontAttackEnabled()) {
                    
                    
                    
                    
                    
                    std::shared_ptr<Mob> mob = std::dynamic_pointer_cast<Mob>(
                        std::make_shared<Spider>(level));
                    mob->moveTo(player->x + 1, player->y, player->z + 1,
                                level->random->nextFloat() * 360, 0);
                    level->addEntity(mob);
                }
            }

            if ((player->ullButtonsPressed &
                 (1LL << MINECRAFT_ACTION_FLY_TOGGLE))) {
                player->abilities.debugflying = !player->abilities.debugflying;
                player->abilities.flying = !player->abilities.flying;
            }
        }
#endif

        if ((player->ullButtonsPressed &
             (1LL << MINECRAFT_ACTION_RENDER_THIRD_PERSON)) &&
            gameMode->isInputAllowed(MINECRAFT_ACTION_RENDER_THIRD_PERSON)) {
            
            player->SetThirdPersonView((player->ThirdPersonView() + 1) % 3);
            
        }

        if ((player->ullButtonsPressed & (1LL << MINECRAFT_ACTION_GAME_INFO)) &&
            gameMode->isInputAllowed(MINECRAFT_ACTION_GAME_INFO)) {
            

            options->renderDebug = !options->renderDebug;
        }

        if ((player->ullButtonsPressed & (1LL << MINECRAFT_ACTION_INVENTORY)) &&
            gameMode->isInputAllowed(MINECRAFT_ACTION_INVENTORY)) {
            std::shared_ptr<MultiplayerLocalPlayer> player =
                Minecraft::GetInstance()->player;
            ui.PlayUISFX(eSFX_Press);
#if defined(ENABLE_JAVA_GUIS)
            setScreen(new InventoryScreen(player));
#else
            app.LoadInventoryMenu(iPad, player);
#endif
        }

        if (Keyboard::isKeyPressed(options->keyChat->key) && screen == nullptr) {
            setScreen(new ChatScreen());
        }

        if (bFirst) {
            Keyboard::update();
        }

        if ((player->ullButtonsPressed & (1LL << MINECRAFT_ACTION_CRAFTING)) &&
            gameMode->isInputAllowed(MINECRAFT_ACTION_CRAFTING)) {
            std::shared_ptr<MultiplayerLocalPlayer> player =
                Minecraft::GetInstance()->player;

            
            
            
            
            if (gameMode->hasInfiniteItems()) {
                

                ui.PlayUISFX(eSFX_Press);
#if defined(ENABLE_JAVA_GUIS)
                setScreen(new CreativeInventoryScreen(player));
            }
#else
                app.LoadCreativeMenu(iPad, player);
            }
            
            
            else if ((hitResult != nullptr) &&
                     (hitResult->type == HitResult::TILE) &&
                     (level->getTile(hitResult->x, hitResult->y,
                                     hitResult->z) == Tile::workBench_Id)) {
                
                
                
                bool usedItem = false;
                gameMode->useItemOn(player, level, nullptr, hitResult->x,
                                    hitResult->y, hitResult->z, 0,
                                    &hitResult->pos, false, &usedItem);
            } else {
                ui.PlayUISFX(eSFX_Press);
                app.LoadCrafting2x2Menu(iPad, player);
            }
#endif
        }

        if ((player->ullButtonsPressed & (1LL << MINECRAFT_ACTION_PAUSEMENU))) {
            app.DebugPrintf(
                "PAUSE PRESS PROCESSING - ipad = %d, NavigateToScene\n",
                player->GetXboxPad());
            ui.PlayUISFX(eSFX_Press);
#if !defined(ENABLE_JAVA_GUIS)
            ui.NavigateToScene(iPad, eUIScene_PauseMenu, nullptr,
                               eUILayer_Scene);
#endif
        }

        if ((player->ullButtonsPressed & (1LL << MINECRAFT_ACTION_DROP)) &&
            gameMode->isInputAllowed(MINECRAFT_ACTION_DROP)) {
            player->drop();
        }

        uint64_t ullButtonsPressed = player->ullButtonsPressed;

        bool selected = false;
        {
            int hotbarSlot = InputManager.GetHotbarSlotPressed(iPad);
            if (hotbarSlot >= 0 && hotbarSlot <= 9) {
                player->inventory->selected = hotbarSlot;
                selected = true;
            }
        }
        if (selected || wheel != 0 ||
            (player->ullButtonsPressed & (1LL << MINECRAFT_ACTION_DROP))) {
            std::wstring itemName = L"";
            std::shared_ptr<ItemInstance> selectedItem =
                player->getSelectedItem();
            
            
            int iCount = 0;

            if (selectedItem != nullptr) iCount = selectedItem->GetCount();
            if (selectedItem != nullptr && !((player->ullButtonsPressed &
                                              (1LL << MINECRAFT_ACTION_DROP)) &&
                                             selectedItem->GetCount() == 1)) {
                itemName = selectedItem->getHoverName();
            }
            if (!(player->ullButtonsPressed & (1LL << MINECRAFT_ACTION_DROP)) ||
                (selectedItem != nullptr && selectedItem->GetCount() <= 1))
                ui.SetSelectedItem(iPad, itemName);
        }
    } else {
        
        
        
        
        
    }

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    if (level != nullptr) {
        if (player != nullptr) {
            recheckPlayerIn++;
            if (recheckPlayerIn == 30) {
                recheckPlayerIn = 0;
                level->ensureAdded(player);
            }
        }
        
        
        
        
        if (!level->isClientSide) {
            
            
            level->difficulty = options->difficulty;
        }

        if (!pause) gameRenderer->tick(bFirst);

        
        
        
        
        
        
        static unsigned int levelsTickedFlags;
        if (bFirst) {
            levelsTickedFlags = 0;

#if !defined(DISABLE_LEVELTICK_THREAD)
            levelTickEventQueue->waitForFinish();

#endif
            SparseLightStorage::tick();     
            CompressedTileStorage::tick();  
            SparseDataStorage::tick();      
        }

        for (unsigned int i = 0; i < levels.size(); ++i) {
            if (player->level != levels[i])
                continue;  
                           

            
            
            
            
            if (!pause && levels[i] != nullptr)
                levels[i]->animateTick(std::floor(player->x),
                                       std::floor(player->y),
                                       std::floor(player->z));

            if (levelsTickedFlags & (1 << i))
                continue;  
                           
            levelsTickedFlags |= (1 << i);

            if (!pause) levelRenderer->tick();

            
            
            
            
            
            if (levels[i] != nullptr) {
                if (!pause) {
                    if (levels[i]->skyFlashTime > 0) levels[i]->skyFlashTime--;
                    levels[i]->tickEntities();
                }

                
                

                
                if (!pause)  
                {
                    
                    
                    levels[i]->setSpawnSettings(level->difficulty > 0, true);
#if defined(DISABLE_LEVELTICK_THREAD)
                    levels[i]->tick();
#else
                    levelTickEventQueue->sendEvent(levels[i]);
#endif
                }
            }
        }

        if (bFirst) {
            if (!pause) particleEngine->tick();
        }

        
        
        if (pause) tickAllConnections();
        
    }

    
    
    
    
    
    
}

void Minecraft::reloadSound() {
    
    soundEngine = new SoundEngine();
    soundEngine->init(options);
    bgLoader->forceReload();
}

bool Minecraft::isClientSide() {
    return level != nullptr && level->isClientSide;
}

void Minecraft::selectLevel(ConsoleSaveFile* saveFile,
                            const std::wstring& levelId,
                            const std::wstring& levelName,
                            LevelSettings* levelSettings) {}

bool Minecraft::saveSlot(int slot, const std::wstring& name) { return false; }

bool Minecraft::loadSlot(const std::wstring& userName, int slot) {
    return false;
}

void Minecraft::releaseLevel(int message) {
    
    setLevel(nullptr, message);
}



void Minecraft::forceStatsSave(int idx) {
    
    stats[idx]->save(idx, true);

    
    if (ProfileManager.IsSignedInLive(idx)) {
        int tempLockedProfile = ProfileManager.GetLockedProfile();
        ProfileManager.SetLockedProfile(idx);
        stats[idx]->saveLeaderboards();
        ProfileManager.SetLockedProfile(tempLockedProfile);
    }
}


MultiPlayerLevel* Minecraft::getLevel(int dimension) {
    if (dimension == -1)
        return levels[1];
    else if (dimension == 1)
        return levels[2];
    else
        return levels[0];
}
















void Minecraft::forceaddLevel(MultiPlayerLevel* level) {
    int dimId = level->dimension->id;
    if (dimId == -1)
        levels[1] = level;
    else if (dimId == 1)
        levels[2] = level;
    else
        levels[0] = level;
}

void Minecraft::setLevel(MultiPlayerLevel* level, int message ,
                         std::shared_ptr<Player> forceInsertPlayer ,
                         bool doForceStatsSave ,
                         bool bPrimaryPlayerSignedOut ) {
    std::lock_guard<std::recursive_mutex> lock(m_setLevelCS);
    bool playerAdded = false;
    int iPrimaryPlayer = InputManager.GetPrimaryPad();

    if (progressRenderer != nullptr) {
        this->progressRenderer->progressStart(message);
        this->progressRenderer->progressStage(-1);
    }

    
    

    
    
    
    gameRenderer->DisableUpdateThread();
    
    
    RenderManager.InitialiseContext();

    for (unsigned int i = 0; i < levels.size(); ++i) {
        
        
        
        if (levels[i] != nullptr && level == nullptr) {
            
            
            if ((doForceStatsSave == true) && player != nullptr)
                forceStatsSave(player->GetXboxPad());

            
            
            
            
            if (levelRenderer != nullptr) {
                for (unsigned int p = 0; p < XUSER_MAX_COUNT; ++p) {
                    levelRenderer->setLevel(p, nullptr);
                }
            }
            if (particleEngine != nullptr) particleEngine->setLevel(nullptr);
        }
    }
    
    
    if (level == nullptr) {
        cameraTargetPlayer = nullptr;
        if (EntityRenderDispatcher::instance != nullptr)
            EntityRenderDispatcher::instance->cameraEntity = nullptr;
        if (TileEntityRenderDispatcher::instance != nullptr)
            TileEntityRenderDispatcher::instance->cameraEntity = nullptr;
        
        
        
        
        this->level = nullptr;
        
        
        
        
        
        
        
        
        gameMode = nullptr;
        player = nullptr;

        if (levels[0] != nullptr) {
            delete levels[0];
            levels[0] = nullptr;

            
            if (levels[1] != nullptr) levels[1]->savedDataStorage = nullptr;
        }
        if (levels[1] != nullptr) {
            delete levels[1];
            levels[1] = nullptr;
        }
        if (levels[2] != nullptr) {
            delete levels[2];
            levels[2] = nullptr;
        }

        
        for (unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
            std::shared_ptr<MultiplayerLocalPlayer> mplp = localplayers[idx];
            if (mplp != nullptr && mplp->connection != nullptr) {
                delete mplp->connection;
                mplp->connection = nullptr;
            }

            if (localgameModes[idx] != nullptr) {
                delete localgameModes[idx];
                localgameModes[idx] = nullptr;
            }

            if (m_pendingLocalConnections[idx] != nullptr) {
                delete m_pendingLocalConnections[idx];
                m_pendingLocalConnections[idx] = nullptr;
            }

            localplayers[idx] = nullptr;
        }
        
        
        
    }
    this->level = level;

    if (level != nullptr) {
        int dimId = level->dimension->id;
        if (dimId == -1)
            levels[1] = level;
        else if (dimId == 1)
            levels[2] = level;
        else
            levels[0] = level;

        
        
        if (player == nullptr) {
            
            if (forceInsertPlayer != nullptr) {
                player = std::dynamic_pointer_cast<MultiplayerLocalPlayer>(forceInsertPlayer);
            } else {
                player = gameMode->createPlayer(level);
            }

            PlayerUID playerXUIDOffline = INVALID_XUID;
            PlayerUID playerXUIDOnline = INVALID_XUID;
            ProfileManager.GetXUID(iPrimaryPlayer, &playerXUIDOffline, false);
            ProfileManager.GetXUID(iPrimaryPlayer, &playerXUIDOnline, true);
            player->setXuid(playerXUIDOffline);
            player->setOnlineXuid(playerXUIDOnline);

            player->m_displayName =
                ProfileManager.GetDisplayName(iPrimaryPlayer);

            player->resetPos();
            gameMode->initPlayer(player);

            player->SetXboxPad(iPrimaryPlayer);

            for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                m_pendingLocalConnections[i] = nullptr;
                if (i != iPrimaryPlayer) localgameModes[i] = nullptr;
                else localgameModes[i] = gameMode;
            }
            
            
            localplayers[iPrimaryPlayer] = std::dynamic_pointer_cast<MultiplayerLocalPlayer>(player);
        }

        if (player != nullptr) {
            player->resetPos();
            
            if (level != nullptr) {
                level->addEntity(player);
                playerAdded = true;
            }
        }

        if (player->input != nullptr) delete player->input;
        player->input = new Input();

        this->cameraTargetPlayer = player;

        if (levelRenderer != nullptr)
            levelRenderer->setLevel(player->GetXboxPad(), level);
        if (particleEngine != nullptr) particleEngine->setLevel(level);

        gameMode->adjustPlayer(player);

        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            m_pendingLocalConnections[i] = nullptr;
        }
        updatePlayerViewportAssignments();

        setLocalPlayerIdx(iPrimaryPlayer);
        
        this->cameraTargetPlayer = player;

        
        
        gameRenderer->EnableUpdateThread();
    } else {
        levelSource->clearAll();
        player = nullptr;

        
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            if (m_pendingLocalConnections[i] != nullptr)
                m_pendingLocalConnections[i]->close();
            m_pendingLocalConnections[i] = nullptr;
            localplayers[i] = nullptr;
            localgameModes[i] = nullptr;
        }
    }

    
    
    
}

void Minecraft::prepareLevel(int title) {
    if (progressRenderer != nullptr) {
        this->progressRenderer->progressStart(title);
        this->progressRenderer->progressStage(IDS_PROGRESS_BUILDING_TERRAIN);
    }
    int r = 128;
    if (gameMode->isCutScene()) r = 64;
    int pp = 0;
    int max = r * 2 / 16 + 1;
    max = max * max;
    ChunkSource* cs = level->getChunkSource();

    Pos* spawnPos = level->getSharedSpawnPos();
    if (player != nullptr) {
        spawnPos->x = (int)player->x;
        spawnPos->z = (int)player->z;
    }

    for (int x = -r; x <= r; x += 16) {
        for (int z = -r; z <= r; z += 16) {
            if (progressRenderer != nullptr)
                this->progressRenderer->progressStagePercentage((pp++) * 100 /
                                                                max);
            level->getTile(spawnPos->x + x, 64, spawnPos->z + z);
            
            
            
        }
    }
    delete spawnPos;
    if (!gameMode->isCutScene()) {
        if (progressRenderer != nullptr)
            this->progressRenderer->progressStage(
                IDS_PROGRESS_SIMULATING_WORLD);
        max = 2000;
    }
}

void Minecraft::fileDownloaded(const std::wstring& name, File* file) {
    int p = (int)name.find(L"/");
    std::wstring category = name.substr(0, p);
    std::wstring name2 = name.substr(p + 1);
    toLower(category);
    if (category == L"sound") {
        soundEngine->add(name, file);
    } else if (category == L"newsound") {
        soundEngine->add(name, file);
    } else if (category == L"streaming") {
        soundEngine->addStreaming(name, file);
    } else if (category == L"music") {
        soundEngine->addMusic(name, file);
    } else if (category == L"newmusic") {
        soundEngine->addMusic(name, file);
    }
}

std::wstring Minecraft::gatherStats1() {
    
    return L"Time to autosave: " +
           toWString<int64_t>(app.SecondsToAutosave()) + L"s";
}

std::wstring Minecraft::gatherStats2() {
    return g_NetworkManager.GatherStats();
    
}

std::wstring Minecraft::gatherStats3() {
    return g_NetworkManager.GatherRTTStats();
    
    
}

std::wstring Minecraft::gatherStats4() {
    return level->gatherChunkSourceStats();
}

void Minecraft::respawnPlayer(int iPad, int dimension, int newEntityId) {
    gameRenderer
        ->DisableUpdateThread();  
                                  
    std::shared_ptr<MultiplayerLocalPlayer> localPlayer = localplayers[iPad];

    level->validateSpawn();
    level->removeAllPendingEntityRemovals();

    if (localPlayer != nullptr) {
        level->removeEntity(localPlayer);
    }

    std::shared_ptr<Player> oldPlayer = localPlayer;
    cameraTargetPlayer = nullptr;

    
    int iTempPad = localPlayer->GetXboxPad();
    int iTempScreenSection = localPlayer->m_iScreenSection;
    EDefaultSkins skin = localPlayer->getPlayerDefaultSkin();
    player = localgameModes[iPad]->createPlayer(level);

    PlayerUID playerXUIDOffline = INVALID_XUID;
    PlayerUID playerXUIDOnline = INVALID_XUID;
    ProfileManager.GetXUID(iTempPad, &playerXUIDOffline, false);
    ProfileManager.GetXUID(iTempPad, &playerXUIDOnline, true);
    player->setXuid(playerXUIDOffline);
    player->setOnlineXuid(playerXUIDOnline);
    player->setIsGuest(ProfileManager.IsGuest(iTempPad));

    player->m_displayName = ProfileManager.GetDisplayName(iPad);

    player->SetXboxPad(iTempPad);

    player->m_iScreenSection = iTempScreenSection;
    player->setPlayerIndex(localPlayer->getPlayerIndex());
    player->setCustomSkin(localPlayer->getCustomSkin());
    player->setPlayerDefaultSkin(skin);
    player->setCustomCape(localPlayer->getCustomCape());
    player->m_sessionTimeStart = localPlayer->m_sessionTimeStart;
    player->m_dimensionTimeStart = localPlayer->m_dimensionTimeStart;
    player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All,
                                   localPlayer->getAllPlayerGamePrivileges());

    player->SetThirdPersonView(oldPlayer->ThirdPersonView());

    
    
    
    
    if (localPlayer->getHealth() > 0 && localPlayer->y > -64) {
        player->inventory->selected = localPlayer->inventory->selected;
    }

    
    std::uint32_t dwSkinID = app.getSkinIdFromPath(player->customTextureUrl);
    if (GET_IS_DLC_SKIN_FROM_BITMASK(dwSkinID)) {
        player->setAnimOverrideBitmask(
            player->getSkinAnimOverrideBitmask(dwSkinID));
    }

    player->dimension = dimension;
    cameraTargetPlayer = player;

    
    if (iPad == InputManager.GetPrimaryPad()) {
        createPrimaryLocalPlayer(iPad);

        
        app.SetGameSettingsDebugMask(InputManager.GetPrimaryPad(),
                                     app.GetGameSettingsDebugMask(-1, true));
    } else {
        storeExtraLocalPlayer(iPad);
    }

    player->setShowOnMaps(
        app.GetGameHostOption(eGameHostOption_Gamertags) != 0 ? true : false);

    player->resetPos();
    level->addEntity(player);
    gameMode->initPlayer(player);

    if (player->input != nullptr) delete player->input;
    player->input = new Input();
    player->entityId = newEntityId;
    player->animateRespawn();
    gameMode->adjustPlayer(player);

    
    if (!level->isClientSide) {
        prepareLevel(IDS_PROGRESS_RESPAWNING);
    }

    
    
    
    player->SetPlayerRespawned(true);

    if (dynamic_cast<DeathScreen*>(screen) != nullptr) setScreen(nullptr);

    gameRenderer->EnableUpdateThread();
}

void Minecraft::start(const std::wstring& name, const std::wstring& sid) {
    startAndConnectTo(name, sid, L"");
}

void Minecraft::startAndConnectTo(const std::wstring& name,
                                  const std::wstring& sid,
                                  const std::wstring& url) {
    bool fullScreen = false;
    std::wstring userName = name;

    















    Minecraft* minecraft;
    

    minecraft = new Minecraft(nullptr, nullptr, nullptr, 1280, 720, fullScreen);

    









    



    minecraft->serverDomain = L"www.minecraft.net";

    {
        if (userName != L"" &&
            sid != L"")  
                         
        {
            minecraft->user = new User(userName, sid);
        } else {
            minecraft->user = new User(
                L"Player" + toWString<int>(System::currentTimeMillis() % 1000),
                L"");
        }
    }
    
    
    
    

    







    













    
    minecraft->run();
}

ClientConnection* Minecraft::getConnection(int iPad) {
    return localplayers[iPad]->connection;
}


Minecraft* Minecraft::GetInstance() { return m_instance; }

bool useLomp = false;

int g_iMainThreadId;

void Minecraft::main() {
    std::wstring name;
    std::wstring sessionId;

    

    useLomp = true;

    Minecraft_RunStaticCtors();
    EntityRenderDispatcher::staticCtor();
    TileEntityRenderDispatcher::staticCtor();
    User::staticCtor();
    Tutorial::staticCtor();
    ColourTable::staticCtor();
    app.loadDefaultGameRules();

#if defined(_LARGE_WORLDS)
    LevelRenderer::staticCtor();
#endif

    

    
    {
        name =
            L"Player" + toWString<int64_t>(System::currentTimeMillis() % 1000);
        sessionId = L"-";
        




    }

    
    IUIScene_CreativeMenu::staticCtor();

    
    
    
    Minecraft::start(name, sessionId);
}




void Minecraft::javaTabAdd(const std::string& uuid, const std::wstring& name,
                           int ping) {
    if (uuid.empty()) return;
    if (m_javaTabList.find(uuid) == m_javaTabList.end())
        m_javaTabOrder.push_back(uuid);
    JavaTabEntry& e = m_javaTabList[uuid];
    e.name = name;
    e.ping = ping;
}

void Minecraft::javaTabRemove(const std::string& uuid) {
    if (m_javaTabList.erase(uuid) == 0) return;
    for (auto it = m_javaTabOrder.begin(); it != m_javaTabOrder.end(); ++it) {
        if (*it == uuid) {
            m_javaTabOrder.erase(it);
            break;
        }
    }
}

void Minecraft::javaTabClear() {
    m_javaTabList.clear();
    m_javaTabOrder.clear();
}

bool Minecraft::renderNames() {
    if (m_instance == nullptr || !m_instance->options->hideGui) {
        return true;
    }
    return false;
}

bool Minecraft::useFancyGraphics() {
    return (m_instance != nullptr && m_instance->options->fancyGraphics);
}

bool Minecraft::useAmbientOcclusion() {
    return (m_instance != nullptr &&
            m_instance->options->ambientOcclusion != Options::AO_OFF);
}

bool Minecraft::renderDebug() {
    return (m_instance != nullptr && m_instance->options->renderDebug);
}

bool Minecraft::handleClientSideCommand(const std::wstring& chatMessage) {
    if (chatMessage.empty() || chatMessage[0] != L'/') return false;

    
    std::wstring rest = chatMessage.substr(1);
    std::wstring cmd;
    for (wchar_t c : rest) {
        if (c == L' ') break;
        cmd.push_back((c >= L'A' && c <= L'Z') ? (wchar_t)(c + 32) : c);
    }

    if (cmd == L"help" || cmd == L"?") {
        int iPad = (player != nullptr) ? player->GetXboxPad()
                                       : InputManager.GetPrimaryPad();
        auto say = [this, iPad](const std::wstring& s) {
            if (gui != nullptr) gui->addMessage(s, iPad);
        };
        say(L"\u00A7e--- Commands ---");
        say(L"\u00A7e/weather <clear|rain|thunder> \u00A77- set weather");
        say(L"\u00A7e/time set <0-24000> | day | night \u00A77- set time");
        say(L"\u00A7e/gamemode <0|1|2> [player] \u00A77- change mode");
        say(L"\u00A7e/give <player> <item> [count] \u00A77- give items");
        say(L"\u00A7e/tp <player> | <x> <y> <z> \u00A77- teleport");
        say(L"\u00A7e/kill [target] \u00A77- kill entities");
        say(L"\u00A7e/seed \u00A77- show world seed");
        say(L"\u00A7e/list \u00A77- list online players");
        say(L"\u00A7e/spawn /home /sethome /back \u00A77- movement");
        say(L"\u00A7e/heal /feed \u00A77- restore health / hunger");
        say(L"\u00A7e/op /deop /kick /ban /pardon \u00A77- admin");
        return true;  
    }

    
    return false;
}

int Minecraft::maxSupportedTextureSize() {
    
    return 1024;

    
    
    
    
    
    
    
    
    
}

void Minecraft::delayTextureReload() { reloadTextures = true; }

int64_t Minecraft::currentTimeMillis() {
    return System::currentTimeMillis();  
                                         
}








































































































































Screen* Minecraft::getScreen() { return screen; }

bool Minecraft::isTutorial() {
    return m_inFullTutorialBits > 0;

    







}

void Minecraft::playerStartedTutorial(int iPad) {
    
    
    if (app.GetTutorialMode())
        m_inFullTutorialBits = m_inFullTutorialBits | (1 << iPad);
}

void Minecraft::playerLeftTutorial(int iPad) {
    
    
    if (m_inFullTutorialBits == 0) {
        app.SetTutorialMode(false);
        return;
    }

    m_inFullTutorialBits = m_inFullTutorialBits & ~(1 << iPad);
    if (m_inFullTutorialBits == 0) {
        app.SetTutorialMode(false);
    }
}

int Minecraft::InGame_SignInReturned(void* pParam, bool bContinue, int iPad) {
    Minecraft* pMinecraftClass = (Minecraft*)pParam;

    if (g_NetworkManager.IsInSession()) {
        
        
        
        
        
        app.DebugPrintf("Disabling Guest Signin\n");
        XEnableGuestSignin(false);
    }

    
    
    if (bContinue == true && g_NetworkManager.IsInSession() &&
        pMinecraftClass->localplayers[iPad] == nullptr) {
        
        
        if (ProfileManager.IsSignedIn(iPad)) {
            if (!g_NetworkManager.SessionHasSpace()) {
                unsigned int uiIDA[1];
                uiIDA[0] = IDS_OK;
                ui.RequestErrorMessage(IDS_MULTIPLAYER_FULL_TITLE,
                                       IDS_MULTIPLAYER_FULL_TEXT, uiIDA, 1);
            }
            
            else if (g_NetworkManager.IsLocalGame() ||
                     (ProfileManager.IsSignedInLive(iPad) &&
                      ProfileManager.AllowedToPlayMultiplayer(iPad))) {
                if (pMinecraftClass->level->isClientSide) {
                    pMinecraftClass->addLocalPlayer(iPad);
                } else {
                    
                    std::shared_ptr<Player> player =
                        pMinecraftClass->localplayers[iPad];
                    if (player == nullptr) {
                        player = pMinecraftClass->createExtraLocalPlayer(
                            iPad,
                            (convStringToWstring(
                                 ProfileManager.GetGamertag(iPad)))
                                .c_str(),
                            iPad, pMinecraftClass->level->dimension->id);
                    }
                }
            } else if (ProfileManager.IsSignedInLive(
                           InputManager.GetPrimaryPad()) &&
                       !ProfileManager.AllowedToPlayMultiplayer(iPad)) {
                
                
                
                
                
                
                
                unsigned int uiIDA[1];
                uiIDA[0] = IDS_CONFIRM_OK;
                ui.RequestErrorMessage(IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE,
                                       IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT,
                                       uiIDA, 1, iPad);
            }
        }
    }
    return 0;
}

void Minecraft::tickAllConnections() {
    int oldIdx = getLocalPlayerIdx();
    for (unsigned int i = 0; i < XUSER_MAX_COUNT; i++) {
        std::shared_ptr<MultiplayerLocalPlayer> mplp = localplayers[i];
        if (mplp && mplp->connection) {
            setLocalPlayerIdx(i);
            mplp->connection->tick();
        }
    }
    setLocalPlayerIdx(oldIdx);
}

bool Minecraft::addPendingClientTextureRequest(
    const std::wstring& textureName) {
    auto it = find(m_pendingTextureRequests.begin(),
                   m_pendingTextureRequests.end(), textureName);
    if (it == m_pendingTextureRequests.end()) {
        m_pendingTextureRequests.push_back(textureName);
        return true;
    }
    return false;
}

void Minecraft::handleClientTextureReceived(const std::wstring& textureName) {
    auto it = find(m_pendingTextureRequests.begin(),
                   m_pendingTextureRequests.end(), textureName);
    if (it != m_pendingTextureRequests.end()) {
        m_pendingTextureRequests.erase(it);
    }
}

unsigned int Minecraft::getCurrentTexturePackId() {
    return skins->getSelected()->getId();
}

ColourTable* Minecraft::getColourTable() {
    TexturePack* selected = skins->getSelected();

    ColourTable* colours = selected->getColourTable();

    if (colours == nullptr) {
        colours = skins->getDefault()->getColourTable();
    }

    return colours;
}

#pragma clang diagnostic pop
