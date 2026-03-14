#include "../../Platform/stdafx.h"
#include "../../Platform/Common/DLC/DLCGameRulesFile.h"
#include "../../Platform/Common/DLC/DLCGameRulesHeader.h"
#include "../../Platform/Common/DLC/DLCGameRules.h"
#include "DLCTexturePack.h"
#include "../../Platform/Common/DLC/DLCColourTableFile.h"
#include "../../Platform/Common/DLC/DLCUIDataFile.h"
#include "../../Platform/Common/DLC/DLCTextureFile.h"
#include "../../Platform/Common/DLC/DLCLocalisationFile.h"
#include "../../../Minecraft.World/Util/StringHelpers.h"
#include "../../../Minecraft.World/Util/PortableFileIO.h"
#include "../../Utils/StringTable.h"
#include "../../Platform/Common/DLC/DLCAudioFile.h"

#include <cstdint>
#include <limits>

#if defined _XBOX || defined _WINDOWS64
#include "../../Platform/Xbox/XML/ATGXmlParser.h"
#include "../../Platform/Xbox/XML/xmlFilesCallback.h"
#endif

namespace {
bool ReadPortableBinaryFile(File& file, std::uint8_t*& data,
                            unsigned int& size) {
    const __int64 fileLength = file.length();
    if (fileLength < 0 ||
        fileLength >
            static_cast<__int64>(std::numeric_limits<unsigned int>::max())) {
        data = NULL;
        size = 0;
        return false;
    }

    const std::size_t capacity = static_cast<std::size_t>(fileLength);
    std::uint8_t* buffer = new std::uint8_t[capacity == 0 ? 1 : capacity];
    const PortableFileIO::BinaryReadResult readResult =
        PortableFileIO::ReadBinaryFile(file.getPath(), buffer, capacity);
    if (readResult.status != PortableFileIO::BinaryReadStatus::ok ||
        readResult.fileSize > std::numeric_limits<unsigned int>::max()) {
        delete[] buffer;
        data = NULL;
        size = 0;
        return false;
    }

    data = buffer;
    size = static_cast<unsigned int>(readResult.fileSize);
    return true;
}
}  // namespace

DLCTexturePack::DLCTexturePack(std::uint32_t id, DLCPack* pack,
                               TexturePack* fallback)
    : AbstractTexturePack(id, NULL, pack->getName(), fallback) {
    m_dlcInfoPack = pack;
    m_dlcDataPack = NULL;
    bUILoaded = false;
    m_bLoadingData = false;
    m_bHasLoadedData = false;
    m_archiveFile = NULL;
    if (app.getLevelGenerationOptions())
        app.getLevelGenerationOptions()->setLoadedData();
    m_bUsingDefaultColourTable = true;

    m_stringTable = NULL;

#ifdef _XBOX
    m_pStreamedWaveBank = NULL;
    m_pSoundBank = NULL;
#endif

    if (m_dlcInfoPack->doesPackContainFile(
            DLCManager::e_DLCType_LocalisationData, L"languages.loc")) {
        DLCLocalisationFile* localisationFile =
            (DLCLocalisationFile*)m_dlcInfoPack->getFile(
                DLCManager::e_DLCType_LocalisationData, L"languages.loc");
        m_stringTable = localisationFile->getStringTable();
    }

    // 4J Stu - These calls need to be in the most derived version of the class
    loadIcon();
    loadName();
    loadDescription();
    // loadDefaultHTMLColourTable();
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
        if (m_stringTable != NULL) {
            texname = m_stringTable->getString(L"IDS_DISPLAY_NAME");
            m_wsWorldName = m_stringTable->getString(L"IDS_WORLD_NAME");
        }
    } else {
        if (m_stringTable != NULL) {
            texname = m_stringTable->getString(L"IDS_DISPLAY_NAME");
        }
    }
}

void DLCTexturePack::loadDescription() {
    desc1 = L"";

    if (m_stringTable != NULL) {
        desc1 = m_stringTable->getString(L"IDS_TP_DESCRIPTION");
    }
}

std::wstring DLCTexturePack::getResource(const std::wstring& name) {
    // 4J Stu - We should never call this function
#ifndef __CONTENT_PACKAGE
    __debugbreak();
#endif
    return L"";
}

InputStream* DLCTexturePack::getResourceImplementation(
    const std::wstring& name)  // throws IOException
{
    // 4J Stu - We should never call this function
#ifndef _CONTENT_PACKAGE
    __debugbreak();
    if (hasFile(name)) return NULL;
#endif
    return NULL;  // resource;
}

bool DLCTexturePack::hasFile(const std::wstring& name) {
    bool hasFile = false;
    if (m_dlcDataPack != NULL)
        hasFile = m_dlcDataPack->doesPackContainFile(
            DLCManager::e_DLCType_Texture, name);
    return hasFile;
}

bool DLCTexturePack::isTerrainUpdateCompatible() { return true; }

std::wstring DLCTexturePack::getPath(bool bTitleUpdateTexture /*= false*/) {
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
    const std::wstring& File, bool filenameHasExtension /*= false*/,
    bool bTitleUpdateTexture /*=false*/, const std::wstring& drive /*=L""*/) {
    if (m_dlcDataPack)
        return new BufferedImage(m_dlcDataPack, L"/" + File,
                                 filenameHasExtension);
    else
        return fallback->getImageResource(File, filenameHasExtension,
                                          bTitleUpdateTexture, drive);
}

DLCPack* DLCTexturePack::getDLCPack() { return m_dlcDataPack; }

void DLCTexturePack::loadColourTable() {
    // Load the game colours
    if (m_dlcDataPack != NULL &&
        m_dlcDataPack->doesPackContainFile(DLCManager::e_DLCType_ColourTable,
                                           L"colours.col")) {
        DLCColourTableFile* colourFile =
            (DLCColourTableFile*)m_dlcDataPack->getFile(
                DLCManager::e_DLCType_ColourTable, L"colours.col");
        m_colourTable = colourFile->getColourTable();
        m_bUsingDefaultColourTable = false;
    } else {
        // 4J Stu - We can delete the default colour table, but not the one from
        // the DLCColourTableFile
        if (!m_bUsingDefaultColourTable) m_colourTable = NULL;
        loadDefaultColourTable();
        m_bUsingDefaultColourTable = true;
    }

    // Load the text colours
#ifdef _XBOX
    if (m_dlcDataPack != NULL &&
        m_dlcDataPack->doesPackContainFile(DLCManager::e_DLCType_UIData,
                                           L"TexturePack.xzp")) {
        DLCUIDataFile* dataFile = (DLCUIDataFile*)m_dlcDataPack->getFile(
            DLCManager::e_DLCType_UIData, L"TexturePack.xzp");

        std::uint32_t dwSize = 0;
        std::uint8_t* pbData = dataFile->getData(dwSize);

        constexpr int LOCATOR_SIZE =
            256;  // Use this to allocate space to hold a ResourceLocator string
        WCHAR szResourceLocator[LOCATOR_SIZE];

        // Try and load the HTMLColours.col based off the common XML first,
        // before the deprecated xuiscene_colourtable
        swprintf(szResourceLocator, LOCATOR_SIZE,
                 L"memory://%08X,%04X#HTMLColours.col", pbData, dwSize);
        std::uint8_t* data;
        unsigned int dataLength;
        if (XuiResourceLoadAll(szResourceLocator, &data, &dataLength) == S_OK) {
            m_colourTable->loadColoursFromData(data, dataLength);

            XuiFree(data);
        } else {
            swprintf(szResourceLocator, LOCATOR_SIZE,
                     L"memory://%08X,%04X#xuiscene_colourtable.xur", pbData,
                     dwSize);
            HXUIOBJ hScene;
            HRESULT hr = XuiSceneCreate(szResourceLocator, szResourceLocator,
                                        NULL, &hScene);

            if (HRESULT_SUCCEEDED(hr)) {
                loadHTMLColourTableFromXuiScene(hScene);
            } else {
                loadDefaultHTMLColourTable();
            }
        }
    } else {
        loadDefaultHTMLColourTable();
    }
#else
    if (app.hasArchiveFile(L"HTMLColours.col")) {
        byteArray textColours = app.getArchiveFile(L"HTMLColours.col");
        m_colourTable->loadColoursFromData(textColours.data,
                                           textColours.length);

        delete[] textColours.data;
    }
#endif
}

void DLCTexturePack::loadData() {
    int mountIndex = m_dlcInfoPack->GetDLCMountIndex();

    if (mountIndex > -1) {
#ifdef _DURANGO
        if (StorageManager.MountInstalledDLC(ProfileManager.GetPrimaryPad(),
                                             mountIndex,
                                             &DLCTexturePack::packMounted, this,
                                             L"TPACK") != ERROR_IO_PENDING)
#else
        if (StorageManager.MountInstalledDLC(ProfileManager.GetPrimaryPad(),
                                             mountIndex,
                                             &DLCTexturePack::packMounted, this,
                                             "TPACK") != ERROR_IO_PENDING)
#endif
        {
            // corrupt DLC
            m_bHasLoadedData = true;
            if (app.getLevelGenerationOptions())
                app.getLevelGenerationOptions()->setLoadedData();
            app.DebugPrintf("Failed to mount texture pack DLC %d for pad %d\n",
                            mountIndex, ProfileManager.GetPrimaryPad());
        } else {
            m_bLoadingData = true;
            app.DebugPrintf("Attempted to mount DLC data for texture pack %d\n",
                            mountIndex);
        }
    } else {
        m_bHasLoadedData = true;
        if (app.getLevelGenerationOptions())
            app.getLevelGenerationOptions()->setLoadedData();
        app.SetAction(ProfileManager.GetPrimaryPad(),
                      eAppAction_ReloadTexturePack);
    }
}

std::wstring DLCTexturePack::getFilePath(std::uint32_t packId,
                                         std::wstring filename,
                                         bool bAddDataFolder) {
    return app.getFilePath(packId, filename, bAddDataFolder);
}

int DLCTexturePack::packMounted(void* pParam, int iPad, std::uint32_t dwErr,
                                std::uint32_t dwLicenceMask) {
    DLCTexturePack* texturePack = static_cast<DLCTexturePack*>(pParam);
    texturePack->m_bLoadingData = false;
    if (dwErr != ERROR_SUCCESS) {
        // corrupt DLC
        app.DebugPrintf("Failed to mount DLC for pad %d: %u\n", iPad, dwErr);
    } else {
        app.DebugPrintf(
            "Mounted DLC for texture pack, attempting to load data\n");
        texturePack->m_dlcDataPack =
            new DLCPack(texturePack->m_dlcInfoPack->getName(), dwLicenceMask);
        texturePack->setHasAudio(false);
        unsigned int dwFilesProcessed = 0;
        // Load the DLC textures
        std::wstring dataFilePath =
            texturePack->m_dlcInfoPack->getFullDataPath();
        if (!dataFilePath.empty()) {
            if (!app.m_dlcManager.readDLCDataFile(
                    dwFilesProcessed,
                    getFilePath(texturePack->m_dlcInfoPack->GetPackID(),
                                dataFilePath),
                    texturePack->m_dlcDataPack)) {
                delete texturePack->m_dlcDataPack;
                texturePack->m_dlcDataPack = NULL;
            }

            // Load the UI data
            if (texturePack->m_dlcDataPack != NULL) {
#ifdef _XBOX
                File xzpPath(
                    getFilePath(texturePack->m_dlcInfoPack->GetPackID(),
                                std::wstring(L"TexturePack.xzp")));

                if (xzpPath.exists()) {
                    std::uint8_t* pbData = NULL;
                    unsigned int bytesRead = 0;
                    if (ReadPortableBinaryFile(xzpPath, pbData, bytesRead)) {
                        DLCUIDataFile* uiDLCFile =
                            (DLCUIDataFile*)texturePack->m_dlcDataPack->addFile(
                                DLCManager::e_DLCType_UIData,
                                L"TexturePack.xzp");
                        uiDLCFile->addData(pbData, bytesRead, true);
                    }
                }
#else
                File archivePath(
                    getFilePath(texturePack->m_dlcInfoPack->GetPackID(),
                                std::wstring(L"media.arc")));
                if (archivePath.exists())
                    texturePack->m_archiveFile = new ArchiveFile(archivePath);
#endif

                /**
                        4J-JEV:
                                For all the GameRuleHeader files we find
                */
                DLCPack* pack = texturePack->m_dlcInfoPack->GetParentPack();
                LevelGenerationOptions* levelGen =
                    app.getLevelGenerationOptions();
                if (levelGen != NULL && !levelGen->hasLoadedData()) {
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
                                std::uint8_t* pbData = NULL;
                                unsigned int fileSize = 0;
                                if (ReadPortableBinaryFile(grf, pbData,
                                                           fileSize)) {
                                    // 4J-PB - is it possible that we can get
                                    // here after a read fail and it's not an
                                    // error?
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
                            std::uint8_t* pbData = NULL;
                            unsigned int fileSize = 0;
                            if (ReadPortableBinaryFile(grf, pbData, fileSize)) {
                                // 4J-PB - is it possible that we can get here
                                // after a read fail and it's not an error?
                                levelGen->setBaseSaveData(pbData, fileSize);
                            } else {
                                app.FatalLoadError();
                            }
                        }
                    }
                }

                // any audio data?
#ifdef _XBOX
                File audioXSBPath(
                    getFilePath(texturePack->m_dlcInfoPack->GetPackID(),
                                std::wstring(L"MashUp.xsb")));
                File audioXWBPath(
                    getFilePath(texturePack->m_dlcInfoPack->GetPackID(),
                                std::wstring(L"MashUp.xwb")));

                if (audioXSBPath.exists() && audioXWBPath.exists()) {
                    texturePack->setHasAudio(true);
                    const char* pchXWBFilename =
                        wstringtofilename(audioXWBPath.getPath());
                    Minecraft::GetInstance()
                        ->soundEngine->CreateStreamingWavebank(
                            pchXWBFilename, &texturePack->m_pStreamedWaveBank);
                    const char* pchXSBFilename =
                        wstringtofilename(audioXSBPath.getPath());
                    Minecraft::GetInstance()->soundEngine->CreateSoundbank(
                        pchXSBFilename, &texturePack->m_pSoundBank);
                }
#else
                // DLCPack *pack = texturePack->m_dlcInfoPack->GetParentPack();
                if (pack->getDLCItemsCount(DLCManager::e_DLCType_Audio) > 0) {
                    DLCAudioFile* dlcFile = (DLCAudioFile*)pack->getFile(
                        DLCManager::e_DLCType_Audio, 0);
                    texturePack->setHasAudio(true);
                    // init the streaming sound ids for this texture pack
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
                        iEndStart + iEndC);  // push the CD start to after
                }
#endif
            }
            texturePack->loadColourTable();
        }

        // 4J-PB - we need to leave the texture pack mounted if it contained
        // streaming audio
        if (texturePack->hasAudio() == false) {
#ifdef _XBOX
            StorageManager.UnmountInstalledDLC("TPACK");
#endif
        }
    }

    texturePack->m_bHasLoadedData = true;
    if (app.getLevelGenerationOptions())
        app.getLevelGenerationOptions()->setLoadedData();
    app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ReloadTexturePack);

    return 0;
}

void DLCTexturePack::loadUI() {
#ifdef _XBOX
    // Syntax: "memory://" + Address + "," + Size + "#" + File
    // L"memory://0123ABCD,21A3#skin_default.xur"

    // Load new skin
    if (m_dlcDataPack != NULL &&
        m_dlcDataPack->doesPackContainFile(DLCManager::e_DLCType_UIData,
                                           L"TexturePack.xzp")) {
        DLCUIDataFile* dataFile = (DLCUIDataFile*)m_dlcDataPack->getFile(
            DLCManager::e_DLCType_UIData, L"TexturePack.xzp");

        std::uint32_t dwSize = 0;
        std::uint8_t* pbData = dataFile->getData(dwSize);

        constexpr int LOCATOR_SIZE =
            256;  // Use this to allocate space to hold a ResourceLocator string
        WCHAR szResourceLocator[LOCATOR_SIZE];
        swprintf(szResourceLocator, LOCATOR_SIZE,
                 L"memory://%08X,%04X#skin_Minecraft.xur", pbData, dwSize);

        XuiFreeVisuals(L"");

        HRESULT hr = app.LoadSkin(szResourceLocator, NULL);  // L"TexturePack");
        if (HRESULT_SUCCEEDED(hr)) {
            bUILoaded = true;
            // CXuiSceneBase::GetInstance()->SetVisualPrefix(L"TexturePack");
            // CXuiSceneBase::GetInstance()->SkinChanged(CXuiSceneBase::GetInstance()->m_hObj);
        }
    }
#else
    if (m_archiveFile && m_archiveFile->hasFile(L"skin.swf")) {
        ui.ReloadSkin();
        bUILoaded = true;
    }
#endif
    else {
        loadDefaultUI();
        bUILoaded = true;
    }

    AbstractTexturePack::loadUI();
#ifndef _XBOX
    if (hasAudio() == false && !ui.IsReloadingSkin()) {
#ifdef _DURANGO
        StorageManager.UnmountInstalledDLC(L"TPACK");
#else
        StorageManager.UnmountInstalledDLC("TPACK");
#endif
    }
#endif
}

void DLCTexturePack::unloadUI() {
    // Unload skin
    if (bUILoaded) {
#ifdef _XBOX
        XuiFreeVisuals(L"TexturePack");
        XuiFreeVisuals(L"");
        CXuiSceneBase::GetInstance()->SetVisualPrefix(L"");
        CXuiSceneBase::GetInstance()->SkinChanged(
            CXuiSceneBase::GetInstance()->m_hObj);
#endif
        setHasAudio(false);
    }
    AbstractTexturePack::unloadUI();

    app.m_dlcManager.removePack(m_dlcDataPack);
    m_dlcDataPack = NULL;
    delete m_archiveFile;
    m_bHasLoadedData = false;

    bUILoaded = false;
}

std::wstring DLCTexturePack::getXuiRootPath() {
    std::wstring path = L"";
    if (m_dlcDataPack != NULL &&
        m_dlcDataPack->doesPackContainFile(DLCManager::e_DLCType_UIData,
                                           L"TexturePack.xzp")) {
        DLCUIDataFile* dataFile = (DLCUIDataFile*)m_dlcDataPack->getFile(
            DLCManager::e_DLCType_UIData, L"TexturePack.xzp");

        std::uint32_t dwSize = 0;
        std::uint8_t* pbData = dataFile->getData(dwSize);

        constexpr int LOCATOR_SIZE =
            256;  // Use this to allocate space to hold a ResourceLocator string
        WCHAR szResourceLocator[LOCATOR_SIZE];
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
