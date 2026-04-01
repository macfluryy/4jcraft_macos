#include "UIComponent_Logo.h"

#include "4J.Render/4J_Render.h"
#include "Minecraft.Client/Common/src/UI/UILayer.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"

UIComponent_Logo::UIComponent_Logo(int iPad, void* initData,
                                   UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    // Setup all the Iggy references we need for this scene
    initialiseMovie();
}

std::wstring UIComponent_Logo::getMoviePath() {
    switch (m_parentLayer->getViewport()) {
        case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
        case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
        case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
        case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
        case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
        case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
        case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
        case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
            return L"ComponentLogoSplit";
            break;
        case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
        default:
            return L"ComponentLogo";
            break;
    }
}