#include <wchar.h>

#include "SignTileEntity.h"
#include "Minecraft.Client/net/minecraft/client/Minecraft.h"
#include "Minecraft.Client/net/minecraft/server/level/ServerLevel.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"
#include "Minecraft.Client/Linux/Stubs/winapi_stubs.h"
#include "nbt/CompoundTag.h"
#include "Minecraft.World/net/minecraft/network/packet/SignUpdatePacket.h"
#include "Minecraft.World/net/minecraft/world/level/tile/entity/TileEntity.h"

const int SignTileEntity::MAX_LINE_LENGTH = 15;

SignTileEntity::SignTileEntity() : TileEntity() {
    m_wsmessages[0] = L"";
    m_wsmessages[1] = L"";
    m_wsmessages[2] = L"";
    m_wsmessages[3] = L"";
    m_bVerified = true;
    m_bCensored = false;

    m_iSelectedLine = -1;

    _isEditable = true;

    playerWhoMayEdit = nullptr;
}

SignTileEntity::~SignTileEntity() {
    // TODO ORBIS_STUBBED;
    // 4J-PB - we don't need to verify strings anymore -
    // PlatformInput.CancelQueuedVerifyStrings(&SignTileEntity::StringVerifyCallback,(void*)this);
}

void SignTileEntity::save(CompoundTag* tag) {
    TileEntity::save(tag);
    tag->putString(L"Text1", m_wsmessages[0]);
    tag->putString(L"Text2", m_wsmessages[1]);
    tag->putString(L"Text3", m_wsmessages[2]);
    tag->putString(L"Text4", m_wsmessages[3]);
#if !defined(_CONTENT_PACKAGE)
    OutputDebugStringW(L"### - Saving a sign with text - \n");
    for (int i = 0; i < 4; i++) {
        OutputDebugStringW(m_wsmessages[i].c_str());
        OutputDebugStringW(L"\n");
    }
#endif
}

void SignTileEntity::load(CompoundTag* tag) {
    _isEditable = false;
    TileEntity::load(tag);
    for (int i = 0; i < MAX_SIGN_LINES; i++) {
        wchar_t* buf = new wchar_t[256];
        swprintf(buf, 256, L"Text%d", (i + 1));
        m_wsmessages[i] = tag->getString(buf);
        if (m_wsmessages[i].length() > MAX_LINE_LENGTH)
            m_wsmessages[i] = m_wsmessages[i].substr(0, MAX_LINE_LENGTH);
    }
#if !defined(_CONTENT_PACKAGE)
    OutputDebugStringW(L"### - Loaded a sign with text - \n");
    for (int i = 0; i < 4; i++) {
        OutputDebugStringW(m_wsmessages[i].c_str());
        OutputDebugStringW(L"\n");
    }
#endif

    // 4J Stu - Fix for #13531 - Bug: Signs do not Censor after loading a save
    // Set verified as false so that it can be re-verified
    m_bVerified = false;

    setChanged();
}

std::shared_ptr<Packet> SignTileEntity::getUpdatePacket() {
    std::wstring copy[MAX_SIGN_LINES];
    for (int i = 0; i < MAX_SIGN_LINES; i++) {
        copy[i] = m_wsmessages[i];
    }
    return std::shared_ptr<SignUpdatePacket>(
        new SignUpdatePacket(x, y, z, m_bVerified, m_bCensored, copy));
}

bool SignTileEntity::isEditable() { return _isEditable; }

void SignTileEntity::setEditable(bool isEditable) {
    this->_isEditable = isEditable;
    if (!isEditable) {
        playerWhoMayEdit = nullptr;
    }
}

void SignTileEntity::setAllowedPlayerEditor(std::shared_ptr<Player> player) {
    playerWhoMayEdit = player;
}

std::shared_ptr<Player> SignTileEntity::getPlayerWhoMayEdit() {
    return playerWhoMayEdit;
}

void SignTileEntity::setChanged() {
    Minecraft* pMinecraft = Minecraft::GetInstance();

    // 4J-PB - For TU14 we are allowed to not verify strings anymore !
    m_bVerified = true;
    /*
    if(!g_NetworkManager.IsLocalGame() && !m_bVerified)
    //if (pMinecraft->level->isClientSide)
    {
            wchar_t *wcMessages[MAX_SIGN_LINES];
            for (int i = 0; i < MAX_SIGN_LINES; ++i)
            {
                    wcMessages[i]=new wchar_t [MAX_LINE_LENGTH+1];
                    ZeroMemory(wcMessages[i],sizeof(wchar_t)*(MAX_LINE_LENGTH+1));
                    if(m_wsmessages[i].length()>0)
                    {
                            memcpy(wcMessages[i],m_wsmessages[i].c_str(),m_wsmessages[i].length()*sizeof(wchar_t));
                    }
            }
            // at this point, we can ask the online string verifier if our sign
text is ok #if 0 m_bVerified=true; #else

            if(!PlatformInput.VerifyStrings((wchar_t**)&wcMessages,MAX_SIGN_LINES,&SignTileEntity::StringVerifyCallback,(void*)this))
            {
                    // Nothing to verify
                    m_bVerified=true;
            }
            for(unsigned int i = 0; i < MAX_SIGN_LINES; ++i)
            {
                    delete [] wcMessages[i];
            }
#endif
    }
    else
    {
            // set the sign to allowed (local game)
            m_bVerified=true;
    }
    */
}

void SignTileEntity::SetMessage(int iIndex, std::wstring& wsText) {
    m_wsmessages[iIndex] = wsText;
}

// 4J-PB - added for string verification
int SignTileEntity::StringVerifyCallback(void* lpParam,
                                         STRING_VERIFY_RESPONSE* pResults) {
    // results will be in m_pStringVerifyResponse
    SignTileEntity* pClass = (SignTileEntity*)lpParam;

    pClass->m_bVerified = true;
    pClass->m_bCensored = false;
    for (int i = 0; i < pResults->wNumStrings; i++) {
        if (pResults->pStringResult[i] != ERROR_SUCCESS) {
            pClass->m_bCensored = true;
        }
    }

    if (!pClass->level->isClientSide) {
        ServerLevel* serverLevel = (ServerLevel*)pClass->level;
        // 4J Stu - This callback gets called on the main thread, but tried to
        // access things on the server thread. Change to go through the
        // protected method.
        // pClass->level->sendTileUpdated(pClass->x, pClass->y, pClass->z);
        serverLevel->queueSendTileUpdate(pClass->x, pClass->y, pClass->z);
    }

    return 0;
}

// 4J Added
std::shared_ptr<TileEntity> SignTileEntity::clone() {
    std::shared_ptr<SignTileEntity> result =
        std::make_shared<SignTileEntity>();
    TileEntity::clone(result);

    result->m_wsmessages[0] = m_wsmessages[0];
    result->m_wsmessages[1] = m_wsmessages[1];
    result->m_wsmessages[2] = m_wsmessages[2];
    result->m_wsmessages[3] = m_wsmessages[3];
    result->m_bVerified = m_bVerified;
    result->m_bCensored = m_bCensored;
    return result;
}