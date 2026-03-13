#include "4J_Storage.h"
#include <cstring>
#include <vector>
#include <string>

C4JStorage StorageManager;

static XMARKETPLACE_CONTENTOFFER_INFO s_dummyOffer = {};
static XCONTENT_DATA s_dummyContentData = {};

C4JStorage::C4JStorage() : m_pStringTable(nullptr) {}

void C4JStorage::Tick(void) {}

C4JStorage::EMessageResult C4JStorage::RequestMessageBox(
    UINT uiTitle, UINT uiText, UINT* uiOptionA, UINT uiOptionC, DWORD dwPad,
    int (*Func)(void*, int, const C4JStorage::EMessageResult), void* lpParam,
    C4JStringTable* pStringTable, WCHAR* pwchFormatString,
    DWORD dwFocusButton) {
    return EMessage_ResultAccept;
}

C4JStorage::EMessageResult C4JStorage::GetMessageBoxResult() {
    return EMessage_Undefined;
}

bool C4JStorage::SetSaveDevice(int (*Func)(void*, const bool), void* lpParam,
                               bool bForceResetOfSaveDevice) {
    return true;
}

void C4JStorage::Init(unsigned int uiSaveVersion, LPCWSTR pwchDefaultSaveName,
                      char* pszSavePackName, int iMinimumSaveSize,
                      int (*Func)(void*, const ESavingMessage, int),
                      void* lpParam, LPCSTR szGroupID) {}
void C4JStorage::ResetSaveData() {}
void C4JStorage::SetDefaultSaveNameForKeyboardDisplay(
    LPCWSTR pwchDefaultSaveName) {}
void C4JStorage::SetSaveTitle(LPCWSTR pwchDefaultSaveName) {}
bool C4JStorage::GetSaveUniqueNumber(INT* piVal) {
    if (piVal) *piVal = 0;
    return true;
}
bool C4JStorage::GetSaveUniqueFilename(char* pszName) {
    if (pszName) pszName[0] = '\0';
    return true;
}
void C4JStorage::SetSaveUniqueFilename(char* szFilename) {}
void C4JStorage::SetState(ESaveGameControlState eControlState,
                          int (*Func)(void*, const bool), void* lpParam) {}
void C4JStorage::SetSaveDisabled(bool bDisable) {}
bool C4JStorage::GetSaveDisabled(void) { return false; }
unsigned int C4JStorage::GetSaveSize() { return 0; }
void C4JStorage::GetSaveData(void* pvData, unsigned int* puiBytes) {
    if (puiBytes) *puiBytes = 0;
}
PVOID C4JStorage::AllocateSaveData(unsigned int uiBytes) {
    return malloc(uiBytes);
}
void C4JStorage::SetSaveImages(PBYTE pbThumbnail, DWORD dwThumbnailBytes,
                               PBYTE pbImage, DWORD dwImageBytes,
                               PBYTE pbTextData, DWORD dwTextDataBytes) {}
C4JStorage::ESaveGameState C4JStorage::SaveSaveData(int (*Func)(void*,
                                                                const bool),
                                                    void* lpParam) {
    return ESaveGame_Idle;
}
void C4JStorage::CopySaveDataToNewSave(PBYTE pbThumbnail, DWORD cbThumbnail,
                                       WCHAR* wchNewName,
                                       int (*Func)(void* lpParam, bool),
                                       void* lpParam) {}
void C4JStorage::SetSaveDeviceSelected(unsigned int uiPad, bool bSelected) {}
bool C4JStorage::GetSaveDeviceSelected(unsigned int iPad) { return true; }
C4JStorage::ESaveGameState C4JStorage::DoesSaveExist(bool* pbExists) {
    if (pbExists) *pbExists = false;
    return ESaveGame_Idle;
}
bool C4JStorage::EnoughSpaceForAMinSaveGame() { return true; }
void C4JStorage::SetSaveMessageVPosition(float fY) {}
C4JStorage::ESaveGameState C4JStorage::GetSavesInfo(
    int iPad,
    int (*Func)(void* lpParam, SAVE_DETAILS* pSaveDetails, const bool),
    void* lpParam, char* pszSavePackName) {
    return ESaveGame_Idle;
}
PSAVE_DETAILS C4JStorage::ReturnSavesInfo() { return nullptr; }
void C4JStorage::ClearSavesInfo() {}
C4JStorage::ESaveGameState C4JStorage::LoadSaveDataThumbnail(
    PSAVE_INFO pSaveInfo,
    int (*Func)(void* lpParam, std::uint8_t* thumbnailData,
                unsigned int thumbnailBytes),
    void* lpParam) {
    return ESaveGame_Idle;
}
void C4JStorage::GetSaveCacheFileInfo(DWORD dwFile,
                                      XCONTENT_DATA& xContentData) {
    memset(&xContentData, 0, sizeof(xContentData));
}
void C4JStorage::GetSaveCacheFileInfo(DWORD dwFile, PBYTE* ppbImageData,
                                      DWORD* pdwImageBytes) {
    if (ppbImageData) *ppbImageData = nullptr;
    if (pdwImageBytes) *pdwImageBytes = 0;
}
C4JStorage::ESaveGameState C4JStorage::LoadSaveData(
    PSAVE_INFO pSaveInfo, int (*Func)(void* lpParam, const bool, const bool),
    void* lpParam) {
    return ESaveGame_Idle;
}
C4JStorage::ESaveGameState C4JStorage::DeleteSaveData(PSAVE_INFO pSaveInfo,
                                                      int (*Func)(void* lpParam,
                                                                  const bool),
                                                      void* lpParam) {
    return ESaveGame_Idle;
}
void C4JStorage::RegisterMarketplaceCountsCallback(
    int (*Func)(void* lpParam, C4JStorage::DLC_TMS_DETAILS*, int),
    void* lpParam) {}
void C4JStorage::SetDLCPackageRoot(char* pszDLCRoot) {}
C4JStorage::EDLCStatus C4JStorage::GetDLCOffers(
    int iPad, int (*Func)(void*, int, std::uint32_t, int), void* lpParam,
    DWORD dwOfferTypesBitmask) {
    return EDLC_NoOffers;
}
DWORD C4JStorage::CancelGetDLCOffers() { return 0; }
void C4JStorage::ClearDLCOffers() {}
XMARKETPLACE_CONTENTOFFER_INFO& C4JStorage::GetOffer(DWORD dw) {
    return s_dummyOffer;
}
int C4JStorage::GetOfferCount() { return 0; }
DWORD C4JStorage::InstallOffer(int iOfferIDC, __uint64* ullOfferIDA,
                               int (*Func)(void*, int, int), void* lpParam,
                               bool bTrial) {
    return 0;
}
DWORD C4JStorage::GetAvailableDLCCount(int iPad) { return 0; }
C4JStorage::EDLCStatus C4JStorage::GetInstalledDLC(int iPad,
                                                   int (*Func)(void*, int, int),
                                                   void* lpParam) {
    return EDLC_NoInstalledDLC;
}
XCONTENT_DATA& C4JStorage::GetDLC(DWORD dw) { return s_dummyContentData; }
std::uint32_t C4JStorage::MountInstalledDLC(
    int iPad, std::uint32_t dwDLC,
    int (*Func)(void*, int, std::uint32_t, std::uint32_t), void* lpParam,
    LPCSTR szMountDrive) {
    return 0;
}
DWORD C4JStorage::UnmountInstalledDLC(LPCSTR szMountDrive) { return 0; }
void C4JStorage::GetMountedDLCFileList(const char* szMountDrive,
                                       std::vector<std::string>& fileList) {
    fileList.clear();
}
std::string C4JStorage::GetMountedPath(std::string szMount) { return ""; }
C4JStorage::ETMSStatus C4JStorage::ReadTMSFile(
    int iQuadrant, eGlobalStorage eStorageFacility,
    C4JStorage::eTMS_FileType eFileType, WCHAR* pwchFilename, BYTE** ppBuffer,
    DWORD* pdwBufferSize, int (*Func)(void*, WCHAR*, int, bool, int),
    void* lpParam, int iAction) {
    return ETMSStatus_Fail;
}
bool C4JStorage::WriteTMSFile(int iQuadrant, eGlobalStorage eStorageFacility,
                              WCHAR* pwchFilename, BYTE* pBuffer,
                              DWORD dwBufferSize) {
    return false;
}
bool C4JStorage::DeleteTMSFile(int iQuadrant, eGlobalStorage eStorageFacility,
                               WCHAR* pwchFilename) {
    return false;
}
void C4JStorage::StoreTMSPathName(WCHAR* pwchName) {}
C4JStorage::ETMSStatus C4JStorage::TMSPP_ReadFile(
    int iPad, C4JStorage::eGlobalStorage eStorageFacility,
    C4JStorage::eTMS_FILETYPEVAL eFileTypeVal, LPCSTR szFilename,
    int (*Func)(void*, int, int, PTMSPP_FILEDATA, LPCSTR), void* lpParam,
    int iUserData) {
    return ETMSStatus_Fail;
}
unsigned int C4JStorage::CRC(unsigned char* buf, int len) {
    unsigned int crc = 0xFFFFFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= buf[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

int C4JStorage::AddSubfile(int regionIndex) {
    (void)regionIndex;
    return 0;
}
unsigned int C4JStorage::GetSubfileCount() { return 0; }
void C4JStorage::GetSubfileDetails(unsigned int i, int* regionIndex,
                                   void** data, unsigned int* size) {
    (void)i;
    if (regionIndex) *regionIndex = 0;
    if (data) *data = 0;
    if (size) *size = 0;
}
void C4JStorage::ResetSubfiles() {}
void C4JStorage::UpdateSubfile(int index, void* data, unsigned int size) {
    (void)index;
    (void)data;
    (void)size;
}
void C4JStorage::SaveSubfiles(int (*Func)(void*, const bool), void* param) {
    if (Func) Func(param, true);
}
C4JStorage::ESaveGameState C4JStorage::GetSaveState() { return ESaveGame_Idle; }
void C4JStorage::ContinueIncompleteOperation() {}
