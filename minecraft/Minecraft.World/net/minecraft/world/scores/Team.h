#pragma once

#include <string>

class Team {
public:
    virtual bool isAlliedTo(Team* other);

    virtual std::wstring getName() = 0;
    virtual std::wstring getFormattedName(
        const std::wstring& teamMemberName) = 0;
    virtual bool canSeeFriendlyInvisibles() = 0;
    virtual bool isAllowFriendlyFire() = 0;
};