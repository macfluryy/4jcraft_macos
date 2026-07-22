
#include "GameMode.h"

#include "minecraft/client/player/LocalPlayer.h"
#include "minecraft/client/renderer/LevelRenderer.h"








GameMode::GameMode(Minecraft* minecraft) {
    instaBuild = false;  
    this->minecraft = minecraft;
}

void GameMode::initLevel(Level* level) {}

bool GameMode::destroyBlock(int x, int y, int z, int face) {
    Level* level = minecraft->level;
    Tile* oldTile = Tile::tiles[level->getTile(x, y, z)];
    if (oldTile == nullptr) return false;

    
    
    minecraft->levelRenderer->destroyedTileManager->destroyingTileAt(level, x,
                                                                     y, z);
    level->levelEvent(
        LevelEvent::PARTICLES_DESTROY_BLOCK, x, y, z,
        oldTile->id + (level->getData(x, y, z) << Tile::TILE_NUM_SHIFT));
    int data = level->getData(x, y, z);
    
    
    
    
    level->getChunkAt(x, z)->recalcHeightmapOnly();
    bool changed = level->setTile(x, y, z, 0);

    if (oldTile != nullptr && changed) {
        oldTile->destroy(level, x, y, z, data);
    }
    return changed;
}

void GameMode::render(float a) {}

bool GameMode::useItem(std::shared_ptr<Player> player, Level* level,
                       std::shared_ptr<ItemInstance> item, bool bTestUseOnly) {}

void GameMode::initPlayer(std::shared_ptr<Player> player) {}

void GameMode::tick() {}

void GameMode::adjustPlayer(std::shared_ptr<Player> player) {}










































std::shared_ptr<Player> GameMode::createPlayer(Level* level) {
    return std::make_shared<LocalPlayer>(minecraft, level, minecraft->user,
                                         level->dimension->id);
}

bool GameMode::interact(std::shared_ptr<Player> player,
                        std::shared_ptr<Entity> entity) {
    return player->interact(entity);
}

void GameMode::attack(std::shared_ptr<Player> player,
                      std::shared_ptr<Entity> entity) {
    player->attack(entity);
}

std::shared_ptr<ItemInstance> GameMode::handleInventoryMouseClick(
    int containerId, int slotNum, int buttonNum, bool quickKeyHeld,
    std::shared_ptr<Player> player) {
    return nullptr;
}

void GameMode::handleCloseInventory(int containerId,
                                    std::shared_ptr<Player> player) {
    player->containerMenu->removed(player);
    delete player->containerMenu;
    player->containerMenu = player->inventoryMenu;
}

void GameMode::handleInventoryButtonClick(int containerId, int buttonId) {}

bool GameMode::isCutScene() { return false; }

void GameMode::releaseUsingItem(std::shared_ptr<Player> player) {
    player->releaseUsingItem();
}

bool GameMode::hasExperience() { return false; }

bool GameMode::hasMissTime() { return true; }

bool GameMode::hasInfiniteItems() { return false; }

bool GameMode::hasFarPickRange() { return false; }

void GameMode::handleCreativeModeItemAdd(std::shared_ptr<ItemInstance> clicked,
                                         int i) {}

void GameMode::handleCreativeModeItemDrop(
    std::shared_ptr<ItemInstance> clicked) {}

bool GameMode::handleCraftItem(int recipe, std::shared_ptr<Player> player) {
    return true;
}


void GameMode::handleDebugOptions(unsigned int uiVal,
                                  std::shared_ptr<Player> player) {
    player->SetDebugOptions(uiVal);
}
