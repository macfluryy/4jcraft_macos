#include "../../../../../../Minecraft.World/Header Files/stdafx.h"
#include "../../UI.h"
#include "UIScene_SaveMessage.h"

#define PROFILE_LOADED_TIMER_ID 0
#define PROFILE_LOADED_TIMER_TIME 50

UIScene_SaveMessage::UIScene_SaveMessage(int iPad, void* initData,
                                         UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    // Setup all the Iggy references we need for this scene
    initialiseMovie();

    parentLayer->addComponent(iPad, eUIComponent_Panorama);
    parentLayer->addComponent(iPad, eUIComponent_Logo);

    m_buttonConfirm.init(app.GetString(IDS_CONFIRM_OK), eControl_Confirm);
    m_labelDescription.init(app.GetString(IDS_SAVE_ICON_MESSAGE));

    IggyDataValue result;

    // Russian needs to resize the box
    IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result,
                                            IggyPlayerRootPath(getMovie()),
                                            m_funcAutoResize, 0, nullptr);

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

    // 4J-TomK - rebuild touch after auto resize
}

UIScene_SaveMessage::~UIScene_SaveMessage() {
    m_parentLayer->removeComponent(eUIComponent_Panorama);
    m_parentLayer->removeComponent(eUIComponent_Logo);
}

std::wstring UIScene_SaveMessage::getMoviePath() { return L"SaveMessage"; }

void UIScene_SaveMessage::updateTooltips() {
    ui.SetTooltips(DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT);
}

void UIScene_SaveMessage::handleInput(int iPad, int key, bool repeat,
                                      bool pressed, bool released,
                                      bool& handled) {
    if (m_bIgnoreInput) return;

    ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_OK:
            sendInputToMovie(key, repeat, pressed, released);
            break;
            // #ifdef 0
            // 	case ACTION_MENU_Y:
            // 		if(pressed)
            // 		{
            // 			// language select - switch to Greek for now
            // 			if(app.GetMinecraftLanguage(iPad)==MINECRAFT_LANGUAGE_DEFAULT)
            // 			{
            // 				app.SetMinecraftLanguage(iPad,MINECRAFT_LANGUAGE_GREEK);
            // 			}
            // 			else
            // 			{
            // 				app.SetMinecraftLanguage(iPad,MINECRAFT_LANGUAGE_DEFAULT);
            // 			}
            // 			// reload the string table
            // 			ui.SetupFont();
            // 			app.loadStringTable();
            // 			handleReload();
            // 		}
            // 		break;
            // #endif
    }
}

void UIScene_SaveMessage::handlePress(F64 controlId, F64 childId) {
    switch ((int)controlId) {
        case eControl_Confirm:

            // CD - Added for audio
            ui.PlayUISFX(eSFX_Press);

            m_bIgnoreInput = true;

            ui.NavigateToHomeMenu();
            break;
    };
}

void UIScene_SaveMessage::handleTimerComplete(int id) {
    switch (id) {
        case PROFILE_LOADED_TIMER_ID: {
        }

        break;
    }
}
