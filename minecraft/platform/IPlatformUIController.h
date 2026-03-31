#pragma once

#include <string>

#include "IPlatformStorage.h"

// Forward declarations for game types used by the UI interface.
// Full definitions live in the Client module.
enum EUIScene : int;
enum EUILayer : int;
enum EUIGroup : int;
enum ESoundEffect : int;
struct TutorialPopupInfo;

class IPlatformUIController {
public:
    virtual ~IPlatformUIController() = default;

    virtual void tick() = 0;
    virtual void render() = 0;

    // Skin
    virtual void StartReloadSkinThread() = 0;
    virtual bool IsReloadingSkin() = 0;
    virtual void CleanUpSkinReload() = 0;

    // Navigation
    virtual bool NavigateToScene(int iPad, EUIScene scene,
                                 void* initData = nullptr,
                                 EUILayer layer = static_cast<EUILayer>(0),
                                 EUIGroup group = static_cast<EUIGroup>(0)) = 0;
    virtual bool NavigateBack(int iPad, bool forceUsePad = false,
                              EUIScene eScene = static_cast<EUIScene>(-1),
                              EUILayer eLayer = static_cast<EUILayer>(-1)) = 0;
    virtual void CloseUIScenes(int iPad, bool forceIPad = false) = 0;
    virtual void CloseAllPlayersScenes() = 0;

    // Menu state
    virtual bool IsPauseMenuDisplayed(int iPad) = 0;
    virtual bool IsContainerMenuDisplayed(int iPad) = 0;
    virtual bool IsIgnorePlayerJoinMenuDisplayed(int iPad) = 0;
    virtual bool IsIgnoreAutosaveMenuDisplayed(int iPad) = 0;
    virtual void SetIgnoreAutosaveMenuDisplayed(int iPad, bool displayed) = 0;
    virtual bool IsSceneInStack(int iPad, EUIScene eScene) = 0;
    virtual bool GetMenuDisplayed(int iPad) = 0;
    virtual void CheckMenuDisplayed() = 0;

    // Tooltips
    virtual void SetTooltipText(unsigned int iPad, unsigned int tooltip,
                                int iTextID) = 0;
    virtual void SetEnableTooltips(unsigned int iPad, bool bVal) = 0;
    virtual void ShowTooltip(unsigned int iPad, unsigned int tooltip,
                             bool show) = 0;
    virtual void SetTooltips(unsigned int iPad, int iA, int iB = -1,
                             int iX = -1, int iY = -1, int iLT = -1,
                             int iRT = -1, int iLB = -1, int iRB = -1,
                             int iLS = -1, int iRS = -1, int iBack = -1,
                             bool forceUpdate = false) = 0;
    virtual void EnableTooltip(unsigned int iPad, unsigned int tooltip,
                               bool enable) = 0;
    virtual void RefreshTooltips(unsigned int iPad) = 0;

    // Sound
    virtual void PlayUISFX(ESoundEffect eSound) = 0;

    // Debug
    virtual void ShowUIDebugConsole(bool show) {}
    virtual void ShowUIDebugMarketingGuide(bool show) {}

    // HUD
    virtual void DisplayGamertag(unsigned int iPad, bool show) = 0;
    virtual void SetSelectedItem(unsigned int iPad,
                                 const std::wstring& name) = 0;
    virtual void UpdateSelectedItemPos(unsigned int iPad) = 0;

    // Events
    virtual void HandleDLCMountingComplete() = 0;
    virtual void HandleDLCInstalled(int iPad) = 0;
    virtual void HandleTMSDLCFileRetrieved(int iPad) = 0;
    virtual void HandleTMSBanFileRetrieved(int iPad) = 0;
    virtual void HandleInventoryUpdated(int iPad) = 0;
    virtual void HandleGameTick() = 0;

    // Tutorial
    virtual void SetTutorialDescription(int iPad, TutorialPopupInfo* info) = 0;
    virtual void SetTutorialVisible(int iPad, bool visible) = 0;
    virtual bool IsTutorialVisible(int iPad) = 0;

    // Layout
    virtual void UpdatePlayerBasePositions() = 0;
    virtual void SetEmptyQuadrantLogo(int iSection) = 0;
    virtual void HideAllGameUIElements() = 0;
    virtual void ShowOtherPlayersBaseScene(unsigned int iPad, bool show) = 0;

    // Trial timer
    virtual void ShowTrialTimer(bool show) = 0;
    virtual void SetTrialTimerLimitSecs(unsigned int uiSeconds) = 0;
    virtual void UpdateTrialTimer(unsigned int iPad) = 0;
    virtual void ReduceTrialTimerValue() = 0;

    // Autosave
    virtual void ShowAutosaveCountdownTimer(bool show) = 0;
    virtual void UpdateAutosaveCountdownTimer(unsigned int uiSeconds) = 0;
    virtual void ShowSavingMessage(unsigned int iPad,
                                   IPlatformStorage::ESavingMessage eVal) = 0;

    // Start screen
    virtual bool PressStartPlaying(unsigned int iPad) = 0;
    virtual void ShowPressStart(unsigned int iPad) = 0;

    virtual void SetWinUserIndex(unsigned int iPad) = 0;
};
