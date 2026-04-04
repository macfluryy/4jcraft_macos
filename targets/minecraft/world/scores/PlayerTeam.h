#pragma once

#include <string>
#include <unordered_set>

#include "Team.h"

class Scoreboard;

class PlayerTeam : public Team {
public:
    static const int MAX_NAME_LENGTH = 16;
    static const int MAX_DISPLAY_NAME_LENGTH = 32;
    static const int MAX_PREFIX_LENGTH = 16;
    static const int MAX_SUFFIX_LENGTH = 16;

private:
    static const int BIT_FRIENDLY_FIRE = 0;
    static const int BIT_SEE_INVISIBLES = 1;

    Scoreboard* scoreboard;
    std::wstring name;
    std::unordered_set<std::wstring> players;
    std::wstring displayName;
    std::wstring prefix;
    std::wstring suffix;
    bool allowFriendlyFire;
    bool seeFriendlyInvisibles;

public:
    PlayerTeam(Scoreboard* scoreboard, const std::wstring& name);

    Scoreboard* getScoreboard();
    std::wstring getName();
    std::wstring getDisplayName();
    void setDisplayName(const std::wstring& displayName);
    std::unordered_set<std::wstring>* getPlayers();
    std::wstring getPrefix();
    void setPrefix(const std::wstring& prefix);
    std::wstring getSuffix();
    void setSuffix(const std::wstring& suffix);
    std::wstring getFormattedName(const std::wstring& teamMemberName);
    static std::wstring formatNameForTeam(PlayerTeam* team);
    static std::wstring formatNameForTeam(Team* team, const std::wstring& name);
    bool isAllowFriendlyFire();
    void setAllowFriendlyFire(bool allowFriendlyFire);
    bool canSeeFriendlyInvisibles();
    void setSeeFriendlyInvisibles(bool seeFriendlyInvisibles);
    int packOptions();
    void unpackOptions(int options);
};