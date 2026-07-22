#include "GameRules.h"

#include <assert.h>

#include "app/common/App_enums.h"
#include "app/mac/MacGame.h"




const int GameRules::RULE_DOFIRETICK = 0;
const int GameRules::RULE_MOBGRIEFING = 1;
const int GameRules::RULE_KEEPINVENTORY = 2;
const int GameRules::RULE_DOMOBSPAWNING = 3;
const int GameRules::RULE_DOMOBLOOT = 4;
const int GameRules::RULE_DOTILEDROPS = 5;

const int GameRules::RULE_NATURAL_REGENERATION = 7;
const int GameRules::RULE_DAYLIGHT = 8;

GameRules::GameRules() {
    








}

GameRules::~GameRules() {
    



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


































































































































