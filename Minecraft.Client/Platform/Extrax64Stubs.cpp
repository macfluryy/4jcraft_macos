#include "stdafx.h"
//#include <compressapi.h>

#if defined(_WINDOWS64)
#include "../Platform/Windows64/Sentient/SentientManager.h"
#include "../GameState/StatsCounter.h"
#include "../Platform/Windows64/Social/SocialManager.h"
#include "../Platform/Windows64/Sentient/DynamicConfigurations.h"
#elif defined(__linux__)
// On Linux, stdafx.h already provides Orbis-compatible Sentient/Dynamic headers
// via #pragma once. Pull in SentientManager for CSentientManager class declaration
// and StatsCounter; CSocialManager is provided as inline stubs via Platform/Linux/Social/SocialManager.h.
#include "../Platform/Linux/Sentient/SentientManager.h"
#include "../GameState/StatsCounter.h"
#else
#include "../GameState/StatsCounter.h"
#endif

#if defined(_WINDOWS64)
//C4JStorage StorageManager;
C_4JProfile ProfileManager;
#endif
CSentientManager SentientManager;
#if !defined(__linux__)
// On Linux this global shadows the project's StringTable class name in unity builds
CXuiStringTable StringTable;
#endif

#if !defined(__linux__)
ATG::XMLParser::XMLParser() {}
ATG::XMLParser::~XMLParser() {}
HRESULT    ATG::XMLParser::ParseXMLBuffer( CONST CHAR* strBuffer, UINT uBufferSize ) { return S_OK; }   
void ATG::XMLParser::RegisterSAXCallbackInterface( ISAXCallback *pISAXCallback ) {}
#endif

#if !defined(__linux__)
bool	CSocialManager::IsTitleAllowedToPostAnything() { return false; }
bool	CSocialManager::AreAllUsersAllowedToPostImages() { return false; }
bool	CSocialManager::IsTitleAllowedToPostImages() { return false; }

bool	CSocialManager::PostLinkToSocialNetwork( ESocialNetwork eSocialNetwork, DWORD dwUserIndex, bool bUsingKinect ) { return false; }
bool	CSocialManager::PostImageToSocialNetwork( ESocialNetwork eSocialNetwork, DWORD dwUserIndex, bool bUsingKinect ) { return false; }

CSocialManager *CSocialManager::Instance() { return nullptr; }
void CSocialManager::SetSocialPostText(const wchar_t* Title, const wchar_t* Caption, const wchar_t* Desc) {};
#endif

DWORD XShowPartyUI(DWORD dwUserIndex) { return 0; }
DWORD XShowFriendsUI(DWORD dwUserIndex) { return 0; }
HRESULT XPartyGetUserList(XPARTY_USER_LIST *pUserList) { return S_OK; }
DWORD XContentGetThumbnail(DWORD dwUserIndex, const XCONTENT_DATA *pContentData,  PBYTE pbThumbnail,  PDWORD pcbThumbnail,  PXOVERLAPPED *pOverlapped) { return 0; }
void XShowAchievementsUI(int i) {}
DWORD XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE Mode) { return 0; }

void PIXAddNamedCounter(int a, const char *b, ...) {}
//#define PS3_USE_PIX_EVENTS 
//#define PS4_USE_PIX_EVENTS 
void PIXBeginNamedEvent(int a, const char *b, ...)
{
#if defined(PS4_USE_PIX_EVENTS)
	char buf[512];
    va_list args;
    va_start(args,b);
    vsprintf(buf,b,args);
	sceRazorCpuPushMarker(buf, 0xffffffff, SCE_RAZOR_MARKER_ENABLE_HUD);

#endif
#if defined(PS3_USE_PIX_EVENTS)
	char buf[256];
	wchar_t wbuf[256];
    va_list args;
    va_start(args,b);
    vsprintf(buf,b,args);
	snPushMarker(buf);

// 	mbstowcs(wbuf,buf,256);
// 	RenderManager.BeginEvent(wbuf);
    va_end(args);
#endif
}


void PIXEndNamedEvent()
{
#if defined(PS4_USE_PIX_EVENTS)
	sceRazorCpuPopMarker();
#endif
#if defined(PS3_USE_PIX_EVENTS)
	snPopMarker();
// 	RenderManager.EndEvent();
#endif
}
void PIXSetMarkerDeprecated(int a, char *b, ...) {}

// void *D3DXBUFFER::GetBufferPointer() { return nullptr; }
// int D3DXBUFFER::GetBufferSize() { return 0; }
// void D3DXBUFFER::Release() {}

// #if 0
// void GetLocalTime(SYSTEMTIME *time) {}
// #endif


bool IsEqualXUID(PlayerUID a, PlayerUID b)
{
	return false;
}

void XMemCpy(void *a, const void *b, size_t s) { memcpy(a, b, s); }
void XMemSet(void *a, int t, size_t s) { memset(a, t, s); }
void XMemSet128(void *a, int t, size_t s) { memset(a, t, s); }
void *XPhysicalAlloc(size_t a, ULONG_PTR  b, ULONG_PTR c, DWORD d) { return malloc(a); }
void XPhysicalFree(void *a) { free(a); }

D3DXVECTOR3::D3DXVECTOR3() {}
D3DXVECTOR3::D3DXVECTOR3(float x,float y,float z) : x(x), y(y), z(z) {}
D3DXVECTOR3& D3DXVECTOR3::operator += ( CONST D3DXVECTOR3& add ) { x += add.x; y += add.y; z += add.z; return *this; }

BYTE IQNetPlayer::GetSmallId() { return 0; }
void IQNetPlayer::SendData(IQNetPlayer *player, const void *pvData, DWORD dwDataSize, DWORD dwFlags)
{
#if !defined(__linux__)
	app.DebugPrintf("Sending from 0x%x to 0x%x %d bytes\n",this,player,dwDataSize);
#endif
}
bool IQNetPlayer::IsSameSystem(IQNetPlayer *player) { return true; }
DWORD IQNetPlayer::GetSendQueueSize( IQNetPlayer *player, DWORD dwFlags ) { return 0; }
DWORD IQNetPlayer::GetCurrentRtt() { return 0; }
bool IQNetPlayer::IsHost() { return this == &IQNet::m_player[0]; }
bool IQNetPlayer::IsGuest() { return false; }
bool IQNetPlayer::IsLocal() { return true; }
PlayerUID IQNetPlayer::GetXuid() { return INVALID_XUID; }
const wchar_t* IQNetPlayer::GetGamertag() { static const wchar_t *test = L"stub"; return test; }
int IQNetPlayer::GetSessionIndex() { return 0; }
bool IQNetPlayer::IsTalking() { return false; }
bool IQNetPlayer::IsMutedByLocalUser(DWORD dwUserIndex) { return false; }
bool IQNetPlayer::HasVoice() { return false; }
bool IQNetPlayer::HasCamera() { return false; }
int IQNetPlayer::GetUserIndex() { return this - &IQNet::m_player[0]; }
void IQNetPlayer::SetCustomDataValue(ULONG_PTR ulpCustomDataValue) {
	m_customData = ulpCustomDataValue;
}
ULONG_PTR IQNetPlayer::GetCustomDataValue() {
	return m_customData;
}

IQNetPlayer IQNet::m_player[4];

bool _bQNetStubGameRunning = false;

HRESULT IQNet::AddLocalPlayerByUserIndex(DWORD dwUserIndex){ return S_OK; }
IQNetPlayer *IQNet::GetHostPlayer() { return &m_player[0]; }
IQNetPlayer *IQNet::GetLocalPlayerByUserIndex(DWORD dwUserIndex) { return &m_player[dwUserIndex]; } 
IQNetPlayer *IQNet::GetPlayerByIndex(DWORD dwPlayerIndex) { return &m_player[0]; }
IQNetPlayer *IQNet::GetPlayerBySmallId(BYTE SmallId){ return &m_player[0]; }
IQNetPlayer *IQNet::GetPlayerByXuid(PlayerUID xuid){ return &m_player[0]; }
DWORD IQNet::GetPlayerCount() { return 1; }
QNET_STATE IQNet::GetState() { return _bQNetStubGameRunning ? QNET_STATE_GAME_PLAY : QNET_STATE_IDLE; }
bool IQNet::IsHost() { return true; }
HRESULT IQNet::JoinGameFromInviteInfo(DWORD dwUserIndex, DWORD dwUserMask, const INVITE_INFO *pInviteInfo) { return S_OK; }
void IQNet::HostGame() { _bQNetStubGameRunning = true; }
void IQNet::EndGame() { _bQNetStubGameRunning = false; }

DWORD MinecraftDynamicConfigurations::GetTrialTime() { return DYNAMIC_CONFIG_DEFAULT_TRIAL_TIME; }

void XSetThreadProcessor(HANDLE a, int b) {}
// #if !(0) && !(0)
// bool XCloseHandle(HANDLE a) { return CloseHandle(a); }
// #endif // 0

DWORD XUserGetSigninInfo(
         DWORD dwUserIndex,
         DWORD dwFlags,
         PXUSER_SIGNIN_INFO pSigninInfo
)
{
	return 0;
}

const wchar_t* CXuiStringTable::Lookup(const wchar_t* szId) { return szId; }
const wchar_t* CXuiStringTable::Lookup(UINT nIndex) { return L"String"; }
void CXuiStringTable::Clear() {}
HRESULT CXuiStringTable::Load(const wchar_t* szId) { return S_OK; }

DWORD XUserAreUsersFriends( DWORD dwUserIndex, PPlayerUID pXuids, DWORD dwXuidCount, bool* pfResult, void *pOverlapped) { return 0; }

HRESULT XMemDecompress(
         XMEMDECOMPRESSION_CONTEXT Context,
         void *pDestination,
         size_t *pDestSize,
         void *pSource,
         size_t SrcSize
)
{
	memcpy(pDestination, pSource, SrcSize);
	*pDestSize = SrcSize;
	return S_OK;

	/*
	DECOMPRESSOR_HANDLE Decompressor    = (DECOMPRESSOR_HANDLE)Context;
	if( Decompress(
        Decompressor,           //  Decompressor handle
        (void *)pSource,		//  Compressed data
        SrcSize,				//  Compressed data size
        pDestination,			//  Decompressed buffer
        *pDestSize,				//  Decompressed buffer size
        pDestSize) )				//  Decompressed data size
	{
		return S_OK;
	}
	else
	*/
	{
		return E_FAIL;
	}
}

HRESULT XMemCompress(
         XMEMCOMPRESSION_CONTEXT Context,
         void *pDestination,
         size_t *pDestSize,
         void *pSource,
         size_t SrcSize
)
{
	memcpy(pDestination, pSource, SrcSize);
	*pDestSize = SrcSize;
	return S_OK;

	/*
	COMPRESSOR_HANDLE Compressor    = (COMPRESSOR_HANDLE)Context;
	if( Compress(
			Compressor,                  //  Compressor Handle
			(void *)pSource,             //  Input buffer, Uncompressed data
			SrcSize,					 //  Uncompressed data size
			pDestination,                //  Compressed Buffer
			*pDestSize,                  //  Compressed Buffer size
			pDestSize)	)				//  Compressed Data size
	{
		return S_OK;
	}
	else
	*/
	{
		return E_FAIL;
	}
}

HRESULT XMemCreateCompressionContext(
         XMEMCODEC_TYPE CodecType,
         CONST void *pCodecParams,
         DWORD Flags,
         XMEMCOMPRESSION_CONTEXT *pContext
)
{
	/*
	COMPRESSOR_HANDLE Compressor    = nullptr;

	HRESULT hr = CreateCompressor(
		COMPRESS_ALGORITHM_XPRESS_HUFF, //  Compression Algorithm
		nullptr,                           //  Optional allocation routine
		&Compressor);                   //  Handle

	pContext = (XMEMDECOMPRESSION_CONTEXT *)Compressor;
	return hr;
	*/
	return 0;
}

HRESULT XMemCreateDecompressionContext(
         XMEMCODEC_TYPE CodecType,
         CONST void *pCodecParams,
         DWORD Flags,
         XMEMDECOMPRESSION_CONTEXT *pContext
)
{
	/*
	DECOMPRESSOR_HANDLE  Decompressor    = nullptr;

	HRESULT hr = CreateDecompressor(
		COMPRESS_ALGORITHM_XPRESS_HUFF, //  Compression Algorithm
		nullptr,                           //  Optional allocation routine
		&Decompressor);                   //  Handle

	pContext = (XMEMDECOMPRESSION_CONTEXT *)Decompressor;
	return hr;
	*/
	return 0;
}

void XMemDestroyCompressionContext(XMEMCOMPRESSION_CONTEXT Context)
{
//	COMPRESSOR_HANDLE Compressor    = (COMPRESSOR_HANDLE)Context;
//	CloseCompressor(Compressor);
}

void XMemDestroyDecompressionContext(XMEMDECOMPRESSION_CONTEXT Context)
{
//	DECOMPRESSOR_HANDLE Decompressor    = (DECOMPRESSOR_HANDLE)Context;
//	CloseDecompressor(Decompressor);
}

//#if 1
DWORD XGetLanguage() { return 1; }
DWORD XGetLocale() { return 0; }
DWORD XEnableGuestSignin(bool fEnable) { return 0; }



/////////////////////////////////////////////// Profile library
#if defined(_WINDOWS64)
static void *profileData[4];
static bool s_bProfileIsFullVersion;
void				C_4JProfile::Initialise( std::uint32_t dwTitleID,
								std::uint32_t dwOfferID,
								unsigned short usProfileVersion,
								unsigned int uiProfileValuesC,
								unsigned int uiProfileSettingsC,
								std::uint32_t *pdwProfileSettingsA,
								int iGameDefinedDataSizeX4,
								unsigned int *puiGameDefinedDataChangedBitmask)
{
	for( int i = 0; i < 4; i++ )
	{
		profileData[i] = new uint8_t[iGameDefinedDataSizeX4/4];
		ZeroMemory(profileData[i],sizeof(uint8_t)*iGameDefinedDataSizeX4/4);

		// Set some sane initial values!
		GAME_SETTINGS *pGameSettings = (GAME_SETTINGS *)profileData[i];
		pGameSettings->ucMenuSensitivity=100; //eGameSetting_Sensitivity_InMenu
		pGameSettings->ucInterfaceOpacity=80; //eGameSetting_Sensitivity_InMenu
		pGameSettings->usBitmaskValues|=0x0200; //eGameSetting_DisplaySplitscreenGamertags - on
		pGameSettings->usBitmaskValues|=0x0400; //eGameSetting_Hints - on
		pGameSettings->usBitmaskValues|=0x1000; //eGameSetting_Autosave - 2
		pGameSettings->usBitmaskValues|=0x8000; //eGameSetting_Tooltips - on
		pGameSettings->uiBitmaskValues=0L; // reset
		pGameSettings->uiBitmaskValues|=GAMESETTING_CLOUDS;					//eGameSetting_Clouds - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_ONLINE;					//eGameSetting_GameSetting_Online - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_FRIENDSOFFRIENDS;		//eGameSetting_GameSetting_FriendsOfFriends - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
		pGameSettings->uiBitmaskValues&=~GAMESETTING_BEDROCKFOG;			//eGameSetting_BedrockFog - off
		pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHUD;				//eGameSetting_DisplayHUD - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHAND;			//eGameSetting_DisplayHand - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_CUSTOMSKINANIM;			//eGameSetting_CustomSkinAnim - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_DEATHMESSAGES;			//eGameSetting_DeathMessages - on
		pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE&0x00000800);				// uisize 2
		pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE_SPLITSCREEN&0x00004000);	// splitscreen ui size 3
		pGameSettings->uiBitmaskValues|=GAMESETTING_ANIMATEDCHARACTER;		//eGameSetting_AnimatedCharacter - on

		// TU12
		// favorite skins added, but only set in TU12 - set to FFs
		for(int i=0;i<MAX_FAVORITE_SKINS;i++)
		{
			pGameSettings->uiFavoriteSkinA[i]=0xFFFFFFFF;
		}
		pGameSettings->ucCurrentFavoriteSkinPos=0;
		// Added a bitmask in TU13 to enable/disable display of the Mash-up pack worlds in the saves list
		pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

		// PS3DEC13
		pGameSettings->uiBitmaskValues&=~GAMESETTING_PS3EULAREAD;		//eGameSetting_PS3_EULA_Read - off

		// PS3 1.05 - added Greek
		pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language

		// PS Vita - network mode added
		pGameSettings->uiBitmaskValues&=~GAMESETTING_PSVITANETWORKMODEADHOC;		//eGameSetting_PSVita_NetworkModeAdhoc - off


		// Tutorials for most menus, and a few other things
		pGameSettings->ucTutorialCompletion[0] = 0xFF;
		pGameSettings->ucTutorialCompletion[1] = 0xFF;
		pGameSettings->ucTutorialCompletion[2] = 0xF;

		// Has gone halfway through the tutorial
		pGameSettings->ucTutorialCompletion[28] |= 1<<0;
	}
}
void				C_4JProfile::SetTrialTextStringTable(CXuiStringTable *pStringTable,int iAccept,int iReject) {}
void				C_4JProfile::SetTrialAwardText(eAwardType AwardType,int iTitle,int iText) {}
int					C_4JProfile::GetLockedProfile() { return 0; }
void				C_4JProfile::SetLockedProfile(int iProf) {}
bool				C_4JProfile::IsSignedIn(int iQuadrant) { return ( iQuadrant == 0); }
bool				C_4JProfile::IsSignedInLive(int iProf) { return true; }
bool				C_4JProfile::IsGuest(int iQuadrant) { return false; }
unsigned int		C_4JProfile::RequestSignInUI(bool bFromInvite,bool bLocalGame,bool bNoGuestsAllowed,bool bMultiplayerSignIn,bool bAddUser, int( *Func)(void *,const bool, const int iPad),void *lpParam,int iQuadrant) { return 0; }
unsigned int		C_4JProfile::DisplayOfflineProfile(int( *Func)(void *,const bool, const int iPad),void *lpParam,int iQuadrant)  { return 0; }
unsigned int		C_4JProfile::RequestConvertOfflineToGuestUI(int( *Func)(void *,const bool, const int iPad),void *lpParam,int iQuadrant) { return 0; }
void				C_4JProfile::SetPrimaryPlayerChanged(bool bVal) {}
bool				C_4JProfile::QuerySigninStatus(void) { return true; }
void				C_4JProfile::GetXUID(int iPad, PlayerUID *pXuid,bool bOnlineXuid) {*pXuid = 0xe000d45248242f2e; }
bool				C_4JProfile::AreXUIDSEqual(PlayerUID xuid1,PlayerUID xuid2) { return false; }
bool				C_4JProfile::XUIDIsGuest(PlayerUID xuid) { return false; }
bool				C_4JProfile::AllowedToPlayMultiplayer(int iProf) { return true; }


void				C_4JProfile::StartTrialGame() {}
void				C_4JProfile::AllowedPlayerCreatedContent(int iPad, bool thisQuadrantOnly, bool *allAllowed, bool *friendsAllowed) {}
bool				C_4JProfile::CanViewPlayerCreatedContent(int iPad, bool thisQuadrantOnly, PPlayerUID pXuids, unsigned int xuidCount) { return true; }
bool				C_4JProfile::GetProfileAvatar(int iPad,int( *Func)(void *lpParam,std::uint8_t *thumbnailData,unsigned int thumbnailBytes), void *lpParam) { return false; }
void				C_4JProfile::CancelProfileAvatarRequest() {}
int					C_4JProfile::GetPrimaryPad() { return 0; }
void				C_4JProfile::SetPrimaryPad(int iPad) {}
char*				C_4JProfile::GetGamertag(int iPad){ return "PlayerName"; }
std::wstring				C_4JProfile::GetDisplayName(int iPad){ return L"PlayerName"; }
bool				C_4JProfile::IsFullVersion() { return s_bProfileIsFullVersion; }
void				C_4JProfile::SetSignInChangeCallback(void ( *Func)(void *, bool, unsigned int),void *lpParam) {}
void				C_4JProfile::SetNotificationsCallback(void ( *Func)(void *, std::uint32_t, unsigned int),void *lpParam) {}
bool				C_4JProfile::RegionIsNorthAmerica(void) { return false; }
bool				C_4JProfile::LocaleIsUSorCanada(void) { return false; }
HRESULT				C_4JProfile::GetLiveConnectionStatus() { return S_OK; }
bool				C_4JProfile::IsSystemUIDisplayed() { return false; }
void				C_4JProfile::SetProfileReadErrorCallback(void ( *Func)(void *), void *lpParam) {}
int( *defaultOptionsCallback)(void *,C_4JProfile::PROFILESETTINGS *, const int iPad) = nullptr;
void *lpProfileParam = nullptr;
int					C_4JProfile::SetDefaultOptionsCallback(int( *Func)(void *,PROFILESETTINGS *, const int iPad),void *lpParam)
{
	defaultOptionsCallback = Func;
	lpProfileParam = lpParam;
	return 0;
}
int					C_4JProfile::SetOldProfileVersionCallback(int( *Func)(void *,unsigned char *, const unsigned short,const int),void *lpParam) { return 0; }

// To store the dashboard preferences for controller flipped, etc.
C_4JProfile::PROFILESETTINGS ProfileSettingsA[XUSER_MAX_COUNT];

C_4JProfile::PROFILESETTINGS *	C_4JProfile::GetDashboardProfileSettings(int iPad) { return &ProfileSettingsA[iPad]; }
void				C_4JProfile::WriteToProfile(int iQuadrant, bool bGameDefinedDataChanged, bool bOverride5MinuteLimitOnProfileWrites) {}
void				C_4JProfile::ForceQueuedProfileWrites(int iPad) {}
void				*C_4JProfile::GetGameDefinedProfileData(int iQuadrant)
{
	// 4J Stu - Don't reset the options when we call this!!
	//defaultOptionsCallback(lpProfileParam, (C_4JProfile::PROFILESETTINGS *)profileData[iQuadrant], iQuadrant);
	//pApp->SetDefaultOptions(pSettings,iPad);

	return profileData[iQuadrant];
}
void				C_4JProfile::ResetProfileProcessState() {}
void				C_4JProfile::Tick( void ) {}
void				C_4JProfile::RegisterAward(int iAwardNumber,int iGamerconfigID, eAwardType eType, bool bLeaderboardAffected, 
	CXuiStringTable*pStringTable, int iTitleStr, int iTextStr, int iAcceptStr, char *pszThemeName, unsigned int ulThemeSize) {}
int					C_4JProfile::GetAwardId(int iAwardNumber) { return 0; }
eAwardType			C_4JProfile::GetAwardType(int iAwardNumber) { return eAwardType_Achievement; }
bool				C_4JProfile::CanBeAwarded(int iQuadrant, int iAwardNumber) { return false; }
void				C_4JProfile::Award(int iQuadrant, int iAwardNumber, bool bForce) {}
bool				C_4JProfile::IsAwardsFlagSet(int iQuadrant, int iAward) { return false; }
void				C_4JProfile::RichPresenceInit(int iPresenceCount, int iContextCount) {}
void				C_4JProfile::RegisterRichPresenceContext(int iGameConfigContextID) {}
void				C_4JProfile::SetRichPresenceContextValue(int iPad,int iContextID, int iVal) {}
void				C_4JProfile::SetCurrentGameActivity(int iPad,int iNewPresence, bool bSetOthersToIdle) {}
void				C_4JProfile::DisplayFullVersionPurchase(bool bRequired, int iQuadrant, int iUpsellParam) {}
void				C_4JProfile::SetUpsellCallback(void ( *Func)(void *lpParam, eUpsellType type, eUpsellResponse response, int iUserData),void *lpParam) {}
void				C_4JProfile::SetDebugFullOverride(bool bVal) {s_bProfileIsFullVersion = bVal;}
void				C_4JProfile::ShowProfileCard(int iPad, PlayerUID targetUid) {}

/////////////////////////////////////////////// Storage library
//#ifdef _WINDOWS64
// TODO???
#if defined(__linux__)
C4JStorage::C4JStorage() {}
void								C4JStorage::Tick() {}
C4JStorage::EMessageResult			C4JStorage::RequestMessageBox(unsigned int uiTitle, unsigned int uiText, unsigned int *uiOptionA,unsigned int uiOptionC, unsigned int pad, int( *Func)(void*,int,const C4JStorage::EMessageResult),void* lpParam, C4JStringTable *pStringTable, wchar_t *pwchFormatString,unsigned int focusButton) { return C4JStorage::EMessage_Undefined; }
C4JStorage::EMessageResult			C4JStorage::GetMessageBoxResult()  { return C4JStorage::EMessage_Undefined; }
bool								C4JStorage::SetSaveDevice(int( *Func)(void*,const bool),void* lpParam, bool bForceResetOfSaveDevice) { return true; }
void								C4JStorage::Init(const wchar_t* pwchDefaultSaveName,char *pszSavePackName,int iMinimumSaveSize, int( *Func)(void*, const ESavingMessage, int),void* lpParam) {}
void								C4JStorage::ResetSaveData() {}
void								C4JStorage::SetDefaultSaveNameForKeyboardDisplay(const wchar_t* pwchDefaultSaveName) {}
void								C4JStorage::SetSaveTitle(const wchar_t* pwchDefaultSaveName) {}
const wchar_t*								C4JStorage::GetSaveTitle() { return L""; }
bool								C4JStorage::GetSaveUniqueNumber(INT *piVal) { return true; }
bool								C4JStorage::GetSaveUniqueFilename(char *pszName) { return true; }
void								C4JStorage::SetSaveUniqueFilename(char *szFilename) { }
void								C4JStorage::SetState(ESaveGameControlState eControlState,int( *Func)(void*,const bool),void* lpParam) {}
void								C4JStorage::SetSaveDisabled(bool bDisable) {}
bool								C4JStorage::GetSaveDisabled(void) { return false; }
unsigned int						C4JStorage::GetSaveSize() { return 0; }
void								C4JStorage::GetSaveData(void *pvData,unsigned int *pulBytes) {}
void*								C4JStorage::AllocateSaveData(unsigned int ulBytes) { return new char[ulBytes]; }
void								C4JStorage::SaveSaveData(unsigned int ulBytes,PBYTE pbThumbnail,DWORD cbThumbnail,PBYTE pbTextData, DWORD dwTextLen) {}
void								C4JStorage::CopySaveDataToNewSave(std::uint8_t *pbThumbnail,unsigned int cbThumbnail,wchar_t *wchNewName,int ( *Func)(void* lpParam, bool), void* lpParam) {}
void								C4JStorage::SetSaveDeviceSelected(unsigned int uiPad,bool bSelected) {}
bool								C4JStorage::GetSaveDeviceSelected(unsigned int iPad) { return true; }
C4JStorage::ELoadGameStatus			C4JStorage::DoesSaveExist(bool *pbExists) { return C4JStorage::ELoadGame_Idle; }
bool								C4JStorage::EnoughSpaceForAMinSaveGame() { return true; }
void								C4JStorage::SetSaveMessageVPosition(float fY) {}
//C4JStorage::ESGIStatus				C4JStorage::GetSavesInfo(int iPad,bool ( *Func)(void*, int, CACHEINFOSTRUCT *, int, HRESULT),void* lpParam,char *pszSavePackName) { return C4JStorage::ESGIStatus_Idle; }
C4JStorage::ESaveGameState			C4JStorage::GetSavesInfo(int iPad,int ( *Func)(void* lpParam,SAVE_DETAILS *pSaveDetails,const bool),void* lpParam,char *pszSavePackName) { return C4JStorage::ESaveGame_Idle; }

void								C4JStorage::GetSaveCacheFileInfo(unsigned int fileIndex,XCONTENT_DATA &xContentData) {}
void								C4JStorage::GetSaveCacheFileInfo(unsigned int fileIndex,	std::uint8_t * *ppbImageData, unsigned int *pImageBytes) {}
C4JStorage::ESaveGameState			C4JStorage::LoadSaveData(PSAVE_INFO pSaveInfo,int( *Func)(void* lpParam,const bool, const bool), void* lpParam) {return C4JStorage::ESaveGame_Idle;}
C4JStorage::EDeleteGameStatus		C4JStorage::DeleteSaveData(PSAVE_INFO pSaveInfo,int( *Func)(void* lpParam,const bool), void* lpParam) { return C4JStorage::EDeleteGame_Idle; }
PSAVE_DETAILS						C4JStorage::ReturnSavesInfo() {return nullptr;}

void								C4JStorage::RegisterMarketplaceCountsCallback(int ( *Func)(void* lpParam, C4JStorage::DLC_TMS_DETAILS *, int), void* lpParam ) {}
void								C4JStorage::SetDLCPackageRoot(char *pszDLCRoot) {}
C4JStorage::EDLCStatus				C4JStorage::GetDLCOffers(int iPad,int( *Func)(void *, int, std::uint32_t, int),void *lpParam, std::uint32_t dwOfferTypesBitmaskT) { return C4JStorage::EDLC_Idle; }
unsigned int						C4JStorage::CancelGetDLCOffers() { return 0; }
void								C4JStorage::ClearDLCOffers() {}
XMARKETPLACE_CONTENTOFFER_INFO&		C4JStorage::GetOffer(unsigned int dw) { static XMARKETPLACE_CONTENTOFFER_INFO retval = {0}; return retval; }
int									C4JStorage::GetOfferCount() { return 0; }
unsigned int						C4JStorage::InstallOffer(int iOfferIDC,uint64_t *ullOfferIDA,int( *Func)(void*, int, int),void* lpParam, bool bTrial) { return 0; }
unsigned int						C4JStorage::GetAvailableDLCCount( int iPad) { return 0; }
XCONTENT_DATA&						C4JStorage::GetDLC(unsigned int dw) { static XCONTENT_DATA retval = {0}; return retval; }
C4JStorage::EDLCStatus				C4JStorage::GetInstalledDLC(int iPad,int( *Func)(void*, int, int),void* lpParam) { return C4JStorage::EDLC_Idle; }
std::uint32_t						C4JStorage::MountInstalledDLC(int iPad,std::uint32_t dwDLC,int( *Func)(void *, int, std::uint32_t, std::uint32_t),void *lpParam,const char* szMountDrive) { return 0; }
unsigned int						C4JStorage::UnmountInstalledDLC(const char* szMountDrive) { return 0; }
C4JStorage::ETMSStatus				C4JStorage::ReadTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,C4JStorage::eTMS_FileType eFileType, wchar_t *pwchFilename,std::uint8_t **ppBuffer,unsigned int *pBufferSize,int( *Func)(void*, wchar_t *,int, bool, int),void* lpParam, int iAction) { return C4JStorage::ETMSStatus_Idle; }
bool								C4JStorage::WriteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,wchar_t *pwchFilename,std::uint8_t *pBuffer,unsigned int bufferSize) { return true; }
bool								C4JStorage::DeleteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,wchar_t *pwchFilename) { return true; }
void								C4JStorage::StoreTMSPathName(wchar_t *pwchName) {}
unsigned int						C4JStorage::CRC(unsigned char *buf, int len) { return 0; }

struct PTMSPP_FILEDATA;
C4JStorage::ETMSStatus				C4JStorage::TMSPP_ReadFile(int iPad,C4JStorage::eGlobalStorage eStorageFacility,C4JStorage::eTMS_FILETYPEVAL eFileTypeVal,const char* szFilename,int( *Func)(void*,int,int,PTMSPP_FILEDATA, const char*)/*=nullptr*/,void* lpParam/*=nullptr*/, int iUserData/*=0*/) {return C4JStorage::ETMSStatus_Idle;}
#endif

#endif

/////////////////////////////////////////////////////// Sentient manager

HRESULT CSentientManager::Init() { return S_OK; }
HRESULT CSentientManager::Tick() { return S_OK; }
HRESULT CSentientManager::Flush() { return S_OK; }
bool CSentientManager::RecordPlayerSessionStart(DWORD dwUserId) { return true; }
bool CSentientManager::RecordPlayerSessionExit(DWORD dwUserId, int exitStatus) { return true; }
bool CSentientManager::RecordHeartBeat(DWORD dwUserId) { return true; }
bool CSentientManager::RecordLevelStart(DWORD dwUserId, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, DWORD numberOfLocalPlayers, DWORD numberOfOnlinePlayers) { return true; }
bool CSentientManager::RecordLevelExit(DWORD dwUserId, ESen_LevelExitStatus levelExitStatus) { return true; }
bool CSentientManager::RecordLevelSaveOrCheckpoint(DWORD dwUserId, INT saveOrCheckPointID, INT saveSizeInBytes) { return true; }
bool CSentientManager::RecordLevelResume(DWORD dwUserId, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, DWORD numberOfLocalPlayers, DWORD numberOfOnlinePlayers, INT saveOrCheckPointID)  { return true; }
bool CSentientManager::RecordPauseOrInactive(DWORD dwUserId)  { return true; }
bool CSentientManager::RecordUnpauseOrActive(DWORD dwUserId) { return true; }
bool CSentientManager::RecordMenuShown(DWORD dwUserId, INT menuID, INT optionalMenuSubID) { return true; }
bool CSentientManager::RecordAchievementUnlocked(DWORD dwUserId, INT achievementID, INT achievementGamerscore) { return true; }
bool CSentientManager::RecordMediaShareUpload(DWORD dwUserId, ESen_MediaDestination mediaDestination, ESen_MediaType mediaType) { return true; }
bool CSentientManager::RecordUpsellPresented(DWORD dwUserId, ESen_UpsellID upsellId, INT marketplaceOfferID) { return true; }
bool CSentientManager::RecordUpsellResponded(DWORD dwUserId, ESen_UpsellID upsellId, INT marketplaceOfferID, ESen_UpsellOutcome upsellOutcome) { return true; }
bool CSentientManager::RecordPlayerDiedOrFailed(DWORD dwUserId, INT lowResMapX, INT lowResMapY, INT lowResMapZ, INT mapID, INT playerWeaponID, INT enemyWeaponID, ETelemetryChallenges enemyTypeID) { return true; }
bool CSentientManager::RecordEnemyKilledOrOvercome(DWORD dwUserId, INT lowResMapX, INT lowResMapY, INT lowResMapZ, INT mapID, INT playerWeaponID, INT enemyWeaponID, ETelemetryChallenges enemyTypeID) { return true; }
bool CSentientManager::RecordSkinChanged(DWORD dwUserId, DWORD dwSkinId) { return true; }
bool CSentientManager::RecordBanLevel(DWORD dwUserId) { return true; }
bool CSentientManager::RecordUnBanLevel(DWORD dwUserId) { return true; }
INT CSentientManager::GetMultiplayerInstanceID() { return 0; }
INT CSentientManager::GenerateMultiplayerInstanceId() { return 0; }
void CSentientManager::SetMultiplayerInstanceId(INT value) {}

////////////////////////////////////////////////////////  Stats counter

/*
StatsCounter::StatsCounter() {}
void StatsCounter::award(Stat *stat, unsigned int difficulty, unsigned int count) {}
bool StatsCounter::hasTaken(Achievement *ach) { return true; }
bool StatsCounter::canTake(Achievement *ach) { return true; }
unsigned int StatsCounter::getValue(Stat *stat, unsigned int difficulty) { return 0; }
unsigned int StatsCounter::getTotalValue(Stat *stat) { return 0; }
void StatsCounter::tick(int player) {}
void StatsCounter::parse(void* data) {}
void StatsCounter::clear() {}
void StatsCounter::save(int player, bool force) {}
void StatsCounter::flushLeaderboards() {}
void StatsCounter::saveLeaderboards() {}
void StatsCounter::setupStatBoards() {}
#if defined(_DEBUG)
void StatsCounter::WipeLeaderboards() {}
#endif
*/
