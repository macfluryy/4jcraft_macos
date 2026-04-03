#pragma once

#include <cstdint>
#include <ctime>
#include <functional>
#include <string>
#include <vector>
// #include <xtms.h>

#include "../PlatformTypes.h"
#include "../IPlatformStorage.h"

class C4JStringTable;

#define MAX_DISPLAYNAME_LENGTH 128  // CELL_SAVEDATA_SYSP_SUBTITLE_SIZE on PS3
#define MAX_DETAILS_LENGTH 128      // CELL_SAVEDATA_SYSP_SUBTITLE_SIZE on PS3
#define MAX_SAVEFILENAME_LENGTH 32  // CELL_SAVEDATA_DIRNAME_SIZE

struct CONTAINER_METADATA {
    time_t modifiedTime;
    unsigned int dataSize;
    unsigned int thumbnailSize;
};

struct SAVE_INFO {
    char UTF8SaveFilename[MAX_SAVEFILENAME_LENGTH];
    char UTF8SaveTitle[MAX_DISPLAYNAME_LENGTH];
    CONTAINER_METADATA metaData;
    std::uint8_t* thumbnailData;
};
using PSAVE_INFO = SAVE_INFO*;

struct SAVE_DETAILS {
    int iSaveC;
    PSAVE_INFO SaveInfoA;
};
using PSAVE_DETAILS = SAVE_DETAILS*;

typedef std::vector<PXMARKETPLACE_CONTENTOFFER_INFO> OfferDataArray;
typedef std::vector<PXCONTENT_DATA> XContentDataArray;
// typedef std::vector <PSAVE_DETAILS> SaveDetailsArray;

// Current version of the dlc data creator
#define CURRENT_DLC_VERSION_NUM 3

class C4JStorage : public IPlatformStorage {
public:
    struct DLC_FILE_DETAILS {
        unsigned int uiFileSize;
        std::uint32_t dwType;
        std::uint32_t dwWchCount;
        wchar_t wchFile[1];
    };
    using PDLC_FILE_DETAILS = DLC_FILE_DETAILS*;

    struct DLC_FILE_PARAM {
        std::uint32_t dwType;
        std::uint32_t dwWchCount;
        wchar_t wchData[1];
    };
    using PDLC_FILE_PARAM = DLC_FILE_PARAM*;

    struct CACHEINFOSTRUCT {
        wchar_t wchDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
        char szFileName[XCONTENT_MAX_FILENAME_LENGTH];
        std::uint32_t dwImageOffset;
        std::uint32_t dwImageBytes;
    };

    enum eGTS_FileTypes { eGTS_Type_Skin = 0, eGTS_Type_Cape, eGTS_Type_MAX };

    enum ELoadGameStatus {
        ELoadGame_Idle = 0,
        ELoadGame_InProgress,
        ELoadGame_NoSaves,
        ELoadGame_ChangedDevice,
        ELoadGame_DeviceRemoved
    };

    enum EDeleteGameStatus {
        EDeleteGame_Idle = 0,
        EDeleteGame_InProgress,
    };

    enum ESGIStatus {
        ESGIStatus_Error = 0,
        ESGIStatus_Idle,
        ESGIStatus_ReadInProgress,
        ESGIStatus_NoSaves,
    };

    enum eTMS_UGCTYPE { TMS_UGCTYPE_NONE, TMS_UGCTYPE_IMAGE, TMS_UGCTYPE_MAX };

    struct TMSPP_FILE_DETAILS {
        char szFilename[256];
        int iFileSize;
        eTMS_FILETYPEVAL eFileTypeVal;
    };
    using PTMSPP_FILE_DETAILS = TMSPP_FILE_DETAILS*;

    struct TMSPP_FILE_LIST {
        int iCount;
        PTMSPP_FILE_DETAILS FileDetailsA;
    };
    using PTMSPP_FILE_LIST = TMSPP_FILE_LIST*;

    C4JStorage();

    void Tick(void);

    // Messages
    C4JStorage::EMessageResult RequestMessageBox(
        unsigned int uiTitle, unsigned int uiText, unsigned int* uiOptionA,
        unsigned int uiOptionC, unsigned int pad = XUSER_INDEX_ANY,
        std::function<int(int, const C4JStorage::EMessageResult)> callback =
            nullptr,
        C4JStringTable* pStringTable = nullptr,
        wchar_t* pwchFormatString = nullptr, unsigned int focusButton = 0);

    C4JStorage::EMessageResult GetMessageBoxResult();

    // save device
    bool SetSaveDevice(std::function<int(const bool)> callback,
                       bool bForceResetOfSaveDevice = false);

    // savegame
    void Init(unsigned int uiSaveVersion, const wchar_t* pwchDefaultSaveName,
              char* pszSavePackName, int iMinimumSaveSize,
              std::function<int(const ESavingMessage, int)> callback,
              const char* szGroupID);
    void ResetSaveData();  // Call before a new save to clear out stored save
                           // file name
    void SetDefaultSaveNameForKeyboardDisplay(
        const wchar_t* pwchDefaultSaveName);
    void SetSaveTitle(const wchar_t* pwchDefaultSaveName);
    bool GetSaveUniqueNumber(int* piVal);
    bool GetSaveUniqueFilename(char* pszName);
    void SetSaveUniqueFilename(char* szFilename);
    void SetState(ESaveGameControlState eControlState,
                  std::function<int(const bool)> callback);
    void SetSaveDisabled(bool bDisable);
    bool GetSaveDisabled(void);
    unsigned int GetSaveSize();
    void GetSaveData(void* pvData, unsigned int* puiBytes);
    void* AllocateSaveData(unsigned int uiBytes);
    void SetSaveImages(
        std::uint8_t* pbThumbnail, unsigned int thumbnailBytes,
        std::uint8_t* pbImage, unsigned int imageBytes,
        std::uint8_t* pbTextData,
        unsigned int textDataBytes);  // Sets the thumbnail & image for the
                                      // save, optionally setting the
                                      // metadata in the png
    C4JStorage::ESaveGameState SaveSaveData(
        std::function<int(const bool)> callback);
    void CopySaveDataToNewSave(std::uint8_t* pbThumbnail,
                               unsigned int cbThumbnail, wchar_t* wchNewName,
                               std::function<int(bool)> callback);
    void SetSaveDeviceSelected(unsigned int uiPad, bool bSelected);
    bool GetSaveDeviceSelected(unsigned int iPad);
    C4JStorage::ESaveGameState DoesSaveExist(bool* pbExists);
    bool EnoughSpaceForAMinSaveGame();

    void SetSaveMessageVPosition(
        float fY);  // The 'Saving' message will display at a default position
                    // unless changed
    // Get the info for the saves
    C4JStorage::ESaveGameState GetSavesInfo(
        int iPad,
        std::function<int(SAVE_DETAILS* pSaveDetails, const bool)> callback,
        char* pszSavePackName);
    PSAVE_DETAILS ReturnSavesInfo();
    void ClearSavesInfo();  // Clears results
    C4JStorage::ESaveGameState LoadSaveDataThumbnail(
        PSAVE_INFO pSaveInfo,
        std::function<int(std::uint8_t* thumbnailData,
                          unsigned int thumbnailBytes)>
            callback);  // Get the thumbnail for an individual save referenced
                        // by pSaveInfo

    void GetSaveCacheFileInfo(unsigned int fileIndex,
                              XCONTENT_DATA& xContentData);
    void GetSaveCacheFileInfo(unsigned int fileIndex,
                              std::uint8_t** ppbImageData,
                              unsigned int* pImageBytes);

    // Load the save. Need to call GetSaveData once the callback is called
    C4JStorage::ESaveGameState LoadSaveData(
        PSAVE_INFO pSaveInfo,
        std::function<int(const bool, const bool)> callback);
    C4JStorage::ESaveGameState DeleteSaveData(
        PSAVE_INFO pSaveInfo,
        std::function<int(const bool)> callback);

    // DLC
    void RegisterMarketplaceCountsCallback(
        std::function<int(C4JStorage::DLC_TMS_DETAILS*, int)> callback);
    void SetDLCPackageRoot(char* pszDLCRoot);
    C4JStorage::EDLCStatus GetDLCOffers(
        int iPad, std::function<int(int, std::uint32_t, int)> callback,
        std::uint32_t dwOfferTypesBitmask = XMARKETPLACE_OFFERING_TYPE_CONTENT);
    unsigned int CancelGetDLCOffers();
    void ClearDLCOffers();
    XMARKETPLACE_CONTENTOFFER_INFO& GetOffer(unsigned int dw);
    int GetOfferCount();
    unsigned int InstallOffer(int iOfferIDC, std::uint64_t* ullOfferIDA,
                              std::function<int(int, int)> callback,
                              bool bTrial = false);
    unsigned int GetAvailableDLCCount(int iPad);

    C4JStorage::EDLCStatus GetInstalledDLC(
        int iPad, std::function<int(int, int)> callback);
    XCONTENT_DATA& GetDLC(unsigned int dw);
    std::uint32_t MountInstalledDLC(
        int iPad, std::uint32_t dwDLC,
        std::function<int(int, std::uint32_t, std::uint32_t)> callback,
        const char* szMountDrive = nullptr);
    unsigned int UnmountInstalledDLC(const char* szMountDrive = nullptr);
    void GetMountedDLCFileList(const char* szMountDrive,
                               std::vector<std::string>& fileList);
    std::string GetMountedPath(std::string szMount);

    // Global title storage
    C4JStorage::ETMSStatus ReadTMSFile(
        int iQuadrant, eGlobalStorage eStorageFacility,
        C4JStorage::eTMS_FileType eFileType, wchar_t* pwchFilename,
        std::uint8_t** ppBuffer, unsigned int* pBufferSize,
        std::function<int(wchar_t*, int, bool, int)> callback = nullptr,
        int iAction = 0);
    bool WriteTMSFile(int iQuadrant, eGlobalStorage eStorageFacility,
                      wchar_t* pwchFilename, std::uint8_t* pBuffer,
                      unsigned int bufferSize);
    bool DeleteTMSFile(int iQuadrant, eGlobalStorage eStorageFacility,
                       wchar_t* pwchFilename);
    void StoreTMSPathName(wchar_t* pwchName = nullptr);

    // TMS++
#ifdef _XBOX
    C4JStorage::ETMSStatus WriteTMSFile(
        int iPad, C4JStorage::eGlobalStorage eStorageFacility,
        C4JStorage::eTMS_FileType eFileType, char* pchFilePath, char* pchBuffer,
        unsigned int bufferSize, TMSCLIENT_CALLBACK Func, void* lpParam);
    int GetUserQuotaInfo(int iPad, TMSCLIENT_CALLBACK Func, void* lpParam);
#endif

    // Older TMS++ write/quota entry points were kept in platform-specific
    // implementations and are intentionally not part of this shared API.
    C4JStorage::ETMSStatus TMSPP_ReadFile(
        int iPad, C4JStorage::eGlobalStorage eStorageFacility,
        C4JStorage::eTMS_FILETYPEVAL eFileTypeVal, const char* szFilename,
        std::function<int(int, int, PTMSPP_FILEDATA, const char*)> callback =
            nullptr,
        int iUserData = 0);
    // Older TMS++ list/delete helpers stayed platform-specific. The shared
    // surface keeps the read path plus CRC/subfile helpers below.

    // 	enum eXBLWS
    // 	{
    // 		eXBLWS_GET,
    // 		eXBLWS_POST,
    // 		eXBLWS_PUT,
    // 		eXBLWS_DELETE,
    // 	};
    // bool
    // XBLWS_Command(eXBLWS eCommand);

    unsigned int CRC(unsigned char* buf, int len);

    int AddSubfile(int regionIndex);
    unsigned int GetSubfileCount();
    void GetSubfileDetails(unsigned int i, int* regionIndex, void** data,
                           unsigned int* size);
    void ResetSubfiles();
    void UpdateSubfile(int index, void* data, unsigned int size);
    void SaveSubfiles(std::function<int(const bool)> callback);
    ESaveGameState GetSaveState();

    void ContinueIncompleteOperation();

    C4JStringTable* m_pStringTable;
};

extern C4JStorage StorageManager;
