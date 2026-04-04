#include "Storage.h"

#include <stdlib.h>

#include <cstring>
#include <string>
#include <vector>

C4JStorage StorageManager;

static XMARKETPLACE_CONTENTOFFER_INFO s_dummyOffer = {};
static XCONTENT_DATA s_dummyContentData = {};

C4JStorage::C4JStorage() : m_pStringTable(nullptr) {}

void C4JStorage::Tick(void) {}

C4JStorage::EMessageResult C4JStorage::RequestMessageBox(
    unsigned int uiTitle, unsigned int uiText, unsigned int* uiOptionA,
    unsigned int uiOptionC, unsigned int pad,
    std::function<int(int, const C4JStorage::EMessageResult)> callback,
    C4JStringTable* pStringTable, wchar_t* pwchFormatString,
    unsigned int focusButton) {
    return EMessage_ResultAccept;
}

C4JStorage::EMessageResult C4JStorage::GetMessageBoxResult() {
    return EMessage_Undefined;
}

bool C4JStorage::SetSaveDevice(std::function<int(const bool)> callback,
                               bool bForceResetOfSaveDevice) {
    return true;
}

void C4JStorage::Init(unsigned int uiSaveVersion,
                      const wchar_t* pwchDefaultSaveName, char* pszSavePackName,
                      int iMinimumSaveSize,
                      std::function<int(const ESavingMessage, int)> callback,
                      const char* szGroupID) {}
void C4JStorage::ResetSaveData() {}
void C4JStorage::SetDefaultSaveNameForKeyboardDisplay(
    const wchar_t* pwchDefaultSaveName) {}
void C4JStorage::SetSaveTitle(const wchar_t* pwchDefaultSaveName) {}
bool C4JStorage::GetSaveUniqueNumber(int* piVal) {
    if (piVal) *piVal = 0;
    return true;
}
bool C4JStorage::GetSaveUniqueFilename(char* pszName) {
    if (pszName) pszName[0] = '\0';
    return true;
}
void C4JStorage::SetSaveUniqueFilename(char* szFilename) {}
void C4JStorage::SetState(ESaveGameControlState eControlState,
                          std::function<int(const bool)> callback) {}
void C4JStorage::SetSaveDisabled(bool bDisable) {}
bool C4JStorage::GetSaveDisabled(void) { return false; }
unsigned int C4JStorage::GetSaveSize() { return 0; }
void C4JStorage::GetSaveData(void* pvData, unsigned int* puiBytes) {
    if (puiBytes) *puiBytes = 0;
}
void* C4JStorage::AllocateSaveData(unsigned int uiBytes) {
    return malloc(uiBytes);
}
void C4JStorage::SetSaveImages(std::uint8_t* pbThumbnail,
                               unsigned int thumbnailBytes,
                               std::uint8_t* pbImage, unsigned int imageBytes,
                               std::uint8_t* pbTextData,
                               unsigned int textDataBytes) {}
C4JStorage::ESaveGameState C4JStorage::SaveSaveData(
    std::function<int(const bool)> callback) {
    return ESaveGame_Idle;
}
void C4JStorage::CopySaveDataToNewSave(std::uint8_t* pbThumbnail,
                                       unsigned int cbThumbnail,
                                       wchar_t* wchNewName,
                                       std::function<int(bool)> callback) {}
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
    std::function<int(SAVE_DETAILS* pSaveDetails, const bool)> callback,
    char* pszSavePackName) {
    return ESaveGame_Idle;
}
PSAVE_DETAILS C4JStorage::ReturnSavesInfo() { return nullptr; }
void C4JStorage::ClearSavesInfo() {}
C4JStorage::ESaveGameState C4JStorage::LoadSaveDataThumbnail(
    PSAVE_INFO pSaveInfo,
    std::function<int(std::uint8_t* thumbnailData,
                      unsigned int thumbnailBytes)>
        callback) {
    return ESaveGame_Idle;
}
void C4JStorage::GetSaveCacheFileInfo(unsigned int fileIndex,
                                      XCONTENT_DATA& xContentData) {
    memset(&xContentData, 0, sizeof(xContentData));
}
void C4JStorage::GetSaveCacheFileInfo(unsigned int fileIndex,
                                      std::uint8_t** ppbImageData,
                                      unsigned int* pImageBytes) {
    if (ppbImageData) *ppbImageData = nullptr;
    if (pImageBytes) *pImageBytes = 0;
}
C4JStorage::ESaveGameState C4JStorage::LoadSaveData(
    PSAVE_INFO pSaveInfo,
    std::function<int(const bool, const bool)> callback) {
    return ESaveGame_Idle;
}
C4JStorage::ESaveGameState C4JStorage::DeleteSaveData(
    PSAVE_INFO pSaveInfo,
    std::function<int(const bool)> callback) {
    return ESaveGame_Idle;
}
void C4JStorage::RegisterMarketplaceCountsCallback(
    std::function<int(C4JStorage::DLC_TMS_DETAILS*, int)> callback) {}
void C4JStorage::SetDLCPackageRoot(char* pszDLCRoot) {}
C4JStorage::EDLCStatus C4JStorage::GetDLCOffers(
    int iPad, std::function<int(int, std::uint32_t, int)> callback,
    std::uint32_t dwOfferTypesBitmask) {
    return EDLC_NoOffers;
}
unsigned int C4JStorage::CancelGetDLCOffers() { return 0; }
void C4JStorage::ClearDLCOffers() {}
XMARKETPLACE_CONTENTOFFER_INFO& C4JStorage::GetOffer(unsigned int dw) {
    return s_dummyOffer;
}
int C4JStorage::GetOfferCount() { return 0; }
unsigned int C4JStorage::InstallOffer(int iOfferIDC, std::uint64_t* ullOfferIDA,
                                      std::function<int(int, int)> callback,
                                      bool bTrial) {
    return 0;
}
unsigned int C4JStorage::GetAvailableDLCCount(int iPad) { return 0; }
C4JStorage::EDLCStatus C4JStorage::GetInstalledDLC(
    int iPad, std::function<int(int, int)> callback) {
    if (callback) {
        callback(0, iPad);
    }
    return EDLC_NoInstalledDLC;
}
XCONTENT_DATA& C4JStorage::GetDLC(unsigned int dw) {
    return s_dummyContentData;
}
std::uint32_t C4JStorage::MountInstalledDLC(
    int iPad, std::uint32_t dwDLC,
    std::function<int(int, std::uint32_t, std::uint32_t)> callback,
    const char* szMountDrive) {
    return 0;
}
unsigned int C4JStorage::UnmountInstalledDLC(const char* szMountDrive) {
    return 0;
}
void C4JStorage::GetMountedDLCFileList(const char* szMountDrive,
                                       std::vector<std::string>& fileList) {
    fileList.clear();
}
std::string C4JStorage::GetMountedPath(std::string szMount) { return ""; }
C4JStorage::ETMSStatus C4JStorage::ReadTMSFile(
    int iQuadrant, eGlobalStorage eStorageFacility,
    C4JStorage::eTMS_FileType eFileType, wchar_t* pwchFilename,
    std::uint8_t** ppBuffer, unsigned int* pBufferSize,
    std::function<int(wchar_t*, int, bool, int)> callback, int iAction) {
    return ETMSStatus_Fail;
}
bool C4JStorage::WriteTMSFile(int iQuadrant, eGlobalStorage eStorageFacility,
                              wchar_t* pwchFilename, std::uint8_t* pBuffer,
                              unsigned int bufferSize) {
    return false;
}
bool C4JStorage::DeleteTMSFile(int iQuadrant, eGlobalStorage eStorageFacility,
                               wchar_t* pwchFilename) {
    return false;
}
void C4JStorage::StoreTMSPathName(wchar_t* pwchName) {}
C4JStorage::ETMSStatus C4JStorage::TMSPP_ReadFile(
    int iPad, C4JStorage::eGlobalStorage eStorageFacility,
    C4JStorage::eTMS_FILETYPEVAL eFileTypeVal, const char* szFilename,
    std::function<int(int, int, PTMSPP_FILEDATA, const char*)> callback,
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
void C4JStorage::SaveSubfiles(std::function<int(const bool)> callback) {
    if (callback) callback(true);
}
C4JStorage::ESaveGameState C4JStorage::GetSaveState() { return ESaveGame_Idle; }
void C4JStorage::ContinueIncompleteOperation() {}
