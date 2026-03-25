#pragma once

#include "AbstractTexturePack.h"

class DLCPack;
class StringTable;

class DLCTexturePack : public AbstractTexturePack {
private:
    DLCPack* m_dlcInfoPack;  // Description, icon etc
    DLCPack* m_dlcDataPack;  // Actual textures
    StringTable* m_stringTable;
    bool bUILoaded;
    bool m_bLoadingData, m_bHasLoadedData;
    bool m_bUsingDefaultColourTable;
    // bool m_bHasAudio;
    ArchiveFile* m_archiveFile;

public:
    using AbstractTexturePack::getResource;

    DLCTexturePack(std::uint32_t id, DLCPack* pack, TexturePack* fallback);
    ~DLCTexturePack() {};

    virtual std::wstring getResource(const std::wstring& name);
    virtual DLCPack* getDLCPack();
    virtual std::wstring getDesc1() {
        return m_stringTable->getString(L"IDS_TP_DESCRIPTION");
    }
    virtual std::wstring getName() {
        return m_stringTable->getString(L"IDS_DISPLAY_NAME");
    }
    virtual std::wstring getWorldName() {
        return m_stringTable->getString(L"IDS_WORLD_NAME");
    }

    // Added for sound banks with MashUp packs
#ifdef _XBOX
    IXACT3WaveBank* m_pStreamedWaveBank;
    IXACT3SoundBank* m_pSoundBank;
#endif
protected:
    //@Override
    void loadIcon();
    void loadComparison();
    void loadName();
    void loadDescription();
    InputStream* getResourceImplementation(
        const std::wstring& name);  // throws IOException

public:
    //@Override
    bool hasFile(const std::wstring& name);
    bool isTerrainUpdateCompatible();

    // 4J Added
    virtual std::wstring getPath(bool bTitleUpdateTexture = false,
                                 const char* pchBDPatchFilename = NULL);
    virtual std::wstring getAnimationString(const std::wstring& textureName,
                                            const std::wstring& path);
    virtual BufferedImage* getImageResource(const std::wstring& File,
                                            bool filenameHasExtension = false,
                                            bool bTitleUpdateTexture = false,
                                            const std::wstring& drive = L"");
    virtual void loadColourTable();
    virtual bool hasData() { return m_bHasLoadedData; }
    virtual bool isLoadingData() { return m_bLoadingData; }

private:
    static std::wstring getRootPath(std::uint32_t packId, bool allowOverride,
                                    bool bAddDataFolder);
    static std::wstring getFilePath(std::uint32_t packId, std::wstring filename,
                                    bool bAddDataFolder = true);

public:
    static int packMounted(void* pParam, int iPad, std::uint32_t dwErr,
                           std::uint32_t dwLicenceMask);
    virtual void loadData();
    virtual void loadUI();
    virtual void unloadUI();
    virtual std::wstring getXuiRootPath();
    virtual ArchiveFile* getArchiveFile() { return m_archiveFile; }

    virtual unsigned int getDLCParentPackId();
    virtual DLCPack* getDLCInfoParentPack();
    virtual unsigned char getDLCSubPackId();
    XCONTENTDEVICEID GetDLCDeviceID();
};
