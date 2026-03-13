#pragma once
#include "../Screen.h"
class EditBox;
class LevelStorageSource;


class CreateWorldScreen : public Screen
{
private:
	Screen *lastScreen;
    EditBox *nameEdit;
    EditBox *seedEdit;
    std::wstring resultFolder;
    bool done;

    bool moreOptions;
    std::wstring gameMode;
    bool generateStructures;
    bool bonusChest;
    bool cheatsEnabled;
    bool flatWorld;

    Button *gameModeButton;
    Button *moreWorldOptionsButton;
    Button *generateStructuresButton;
    Button *bonusChestButton;
    Button *worldTypeButton;
    Button *cheatsEnabledButton;

    std::wstring gameModeDescriptionLine1;
    std::wstring gameModeDescriptionLine2;
    std::wstring seed;
public:
	CreateWorldScreen(Screen *lastScreen);
    virtual void tick();
    virtual void init();
private:
	void updateResultFolder();
    void updateStrings();
public:
	static std::wstring findAvailableFolderName(LevelStorageSource *levelSource, const std::wstring& folder);
    virtual void removed();
protected:
	virtual void buttonClicked(Button *button);
    virtual void keyPressed(wchar_t ch, int eventKey);
    virtual void mouseClicked(int x, int y, int buttonNum);
public:
	virtual void render(int xm, int ym, float a);
    virtual void tabPressed();

private:
    int m_iGameModeId;
    bool m_bGameModeCreative;
    
    struct MoreOptionsParams
    {
        bool bGenerateOptions;
        bool bStructures;
        bool bFlatWorld;
        bool bBonusChest;
        bool bPVP;
        bool bTrust;
        bool bFireSpreads;
        bool bHostPrivileges;
        bool bTNT;
        bool bMobGriefing;
        bool bKeepInventory;
        bool bDoMobSpawning;
        bool bDoMobLoot;
        bool bDoTileDrops;
        bool bNaturalRegeneration;
        bool bDoDaylightCycle;
        bool bOnlineGame;
        bool bInviteOnly;
        bool bAllowFriendsOfFriends;
        bool bOnlineSettingChangedBySystem;
        bool bCheatsEnabled;
        int dwTexturePack;
        int iPad;
        std::wstring worldName;
        std::wstring seed;
    } m_MoreOptionsParams;
};