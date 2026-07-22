
#include "UIScene_InGameSaveManagementMenu.h"

int UIScene_InGameSaveManagementMenu::loadSaveDataThumbnailReturned(
    std::uint8_t* pbThumbnail, unsigned int dwThumbnailBytes) {
    app.DebugPrintf("Received data for save thumbnail\n");

    if (pbThumbnail && dwThumbnailBytes) {
        m_saveDetails[m_iRequestingThumbnailId].pbThumbnailData =
            new std::uint8_t[dwThumbnailBytes];
        memcpy(m_saveDetails[m_iRequestingThumbnailId].pbThumbnailData,
               pbThumbnail, dwThumbnailBytes);
        m_saveDetails[m_iRequestingThumbnailId].dwThumbnailSize =
            dwThumbnailBytes;
    } else {
        m_saveDetails[m_iRequestingThumbnailId].pbThumbnailData = nullptr;
        m_saveDetails[m_iRequestingThumbnailId].dwThumbnailSize = 0;
        app.DebugPrintf("Save thumbnail data is nullptr, or has size 0\n");
    }
    m_bSaveThumbnailReady = true;

    return 0;
}

UIScene_InGameSaveManagementMenu::UIScene_InGameSaveManagementMenu(
    int iPad, void* initData, UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    
    initialiseMovie();

    m_iRequestingThumbnailId = 0;
    m_iSaveInfoC = 0;
    m_bIgnoreInput = false;
    m_iState = e_SavesIdle;
    

    m_buttonListSaves.init(eControl_SavesList);

    m_labelSavesListTitle.init(app.GetString(IDS_SAVE_INCOMPLETE_DELETE_SAVES));
    m_controlSavesTimer.setVisible(true);

    m_bUpdateSaveSize = false;

    m_bAllLoaded = false;
    m_bRetrievingSaveThumbnails = false;
    m_bSaveThumbnailReady = false;
    m_bExitScene = false;
    m_pSaveDetails = nullptr;
    m_bSavesDisplayed = false;
    m_saveDetails = nullptr;
    m_iSaveDetailsCount = 0;

    
    
    if (app.StartInstallDLCProcess(m_iPad) == true || app.DLCInstallPending()) {
        
        
        m_bIgnoreInput = true;
    } else {
        Initialise();
    }

    
    
    
    if (!m_bIgnoreInput) {
        app.m_dlcManager.checkForCorruptDLCAndAlert();
    }

    parentLayer->addComponent(iPad, eUIComponent_MenuBackground);
}

UIScene_InGameSaveManagementMenu::~UIScene_InGameSaveManagementMenu() {
    m_parentLayer->removeComponent(eUIComponent_MenuBackground);

    if (m_saveDetails) {
        for (int i = 0; i < m_iSaveDetailsCount; ++i) {
            delete m_saveDetails[i].pbThumbnailData;
        }
        delete[] m_saveDetails;
    }
    app.unlockSaveNotification();
    StorageManager.SetSaveDisabled(false);
    StorageManager.ContinueIncompleteOperation();
}

void UIScene_InGameSaveManagementMenu::updateTooltips() {
    int iA = -1;
    if (m_bSavesDisplayed && m_iSaveDetailsCount > 0) {
        iA = IDS_TOOLTIPS_DELETESAVE;
    }
    ui.SetTooltips(
        m_parentLayer->IsFullscreenGroup() ? XUSER_INDEX_ANY : m_iPad, iA,
        IDS_SAVE_INCOMPLETE_RETRY_SAVING);
}


void UIScene_InGameSaveManagementMenu::Initialise() {
    m_iSaveListIndex = 0;

    if (StorageManager.GetSaveDisabled()) {
        GetSaveInfo();
    } else {
        
        
        bool bCanRename = StorageManager.EnoughSpaceForAMinSaveGame();

        GetSaveInfo();
    }

    m_bIgnoreInput = false;
}

void UIScene_InGameSaveManagementMenu::handleReload() {
    m_bIgnoreInput = false;
    m_iRequestingThumbnailId = 0;
    m_bAllLoaded = false;
    m_bRetrievingSaveThumbnails = false;
    m_bSavesDisplayed = false;
    m_iSaveInfoC = 0;
}

void UIScene_InGameSaveManagementMenu::handleGainFocus(bool navBack) {
    UIScene::handleGainFocus(navBack);

    updateTooltips();

    if (navBack) {
        
        m_bIgnoreInput = false;
    }
}

std::wstring UIScene_InGameSaveManagementMenu::getMoviePath() {
    return L"SaveMenu";
}

void UIScene_InGameSaveManagementMenu::tick() {
    UIScene::tick();

    if (m_bExitScene)  
    {
        if (!m_bRetrievingSaveThumbnails) {
            
            navigateBack();
        }
    }
    
    if (hasFocus(m_iPad)) {
        if (m_bUpdateSaveSize) {
            m_spaceIndicatorSaves.selectSave(m_iSaveListIndex);
            m_bUpdateSaveSize = false;
        }

        
        if (!m_bSavesDisplayed) {
            m_pSaveDetails = StorageManager.ReturnSavesInfo();
            if (m_pSaveDetails != nullptr) {
                m_spaceIndicatorSaves.reset();

                m_bSavesDisplayed = true;

                if (m_saveDetails != nullptr) {
                    for (unsigned int i = 0; i < m_pSaveDetails->iSaveC; ++i) {
                        if (m_saveDetails[i].pbThumbnailData != nullptr) {
                            delete m_saveDetails[i].pbThumbnailData;
                        }
                    }
                    delete m_saveDetails;
                }
                m_saveDetails = new SaveListDetails[m_pSaveDetails->iSaveC];

                m_iSaveDetailsCount = m_pSaveDetails->iSaveC;
                for (unsigned int i = 0; i < m_pSaveDetails->iSaveC; ++i) {
                    m_buttonListSaves.addItem(
                        m_pSaveDetails->SaveInfoA[i].UTF8SaveTitle, L"");

                    m_saveDetails[i].saveId = i;
                    memcpy(m_saveDetails[i].UTF8SaveName,
                           m_pSaveDetails->SaveInfoA[i].UTF8SaveTitle, 128);
                    memcpy(m_saveDetails[i].UTF8SaveFilename,
                           m_pSaveDetails->SaveInfoA[i].UTF8SaveFilename,
                           MAX_SAVEFILENAME_LENGTH);
                }
                m_controlSavesTimer.setVisible(false);

                
            }
        }

        if (!m_bExitScene && m_bSavesDisplayed &&
            !m_bRetrievingSaveThumbnails && !m_bAllLoaded) {
            if (m_iRequestingThumbnailId < (m_buttonListSaves.getItemCount())) {
                m_bRetrievingSaveThumbnails = true;
                app.DebugPrintf("Requesting the first thumbnail\n");
                
                PSAVE_DETAILS pSaveDetails = StorageManager.ReturnSavesInfo();
                C4JStorage::ESaveGameState eLoadStatus =
                    StorageManager.LoadSaveDataThumbnail(
                        &pSaveDetails->SaveInfoA[(int)m_iRequestingThumbnailId],
                        [this](std::uint8_t* data, unsigned int bytes) {
                            return loadSaveDataThumbnailReturned(data, bytes);
                        });

                if (eLoadStatus != C4JStorage::ESaveGame_GetSaveThumbnail) {
                    
                    m_bRetrievingSaveThumbnails = false;
                    m_bAllLoaded = true;
                }
            }
        } else if (m_bSavesDisplayed && m_bSaveThumbnailReady) {
            m_bSaveThumbnailReady = false;

            
            if (!m_bExitScene) {
                
                std::uint16_t u16Message[MAX_SAVEFILENAME_LENGTH];
#if defined(_WINDOWS64)
                int result = ::MultiByteToWideChar(
                    CP_UTF8,               
                    MB_ERR_INVALID_CHARS,  
                    m_saveDetails[m_iRequestingThumbnailId]
                        .UTF8SaveFilename,    
                    MAX_SAVEFILENAME_LENGTH,  
                                              
                    
                    (wchar_t*)u16Message,    
                    MAX_SAVEFILENAME_LENGTH  
                                             
                );
#else
                uint32_t srcmax, dstmax;
                uint32_t srclen, dstlen;
                srcmax = MAX_SAVEFILENAME_LENGTH;
                dstmax = MAX_SAVEFILENAME_LENGTH;

                SceCesUcsContext context;
                sceCesUcsContextInit(&context);

                sceCesUtf8StrToUtf16Str(
                    &context,
                    (uint8_t*)m_saveDetails[m_iRequestingThumbnailId]
                        .UTF8SaveFilename,
                    srcmax, &srclen, u16Message, dstmax, &dstlen);
#endif
                if (m_saveDetails[m_iRequestingThumbnailId].pbThumbnailData) {
                    registerSubstitutionTexture(
                        (wchar_t*)u16Message,
                        m_saveDetails[m_iRequestingThumbnailId].pbThumbnailData,
                        m_saveDetails[m_iRequestingThumbnailId]
                            .dwThumbnailSize);
                }
                m_buttonListSaves.setTextureName(m_iRequestingThumbnailId,
                                                 (wchar_t*)u16Message);

                ++m_iRequestingThumbnailId;
                if (m_iRequestingThumbnailId <
                    (m_buttonListSaves.getItemCount())) {
                    app.DebugPrintf("Requesting another thumbnail\n");
                    
                    PSAVE_DETAILS pSaveDetails =
                        StorageManager.ReturnSavesInfo();
                    C4JStorage::ESaveGameState eLoadStatus =
                        StorageManager.LoadSaveDataThumbnail(
                            &pSaveDetails
                                 ->SaveInfoA[(int)m_iRequestingThumbnailId],
                            [this](std::uint8_t* data, unsigned int bytes) {
                                return loadSaveDataThumbnailReturned(data,
                                                                     bytes);
                            });
                    if (eLoadStatus != C4JStorage::ESaveGame_GetSaveThumbnail) {
                        
                        m_bRetrievingSaveThumbnails = false;
                        m_bAllLoaded = true;
                    }
                } else {
                    m_bRetrievingSaveThumbnails = false;
                    m_bAllLoaded = true;
                }
            } else {
                
                m_bRetrievingSaveThumbnails = false;
            }
        }
    }

    switch (m_iState) {
        case e_SavesIdle:
            break;
        case e_SavesRepopulateAfterDelete:
            m_bIgnoreInput = false;
            m_iRequestingThumbnailId = 0;
            m_bAllLoaded = false;
            m_bRetrievingSaveThumbnails = false;
            m_bSavesDisplayed = false;
            m_iSaveInfoC = 0;
            m_buttonListSaves.clearList();
            
            
            m_iState = e_SavesIdle;
            break;
    }
}

void UIScene_InGameSaveManagementMenu::GetSaveInfo() {
    unsigned int uiSaveC = 0;

    

    
    m_bSavesDisplayed =
        false;  
    m_buttonListSaves.clearList();
    m_iSaveInfoC = 0;
    m_controlSavesTimer.setVisible(true);

    m_pSaveDetails = StorageManager.ReturnSavesInfo();
    if (m_pSaveDetails == nullptr) {
        C4JStorage::ESaveGameState eSGIStatus =
            StorageManager.GetSavesInfo(m_iPad, nullptr, (char*)"save");
    }

    return;
}

void UIScene_InGameSaveManagementMenu::handleInput(int iPad, int key,
                                                   bool repeat, bool pressed,
                                                   bool released,
                                                   bool& handled) {
    if (m_bIgnoreInput) return;

    
    if (!m_bSavesDisplayed) return;

    ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_CANCEL:
            if (pressed) {
                navigateBack();
                handled = true;
            }
            break;
        case ACTION_MENU_OK:
        case ACTION_MENU_UP:
        case ACTION_MENU_DOWN:
        case ACTION_MENU_PAGEUP:
        case ACTION_MENU_PAGEDOWN:
            sendInputToMovie(key, repeat, pressed, released);
            handled = true;
            break;
    }
}

void UIScene_InGameSaveManagementMenu::handleInitFocus(F64 controlId,
                                                       F64 childId) {
    app.DebugPrintf(
        app.USER_SR,
        "UIScene_InGameSaveManagementMenu::handleInitFocus - %d , %d\n",
        (int)controlId, (int)childId);
}

void UIScene_InGameSaveManagementMenu::handleFocusChange(F64 controlId,
                                                         F64 childId) {
    app.DebugPrintf(
        app.USER_SR,
        "UIScene_InGameSaveManagementMenu::handleFocusChange - %d , %d\n",
        (int)controlId, (int)childId);
    m_iSaveListIndex = childId;
    if (m_bSavesDisplayed) m_bUpdateSaveSize = true;
    updateTooltips();
}

void UIScene_InGameSaveManagementMenu::handlePress(F64 controlId, F64 childId) {
    switch ((int)controlId) {
        case eControl_SavesList: {
            m_bIgnoreInput = true;

            
            
            
            unsigned int uiIDA[2];
            uiIDA[0] = IDS_CONFIRM_CANCEL;
            uiIDA[1] = IDS_CONFIRM_OK;
            ui.RequestErrorMessage(
                IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE, uiIDA, 2, m_iPad,
                &UIScene_InGameSaveManagementMenu::DeleteSaveDialogReturned,
                this);

            ui.PlayUISFX(eSFX_Press);
            break;
        }
    }
}

int UIScene_InGameSaveManagementMenu::DeleteSaveDialogReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    UIScene_InGameSaveManagementMenu* pClass =
        (UIScene_InGameSaveManagementMenu*)pParam;
    

    if (result == C4JStorage::EMessage_ResultDecline) {
        if (app.DebugSettingsOn() && app.GetLoadSavesFromFolderEnabled()) {
            pClass->m_bIgnoreInput = false;
        } else {
            StorageManager.DeleteSaveData(
                &pClass->m_pSaveDetails->SaveInfoA[pClass->m_iSaveListIndex],
                [pClass](const bool bRes) {
                    return pClass->deleteSaveDataReturned(bRes);
                });
            pClass->m_controlSavesTimer.setVisible(true);
        }
    } else {
        pClass->m_bIgnoreInput = false;
    }

    return 0;
}

int UIScene_InGameSaveManagementMenu::deleteSaveDataReturned(bool bRes) {
    if (bRes) {
        
        m_iState = e_SavesRepopulateAfterDelete;
    } else
        m_bIgnoreInput = false;

    updateTooltips();

    return 0;
}

bool UIScene_InGameSaveManagementMenu::hasFocus(int iPad) {
    return bHasFocus && (iPad == m_iPad || m_iPad == XUSER_INDEX_ANY);
}
