// ts just stink, social manager is a stupid stub rn
#pragma once
#include "../Stubs/winapi_stubs.h"

#ifndef _SOCIAL_MANAGER_H
#define _SOCIAL_MANAGER_H

enum ESocialNetwork { eFacebook = 0, eNumSocialNetworks };

class CSocialManager {
public:
    static CSocialManager* Instance() {
        static CSocialManager s_instance;
        return &s_instance;
    }

    void Initialise() {}
    void Tick() {}

    bool RefreshPostingCapability() { return false; }
    bool IsTitleAllowedToPostAnything() { return false; }
    bool IsTitleAllowedToPostImages() { return false; }
    bool IsTitleAllowedToPostLinks() { return false; }
    bool AreAllUsersAllowedToPostImages() { return false; }

    bool PostLinkToSocialNetwork(ESocialNetwork, DWORD, bool) { return false; }
    bool PostImageToSocialNetwork(ESocialNetwork, DWORD, bool) { return false; }

    void SetSocialPostText(const wchar_t*, const wchar_t*, const wchar_t*) {}

private:
    CSocialManager() {}
    CSocialManager(const CSocialManager&);
    CSocialManager& operator=(const CSocialManager&);
};

#endif  // _SOCIAL_MANAGER_H
