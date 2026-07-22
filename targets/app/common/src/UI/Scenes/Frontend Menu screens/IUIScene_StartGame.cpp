#include "IUIScene_StartGame.h"

#include <wchar.h>

#include <cstdint>

#include "platform/sdl2/Profile.h"
#include "app/common/App_structs.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/UI/Controls/UIControl_BitmapIcon.h"
#include "app/common/src/UI/Controls/UIControl_Label.h"
#include "app/common/src/UI/Controls/UIControl_TexturePackList.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/Iggy/include/rrCore.h"
#include "app/mac/MacGame.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/skins/TexturePack.h"
#include "minecraft/client/skins/TexturePackRepository.h"

class UILayer;

IUIScene_StartGame::IUIScene_StartGame(int iPad, UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    m_bIgnoreInput = false;
    m_iTexturePacksNotInstalled = 0;
    m_texturePackDescDisplayed = false;
    m_bShowTexturePackDescription = false;
    m_iSetTexturePackDescription = -1;

    Minecraft* pMinecraft = Minecraft::GetInstance();
    m_currentTexturePackIndex = pMinecraft->skins->getTexturePackIndex(0);
}

void IUIScene_StartGame::HandleDLCMountingComplete() {
    Minecraft* pMinecraft = Minecraft::GetInstance();
    
    m_texturePackList.clearSlots();

    int texturePacksCount = pMinecraft->skins->getTexturePackCount();

    for (unsigned int i = 0; i < texturePacksCount; ++i) {
        TexturePack* tp = pMinecraft->skins->getTexturePackByIndex(i);

        std::uint32_t imageBytes = 0;
        std::uint8_t* imageData = tp->getPackIcon(imageBytes);

        if (imageBytes > 0 && imageData) {
            wchar_t imageName[64];
            swprintf(imageName, 64, L"tpack%08x", tp->getId());
            registerSubstitutionTexture(imageName, imageData, imageBytes);
            m_texturePackList.addPack(i, imageName);
        }
    }

    m_iTexturePacksNotInstalled = 0;

    
    
    DLC_INFO* pDLCInfo = nullptr;

    
    bool bTexturePackAlreadyListed;
    bool bNeedToGetTPD = false;

    for (unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i) {
        bTexturePackAlreadyListed = false;
        uint64_t ull = app.GetDLCInfoTexturesFullOffer(i);
        pDLCInfo = app.GetDLCInfoForFullOfferID(ull);
        for (unsigned int i = 0; i < texturePacksCount; ++i) {
            TexturePack* tp = pMinecraft->skins->getTexturePackByIndex(i);
            if (pDLCInfo->iConfig == tp->getDLCParentPackId()) {
                bTexturePackAlreadyListed = true;
            }
        }
        if (bTexturePackAlreadyListed == false) {
            
            bNeedToGetTPD = true;

            m_iTexturePacksNotInstalled++;
        }
    }

#if TO_BE_IMPLEMENTED
    if (bNeedToGetTPD == true) {
        
        app.DebugPrintf("+++ Adding TMSPP request for texture pack data\n");
        app.AddTMSPPFileTypeRequest(e_DLC_TexturePackData);
        if (m_iConfigA != nullptr) {
            delete m_iConfigA;
        }
        m_iConfigA = new int[m_iTexturePacksNotInstalled];
        m_iTexturePacksNotInstalled = 0;

        for (unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i) {
            bTexturePackAlreadyListed = false;
            uint64_t ull = app.GetDLCInfoTexturesFullOffer(i);
            pDLCInfo = app.GetDLCInfoForFullOfferID(ull);
            for (unsigned int i = 0; i < texturePacksCount; ++i) {
                TexturePack* tp = pMinecraft->skins->getTexturePackByIndex(i);
                if (pDLCInfo->iConfig == tp->getDLCParentPackId()) {
                    bTexturePackAlreadyListed = true;
                }
            }
            if (bTexturePackAlreadyListed == false) {
                m_iConfigA[m_iTexturePacksNotInstalled++] = pDLCInfo->iConfig;
            }
        }
    }
#endif
    m_currentTexturePackIndex = pMinecraft->skins->getTexturePackIndex(0);
    UpdateTexturePackDescription(m_currentTexturePackIndex);

    m_texturePackList.selectSlot(m_currentTexturePackIndex);
    m_bIgnoreInput = false;
    app.m_dlcManager.checkForCorruptDLCAndAlert();
}

void IUIScene_StartGame::handleSelectionChanged(F64 selectedId) {
    m_iSetTexturePackDescription = (int)selectedId;

    if (!m_texturePackDescDisplayed) {
        m_bShowTexturePackDescription = true;
    }
}

void IUIScene_StartGame::UpdateTexturePackDescription(int index) {
    TexturePack* tp =
        Minecraft::GetInstance()->skins->getTexturePackByIndex(index);

    if (tp == nullptr) {
#if TO_BE_IMPLEMENTED
        

        unsigned int dwBytes = 0;
        unsigned int dwFileBytes = 0;
        std::uint8_t* pbData = nullptr;
        std::uint8_t* pbFileData = nullptr;

        CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
        
        ListItem = m_pTexturePacksList->GetData(index);

        app.GetTPD(ListItem.iData, &pbData, &dwBytes);

        app.GetFileFromTPD(eTPDFileType_Loc, pbData, dwBytes, &pbFileData,
                           &dwFileBytes);
        if (dwFileBytes > 0 && pbFileData) {
            StringTable* pStringTable =
                new StringTable(pbFileData, dwFileBytes);
            m_texturePackTitle.SetText(
                pStringTable->getString(L"IDS_DISPLAY_NAME"));
            m_texturePackDescription.SetText(
                pStringTable->getString(L"IDS_TP_DESCRIPTION"));
        }

        app.GetFileFromTPD(eTPDFileType_Icon, pbData, dwBytes, &pbFileData,
                           &dwFileBytes);
        if (dwFileBytes > 0 && pbFileData) {
            XuiCreateTextureBrushFromMemory(pbFileData, dwFileBytes,
                                            &m_hTexturePackIconBrush);
            m_texturePackIcon->UseBrush(m_hTexturePackIconBrush);
        }
        app.GetFileFromTPD(eTPDFileType_Comparison, pbData, dwBytes,
                           &pbFileData, &dwFileBytes);
        if (dwFileBytes > 0 && pbFileData) {
            XuiCreateTextureBrushFromMemory(pbFileData, dwFileBytes,
                                            &m_hTexturePackComparisonBrush);
            m_texturePackComparison->UseBrush(m_hTexturePackComparisonBrush);
        } else {
            m_texturePackComparison->UseBrush(nullptr);
        }
#endif
    } else {
        m_labelTexturePackName.setLabel(tp->getName());
        m_labelTexturePackDescription.setLabel(tp->getDesc1());

        std::uint32_t imageBytes = 0;
        std::uint8_t* imageData = tp->getPackIcon(imageBytes);

        
        
        
        
        
        

        wchar_t imageName[64];
        swprintf(imageName, 64, L"tpack%08x", tp->getId());
        m_bitmapTexturePackIcon.setTextureName(imageName);

        imageData = tp->getPackComparison(imageBytes);

        if (imageBytes > 0 && imageData) {
            swprintf(imageName, 64, L"texturePackComparison%08x", tp->getId());
            registerSubstitutionTexture(imageName, imageData, imageBytes);
            m_bitmapComparison.setTextureName(imageName);
        } else {
            m_bitmapComparison.setTextureName(L"");
        }
    }
}

void IUIScene_StartGame::UpdateCurrentTexturePack(int iSlot) {
    m_currentTexturePackIndex = iSlot;
    TexturePack* tp = Minecraft::GetInstance()->skins->getTexturePackByIndex(
        m_currentTexturePackIndex);

    
    if (tp == nullptr) {
#if TO_BE_IMPLEMENTED
        

        CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
        
        ListItem = m_pTexturePacksList->GetData(m_currentTexturePackIndex);

        
        
        uint64_t ullOfferID_Full;
        app.GetDLCFullOfferIDForPackID(ListItem.iData, &ullOfferID_Full);

        unsigned int uiIDA[3];

        uiIDA[0] = IDS_TEXTUREPACK_FULLVERSION;
        uiIDA[1] = IDS_TEXTURE_PACK_TRIALVERSION;
        uiIDA[2] = IDS_CONFIRM_CANCEL;

        
        ui.RequestErrorMessage(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE,
                               IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 3,
                               ProfileManager.GetPrimaryPad(),
                               & : TexturePackDialogReturned, this);

        
        
        m_MoreOptionsParams.dwTexturePack = ListItem.iData;
        return;
#endif
    } else {
        m_MoreOptionsParams.dwTexturePack = tp->getId();
    }
}

int IUIScene_StartGame::TrialTexturePackWarningReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    IUIScene_StartGame* pScene = (IUIScene_StartGame*)pParam;

    if (result == C4JStorage::EMessage_ResultAccept) {
        pScene->checkStateAndStartGame();
    } else {
        pScene->m_bIgnoreInput = false;
    }
    return 0;
}

int IUIScene_StartGame::UnlockTexturePackReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    IUIScene_StartGame* pScene = (IUIScene_StartGame*)pParam;

    if (result == C4JStorage::EMessage_ResultAccept) {
        if (ProfileManager.IsSignedIn(iPad)) {
            
            
        }
    } else {
    }

    pScene->m_bIgnoreInput = false;

    return 0;
}

int IUIScene_StartGame::TexturePackDialogReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    IUIScene_StartGame* pClass = (IUIScene_StartGame*)pParam;

    pClass->m_bIgnoreInput = false;
    return 0;
}
