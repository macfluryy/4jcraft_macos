#include "SignTileEntity.h"

#include <wchar.h>

#include "app/mac/Stubs/winapi_stubs.h"
#include "PlatformTypes.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/network/packet/SignUpdatePacket.h"
#include "minecraft/server/level/ServerLevel.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"
#include "nbt/CompoundTag.h"

class Player;

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

    
    m_bVerified = true;
    

































}

void SignTileEntity::SetMessage(int iIndex, std::wstring& wsText) {
    m_wsmessages[iIndex] = wsText;
}


int SignTileEntity::handleStringVerify(STRING_VERIFY_RESPONSE* pResults) {
    
    m_bVerified = true;
    m_bCensored = false;
    for (int i = 0; i < pResults->wNumStrings; i++) {
        if (pResults->pStringResult[i] != ERROR_SUCCESS) {
            m_bCensored = true;
        }
    }

    if (!level->isClientSide) {
        ServerLevel* serverLevel = (ServerLevel*)level;
        
        
        
        
        serverLevel->queueSendTileUpdate(x, y, z);
    }

    return 0;
}


std::shared_ptr<TileEntity> SignTileEntity::clone() {
    std::shared_ptr<SignTileEntity> result = std::make_shared<SignTileEntity>();
    TileEntity::clone(result);

    result->m_wsmessages[0] = m_wsmessages[0];
    result->m_wsmessages[1] = m_wsmessages[1];
    result->m_wsmessages[2] = m_wsmessages[2];
    result->m_wsmessages[3] = m_wsmessages[3];
    result->m_bVerified = m_bVerified;
    result->m_bCensored = m_bCensored;
    return result;
}