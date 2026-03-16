#ifdef __linux__

#include <cstddef>
#include <cstring>
#include <string>
#include <pthread.h>

#include "Stubs/LinuxStubs.h"
#include "../Common/Consoles_App.h"

void Display::update() {}

int CMinecraftApp::GetTPConfigVal(WCHAR* pwchDataFile) { return 0; }

#include "../../Minecraft.World/Platform/x64headers/extraX64.h"

void PIXSetMarkerDeprecated(int a, const char* b, ...) {}

#include "../Xbox/Network/NetworkPlayerXbox.h"

NetworkPlayerXbox::NetworkPlayerXbox(IQNetPlayer* p)
    : m_qnetPlayer(p), m_pSocket(nullptr) {}
IQNetPlayer* NetworkPlayerXbox::GetQNetPlayer() { return m_qnetPlayer; }
unsigned char NetworkPlayerXbox::GetSmallId() { return 0; }
void NetworkPlayerXbox::SendData(INetworkPlayer*, const void*, int, bool) {}
bool NetworkPlayerXbox::IsSameSystem(INetworkPlayer*) { return false; }
int NetworkPlayerXbox::GetSendQueueSizeBytes(INetworkPlayer*, bool) {
    return 0;
}
int NetworkPlayerXbox::GetSendQueueSizeMessages(INetworkPlayer*, bool) {
    return 0;
}
int NetworkPlayerXbox::GetCurrentRtt() { return 0; }
bool NetworkPlayerXbox::IsHost() { return false; }
bool NetworkPlayerXbox::IsGuest() { return false; }
bool NetworkPlayerXbox::IsLocal() { return true; }
int NetworkPlayerXbox::GetSessionIndex() { return 0; }
bool NetworkPlayerXbox::IsTalking() { return false; }
bool NetworkPlayerXbox::IsMutedByLocalUser(int) { return false; }
bool NetworkPlayerXbox::HasVoice() { return false; }
bool NetworkPlayerXbox::HasCamera() { return false; }
int NetworkPlayerXbox::GetUserIndex() { return 0; }
void NetworkPlayerXbox::SetSocket(Socket* s) { m_pSocket = s; }
Socket* NetworkPlayerXbox::GetSocket() { return m_pSocket; }
const wchar_t* NetworkPlayerXbox::GetOnlineName() { return L"Player"; }
std::wstring NetworkPlayerXbox::GetDisplayName() { return L"Player"; }
PlayerUID NetworkPlayerXbox::GetUID() { return PlayerUID(); }

#endif
