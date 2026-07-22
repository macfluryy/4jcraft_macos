#pragma once

#include <string>
#include <vector>

#include "app/common/src/GameRules/ConsoleGameRulesConstants.h"
#include "XboxStructureActionPlaceBlock.h"

class AddItemRuleDefinition;
class StructurePiece;
class Level;
class BoundingBox;

class XboxStructureActionPlaceContainer : public XboxStructureActionPlaceBlock {
private:
    std::vector<AddItemRuleDefinition*> m_items;

public:
    XboxStructureActionPlaceContainer();
    ~XboxStructureActionPlaceContainer();

    virtual ConsoleGameRules::EGameRuleType getActionType() {
        return ConsoleGameRules::eGameRuleType_PlaceContainer;
    }

    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition* addChild(
        ConsoleGameRules::EGameRuleType ruleType);

    
    
    

    virtual void addAttribute(const std::wstring& attributeName,
                              const std::wstring& attributeValue);

    bool placeContainerInLevel(StructurePiece* structure, Level* level,
                               BoundingBox* chunkBB);
};