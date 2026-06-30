#pragma once

#include <string>

#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/common/src/UI/UIController.h"
#include "app/mac/Iggy/include/iggy.h"
#include "app/mac/Iggy/include/rrCore.h"

class ConsoleUIController : public UIController {
public:
    void init(S32 w, S32 h);
    void render();
    void shutdown();

    void beginIggyCustomDraw4J(IggyCustomDrawCallbackRegion* region,
                               CustomDrawData*               customDrawRegion);

    virtual CustomDrawData* setupCustomDraw(UIScene*                      scene,
                                            IggyCustomDrawCallbackRegion* region);

    virtual CustomDrawData* calculateCustomDraw(
            IggyCustomDrawCallbackRegion* region);

    virtual void endCustomDraw(IggyCustomDrawCallbackRegion* region);
    virtual void SetTutorialDescription(int iPad, TutorialPopupInfo* info);

    struct TutorialOverlayState {
        std::wstring title;
        std::wstring desc;
        bool visible = false;
    };
    static TutorialOverlayState s_tutorialOverlay;

protected:
    virtual void setTileOrigin(S32 xPos, S32 yPos);

public:
    GDrawTexture* getSubstitutionTexture(int textureId);
    void          destroySubstitutionTexture(void*         destroyCallBackData,
                                             GDrawTexture* handle);

    static void handleUnlockFullVersionCallback();
};

extern ConsoleUIController ui;