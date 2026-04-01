#pragma once
// using namespace std;
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "4J.Common/4J_Compat.h"
#include "DLCManager.h"
#include "Minecraft.Client/Common/src/DLC/DLCSkinFile.h"

class DLCFile;
class DLCSkinFile;

class DLCPack {
private:
    std::vector<DLCFile*> m_files[DLCManager::e_DLCType_Max];
    std::vector<DLCPack*> m_childPacks;
    DLCPack* m_parentPack;

    std::unordered_map<int, std::wstring> m_parameters;

    std::wstring m_packName;
    std::wstring m_dataPath;
    std::uint32_t m_dwLicenseMask;
    int m_dlcMountIndex;
    XCONTENTDEVICEID m_dlcDeviceID;
    uint64_t m_ullFullOfferId;
    bool m_isCorrupt;
    std::uint32_t m_packId;
    std::uint32_t m_packVersion;

    std::uint8_t*
        m_data;  // This pointer is for all the data used for this pack, so
                 // deleting it invalidates ALL of it's children.
public:
    DLCPack(const std::wstring& name, std::uint32_t dwLicenseMask);
    ~DLCPack();

    std::wstring getFullDataPath() { return m_dataPath; }

    void SetDataPointer(std::uint8_t* pbData) { m_data = pbData; }

    bool IsCorrupt() { return m_isCorrupt; }
    void SetIsCorrupt(bool val) { m_isCorrupt = val; }

    void SetPackId(std::uint32_t id) { m_packId = id; }
    std::uint32_t GetPackId() { return m_packId; }

    void SetPackVersion(std::uint32_t version) { m_packVersion = version; }
    std::uint32_t GetPackVersion() { return m_packVersion; }

    DLCPack* GetParentPack() { return m_parentPack; }
    std::uint32_t GetParentPackId() { return m_parentPack->m_packId; }

    void SetDLCMountIndex(int id) { m_dlcMountIndex = id; }
    int GetDLCMountIndex();
    void SetDLCDeviceID(XCONTENTDEVICEID deviceId) { m_dlcDeviceID = deviceId; }
    XCONTENTDEVICEID GetDLCDeviceID();

    void addChildPack(DLCPack* childPack);
    void setParentPack(DLCPack* parentPack);

    void addParameter(DLCManager::EDLCParameterType type,
                      const std::wstring& value);
    bool getParameterAsUInt(DLCManager::EDLCParameterType type,
                            unsigned int& param);

    void updateLicenseMask(std::uint32_t dwLicenseMask) {
        m_dwLicenseMask = dwLicenseMask;
    }
    std::uint32_t getLicenseMask() { return m_dwLicenseMask; }

    std::wstring getName() { return m_packName; }

    void UpdateLanguage();
    uint64_t getPurchaseOfferId() { return m_ullFullOfferId; }

    DLCFile* addFile(DLCManager::EDLCType type, const std::wstring& path);
    DLCFile* getFile(DLCManager::EDLCType type, unsigned int index);
    DLCFile* getFile(DLCManager::EDLCType type, const std::wstring& path);

    unsigned int getDLCItemsCount(
        DLCManager::EDLCType type = DLCManager::e_DLCType_All);
    unsigned int getFileIndexAt(DLCManager::EDLCType type,
                                const std::wstring& path, bool& found);
    bool doesPackContainFile(DLCManager::EDLCType type,
                             const std::wstring& path);
    std::uint32_t GetPackID() { return m_packId; }

    unsigned int getSkinCount() {
        return getDLCItemsCount(DLCManager::e_DLCType_Skin);
    }
    unsigned int getSkinIndexAt(const std::wstring& path, bool& found) {
        return getFileIndexAt(DLCManager::e_DLCType_Skin, path, found);
    }
    DLCSkinFile* getSkinFile(const std::wstring& path) {
        return (DLCSkinFile*)getFile(DLCManager::e_DLCType_Skin, path);
    }
    DLCSkinFile* getSkinFile(unsigned int index) {
        return (DLCSkinFile*)getFile(DLCManager::e_DLCType_Skin, index);
    }
    bool doesPackContainSkin(const std::wstring& path) {
        return doesPackContainFile(DLCManager::e_DLCType_Skin, path);
    }

    bool hasPurchasedFile(DLCManager::EDLCType type, const std::wstring& path);
};
