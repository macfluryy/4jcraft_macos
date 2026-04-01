#include "DLCSkinFile.h"

#include <wchar.h>
#include <string.h>

#include "4J_Render.h"
#include "DLCManager.h"
#include "Minecraft.Client/Linux/Linux_App.h"
#include "Minecraft.Client/include/SkinBox.h"
#include "Minecraft.Client/include/XboxStubs.h"
#include "Minecraft.Client/Common/src/DLC/DLCFile.h"

DLCSkinFile::DLCSkinFile(const std::wstring& path)
    : DLCFile(DLCManager::e_DLCType_Skin, path) {
    m_displayName = L"";
    m_themeName = L"";
    m_cape = L"";
    m_bIsFree = false;
    m_uiAnimOverrideBitmask = 0L;
}

void DLCSkinFile::addData(std::uint8_t* pbData, std::uint32_t dataBytes) {
    app.AddMemoryTextureFile(m_path, pbData, dataBytes);
}

void DLCSkinFile::addParameter(DLCManager::EDLCParameterType type,
                               const std::wstring& value) {
    switch (type) {
        case DLCManager::e_DLCParamType_DisplayName: {
            // 4J Stu - In skin pack 2, the name for Zap is mis-spelt with two
            // p's as Zapp dlcskin00000109.png
            if (m_path.compare(L"dlcskin00000109.png") == 0) {
                m_displayName = L"Zap";
            } else {
                m_displayName = value;
            }
        } break;
        case DLCManager::e_DLCParamType_ThemeName:
            m_themeName = value;
            break;
        case DLCManager::e_DLCParamType_Free:  // If this parameter exists, then
                                               // mark this as free
            m_bIsFree = true;
            break;
        case DLCManager::e_DLCParamType_Credit:  // If this parameter exists,
                                                 // then mark this as free
                                                 // add it to the DLC credits
                                                 // list

            // we'll need to justify this text since we don't have a lot of room
            // for lines of credits
            {
                if (app.AlreadySeenCreditText(value)) break;
                // first add a blank string for spacing
                app.AddCreditText(L"");

                int maximumChars = 55;

                bool bIsSDMode =
                    !RenderManager.IsHiDef() && !RenderManager.IsWidescreen();

                if (bIsSDMode) {
                    maximumChars = 45;
                }

                switch (XGetLanguage()) {
                    case XC_LANGUAGE_JAPANESE:
                    case XC_LANGUAGE_TCHINESE:
                    case XC_LANGUAGE_KOREAN:
                        maximumChars = 35;
                        break;
                    default:
                        break;
                }
                std::wstring creditValue = value;
                while (creditValue.length() > maximumChars) {
                    unsigned int i = 1;
                    while (i < creditValue.length() &&
                           (i + 1) <= maximumChars) {
                        i++;
                    }
                    int iLast = (int)creditValue.find_last_of(L" ", i);
                    switch (XGetLanguage()) {
                        case XC_LANGUAGE_JAPANESE:
                        case XC_LANGUAGE_TCHINESE:
                        case XC_LANGUAGE_KOREAN:
                            iLast = maximumChars;
                            break;
                        default:
                            iLast = (int)creditValue.find_last_of(L" ", i);
                            break;
                    }

                    // if a space was found, include the space on this line
                    if (iLast != i) {
                        iLast++;
                    }

                    app.AddCreditText((creditValue.substr(0, iLast)).c_str());
                    creditValue = creditValue.substr(iLast);
                }
                app.AddCreditText(creditValue.c_str());
            }
            break;
        case DLCManager::e_DLCParamType_Cape:
            m_cape = value;
            break;
        case DLCManager::e_DLCParamType_Box: {
            wchar_t wchBodyPart[10];
            SKIN_BOX* pSkinBox = new SKIN_BOX;
            memset(pSkinBox, 0, sizeof(SKIN_BOX));

            swscanf(value.c_str(), L"%9ls%f%f%f%f%f%f%f%f", wchBodyPart, 10,
                    &pSkinBox->fX, &pSkinBox->fY, &pSkinBox->fZ, &pSkinBox->fW,
                    &pSkinBox->fH, &pSkinBox->fD, &pSkinBox->fU, &pSkinBox->fV);

            if (wcscmp(wchBodyPart, L"HEAD") == 0) {
                pSkinBox->ePart = eBodyPart_Head;
            } else if (wcscmp(wchBodyPart, L"BODY") == 0) {
                pSkinBox->ePart = eBodyPart_Body;
            } else if (wcscmp(wchBodyPart, L"ARM0") == 0) {
                pSkinBox->ePart = eBodyPart_Arm0;
            } else if (wcscmp(wchBodyPart, L"ARM1") == 0) {
                pSkinBox->ePart = eBodyPart_Arm1;
            } else if (wcscmp(wchBodyPart, L"LEG0") == 0) {
                pSkinBox->ePart = eBodyPart_Leg0;
            } else if (wcscmp(wchBodyPart, L"LEG1") == 0) {
                pSkinBox->ePart = eBodyPart_Leg1;
            }

            // add this to the skin's vector of parts
            m_AdditionalBoxes.push_back(pSkinBox);
        } break;
        case DLCManager::e_DLCParamType_Anim: {
            swscanf(value.c_str(), L"%X", &m_uiAnimOverrideBitmask,
                    sizeof(unsigned int));
            uint32_t skinId = app.getSkinIdFromPath(m_path);
            app.SetAnimOverrideBitmask(skinId, m_uiAnimOverrideBitmask);
            break;
        }
        default:
            break;
    }
}

// std::vector<ModelPart *> *DLCSkinFile::getAdditionalModelParts()
// {
// 	return &m_AdditionalModelParts;
// }

int DLCSkinFile::getAdditionalBoxesCount() {
    return (int)m_AdditionalBoxes.size();
}
std::vector<SKIN_BOX*>* DLCSkinFile::getAdditionalBoxes() {
    return &m_AdditionalBoxes;
}

std::wstring DLCSkinFile::getParameterAsString(
    DLCManager::EDLCParameterType type) {
    switch (type) {
        case DLCManager::e_DLCParamType_DisplayName:
            return m_displayName;
        case DLCManager::e_DLCParamType_ThemeName:
            return m_themeName;
        case DLCManager::e_DLCParamType_Cape:
            return m_cape;
        default:
            return L"";
    }
}

bool DLCSkinFile::getParameterAsBool(DLCManager::EDLCParameterType type) {
    switch (type) {
        case DLCManager::e_DLCParamType_Free:
            return m_bIsFree;
        default:
            return false;
    }
}
