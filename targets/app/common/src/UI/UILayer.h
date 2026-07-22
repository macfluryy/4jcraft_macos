#pragma once
#include <stdint.h>

#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

#include "platform/sdl2/Render.h"
#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/mac/Iggy/include/rrCore.h"


class UIScene;
class UIGroup;


class UILayer {
private:
    std::vector<UIScene*>
        m_sceneStack;  
                       
    std::vector<UIScene*>
        m_components;  
                       
    std::vector<UIScene*> m_scenesToDelete;  
    std::vector<UIScene*>
        m_scenesToDestroy;  

    std::unordered_map<EUIScene, std::pair<int, bool>> m_componentRefCount;

public:
    bool m_hasFocus;  
                      
    bool m_bMenuDisplayed;
    bool m_bPauseMenuDisplayed;
    bool m_bContainerMenuDisplayed;
    bool m_bIgnoreAutosaveMenuDisplayed;
    bool m_bIgnorePlayerJoinMenuDisplayed;

    UIGroup* m_parentGroup;

public:
    UILayer(UIGroup* parent);

    void tick();
    void render(S32 width, S32 height, C4JRender::eViewportType viewport);
    void getRenderDimensions(S32& width, S32& height);

    void DestroyAll();
    void ReloadAll(bool force = false);

    
    bool NavigateToScene(int iPad, EUIScene scene, void* initData);
    bool NavigateBack(int iPad, EUIScene eScene);
    void removeScene(UIScene* scene);
    void closeAllScenes();
    UIScene* GetTopScene();

    bool GetMenuDisplayed();
    bool IsPauseMenuDisplayed() { return m_bPauseMenuDisplayed; }

    bool IsSceneInStack(EUIScene scene);
    bool HasFocus(int iPad);

    bool hidesLowerScenes();

    
    
    
    
    void showComponent(int iPad, EUIScene scene, bool show);
    bool isComponentVisible(EUIScene scene);
    UIScene* addComponent(int iPad, EUIScene scene, void* initData = nullptr);
    void removeComponent(EUIScene scene);

    
    void handleInput(int iPad, int key, bool repeat, bool pressed,
                     bool released, bool& handled);
    

    bool updateFocusState(bool allowedFocus = false);

public:
    bool IsFullscreenGroup();
    C4JRender::eViewportType getViewport();

    virtual void HandleDLCMountingComplete();
    virtual void HandleDLCInstalled();
    virtual void HandleMessage(EUIMessage message, void* data);

    void handleUnlockFullVersion();
    UIScene* FindScene(EUIScene sceneType);

    void PrintTotalMemoryUsage(int64_t& totalStatic, int64_t& totalDynamic);
};
