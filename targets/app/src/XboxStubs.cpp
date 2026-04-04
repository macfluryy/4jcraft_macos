

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "platform/PlatformTypes.h"
#include "app/linux/Stubs/winapi_stubs.h"
#include "app/include/NetTypes.h"
#include "app/include/XboxStubs.h"

class INVITE_INFO;

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

// #if 1
uint32_t XGetLanguage() { return 1; }
uint32_t XGetLocale() { return 0; }
uint32_t XEnableGuestSignin(bool fEnable) { return 0; }
