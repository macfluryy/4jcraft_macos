#include "../../../../../Header Files/stdafx.h"
#include "../../level/net.minecraft.world.level.h"
#include "../../level/tile/net.minecraft.world.level.tile.h"
#include "../../item/net.minecraft.world.item.h"
#include "../player/net.minecraft.world.entity.player.h"
#include "../item/net.minecraft.world.entity.item.h"
#include "Minecraft.Client/net/minecraft/client/renderer/Textures.h"
#include "MushroomCow.h"
#include "../MobCategory.h"
#include "../../phys/AABB.h"

MushroomCow::MushroomCow(Level* level) : Cow(level) {
    // 4J Stu - This function call had to be moved here from the Entity ctor to
    // ensure that the derived version of the function is called
    this->defineSynchedData();
    setHealth(getMaxHealth());

    this->setSize(0.9f, 1.3f);
}

bool MushroomCow::mobInteract(std::shared_ptr<Player> player) {
    std::shared_ptr<ItemInstance> item = player->inventory->getSelected();
    if (item != nullptr && item->id == Item::bowl_Id && getAge() >= 0) {
        if (item->count == 1) {
            player->inventory->setItem(
                player->inventory->selected,
                std::shared_ptr<ItemInstance>(
                    new ItemInstance(Item::mushroomStew)));
            return true;
        }

        if (player->inventory->add(std::shared_ptr<ItemInstance>(
                new ItemInstance(Item::mushroomStew))) &&
            !player->abilities.instabuild) {
            player->inventory->removeItem(player->inventory->selected, 1);
            return true;
        }
    }
    // 4J: Do not allow shearing if we can't create more cows
    if (item != nullptr && item->id == Item::shears_Id && getAge() >= 0 &&
        level->canCreateMore(eTYPE_COW, Level::eSpawnType_Breed)) {
        remove();
        level->addParticle(eParticleType_largeexplode, x, y + bbHeight / 2, z,
                           0, 0, 0);
        if (!level->isClientSide) {
            remove();
            std::shared_ptr<Cow> cow = std::shared_ptr<Cow>(new Cow(level));
            cow->moveTo(x, y, z, yRot, xRot);
            cow->setHealth(getHealth());
            cow->yBodyRot = yBodyRot;
            level->addEntity(cow);
            for (int i = 0; i < 5; i++) {
                level->addEntity(std::shared_ptr<ItemEntity>(
                    new ItemEntity(level, x, y + bbHeight, z,
                                   std::shared_ptr<ItemInstance>(
                                       new ItemInstance(Tile::mushroom_red)))));
            }
            return true;
        }
        return true;
    }
    return Cow::mobInteract(player);
}

// 4J - added so that mushroom cows have more of a chance of spawning, they can
// now spawn on mycelium as well as grass - seems a bit odd that they don't
// already really
bool MushroomCow::canSpawn() {
    int xt = GameMath::floor(x);
    int yt = GameMath::floor(bb.y0);
    int zt = GameMath::floor(z);
    return (level->getTile(xt, yt - 1, zt) == Tile::grass_Id ||
            level->getTile(xt, yt - 1, zt) == Tile::mycel_Id) &&
           level->getDaytimeRawBrightness(xt, yt, zt) > 8 &&
           PathfinderMob::canSpawn();
}

std::shared_ptr<AgableMob> MushroomCow::getBreedOffspring(
    std::shared_ptr<AgableMob> target) {
    // 4J - added limit to number of animals that can be bred
    if (level->canCreateMore(GetType(), Level::eSpawnType_Breed)) {
        return std::shared_ptr<MushroomCow>(new MushroomCow(level));
    } else {
        return nullptr;
    }
}
