#pragma once

#include <cstdint>

#include "platform/sdl2/Storage.h"
#include "app/common/App_Defines.h"
#include "app/common/App_enums.h"
#include "app/common/src/Tutorial/TutorialEnum.h"
#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/include/NetTypes.h"
#include "app/include/SkinBox.h"
#include "app/include/XboxStubs.h"

typedef struct {
    wchar_t* wchFilename;
    eFileExtensionType eEXT;
    eTMSFileType eTMSType;
    std::uint8_t* pbData;
    unsigned int uiSize;
    int iConfig;  
} TMS_FILE;

typedef struct {
    std::uint8_t* pbData;
    unsigned int byteCount;
    std::uint8_t ucRefCount;
} MEMDATA, *PMEMDATA;

typedef struct {
    unsigned int dwNotification;
    unsigned int uiParam;
} NOTIFICATION, *PNOTIFICATION;

typedef struct {
    bool bSettingsChanged;
    unsigned char ucMusicVolume;
    unsigned char ucSoundFXVolume;
    unsigned char ucSensitivity;
    unsigned char ucGamma;
    unsigned char ucPad01;           
    unsigned short usBitmaskValues;  
    
    
    
    
    
    

    
    
    
    
    

    
    unsigned int uiDebugBitmask;

    
    
    
    union {
        struct {
            unsigned char ucTutorialCompletion[TUTORIAL_PROFILE_STORAGE_BYTES];
            

            
            
            std::uint32_t dwSelectedSkin;

            
            unsigned char ucMenuSensitivity;
            unsigned char ucInterfaceOpacity;
            unsigned char ucPad02;  
            unsigned char usPad03;

            
            unsigned int
                uiBitmaskValues;  
            
            
            
            

            
            
            
            
            
            
            

            
            

            
            
            unsigned int uiSpecialTutorialBitmask;

            
            std::uint32_t dwSelectedCape;

            unsigned int uiFavoriteSkinA[MAX_FAVORITE_SKINS];
            unsigned char ucCurrentFavoriteSkinPos;

            
            unsigned int
                uiMashUpPackWorldsDisplay;  
                                            
                                            

            
            unsigned char ucLanguage;

            
            unsigned char ucLocale;

            
            
            
            
            
            
        };

        unsigned char ucReservedSpace[192];
    };
} GAME_SETTINGS;

typedef struct {
    PlayerUID xuid;
    char pszLevelName[14];
} BANNEDLISTDATA, *PBANNEDLISTDATA;

typedef std::vector<PBANNEDLISTDATA> VBANNEDLIST;

typedef struct {
    int iPad;
    eXuiAction action;
} XuiActionParam;


typedef struct {
    int iSortValue;
    int uiStringID;
} TIPSTRUCT;

typedef struct {
    eXUID eXuid;
    wchar_t wchCape[MAX_CAPENAME_SIZE];
    wchar_t wchSkin[MAX_CAPENAME_SIZE];
} MOJANG_DATA;

typedef struct {
    eDLCContentType eDLCType;

    uint64_t ullOfferID_Full;
    uint64_t ullOfferID_Trial;
    wchar_t wchBanner[MAX_BANNERNAME_SIZE];
    wchar_t wchDataFile[MAX_BANNERNAME_SIZE];
    int iGender;
    int iConfig;
    unsigned int uiSortIndex;
} DLC_INFO;

typedef struct {
    int x, z;
    _eTerrainFeatureType eTerrainFeature;
} FEATURE_DATA;


typedef struct {
    std::uint8_t* pBannedList;
    unsigned int byteCount;
} BANNEDLIST;

typedef struct _DLCRequest {
    std::uint32_t dwType;
    eDLCContentState eState;
} DLCRequest;

typedef struct _TMSPPRequest {
    eTMSContentState eState;
    eDLCContentType eType;
    C4JStorage::eGlobalStorage eStorageFacility;
    C4JStorage::eTMS_FILETYPEVAL eFileTypeVal;
    
    int (*CallbackFunc)(void*, int, int, C4JStorage::PTMSPP_FILEDATA,
                        const char* szFilename);
    wchar_t wchFilename[MAX_TMSFILENAME_SIZE];

    void* lpCallbackParam;
} TMSPPRequest;

typedef std::pair<EUIScene, HXUIOBJ> SceneStackPair;
