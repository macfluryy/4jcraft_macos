#pragma once

#include "../Common/UI/IUIController.h"

class ConsoleUIController : public IUIController
{
public:
	virtual void init();
	virtual void tick();
	virtual void render();
	virtual void StartReloadSkinThread();
	virtual bool IsReloadingSkin();
	virtual void CleanUpSkinReload();
	virtual bool NavigateToScene(int iPad, EUIScene scene, void *initData = NULL, EUILayer layer = eUILayer_Scene, EUIGroup group = eUIGroup_PAD);
	virtual bool NavigateBack(int iPad, bool forceUsePad = false, EUIScene eScene = eUIScene_COUNT, EUILayer eLayer = eUILayer_COUNT);
	virtual void NavigateToHomeMenu();
	virtual void CloseUIScenes(int iPad, bool forceIPad = false);
	virtual void CloseAllPlayersScenes();
	
	virtual bool IsPauseMenuDisplayed(int iPad);
	virtual bool IsContainerMenuDisplayed(int iPad);
	virtual bool IsIgnorePlayerJoinMenuDisplayed(int iPad);
	virtual bool IsIgnoreAutosaveMenuDisplayed(int iPad);
	virtual void SetIgnoreAutosaveMenuDisplayed(int iPad, bool displayed);
	virtual bool IsSceneInStack(int iPad, EUIScene eScene);
	virtual bool GetMenuDisplayed(int iPad);
	virtual void CheckMenuDisplayed();
	virtual void AnimateKeyPress(int iPad, int dwKeyCode);

	virtual void SetTooltipText( unsigned int iPad, unsigned int tooltip, int iTextID );
	virtual void SetEnableTooltips( unsigned int iPad, bool bVal );
	virtual void ShowTooltip( unsigned int iPad, unsigned int tooltip, bool show );
	virtual void SetTooltips( unsigned int iPad, int iA, int iB=-1, int iX=-1, int iY=-1 , int iLT=-1, int iRT=-1, int iLB=-1, int iRB=-1, int iLS=-1, bool forceUpdate = false);
	virtual void EnableTooltip( unsigned int iPad, unsigned int tooltip, bool enable );
	virtual void RefreshTooltips(unsigned int iPad);

	virtual void PlayUISFX(ESoundEffect eSound);

	virtual void DisplayGamertag(unsigned int iPad, bool show);
	virtual void SetSelectedItem(unsigned int iPad, const std::wstring &name);
	virtual void UpdateSelectedItemPos(unsigned int iPad);

	virtual void HandleDLCMountingComplete();
	virtual void HandleDLCInstalled(int iPad);
	virtual void HandleTMSDLCFileRetrieved(int iPad);
	virtual void HandleTMSBanFileRetrieved(int iPad);
	virtual void HandleInventoryUpdated(int iPad);
	virtual void HandleGameTick();

	virtual void SetTutorialDescription(int iPad, TutorialPopupInfo *info);
	virtual void SetTutorialVisible(int iPad, bool visible);
	virtual bool IsTutorialVisible(int iPad);

	virtual void UpdatePlayerBasePositions();
	virtual void SetEmptyQuadrantLogo(int iSection);
	virtual void HideAllGameUIElements();
	virtual void ShowOtherPlayersBaseScene(unsigned int iPad, bool show);

	virtual void ShowTrialTimer(bool show);
	virtual void SetTrialTimerLimitSecs(unsigned int uiSeconds);
	virtual void UpdateTrialTimer(unsigned int iPad);
	virtual void ReduceTrialTimerValue();

	virtual void ShowAutosaveCountdownTimer(bool show);
	virtual void UpdateAutosaveCountdownTimer(unsigned int uiSeconds);
	virtual void ShowSavingMessage(unsigned int iPad, C4JStorage::ESavingMessage eVal);

	virtual bool PressStartPlaying(unsigned int iPad);
	virtual void ShowPressStart(unsigned int iPad);

	virtual void SetWinUserIndex(unsigned int iPad);

	virtual C4JStorage::EMessageResult RequestMessageBox(unsigned int uiTitle, unsigned int uiText, unsigned int *uiOptionA, unsigned int uiOptionC, unsigned int dwPad = XUSER_INDEX_ANY,
						int( *Func)(void *,int,const C4JStorage::EMessageResult)=NULL, void *lpParam=NULL, CXuiStringTable *pStringTable=NULL, wchar_t *pwchFormatString=NULL, unsigned int dwFocusButton=0, bool bIsError = true);

	C4JStorage::EMessageResult RequestUGCMessageBox(int title = -1, int message = -1, int iPad = -1, int( *Func)(void *,int,const C4JStorage::EMessageResult) = NULL, void *lpParam = NULL);
};

extern ConsoleUIController ui;
