#pragma once

#include "app/include/NetTypes.h"



typedef struct _GameSessionData {
    unsigned short netVersion;          
    unsigned int m_uiGameHostSettings;  
    unsigned int texturePackParentId;   
    unsigned char subTexturePackId;     

    bool isReadyToJoin;  

    _GameSessionData() {
        netVersion = 0;
        m_uiGameHostSettings = 0;
        texturePackParentId = 0;
        subTexturePackId = 0;
    }
} GameSessionData;

class FriendSessionInfo {
public:
    SessionID sessionId;
    wchar_t* displayLabel;
    unsigned char displayLabelLength;
    unsigned char displayLabelViewableStartIndex;
    GameSessionData data;
    bool hasPartyMember;

    FriendSessionInfo() {
        displayLabel = nullptr;
        displayLabelLength = 0;
        displayLabelViewableStartIndex = 0;
        hasPartyMember = false;
    }

    ~FriendSessionInfo() {
        if (displayLabel != nullptr) delete displayLabel;
    }
};
