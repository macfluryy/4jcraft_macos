#pragma once


class GameType;

class LevelSummary
{
	const std::wstring levelId;
    const std::wstring levelName;
    const __int64 lastPlayed;
    const __int64 sizeOnDisk;
    const bool requiresConversion;
	GameType *gameMode;
	const bool hardcore;
	const bool _hasCheats;

public:
	LevelSummary(const std::wstring& levelId, const std::wstring& levelName, __int64 lastPlayed, __int64 sizeOnDisk,  GameType *gameMode, bool requiresConversion, bool hardcore, bool hasCheats);
    std::wstring getLevelId();
    std::wstring getLevelName();
    __int64 getSizeOnDisk();
    bool isRequiresConversion();
    __int64 getLastPlayed();
    int compareTo(LevelSummary *rhs);
	GameType *getGameMode();
	bool isHardcore();
	bool hasCheats();
};
