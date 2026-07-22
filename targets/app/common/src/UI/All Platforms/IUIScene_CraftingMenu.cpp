#include "IUIScene_CraftingMenu.h"

#include <assert.h>
#include <string.h>
#include <wchar.h>

#include <string>
#include <vector>

#include "platform/InputActions.h"
#include "platform/sdl2/Profile.h"
#include "platform/sdl2/Render.h"
#include "app/common/App_enums.h"
#include "app/common/src/Console_Debug_enum.h"
#include "app/common/src/Tutorial/Tutorial.h"
#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/multiplayer/MultiPlayerGameMode.h"
#include "minecraft/client/player/LocalPlayer.h"
#include "minecraft/sounds/SoundTypes.h"
#include "minecraft/stats/GenericStats.h"
#include "minecraft/world/entity/player/Inventory.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/item/crafting/Recipes.h"
#include "minecraft/world/item/crafting/ShapedRecipy.h"
#include "minecraft/world/level/tile/Tile.h"
#include "strings.h"

Recipy::_eGroupType IUIScene_CraftingMenu::m_GroupTypeMapping4GridA
    [IUIScene_CraftingMenu::m_iMaxGroup2x2] = {
        Recipy::eGroupType_Structure, Recipy::eGroupType_Tool,
        Recipy::eGroupType_Food,      Recipy::eGroupType_Mechanism,
        Recipy::eGroupType_Transport, Recipy::eGroupType_Decoration,
};

Recipy::_eGroupType IUIScene_CraftingMenu::m_GroupTypeMapping9GridA
    [IUIScene_CraftingMenu::m_iMaxGroup3x3] = {
        Recipy::eGroupType_Structure,  Recipy::eGroupType_Tool,
        Recipy::eGroupType_Food,       Recipy::eGroupType_Armour,
        Recipy::eGroupType_Mechanism,  Recipy::eGroupType_Transport,
        Recipy::eGroupType_Decoration,
};

const wchar_t* IUIScene_CraftingMenu::m_GroupIconNameA[m_iMaxGroup3x3] = {
    L"Structures",  
    L"Tools",       
    L"Food",        
    L"Armour",      
    L"Mechanisms",  
    L"Transport",   
    L"Decoration",  
};

IUIScene_CraftingMenu::_eGroupTab
    IUIScene_CraftingMenu::m_GroupTabBkgMapping2x2A[m_iMaxGroup2x2] = {
        eGroupTab_Left,   eGroupTab_Middle, eGroupTab_Middle,
        eGroupTab_Middle, eGroupTab_Middle, eGroupTab_Right,
};

IUIScene_CraftingMenu::_eGroupTab
    IUIScene_CraftingMenu::m_GroupTabBkgMapping3x3A[m_iMaxGroup3x3] = {
        eGroupTab_Left,   eGroupTab_Middle, eGroupTab_Middle, eGroupTab_Middle,
        eGroupTab_Middle, eGroupTab_Middle, eGroupTab_Right,
};



































IUIScene_CraftingMenu::IUIScene_CraftingMenu() {
    m_iCurrentSlotHIndex = 0;
    m_iCurrentSlotVIndex = 1;

    for (int i = 0; i < m_iMaxHSlotC; i++) {
        CanBeMadeA[i].iCount = 0;
        CanBeMadeA[i].iItemBaseType = 0;
    }
    memset(CanBeMadeA, 0, sizeof(CANBEMADE) * m_iMaxHSlotC);
    m_iRecipeC = 0;
    m_iGroupIndex = 0;

    for (int i = 0; i < m_iMaxDisplayedVSlotC; i++) {
        iVSlotIndexA[i] = i;  
    }

    m_iDisplayDescription = DISPLAY_INVENTORY;
    m_iIngredientsC = 0;
}

const wchar_t* IUIScene_CraftingMenu::GetGroupNameText(int iGroupType) {
    switch (iGroupType) {
        case ShapedRecipy::eGroupType_Tool:
            return app.GetString(IDS_GROUPNAME_TOOLS);
        case ShapedRecipy::eGroupType_Food:
            return app.GetString(IDS_GROUPNAME_FOOD);
        case ShapedRecipy::eGroupType_Structure:
            return app.GetString(IDS_GROUPNAME_STRUCTURES);
        case ShapedRecipy::eGroupType_Armour:
            return app.GetString(IDS_GROUPNAME_ARMOUR);
        case ShapedRecipy::eGroupType_Mechanism:
            return app.GetString(IDS_GROUPNAME_MECHANISMS);
        case ShapedRecipy::eGroupType_Transport:
            return app.GetString(IDS_GROUPNAME_TRANSPORT);
        case ShapedRecipy::eGroupType_Decoration:
        default:
            return app.GetString(IDS_GROUPNAME_DECORATIONS);
    }
}

bool IUIScene_CraftingMenu::handleKeyDown(int iPad, int iAction, bool bRepeat) {
    bool bHandled = false;

    if (m_bIgnoreKeyPresses) return bHandled;

    
    
    if (bRepeat && (iAction == ACTION_MENU_X)) {
        return 0;
    }

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

    switch (iAction) {
        case ACTION_MENU_X:

            
            m_iDisplayDescription++;
            if (m_iDisplayDescription == DISPLAY_MAX)
                m_iDisplayDescription = DISPLAY_INVENTORY;
            ui.PlayUISFX(eSFX_Focus);
            UpdateMultiPanel();
            UpdateTooltips();
            break;
        case ACTION_MENU_PAUSEMENU:
        case ACTION_MENU_B:
            ui.ShowTooltip(iPad, eToolTipButtonX, false);
            ui.ShowTooltip(iPad, eToolTipButtonB, false);
            ui.ShowTooltip(iPad, eToolTipButtonA, false);
            ui.ShowTooltip(iPad, eToolTipButtonRB, false);
            
            
            ui.CloseUIScenes(iPad);

            bHandled = true;
            break;
        case ACTION_MENU_A:
            
            if (m_pPlayer && m_pPlayer->inventory) {
                
                
                Recipy::INGREDIENTS_REQUIRED* pRecipeIngredientsRequired =
                    Recipes::getInstance()->getRecipeIngredientsArray();
                
                if (app.DebugSettingsOn() &&
                    app.GetGameSettingsDebugMask(
                        ProfileManager.GetPrimaryPad()) &
                        (1L << eDebugSetting_CraftAnything)) {
                    if (CanBeMadeA[m_iCurrentSlotHIndex].iCount != 0) {
                        int iSlot = iVSlotIndexA[m_iCurrentSlotVIndex];

                        int iRecipe =
                            CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
                        std::shared_ptr<ItemInstance> pTempItemInst =
                            pRecipeIngredientsRequired[iRecipe]
                                .pRecipy->assemble(nullptr);
                        
                        

                        if (pMinecraft->localgameModes[iPad] != nullptr) {
                            Tutorial* tutorial =
                                pMinecraft->localgameModes[iPad]->getTutorial();
                            if (tutorial != nullptr) {
                                tutorial->onCrafted(pTempItemInst);
                            }
                        }

                        pMinecraft->localgameModes[iPad]->handleCraftItem(
                            iRecipe, m_pPlayer);

                        if (m_pPlayer->inventory->add(pTempItemInst) == false) {
                            
                            m_pPlayer->drop(pTempItemInst);
                        }
                        
                        
                        
                        ui.PlayUISFX(eSFX_Craft);
                    }
                } else if (CanBeMadeA[m_iCurrentSlotHIndex].iCount != 0) {
                    int iSlot;
                    if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 1) {
                        iSlot = iVSlotIndexA[m_iCurrentSlotVIndex];
                    } else {
                        iSlot = 0;
                    }
                    int iRecipe =
                        CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
                    std::shared_ptr<ItemInstance> pTempItemInst =
                        pRecipeIngredientsRequired[iRecipe].pRecipy->assemble(
                            nullptr);
                    
                    

                    if (pMinecraft->localgameModes[iPad] != nullptr) {
                        Tutorial* tutorial =
                            pMinecraft->localgameModes[iPad]->getTutorial();
                        if (tutorial != nullptr) {
                            tutorial->createItemSelected(
                                pTempItemInst,
                                pRecipeIngredientsRequired[iRecipe]
                                    .bCanMake[iPad]);
                        }
                    }

                    if (pRecipeIngredientsRequired[iRecipe].bCanMake[iPad]) {
                        pTempItemInst->onCraftedBy(
                            m_pPlayer->level,
                            std::dynamic_pointer_cast<Player>(
                                m_pPlayer->shared_from_this()),
                            pTempItemInst->count);
                        
                        
                        
                        pMinecraft->localgameModes[iPad]->handleCraftItem(
                            iRecipe, m_pPlayer);

                        
                        
                        
                        ui.PlayUISFX(eSFX_Craft);

                        if (pTempItemInst->id != Item::fireworksCharge_Id &&
                            pTempItemInst->id != Item::fireworks_Id) {
                            
                            for (int i = 0;
                                 i < pRecipeIngredientsRequired[iRecipe].iIngC;
                                 i++) {
                                for (int j = 0;
                                     j < pRecipeIngredientsRequired[iRecipe]
                                             .iIngValA[i];
                                     j++) {
                                    std::shared_ptr<ItemInstance> ingItemInst =
                                        nullptr;
                                    
                                    
                                    if (pRecipeIngredientsRequired[iRecipe]
                                            .iIngAuxValA[i] !=
                                        Recipes::ANY_AUX_VALUE) {
                                        ingItemInst =
                                            m_pPlayer->inventory
                                                ->getResourceItem(
                                                    pRecipeIngredientsRequired
                                                        [iRecipe]
                                                            .iIngIDA[i],
                                                    pRecipeIngredientsRequired
                                                        [iRecipe]
                                                            .iIngAuxValA[i]);
                                        m_pPlayer->inventory->removeResource(
                                            pRecipeIngredientsRequired[iRecipe]
                                                .iIngIDA[i],
                                            pRecipeIngredientsRequired[iRecipe]
                                                .iIngAuxValA[i]);
                                    } else {
                                        ingItemInst =
                                            m_pPlayer->inventory
                                                ->getResourceItem(
                                                    pRecipeIngredientsRequired
                                                        [iRecipe]
                                                            .iIngIDA[i]);
                                        m_pPlayer->inventory->removeResource(
                                            pRecipeIngredientsRequired[iRecipe]
                                                .iIngIDA[i]);
                                    }

                                    
                                    
                                    if (ingItemInst != nullptr) {
                                        if (ingItemInst->getItem()
                                                ->hasCraftingRemainingItem()) {
                                            
                                            
                                            m_pPlayer->inventory->add(
                                                std::shared_ptr<
                                                    ItemInstance>(new ItemInstance(
                                                    ingItemInst->getItem()
                                                        ->getCraftingRemainingItem())));
                                        }
                                    }
                                }
                            }

                            
                            
                            if (m_pPlayer->inventory->add(pTempItemInst) ==
                                false) {
                                
                                m_pPlayer->drop(pTempItemInst);
                            }

                            
                            switch (pTempItemInst->id) {
                                case Tile::workBench_Id:
                                    m_pPlayer->awardStat(
                                        GenericStats::buildWorkbench(),
                                        GenericStats::param_buildWorkbench());
                                    break;
                                case Item::pickAxe_wood_Id:
                                    m_pPlayer->awardStat(
                                        GenericStats::buildPickaxe(),
                                        GenericStats::param_buildPickaxe());
                                    break;
                                case Tile::furnace_Id:
                                    m_pPlayer->awardStat(
                                        GenericStats::buildFurnace(),
                                        GenericStats::param_buildFurnace());
                                    break;
                                case Item::hoe_wood_Id:
                                    m_pPlayer->awardStat(
                                        GenericStats::buildHoe(),
                                        GenericStats::param_buildHoe());
                                    break;
                                case Item::bread_Id:
                                    m_pPlayer->awardStat(
                                        GenericStats::makeBread(),
                                        GenericStats::param_makeBread());
                                    break;
                                case Item::cake_Id:
                                    m_pPlayer->awardStat(
                                        GenericStats::bakeCake(),
                                        GenericStats::param_bakeCake());
                                    break;
                                case Item::pickAxe_stone_Id:
                                    m_pPlayer->awardStat(
                                        GenericStats::buildBetterPickaxe(),
                                        GenericStats::
                                            param_buildBetterPickaxe());
                                    break;
                                case Item::sword_wood_Id:
                                    m_pPlayer->awardStat(
                                        GenericStats::buildSword(),
                                        GenericStats::param_buildSword());
                                    break;
                                case Tile::dispenser_Id:
                                    m_pPlayer->awardStat(
                                        GenericStats::dispenseWithThis(),
                                        GenericStats::param_dispenseWithThis());
                                    break;
                                case Tile::enchantTable_Id:
                                    m_pPlayer->awardStat(
                                        GenericStats::enchantments(),
                                        GenericStats::param_enchantments());
                                    break;
                                case Tile::bookshelf_Id:
                                    m_pPlayer->awardStat(
                                        GenericStats::bookcase(),
                                        GenericStats::param_bookcase());
                                    break;
                            }

                            
                            
                            CheckRecipesAvailable();
                            
                            
                            UpdateVerticalSlots();
                            UpdateHighlight();
                        }
                    } else {
                        
                        
                        ui.PlayUISFX(eSFX_CraftFail);
                    }
                }
            }
            break;

        case ACTION_MENU_LEFT_SCROLL:
            
            showTabHighlight(m_iGroupIndex, false);

            if (m_iGroupIndex == 0) {
                if (m_iContainerType == RECIPE_TYPE_3x3) {
                    m_iGroupIndex = m_iMaxGroup3x3 - 1;
                } else {
                    m_iGroupIndex = m_iMaxGroup2x2 - 1;
                }
            } else {
                m_iGroupIndex--;
            }
            
            showTabHighlight(m_iGroupIndex, true);

            m_iCurrentSlotHIndex = 0;
            m_iCurrentSlotVIndex = 1;

            CheckRecipesAvailable();
            
            iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
            iVSlotIndexA[1] = 0;
            iVSlotIndexA[2] = 1;
            ui.PlayUISFX(eSFX_Focus);
            UpdateVerticalSlots();
            UpdateHighlight();
            setGroupText(GetGroupNameText(m_pGroupA[m_iGroupIndex]));

            break;
        case ACTION_MENU_RIGHT_SCROLL:
            
            showTabHighlight(m_iGroupIndex, false);

            m_iGroupIndex++;
            if (m_iContainerType == RECIPE_TYPE_3x3) {
                if (m_iGroupIndex == m_iMaxGroup3x3) m_iGroupIndex = 0;
            } else {
                if (m_iGroupIndex == m_iMaxGroup2x2) m_iGroupIndex = 0;
            }
            
            showTabHighlight(m_iGroupIndex, true);

            m_iCurrentSlotHIndex = 0;
            m_iCurrentSlotVIndex = 1;
            CheckRecipesAvailable();
            
            iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
            iVSlotIndexA[1] = 0;
            iVSlotIndexA[2] = 1;
            ui.PlayUISFX(eSFX_Focus);
            UpdateVerticalSlots();
            UpdateHighlight();
            setGroupText(GetGroupNameText(m_pGroupA[m_iGroupIndex]));
            break;
    }

    
    
    bool bNoScrollSlots = false;
    if (m_bSplitscreen ||
        (!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())) {
        bNoScrollSlots = true;
    }

    
    
    switch (iAction) {
        case ACTION_MENU_OTHER_STICK_UP:
            scrollDescriptionUp();
            break;
        case ACTION_MENU_OTHER_STICK_DOWN:
            scrollDescriptionDown();
            break;
        case ACTION_MENU_RIGHT: {
            int iOldHSlot = m_iCurrentSlotHIndex;

            m_iCurrentSlotHIndex++;
            if (m_iCurrentSlotHIndex >= m_iCraftablesMaxHSlotC)
                m_iCurrentSlotHIndex = 0;
            m_iCurrentSlotVIndex = 1;
            
            iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
            iVSlotIndexA[1] = 0;
            iVSlotIndexA[2] = 1;

            UpdateVerticalSlots();
            UpdateHighlight();
            
            if (CanBeMadeA[iOldHSlot].iCount > 0) {
                setShowCraftHSlot(iOldHSlot, true);
            }
            ui.PlayUISFX(eSFX_Focus);
            bHandled = true;
        } break;
        case ACTION_MENU_LEFT: {
            if (m_iCraftablesMaxHSlotC != 0) {
                int iOldHSlot = m_iCurrentSlotHIndex;
                if (m_iCurrentSlotHIndex == 0)
                    m_iCurrentSlotHIndex = m_iCraftablesMaxHSlotC - 1;
                else
                    m_iCurrentSlotHIndex--;
                m_iCurrentSlotVIndex = 1;
                
                iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
                iVSlotIndexA[1] = 0;
                iVSlotIndexA[2] = 1;

                UpdateVerticalSlots();
                UpdateHighlight();
                
                if (CanBeMadeA[iOldHSlot].iCount > 0) {
                    setShowCraftHSlot(iOldHSlot, true);
                }
                ui.PlayUISFX(eSFX_Focus);
            }
            bHandled = true;
        } break;
        case ACTION_MENU_UP: {
            if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 1) {
                if (bNoScrollSlots) {
                    if (iVSlotIndexA[1] == 0) {
                        iVSlotIndexA[1] =
                            CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
                    } else {
                        iVSlotIndexA[1]--;
                    }
                    ui.PlayUISFX(eSFX_Focus);
                } else if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 2) {
                    {
                        if (m_iCurrentSlotVIndex != 0) {
                            
                            m_iCurrentSlotVIndex--;
                            ui.PlayUISFX(eSFX_Focus);
                        } else {
                            
                            iVSlotIndexA[2] = iVSlotIndexA[1];
                            iVSlotIndexA[1] = iVSlotIndexA[0];
                            
                            if (iVSlotIndexA[0] == 0) {
                                iVSlotIndexA[0] =
                                    CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
                            } else {
                                iVSlotIndexA[0]--;
                            }
                            ui.PlayUISFX(eSFX_Focus);
                        }
                    }
                } else {
                    if (m_iCurrentSlotVIndex != 1) {
                        
                        m_iCurrentSlotVIndex--;
                        ui.PlayUISFX(eSFX_Focus);
                    }
                }
                UpdateVerticalSlots();
                UpdateHighlight();
            }

        } break;
        case ACTION_MENU_DOWN: {
            if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 1) {
                if (bNoScrollSlots) {
                    if (iVSlotIndexA[1] ==
                        (CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1)) {
                        iVSlotIndexA[1] = 0;
                    } else {
                        iVSlotIndexA[1]++;
                    }
                    ui.PlayUISFX(eSFX_Focus);

                } else if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 2) {
                    if (m_iCurrentSlotVIndex != 2) {
                        m_iCurrentSlotVIndex++;
                        ui.PlayUISFX(eSFX_Focus);
                    } else {
                        iVSlotIndexA[0] = iVSlotIndexA[1];
                        iVSlotIndexA[1] = iVSlotIndexA[2];
                        if (iVSlotIndexA[m_iCurrentSlotVIndex] ==
                            (CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1)) {
                            iVSlotIndexA[2] = 0;
                        } else {
                            iVSlotIndexA[2]++;
                        }
                        ui.PlayUISFX(eSFX_Focus);
                    }
                } else {
                    if (m_iCurrentSlotVIndex !=
                        (CanBeMadeA[m_iCurrentSlotHIndex].iCount)) {
                        m_iCurrentSlotVIndex++;
                        ui.PlayUISFX(eSFX_Focus);
                    }
                }
                UpdateVerticalSlots();
                UpdateHighlight();
            }
        } break;
    }

    return bHandled;
}






void IUIScene_CraftingMenu::CheckRecipesAvailable() {
    int iHSlotBrushControl = 0;

    
    memset(CanBeMadeA, 0, sizeof(CANBEMADE) * m_iCraftablesMaxHSlotC);

    hideAllHSlots();

    if (m_pPlayer && m_pPlayer->inventory) {
        
        













        std::vector<Recipy*>* recipes =
            ((Recipes*)Recipes::getInstance())->getRecipies();
        Recipy::INGREDIENTS_REQUIRED* pRecipeIngredientsRequired =
            Recipes::getInstance()->getRecipeIngredientsArray();
        int iRecipeC = (int)recipes->size();
        auto itRecipe = recipes->begin();

        

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        for (int i = 0; i < iRecipeC; i++) {
            Recipy* r = *itRecipe;

            
            if (r->getGroup() != m_pGroupA[m_iGroupIndex]) {
                itRecipe++;
                pRecipeIngredientsRequired[i].bCanMake[getPad()] = false;
                continue;
            }
            
            
            if ((m_iContainerType == RECIPE_TYPE_2x2) &&
                (pRecipeIngredientsRequired[i].iType == RECIPE_TYPE_3x3)) {
                
                itRecipe++;
                pRecipeIngredientsRequired[i].bCanMake[getPad()] = false;
                continue;
            }
            
            pRecipeIngredientsRequired[i]
                .usBitmaskMissingGridIngredients[getPad()] = 0;

            
            bool* bFoundA = new bool[pRecipeIngredientsRequired[i].iIngC];
            for (int j = 0; j < pRecipeIngredientsRequired[i].iIngC; j++) {
                bFoundA[j] = false;
                int iTotalCount = 0;

                
                for (unsigned int k = 0; k < m_pPlayer->inventory->items.size();
                     k++) {
                    if (m_pPlayer->inventory->items[k] != nullptr) {
                        
                        
                        if ((m_pPlayer->inventory->items[k]->id ==
                             pRecipeIngredientsRequired[i].iIngIDA[j]) &&
                            
                            
                            
                            ((pRecipeIngredientsRequired[i].iIngAuxValA[j] ==
                              Recipes::ANY_AUX_VALUE) ||
                             (pRecipeIngredientsRequired[i].iIngAuxValA[j] ==
                              m_pPlayer->inventory->items[k]->getAuxValue()))) {
                            
                            
                            
                            
                            if (m_pPlayer->inventory->items[k]->GetCount() >=
                                pRecipeIngredientsRequired[i].iIngValA[j]) {
                                
                                bFoundA[j] = true;
                            } else {
                                
                                

                                for (unsigned int l = 0;
                                     l < m_pPlayer->inventory->items.size();
                                     l++) {
                                    if (m_pPlayer->inventory->items[l] !=
                                        nullptr) {
                                        if ((m_pPlayer->inventory->items[l]
                                                 ->id ==
                                             pRecipeIngredientsRequired[i]
                                                 .iIngIDA[j]) &&
                                            ((pRecipeIngredientsRequired[i]
                                                  .iIngAuxValA[j] ==
                                              Recipes::ANY_AUX_VALUE) ||
                                             (pRecipeIngredientsRequired[i]
                                                  .iIngAuxValA[j] ==
                                              m_pPlayer->inventory->items[l]
                                                  ->getAuxValue()))) {
                                            iTotalCount +=
                                                m_pPlayer->inventory->items[l]
                                                    ->GetCount();
                                        }
                                    }
                                }

                                if (iTotalCount >=
                                    pRecipeIngredientsRequired[i].iIngValA[j]) {
                                    bFoundA[j] = true;
                                }
                            }

                            
                            
                            
                            
                            break;
                        }
                    }
                }
                
                
                
                if (bFoundA[j] == false) {
                    int iMissing =
                        pRecipeIngredientsRequired[i].iIngValA[j] - iTotalCount;
                    int iGridIndex = 0;
                    while (iMissing != 0) {
                        
                        if (((pRecipeIngredientsRequired[i]
                                  .uiGridA[iGridIndex] &
                              0x00FFFFFF) ==
                             pRecipeIngredientsRequired[i].iIngIDA[j]) &&
                            ((pRecipeIngredientsRequired[i].iIngAuxValA[j] ==
                              Recipes::ANY_AUX_VALUE) ||
                             (pRecipeIngredientsRequired[i].iIngAuxValA[j] ==
                              ((pRecipeIngredientsRequired[i]
                                    .uiGridA[iGridIndex] &
                                0xFF000000) >>
                               24)))) {
                            
                            
                            pRecipeIngredientsRequired[i]
                                .usBitmaskMissingGridIngredients[getPad()] |=
                                1 << iGridIndex;
                            iMissing--;
                        }
                        iGridIndex++;
                    }
                }
            }

            
            bool bCanMake = true;
            for (int j = 0; j < pRecipeIngredientsRequired[i].iIngC; j++) {
                if (bFoundA[j] == false) {
                    bCanMake = false;
                    break;
                }
            }

            pRecipeIngredientsRequired[i].bCanMake[getPad()] = bCanMake;

            
            if (iHSlotBrushControl <= m_iCraftablesMaxHSlotC) {
                bool bFound = false;
                std::shared_ptr<ItemInstance> pTempItemInst =
                    pRecipeIngredientsRequired[i].pRecipy->assemble(nullptr);
                
                
                int iID = pTempItemInst->getItem()->id;
                int iBaseType;

                if (iID < 256)  
                {
                    iBaseType = Tile::tiles[iID]->getBaseItemType();
                } else {
                    iBaseType = pTempItemInst->getItem()->getBaseItemType();
                }

                
                
                if (iBaseType != Item::eBaseItemType_undefined) {
                    for (int k = 0; k < iHSlotBrushControl; k++) {
                        
                        
                        if (CanBeMadeA[k].iItemBaseType == iBaseType) {
                            
                            bFound = true;
                            if (CanBeMadeA[k].iCount < m_iMaxVSlotC) {
                                CanBeMadeA[k].iRecipeA[CanBeMadeA[k].iCount++] =
                                    i;
                            } else {
                                app.DebugPrintf("Need more V slots\n");
                            }
                            break;
                        }
                    }
                }

                if (!bFound) {
                    if (iHSlotBrushControl < m_iCraftablesMaxHSlotC) {
                        
                        CanBeMadeA[iHSlotBrushControl].iItemBaseType =
                            iBaseType;
                        CanBeMadeA[iHSlotBrushControl]
                            .iRecipeA[CanBeMadeA[iHSlotBrushControl].iCount++] =
                            i;
                        iHSlotBrushControl++;
                    } else {
                        app.DebugPrintf("Need more H slots - ");
#if !defined(_CONTENT_PACKAGE)
                        OutputDebugStringW(
                            app.GetString(pTempItemInst->getDescriptionId()));
#endif
                        app.DebugPrintf("\n");
                    }
                }
            } else {
                app.DebugPrintf("Need more HSlots\n");
            }

            delete[] bFoundA;
            itRecipe++;
        }
    }

    
    int iIndex = 0;
    
    
    Recipy::INGREDIENTS_REQUIRED* pRecipeIngredientsRequired =
        Recipes::getInstance()->getRecipeIngredientsArray();

    while ((iIndex < m_iCraftablesMaxHSlotC) &&
           CanBeMadeA[iIndex].iCount != 0) {
        std::shared_ptr<ItemInstance> pTempItemInst =
            pRecipeIngredientsRequired[CanBeMadeA[iIndex].iRecipeA[0]]
                .pRecipy->assemble(nullptr);
        assert(pTempItemInst->id != 0);
        unsigned int uiAlpha;

        if (app.DebugSettingsOn() &&
            app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad()) &
                (1L << eDebugSetting_CraftAnything)) {
            uiAlpha = 31;
        } else {
            if (pRecipeIngredientsRequired[CanBeMadeA[iIndex].iRecipeA[0]]
                    .bCanMake[getPad()]) {
                uiAlpha = 31;
            } else {
                uiAlpha = 16;
            }
        }

        
        
        
        if (pTempItemInst->id == Item::clock_Id ||
            pTempItemInst->id == Item::compass_Id) {
            pTempItemInst->setAuxValue(255);
        }
        setCraftHSlotItem(getPad(), iIndex, pTempItemInst, uiAlpha);

        iIndex++;
    }

    
    
    









}






void IUIScene_CraftingMenu::UpdateHighlight() {
    updateHighlightAndScrollPositions();

    bool bCanBeMade = CanBeMadeA[m_iCurrentSlotHIndex].iCount != 0;
    if (bCanBeMade) {
        
        
        Recipy::INGREDIENTS_REQUIRED* pRecipeIngredientsRequired =
            Recipes::getInstance()->getRecipeIngredientsArray();
        int iSlot;
        if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 1) {
            iSlot = iVSlotIndexA[m_iCurrentSlotVIndex];
        } else {
            iSlot = 0;
        }
        std::shared_ptr<ItemInstance> pTempItemInstAdditional =
            pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex]
                                           .iRecipeA[iSlot]]
                .pRecipy->assemble(nullptr);

        
        int id = pTempItemInstAdditional->getDescriptionId();
        const wchar_t* itemstring;

        switch (id) {
            case IDS_TILE_TORCH: {
                if (pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex]
                                                   .iRecipeA[iSlot]]
                        .iIngAuxValA[0] == 1) {
                    itemstring = app.GetString(IDS_TILE_TORCHCHARCOAL);
                } else {
                    itemstring = app.GetString(IDS_TILE_TORCHCOAL);
                }
            } break;
            case IDS_ITEM_FIREBALL: {
                if (pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex]
                                                   .iRecipeA[iSlot]]
                        .iIngAuxValA[2] == 1) {
                    itemstring = app.GetString(IDS_ITEM_FIREBALLCHARCOAL);
                } else {
                    itemstring = app.GetString(IDS_ITEM_FIREBALLCOAL);
                }
            } break;
            default:
                itemstring = app.GetString(id);
                break;
        }

        setItemText(itemstring);
    } else {
        setItemText(L"");
    }
    UpdateDescriptionText(bCanBeMade);
    DisplayIngredients();

    UpdateMultiPanel();

    UpdateTooltips();
}






void IUIScene_CraftingMenu::UpdateVerticalSlots() {
    
    
    Recipy::INGREDIENTS_REQUIRED* pRecipeIngredientsRequired =
        Recipes::getInstance()->getRecipeIngredientsArray();

    
    hideAllVSlots();

    
    
    if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 1) {
        
        setShowCraftHSlot(m_iCurrentSlotHIndex, false);
        int iSlots = (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 2) ? 3 : 2;

        
        
        bool bNoScrollSlots = false;
        if (m_bSplitscreen ||
            (!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())) {
            bNoScrollSlots = true;
        }

        for (int i = 0; i < iSlots; i++) {
            
            
            if (bNoScrollSlots) {
                if (i != 1) continue;
            }
            std::shared_ptr<ItemInstance> pTempItemInstAdditional =
                pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex]
                                               .iRecipeA[iVSlotIndexA[i]]]
                    .pRecipy->assemble(nullptr);

            assert(pTempItemInstAdditional->id != 0);
            unsigned int uiAlpha;

            if (app.DebugSettingsOn() &&
                app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad()) &
                    (1L << eDebugSetting_CraftAnything)) {
                uiAlpha = 31;
            } else {
                if (pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex]
                                                   .iRecipeA[iVSlotIndexA[i]]]
                        .bCanMake[getPad()]) {
                    uiAlpha = 31;
                } else {
                    uiAlpha = 16;
                }
            }

            
            
            
            if (pTempItemInstAdditional->id == Item::clock_Id ||
                pTempItemInstAdditional->id == Item::compass_Id) {
                pTempItemInstAdditional->setAuxValue(255);
            }

            setCraftVSlotItem(getPad(), i, pTempItemInstAdditional, uiAlpha);

            updateVSlotPositions(iSlots, i);
        }
    }
}






void IUIScene_CraftingMenu::DisplayIngredients() {
    
    
    Recipy::INGREDIENTS_REQUIRED* pRecipeIngredientsRequired =
        Recipes::getInstance()->getRecipeIngredientsArray();

    
    hideAllIngredientsSlots();

    if (CanBeMadeA[m_iCurrentSlotHIndex].iCount != 0) {
        int iSlot, iRecipy;
        if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 1) {
            iSlot = iVSlotIndexA[m_iCurrentSlotVIndex];
            iRecipy = CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
        } else {
            iSlot = 0;
            iRecipy = CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[0];
        }

        
        int iBoxWidth = (m_iContainerType == RECIPE_TYPE_2x2) ? 2 : 3;
        int iRecipe = CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
        bool bCanMakeRecipe =
            pRecipeIngredientsRequired[iRecipe].bCanMake[getPad()];
        std::shared_ptr<ItemInstance> pTempItemInst =
            pRecipeIngredientsRequired[iRecipe].pRecipy->assemble(nullptr);

        m_iIngredientsC = pRecipeIngredientsRequired[iRecipe].iIngC;

        
        
        for (int i = 0; i < pRecipeIngredientsRequired[iRecipe].iIngC; i++) {
            int id = pRecipeIngredientsRequired[iRecipe].iIngIDA[i];
            int iAuxVal = pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i];
            Item* item = Item::items[id];

            std::shared_ptr<ItemInstance> itemInst =
                std::shared_ptr<ItemInstance>(new ItemInstance(
                    item, pRecipeIngredientsRequired[iRecipe].iIngValA[i],
                    iAuxVal));

            
            
            
            int idescID;

            if (((pTempItemInst->id == Item::bed_Id) &&
                 (id == Tile::wool_Id)) ||
                ((pTempItemInst->id == Item::painting_Id) &&
                 (id == Tile::wool_Id))) {
                idescID = IDS_ANY_WOOL;
            } else {
                idescID = itemInst->getDescriptionId();
            }
            setIngredientDescriptionText(i, app.GetString(idescID));

            if ((iAuxVal & 0xFF) ==
                0xFF)  
                iAuxVal = 0;

            
            
            
            if (id == Item::clock_Id || id == Item::compass_Id) {
                iAuxVal = 0xFF;
            }
            itemInst->setAuxValue(iAuxVal);

            setIngredientDescriptionItem(getPad(), i, itemInst);
            setIngredientDescriptionRedBox(i, false);
        }

        
        
        
        if (pTempItemInst->id == Item::clock_Id ||
            pTempItemInst->id == Item::compass_Id) {
            pTempItemInst->setAuxValue(255);
        }

        
        setCraftingOutputSlotItem(getPad(), pTempItemInst);

        if (app.DebugSettingsOn() &&
            app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad()) &
                (1L << eDebugSetting_CraftAnything)) {
            setCraftingOutputSlotRedBox(false);
        } else {
            if (bCanMakeRecipe == false) {
                setCraftingOutputSlotRedBox(true);
            } else {
                setCraftingOutputSlotRedBox(false);
            }
        }
        for (int x = 0; x < iBoxWidth; x++) {
            for (int y = 0; y < iBoxWidth; y++) {
                int index = x + y * iBoxWidth;
                if (pRecipeIngredientsRequired[iRecipy].uiGridA[x + y * 3] !=
                    0) {
                    int id =
                        pRecipeIngredientsRequired[iRecipy].uiGridA[x + y * 3] &
                        0x00FFFFFF;
                    assert(id != 0);
                    int iAuxVal = (pRecipeIngredientsRequired[iRecipy]
                                       .uiGridA[x + y * 3] &
                                   0xFF000000) >>
                                  24;

                    
                    
                    
                    if (id == Item::clock_Id || id == Item::compass_Id) {
                        iAuxVal = 0xFF;
                    } else if (pTempItemInst->id == Item::fireworksCharge_Id &&
                               id == Item::dye_powder_Id) {
                        iAuxVal = 1;
                    }
                    std::shared_ptr<ItemInstance> itemInst =
                        std::shared_ptr<ItemInstance>(
                            new ItemInstance(id, 1, iAuxVal));
                    setIngredientSlotItem(getPad(), index, itemInst);
                    
                    
                    if (app.DebugSettingsOn() &&
                        app.GetGameSettingsDebugMask(
                            ProfileManager.GetPrimaryPad()) &
                            (1L << eDebugSetting_CraftAnything)) {
                        setIngredientSlotRedBox(index, false);
                    } else {
                        if ((pRecipeIngredientsRequired[iRecipy]
                                 .usBitmaskMissingGridIngredients[getPad()] &
                             (1 << (x + y * 3))) != 0) {
                            setIngredientSlotRedBox(index, true);
                        } else {
                            setIngredientSlotRedBox(index, false);
                        }
                    }
                } else {
                    setIngredientSlotRedBox(index, false);
                    setIngredientSlotItem(getPad(), index, nullptr);
                }
            }
        }
    } else {
        setCraftingOutputSlotItem(getPad(), nullptr);
        setCraftingOutputSlotRedBox(false);
        m_iIngredientsC = 0;
        int iIngredientsSlots;
        
        if (m_iContainerType == RECIPE_TYPE_2x2) {
            iIngredientsSlots = 4;
        } else {
            iIngredientsSlots = m_iIngredients3x3SlotC;
        }

        for (int i = 0; i < iIngredientsSlots; i++) {
            setIngredientSlotRedBox(i, false);
            setIngredientSlotItem(getPad(), i, nullptr);
        }
    }
}






void IUIScene_CraftingMenu::UpdateDescriptionText(bool bCanBeMade) {
    int iIDSString = 0;
    
    
    Recipy::INGREDIENTS_REQUIRED* pRecipeIngredientsRequired =
        Recipes::getInstance()->getRecipeIngredientsArray();

    if (bCanBeMade) {
        int iSlot;  
        if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 1) {
            iSlot = iVSlotIndexA[m_iCurrentSlotVIndex];
            
        } else {
            iSlot = 0;
            
        }

        std::shared_ptr<ItemInstance> pTempItemInst =
            pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex]
                                           .iRecipeA[iSlot]]
                .pRecipy->assemble(nullptr);
        int iID = pTempItemInst->getItem()->id;
        int iAuxVal = pTempItemInst->getAuxValue();
        int iBaseType;

        if (iID < 256)  
        {
            iBaseType = Tile::tiles[iID]->getBaseItemType();

            iIDSString = Tile::tiles[iID]->getUseDescriptionId();
        } else {
            iBaseType = pTempItemInst->getItem()->getBaseItemType();

            iIDSString = pTempItemInst->getUseDescriptionId();
        }

        
        
        if (iBaseType != Item::eBaseItemType_undefined) {
            switch (iBaseType) {
                case Item::eBaseItemType_cloth:
                    switch (iAuxVal) {
                        case 0:
                            iIDSString = IDS_DESC_WOOLSTRING;
                            break;
                    }
                    break;
            }
        }

        

        if (iIDSString >= 0) {
            
            
            std::wstring wsText =
                app.FormatHTMLString(getPad(), app.GetString(iIDSString));

            
            EHTMLFontSize size = eHTMLSize_Normal;
            if (m_bSplitscreen ||
                (!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())) {
                size = eHTMLSize_Splitscreen;
            }
            wchar_t startTags[64];
            swprintf(startTags, 64, L"<font color=\"#%08x\"><P ALIGN=LEFT>",
                     app.GetHTMLColour(eHTMLColor_Black));
            wsText = startTags + wsText + L"</P>";

            setDescriptionText(wsText.c_str());
        } else {
            
#if defined(_DEBUG)
            setDescriptionText(
                L"This is some placeholder description text about the "
                L"craftable item.");
#else
            setDescriptionText(L"");
#endif
        }
    } else {
        setDescriptionText(L"");
    }
}






void IUIScene_CraftingMenu::UpdateTooltips() {
    
    
    Recipy::INGREDIENTS_REQUIRED* pRecipeIngredientsRequired =
        Recipes::getInstance()->getRecipeIngredientsArray();
    

    bool bDisplayCreate;

    if (CanBeMadeA[m_iCurrentSlotHIndex].iCount != 0) {
        int iSlot;
        if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 1) {
            iSlot = iVSlotIndexA[m_iCurrentSlotVIndex];
        } else {
            iSlot = 0;
        }

        if (pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex]
                                           .iRecipeA[iSlot]]
                .bCanMake[getPad()]) {
            bDisplayCreate = true;
        } else {
            bDisplayCreate = false;
        }
    } else {
        bDisplayCreate = false;
    }

    switch (m_iDisplayDescription) {
        case DISPLAY_INVENTORY:
            ui.SetTooltips(getPad(), bDisplayCreate ? IDS_TOOLTIPS_CREATE : -1,
                           IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_SHOW_DESCRIPTION, -1,
                           -1, -1, -2, IDS_TOOLTIPS_CHANGE_GROUP);
            break;
        case DISPLAY_DESCRIPTION:
            ui.SetTooltips(getPad(), bDisplayCreate ? IDS_TOOLTIPS_CREATE : -1,
                           IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_SHOW_INGREDIENTS, -1,
                           -1, -1, -2, IDS_TOOLTIPS_CHANGE_GROUP);
            break;
        case DISPLAY_INGREDIENTS:
            ui.SetTooltips(getPad(), bDisplayCreate ? IDS_TOOLTIPS_CREATE : -1,
                           IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_SHOW_INVENTORY, -1,
                           -1, -1, -2, IDS_TOOLTIPS_CHANGE_GROUP);
            break;
    }

    
























}

void IUIScene_CraftingMenu::HandleInventoryUpdated() {
    
    CheckRecipesAvailable();
    UpdateVerticalSlots();
    UpdateHighlight();
    UpdateTooltips();
}

bool IUIScene_CraftingMenu::isItemSelected(int itemId) {
    bool isSelected = false;
    if (m_pPlayer && m_pPlayer->inventory) {
        
        
        Recipy::INGREDIENTS_REQUIRED* pRecipeIngredientsRequired =
            Recipes::getInstance()->getRecipeIngredientsArray();

        if (CanBeMadeA[m_iCurrentSlotHIndex].iCount != 0) {
            int iSlot;
            if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 1) {
                iSlot = iVSlotIndexA[m_iCurrentSlotVIndex];
            } else {
                iSlot = 0;
            }
            int iRecipe = CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
            ItemInstance* pTempItemInst =
                (ItemInstance*)pRecipeIngredientsRequired[iRecipe]
                    .pRecipy->getResultItem();

            if (pTempItemInst->id == itemId) {
                isSelected = true;
            }
        }
    }
    return isSelected;
}
