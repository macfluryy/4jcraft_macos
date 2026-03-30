#pragma once

#include "Common/Leaderboards/LeaderboardManager.h"

typedef SceRtcTick SonyRtcTick;

class SonyLeaderboardManager : public LeaderboardManager {
protected:
    enum EStatsState {
        eStatsState_Idle,
        eStatsState_Getting,
        eStatsState_Failed,
        eStatsState_Ready,
        eStatsState_Canceled,
        eStatsState_Max
    };

public:
    SonyLeaderboardManager();
    virtual ~SonyLeaderboardManager();

protected:
    unsigned short m_openSessions;

    C4JThread* m_threadScoreboard;
    bool m_running;

    int m_titleContext;
    int32_t m_requestId;

    // SceNpId m_myNpId;

    static int scoreboardThreadEntry(void* lpParam);
    void scoreboardThreadInternal();

    virtual bool getScoreByIds();
    virtual bool getScoreByRange();

    virtual bool setScore();

    std::queue<RegisterScore> m_views;

    CRITICAL_SECTION m_csViewsLock;

    EStatsState m_eStatsState;  // State of the stats read
    //	EFilterMode		m_eFilterMode;

    ReadScore* m_scores;
    unsigned int m_maxRank;
    // SceNpScoreRankData *m_stats;

public:
    virtual void Tick();

    // Open a session
    virtual bool OpenSession();

    // Close a session
    virtual void CloseSession();

    // Delete a session
    virtual void DeleteSession();

    // Write the given stats
    // This is called synchronously and will not free any memory allocated for
    // views when it is done

    virtual bool WriteStats(unsigned int viewCount, ViewIn views);

    virtual bool ReadStats_Friends(LeaderboardReadListener* callback,
                                   int difficulty, EStatsType type,
                                   PlayerUID myUID, unsigned int startIndex,
                                   unsigned int readCount);
    virtual bool ReadStats_MyScore(LeaderboardReadListener* callback,
                                   int difficulty, EStatsType type,
                                   PlayerUID myUID, unsigned int readCount);
    virtual bool ReadStats_TopRank(LeaderboardReadListener* callback,
                                   int difficulty, EStatsType type,
                                   unsigned int startIndex,
                                   unsigned int readCount);

    // Perform a flush of the stats
    virtual void FlushStats();

    // Cancel the current operation
    virtual void CancelOperation();

    // Is the leaderboard manager idle.
    virtual bool isIdle();

protected:
    int getBoardId(int difficulty, EStatsType);

    SceNpScorePlayerRankData* addPadding(unsigned int num,
                                         SceNpScoreRankData* rankData);

    void convertToOutput(unsigned int& num, ReadScore* out,
                         SceNpScorePlayerRankData* rankData,
                         SceNpScoreComment* comm);

    void toBinary(void* out, SceNpScoreComment* in);
    void fromBinary(SceNpScoreComment** out, void* in);

    void toBase32(SceNpScoreComment* out, void* in);
    void fromBase32(void* out, SceNpScoreComment* in);

    void toSymbols(char*);
    void fromSymbols(char*);

    bool test_string(std::string);

    void initReadScoreStruct(ReadScore& out, SceNpScoreRankData&);
    void fillReadScoreStruct(ReadScore& out, SceNpScoreComment& comment);

    static bool SortByRank(const ReadScore& lhs, const ReadScore& rhs);

protected:
    // 4J-JEV: Interface differences:

    // Sce NP score library function redirects.
    virtual int32_t initialiseScoreUtility() { return ERROR_SUCCESS; }
    virtual bool scoreUtilityAlreadyInitialised(int32_t hr) { return false; }

    virtual int32_t createTitleContext(const SceNpId& npId) = 0;
    virtual int32_t destroyTitleContext(int titleContext) = 0;

    virtual int32_t createTransactionContext(int titleContext) = 0;
    virtual int32_t abortTransactionContext(int transactionContext) = 0;
    virtual int32_t destroyTransactionContext(int transactionContext) = 0;

    virtual int32_t fillByIdsQuery(const SceNpId& myNpId, SceNpId*& npIds,
                                   uint32_t& len);


    virtual char* getComment(SceNpScoreComment* comment) = 0;
};
