#pragma once
#include <vector>
//#include <xtms.h>
class C4JStringTable;

#define MAX_DISPLAYNAME_LENGTH 128 // CELL_SAVEDATA_SYSP_SUBTITLE_SIZE on PS3
#define MAX_DETAILS_LENGTH 128 // CELL_SAVEDATA_SYSP_SUBTITLE_SIZE on PS3
#define MAX_SAVEFILENAME_LENGTH 32 // CELL_SAVEDATA_DIRNAME_SIZE

typedef struct
{
	time_t			modifiedTime;
	unsigned int	dataSize;
	unsigned int	thumbnailSize;
}
CONTAINER_METADATA;

typedef struct  
{
	char UTF8SaveFilename[MAX_SAVEFILENAME_LENGTH];
	char UTF8SaveTitle[MAX_DISPLAYNAME_LENGTH];
	CONTAINER_METADATA metaData;
	uint8_t* thumbnailData;
}
SAVE_INFO,*PSAVE_INFO;

typedef struct  
{
	int iSaveC;
	PSAVE_INFO SaveInfoA;
}
SAVE_DETAILS,*PSAVE_DETAILS;

typedef std::vector <PXMARKETPLACE_CONTENTOFFER_INFO> OfferDataArray;
typedef std::vector <PXCONTENT_DATA> XContentDataArray;
//typedef std::vector <PSAVE_DETAILS> SaveDetailsArray;

// Current version of the dlc data creator
#define CURRENT_DLC_VERSION_NUM 3

class C4JStorage
{

public:
	// Structs defined in the DLC_Creator, but added here to be used in the app
	typedef struct  
	{
		unsigned int	uiFileSize;
		uint32_t			dwType;
		uint32_t			dwWchCount; // count of wchar_t in next array
		wchar_t			wchFile[1];
	}
	DLC_FILE_DETAILS, *PDLC_FILE_DETAILS;

	typedef struct
	{
		uint32_t	dwType;
		uint32_t	dwWchCount; // count of wchar_t in next array;
		wchar_t	wchData[1]; // will be an array of size dwBytes
	}
	DLC_FILE_PARAM, *PDLC_FILE_PARAM;
	// End of DLC_Creator structs

	typedef struct  
	{
 		wchar_t                               wchDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
 		char                                szFileName[XCONTENT_MAX_FILENAME_LENGTH];
		uint32_t								dwImageOffset;
		uint32_t								dwImageBytes;
	}
	CACHEINFOSTRUCT;

	// structure to hold DLC info in TMS
	typedef struct  
	{
		uint32_t dwVersion;
		uint32_t dwNewOffers;
		uint32_t dwTotalOffers;
		uint32_t dwInstalledTotalOffers;
		uint8_t bPadding[1024-sizeof(uint32_t)*4]; // future expansion
	}
	DLC_TMS_DETAILS;

	enum eGTS_FileTypes
	{
		eGTS_Type_Skin=0,
		eGTS_Type_Cape,
		eGTS_Type_MAX
	};

	enum eGlobalStorage
	{
		//eGlobalStorage_GameClip=0,
		eGlobalStorage_Title=0,
		eGlobalStorage_TitleUser,
		eGlobalStorage_Max
	};

	enum EMessageResult
	{
		EMessage_Undefined=0,
		EMessage_Busy,
		EMessage_Pending,
		EMessage_Cancelled,
		EMessage_ResultAccept,
		EMessage_ResultDecline,
		EMessage_ResultThirdOption,
		EMessage_ResultFourthOption
	};

	enum ESaveGameControlState
	{
		ESaveGameControl_Idle=0,
		ESaveGameControl_Save,	
		ESaveGameControl_InternalRequestingDevice,
		ESaveGameControl_InternalGetSaveName,
		ESaveGameControl_InternalSaving,
		ESaveGameControl_CopySave,
		ESaveGameControl_CopyingSave,
	};

	enum ESaveGameState
	{
		ESaveGame_Idle=0,
		ESaveGame_Save,	
		ESaveGame_InternalRequestingDevice,
		ESaveGame_InternalGetSaveName,
		ESaveGame_InternalSaving,
		ESaveGame_CopySave,
		ESaveGame_CopyingSave,
		ESaveGame_Load,	
		ESaveGame_GetSavesInfo,	
		ESaveGame_Rename,	
		ESaveGame_Delete,	

		ESaveGame_GetSaveThumbnail		// Not used as an actual state in the PS4, but the game expects this to be returned to indicate success when getting a thumbnail

	};
	enum ELoadGameStatus
	{
		ELoadGame_Idle=0,
		ELoadGame_InProgress,
		ELoadGame_NoSaves,
		ELoadGame_ChangedDevice,
		ELoadGame_DeviceRemoved
	};

	enum EDeleteGameStatus
	{
		EDeleteGame_Idle=0,
		EDeleteGame_InProgress,
	};


	enum ESGIStatus
	{
		ESGIStatus_Error=0,
		ESGIStatus_Idle,
		ESGIStatus_ReadInProgress,
		ESGIStatus_NoSaves,
	};

	enum EDLCStatus
	{
		EDLC_Error=0,
		EDLC_Idle,
		EDLC_NoOffers,
		EDLC_AlreadyEnumeratedAllOffers,
		EDLC_NoInstalledDLC,
		EDLC_Pending,
		EDLC_LoadInProgress,
		EDLC_Loaded,
		EDLC_ChangedDevice
	};

	enum ESavingMessage
	{
		ESavingMessage_None=0,
		ESavingMessage_Short,
		ESavingMessage_Long
	};

	enum ETMSStatus
	{
		ETMSStatus_Idle=0,
		ETMSStatus_Fail,
		ETMSStatus_Fail_ReadInProgress,
		ETMSStatus_Fail_WriteInProgress,
		ETMSStatus_Pending,
	};

	enum eTMS_FileType
	{
		eTMS_FileType_Normal=0,
		eTMS_FileType_Graphic,
	};

	enum eTMS_FILETYPEVAL
	{
		TMS_FILETYPE_BINARY,
		TMS_FILETYPE_CONFIG,
		TMS_FILETYPE_JSON,
		TMS_FILETYPE_MAX
	};
	enum eTMS_UGCTYPE
	{
		TMS_UGCTYPE_NONE,
		TMS_UGCTYPE_IMAGE,
		TMS_UGCTYPE_MAX
	};


	typedef struct  
	{
		char			szFilename[256];
		int				iFileSize;
		eTMS_FILETYPEVAL	eFileTypeVal;
	}
	TMSPP_FILE_DETAILS, *PTMSPP_FILE_DETAILS;

	typedef struct  
	{
		int iCount;
		PTMSPP_FILE_DETAILS FileDetailsA;
	}
	TMSPP_FILE_LIST, *PTMSPP_FILE_LIST;

	typedef struct  
	{
		uint32_t dwSize;
		uint8_t* pbData;
	}
	TMSPP_FILEDATA, *PTMSPP_FILEDATA;


	C4JStorage();

	void								Tick(void);

	// Messages
	C4JStorage::EMessageResult			RequestMessageBox(uint32_t uiTitle, uint32_t uiText, uint32_t *uiOptionA,uint32_t uiOptionC, uint32_t dwPad=XUSER_INDEX_ANY,
	int( *Func)(void*,int,const C4JStorage::EMessageResult)=nullptr,void* lpParam=nullptr, C4JStringTable *pStringTable=nullptr, wchar_t *pwchFormatString=nullptr,uint32_t dwFocusButton=0);


	C4JStorage::EMessageResult			GetMessageBoxResult();

	// save device
	bool								SetSaveDevice(int( *Func)(void*,const bool),void* lpParam, bool bForceResetOfSaveDevice=false);

	// savegame
	void						Init(unsigned int uiSaveVersion,const wchar_t* pwchDefaultSaveName,char *pszSavePackName,int iMinimumSaveSize,int( *Func)(void*, const ESavingMessage, int),void* lpParam,const char* szGroupID);
	void						ResetSaveData(); // Call before a new save to clear out stored save file name
	void						SetDefaultSaveNameForKeyboardDisplay(const wchar_t* pwchDefaultSaveName);
	void						SetSaveTitle(const wchar_t* pwchDefaultSaveName);
	bool						GetSaveUniqueNumber(int32_t *piVal);
	bool						GetSaveUniqueFilename(char *pszName);
	void						SetSaveUniqueFilename(char *szFilename);
	void						SetState(ESaveGameControlState eControlState,int( *Func)(void*,const bool),void* lpParam);
	void						SetSaveDisabled(bool bDisable);
	bool						GetSaveDisabled(void);
	unsigned int				GetSaveSize();
	void						GetSaveData(void *pvData,unsigned int *puiBytes);
	void*						AllocateSaveData(unsigned int uiBytes);
	void						SetSaveImages( uint8_t* pbThumbnail,uint32_t dwThumbnailBytes,uint8_t* pbImage,uint32_t dwImageBytes, uint8_t* pbTextData ,uint32_t dwTextDataBytes);					// Sets the thumbnail & image for the save, optionally setting the metadata in the png
	C4JStorage::ESaveGameState	SaveSaveData(int( *Func)(void* ,const bool),void* lpParam);
	void						CopySaveDataToNewSave(uint8_t* pbThumbnail,uint32_t cbThumbnail,wchar_t *wchNewName,int ( *Func)(void* lpParam, bool), void* lpParam);
	void						SetSaveDeviceSelected(unsigned int uiPad,bool bSelected);	
	bool						GetSaveDeviceSelected(unsigned int iPad);
	C4JStorage::ESaveGameState	DoesSaveExist(bool *pbExists);
	bool						EnoughSpaceForAMinSaveGame();

	void								SetSaveMessageVPosition(float fY); // The 'Saving' message will display at a default position unless changed
	// Get the info for the saves
	C4JStorage::ESaveGameState	GetSavesInfo(int iPad,int ( *Func)(void* lpParam,SAVE_DETAILS *pSaveDetails,const bool),void* lpParam,char *pszSavePackName);
	PSAVE_DETAILS				ReturnSavesInfo();
	void						ClearSavesInfo();	// Clears results
	C4JStorage::ESaveGameState	LoadSaveDataThumbnail(PSAVE_INFO pSaveInfo,int( *Func)(void* lpParam,uint8_t* pbThumbnail,uint32_t dwThumbnailBytes), void* lpParam);	// Get the thumbnail for an individual save referenced by pSaveInfo

	void								GetSaveCacheFileInfo(uint32_t dwFile,XCONTENT_DATA &xContentData);
	void								GetSaveCacheFileInfo(uint32_t dwFile,	uint8_t* *ppbImageData, uint32_t *pdwImageBytes);

	// Load the save. Need to call GetSaveData once the callback is called
	C4JStorage::ESaveGameState			LoadSaveData(PSAVE_INFO pSaveInfo,int( *Func)(void* lpParam,const bool, const bool), void* lpParam);
	C4JStorage::ESaveGameState		DeleteSaveData(PSAVE_INFO pSaveInfo,int( *Func)(void* lpParam,const bool), void* lpParam);

	// DLC
	void								RegisterMarketplaceCountsCallback(int ( *Func)(void* lpParam, C4JStorage::DLC_TMS_DETAILS *, int), void* lpParam );
	void								SetDLCPackageRoot(char *pszDLCRoot);
	C4JStorage::EDLCStatus				GetDLCOffers(int iPad,int( *Func)(void*, int, uint32_t, int),void* lpParam, uint32_t dwOfferTypesBitmask=XMARKETPLACE_OFFERING_TYPE_CONTENT);	
	uint32_t								CancelGetDLCOffers();
	void								ClearDLCOffers();
	XMARKETPLACE_CONTENTOFFER_INFO&		GetOffer(uint32_t dw);
	int									GetOfferCount();
	uint32_t								InstallOffer(int iOfferIDC, uint64_t *ullOfferIDA,int( *Func)(void*, int, int),void* lpParam, bool bTrial=false);
	uint32_t								GetAvailableDLCCount( int iPad);

	C4JStorage::EDLCStatus				GetInstalledDLC(int iPad,int( *Func)(void*, int, int),void* lpParam);
	XCONTENT_DATA&						GetDLC(uint32_t dw);
	uint32_t								MountInstalledDLC(int iPad,uint32_t dwDLC,int( *Func)(void*, int, uint32_t,uint32_t),void* lpParam,const char* szMountDrive=nullptr);
	uint32_t								UnmountInstalledDLC(const char* szMountDrive = nullptr);
	void								GetMountedDLCFileList(const char* szMountDrive, std::vector<std::string>& fileList);
	std::string							GetMountedPath(std::string szMount);

	// Global title storage
	C4JStorage::ETMSStatus				ReadTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,C4JStorage::eTMS_FileType eFileType,

										wchar_t *pwchFilename,uint8_t **ppBuffer,uint32_t *pdwBufferSize,int( *Func)(void*, wchar_t *,int, bool, int)=nullptr,void* lpParam=nullptr, int iAction=0);
	bool								WriteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,wchar_t *pwchFilename,uint8_t *pBuffer,uint32_t dwBufferSize);
	bool								DeleteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,wchar_t *pwchFilename);
	void								StoreTMSPathName(wchar_t *pwchName=NULL);

	// TMS++

	// 	C4JStorage::ETMSStatus				TMSPP_WriteFile(int iPad,C4JStorage::eGlobalStorage eStorageFacility,C4JStorage::eTMS_FILETYPEVAL eFileTypeVal,C4JStorage::eTMS_UGCTYPE eUGCType,char *pchFilePath,char *pchBuffer,uint32_t dwBufferSize,int( *Func)(void*,int,int)=nullptr,void* lpParam=nullptr, int iUserData=0);
	// 	C4JStorage::ETMSStatus				TMSPP_GetUserQuotaInfo(int iPad,TMSCLIENT_CALLBACK Func,void* lpParam, int iUserData=0);
	C4JStorage::ETMSStatus				TMSPP_ReadFile(int iPad,C4JStorage::eGlobalStorage eStorageFacility,C4JStorage::eTMS_FILETYPEVAL eFileTypeVal,const char* szFilename,int( *Func)(void*,int,int,PTMSPP_FILEDATA, const char*)=nullptr,void* lpParam=nullptr, int iUserData=0);
	// 	C4JStorage::ETMSStatus				TMSPP_ReadFileList(int iPad,C4JStorage::eGlobalStorage eStorageFacility,char *pchFilePath,int( *Func)(void*,int,int,PTMSPP_FILE_LIST)=nullptr,void* lpParam=nullptr, int iUserData=0);
	// 	C4JStorage::ETMSStatus				TMSPP_DeleteFile(int iPad,const char* szFilePath,C4JStorage::eTMS_FILETYPEVAL eFileTypeVal,int( *Func)(void*,int,int),void* lpParam=nullptr, int iUserData=0);
	// 	bool								TMSPP_InFileList(eGlobalStorage eStorageFacility, int iPad,const std::wstring &Filename);
	// 	unsigned int						CRC(unsigned char *buf, int len);

// 	enum eXBLWS
// 	{
// 		eXBLWS_GET,
// 		eXBLWS_POST,
// 		eXBLWS_PUT,
// 		eXBLWS_DELETE,
// 	};
	//bool								XBLWS_Command(eXBLWS eCommand);


	unsigned int						CRC(unsigned char *buf, int len);

// #ifdef _DEBUG
// 	void SetSaveName(int i);					
// #endif
	// string table for all the Storage problems. Loaded by the application
	C4JStringTable				*m_pStringTable;
};

extern C4JStorage StorageManager;
