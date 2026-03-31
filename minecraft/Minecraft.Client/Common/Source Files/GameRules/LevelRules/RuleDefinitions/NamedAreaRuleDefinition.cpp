#include <wchar.h>

#include "NamedAreaRuleDefinition.h"
#include "Minecraft.World/ConsoleHelpers/StringHelpers.h"
#include "Minecraft.Client/Common/Source Files/GameRules/LevelRules/RuleDefinitions/NamedAreaRuleDefinition.h"
#include "Minecraft.Client/Common/Source Files/GameRules/LevelRules/RuleDefinitions/GameRuleDefinition.h"
#include "Minecraft.Client/Linux/Linux_App.h"
#include "java/InputOutputStream/DataOutputStream.h"

NamedAreaRuleDefinition::NamedAreaRuleDefinition() {
    m_name = L"";
    m_area = AABB(0, 0, 0, 0, 0, 0);
}

void NamedAreaRuleDefinition::writeAttributes(DataOutputStream* dos,
                                              unsigned int numAttributes) {
    GameRuleDefinition::writeAttributes(dos, numAttributes + 7);

    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_name);
    dos->writeUTF(m_name);

    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_x0);
    dos->writeUTF(_toString(m_area.x0));
    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_y0);
    dos->writeUTF(_toString(m_area.y0));
    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_z0);
    dos->writeUTF(_toString(m_area.z0));

    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_x1);
    dos->writeUTF(_toString(m_area.x1));
    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_y1);
    dos->writeUTF(_toString(m_area.y1));
    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_z1);
    dos->writeUTF(_toString(m_area.z1));
}

void NamedAreaRuleDefinition::addAttribute(const std::wstring& attributeName,
                                           const std::wstring& attributeValue) {
    if (attributeName.compare(L"name") == 0) {
        m_name = attributeValue;
#ifndef _CONTENT_PACKAGE
        wprintf(L"NamedAreaRuleDefinition: Adding parameter name=%ls\n",
                m_name.c_str());
#endif
    } else if (attributeName.compare(L"x0") == 0) {
        m_area.x0 = _fromString<int>(attributeValue);
        app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter x0=%f\n",
                        m_area.x0);
    } else if (attributeName.compare(L"y0") == 0) {
        m_area.y0 = _fromString<int>(attributeValue);
        if (m_area.y0 < 0) m_area.y0 = 0;
        app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter y0=%f\n",
                        m_area.y0);
    } else if (attributeName.compare(L"z0") == 0) {
        m_area.z0 = _fromString<int>(attributeValue);
        app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter z0=%f\n",
                        m_area.z0);
    } else if (attributeName.compare(L"x1") == 0) {
        m_area.x1 = _fromString<int>(attributeValue);
        app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter x1=%f\n",
                        m_area.x1);
    } else if (attributeName.compare(L"y1") == 0) {
        m_area.y1 = _fromString<int>(attributeValue);
        if (m_area.y1 < 0) m_area.y1 = 0;
        app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter y1=%f\n",
                        m_area.y1);
    } else if (attributeName.compare(L"z1") == 0) {
        m_area.z1 = _fromString<int>(attributeValue);
        app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter z1=%f\n",
                        m_area.z1);
    } else {
        GameRuleDefinition::addAttribute(attributeName, attributeValue);
    }
}
