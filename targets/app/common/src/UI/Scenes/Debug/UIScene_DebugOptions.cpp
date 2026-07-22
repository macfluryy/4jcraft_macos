#include "UIScene_DebugOptions.h"

#include "platform/InputActions.h"
#include "app/common/src/Console_Debug_enum.h"
#include "app/common/src/UI/Controls/UIControl_CheckBox.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/mac/Stubs/iggy_stubs.h"
#endif
#include "app/mac/MacGame.h"

class UILayer;

const wchar_t*
    UIScene_DebugOptionsMenu::m_DebugCheckboxTextA[eDebugSetting_Max + 1] = {
        L"Load Saves From Local Folder Mode",
        L"Write Saves To Local Folder Mode",
        L"Freeze Players",  
        L"Display Safe Area",
        L"Mobs don't attack",
        L"Freeze Time",
        L"Disable Weather",
        L"Craft Anything",
        L"Use DPad for debug",
        L"Mobs don't tick",
        L"Art tools",  
        L"Show UI Console",
        L"Distributable Save",
        L"Debug Leaderboards",
        L"Height-Water Maps",
        L"Superflat Nether",
        
        L"More lightning when thundering",
        L"Biome override",
        
        L"Go To Overworld",
        L"Unlock All DLC",  
        L"Show Marketing Guide",
};

UIScene_DebugOptionsMenu::UIScene_DebugOptionsMenu(int iPad, void* initData,
                                                   UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    
    initialiseMovie();

    unsigned int uiDebugBitmask = app.GetGameSettingsDebugMask(iPad);

    IggyValuePath* root = IggyPlayerRootPath(getMovie());
    for (m_iTotalCheckboxElements = 0;
         m_iTotalCheckboxElements < eDebugSetting_Max &&
         m_iTotalCheckboxElements < 21;
         ++m_iTotalCheckboxElements) {
        std::wstring label(m_DebugCheckboxTextA[m_iTotalCheckboxElements]);
        m_checkboxes[m_iTotalCheckboxElements].init(
            label, m_iTotalCheckboxElements,
            (uiDebugBitmask & (1 << m_iTotalCheckboxElements)) ? true : false);
    }
}

std::wstring UIScene_DebugOptionsMenu::getMoviePath() {
    return L"DebugOptionsMenu";
}

void UIScene_DebugOptionsMenu::handleInput(int iPad, int key, bool repeat,
                                           bool pressed, bool released,
                                           bool& handled) {
    
    
    

    switch (key) {
        case ACTION_MENU_CANCEL:
            if (pressed) {
                int iCurrentBitmaskIndex = 0;
                unsigned int uiDebugBitmask = 0L;
                for (int i = 0; i < m_iTotalCheckboxElements; i++) {
                    uiDebugBitmask |= m_checkboxes[i].IsChecked()
                                          ? (1 << iCurrentBitmaskIndex)
                                          : 0;
                    iCurrentBitmaskIndex++;
                }

                if (uiDebugBitmask != app.GetGameSettingsDebugMask(iPad)) {
                    app.SetGameSettingsDebugMask(iPad, uiDebugBitmask);
                    if (app.DebugSettingsOn()) {
                        app.ActionDebugMask(iPad);
                    } else {
                        
                        app.ActionDebugMask(iPad, true);
                    }

                    app.CheckGameSettingsChanged(true, iPad);
                }

                navigateBack();
            }
            break;
        case ACTION_MENU_OK:
        case ACTION_MENU_UP:
        case ACTION_MENU_DOWN:
        case ACTION_MENU_PAGEUP:
        case ACTION_MENU_PAGEDOWN:
            sendInputToMovie(key, repeat, pressed, released);
            break;
    }
}
