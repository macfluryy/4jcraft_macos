#pragma once
#include <string>

#include "Minecraft.Client/Common/Source Files/GameRules/LevelGeneration/ConsoleGenerateStructureAction.h"
#include "Minecraft.Client/Common/Source Files/GameRules/ConsoleGameRulesConstants.h"

class StructurePiece;
class Level;
class BoundingBox;

class XboxStructureActionGenerateBox : public ConsoleGenerateStructureAction {
private:
    int m_x0, m_y0, m_z0, m_x1, m_y1, m_z1, m_edgeTile, m_fillTile;
    bool m_skipAir;

public:
    XboxStructureActionGenerateBox();

    virtual ConsoleGameRules::EGameRuleType getActionType() {
        return ConsoleGameRules::eGameRuleType_GenerateBox;
    }

    virtual int getEndX() { return m_x1; }
    virtual int getEndY() { return m_y1; }
    virtual int getEndZ() { return m_z1; }

    virtual void writeAttributes(DataOutputStream* dos, unsigned int numAttrs);
    virtual void addAttribute(const std::wstring& attributeName,
                              const std::wstring& attributeValue);

    bool generateBoxInLevel(StructurePiece* structure, Level* level,
                            BoundingBox* chunkBB);
};