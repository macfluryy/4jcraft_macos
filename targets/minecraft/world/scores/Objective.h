#pragma once

#include <string>

class Scoreboard;
class ObjectiveCriteria;

class Objective {
public:
    static const int MAX_NAME_LENGTH = 16;
    static const int MAX_DISPLAY_NAME_LENGTH = 32;

private:
    Scoreboard* scoreboard;
    std::wstring name;
    ObjectiveCriteria* criteria;
    std::wstring displayName;

public:
    Objective(Scoreboard* scoreboard, const std::wstring& name,
              ObjectiveCriteria* criteria);

    Scoreboard* getScoreboard();
    std::wstring getName();
    ObjectiveCriteria* getCriteria();
    std::wstring getDisplayName();
    void setDisplayName(const std::wstring& name);
};