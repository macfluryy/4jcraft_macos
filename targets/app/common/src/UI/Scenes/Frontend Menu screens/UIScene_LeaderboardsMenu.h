#pragma once

#include <string>
#include <vector>

#include "platform/PlatformTypes.h"
#include "platform/sdl2/Storage.h"
#include "app/common/src/Leaderboards/LeaderboardInterface.h"
#include "app/common/src/Leaderboards/LeaderboardManager.h"
#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/common/src/UI/Controls/UIControl_Label.h"
#include "app/common/src/UI/Controls/UIControl_LeaderboardList.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/mac/Stubs/iggy_stubs.h"
#endif
#include "app/mac/Iggy/include/rrCore.h"

class UILayer;

class UIScene_LeaderboardsMenu : public UIScene,
                                 public LeaderboardReadListener {
private:
    
    
    
    
    static const int LEADERBOARD_KILLS_POSITION = 3;

    static const int NUM_LEADERBOARDS =
        4;                               
    static const int NUM_ENTRIES = 101;  
    static const int READ_SIZE = 15;     

    struct LeaderboardDescriptor {
        unsigned int m_columnCount;
        bool m_isDistanceLeaderboard;
        unsigned int m_title;

        LeaderboardDescriptor(unsigned int columnCount,
                              bool isDistanceLeaderboard, unsigned int title) {
            m_columnCount = columnCount;
            m_isDistanceLeaderboard = isDistanceLeaderboard;
            m_title = title;
        }
    };

    static const LeaderboardDescriptor LEADERBOARD_DESCRIPTORS[NUM_LEADERBOARDS]
                                                              [4];
    static const int TitleIcons[NUM_LEADERBOARDS][7];

    struct LeaderboardEntry {
        PlayerUID m_xuid;
        unsigned int
            m_row;  
        unsigned int m_rank;
        wchar_t m_wcRank[12];
        wchar_t m_gamerTag[XUSER_NAME_SIZE + 1];
        
        unsigned int m_columns[7];
        wchar_t m_wcColumns[7][12];
        bool m_bPlayer;           
        bool m_bOnline;           
        bool m_bFriend;           
        bool m_bRequestedFriend;  
        int m_idsErrorMessage;    
                                  
    };

    struct Leaderboard {
        unsigned int m_totalEntryCount;  
                                         
                                         
        std::vector<LeaderboardEntry> m_entries;
        unsigned int m_numColumns;
    };

    Leaderboard m_leaderboard;  
                                

    unsigned int
        m_currentLeaderboard;  
    IPlatformLeaderboard::EFilterMode
        m_currentFilter;               
    unsigned int m_currentDifficulty;  

    unsigned int m_newEntryIndex;  
    unsigned int
        m_newReadSize;  

    unsigned int m_newEntriesCount;  

    int m_newTop;  
    int m_newSel;  

    bool m_isProcessingStatsRead;
    bool m_bPopulatedOnce;
    bool m_bReady;

    LeaderboardInterface m_interface;

    UIControl_LeaderboardList m_listEntries;
    UIControl_Label m_labelFilter, m_labelLeaderboard, m_labelEntries,
        m_labelInfo;
    UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
    UI_MAP_ELEMENT(m_listEntries, "Gamers")

    UI_MAP_ELEMENT(m_labelFilter, "Filter")
    UI_MAP_ELEMENT(m_labelLeaderboard, "Leaderboard")
    UI_MAP_ELEMENT(m_labelEntries, "Entries")
    UI_MAP_ELEMENT(m_labelInfo, "Info")
    UI_END_MAP_ELEMENTS_AND_NAMES()

    static int ExitLeaderboards(void* pParam, int iPad,
                                C4JStorage::EMessageResult result);

public:
    UIScene_LeaderboardsMenu(int iPad, void* initData, UILayer* parentLayer);
    ~UIScene_LeaderboardsMenu();

    virtual void updateTooltips();
    virtual void updateComponents();

    virtual EUIScene getSceneType() { return eUIScene_LeaderboardsMenu; }

    
    virtual bool hasFocus(int iPad) { return bHasFocus; }
    virtual void handleTimerComplete(int id);

private:
    int GetEntryStartIndex();

protected:
    virtual std::wstring getMoviePath();

public:
    virtual void tick();
    virtual void handleReload();

    
    virtual void handleInput(int iPad, int key, bool repeat, bool pressed,
                             bool released, bool& handled);

private:
    
    void ReadStats(int startIndex);

    
    
    int m_numStats;
    IPlatformLeaderboard::ViewOut m_stats;
    bool RetrieveStats();

    
    void CopyLeaderboardEntry(IPlatformLeaderboard::ReadScore* statsRow,
                              int leaderboardEntryIndex,
                              bool isDistanceLeaderboard);

    
    void PopulateLeaderboard(IPlatformLeaderboard::eStatsReturn ret);

    
    void SetLeaderboardHeader();

    
    int SetLeaderboardTitleIcons();

    
    
    virtual bool OnStatsReadComplete(IPlatformLeaderboard::eStatsReturn ret,
                                     int numResults,
                                     IPlatformLeaderboard::ViewOut results);

    virtual void customDraw(IggyCustomDrawCallbackRegion* region);

    virtual void handleSelectionChanged(F64 selectedId);
    virtual void handleRequestMoreData(F64 startIndex, bool up);

    bool m_bIgnoreInput;
};
