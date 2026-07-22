#include "IUIScene_AbstractContainerMenu.h"

#include <assert.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "platform/InputActions.h"
#include "platform/sdl2/Input.h"
#include "platform/sdl2/Render.h"
#include "app/common/App_enums.h"
#include "app/common/src/Tutorial/Tutorial.h"
#include "app/common/src/Tutorial/TutorialMode.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/multiplayer/MultiPlayerGameMode.h"
#include "minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"
#include "minecraft/client/player/LocalPlayer.h"
#include "minecraft/sounds/SoundTypes.h"
#include "minecraft/util/HtmlString.h"
#include "minecraft/world/entity/player/Inventory.h"
#include "minecraft/world/inventory/AbstractContainerMenu.h"
#include "minecraft/world/inventory/Slot.h"
#include "minecraft/world/item/ArmorItem.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/item/Rarity.h"
#include "minecraft/world/item/crafting/ArmorRecipes.h"
#include "minecraft/world/item/crafting/FurnaceRecipes.h"
#include "minecraft/world/level/tile/entity/FurnaceTileEntity.h"
#include "minecraft/world/phys/Vec3.h"
#include "strings.h"

IUIScene_AbstractContainerMenu::IUIScene_AbstractContainerMenu() {
    m_menu = nullptr;
    m_autoDeleteMenu = false;
    m_lastPointerLabelSlot = nullptr;

    m_pointerPos.x = 0.0f;
    m_pointerPos.y = 0.0f;
}

IUIScene_AbstractContainerMenu::~IUIScene_AbstractContainerMenu() {
    
    
    
    
    if (m_autoDeleteMenu) delete m_menu;
}

void IUIScene_AbstractContainerMenu::Initialize(
    int iPad, AbstractContainerMenu* menu, bool autoDeleteMenu, int startIndex,
    ESceneSection firstSection, ESceneSection maxSection, bool bNavigateBack) {
    assert(menu != nullptr);

    m_menu = menu;
    m_autoDeleteMenu = autoDeleteMenu;

    Minecraft::GetInstance()->localplayers[iPad]->containerMenu = menu;

    
    
    
    for (int i = 0; i < eToolTipNumButtons; ++i) {
        m_aeToolTipSettings[i] = eToolTipNone;
    }
    
    
    SetToolTip(eToolTipButtonB, eToolTipExit);
    SetToolTip(eToolTipButtonA, eToolTipNone);
    SetToolTip(eToolTipButtonX, eToolTipNone);
    SetToolTip(eToolTipButtonY, eToolTipNone);

    
    m_bPointerOutsideMenu = false;

    
    m_eFirstSection = firstSection;
    m_eMaxSection = maxSection;

    m_iConsectiveInputTicks = 0;

    m_bNavigateBack = bNavigateBack;

    
#if defined(TAP_DETECTION)
    m_eCurrSection = firstSection;
    m_eCurrTapState = eTapStateNoInput;
    m_iCurrSlotX = 0;
    m_iCurrSlotY = 0;
#endif
    
    
    
    
    

    PlatformInitialize(iPad, startIndex);
}

int IUIScene_AbstractContainerMenu::GetSectionDimensions(ESceneSection eSection,
                                                         int* piNumColumns,
                                                         int* piNumRows) {
    if (IsSectionSlotList(eSection)) {
        *piNumRows = getSectionRows(eSection);
        *piNumColumns = getSectionColumns(eSection);
    } else {
        *piNumRows = 0;
        *piNumColumns = 0;
    }
    return ((*piNumRows) * (*piNumColumns));
}

void IUIScene_AbstractContainerMenu::updateSlotPosition(
    ESceneSection eSection, ESceneSection newSection, ETapState eTapDirection,
    int* piTargetX, int* piTargetY, int xOffset, int yOffset) {
    
    int columns, rows;

    
    
    
    GetSectionDimensions(newSection, &columns, &rows);

    if (newSection != eSection) {
        
        if (eTapDirection == eTapStateUp) {
            (*piTargetY) = rows - 1;
        } else if (eTapDirection == eTapStateDown) {
            (*piTargetY) = 0;
        }
        int offsetY = (*piTargetY) - yOffset;
        if (offsetY < 0) {
            (*piTargetY) = 0;
        } else if (offsetY >= rows) {
            (*piTargetY) = rows - 1;
        } else {
            (*piTargetY) = offsetY;
        }

        
        int offsetX = (*piTargetX) - xOffset;
        if (offsetX < 0) {
            *piTargetX = 0;
        } else if (offsetX >= columns) {
            *piTargetX = columns - 1;
        } else {
            *piTargetX = offsetX;
        }
    } else {
        
        int offsetX = (*piTargetX) - xOffset;
        if (offsetX < 0) {
            *piTargetX = columns - 1;
        } else if (offsetX >= columns) {
            *piTargetX = 0;
        } else {
            *piTargetX = offsetX;
        }
    }
}

#if defined(TAP_DETECTION)
IUIScene_AbstractContainerMenu::ETapState
IUIScene_AbstractContainerMenu::GetTapInputType(float fInputX, float fInputY) {
    if ((fabs(fInputX) < 0.3f) && (fabs(fInputY) < 0.3f)) {
        return eTapStateNoInput;
    } else if ((fInputX < -0.3f) && (fabs(fInputY) < 0.3f)) {
        return eTapStateLeft;
    } else if ((fInputX > 0.3f) && (fabs(fInputY) < 0.3f)) {
        return eTapStateRight;
    } else if ((fInputY < -0.3f) && (fabs(fInputX) < 0.3f)) {
        return eTapStateDown;
    } else if ((fInputY > 0.3f) && (fabs(fInputX) < 0.3f)) {
        return eTapStateUp;
    } else {
        return eTapNone;
    }
}
#endif

void IUIScene_AbstractContainerMenu::SetToolTip(EToolTipButton eButton,
                                                EToolTipItem eItem) {
    if (m_aeToolTipSettings[eButton] != eItem) {
        m_aeToolTipSettings[eButton] = eItem;
        UpdateTooltips();
    }
}

void IUIScene_AbstractContainerMenu::UpdateTooltips() {
    
    static const int kaToolTipextIds[eNumToolTips] = {
        IDS_TOOLTIPS_PICKUPPLACE,            
        IDS_TOOLTIPS_EXIT,                   
        IDS_TOOLTIPS_PICKUP_GENERIC,         
        IDS_TOOLTIPS_PICKUP_ALL,             
        IDS_TOOLTIPS_PICKUP_HALF,            
        IDS_TOOLTIPS_PLACE_GENERIC,          
        IDS_TOOLTIPS_PLACE_ONE,              
        IDS_TOOLTIPS_PLACE_ALL,              
        IDS_TOOLTIPS_DROP_GENERIC,           
        IDS_TOOLTIPS_DROP_ONE,               
        IDS_TOOLTIPS_DROP_ALL,               
        IDS_TOOLTIPS_SWAP,                   
        IDS_TOOLTIPS_QUICK_MOVE,             
        IDS_TOOLTIPS_QUICK_MOVE_INGREDIENT,  
        IDS_TOOLTIPS_QUICK_MOVE_FUEL,        
        IDS_TOOLTIPS_WHAT_IS_THIS,           
        IDS_TOOLTIPS_EQUIP,                  
        IDS_TOOLTIPS_CLEAR_QUICK_SELECT,     
        IDS_TOOLTIPS_QUICK_MOVE_TOOL,        
        IDS_TOOLTIPS_QUICK_MOVE_ARMOR,       
        IDS_TOOLTIPS_QUICK_MOVE_WEAPON,      
        IDS_TOOLTIPS_DYE,                    
        IDS_TOOLTIPS_REPAIR,                 
    };

    int focusUser = getPad();

    for (int i = 0; i < eToolTipNumButtons; ++i) {
        if (m_aeToolTipSettings[i] == eToolTipNone) {
            ui.ShowTooltip(focusUser, i, false);
        } else {
            ui.SetTooltipText(focusUser, i,
                              kaToolTipextIds[m_aeToolTipSettings[i]]);
            ui.ShowTooltip(focusUser, i, true);
        }
    }
}

void IUIScene_AbstractContainerMenu::onMouseTick() {
    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (pMinecraft->localgameModes[getPad()] != nullptr) {
        Tutorial* tutorial =
            pMinecraft->localgameModes[getPad()]->getTutorial();
        if (tutorial != nullptr) {
            if (ui.IsTutorialVisible(getPad()) &&
                !tutorial->isInputAllowed(ACTION_MENU_UP)) {
                return;
            }
        }
    }

    
    
    
    float fInputDirX = 0.0f;
    float fInputDirY = 0.0f;

    
    UIVec2D vPointerPos = m_pointerPos;

    
    vPointerPos.x += m_fPointerImageOffsetX;
    vPointerPos.y += m_fPointerImageOffsetY;

    
    int iPad = getPad();

    bool bStickInput = false;
    float fInputX =
        InputManager.GetJoypadStick_LX(iPad, false) *
        ((float)app.GetGameSettings(iPad, eGameSetting_Sensitivity_InMenu) /
         100.0f);  
    float fInputY =
        InputManager.GetJoypadStick_LY(iPad, false) *
        ((float)app.GetGameSettings(iPad, eGameSetting_Sensitivity_InMenu) /
         100.0f);  

    
    if ((fabs(fInputX) >= 0.01f) || (fabs(fInputY) >= 0.01f)) {
        fInputDirX = (fInputX > 0.0f) ? 1.0f : (fInputX < 0.0f) ? -1.0f : 0.0f;
        fInputDirY = (fInputY > 0.0f) ? 1.0f : (fInputY < 0.0f) ? -1.0f : 0.0f;

#if defined(TAP_DETECTION)
        
        ETapState eNewTapInput = GetTapInputType(fInputX, fInputY);

        switch (m_eCurrTapState) {
            case eTapStateNoInput:
                m_eCurrTapState = eNewTapInput;
                break;

            case eTapStateUp:
            case eTapStateDown:
            case eTapStateLeft:
            case eTapStateRight:
                if ((eNewTapInput != m_eCurrTapState) &&
                    (eNewTapInput != eTapStateNoInput)) {
                    
                    m_eCurrTapState = eTapNone;
                }
                break;

            case eTapNone:
                
                break;
            default:
                break;
        }
#endif

        
        fInputX = fInputX * fInputX * fInputDirX * POINTER_SPEED_FACTOR;
        fInputY = fInputY * fInputY * fInputDirY * POINTER_SPEED_FACTOR;
        
        
        float fInputScale = 1.0f;

        
        
        
        if (m_iConsectiveInputTicks < MAX_INPUT_TICKS_FOR_SCALING) {
            ++m_iConsectiveInputTicks;
            fInputScale = ((float)(m_iConsectiveInputTicks) /
                           (float)(MAX_INPUT_TICKS_FOR_SCALING));
        }
#if defined(TAP_DETECTION)
        else if (m_iConsectiveInputTicks < MAX_INPUT_TICKS_FOR_TAPPING) {
            ++m_iConsectiveInputTicks;
        } else {
            m_eCurrTapState = eTapNone;
        }
#endif
        
        
        
        if (!RenderManager.IsHiDef() || app.GetLocalPlayerCount() > 1)
            fInputScale *= 0.6f;

        fInputX *= fInputScale;
        fInputY *= fInputScale;

#if defined(USE_POINTER_ACCEL)
        m_fPointerAccelX += fInputX / 50.0f;
        m_fPointerAccelY += fInputY / 50.0f;

        if (fabsf(fInputX) > fabsf(m_fPointerVelX + m_fPointerAccelX)) {
            m_fPointerVelX += m_fPointerAccelX;
        } else {
            m_fPointerAccelX = fInputX - m_fPointerVelX;
            m_fPointerVelX = fInputX;
        }

        if (fabsf(fInputY) > fabsf(m_fPointerVelY + m_fPointerAccelY)) {
            m_fPointerVelY += m_fPointerAccelY;
        } else {
            m_fPointerAccelY = fInputY - m_fPointerVelY;
            m_fPointerVelY = fInputY;
        }
        
        

        vPointerPos.x += m_fPointerVelX;
        vPointerPos.y -= m_fPointerVelY;
#else
        
        vPointerPos.x += fInputX;
        vPointerPos.y -= fInputY;
#endif
        
        if (vPointerPos.x < m_fPointerMinX)
            vPointerPos.x = m_fPointerMinX;
        else if (vPointerPos.x > m_fPointerMaxX)
            vPointerPos.x = m_fPointerMaxX;
        if (vPointerPos.y < m_fPointerMinY)
            vPointerPos.y = m_fPointerMinY;
        else if (vPointerPos.y > m_fPointerMaxY)
            vPointerPos.y = m_fPointerMaxY;

        bStickInput = true;
    } else {
        m_iConsectiveInputTicks = 0;
#if defined(USE_POINTER_ACCEL)
        m_fPointerVelX = 0.0f;
        m_fPointerVelY = 0.0f;
        m_fPointerAccelX = 0.0f;
        m_fPointerAccelY = 0.0f;
#endif
    }

    
    ESceneSection eSectionUnderPointer = eSectionNone;
    int iNewSlotX = -1;
    int iNewSlotY = -1;
    int iNewSlotIndex = -1;
    bool bPointerIsOverSlot = false;

    
    Vec3 vSnapPos;

    for (int iSection = m_eFirstSection; iSection < m_eMaxSection; ++iSection) {
        
        
        if (m_eCurrTapState == eTapStateJump) {
            eSectionUnderPointer = m_eCurrSection;
        } else if (eSectionUnderPointer == eSectionNone) {
            ESceneSection eSection = (ESceneSection)(iSection);

            
            UIVec2D sectionPos;
            GetPositionOfSection(eSection, &(sectionPos));

            if (!IsSectionSlotList(eSection)) {
                UIVec2D itemPos;
                UIVec2D itemSize;
                GetItemScreenData(eSection, 0, &(itemPos), &(itemSize));

                UIVec2D itemMax = itemSize;
                itemMax += itemPos;

                if ((vPointerPos.x >= sectionPos.x) &&
                    (vPointerPos.x <= itemMax.x) &&
                    (vPointerPos.y >= sectionPos.y) &&
                    (vPointerPos.y <= itemMax.y)) {
                    
                    eSectionUnderPointer = eSection;

                    vSnapPos.x = itemPos.x + (itemSize.x / 2.0f);
                    vSnapPos.y = itemPos.y + (itemSize.y / 2.0f);

                    
                    if (!doesSectionTreeHaveFocus(eSection)) {
                        
                        setSectionFocus(eSection, getPad());
                    }

                    bPointerIsOverSlot = false;

                    
                    
                    if ((eSectionUnderPointer != m_eCurrSection) ||
                        (iNewSlotX != m_iCurrSlotX) ||
                        (iNewSlotY != m_iCurrSlotY)) {
                        m_eCurrTapState = eTapNone;
                    }

                    
                    m_eCurrSection = eSectionUnderPointer;
                }
            } else {
                
                int iNumRows;
                int iNumColumns;
                int iNumItems =
                    GetSectionDimensions(eSection, &(iNumColumns), &(iNumRows));

                
                for (int iItem = 0; iItem < iNumItems; ++iItem) {
                    UIVec2D itemPos;
                    UIVec2D itemSize;
                    GetItemScreenData(eSection, iItem, &(itemPos), &(itemSize));

                    itemPos += sectionPos;

                    UIVec2D itemMax = itemSize;
                    itemMax += itemPos;

                    if ((vPointerPos.x >= itemPos.x) &&
                        (vPointerPos.x <= itemMax.x) &&
                        (vPointerPos.y >= itemPos.y) &&
                        (vPointerPos.y <= itemMax.y)) {
                        
                        eSectionUnderPointer = eSection;
                        iNewSlotIndex = iItem;
                        iNewSlotX = iNewSlotIndex % iNumColumns;
                        iNewSlotY = iNewSlotIndex / iNumColumns;

                        vSnapPos.x = itemPos.x + (itemSize.x / 2.0f);
                        vSnapPos.y = itemPos.y + (itemSize.y / 2.0f);

                        
                        if (!doesSectionTreeHaveFocus(eSection)) {
                            
                            setSectionFocus(eSection, getPad());
                        }

                        
                        setSectionSelectedSlot(eSection, iNewSlotX, iNewSlotY);

                        bPointerIsOverSlot = true;

#if defined(TAP_DETECTION)
                        
                        
                        if ((eSectionUnderPointer != m_eCurrSection) ||
                            (iNewSlotX != m_iCurrSlotX) ||
                            (iNewSlotY != m_iCurrSlotY)) {
                            m_eCurrTapState = eTapNone;
                        }

                        
                        m_eCurrSection = eSectionUnderPointer;
                        m_iCurrSlotX = iNewSlotX;
                        m_iCurrSlotY = iNewSlotY;
#endif
                        
                        
                        break;
                    }
                }
            }
        }
    }

    
    if (!IsVisible(eSectionUnderPointer)) eSectionUnderPointer = eSectionNone;

    
    if (eSectionUnderPointer == eSectionNone) {
        setFocusToPointer(getPad());
#if defined(TAP_DETECTION)
        
        m_eCurrTapState = eTapNone;

        
        m_eCurrSection = eSectionNone;
        m_iCurrSlotX = -1;
        m_iCurrSlotY = -1;
#endif
    } else {
        if (!bStickInput) {
            
            int iDesiredSlotX = -1;
            int iDesiredSlotY = -1;

            switch (m_eCurrTapState) {
                case eTapStateUp:
                    iDesiredSlotX = m_iCurrSlotX;
                    iDesiredSlotY = m_iCurrSlotY - 1;
                    break;
                case eTapStateDown:
                    iDesiredSlotX = m_iCurrSlotX;
                    iDesiredSlotY = m_iCurrSlotY + 1;
                    break;
                case eTapStateLeft:
                    iDesiredSlotX = m_iCurrSlotX - 1;
                    iDesiredSlotY = m_iCurrSlotY;
                    break;
                case eTapStateRight:
                    iDesiredSlotX = m_iCurrSlotX + 1;
                    iDesiredSlotY = m_iCurrSlotY;
                    break;
                case eTapStateJump:
                    iDesiredSlotX = m_iCurrSlotX;
                    iDesiredSlotY = m_iCurrSlotY;
                    break;
                default:
                    break;
            }

            int iNumRows;
            int iNumColumns;
            int iNumItems = GetSectionDimensions(eSectionUnderPointer,
                                                 &(iNumColumns), &(iNumRows));

            if ((m_eCurrTapState != eTapNone &&
                 m_eCurrTapState != eTapStateNoInput) &&
                (!IsSectionSlotList(eSectionUnderPointer) ||
                 ((iDesiredSlotX < 0) || (iDesiredSlotX >= iNumColumns) ||
                  (iDesiredSlotY < 0) || (iDesiredSlotY >= iNumRows)))) {
                eSectionUnderPointer = GetSectionAndSlotInDirection(
                    eSectionUnderPointer, m_eCurrTapState, &iDesiredSlotX,
                    &iDesiredSlotY);

                if (!IsSectionSlotList(eSectionUnderPointer))
                    bPointerIsOverSlot = false;

                
                iNumItems = GetSectionDimensions(eSectionUnderPointer,
                                                 &(iNumColumns), &(iNumRows));
            }

            if (!IsSectionSlotList(eSectionUnderPointer) ||
                ((iDesiredSlotX >= 0) && (iDesiredSlotX < iNumColumns) &&
                 (iDesiredSlotY >= 0) && (iDesiredSlotY < iNumRows))) {
                
                
                UIVec2D sectionPos;
                GetPositionOfSection(eSectionUnderPointer, &(sectionPos));

                iNewSlotIndex = (iDesiredSlotY * iNumColumns) + iDesiredSlotX;

                UIVec2D itemPos;
                UIVec2D itemSize;
                GetItemScreenData(eSectionUnderPointer, iNewSlotIndex,
                                  &(itemPos), &(itemSize));

                if (IsSectionSlotList(eSectionUnderPointer))
                    itemPos += sectionPos;

                vSnapPos.x = itemPos.x + (itemSize.x / 2.0f);
                vSnapPos.y = itemPos.y + (itemSize.y / 2.0f);

                m_eCurrSection = eSectionUnderPointer;
                m_iCurrSlotX = iDesiredSlotX;
                m_iCurrSlotY = iDesiredSlotY;
            }

            m_eCurrTapState = eTapStateNoInput;

            
            
            
            if (CanHaveFocus(eSectionUnderPointer)) {
                vPointerPos.x = vSnapPos.x;
                vPointerPos.y = vSnapPos.y;
            }
        }
    }

    
    if (vPointerPos.x < m_fPointerMinX)
        vPointerPos.x = m_fPointerMinX;
    else if (vPointerPos.x > m_fPointerMaxX)
        vPointerPos.x = m_fPointerMaxX;
    if (vPointerPos.y < m_fPointerMinY)
        vPointerPos.y = m_fPointerMinY;
    else if (vPointerPos.y > m_fPointerMaxY)
        vPointerPos.y = m_fPointerMaxY;

    
    bool bPointerIsOutsidePanel = false;
    if ((vPointerPos.x < m_fPanelMinX) || (vPointerPos.x > m_fPanelMaxX) ||
        (vPointerPos.y < m_fPanelMinY) || (vPointerPos.y > m_fPanelMaxY)) {
        bPointerIsOutsidePanel = true;
    }

    
    

    
    std::shared_ptr<LocalPlayer> player =
        Minecraft::GetInstance()->localplayers[getPad()];
    std::shared_ptr<ItemInstance> carriedItem = nullptr;
    if (player != nullptr) carriedItem = player->inventory->getCarried();

    std::shared_ptr<ItemInstance> slotItem = nullptr;
    Slot* slot = nullptr;
    int slotIndex = 0;
    if (bPointerIsOverSlot) {
        slotIndex = iNewSlotIndex + getSectionStartOffset(eSectionUnderPointer);
        slot = m_menu->getSlot(slotIndex);
    }
    bool bIsItemCarried = carriedItem != nullptr;
    int iCarriedCount = 0;
    bool bCarriedIsSameAsSlot = false;  
                                        
    if (bIsItemCarried) {
        iCarriedCount = carriedItem->count;
    }

    
    bool bSlotHasItem = false;
    bool bMayPlace = false;
    bool bCanPlaceOne = false;
    bool bCanPlaceAll = false;
    bool bCanCombine = false;
    bool bCanDye = false;
    int iSlotCount = 0;
    int iSlotStackSizeRemaining =
        0;  
    if (bPointerIsOverSlot) {
        slotItem = slot->getItem();
        bSlotHasItem = slotItem != nullptr;
        if (bSlotHasItem) {
            iSlotCount = slotItem->GetCount();

            if (bIsItemCarried) {
                bCarriedIsSameAsSlot = IsSameItemAs(carriedItem, slotItem);
                bCanCombine = m_menu->mayCombine(slot, carriedItem);
                bCanDye = bCanCombine &&
                          dynamic_cast<ArmorItem*>(slot->getItem()->getItem());

                if (bCarriedIsSameAsSlot) {
                    iSlotStackSizeRemaining =
                        GetEmptyStackSpace(m_menu->getSlot(slotIndex));
                }
            }
        }

        if (bIsItemCarried) {
            bMayPlace = slot->mayPlace(carriedItem);

            if (bSlotHasItem)
                iSlotStackSizeRemaining = GetEmptyStackSpace(slot);
            else
                iSlotStackSizeRemaining = slot->getMaxStackSize();

            if (bMayPlace && iSlotStackSizeRemaining > 0) bCanPlaceOne = true;
            if (bMayPlace && iSlotStackSizeRemaining > 1 &&
                carriedItem->count > 1)
                bCanPlaceAll = true;
        }
    }

    if (bPointerIsOverSlot && bSlotHasItem) {
        std::vector<HtmlString>* desc = GetItemDescription(slot);
        SetPointerText(desc, slot != m_lastPointerLabelSlot);
        m_lastPointerLabelSlot = slot;
        delete desc;
    } else if (eSectionUnderPointer != eSectionNone &&
               !IsSectionSlotList(eSectionUnderPointer)) {
        std::vector<HtmlString>* desc =
            GetSectionHoverText(eSectionUnderPointer);
        SetPointerText(desc, false);
        m_lastPointerLabelSlot = nullptr;
        delete desc;
    } else {
        SetPointerText(nullptr, false);
        m_lastPointerLabelSlot = nullptr;
    }

    EToolTipItem buttonA, buttonX, buttonY, buttonRT, buttonBack;
    buttonA = buttonX = buttonY = buttonRT = buttonBack = eToolTipNone;
    if (bPointerIsOverSlot) {
        SetPointerOutsideMenu(false);
        if (bIsItemCarried) {
            if (bSlotHasItem) {
                
                
                if (bCarriedIsSameAsSlot) {
                    
                    if (iSlotStackSizeRemaining == 0) {
                        
                        buttonRT = eToolTipWhatIsThis;
                    } else if (iSlotStackSizeRemaining == 1) {
                        
                        buttonA = eToolTipPlaceGeneric;
                        buttonRT = eToolTipWhatIsThis;
                    } else  
                    {
                        if (bCanPlaceAll) {
                            
                            buttonA = eToolTipPlaceAll;
                            buttonX = eToolTipPlaceOne;
                        } else if (bCanPlaceOne) {
                            if (iCarriedCount > 1)
                                buttonA = eToolTipPlaceOne;
                            else
                                buttonA = eToolTipPlaceGeneric;
                        }
                        buttonRT = eToolTipWhatIsThis;
                    }
                } else  
                {
                    if (bMayPlace) buttonA = eToolTipSwap;
                    buttonRT = eToolTipWhatIsThis;
                }
                if (bCanDye) {
                    buttonX = eToolTipDye;
                } else if (bCanCombine) {
                    buttonX = eToolTipRepair;
                }
            } else  
            {
                
                if (iCarriedCount == 1) {
                    
                    buttonA = eToolTipPlaceGeneric;
                } else {
                    if (bCanPlaceAll) {
                        
                        buttonA = eToolTipPlaceAll;
                        buttonX = eToolTipPlaceOne;
                    } else if (bCanPlaceOne) {
                        buttonA = eToolTipPlaceOne;
                    }
                }
            }
        } else  
        {
            if (bSlotHasItem) {
                if (iSlotCount == 1) {
                    buttonA = eToolTipPickUpGeneric;
                } else {
                    
                    buttonA = eToolTipPickUpAll;
                    buttonX = eToolTipPickUpHalf;
                }

                {
                    buttonRT = eToolTipWhatIsThis;
                }
            } else {
                
            }
        }

        if (bSlotHasItem) {
            

            

            if ((eSectionUnderPointer == eSectionInventoryUsing) ||
                (eSectionUnderPointer == eSectionInventoryInventory)) {
                std::shared_ptr<ItemInstance> item =
                    getSlotItem(eSectionUnderPointer, iNewSlotIndex);
                ArmorRecipes::_eArmorType eArmourType =
                    ArmorRecipes::GetArmorType(item->id);

                if (eArmourType == ArmorRecipes::eArmorType_None) {
                    buttonY = eToolTipQuickMove;
                } else {
                    
                    switch (eArmourType) {
                        case ArmorRecipes::eArmorType_Helmet:
                            if (isSlotEmpty(eSectionInventoryArmor, 0)) {
                                buttonY = eToolTipEquip;
                            } else {
                                buttonY = eToolTipQuickMove;
                            }
                            break;
                        case ArmorRecipes::eArmorType_Chestplate:
                            if (isSlotEmpty(eSectionInventoryArmor, 1)) {
                                buttonY = eToolTipEquip;
                            } else {
                                buttonY = eToolTipQuickMove;
                            }
                            break;
                        case ArmorRecipes::eArmorType_Leggings:
                            if (isSlotEmpty(eSectionInventoryArmor, 2)) {
                                buttonY = eToolTipEquip;
                            } else {
                                buttonY = eToolTipQuickMove;
                            }
                            break;
                        case ArmorRecipes::eArmorType_Boots:
                            if (isSlotEmpty(eSectionInventoryArmor, 3)) {
                                buttonY = eToolTipEquip;
                            } else {
                                buttonY = eToolTipQuickMove;
                            }
                            break;
                        default:
                            buttonY = eToolTipQuickMove;
                            break;
                    }
                }
            }
            
            else if ((eSectionUnderPointer == eSectionFurnaceUsing) ||
                     (eSectionUnderPointer == eSectionFurnaceInventory)) {
                
                std::shared_ptr<ItemInstance> item =
                    getSlotItem(eSectionUnderPointer, iNewSlotIndex);
                bool bValidFuel = FurnaceTileEntity::isFuel(item);
                bool bValidIngredient =
                    FurnaceRecipes::getInstance()->getResult(
                        item->getItem()->id) != nullptr;

                if (bValidIngredient) {
                    
                    if (!isSlotEmpty(eSectionFurnaceIngredient, 0)) {
                        
                        std::shared_ptr<ItemInstance> IngredientItem =
                            getSlotItem(eSectionFurnaceIngredient, 0);
                        if (IngredientItem->id == item->id) {
                            buttonY = eToolTipQuickMoveIngredient;
                        } else {
                            if (FurnaceRecipes::getInstance()->getResult(
                                    item->id) == nullptr) {
                                buttonY = eToolTipQuickMove;
                            } else {
                                buttonY = eToolTipQuickMoveIngredient;
                            }
                        }
                    } else {
                        
                        buttonY = eToolTipQuickMoveIngredient;
                    }
                } else if (bValidFuel) {
                    
                    if (!isSlotEmpty(eSectionFurnaceFuel, 0)) {
                        
                        std::shared_ptr<ItemInstance> fuelItem =
                            getSlotItem(eSectionFurnaceFuel, 0);
                        if (fuelItem->id == item->id) {
                            buttonY = eToolTipQuickMoveFuel;
                        } else if (bValidIngredient) {
                            
                            
                            if (!isSlotEmpty(eSectionFurnaceIngredient, 0)) {
                                
                                std::shared_ptr<ItemInstance> IngredientItem =
                                    getSlotItem(eSectionFurnaceIngredient, 0);
                                if (IngredientItem->id == item->id) {
                                    buttonY = eToolTipQuickMoveIngredient;
                                } else {
                                    if (FurnaceRecipes::getInstance()
                                            ->getResult(item->id) == nullptr) {
                                        buttonY = eToolTipQuickMove;
                                    } else {
                                        buttonY = eToolTipQuickMoveIngredient;
                                    }
                                }
                            } else {
                                
                                buttonY = eToolTipQuickMoveIngredient;
                            }
                        } else {
                            buttonY = eToolTipQuickMove;
                        }
                    } else {
                        buttonY = eToolTipQuickMoveFuel;
                    }
                } else {
                    buttonY = eToolTipQuickMove;
                }
            }
            
            else if ((eSectionUnderPointer == eSectionBrewingUsing) ||
                     (eSectionUnderPointer == eSectionBrewingInventory)) {
                
                std::shared_ptr<ItemInstance> item =
                    getSlotItem(eSectionUnderPointer, iNewSlotIndex);
                int iId = item->id;

                
                bool bValidIngredient = false;
                

                if (Item::items[iId]->hasPotionBrewingFormula() ||
                    (iId == Item::netherwart_seeds_Id)) {
                    bValidIngredient = true;
                }

                if (bValidIngredient) {
                    
                    if (!isSlotEmpty(eSectionBrewingIngredient, 0)) {
                        
                        std::shared_ptr<ItemInstance> IngredientItem =
                            getSlotItem(eSectionBrewingIngredient, 0);
                        if (IngredientItem->id == item->id) {
                            buttonY = eToolTipQuickMoveIngredient;
                        } else {
                            buttonY = eToolTipQuickMove;
                        }
                    } else {
                        
                        buttonY = eToolTipQuickMoveIngredient;
                    }
                } else {
                    
                    
                    if (iId == Item::potion_Id) {
                        
                        if (isSlotEmpty(eSectionBrewingBottle1, 0) ||
                            isSlotEmpty(eSectionBrewingBottle2, 0) ||
                            isSlotEmpty(eSectionBrewingBottle3, 0)) {
                            buttonY = eToolTipQuickMoveIngredient;
                        } else {
                            buttonY = eToolTipNone;
                        }
                    } else {
                        buttonY = eToolTipQuickMove;
                    }
                }
            } else if ((eSectionUnderPointer == eSectionEnchantUsing) ||
                       (eSectionUnderPointer == eSectionEnchantInventory)) {
                
                std::shared_ptr<ItemInstance> item =
                    getSlotItem(eSectionUnderPointer, iNewSlotIndex);
                int iId = item->id;

                
                if (Item::items[iId]->isEnchantable(item)) {
                    
                    if (isSlotEmpty(eSectionEnchantSlot, 0)) {
                        
                        switch (iId) {
                            case Item::bow_Id:
                            case Item::sword_wood_Id:
                            case Item::sword_stone_Id:
                            case Item::sword_iron_Id:
                            case Item::sword_diamond_Id:
                                buttonY = eToolTipQuickMoveWeapon;
                                break;

                            case Item::helmet_leather_Id:
                            case Item::chestplate_leather_Id:
                            case Item::leggings_leather_Id:
                            case Item::boots_leather_Id:

                            case Item::helmet_chain_Id:
                            case Item::chestplate_chain_Id:
                            case Item::leggings_chain_Id:
                            case Item::boots_chain_Id:

                            case Item::helmet_iron_Id:
                            case Item::chestplate_iron_Id:
                            case Item::leggings_iron_Id:
                            case Item::boots_iron_Id:

                            case Item::helmet_diamond_Id:
                            case Item::chestplate_diamond_Id:
                            case Item::leggings_diamond_Id:
                            case Item::boots_diamond_Id:

                            case Item::helmet_gold_Id:
                            case Item::chestplate_gold_Id:
                            case Item::leggings_gold_Id:
                            case Item::boots_gold_Id:
                                buttonY = eToolTipQuickMoveArmor;

                                break;
                            case Item::book_Id:
                                buttonY = eToolTipQuickMove;
                                break;
                            default:
                                buttonY = eToolTipQuickMoveTool;
                                break;
                        }
                    } else {
                        buttonY = eToolTipQuickMove;
                    }
                } else {
                    buttonY = eToolTipQuickMove;
                }
            } else {
                buttonY = eToolTipQuickMove;
            }
        }
    }

    if (bPointerIsOutsidePanel) {
        SetPointerOutsideMenu(true);
        
        if (bIsItemCarried) {
            
            
            if (iCarriedCount > 1) {
                buttonA = eToolTipDropAll;
                buttonX = eToolTipDropOne;
            } else {
                buttonA = eToolTipDropGeneric;
            }
        }
    } else  
    {
        SetPointerOutsideMenu(false);
    }

    std::shared_ptr<ItemInstance> item = nullptr;
    if (bPointerIsOverSlot && bSlotHasItem)
        item = getSlotItem(eSectionUnderPointer, iNewSlotIndex);
    overrideTooltips(eSectionUnderPointer, item, bIsItemCarried, bSlotHasItem,
                     bCarriedIsSameAsSlot, iSlotStackSizeRemaining, buttonA,
                     buttonX, buttonY, buttonRT, buttonBack);

    SetToolTip(eToolTipButtonA, buttonA);
    SetToolTip(eToolTipButtonX, buttonX);
    SetToolTip(eToolTipButtonY, buttonY);
    SetToolTip(eToolTipButtonRT, buttonRT);
    SetToolTip(eToolTipButtonBack, buttonBack);

    
    vPointerPos.x -= m_fPointerImageOffsetX;
    vPointerPos.y -= m_fPointerImageOffsetY;

    
    
    

    
    
    if (fInputDirX != 0.0f) {
        if (fInputDirX == 1.0f) {
            vPointerPos.x += 0.999999f;
        } else {
            vPointerPos.x -= 0.999999f;
        }
    }

    if (fInputDirY != 0.0f) {
        if (fInputDirY == 1.0f) {
            vPointerPos.y += 0.999999f;
        } else {
            vPointerPos.y -= 0.999999f;
        }
    }

    vPointerPos.x = floor(vPointerPos.x);
    vPointerPos.x += ((int)vPointerPos.x % 2);
    vPointerPos.y = floor(vPointerPos.y);
    vPointerPos.y += ((int)vPointerPos.y % 2);
    m_pointerPos = vPointerPos;

    adjustPointerForSafeZone();
}

bool IUIScene_AbstractContainerMenu::handleKeyDown(int iPad, int iAction,
                                                   bool bRepeat) {
    bool bHandled = false;

    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (pMinecraft->localgameModes[getPad()] != nullptr) {
        Tutorial* tutorial =
            pMinecraft->localgameModes[getPad()]->getTutorial();
        if (tutorial != nullptr) {
            tutorial->handleUIInput(iAction);
            if (ui.IsTutorialVisible(getPad()) &&
                !tutorial->isInputAllowed(iAction)) {
                return 0;
            }
        }
    }

    ui.AnimateKeyPress(iPad, iAction, bRepeat, true, false);

    int buttonNum = 0;          
    bool quickKeyHeld = false;  

    bool validKeyPress = false;
    bool itemEditorKeyPress = false;

    
    

    switch (iAction) {
#if defined(_DEBUG_MENUS_ENABLED)
        case ACTION_MENU_OTHER_STICK_PRESS:
            itemEditorKeyPress = true;
            break;
#endif
        case ACTION_MENU_A:
            if (!bRepeat) {
                validKeyPress = true;

                
                buttonNum = 0;
                quickKeyHeld = false;
                ui.PlayUISFX(eSFX_Press);
            }
            break;
        case ACTION_MENU_X:
            if (!bRepeat) {
                validKeyPress = true;

                
                buttonNum = 1;
                quickKeyHeld = false;
                ui.PlayUISFX(eSFX_Press);
            }
            break;
        case ACTION_MENU_Y:
            if (!bRepeat) {
                
                

                
                

                
                
                
                
                
                {
                    validKeyPress = true;

                    
                    buttonNum = 0;
                    quickKeyHeld = true;
                    ui.PlayUISFX(eSFX_Press);
                }
            }
            break;
            
            
        case ACTION_MENU_PAUSEMENU:
        case ACTION_MENU_B: {
            ui.SetTooltips(iPad, -1);

            
            
            
            
            
            
            
            

            
            
            

            if (m_bNavigateBack) {
                ui.NavigateBack(iPad);
            } else {
                ui.CloseUIScenes(iPad);
            }

            bHandled = true;
            return 0;
        } break;
        case ACTION_MENU_LEFT: {
            
            m_eCurrTapState = eTapStateLeft;
        } break;
        case ACTION_MENU_RIGHT: {
            
            m_eCurrTapState = eTapStateRight;
        } break;
        case ACTION_MENU_UP: {
            
            m_eCurrTapState = eTapStateUp;
        } break;
        case ACTION_MENU_DOWN: {
            
            m_eCurrTapState = eTapStateDown;
        } break;
        case ACTION_MENU_PAGEUP: {
            
            bHandled = true;
        } break;
        case ACTION_MENU_PAGEDOWN: {
            if (IsSectionSlotList(m_eCurrSection)) {
                int currentIndex = getCurrentIndex(m_eCurrSection) -
                                   getSectionStartOffset(m_eCurrSection);

                bool bSlotHasItem = !isSlotEmpty(m_eCurrSection, currentIndex);
                if (bSlotHasItem) {
                    std::shared_ptr<ItemInstance> item =
                        getSlotItem(m_eCurrSection, currentIndex);
                    if (Minecraft::GetInstance()->localgameModes[iPad] !=
                        nullptr) {
                        Tutorial::PopupMessageDetails* message =
                            new Tutorial::PopupMessageDetails;
                        message->m_messageId = item->getUseDescriptionId();

                        if (Item::items[item->id] != nullptr)
                            message->m_titleString =
                                Item::items[item->id]->getHoverName(item);
                        message->m_titleId = item->getDescriptionId();

                        message->m_icon = item->id;
                        message->m_iAuxVal = item->getAuxValue();
                        message->m_forceDisplay = true;

                        TutorialMode* gameMode =
                            (TutorialMode*)Minecraft::GetInstance()
                                ->localgameModes[iPad];
                        gameMode->getTutorial()->setMessage(nullptr, message);
                        ui.PlayUISFX(eSFX_Press);
                    }
                }
            }
            bHandled = true;
        } break;
    };

    if (validKeyPress) {
        if (handleValidKeyPress(iPad, buttonNum, quickKeyHeld)) {
            
        } else {
            if (IsSectionSlotList(m_eCurrSection)) {
                handleSlotListClicked(m_eCurrSection, buttonNum, quickKeyHeld);
            } else {
                
                

                
                
                
                if (m_bPointerOutsideMenu) {
                    handleOutsideClicked(iPad, buttonNum, quickKeyHeld);
                } else  
                {
                    
                    handleOtherClicked(iPad, m_eCurrSection, buttonNum,
                                       quickKeyHeld ? true : false);
                    
                }
            }
        }
        bHandled = true;
    }
#if defined(_DEBUG_MENUS_ENABLED)
    else if (itemEditorKeyPress == true) {
        if (IsSectionSlotList(m_eCurrSection)) {
            ItemEditorInput* initData = new ItemEditorInput();
            initData->iPad = getPad();
            initData->slot =
                getSlot(m_eCurrSection, getCurrentIndex(m_eCurrSection));
            initData->menu = m_menu;

            ui.NavigateToScene(getPad(), eUIScene_DebugItemEditor,
                               (void*)initData);
        }
    }
#endif
    else {
        handleAdditionalKeyPress(iAction);
    }

    UpdateTooltips();

    return bHandled;
}

bool IUIScene_AbstractContainerMenu::handleValidKeyPress(int iUserIndex,
                                                         int buttonNum,
                                                         bool quickKeyHeld) {
    return false;
}

void IUIScene_AbstractContainerMenu::handleOutsideClicked(int iPad,
                                                          int buttonNum,
                                                          bool quickKeyHeld) {
    

    
    
    
    slotClicked(AbstractContainerMenu::SLOT_CLICKED_OUTSIDE, buttonNum,
                quickKeyHeld ? true : false);
}

void IUIScene_AbstractContainerMenu::handleOtherClicked(int iPad,
                                                        ESceneSection eSection,
                                                        int buttonNum,
                                                        bool quickKey) {
    
}

void IUIScene_AbstractContainerMenu::handleAdditionalKeyPress(int iAction) {
    
}

void IUIScene_AbstractContainerMenu::handleSlotListClicked(
    ESceneSection eSection, int buttonNum, bool quickKeyHeld) {
    int currentIndex = getCurrentIndex(eSection);

    
    
    
    slotClicked(currentIndex, buttonNum, quickKeyHeld ? true : false);

    handleSectionClick(eSection);
}

void IUIScene_AbstractContainerMenu::slotClicked(int slotId, int buttonNum,
                                                 bool quickKey) {
    
    

    Minecraft* pMinecraft = Minecraft::GetInstance();
    pMinecraft->localgameModes[getPad()]->handleInventoryMouseClick(
        m_menu->containerId, slotId, buttonNum, quickKey,
        pMinecraft->localplayers[getPad()]);
}

int IUIScene_AbstractContainerMenu::getCurrentIndex(ESceneSection eSection) {
    int rows, columns;
    GetSectionDimensions(eSection, &columns, &rows);
    int currentIndex = (m_iCurrSlotY * columns) + m_iCurrSlotX;

    return currentIndex + getSectionStartOffset(eSection);
}

bool IUIScene_AbstractContainerMenu::IsSameItemAs(
    std::shared_ptr<ItemInstance> itemA, std::shared_ptr<ItemInstance> itemB) {
    if (itemA == nullptr || itemB == nullptr) return false;

    return (itemA->id == itemB->id &&
            (!itemB->isStackedByData() ||
             itemB->getAuxValue() == itemA->getAuxValue()) &&
            ItemInstance::tagMatches(itemB, itemA));
}

int IUIScene_AbstractContainerMenu::GetEmptyStackSpace(Slot* slot) {
    int iResult = 0;

    if (slot != nullptr && slot->hasItem()) {
        std::shared_ptr<ItemInstance> item = slot->getItem();
        if (item->isStackable()) {
            int iCount = item->GetCount();
            int iMaxStackSize =
                std::min(item->getMaxStackSize(), slot->getMaxStackSize());

            iResult = iMaxStackSize - iCount;

            if (iResult < 0) iResult = 0;
        }
    }

    return iResult;
}

std::vector<HtmlString>* IUIScene_AbstractContainerMenu::GetItemDescription(
    Slot* slot) {
    if (slot == nullptr) return nullptr;

    std::vector<HtmlString>* lines =
        slot->getItem()->getHoverText(nullptr, false);

    
    if (lines->size() > 0) {
        lines->at(0).color = slot->getItem()->getRarity()->color;

        if (slot->getItem()->hasCustomHoverName()) {
            lines->at(0).color = eTextColor_RenamedItemTitle;
        }
    }

    return lines;
}

std::vector<HtmlString>* IUIScene_AbstractContainerMenu::GetSectionHoverText(
    ESceneSection eSection) {
    return nullptr;
}