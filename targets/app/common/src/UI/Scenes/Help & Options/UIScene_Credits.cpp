
#include "UIScene_Credits.h"

#include <string.h>
#include <wchar.h>

#include "platform/InputActions.h"
#include "app/common/src/UI/UILayer.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "util/StringHelpers.h"
#include "strings.h"

#define CREDIT_ICON -2

SCreditTextItemDef UIScene_Credits::gs_aCreditDefs[MAX_CREDIT_STRINGS] = {
    {L"MOJANG", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eExtraLargeText},
    {L"", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},  
    {L"%ls", IDS_CREDITS_ORIGINALDESIGN, NO_TRANSLATED_STRING, eLargeText},
    {L"Markus Persson", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},  
    {L"%ls", IDS_CREDITS_PMPROD, NO_TRANSLATED_STRING, eLargeText},
    {L"Daniel Kaplan", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},  
    {L"%ls", IDS_CREDITS_RESTOFMOJANG, NO_TRANSLATED_STRING, eMediumText},
    {L"%ls", IDS_CREDITS_LEADPC, NO_TRANSLATED_STRING, eLargeText},
    {L"Jens Bergensten", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"%ls", IDS_CREDITS_JON_KAGSTROM, NO_TRANSLATED_STRING, eSmallText},
    {L"%ls", IDS_CREDITS_CEO, NO_TRANSLATED_STRING, eLargeText},
    {L"Carl Manneh", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"%ls", IDS_CREDITS_DOF, NO_TRANSLATED_STRING, eLargeText},
    {L"Lydia Winters", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"%ls", IDS_CREDITS_WCW, NO_TRANSLATED_STRING, eLargeText},
    {L"Karin Severinsson", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"%ls", IDS_CREDITS_CUSTOMERSUPPORT, NO_TRANSLATED_STRING, eLargeText},
    {L"Marc Watson", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},  
    {L"%ls", IDS_CREDITS_DESPROG, NO_TRANSLATED_STRING, eLargeText},
    {L"Aron Nieminen", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},  
    {L"%ls", IDS_CREDITS_CHIEFARCHITECT, NO_TRANSLATED_STRING, eLargeText},
    {L"Daniel Frisk", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"%ls", IDS_CREDITS_CODENINJA, NO_TRANSLATED_STRING, eLargeText},
    {L"%ls", IDS_CREDITS_TOBIAS_MOLLSTAM, NO_TRANSLATED_STRING, eSmallText},
    {L"%ls", IDS_CREDITS_OFFICEDJ, NO_TRANSLATED_STRING, eLargeText},
    {L"Kristoffer Jelbring", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"%ls", IDS_CREDITS_DEVELOPER, NO_TRANSLATED_STRING, eLargeText},
    {L"Leonard Axelsson", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"%ls", IDS_CREDITS_BULLYCOORD, NO_TRANSLATED_STRING, eLargeText},
    {L"Jakob Porser", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"%ls", IDS_CREDITS_ARTDEVELOPER, NO_TRANSLATED_STRING, eLargeText},
    {L"Junkboy", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"%ls", IDS_CREDITS_EXPLODANIM, NO_TRANSLATED_STRING, eLargeText},
    {L"Mattis Grahm", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"%ls", IDS_CREDITS_CONCEPTART, NO_TRANSLATED_STRING, eLargeText},
    {L"Henrik Petterson", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"%ls", IDS_CREDITS_CRUNCHER, NO_TRANSLATED_STRING, eLargeText},
    {L"Patrick Geuder", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"%ls", IDS_CREDITS_MUSICANDSOUNDS, NO_TRANSLATED_STRING, eLargeText},
    {L"Daniel Rosenfeld (C418)", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},  

    
    {L"Developers of Mo' Creatures:", NO_TRANSLATED_STRING,
     NO_TRANSLATED_STRING, eExtraLargeText},
    {L"John Olarte (DrZhark)", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"Kent Christian Jensen", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"Dan Roque", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},  

    {L"4J Studios", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eExtraLargeText},
    {L"%ls", IDS_CREDITS_PROGRAMMING, NO_TRANSLATED_STRING, eLargeText},
    {L"Paddy Burns", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Richard Reavy", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Stuart Ross", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"James Vaughan", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Mark Hughes", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Harry Gordon", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Thomas Kronberg", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},

    {L"%ls", IDS_CREDITS_ART, NO_TRANSLATED_STRING, eLargeText},
    {L"David Keningale", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"Alan Redmond", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Chris Reeves", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Kate Wright", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Michael Hansen", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Donald Robertson", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"Jamie Keddie", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Thomas Naylor", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Brian Lindsay", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Hannah Watts", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Rebecca O'Neil", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},

    {L"%ls", IDS_CREDITS_QA, NO_TRANSLATED_STRING, eLargeText},
    {L"Steven Gary Woodward", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"George Vaughan", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},  
    {L"%ls", IDS_CREDITS_SPECIALTHANKS, NO_TRANSLATED_STRING, eLargeText},
    {L"Chris van der Kuyl", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},
    {L"Roni Percy", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Anne Clarke", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},
    {L"Anthony Kent", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING, eSmallText},

    
    {L"", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},                                      
    {L"", CREDIT_ICON, eCreditIcon_Iggy, eSmallText},  
    {L"Uses Iggy.", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},  
    {L"Copyright (C) 2009-2014 by RAD Game Tools, Inc.", NO_TRANSLATED_STRING,
     NO_TRANSLATED_STRING, eSmallText},  
    {L"", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},                                       
    {L"", CREDIT_ICON, eCreditIcon_Miles, eSmallText},  
    {L"Uses Miles Sound System.", NO_TRANSLATED_STRING, NO_TRANSLATED_STRING,
     eSmallText},  
    {L"Copyright (C) 1991-2014 by RAD Game Tools, Inc.", NO_TRANSLATED_STRING,
     NO_TRANSLATED_STRING, eSmallText},  
};

UIScene_Credits::UIScene_Credits(int iPad, void* initData, UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    
    initialiseMovie();

    m_bAddNextLabel = false;

    
    m_iNumTextDefs = MAX_CREDIT_STRINGS;

    
    m_iNumTextDefs += app.GetDLCCreditsCount();

    m_iCurrDefIndex = -1;

    
    for (unsigned int i = 0; i < 20; ++i) {
        ++m_iCurrDefIndex;

        
        if (gs_aCreditDefs[i].m_iStringID[0] == NO_TRANSLATED_STRING) {
            setNextLabel(gs_aCreditDefs[i].m_Text, gs_aCreditDefs[i].m_eType);
        } else  
        {
            wchar_t* creditsString = new wchar_t[128];
            if (gs_aCreditDefs[i].m_iStringID[1] != NO_TRANSLATED_STRING) {
                swprintf(creditsString, 128, gs_aCreditDefs[i].m_Text,
                         app.GetString(gs_aCreditDefs[i].m_iStringID[0]),
                         app.GetString(gs_aCreditDefs[i].m_iStringID[1]));
            } else {
                swprintf(creditsString, 128, gs_aCreditDefs[i].m_Text,
                         app.GetString(gs_aCreditDefs[i].m_iStringID[0]));
            }
            setNextLabel(creditsString, gs_aCreditDefs[i].m_eType);
            delete[] creditsString;
        }
    }
}

std::wstring UIScene_Credits::getMoviePath() { return L"Credits"; }

void UIScene_Credits::updateTooltips() {
    ui.SetTooltips(m_iPad, -1, IDS_TOOLTIPS_BACK);
}

void UIScene_Credits::updateComponents() {
    m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, true);
}

void UIScene_Credits::handleReload() {
    
    navigateBack();
}

void UIScene_Credits::tick() {
    UIScene::tick();

    if (m_bAddNextLabel) {
        m_bAddNextLabel = false;

        const SCreditTextItemDef* pDef;

        
        ++m_iCurrDefIndex;

        
        if (m_iCurrDefIndex >= m_iNumTextDefs) {
            m_iCurrDefIndex = 0;
        }

        if (m_iCurrDefIndex >= MAX_CREDIT_STRINGS) {
            app.DebugPrintf("DLC credit %d\n",
                            m_iCurrDefIndex - MAX_CREDIT_STRINGS);
            
            pDef = app.GetDLCCredits(m_iCurrDefIndex - MAX_CREDIT_STRINGS);
        } else {
            
            pDef = &(gs_aCreditDefs[m_iCurrDefIndex]);
        }

        
        if (pDef->m_Text != nullptr)  
                                      
        {
            if (pDef->m_iStringID[0] == CREDIT_ICON) {
                addImage((ECreditIcons)pDef->m_iStringID[1]);
            } else  
            {
                std::wstring sanitisedString = std::wstring(pDef->m_Text);

                
                
                if (!ui.UsingBitmapFont()) {
                    sanitisedString =
                        replaceAll(sanitisedString, L"\u00A9", L"(C)");
                    sanitisedString =
                        replaceAll(sanitisedString, L"\u00AE", L"(R)");
                    sanitisedString =
                        replaceAll(sanitisedString, L"\u2013", L"-");
                }

                wchar_t* creditsString = new wchar_t[128];
                if (pDef->m_iStringID[0] == NO_TRANSLATED_STRING) {
                    memset(creditsString, 0, 128);
                    memcpy(creditsString, sanitisedString.c_str(),
                           sizeof(wchar_t) * sanitisedString.length());
                } else if (pDef->m_iStringID[1] != NO_TRANSLATED_STRING) {
                    swprintf(creditsString, 128, sanitisedString.c_str(),
                             app.GetString(pDef->m_iStringID[0]),
                             app.GetString(pDef->m_iStringID[1]));
                } else {
                    swprintf(creditsString, 128, sanitisedString.c_str(),
                             app.GetString(pDef->m_iStringID[0]));
                }

                setNextLabel(creditsString, pDef->m_eType);
                delete[] creditsString;
            }
        }
    }
}

void UIScene_Credits::handleInput(int iPad, int key, bool repeat, bool pressed,
                                  bool released, bool& handled) {
    
    
    

    ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_CANCEL:
            if (pressed && !repeat) {
                navigateBack();
            }
            break;
        case ACTION_MENU_OK:
        case ACTION_MENU_UP:
        case ACTION_MENU_DOWN:
            sendInputToMovie(key, repeat, pressed, released);
            break;
    }
}

void UIScene_Credits::setNextLabel(const std::wstring& label,
                                   ECreditTextTypes size) {
    IggyDataValue result;
    IggyDataValue value[3];

    const std::u16string convLabel = wstring_to_u16string(label);

    IggyStringUTF16 stringVal;
    stringVal.string = convLabel.c_str();
    stringVal.length = convLabel.length();
    value[0].type = IGGY_DATATYPE_string_UTF16;
    value[0].string16 = stringVal;

    value[1].type = IGGY_DATATYPE_number;
    value[1].number = (int)size;

    value[2].type = IGGY_DATATYPE_boolean;
    value[2].boolval = (m_iCurrDefIndex == (m_iNumTextDefs - 1));

    IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result,
                                            IggyPlayerRootPath(getMovie()),
                                            m_funcSetNextLabel, 3, value);
}

void UIScene_Credits::addImage(ECreditIcons icon) {
    IggyDataValue result;
    IggyDataValue value[2];

    value[0].type = IGGY_DATATYPE_number;
    value[0].number = (int)icon;

    value[1].type = IGGY_DATATYPE_boolean;
    value[1].boolval = (m_iCurrDefIndex == (m_iNumTextDefs - 1));

    IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result,
                                            IggyPlayerRootPath(getMovie()),
                                            m_funcAddImage, 2, value);
}

void UIScene_Credits::handleRequestMoreData(F64 startIndex, bool up) {
    m_bAddNextLabel = true;
}
