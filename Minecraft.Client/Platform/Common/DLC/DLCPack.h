#pragma once
//using namespace std;
#include <cstdint>
#include "DLCManager.h"

class DLCFile;
class DLCSkinFile;

class DLCPack
{
private:
	std::vector<DLCFile *> m_files[DLCManager::e_DLCType_Max];
	std::vector<DLCPack *> m_childPacks;
	DLCPack *m_parentPack;

	std::unordered_map<int, std::wstring> m_parameters;

	std::wstring m_packName;
	std::wstring m_dataPath;
	DWORD m_dwLicenseMask;
	int m_dlcMountIndex;
	XCONTENTDEVICEID m_dlcDeviceID;
#ifdef _XBOX_ONE
	std::wstring m_wsProductId;
#else
	ULONGLONG m_ullFullOfferId;
#endif
	bool m_isCorrupt;
	DWORD m_packId;
	DWORD m_packVersion;

	std::uint8_t *m_data; // This pointer is for all the data used for this pack, so deleting it invalidates ALL of it's children.
public:

	DLCPack(const std::wstring &name,DWORD dwLicenseMask);
#ifdef _XBOX_ONE
	DLCPack(const std::wstring &name,const std::wstring &productID,DWORD dwLicenseMask);
#endif
	~DLCPack();

	std::wstring getFullDataPath() { return m_dataPath; }

	void SetDataPointer(std::uint8_t *pbData) { m_data = pbData; }

	bool IsCorrupt() { return m_isCorrupt; }
	void SetIsCorrupt(bool val) { m_isCorrupt = val; }

	void SetPackId(DWORD id) { m_packId = id; }
	DWORD GetPackId() { return m_packId; }

	void SetPackVersion(DWORD version) { m_packVersion = version; }
	DWORD GetPackVersion() { return m_packVersion; }

	DLCPack * GetParentPack() { return m_parentPack; }
	DWORD GetParentPackId() { return m_parentPack->m_packId; }

	void SetDLCMountIndex(DWORD id) { m_dlcMountIndex = id; }
	DWORD GetDLCMountIndex();
	void SetDLCDeviceID(XCONTENTDEVICEID deviceId) { m_dlcDeviceID = deviceId; }
	XCONTENTDEVICEID GetDLCDeviceID();

	void addChildPack(DLCPack *childPack);
	void setParentPack(DLCPack *parentPack);

	void addParameter(DLCManager::EDLCParameterType type, const std::wstring &value);
	bool getParameterAsUInt(DLCManager::EDLCParameterType type, unsigned int &param);

	void updateLicenseMask( DWORD dwLicenseMask ) { m_dwLicenseMask = dwLicenseMask; }
	DWORD getLicenseMask( ) { return m_dwLicenseMask; }
	
	std::wstring getName() { return m_packName; }
#ifdef _XBOX_ONE
	std::wstring getPurchaseOfferId() { return m_wsProductId; }
#else
	ULONGLONG getPurchaseOfferId() { return m_ullFullOfferId; }
#endif

	DLCFile *addFile(DLCManager::EDLCType type, const std::wstring &path);
	DLCFile *getFile(DLCManager::EDLCType type, DWORD index);
	DLCFile *getFile(DLCManager::EDLCType type, const std::wstring &path);

	DWORD getDLCItemsCount(DLCManager::EDLCType type = DLCManager::e_DLCType_All);	
	DWORD getFileIndexAt(DLCManager::EDLCType type, const std::wstring &path, bool &found);
	bool doesPackContainFile(DLCManager::EDLCType type, const std::wstring &path);
	DWORD GetPackID() {return m_packId;}
	
	DWORD getSkinCount() { return getDLCItemsCount(DLCManager::e_DLCType_Skin); }
	DWORD getSkinIndexAt(const std::wstring &path, bool &found) { return getFileIndexAt(DLCManager::e_DLCType_Skin, path, found); }
	DLCSkinFile *getSkinFile(const std::wstring &path) { return (DLCSkinFile *)getFile(DLCManager::e_DLCType_Skin, path); }
	DLCSkinFile *getSkinFile(DWORD index) { return (DLCSkinFile *)getFile(DLCManager::e_DLCType_Skin, index); }
	bool doesPackContainSkin(const std::wstring &path) { return doesPackContainFile(DLCManager::e_DLCType_Skin, path); }

	bool hasPurchasedFile(DLCManager::EDLCType type, const std::wstring &path);
};
