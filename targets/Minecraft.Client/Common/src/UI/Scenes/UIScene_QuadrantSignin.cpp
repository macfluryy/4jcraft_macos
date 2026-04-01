
#include "UIScene_QuadrantSignin.h"

#include <wchar.h>

#include "4J.Common/4J_Compat.h"
#include "4J.Common/4J_InputActions.h"
#include "4J_Input.h"
#include "4J_Profile.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_BitmapIcon.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Label.h"
#include "Minecraft.Client/Common/src/UI/UILayer.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Linux_App.h"
#include "Minecraft.Client/Linux/Linux_UIController.h"
#include "strings.h"

UIScene_QuadrantSignin::UIScene_QuadrantSignin(int iPad, void* _initData,
                                               UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    // Setup all the Iggy references we need for this scene
    initialiseMovie();

    m_signInInfo = *((SignInInfo*)_initData);

    m_bIgnoreInput = false;

    m_lastRequestedAvatar = -1;

    _initQuadrants();

    parentLayer->addComponent(iPad, eUIComponent_MenuBackground);
}

UIScene_QuadrantSignin::~UIScene_QuadrantSignin() {
    m_parentLayer->removeComponent(eUIComponent_MenuBackground);
}

std::wstring UIScene_QuadrantSignin::getMoviePath() {
    return L"QuadrantSignin";
}

void UIScene_QuadrantSignin::updateTooltips() {
    ui.SetTooltips(m_iPad, IDS_TOOLTIPS_CONTINUE, IDS_TOOLTIPS_CANCEL);
}

// Returns true if this scene has focus for the pad passed in
bool UIScene_QuadrantSignin::hasFocus(int iPad) {
    // Allow input from any controller
    return bHasFocus;
}

bool UIScene_QuadrantSignin::hidesLowerScenes() {
    // This is a Modal dialog, so don't need to hide the scene behind
    return false;
}

void UIScene_QuadrantSignin::tick() {
    if (!getMovie()) return;

    UIScene::tick();

    updateState();
}

void UIScene_QuadrantSignin::handleInput(int iPad, int key, bool repeat,
                                         bool pressed, bool released,
                                         bool& handled) {
    app.DebugPrintf(
        "UIScene_QuadrantSignin handling input for pad %d, key %d, repeat- %s, "
        "pressed- %s, released- %s\n",
        iPad, key, repeat ? "true" : "false", pressed ? "true" : "false",
        released ? "true" : "false");

    if (!m_bIgnoreInput) {
        ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

        switch (key) {
            case ACTION_MENU_CANCEL: {
                if (pressed) {
                    {
                        m_bIgnoreInput = true;
                        m_signInInfo.Func(m_signInInfo.lpParam, false, iPad);
                        ProfileManager.CancelProfileAvatarRequest();

                        navigateBack();
                    }
                }
            } break;
            case ACTION_MENU_OK:
                if (pressed) {
                    m_bIgnoreInput = true;
                    if (ProfileManager.IsSignedIn(iPad)) {
                        app.DebugPrintf("Signed in pad pressed\n");
                        ProfileManager.CancelProfileAvatarRequest();

                        navigateBack();
                        m_signInInfo.Func(m_signInInfo.lpParam, true, m_iPad);
                    } else {
                        {
                            app.DebugPrintf("Non-signed in pad pressed\n");
                            ProfileManager.RequestSignInUI(
                                false, false, false, true, true,
                                &UIScene_QuadrantSignin::SignInReturned, this,
                                iPad);
                        }
                    }
                }
                break;
            case ACTION_MENU_UP:
            case ACTION_MENU_DOWN:
                if (pressed) {
                    sendInputToMovie(key, repeat, pressed, released);
                }
                break;
        }
    }

    handled = true;
}

int UIScene_QuadrantSignin::SignInReturned(void* pParam, bool bContinue,
                                           int iPad) {
    app.DebugPrintf("SignInReturned for pad %d\n", iPad);

    UIScene_QuadrantSignin* pClass = (UIScene_QuadrantSignin*)pParam;

    {
        pClass->m_bIgnoreInput = false;
        pClass->updateState();
    }

    return 0;
}

namespace {
int AvatarReturnedThunk(void* lpParam, std::uint8_t* thumbnailData,
                        unsigned int thumbnailBytes) {
    return UIScene_QuadrantSignin::AvatarReturned(lpParam, thumbnailData,
                                                  thumbnailBytes);
}
}  // namespace

void UIScene_QuadrantSignin::updateState() {
    for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
        if (ProfileManager.IsSignedIn(i) && InputManager.IsPadConnected(i)) {
            // app.DebugPrintf("Index %d is signed in, display name - '%s'\n",
            // i, ProfileManager.GetDisplayName(i).data());

            {
                setControllerState(i, eControllerStatus_PlayerDetails);
            }

            m_labelDisplayName[i].setLabel(ProfileManager.GetDisplayName(i));
            // m_buttonControllers[i].setLabel(app.GetString(IDS_TOOLTIPS_CONTINUE),i);

            if (!m_iconRequested[i]) {
                app.DebugPrintf(app.USER_SR, "Requesting avatar for %d\n", i);
                if (ProfileManager.GetProfileAvatar(i, &AvatarReturnedThunk,
                                                    this)) {
                    m_iconRequested[i] = true;
                    m_lastRequestedAvatar = i;
                }
            }
        } else if (InputManager.IsPadConnected(i)) {
            // app.DebugPrintf("Index %d is not signed in\n", i);

            setControllerState(i, eControllerStatus_PressToJoin);
            m_labelDisplayName[i].setLabel(L"");
            m_iconRequested[i] = false;
        } else {
            // app.DebugPrintf("Index %d is not connected\n", i);

            setControllerState(i, eControllerStatus_ConnectController);
            m_iconRequested[i] = false;
        }
    }
}

void UIScene_QuadrantSignin::setControllerState(int iPad,
                                                EControllerStatus state) {
    if (m_controllerStatus[iPad] != state) {
        m_controllerStatus[iPad] = state;

        IggyDataValue result;
        IggyDataValue value[2];
        value[0].type = IGGY_DATATYPE_number;
        value[0].number = iPad;

        value[1].type = IGGY_DATATYPE_number;
        value[1].number = (int)state;

        IggyResult out = IggyPlayerCallMethodRS(
            getMovie(), &result, IggyPlayerRootPath(getMovie()),
            m_funcSetControllerStatus, 2, value);
    }
}

int UIScene_QuadrantSignin::AvatarReturned(void* lpParam,
                                           std::uint8_t* pbThumbnail,
                                           unsigned int dwThumbnailBytes) {
    UIScene_QuadrantSignin* pClass = (UIScene_QuadrantSignin*)lpParam;
    app.DebugPrintf(app.USER_SR, "AvatarReturned callback\n");
    if (pbThumbnail != nullptr) {
        // 4J-JEV - Added to ensure each new texture gets a unique name.
        static unsigned int quadrantImageCount = 0;

        wchar_t iconName[32];
        swprintf(iconName, 32, L"quadrantImage%05d", quadrantImageCount++);

        pClass->registerSubstitutionTexture(iconName, pbThumbnail,
                                            dwThumbnailBytes, true);
        pClass->m_bitmapIcon[pClass->m_lastRequestedAvatar].setTextureName(
            iconName);
    }

    pClass->m_lastRequestedAvatar = -1;

    return 0;
}

void UIScene_QuadrantSignin::_initQuadrants() {
    for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
        m_iconRequested[i] = false;

        m_labelPressToJoin[i].init(IDS_MUST_SIGN_IN_TITLE);
        m_labelConnectController[i].init(L"");
        m_labelAccountType[i].init(L"");

        m_controllerStatus[i] = eControllerStatus_ConnectController;

        if (ProfileManager.IsSignedIn(i)) {
            app.DebugPrintf("Index %d is signed in\n", i);

            {
                setControllerState(i, eControllerStatus_PlayerDetails);
            }

            m_labelDisplayName[i].init(ProfileManager.GetDisplayName(i));
        } else if (InputManager.IsPadConnected(i)) {
            app.DebugPrintf("Index %d is not signed in\n", i);

            setControllerState(i, eControllerStatus_PressToJoin);
            m_labelDisplayName[i].init(L"");
        } else {
            app.DebugPrintf("Index %d is not connected\n", i);

            setControllerState(i, eControllerStatus_ConnectController);
        }
    }
}

void UIScene_QuadrantSignin::handleReload() { _initQuadrants(); }