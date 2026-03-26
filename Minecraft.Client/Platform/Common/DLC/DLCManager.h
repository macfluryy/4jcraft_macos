#pragma once
// using namespace std;
#include <cstdint>
#include <vector>
class DLCPack;
class DLCSkinFile;

class DLCManager {
public:
    enum EDLCType {
        e_DLCType_Skin = 0,
        e_DLCType_Cape,
        e_DLCType_Texture,
        e_DLCType_UIData,
        e_DLCType_PackConfig,
        e_DLCType_TexturePack,
        e_DLCType_LocalisationData,
        e_DLCType_GameRules,
        e_DLCType_Audio,
        e_DLCType_ColourTable,
        e_DLCType_GameRulesHeader,

        e_DLCType_Max,
        e_DLCType_All,
    };

    // If you add to the Enum,then you need to add the array of type names
    // These are the names used in the XML for the parameters
    enum EDLCParameterType {
        e_DLCParamType_Invalid = -1,

        e_DLCParamType_DisplayName = 0,
        e_DLCParamType_ThemeName,
        e_DLCParamType_Free,    // identify free skins
        e_DLCParamType_Credit,  // legal credits for DLC
        e_DLCParamType_Cape,
        e_DLCParamType_Box,
        e_DLCParamType_Anim,
        e_DLCParamType_PackId,
        e_DLCParamType_NetherParticleColour,
        e_DLCParamType_EnchantmentTextColour,
        e_DLCParamType_EnchantmentTextFocusColour,
        e_DLCParamType_DataPath,
        e_DLCParamType_PackVersion,

        e_DLCParamType_Max,

    };
    const static WCHAR* wchTypeNamesA[e_DLCParamType_Max];

private:
    std::vector<DLCPack*> m_packs;
    // bool m_bNeedsUpdated;
    bool m_bNeedsCorruptCheck;
    unsigned int m_dwUnnamedCorruptDLCCount;

public:
    DLCManager();
    ~DLCManager();

    static EDLCParameterType getParameterType(const std::wstring& paramName);

    unsigned int getPackCount(EDLCType type = e_DLCType_All);

    // bool NeedsUpdated() { return m_bNeedsUpdated; }
    // void SetNeedsUpdated(bool val) { m_bNeedsUpdated = val; }

    bool NeedsCorruptCheck() { return m_bNeedsCorruptCheck; }
    void SetNeedsCorruptCheck(bool val) { m_bNeedsCorruptCheck = val; }

    void resetUnnamedCorruptCount() { m_dwUnnamedCorruptDLCCount = 0; }
    void incrementUnnamedCorruptCount() { ++m_dwUnnamedCorruptDLCCount; }

    void addPack(DLCPack* pack);
    void removePack(DLCPack* pack);
    void removeAllPacks(void);
    void LanguageChanged(void);

    DLCPack* getPack(const std::wstring& name);
#if 0
    DLCPack* DLCManager::getPackFromProductID(const std::wstring& productID);
#endif
    DLCPack* getPack(unsigned int index, EDLCType type = e_DLCType_All);
    unsigned int getPackIndex(DLCPack* pack, bool& found,
                              EDLCType type = e_DLCType_All);
    DLCSkinFile* getSkinFile(
        const std::wstring& path);  // Will hunt all packs of type skin to find
                                    // the right skinfile

    DLCPack* getPackContainingSkin(const std::wstring& path);
    unsigned int getPackIndexContainingSkin(const std::wstring& path,
                                            bool& found);

    unsigned int checkForCorruptDLCAndAlert(bool showMessage = true);

    bool readDLCDataFile(unsigned int& dwFilesProcessed,
                         const std::wstring& path, DLCPack* pack,
                         bool fromArchive = false);
    bool readDLCDataFile(unsigned int& dwFilesProcessed,
                         const std::string& path, DLCPack* pack,
                         bool fromArchive = false);
    std::uint32_t retrievePackIDFromDLCDataFile(const std::string& path,
                                                DLCPack* pack);

private:
    bool processDLCDataFile(unsigned int& dwFilesProcessed,
                            std::uint8_t* pbData, unsigned int dwLength,
                            DLCPack* pack);

    std::uint32_t retrievePackID(std::uint8_t* pbData, unsigned int dwLength,
                                 DLCPack* pack);
};
