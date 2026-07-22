
#include "UIScene_HowToPlay.h"

#include <stdint.h>
#include <wchar.h>

#include <vector>

#include "platform/InputActions.h"
#include "app/common/App_enums.h"
#include "app/common/src/UI/Controls/UIControl_Label.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "util/StringHelpers.h"
#include "minecraft/sounds/SoundTypes.h"
#include "strings.h"

class UILayer;

static UIScene_HowToPlay::SHowToPlayPageDef gs_aPageDefs[eHowToPlay_NumPages] =
    {
        {IDS_HOW_TO_PLAY_WHATSNEW, 0, 0},     
        {IDS_HOW_TO_PLAY_BASICS, 0, 0},       
        {IDS_HOW_TO_PLAY_MULTIPLAYER, 0, 0},  
        {IDS_HOW_TO_PLAY_HUD, 0, 0},          
        {IDS_HOW_TO_PLAY_CREATIVE,
         UIScene_HowToPlay::eHowToPlay_LabelCreativeInventory,
         1},  
        {IDS_HOW_TO_PLAY_INVENTORY,
         UIScene_HowToPlay::eHowToPlay_LabelIInventory,
         1},  
        {IDS_HOW_TO_PLAY_CHEST, UIScene_HowToPlay::eHowToPlay_LabelSCInventory,
         2},  
        {IDS_HOW_TO_PLAY_LARGECHEST,
         UIScene_HowToPlay::eHowToPlay_LabelLCInventory,
         2},                                 
        {IDS_HOW_TO_PLAY_ENDERCHEST, 0, 0},  
        {IDS_HOW_TO_PLAY_CRAFTING, UIScene_HowToPlay::eHowToPlay_LabelCItem,
         3},  
        {IDS_HOW_TO_PLAY_CRAFT_TABLE, UIScene_HowToPlay::eHowToPlay_LabelCTItem,
         3},  
        {IDS_HOW_TO_PLAY_FURNACE, UIScene_HowToPlay::eHowToPlay_LabelFFuel,
         4},  
        {IDS_HOW_TO_PLAY_DISPENSER, UIScene_HowToPlay::eHowToPlay_LabelDText,
         2},  
        {IDS_HOW_TO_PLAY_BREWING, UIScene_HowToPlay::eHowToPlay_LabelBBrew,
         2},  
        {IDS_HOW_TO_PLAY_ENCHANTMENT,
         UIScene_HowToPlay::eHowToPlay_LabelEEnchant,
         2},  
        {IDS_HOW_TO_PLAY_ANVIL,
         UIScene_HowToPlay::eHowToPlay_LabelAnvil_Inventory,
         3},                                   
        {IDS_HOW_TO_PLAY_FARMANIMALS, 0, 0},   
        {IDS_HOW_TO_PLAY_BREEDANIMALS, 0, 0},  
        {IDS_HOW_TO_PLAY_TRADING,
         UIScene_HowToPlay::eHowToPlay_LabelTrading_Inventory,
         5},                                   
        {IDS_HOW_TO_PLAY_HORSES, 0, 0},        
        {IDS_HOW_TO_PLAY_BEACONS, 0, 0},       
        {IDS_HOW_TO_PLAY_FIREWORKS, 0, 0},     
        {IDS_HOW_TO_PLAY_HOPPERS, 0, 0},       
        {IDS_HOW_TO_PLAY_DROPPERS, 0, 0},      
        {IDS_HOW_TO_PLAY_NETHERPORTAL, 0, 0},  
        {IDS_HOW_TO_PLAY_THEEND, 0, 0},        
        {IDS_HOW_TO_PLAY_HOSTOPTIONS, 0, 0},   
};

int gs_pageToFlashMapping[eHowToPlay_NumPages] = {
    0,   
    1,   
    2,   
    3,   
    4,   
    5,   
    6,   
    7,   
    23,  
    8,   
    9,   
    10,  
    11,  

    12,  
    13,  
    21,  
    14,  
    15,  
    22,  

    24,  
    25,  
    26,  
    27,  
    28,  

    16,  
    17,  
    20,  
};

UIScene_HowToPlay::UIScene_HowToPlay(int iPad, void* initData,
                                     UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    
    initialiseMovie();

    std::wstring inventoryString = app.GetString(IDS_INVENTORY);
    m_labels[eHowToPlay_LabelCTItem].init(app.GetString(IDS_ITEM_HATCHET_WOOD));
    m_labels[eHowToPlay_LabelCTGroup].init(app.GetString(IDS_GROUPNAME_TOOLS));
    m_labels[eHowToPlay_LabelCTInventory3x3].init(inventoryString);
    m_labels[eHowToPlay_LabelCItem].init(app.GetString(IDS_TILE_WORKBENCH));
    m_labels[eHowToPlay_LabelCGroup].init(
        app.GetString(IDS_GROUPNAME_STRUCTURES));
    m_labels[eHowToPlay_LabelCInventory2x2].init(inventoryString);
    m_labels[eHowToPlay_LabelFFuel].init(app.GetString(IDS_FUEL));
    m_labels[eHowToPlay_LabelFInventory].init(inventoryString);
    m_labels[eHowToPlay_LabelFIngredient].init(app.GetString(IDS_INGREDIENT));
    m_labels[eHowToPlay_LabelFChest].init(app.GetString(IDS_FURNACE));
    m_labels[eHowToPlay_LabelLCInventory].init(inventoryString);
    m_labels[eHowToPlay_LabelCreativeInventory].init(
        app.GetString(IDS_GROUPNAME_BUILDING_BLOCKS));
    m_labels[eHowToPlay_LabelLCChest].init(app.GetString(IDS_CHEST_LARGE));
    m_labels[eHowToPlay_LabelSCInventory].init(inventoryString);
    m_labels[eHowToPlay_LabelSCChest].init(app.GetString(IDS_CHEST));
    m_labels[eHowToPlay_LabelIInventory].init(inventoryString);
    m_labels[eHowToPlay_LabelDInventory].init(inventoryString);
    m_labels[eHowToPlay_LabelDText].init(app.GetString(IDS_DISPENSER));
    m_labels[eHowToPlay_LabelEEnchant].init(app.GetString(IDS_ENCHANT));
    m_labels[eHowToPlay_LabelEInventory].init(inventoryString);
    m_labels[eHowToPlay_LabelBBrew].init(app.GetString(IDS_BREWING_STAND));
    m_labels[eHowToPlay_LabelBInventory].init(inventoryString);
    m_labels[eHowToPlay_LabelAnvil_Inventory].init(inventoryString.c_str());

    std::wstring wsTemp = app.GetString(IDS_REPAIR_COST);
    wsTemp.replace(wsTemp.find(L"%d"), 2, std::wstring(L"8"));

    m_labels[eHowToPlay_LabelAnvil_Cost].init(wsTemp.c_str());
    m_labels[eHowToPlay_LabelAnvil_ARepairAndName].init(
        app.GetString(IDS_REPAIR_AND_NAME));
    m_labels[eHowToPlay_LabelTrading_Inventory].init(inventoryString.c_str());
    m_labels[eHowToPlay_LabelTrading_Offer2].init(
        app.GetString(IDS_ITEM_EMERALD));
    m_labels[eHowToPlay_LabelTrading_Offer1].init(
        app.GetString(IDS_ITEM_EMERALD));
    m_labels[eHowToPlay_LabelTrading_NeededForTrade].init(
        app.GetString(IDS_REQUIRED_ITEMS_FOR_TRADE));

    m_labels[eHowToPlay_LabelBeacon_PrimaryPower].init(
        app.GetString(IDS_CONTAINER_BEACON_PRIMARY_POWER));
    m_labels[eHowToPlay_LabelBeacon_SecondaryPower].init(
        app.GetString(IDS_CONTAINER_BEACON_SECONDARY_POWER));

    m_labels[eHowToPlay_LabelFireworksText].init(
        app.GetString(IDS_HOW_TO_PLAY_MENU_FIREWORKS));
    m_labels[eHowToPlay_LabelFireworksInventory].init(inventoryString.c_str());

    m_labels[eHowToPlay_LabelHopperText].init(app.GetString(IDS_TILE_HOPPER));
    m_labels[eHowToPlay_LabelHopperInventory].init(inventoryString.c_str());

    m_labels[eHowToPlay_LabelDropperText].init(app.GetString(IDS_TILE_DROPPER));
    m_labels[eHowToPlay_LabelDropperInventory].init(inventoryString.c_str());

    wsTemp = app.GetString(IDS_VILLAGER_OFFERS_ITEM);
    wsTemp = replaceAll(wsTemp, L"{*VILLAGER_TYPE*}",
                        app.GetString(IDS_VILLAGER_PRIEST));
    wsTemp.replace(wsTemp.find(L"%s"), 2, app.GetString(IDS_TILE_LIGHT_GEM));
    m_labels[eHowToPlay_LabelTrading_VillagerOffers].init(wsTemp.c_str());

    
    
    uintptr_t uiInitData = reinterpret_cast<uintptr_t>(initData);

    EHowToPlayPage eStartPage =
        (EHowToPlayPage)((uiInitData >> 16) &
                         0xFFFu);  

    StartPage(eStartPage);
}

std::wstring UIScene_HowToPlay::getMoviePath() {
    if (app.GetLocalPlayerCount() > 1) {
        return L"HowToPlaySplit";
    } else {
        return L"HowToPlay";
    }
}

void UIScene_HowToPlay::updateTooltips() {
    
    int iPage = (int)(m_eCurrPage);

    int firstPage = eHowToPlay_WhatsNew;

    

    int iA = -1;
    int iX = -1;
    if (iPage == firstPage) {
        
        iA = IDS_HOW_TO_PLAY_NEXT;
    } else if ((iPage + 1) == eHowToPlay_NumPages) {
        
        iX = IDS_HOW_TO_PLAY_PREV;
    } else {
        iA = IDS_HOW_TO_PLAY_NEXT;
        iX = IDS_HOW_TO_PLAY_PREV;
    }
    ui.SetTooltips(m_iPad, iA, IDS_TOOLTIPS_BACK, iX);
}

void UIScene_HowToPlay::handleReload() { StartPage(m_eCurrPage); }

void UIScene_HowToPlay::handleInput(int iPad, int key, bool repeat,
                                    bool pressed, bool released,
                                    bool& handled) {
    ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_CANCEL:
            if (pressed) {
                navigateBack();
                handled = true;
            }
            break;
        case ACTION_MENU_A:
            if (pressed) {
                
                int iNextPage = (int)(m_eCurrPage) + 1;
                if (iNextPage != eHowToPlay_NumPages) {
                    StartPage((EHowToPlayPage)(iNextPage));
                    ui.PlayUISFX(eSFX_Press);
                }
                handled = true;
            }
            break;
        case ACTION_MENU_X:
            if (pressed) {
                
                int iPrevPage = (int)(m_eCurrPage)-1;

                
                {
                    if (iPrevPage >= 0) {
                        StartPage((EHowToPlayPage)(iPrevPage));
                        ui.PlayUISFX(eSFX_Press);
                    }
                }
                handled = true;
            }
            break;
        case ACTION_MENU_UP:
        case ACTION_MENU_DOWN:
        case ACTION_MENU_PAGEUP:
        case ACTION_MENU_PAGEDOWN:
            sendInputToMovie(key, repeat, pressed, released);
            break;
    }
}

void UIScene_HowToPlay::StartPage(EHowToPlayPage ePage) {
    m_eCurrPage = ePage;

    
    SHowToPlayPageDef* pDef = &(gs_aPageDefs[m_eCurrPage]);

    
    std::wstring replacedText =
        app.FormatHTMLString(m_iPad, app.GetString(pDef->m_iTextStringID));
    
    
    
    
    replacedText = replaceAll(replacedText, L"{*KICK_PLAYER_DESCRIPTION*}",
                              app.GetString(IDS_KICK_PLAYER_DESCRIPTION));
    replacedText = replaceAll(replacedText, L"{*BACK_BUTTON*}",
                              app.GetString(IDS_BACK_BUTTON));
    replacedText =
        replaceAll(replacedText, L"{*DISABLES_ACHIEVEMENTS*}",
                   app.GetString(IDS_HOST_OPTION_DISABLES_ACHIEVEMENTS));

    
    
    if (!ui.UsingBitmapFont()) {
        replacedText = replaceAll(replacedText, L"\u00A9", L"(C)");
        replacedText = replaceAll(replacedText, L"\u00AE", L"(R)");
        replacedText = replaceAll(replacedText, L"\u2013", L"-");
    }

    
    stripWhitespaceForHtml(replacedText, true);

    
    std::wstring finalText(replacedText.c_str());
    wchar_t startTags[64];
    swprintf(startTags, 64, L"<font color=\"#%08x\">",
             app.GetHTMLColour(eHTMLColor_White));
    finalText = startTags + finalText;

    std::vector<std::wstring> paragraphs;
    int lastIndex = 0;
    for (int index = finalText.find(L"\r\n", lastIndex, 2);
         index != std::wstring::npos;
         index = finalText.find(L"\r\n", lastIndex, 2)) {
        paragraphs.push_back(finalText.substr(lastIndex, index - lastIndex) +
                             L" ");
        lastIndex = index + 2;
    }
    paragraphs.push_back(
        finalText.substr(lastIndex, finalText.length() - lastIndex));

    
    IggyDataValue result;

    IggyDataValue* value = new IggyDataValue[paragraphs.size() + 1];
    IggyStringUTF16* stringVal = new IggyStringUTF16[paragraphs.size()];

    value[0].type = IGGY_DATATYPE_number;
    value[0].number = gs_pageToFlashMapping[(int)ePage];

    for (unsigned int i = 0; i < paragraphs.size(); ++i) {
        const std::u16string convParagraph =
            wstring_to_u16string(paragraphs[i]);

        stringVal[i].string = convParagraph.c_str();
        stringVal[i].length = convParagraph.length();
        value[i + 1].type = IGGY_DATATYPE_string_UTF16;
        value[i + 1].string16 = stringVal[i];
    }

    IggyResult out = IggyPlayerCallMethodRS(
        getMovie(), &result, IggyPlayerRootPath(getMovie()), m_funcLoadPage,
        1 + paragraphs.size(), value);

    delete[] value;
    delete[] stringVal;

    updateTooltips();
}
