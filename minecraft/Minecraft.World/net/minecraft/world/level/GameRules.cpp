#include <assert.h>

#include "GameRules.h"
#include "Minecraft.Client/Common/App_enums.h"
#include "Minecraft.Client/Linux/Linux_App.h"

// 4J: GameRules isn't in use anymore, just routes any requests to app game host
// options, kept things commented out for context

const int GameRules::RULE_DOFIRETICK = 0;
const int GameRules::RULE_MOBGRIEFING = 1;
const int GameRules::RULE_KEEPINVENTORY = 2;
const int GameRules::RULE_DOMOBSPAWNING = 3;
const int GameRules::RULE_DOMOBLOOT = 4;
const int GameRules::RULE_DOTILEDROPS = 5;
// const int GameRules::RULE_COMMANDBLOCKOUTPUT = 6;
const int GameRules::RULE_NATURAL_REGENERATION = 7;
const int GameRules::RULE_DAYLIGHT = 8;

GameRules::GameRules() {
    /*registerRule(RULE_DOFIRETICK, L"1");
    registerRule(RULE_MOBGRIEFING, L"1");
    registerRule(RULE_KEEPINVENTORY, L"0");
    registerRule(RULE_DOMOBSPAWNING, L"1");
    registerRule(RULE_DOMOBLOOT, L"1");
    registerRule(RULE_DOTILEDROPS, L"1");
    registerRule(RULE_COMMANDBLOCKOUTPUT, L"1");
    registerRule(RULE_NATURAL_REGENERATION, L"1");
    registerRule(RULE_DAYLIGHT, L"1");*/
}

GameRules::~GameRules() {
    /*for(auto it = rules.begin(); it != rules.end(); ++it)
    {
            delete it->second;
    }*/
}

bool GameRules::getBoolean(const int rule) {
    switch (rule) {
        case GameRules::RULE_DOFIRETICK:
            return app.GetGameHostOption(eGameHostOption_FireSpreads);
        case GameRules::RULE_MOBGRIEFING:
            return app.GetGameHostOption(eGameHostOption_MobGriefing);
        case GameRules::RULE_KEEPINVENTORY:
            return app.GetGameHostOption(eGameHostOption_KeepInventory);
        case GameRules::RULE_DOMOBSPAWNING:
            return app.GetGameHostOption(eGameHostOption_DoMobSpawning);
        case GameRules::RULE_DOMOBLOOT:
            return app.GetGameHostOption(eGameHostOption_DoMobLoot);
        case GameRules::RULE_DOTILEDROPS:
            return app.GetGameHostOption(eGameHostOption_DoTileDrops);
        case GameRules::RULE_NATURAL_REGENERATION:
            return app.GetGameHostOption(eGameHostOption_NaturalRegeneration);
        case GameRules::RULE_DAYLIGHT:
            return app.GetGameHostOption(eGameHostOption_DoDaylightCycle);
        default:
            assert(0);
            return false;
    }
}

/*
void GameRules::registerRule(const std::wstring &name, const std::wstring
&startValue)
{
        rules[name] = new GameRule(startValue);
}

void GameRules::set(const std::wstring &ruleName, const std::wstring &newValue)
{
        auto it = rules.find(ruleName);
        if(it != rules.end() )
        {
                GameRule *gameRule = it->second;
                gameRule->set(newValue);
        }
        else
        {
                registerRule(ruleName, newValue);
        }
}

std::wstring GameRules::get(const std::wstring &ruleName)
{
        auto it = rules.find(ruleName);
        if(it != rules.end() )
        {
                GameRule *gameRule = it->second;
                return gameRule->get();
        }
        return L"";
}

int GameRules::getInt(const std::wstring &ruleName)
{
        auto it = rules.find(ruleName);
        if(it != rules.end() )
        {
                GameRule *gameRule = it->second;
                return gameRule->getInt();
        }
        return 0;
}

double GameRules::getDouble(const std::wstring &ruleName)
{
        auto it = rules.find(ruleName);
        if(it != rules.end() )
        {
                GameRule *gameRule = it->second;
                return gameRule->getDouble();
        }
        return 0;
}

CompoundTag *GameRules::createTag()
{
        CompoundTag *result = new CompoundTag(L"GameRules");

        for(auto it = rules.begin(); it != rules.end(); ++it)
        {
                GameRule *gameRule = it->second;
                result->putString(it->first, gameRule->get());
        }

        return result;
}

void GameRules::loadFromTag(CompoundTag *tag)
{
        vector<Tag *> allTags = tag->getAllTags();
        for (auto it = allTags.begin(); it != allTags.end(); ++it)
        {
                Tag *ruleTag = *it;
                std::wstring ruleName = ruleTag->getName();
                std::wstring value = tag->getString(ruleTag->getName());

                set(ruleName, value);
        }
}

// Need to delete returned vector.
vector<std::wstring> *GameRules::getRuleNames()
{
        vector<std::wstring> *out = new vector<std::wstring>();
        for (auto it = rules.begin(); it != rules.end(); it++)
out->push_back(it->first); return out;
}

bool GameRules::contains(const std::wstring &rule)
{
        auto it = rules.find(rule);
        return it != rules.end();
}

GameRules::GameRule::GameRule(const std::wstring &startValue)
{
        value = L"";
        booleanValue = false;
        intValue = 0;
        doubleValue = 0.0;
        set(startValue);
}

void GameRules::GameRule::set(const std::wstring &newValue)
{
        value = newValue;
        booleanValue = _fromString<bool>(newValue);
        intValue = _fromString<int>(newValue);
        doubleValue = _fromString<double>(newValue);
}

std::wstring GameRules::GameRule::get()
{
        return value;
}

bool GameRules::GameRule::getBoolean()
{
        return booleanValue;
}

int GameRules::GameRule::getInt()
{
        return intValue;
}

double GameRules::GameRule::getDouble()
{
        return doubleValue;
}*/