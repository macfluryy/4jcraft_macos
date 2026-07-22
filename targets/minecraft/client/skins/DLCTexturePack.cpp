#include "DLCTexturePack.h"

#include <cstdint>
#include <cwchar>
#include <limits>
#include <vector>

#include "platform/sdl2/Input.h"
#include "platform/sdl2/Storage.h"
#include "app/common/App_enums.h"
#include "app/common/src/Audio/SoundEngine.h"
#include "app/common/src/Colours/ColourTable.h"
#include "app/common/src/DLC/DLCAudioFile.h"
#include "app/common/src/DLC/DLCColourTableFile.h"
#include "app/common/src/DLC/DLCFile.h"
#include "app/common/src/DLC/DLCGameRulesHeader.h"
#include "app/common/src/DLC/DLCLocalisationFile.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/DLC/DLCPack.h"
#include "app/common/src/DLC/DLCTextureFile.h"
#include "app/common/src/DLC/DLCUIDataFile.h"
#include "app/common/src/GameRules/GameRuleManager.h"
#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/common/src/Localisation/StringTable.h"
#include "app/common/src/UI/All Platforms/ArchiveFile.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "app/include/BufferedImage.h"
#include "platform/PlatformServices.h"
#include "java/File.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/skins/AbstractTexturePack.h"
#include "minecraft/client/skins/TexturePack.h"

#if defined(_WINDOWS64)
#include "app/windows/XML/ATGXmlParser.h"
#include "app/windows/XML/xmlFilesCallback.h"
#endif

namespace {
bool ReadPortableBinaryFile(File& file, std::uint8_t*& data,
                            unsigned int& size) {
    const int64_t fileLength = file.length();
    if (fileLength < 0 ||
        fileLength >
            static_cast<int64_t>(std::numeric_limits<unsigned int>::max())) {
        data = nullptr;
        size = 0;
        return false;
    }

    const std::size_t capacity = static_cast<std::size_t>(fileLength);
    std::uint8_t* buffer = new std::uint8_t[capacity == 0 ? 1 : capacity];
    auto readResult =
        PlatformFileIO.readFile(file.getPath(), buffer, capacity);
    if (readResult.status != IPlatformFileIO::ReadStatus::Ok ||
        readResult.fileSize > std::numeric_limits<unsigned int>::max()) {
        delete[] buffer;
        data = nullptr;
        size = 0;
        return false;
    }

    data = buffer;
    size = static_cast<unsigned int>(readResult.fileSize);
    return true;
}
}  

DLCTexturePack::DLCTexturePack(std::uint32_t id, DLCPack* pack,
                               TexturePack* fallback)
    : AbstractTexturePack(id, nullptr, pack->getName(), fallback) {
    m_dlcInfoPack = pack;
    m_dlcDataPack = nullptr;
    bUILoaded = false;
    m_bLoadingData = false;
    m_bHasLoadedData = false;
    m_archiveFile = nullptr;
    if (app.getLevelGenerationOptions())
        app.getLevelGenerationOptions()->setLoadedData();
    m_bUsingDefaultColourTable = true;

    m_stringTable = nullptr;

    if (m_dlcInfoPack->doesPackContainFile(
            DLCManager::e_DLCType_LocalisationData, L"languages.loc")) {
        DLCLocalisationFile* localisationFile =
            (DLCLocalisationFile*)m_dlcInfoPack->getFile(
                DLCManager::e_DLCType_LocalisationData, L"languages.loc");
        m_stringTable = localisationFile->getStringTable();
    }

    
    loadIcon();
    loadName();
    loadDescription();
    
}

void DLCTexturePack::loadIcon() {
    if (m_dlcInfoPack->doesPackContainFile(DLCManager::e_DLCType_Texture,
                                           L"icon.png")) {
        DLCTextureFile* textureFile = (DLCTextureFile*)m_dlcInfoPack->getFile(
            DLCManager::e_DLCType_Texture, L"icon.png");
        std::uint32_t iconSize = 0;
        m_iconData = textureFile->getData(iconSize);
        m_iconSize = iconSize;
    } else {
        AbstractTexturePack::loadIcon();
    }
}

void DLCTexturePack::loadComparison() {
    if (m_dlcInfoPack->doesPackContainFile(DLCManager::e_DLCType_Texture,
                                           L"comparison.png")) {
        DLCTextureFile* textureFile = (DLCTextureFile*)m_dlcInfoPack->getFile(
            DLCManager::e_DLCType_Texture, L"comparison.png");
        std::uint32_t comparisonSize = 0;
        m_comparisonData = textureFile->getData(comparisonSize);
        m_comparisonSize = comparisonSize;
    }
}

void DLCTexturePack::loadName() {
    texname = L"";

    if (m_dlcInfoPack->GetPackID() & 1024) {
        if (m_stringTable != nullptr) {
            texname = m_stringTable->getString(L"IDS_DISPLAY_NAME");
            m_wsWorldName = m_stringTable->getString(L"IDS_WORLD_NAME");
        }
    } else {
        if (m_stringTable != nullptr) {
            texname = m_stringTable->getString(L"IDS_DISPLAY_NAME");
        }
    }
}

void DLCTexturePack::loadDescription() {
    desc1 = L"";

    if (m_stringTable != nullptr) {
        desc1 = m_stringTable->getString(L"IDS_TP_DESCRIPTION");
    }
}

std::wstring DLCTexturePack::getResource(const std::wstring& name) {
    
#if !defined(__CONTENT_PACKAGE)
    __debugbreak();
#endif
    return L"";
}

InputStream* DLCTexturePack::getResourceImplementation(
    const std::wstring& name)  
{
    
#if !defined(_CONTENT_PACKAGE)
    __debugbreak();
    if (hasFile(name)) return nullptr;
#endif
    return nullptr;  
}

bool DLCTexturePack::hasFile(const std::wstring& name) {
    bool hasFile = false;
    if (m_dlcDataPack != nullptr)
        hasFile = m_dlcDataPack->doesPackContainFile(
            DLCManager::e_DLCType_Texture, name);
    return hasFile;
}

bool DLCTexturePack::isTerrainUpdateCompatible() { return true; }

std::wstring DLCTexturePack::getPath(bool bTitleUpdateTexture ,
                                     const char* pchBDPatchFilename) {
    return L"";
}

std::wstring DLCTexturePack::getAnimationString(const std::wstring& textureName,
                                                const std::wstring& path) {
    std::wstring result = L"";

    std::wstring fullpath = L"res/" + path + textureName + L".png";
    if (hasFile(fullpath)) {
        result = m_dlcDataPack->getFile(DLCManager::e_DLCType_Texture, fullpath)
                     ->getParameterAsString(DLCManager::e_DLCParamType_Anim);
    }

    return result;
}

BufferedImage* DLCTexturePack::getImageResource(
    const std::wstring& File, bool filenameHasExtension ,
    bool bTitleUpdateTexture , const std::wstring& drive ) {
    if (m_dlcDataPack)
        return new BufferedImage(m_dlcDataPack, L"/" + File,
                                 filenameHasExtension);
    else
        return fallback->getImageResource(File, filenameHasExtension,
                                          bTitleUpdateTexture, drive);
}

DLCPack* DLCTexturePack::getDLCPack() { return m_dlcDataPack; }

void DLCTexturePack::loadColourTable() {
    
    if (m_dlcDataPack != nullptr &&
        m_dlcDataPack->doesPackContainFile(DLCManager::e_DLCType_ColourTable,
                                           L"colours.col")) {
        DLCColourTableFile* colourFile =
            (DLCColourTableFile*)m_dlcDataPack->getFile(
                DLCManager::e_DLCType_ColourTable, L"colours.col");
        m_colourTable = colourFile->getColourTable();
        m_bUsingDefaultColourTable = false;
    } else {
        
        
        if (!m_bUsingDefaultColourTable) m_colourTable = nullptr;
        loadDefaultColourTable();
        m_bUsingDefaultColourTable = true;
    }

    
    if (app.hasArchiveFile(L"HTMLColours.col")) {
        std::vector<uint8_t> textColours =
            app.getArchiveFile(L"HTMLColours.col");
        m_colourTable->loadColoursFromData(textColours.data(),
                                           textColours.size());
    }
}

void DLCTexturePack::loadData() {
    int mountIndex = m_dlcInfoPack->GetDLCMountIndex();

    if (mountIndex > -1) {
        if (StorageManager.MountInstalledDLC(
                InputManager.GetPrimaryPad(), mountIndex,
                [this](int pad, std::uint32_t err, std::uint32_t lic) {
                    return onPackMounted(pad, err, lic);
                },
                "TPACK") != ERROR_IO_PENDING) {
            
            m_bHasLoadedData = true;
            if (app.getLevelGenerationOptions())
                app.getLevelGenerationOptions()->setLoadedData();
            app.DebugPrintf("Failed to mount texture pack DLC %d for pad %d\n",
                            mountIndex, InputManager.GetPrimaryPad());
        } else {
            m_bLoadingData = true;
            app.DebugPrintf("Attempted to mount DLC data for texture pack %d\n",
                            mountIndex);
        }
    } else {
        m_bHasLoadedData = true;
        if (app.getLevelGenerationOptions())
            app.getLevelGenerationOptions()->setLoadedData();
        app.SetAction(InputManager.GetPrimaryPad(),
                      eAppAction_ReloadTexturePack);
    }
}

std::wstring DLCTexturePack::getFilePath(std::uint32_t packId,
                                         std::wstring filename,
                                         bool bAddDataFolder) {
    return app.getFilePath(packId, filename, bAddDataFolder);
}

int DLCTexturePack::onPackMounted(int iPad, std::uint32_t dwErr,
                                  std::uint32_t dwLicenceMask) {
    DLCTexturePack* texturePack = this;
    texturePack->m_bLoadingData = false;
    if (dwErr != ERROR_SUCCESS) {
        
        app.DebugPrintf("Failed to mount DLC for pad %d: %u\n", iPad, dwErr);
    } else {
        app.DebugPrintf(
            "Mounted DLC for texture pack, attempting to load data\n");
        texturePack->m_dlcDataPack =
            new DLCPack(texturePack->m_dlcInfoPack->getName(), dwLicenceMask);
        texturePack->setHasAudio(false);
        unsigned int dwFilesProcessed = 0;
        
        std::wstring dataFilePath =
            texturePack->m_dlcInfoPack->getFullDataPath();
        if (!dataFilePath.empty()) {
            if (!app.m_dlcManager.readDLCDataFile(
                    dwFilesProcessed,
                    getFilePath(texturePack->m_dlcInfoPack->GetPackID(),
                                dataFilePath),
                    texturePack->m_dlcDataPack)) {
                delete texturePack->m_dlcDataPack;
                texturePack->m_dlcDataPack = nullptr;
            }

            
            if (texturePack->m_dlcDataPack != nullptr) {
                File archivePath(
                    getFilePath(texturePack->m_dlcInfoPack->GetPackID(),
                                std::wstring(L"media.arc")));
                if (archivePath.exists())
                    texturePack->m_archiveFile = new ArchiveFile(archivePath);

                



                DLCPack* pack = texturePack->m_dlcInfoPack->GetParentPack();
                LevelGenerationOptions* levelGen =
                    app.getLevelGenerationOptions();
                if (levelGen != nullptr && !levelGen->hasLoadedData()) {
                    int gameRulesCount = pack->getDLCItemsCount(
                        DLCManager::e_DLCType_GameRulesHeader);
                    for (int i = 0; i < gameRulesCount; ++i) {
                        DLCGameRulesHeader* dlcFile =
                            (DLCGameRulesHeader*)pack->getFile(
                                DLCManager::e_DLCType_GameRulesHeader, i);

                        if (!dlcFile->getGrfPath().empty()) {
                            File grf(getFilePath(
                                texturePack->m_dlcInfoPack->GetPackID(),
                                dlcFile->getGrfPath()));
                            if (grf.exists()) {
                                std::uint8_t* pbData = nullptr;
                                unsigned int fileSize = 0;
                                if (ReadPortableBinaryFile(grf, pbData,
                                                           fileSize)) {
                                    
                                    
                                    
                                    dlcFile->setGrfData(
                                        pbData, fileSize,
                                        texturePack->m_stringTable);

                                    delete[] pbData;

                                    app.m_gameRules.setLevelGenerationOptions(
                                        dlcFile->lgo);
                                } else {
                                    app.FatalLoadError();
                                }
                            }
                        }
                    }
                    if (levelGen->requiresBaseSave() &&
                        !levelGen->getBaseSavePath().empty()) {
                        File grf(
                            getFilePath(texturePack->m_dlcInfoPack->GetPackID(),
                                        levelGen->getBaseSavePath()));
                        if (grf.exists()) {
                            std::uint8_t* pbData = nullptr;
                            unsigned int fileSize = 0;
                            if (ReadPortableBinaryFile(grf, pbData, fileSize)) {
                                
                                
                                levelGen->setBaseSaveData(pbData, fileSize);
                            } else {
                                app.FatalLoadError();
                            }
                        }
                    }
                }

                
                
                if (pack->getDLCItemsCount(DLCManager::e_DLCType_Audio) > 0) {
                    DLCAudioFile* dlcFile = (DLCAudioFile*)pack->getFile(
                        DLCManager::e_DLCType_Audio, 0);
                    texturePack->setHasAudio(true);
                    
                    int iOverworldStart, iNetherStart, iEndStart;
                    int iOverworldC, iNetherC, iEndC;

                    iOverworldStart = 0;
                    iOverworldC = dlcFile->GetCountofType(
                        DLCAudioFile::e_AudioType_Overworld);
                    iNetherStart = iOverworldC;
                    iNetherC = dlcFile->GetCountofType(
                        DLCAudioFile::e_AudioType_Nether);
                    iEndStart = iOverworldC + iNetherC;
                    iEndC =
                        dlcFile->GetCountofType(DLCAudioFile::e_AudioType_End);

                    Minecraft::GetInstance()->soundEngine->SetStreamingSounds(
                        iOverworldStart, iOverworldStart + iOverworldC,
                        iNetherStart, iNetherStart + iNetherC, iEndStart,
                        iEndStart + iEndC,
                        iEndStart + iEndC);  
                }
            }
            texturePack->loadColourTable();
        }

        
        
        if (texturePack->hasAudio() == false) {
        }
    }

    texturePack->m_bHasLoadedData = true;
    if (app.getLevelGenerationOptions())
        app.getLevelGenerationOptions()->setLoadedData();
    app.SetAction(InputManager.GetPrimaryPad(), eAppAction_ReloadTexturePack);

    return 0;
}

void DLCTexturePack::loadUI() {
    if (m_archiveFile && m_archiveFile->hasFile(L"skin.swf")) {
        ui.ReloadSkin();
        bUILoaded = true;
    } else {
        loadDefaultUI();
        bUILoaded = true;
    }

    AbstractTexturePack::loadUI();
    if (hasAudio() == false && !ui.IsReloadingSkin()) {
        StorageManager.UnmountInstalledDLC("TPACK");
    }
}

void DLCTexturePack::unloadUI() {
    
    if (bUILoaded) {
        setHasAudio(false);
    }
    AbstractTexturePack::unloadUI();

    app.m_dlcManager.removePack(m_dlcDataPack);
    m_dlcDataPack = nullptr;
    delete m_archiveFile;
    m_bHasLoadedData = false;

    bUILoaded = false;
}

std::wstring DLCTexturePack::getXuiRootPath() {
    std::wstring path = L"";
    if (m_dlcDataPack != nullptr &&
        m_dlcDataPack->doesPackContainFile(DLCManager::e_DLCType_UIData,
                                           L"TexturePack.xzp")) {
        DLCUIDataFile* dataFile = (DLCUIDataFile*)m_dlcDataPack->getFile(
            DLCManager::e_DLCType_UIData, L"TexturePack.xzp");

        std::uint32_t dwSize = 0;
        std::uint8_t* pbData = dataFile->getData(dwSize);

        constexpr int LOCATOR_SIZE =
            256;  
        wchar_t szResourceLocator[LOCATOR_SIZE];
        swprintf(szResourceLocator, LOCATOR_SIZE, L"memory://%08X,%04X#",
                 pbData, dwSize);
        path = szResourceLocator;
    }
    return path;
}

unsigned int DLCTexturePack::getDLCParentPackId() {
    return m_dlcInfoPack->GetParentPackId();
}

unsigned char DLCTexturePack::getDLCSubPackId() {
    return (m_dlcInfoPack->GetPackId() >> 24) & 0xFF;
}

DLCPack* DLCTexturePack::getDLCInfoParentPack() {
    return m_dlcInfoPack->GetParentPack();
}

XCONTENTDEVICEID DLCTexturePack::GetDLCDeviceID() {
    return m_dlcInfoPack->GetDLCDeviceID();
}
