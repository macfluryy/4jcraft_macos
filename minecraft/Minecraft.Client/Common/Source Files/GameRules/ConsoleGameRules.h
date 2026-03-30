#pragma once
#include "ConsoleGameRulesConstants.h"

#include "GameRuleManager.h"

#include "LevelRules/Rules/GameRule.h"

#include "LevelRules/RuleDefinitions/GameRuleDefinition.h"

#include "LevelRules/RuleDefinitions/LevelRuleset.h"
#include "LevelRules/RuleDefinitions/NamedAreaRuleDefinition.h"

#include "LevelRules/RuleDefinitions/CollectItemRuleDefinition.h"
#include "LevelRules/RuleDefinitions/CompleteAllRuleDefinition.h"
#include "LevelRules/RuleDefinitions/CompoundGameRuleDefinition.h"
#include "LevelRules/RuleDefinitions/UseTileRuleDefinition.h"
#include "LevelRules/RuleDefinitions/UpdatePlayerRuleDefinition.h"
#include "LevelRules/RuleDefinitions/AddItemRuleDefinition.h"
#include "LevelRules/RuleDefinitions/AddEnchantmentRuleDefinition.h"

#include "LevelGeneration/LevelGenerationOptions.h"
#include "LevelGeneration/ApplySchematicRuleDefinition.h"
#include "LevelGeneration/ConsoleGenerateStructure.h"
#include "LevelGeneration/ConsoleGenerateStructureAction.h"
#include "LevelGeneration/StructureActions/XboxStructureActionGenerateBox.h"
#include "LevelGeneration/StructureActions/XboxStructureActionPlaceBlock.h"
#include "LevelGeneration/StructureActions/XboxStructureActionPlaceContainer.h"
#include "LevelGeneration/StructureActions/XboxStructureActionPlaceSpawner.h"
#include "LevelGeneration/BiomeOverride.h"
#include "LevelGeneration/StartFeature.h"

#include "LevelRules/Rules/GameRulesInstance.h"
