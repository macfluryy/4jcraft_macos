#include "Options.h"

#include <algorithm>
#include <vector>

#include "KeyMapping.h"
#include "platform/sdl2/Input.h"
#include "platform/sdl2/Profile.h"
#include "app/common/src/Audio/SoundEngine.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/Socket.h"
#include "app/mac/MacGame.h"
#include "app/include/stubs.h"
#include "util/StringHelpers.h"
#include "java/File.h"
#include "java/InputOutputStream/BufferedReader.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "java/InputOutputStream/FileInputStream.h"
#include "java/InputOutputStream/FileOutputStream.h"
#include "java/InputOutputStream/InputStreamReader.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/multiplayer/ClientConnection.h"
#include "minecraft/client/renderer/LevelRenderer.h"
#include "minecraft/client/renderer/Textures.h"
#include "minecraft/locale/I18n.h"
#include "minecraft/locale/Language.h"
#include "minecraft/network/packet/ClientInformationPacket.h"
#include "minecraft/world/level/ViewDistanceUtil.h"



const Options::Option Options::Option::options[17] = {
    Options::Option(L"options.music", true, false),
    Options::Option(L"options.sound", true, false),
    Options::Option(L"options.invertMouse", false, true),
    Options::Option(L"options.sensitivity", true, false),
    Options::Option(L"options.renderDistance", false, false),
    Options::Option(L"options.viewBobbing", false, true),
    Options::Option(L"options.anaglyph", false, true),
    Options::Option(L"options.advancedOpengl", false, true),
    Options::Option(L"options.framerateLimit", false, false),
    Options::Option(L"options.difficulty", false, false),
    Options::Option(L"options.graphics", false, false),
    Options::Option(L"options.ao", false, true),
    Options::Option(L"options.guiScale", false, false),
    Options::Option(L"options.fov", true, false),
    Options::Option(L"options.gamma", true, false),
    Options::Option(L"options.renderClouds", false, true),
    Options::Option(L"options.particles", false, false),
};

const Options::Option* Options::Option::MUSIC = &Options::Option::options[0];
const Options::Option* Options::Option::SOUND = &Options::Option::options[1];
const Options::Option* Options::Option::INVERT_MOUSE =
    &Options::Option::options[2];
const Options::Option* Options::Option::SENSITIVITY =
    &Options::Option::options[3];
const Options::Option* Options::Option::RENDER_DISTANCE =
    &Options::Option::options[4];
const Options::Option* Options::Option::VIEW_BOBBING =
    &Options::Option::options[5];
const Options::Option* Options::Option::ANAGLYPH = &Options::Option::options[6];
const Options::Option* Options::Option::ADVANCED_OPENGL =
    &Options::Option::options[7];
const Options::Option* Options::Option::FRAMERATE_LIMIT =
    &Options::Option::options[8];
const Options::Option* Options::Option::DIFFICULTY =
    &Options::Option::options[9];
const Options::Option* Options::Option::GRAPHICS =
    &Options::Option::options[10];
const Options::Option* Options::Option::AMBIENT_OCCLUSION =
    &Options::Option::options[11];
const Options::Option* Options::Option::GUI_SCALE =
    &Options::Option::options[12];
const Options::Option* Options::Option::FOV = &Options::Option::options[13];
const Options::Option* Options::Option::GAMMA = &Options::Option::options[14];
const Options::Option* Options::Option::RENDER_CLOUDS =
    &Options::Option::options[15];
const Options::Option* Options::Option::PARTICLES =
    &Options::Option::options[16];

const Options::Option* Options::Option::getItem(int id) { return &options[id]; }

Options::Option::Option(const std::wstring& captionId, bool hasProgress,
                        bool isBoolean)
    : _isProgress(hasProgress), _isBoolean(isBoolean), captionId(captionId) {}

bool Options::Option::isProgress() const { return _isProgress; }

bool Options::Option::isBoolean() const { return _isBoolean; }

int Options::Option::getId() const { return (int)(this - options); }

std::wstring Options::Option::getCaptionId() const { return captionId; }

const std::wstring Options::RENDER_DISTANCE_NAMES[] = {
    L"options.renderDistance.far", L"options.renderDistance.normal",
    L"options.renderDistance.short", L"options.renderDistance.tiny"};
const std::wstring Options::DIFFICULTY_NAMES[] = {
    L"options.difficulty.peaceful", L"options.difficulty.easy",
    L"options.difficulty.normal", L"options.difficulty.hard"};
const std::wstring Options::GUI_SCALE[] = {
    L"options.guiScale.auto", L"options.guiScale.small",
    L"options.guiScale.normal", L"options.guiScale.large"};

#ifdef ENABLE_VSYNC
const std::wstring Options::FRAMERATE_LIMITS[] = {
    L"performance.max", L"performance.balanced", L"performance.powersaver"};
#else
const std::wstring Options::FRAMERATE_LIMITS[] = {
    L"performance.max", L"performance.balanced", L"performance.powersaver",
    L"performance.unlimited"};
#endif

const std::wstring Options::PARTICLES[] = {L"options.particles.all",
                                           L"options.particles.decreased",
                                           L"options.particles.minimal"};


void Options::init() {
    music = 1;
    sound = 1;
    sensitivity = 0.5f;
    invertYMouse = false;
    viewDistance = 0;
    bobView = true;
    anaglyph3d = false;
    advancedOpengl = false;


#if defined(ENABLE_VSYNC)
    framerateLimit = 2;
#else
    framerateLimit = 3;
#endif
    fancyGraphics = true;
    ambientOcclusion = true;
    renderClouds = true;
    skin = L"Default";

    keyUp = new KeyMapping(L"key.forward", Keyboard::KEY_W);
    keyLeft = new KeyMapping(L"key.left", Keyboard::KEY_A);
    keyDown = new KeyMapping(L"key.back", Keyboard::KEY_S);
    keyRight = new KeyMapping(L"key.right", Keyboard::KEY_D);
    keyJump = new KeyMapping(L"key.jump", Keyboard::KEY_SPACE);
    keyBuild = new KeyMapping(L"key.inventory", Keyboard::KEY_E);
    keyDrop = new KeyMapping(L"key.drop", Keyboard::KEY_Q);
    keyChat = new KeyMapping(L"key.chat", Keyboard::KEY_T);
    keySneak = new KeyMapping(L"key.sneak", Keyboard::KEY_LSHIFT);
    keyAttack = new KeyMapping(L"key.attack", -100 + 0);
    keyUse = new KeyMapping(L"key.use", -100 + 1);
    keyPlayerList = new KeyMapping(L"key.playerlist", Keyboard::KEY_TAB);
    keyPickItem = new KeyMapping(L"key.pickItem", -100 + 2);
    keyToggleFog = new KeyMapping(L"key.fog", Keyboard::KEY_F);

    keyMappings[0] = keyAttack;
    keyMappings[1] = keyUse;
    keyMappings[2] = keyUp;
    keyMappings[3] = keyLeft;
    keyMappings[4] = keyDown;
    keyMappings[5] = keyRight;
    keyMappings[6] = keyJump;
    keyMappings[7] = keySneak;
    keyMappings[8] = keyDrop;
    keyMappings[9] = keyBuild;
    keyMappings[10] = keyChat;
    keyMappings[11] = keyPlayerList;
    keyMappings[12] = keyPickItem;
    keyMappings[13] = keyToggleFog;

    minecraft = nullptr;
    

    difficulty = 2;
    hideGui = false;
    thirdPersonView = false;
    renderDebug = false;
    lastMpIp = L"";
    lastMpNickname = L"";

    isFlying = false;
    smoothCamera = false;
    fixedCamera = false;
    flySpeed = 1;
    cameraSpeed = 1;
    guiScale = 3;
    particles = 0;
    fov = 0;
    gamma = 0;
}

Options::Options(Minecraft* minecraft, File workingDirectory) {
    init();
    this->minecraft = minecraft;
    optionsFile = File(workingDirectory, L"options.txt");
    load();
}

Options::Options() { init(); }

std::wstring Options::getKeyDescription(int i) {
    Language* language = Language::getInstance();
    return language->getElement(keyMappings[i]->name);
}

std::wstring Options::getKeyMessage(int i) {
    int key = keyMappings[i]->key;
    if (key < 0) {
        return I18n::get(L"key.mouseButton", key + 101);
    } else {
        return Keyboard::getKeyName(keyMappings[i]->key);
    }
}

void Options::setKey(int i, int key) {
    keyMappings[i]->key = key;
    save();
}

void Options::set(const Options::Option* item, float fVal) {
    if (item == Option::MUSIC) {
        music = fVal;
        minecraft->soundEngine->updateMusicVolume(fVal);
    }
    if (item == Option::SOUND) {
        sound = fVal;
        minecraft->soundEngine->updateSoundEffectVolume(fVal);
    }
    if (item == Option::SENSITIVITY) {
        sensitivity = fVal;
    }
    if (item == Option::FOV) {
        fov = fVal;
    }
    if (item == Option::GAMMA) {
        gamma = fVal;
    }
}

void Options::toggle(const Options::Option* option, int dir) {
    if (option == Option::INVERT_MOUSE) invertYMouse = !invertYMouse;
    if (option == Option::RENDER_DISTANCE) {
        viewDistance = (viewDistance + dir) & 3;
        if (minecraft != nullptr && g_NetworkManager.IsInSession() &&
            !g_NetworkManager.IsHost()) {
            int primaryPad = InputManager.GetPrimaryPad();
            ClientConnection* conn = nullptr;
            if (minecraft->localplayers[primaryPad] != nullptr) {
                conn = minecraft->getConnection(primaryPad);
            }
            if (conn != nullptr && conn->getSocket() != nullptr &&
                !conn->getSocket()->isLocal()) {
                conn->send(std::make_shared<ClientInformationPacket>(
                    viewDistanceOptionToChunks(viewDistance)));
            }
        }
    }
    if (option == Option::GUI_SCALE) guiScale = (guiScale + dir) & 3;
    if (option == Option::PARTICLES) particles = (particles + dir) % 3;

    
    
    if (option == Option::VIEW_BOBBING) bobView = !bobView;
    if (option == Option::RENDER_CLOUDS) renderClouds = !renderClouds;
    if (option == Option::ADVANCED_OPENGL) {
        advancedOpengl = !advancedOpengl;
        
        if (minecraft->level) minecraft->levelRenderer->allChanged();
    }
    if (option == Option::ANAGLYPH) {
        anaglyph3d = !anaglyph3d;
        minecraft->textures->reloadAll();
    }
    if (option == Option::FRAMERATE_LIMIT)
#ifdef ENABLE_VSYNC
        framerateLimit = (framerateLimit + dir + 3) % 3;
#else
        framerateLimit = (framerateLimit + dir + 4) % 4;
#endif

    
    
    
    
    
    
    
    
    
    
    
    
    if (option == Option::DIFFICULTY)
        difficulty = (difficulty + dir) & 3;

    app.DebugPrintf("Option::DIFFICULTY = %d", difficulty);

    if (option == Option::GRAPHICS) {
        fancyGraphics = !fancyGraphics;
        
        if (minecraft->level) minecraft->levelRenderer->allChanged();
    }
    if (option == Option::AMBIENT_OCCLUSION) {
        ambientOcclusion = !ambientOcclusion;
        
        if (minecraft->level) minecraft->levelRenderer->allChanged();
    }

    
    
}

float Options::getProgressValue(const Options::Option* item) {
    if (item == Option::FOV) return fov;
    if (item == Option::GAMMA) return gamma;
    if (item == Option::MUSIC) return music;
    if (item == Option::SOUND) return sound;
    if (item == Option::SENSITIVITY) return sensitivity;
    return 0;
}

bool Options::getBooleanValue(const Options::Option* item) {
    
    
    if (item == Option::INVERT_MOUSE) return invertYMouse;
    if (item == Option::VIEW_BOBBING) return bobView;
    if (item == Option::ANAGLYPH) return anaglyph3d;
    if (item == Option::ADVANCED_OPENGL) return advancedOpengl;
    if (item == Option::AMBIENT_OCCLUSION) return ambientOcclusion;
    if (item == Option::RENDER_CLOUDS) return renderClouds;
    return false;
}

std::wstring Options::getMessage(const Options::Option* item) {
    

    Language* language = Language::getInstance();
    std::wstring caption = language->getElement(item->getCaptionId()) + L": ";

    if (item->isProgress()) {
        float progressValue = getProgressValue(item);

        if (item == Option::SENSITIVITY) {
            if (progressValue == 0) {
                return caption +
                       language->getElement(L"options.sensitivity.min");
            }
            if (progressValue == 1) {
                return caption +
                       language->getElement(L"options.sensitivity.max");
            }
            return caption + toWString<int>((int)(progressValue * 200)) + L"%";
        } else if (item == Option::FOV) {
            if (progressValue == 0) {
                return caption + language->getElement(L"options.fov.min");
            }
            if (progressValue == 1) {
                return caption + language->getElement(L"options.fov.max");
            }
            return caption + toWString<int>((int)(70 + progressValue * 40));
        } else if (item == Option::GAMMA) {
            if (progressValue == 0) {
                return caption + language->getElement(L"options.gamma.min");
            }
            if (progressValue == 1) {
                return caption + language->getElement(L"options.gamma.max");
            }
            return caption + L"+" + toWString<int>((int)(progressValue * 100)) +
                   L"%";
        } else {
            if (progressValue == 0) {
                return caption + language->getElement(L"options.off");
            }
            return caption + toWString<int>((int)(progressValue * 100)) + L"%";
        }
    } else if (item->isBoolean()) {
        bool booleanValue = getBooleanValue(item);
        if (booleanValue) {
            return caption + language->getElement(L"options.on");
        }
        return caption + language->getElement(L"options.off");
    } else if (item == Option::RENDER_DISTANCE) {
        return caption +
               language->getElement(RENDER_DISTANCE_NAMES[viewDistance]);
    } else if (item == Option::DIFFICULTY) {
        return caption + language->getElement(DIFFICULTY_NAMES[difficulty]);
    } else if (item == Option::GUI_SCALE) {
        return caption + language->getElement(GUI_SCALE[guiScale]);
    } else if (item == Option::PARTICLES) {
        return caption + language->getElement(PARTICLES[particles]);
    } else if (item == Option::FRAMERATE_LIMIT) {
        return caption + I18n::get(FRAMERATE_LIMITS[framerateLimit]);
    } else if (item == Option::GRAPHICS) {
        if (fancyGraphics) {
            return caption + language->getElement(L"options.graphics.fancy");
        }
        return caption + language->getElement(L"options.graphics.fast");
    }

    return caption;
}

void Options::load() {
    
    
    if (!optionsFile.exists()) return;
    
    BufferedReader* br = new BufferedReader(
        new InputStreamReader(new FileInputStream(optionsFile)));

    std::wstring line = L"";
    static const std::wstring kKnownKeys[] = {
        L"music",          L"sound",          L"mouseSensitivity",
        L"fov",            L"gamma",          L"invertYMouse",
        L"viewDistance",   L"guiScale",       L"particles",
        L"bobView",        L"anaglyph3d",     L"advancedOpengl",
        L"fpsLimit",       L"difficulty",     L"fancyGraphics",
        L"ao",             L"clouds",         L"skin",
        L"lastServer",     L"lastNickname",
    };

    while ((line = br->readLine()) !=
           L"")  
                 
    {
        if (line.find(L'\0') != std::wstring::npos) {
            std::wstring filtered;
            filtered.reserve(line.size());
            for (wchar_t wc : line) {
                if (wc != L'\0') filtered.push_back(wc);
            }
            line = filtered;
        }
        std::vector<std::wstring> pairs;
        {
            std::vector<size_t> cuts;
            cuts.push_back(0);
            auto considerCutAt = [&](size_t pos) {
                if (pos != 0 && pos != std::wstring::npos &&
                    std::find(cuts.begin(), cuts.end(), pos) == cuts.end()) {
                    cuts.push_back(pos);
                }
            };
            for (const auto& key : kKnownKeys) {
                size_t pos = 0;
                while ((pos = line.find(key, pos)) != std::wstring::npos) {
                    bool followedByColon =
                        pos + key.length() < line.length() &&
                        line[pos + key.length()] == L':';
                    bool atBoundary =
                        pos == 0 ||
                        !((line[pos - 1] >= L'a' && line[pos - 1] <= L'z') ||
                          (line[pos - 1] >= L'A' && line[pos - 1] <= L'Z') ||
                          (line[pos - 1] >= L'0' && line[pos - 1] <= L'9') ||
                          line[pos - 1] == L'_');
                    if (followedByColon && atBoundary) {
                        considerCutAt(pos);
                    }
                    pos += key.length();
                }
            }
            size_t pos = 0;
            while ((pos = line.find(L"key_", pos)) != std::wstring::npos) {
                bool atBoundary =
                    pos == 0 ||
                    !((line[pos - 1] >= L'a' && line[pos - 1] <= L'z') ||
                      (line[pos - 1] >= L'A' && line[pos - 1] <= L'Z') ||
                      (line[pos - 1] >= L'0' && line[pos - 1] <= L'9') ||
                      line[pos - 1] == L'_');
                if (atBoundary) considerCutAt(pos);
                pos += 4;
            }
            std::sort(cuts.begin(), cuts.end());
            for (size_t i = 0; i < cuts.size(); ++i) {
                size_t startPos = cuts[i];
                size_t endPos =
                    (i + 1 < cuts.size()) ? cuts[i + 1] : line.length();
                pairs.push_back(line.substr(startPos, endPos - startPos));
            }
        }

        for (const auto& rawPair : pairs) {
            std::wstring pair = rawPair;
            
            
            std::wstring cmds[2];
            int splitpos = (int)pair.find(L":");
            if (splitpos == (int)std::wstring::npos) {
                cmds[0] = pair;
                cmds[1] = L"";
            } else {
                cmds[0] = pair.substr(0, splitpos);
                cmds[1] = pair.substr(splitpos + 1);
            }

            if (cmds[0] == L"music") {
                music = readFloat(cmds[1]);
            }
            if (cmds[0] == L"sound") {
                sound = readFloat(cmds[1]);
            }
            if (cmds[0] == L"mouseSensitivity")
                sensitivity = readFloat(cmds[1]);
            if (cmds[0] == L"fov") fov = readFloat(cmds[1]);
            if (cmds[0] == L"gamma") gamma = readFloat(cmds[1]);
            if (cmds[0] == L"invertYMouse")
                invertYMouse = cmds[1] == L"true";
            if (cmds[0] == L"viewDistance")
                viewDistance = fromWString<int>(cmds[1]);
            if (cmds[0] == L"guiScale")
                guiScale = fromWString<int>(cmds[1]);
            if (cmds[0] == L"particles")
                particles = fromWString<int>(cmds[1]);
            if (cmds[0] == L"bobView") bobView = cmds[1] == L"true";
            if (cmds[0] == L"anaglyph3d") anaglyph3d = cmds[1] == L"true";
            if (cmds[0] == L"advancedOpengl")
                advancedOpengl = cmds[1] == L"true";
            if (cmds[0] == L"fpsLimit")
                framerateLimit = fromWString<int>(cmds[1]);
            if (cmds[0] == L"difficulty")
                difficulty = fromWString<int>(cmds[1]);
            if (cmds[0] == L"fancyGraphics")
                fancyGraphics = cmds[1] == L"true";
            if (cmds[0] == L"ao") ambientOcclusion = cmds[1] == L"true";
            if (cmds[0] == L"clouds") renderClouds = cmds[1] == L"true";
            if (cmds[0] == L"skin") skin = cmds[1];
            if (cmds[0] == L"lastServer") lastMpIp = cmds[1];
            if (cmds[0] == L"lastNickname") lastMpNickname = cmds[1];

            for (int i = 0; i < keyMappings_length; i++) {
                if (cmds[0] == (L"key_" + keyMappings[i]->name)) {
                    keyMappings[i]->key = fromWString<int>(cmds[1]);
                }
            }
            
            
            
            
        }
    }
    
    br->close();
    if (!lastMpNickname.empty()) {
        for (int p = 0; p < XUSER_MAX_COUNT; ++p) {
            SetUserGamertag(p, lastMpNickname);
        }
    }
    
    
    
    
}

float Options::readFloat(std::wstring string) {
    if (string == L"true") return 1;
    if (string == L"false") return 0;
    return fromWString<float>(string);
}

void Options::save() {
    
    

    
    
    FileOutputStream fos = FileOutputStream(optionsFile);
    DataOutputStream dos = DataOutputStream(&fos);
    

    auto writeAscii = [&dos](const std::wstring& s) {
        for (wchar_t wc : s) {
            unsigned int b = static_cast<unsigned int>(wc) & 0xff;
            dos.write(b);
        }
    };

    writeAscii(L"music:" + toWString<float>(music) + L"\n");
    writeAscii(L"sound:" + toWString<float>(sound) + L"\n");
    writeAscii(L"invertYMouse:" +
               std::wstring(invertYMouse ? L"true" : L"false") + L"\n");
    writeAscii(L"mouseSensitivity:" + toWString<float>(sensitivity) + L"\n");
    writeAscii(L"fov:" + toWString<float>(fov) + L"\n");
    writeAscii(L"gamma:" + toWString<float>(gamma) + L"\n");
    writeAscii(L"viewDistance:" + toWString<int>(viewDistance) + L"\n");
    writeAscii(L"guiScale:" + toWString<int>(guiScale) + L"\n");
    writeAscii(L"particles:" + toWString<int>(particles) + L"\n");
    writeAscii(L"bobView:" + std::wstring(bobView ? L"true" : L"false") +
               L"\n");
    writeAscii(L"anaglyph3d:" +
               std::wstring(anaglyph3d ? L"true" : L"false") + L"\n");
    writeAscii(L"advancedOpengl:" +
               std::wstring(advancedOpengl ? L"true" : L"false") + L"\n");
    writeAscii(L"fpsLimit:" + toWString<int>(framerateLimit) + L"\n");
    writeAscii(L"difficulty:" + toWString<int>(difficulty) + L"\n");
    writeAscii(L"fancyGraphics:" +
               std::wstring(fancyGraphics ? L"true" : L"false") + L"\n");
    writeAscii(L"ao:" +
               std::wstring(ambientOcclusion ? L"true" : L"false") + L"\n");
    writeAscii(L"clouds:" + toWString<bool>(renderClouds) + L"\n");
    writeAscii(L"skin:" + skin + L"\n");
    writeAscii(L"lastServer:" + lastMpIp + L"\n");
    writeAscii(L"lastNickname:" + lastMpNickname + L"\n");

    for (int i = 0; i < keyMappings_length; i++) {
        writeAscii(L"key_" + keyMappings[i]->name + L":" +
                   toWString<int>(keyMappings[i]->key) + L"\n");
    }

    dos.close();
    
    
    
    
}

bool Options::isCloudsOn() { return viewDistance < 2 && renderClouds; }
