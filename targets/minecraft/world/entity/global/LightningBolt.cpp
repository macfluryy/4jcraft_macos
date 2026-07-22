#include "LightningBolt.h"

#include <math.h>

#include <memory>
#include <vector>

#include "java/Random.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/sounds/SoundTypes.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/global/GlobalEntity.h"
#include "minecraft/world/level/GameRules.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/tile/FireTile.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/phys/AABB.h"

LightningBolt::LightningBolt(Level* level, double x, double y, double z)
    : life(0), seed(0), flashes(0), GlobalEntity(level) {
    
    
    this->defineSynchedData();

    moveTo(x, y, z, 0, 0);
    life = START_LIFE;
    seed = random->nextLong();
    
    
    
    flashes = 1;

    
    if (!level->isClientSide &&
        level->getGameRules()->getBoolean(GameRules::RULE_DOFIRETICK) &&
        level->difficulty >= 2 &&
        level->hasChunksAt(Mth::floor(x), Mth::floor(y), Mth::floor(z), 10)) {
        {
            int xt = Mth::floor(x);
            int yt = Mth::floor(y);
            int zt = Mth::floor(z);
            
            
            if (MinecraftServer::getInstance()->getPlayers()->isTrackingTile(
                    xt, yt, zt, level->dimension->id)) {
                if (level->getTile(xt, yt, zt) == 0 &&
                    Tile::fire->mayPlace(level, xt, yt, zt))
                    level->setTileAndUpdate(xt, yt, zt, Tile::fire_Id);
            }
        }

        for (int i = 0; i < 4; i++) {
            int xt = Mth::floor(x) + random->nextInt(3) - 1;
            int yt = Mth::floor(y) + random->nextInt(3) - 1;
            int zt = Mth::floor(z) + random->nextInt(3) - 1;
            
            
            if (MinecraftServer::getInstance()->getPlayers()->isTrackingTile(
                    xt, yt, zt, level->dimension->id)) {
                if (level->getTile(xt, yt, zt) == 0 &&
                    Tile::fire->mayPlace(level, xt, yt, zt))
                    level->setTileAndUpdate(xt, yt, zt, Tile::fire_Id);
            }
        }
    }
}

void LightningBolt::tick() {
    GlobalEntity::tick();

    if (life == START_LIFE) {
        
        
        
        level->playSound(x, y, z, eSoundType_AMBIENT_WEATHER_THUNDER, 10000,
                         0.8f + random->nextFloat() * 0.2f);
        level->playSound(x, y, z, eSoundType_RANDOM_EXPLODE, 2,
                         0.5f + random->nextFloat() * 0.2f);
    }

    life--;
    if (life < 0) {
        if (flashes == 0) {
            remove();
        } else if (life < -random->nextInt(10)) {
            flashes--;
            life = 1;

            seed = random->nextLong();
            if (!level->isClientSide &&
                level->getGameRules()->getBoolean(GameRules::RULE_DOFIRETICK) &&
                level->hasChunksAt((int)floor(x), (int)floor(y), (int)floor(z),
                                   10)) {
                int xt = (int)floor(x);
                int yt = (int)floor(y);
                int zt = (int)floor(z);

                
                
                if (MinecraftServer::getInstance()
                        ->getPlayers()
                        ->isTrackingTile(xt, yt, zt, level->dimension->id)) {
                    if (level->getTile(xt, yt, zt) == 0 &&
                        Tile::fire->mayPlace(level, xt, yt, zt))
                        level->setTileAndUpdate(xt, yt, zt, Tile::fire_Id);
                }
            }
        }
    }

    if (life >= 0) {
        if (level->isClientSide) {
            level->skyFlashTime = 2;
        } else {
            double r = 3;
            AABB aoe_bb = AABB(x, y, z, x, y + 6, z).grow(r, r, r);
            std::vector<std::shared_ptr<Entity> > entities;
            level->getEntities(shared_from_this(), &aoe_bb, entities);
            auto itEnd = entities.end();
            for (auto it = entities.begin(); it != itEnd; it++) {
                std::shared_ptr<Entity> e = (*it);  
                e->thunderHit(this);
            }
        }
    }
}

void LightningBolt::defineSynchedData() {}

void LightningBolt::readAdditionalSaveData(CompoundTag* tag) {}

void LightningBolt::addAdditonalSaveData(CompoundTag* tag) {}

bool LightningBolt::shouldAlwaysRender() { return true; }

bool LightningBolt::shouldRender(Vec3* c) { return life >= 0; }
