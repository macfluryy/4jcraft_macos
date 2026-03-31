#pragma once

#include <string>

#include "UIControl_Base.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Common/Source Files/UI/UIScene.h"

class UIControl_LeaderboardList : public UIControl_Base {
private:
    IggyName m_funcInitLeaderboard, m_funcAddDataSet;
    IggyName m_funcResetLeaderboard;
    IggyName m_funcSetupTitles, m_funcSetColumnIcon;

public:
    enum ELeaderboardIcons {
        e_ICON_TYPE_IGGY = 0,
        e_ICON_TYPE_CLIMBED = 32001,
        e_ICON_TYPE_FALLEN = 32002,
        e_ICON_TYPE_WALKED = 32003,
        e_ICON_TYPE_SWAM = 32004,
        e_ICON_TYPE_ZOMBIE = 32005,
        e_ICON_TYPE_ZOMBIEPIGMAN = 32006,
        e_ICON_TYPE_GHAST = 32007,
        e_ICON_TYPE_CREEPER = 32008,
        e_ICON_TYPE_SKELETON = 32009,
        e_ICON_TYPE_SPIDER = 32010,
        e_ICON_TYPE_SPIDERJOKEY = 32011,
        e_ICON_TYPE_SLIME = 32012,
        e_ICON_TYPE_PORTAL = 32013,
    };
    UIControl_LeaderboardList();

    virtual bool setupControl(UIScene* scene, IggyValuePath* parent,
                              const std::string& controlName);

    void init(int id);
    virtual void ReInit();

    void clearList();

    void setupTitles(const std::wstring& rank, const std::wstring& gamertag);
    void initLeaderboard(int iFirstFocus, int iTotalEntries, int iNumColumns);
    void setColumnIcon(int iColumn, int iType);
    void addDataSet(bool bLast, int iId, int iRank,
                    const std::wstring& gamertag, bool bDisplayMessage,
                    const std::wstring& col0, const std::wstring& col1,
                    const std::wstring& col2, const std::wstring& col3,
                    const std::wstring& col4, const std::wstring& col5,
                    const std::wstring& col6);
};