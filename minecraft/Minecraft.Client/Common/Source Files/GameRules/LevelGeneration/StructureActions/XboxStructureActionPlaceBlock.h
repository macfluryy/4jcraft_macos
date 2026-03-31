#pragma once
#include <string>

#include "Minecraft.Client/Common/Source Files/GameRules/LevelGeneration/ConsoleGenerateStructureAction.h"
#include "Minecraft.Client/Common/Source Files/GameRules/ConsoleGameRulesConstants.h"

class StructurePiece;
class Level;
class BoundingBox;

class XboxStructureActionPlaceBlock : public ConsoleGenerateStructureAction {
protected:
    int m_x, m_y, m_z, m_tile, m_data;

public:
    XboxStructureActionPlaceBlock();

    virtual ConsoleGameRules::EGameRuleType getActionType() {
        return ConsoleGameRules::eGameRuleType_PlaceBlock;
    }

    virtual int getEndX() { return m_x; }
    virtual int getEndY() { return m_y; }
    virtual int getEndZ() { return m_z; }

    virtual void writeAttributes(DataOutputStream* dos, unsigned int numAttrs);
    virtual void addAttribute(const std::wstring& attributeName,
                              const std::wstring& attributeValue);

    bool placeBlockInLevel(StructurePiece* structure, Level* level,
                           BoundingBox* chunkBB);
};