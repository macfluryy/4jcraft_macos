#pragma once

#include <string>

#include "app/common/src/GameRules/ConsoleGameRulesConstants.h"
#include "XboxStructureActionPlaceBlock.h"

class StructurePiece;
class Level;
class BoundingBox;
class GRFObject;

class XboxStructureActionPlaceSpawner : public XboxStructureActionPlaceBlock {
private:
    std::wstring m_entityId;

public:
    XboxStructureActionPlaceSpawner();
    ~XboxStructureActionPlaceSpawner();

    virtual ConsoleGameRules::EGameRuleType getActionType() {
        return ConsoleGameRules::eGameRuleType_PlaceSpawner;
    }

    virtual void writeAttributes(DataOutputStream* dos, unsigned int numAttrs);
    virtual void addAttribute(const std::wstring& attributeName,
                              const std::wstring& attributeValue);

    bool placeSpawnerInLevel(StructurePiece* structure, Level* level,
                             BoundingBox* chunkBB);
};