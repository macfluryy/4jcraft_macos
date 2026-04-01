

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "4J.Common/4J_Compat.h"
#include "Minecraft.Client/Linux/Stubs/winapi_stubs.h"
#include "Minecraft.Client/include/NetTypes.h"
#include "Minecraft.Client/include/XboxStubs.h"

class INVITE_INFO;
#if defined(_WINDOWS64)
// C4JStorage StorageManager;
C_4JProfile ProfileManager;
#endif

#if !defined(__linux__)
// On Linux this global shadows the project's StringTable class name in unity
// builds
CXuiStringTable StringTable;
#endif

#if !defined(__linux__)
ATG::XMLParser::XMLParser() {}
ATG::XMLParser::~XMLParser() {}
int32_t ATG::XMLParser::ParseXMLBuffer(const char* strBuffer,
                                       uint32_t uBufferSize) {
    return 0;
}
void ATG::XMLParser::RegisterSAXCallbackInterface(ISAXCallback* pISAXCallback) {
}
#endif

int32_t XPartyGetUserList(void* pUserList) { return 0; }

bool IsEqualXUID(PlayerUID a, PlayerUID b) { return false; }

uint8_t IQNetPlayer::GetSmallId() { return 0; }
void IQNetPlayer::SendData(IQNetPlayer* player, const void* pvData,
                           uint32_t dwDataSize, uint32_t dwFlags) {
#if !defined(__linux__)
    app.DebugPrintf("Sending from 0x%x to 0x%x %d bytes\n", this, player,
                    dwDataSize);
#endif
}
bool IQNetPlayer::IsSameSystem(IQNetPlayer* player) { return true; }
uint32_t IQNetPlayer::GetSendQueueSize(IQNetPlayer* player, uint32_t dwFlags) {
    return 0;
}
uint32_t IQNetPlayer::GetCurrentRtt() { return 0; }
bool IQNetPlayer::IsHost() { return this == &IQNet::m_player[0]; }
bool IQNetPlayer::IsGuest() { return false; }
bool IQNetPlayer::IsLocal() { return true; }
PlayerUID IQNetPlayer::GetXuid() { return INVALID_XUID; }
const wchar_t* IQNetPlayer::GetGamertag() {
    static const wchar_t* test = L"stub";
    return test;
}
int IQNetPlayer::GetSessionIndex() { return 0; }
bool IQNetPlayer::IsTalking() { return false; }
bool IQNetPlayer::IsMutedByLocalUser(uint32_t dwUserIndex) { return false; }
bool IQNetPlayer::HasVoice() { return false; }
bool IQNetPlayer::HasCamera() { return false; }
int IQNetPlayer::GetUserIndex() { return this - &IQNet::m_player[0]; }
void IQNetPlayer::SetCustomDataValue(uintptr_t ulpCustomDataValue) {
    m_customData = ulpCustomDataValue;
}
uintptr_t IQNetPlayer::GetCustomDataValue() { return m_customData; }

IQNetPlayer IQNet::m_player[4];

bool _bQNetStubGameRunning = false;

int32_t IQNet::AddLocalPlayerByUserIndex(uint32_t dwUserIndex) { return 0; }
IQNetPlayer* IQNet::GetHostPlayer() { return &m_player[0]; }
IQNetPlayer* IQNet::GetLocalPlayerByUserIndex(uint32_t dwUserIndex) {
    return &m_player[dwUserIndex];
}
IQNetPlayer* IQNet::GetPlayerByIndex(uint32_t dwPlayerIndex) {
    return &m_player[0];
}
IQNetPlayer* IQNet::GetPlayerBySmallId(uint8_t SmallId) { return &m_player[0]; }
IQNetPlayer* IQNet::GetPlayerByXuid(PlayerUID xuid) { return &m_player[0]; }
uint32_t IQNet::GetPlayerCount() { return 1; }
QNET_STATE IQNet::GetState() {
    return _bQNetStubGameRunning ? QNET_STATE_GAME_PLAY : QNET_STATE_IDLE;
}
bool IQNet::IsHost() { return true; }
int32_t IQNet::JoinGameFromInviteInfo(uint32_t dwUserIndex, uint32_t dwUserMask,
                                      const INVITE_INFO* pInviteInfo) {
    return 0;
}
void IQNet::HostGame() { _bQNetStubGameRunning = true; }
void IQNet::EndGame() { _bQNetStubGameRunning = false; }

uint32_t XUserGetSigninInfo(uint32_t dwUserIndex, uint32_t dwFlags,
                            PXUSER_SIGNIN_INFO pSigninInfo) {
    return 0;
}

const wchar_t* CXuiStringTable::Lookup(const wchar_t* szId) { return szId; }
const wchar_t* CXuiStringTable::Lookup(uint32_t nIndex) { return L"String"; }
void CXuiStringTable::Clear() {}
int32_t CXuiStringTable::Load(const wchar_t* szId) { return 0; }

uint32_t XUserAreUsersFriends(uint32_t dwUserIndex, PPlayerUID pXuids,
                              uint32_t dwXuidCount, bool* pfResult,
                              void* pOverlapped) {
    return 0;
}

int32_t XMemDecompress(XMEMDECOMPRESSION_CONTEXT Context, void* pDestination,
                       size_t* pDestSize, void* pSource, size_t SrcSize) {
    memcpy(pDestination, pSource, SrcSize);
    *pDestSize = SrcSize;
    return 0;

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
            return 0;
    }
    else
    */
    {
        return E_FAIL;
    }
}

int32_t XMemCompress(XMEMCOMPRESSION_CONTEXT Context, void* pDestination,
                     size_t* pDestSize, void* pSource, size_t SrcSize) {
    memcpy(pDestination, pSource, SrcSize);
    *pDestSize = SrcSize;
    return 0;

    /*
    COMPRESSOR_HANDLE Compressor    = (COMPRESSOR_HANDLE)Context;
    if( Compress(
                    Compressor,                  //  Compressor Handle
                    (void *)pSource,             //  Input buffer, Uncompressed
    data SrcSize,					 //  Uncompressed data
    size pDestination,                //  Compressed Buffer *pDestSize, //
    Compressed Buffer size pDestSize)	)				//
    Compressed Data size
    {
            return 0;
    }
    else
    */
    {
        return E_FAIL;
    }
}

int32_t XMemCreateCompressionContext(XMEMCODEC_TYPE CodecType,
                                     const void* pCodecParams, uint32_t Flags,
                                     XMEMCOMPRESSION_CONTEXT* pContext) {
    /*
    COMPRESSOR_HANDLE Compressor    = nullptr;

    int32_t hr = CreateCompressor(
            COMPRESS_ALGORITHM_XPRESS_HUFF, //  Compression Algorithm
            nullptr,                           //  Optional allocation routine
            &Compressor);                   //  Handle

    pContext = (XMEMDECOMPRESSION_CONTEXT *)Compressor;
    return hr;
    */
    return 0;
}

int32_t XMemCreateDecompressionContext(XMEMCODEC_TYPE CodecType,
                                       const void* pCodecParams, uint32_t Flags,
                                       XMEMDECOMPRESSION_CONTEXT* pContext) {
    /*
    DECOMPRESSOR_HANDLE  Decompressor    = nullptr;

    int32_t hr = CreateDecompressor(
            COMPRESS_ALGORITHM_XPRESS_HUFF, //  Compression Algorithm
            nullptr,                           //  Optional allocation routine
            &Decompressor);                   //  Handle

    pContext = (XMEMDECOMPRESSION_CONTEXT *)Decompressor;
    return hr;
    */
    return 0;
}

void XMemDestroyCompressionContext(XMEMCOMPRESSION_CONTEXT Context) {
    //	COMPRESSOR_HANDLE Compressor    = (COMPRESSOR_HANDLE)Context;
    //	CloseCompressor(Compressor);
}

void XMemDestroyDecompressionContext(XMEMDECOMPRESSION_CONTEXT Context) {
    //	DECOMPRESSOR_HANDLE Decompressor    = (DECOMPRESSOR_HANDLE)Context;
    //	CloseDecompressor(Decompressor);
}

// #if 1
uint32_t XGetLanguage() { return 1; }
uint32_t XGetLocale() { return 0; }
uint32_t XEnableGuestSignin(bool fEnable) { return 0; }
