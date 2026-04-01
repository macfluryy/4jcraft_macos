#include "XboxStructureActionPlaceBlock.h"

#include "Minecraft.Client/Common/src/GameRules/LevelGeneration/ConsoleGenerateStructureAction.h"
#include "Minecraft.Client/Common/src/GameRules/LevelGeneration/StructureActions/XboxStructureActionPlaceBlock.h"
#include "Minecraft.Client/Common/src/GameRules/LevelRules/RuleDefinitions/GameRuleDefinition.h"
#include "Minecraft.Client/Linux/Linux_App.h"
#include "console_helpers/StringHelpers.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/world/level/levelgen/structure/StructurePiece.h"

XboxStructureActionPlaceBlock::XboxStructureActionPlaceBlock() {
    m_x = m_y = m_z = m_tile = m_data = 0;
}

void XboxStructureActionPlaceBlock::writeAttributes(DataOutputStream* dos,
                                                    unsigned int numAttrs) {
    ConsoleGenerateStructureAction::writeAttributes(dos, numAttrs + 5);

    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_x);
    dos->writeUTF(_toString(m_x));
    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_y);
    dos->writeUTF(_toString(m_y));
    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_z);
    dos->writeUTF(_toString(m_z));

    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_data);
    dos->writeUTF(_toString(m_data));
    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_block);
    dos->writeUTF(_toString(m_tile));
}

void XboxStructureActionPlaceBlock::addAttribute(
    const std::wstring& attributeName, const std::wstring& attributeValue) {
    if (attributeName.compare(L"x") == 0) {
        int value = _fromString<int>(attributeValue);
        m_x = value;
        app.DebugPrintf(
            "XboxStructureActionPlaceBlock: Adding parameter x=%d\n", m_x);
    } else if (attributeName.compare(L"y") == 0) {
        int value = _fromString<int>(attributeValue);
        m_y = value;
        app.DebugPrintf(
            "XboxStructureActionPlaceBlock: Adding parameter y=%d\n", m_y);
    } else if (attributeName.compare(L"z") == 0) {
        int value = _fromString<int>(attributeValue);
        m_z = value;
        app.DebugPrintf(
            "XboxStructureActionPlaceBlock: Adding parameter z=%d\n", m_z);
    } else if (attributeName.compare(L"block") == 0) {
        int value = _fromString<int>(attributeValue);
        m_tile = value;
        app.DebugPrintf(
            "XboxStructureActionPlaceBlock: Adding parameter block=%d\n",
            m_tile);
    } else if (attributeName.compare(L"data") == 0) {
        int value = _fromString<int>(attributeValue);
        m_data = value;
        app.DebugPrintf(
            "XboxStructureActionPlaceBlock: Adding parameter data=%d\n",
            m_data);
    } else {
        GameRuleDefinition::addAttribute(attributeName, attributeValue);
    }
}

bool XboxStructureActionPlaceBlock::placeBlockInLevel(StructurePiece* structure,
                                                      Level* level,
                                                      BoundingBox* chunkBB) {
    app.DebugPrintf("XboxStructureActionPlaceBlock - placing a block\n");
    structure->placeBlock(level, m_tile, m_data, m_x, m_y, m_z, chunkBB);
    return true;
}