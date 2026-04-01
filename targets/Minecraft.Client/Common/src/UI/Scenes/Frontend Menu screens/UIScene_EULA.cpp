
#include "UIScene_EULA.h"

#include "Minecraft.Client/Common/src/UI/UI.h"
#include "console_helpers/StringHelpers.h"

UIScene_EULA::UIScene_EULA(int iPad, void* initData, UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    // Setup all the Iggy references we need for this scene
    initialiseMovie();

    parentLayer->addComponent(iPad, eUIComponent_Panorama);
    parentLayer->addComponent(iPad, eUIComponent_Logo);

    m_buttonConfirm.init(app.GetString(IDS_TOOLTIPS_ACCEPT), eControl_Confirm);

    std::wstring EULA = L"";

    std::vector<std::wstring> paragraphs;
    int lastIndex = 0;
    for (int index = EULA.find(L"\r\n", lastIndex, 2);
         index != std::wstring::npos;
         index = EULA.find(L"\r\n", lastIndex, 2)) {
        paragraphs.push_back(EULA.substr(lastIndex, index - lastIndex) + L" ");
        lastIndex = index + 2;
    }
    paragraphs.push_back(EULA.substr(lastIndex, EULA.length() - lastIndex));

    for (unsigned int i = 0; i < paragraphs.size(); ++i) {
        m_labelDescription.addText(paragraphs[i], i == (paragraphs.size() - 1));
    }

    // 4J-PB - If we have a signed in user connected, let's get the DLC now
    for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
        if ((InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i))) {
            if (!app.DLCInstallProcessCompleted() && !app.DLCInstallPending()) {
                app.StartInstallDLCProcess(i);
                break;
            }
        }
    }

    m_bIgnoreInput = false;

    // ui.setFontCachingCalculationBuffer(20000);
}

UIScene_EULA::~UIScene_EULA() {
    m_parentLayer->removeComponent(eUIComponent_Panorama);
    m_parentLayer->removeComponent(eUIComponent_Logo);
}

std::wstring UIScene_EULA::getMoviePath() { return L"EULA"; }

void UIScene_EULA::updateTooltips() {
    ui.SetTooltips(DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT);
}

void UIScene_EULA::handleInput(int iPad, int key, bool repeat, bool pressed,
                               bool released, bool& handled) {
    if (m_bIgnoreInput) return;

    ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_OK:
        case ACTION_MENU_DOWN:
        case ACTION_MENU_UP:
        case ACTION_MENU_PAGEUP:
        case ACTION_MENU_PAGEDOWN:
        case ACTION_MENU_OTHER_STICK_DOWN:
        case ACTION_MENU_OTHER_STICK_UP:
            sendInputToMovie(key, repeat, pressed, released);
            break;
    }
}

void UIScene_EULA::handlePress(F64 controlId, F64 childId) {
    switch ((int)controlId) {
        case eControl_Confirm:
            // CD - Added for audio
            ui.PlayUISFX(eSFX_Press);
            app.SetGameSettings(0, eGameSetting_PS3_EULA_Read, 1);
            ui.NavigateToScene(0, eUIScene_SaveMessage);
            ui.setFontCachingCalculationBuffer(-1);
            break;
    };
}
