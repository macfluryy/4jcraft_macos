#pragma once

#include "app/common/src/Leaderboards/LeaderboardManager.h"

class WindowsLeaderboardManager : public LeaderboardManager {
public:
    virtual void Tick() {}

    
    virtual bool OpenSession() { return true; }

    
    virtual void CloseSession() {}

    
    virtual void DeleteSession() {}

    
    
    

    virtual bool WriteStats(unsigned int viewCount, ViewIn views) {
        return false;
    }

    virtual bool ReadStats_Friends(LeaderboardReadListener* callback,
                                   int difficulty, EStatsType type,
                                   PlayerUID myUID) {
        return false;
    }
    virtual bool ReadStats_MyScore(LeaderboardReadListener* callback,
                                   int difficulty, EStatsType type,
                                   PlayerUID myUID, unsigned int readCount) {
        return false;
    }
    virtual bool ReadStats_TopRank(LeaderboardReadListener* callback,
                                   int difficulty, EStatsType type,
                                   unsigned int startIndex,
                                   unsigned int readCount) {
        return false;
    }

    
    virtual void FlushStats() {}

    
    virtual void CancelOperation() {}

    
    virtual bool isIdle() { return true; }
};
