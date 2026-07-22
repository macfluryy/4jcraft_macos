#include "Explosion.h"

#include <math.h>
#include <stddef.h>

#include <utility>
#include <vector>

#include "app/mac/MacGame.h"
#include "java/Class.h"
#include "java/Random.h"
#include "minecraft/core/particles/ParticleTypes.h"
#include "minecraft/sounds/SoundTypes.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/damageSource/DamageSource.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/LivingEntity.h"
#include "minecraft/world/entity/item/PrimedTnt.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/item/enchantment/ProtectionEnchantment.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/TilePos.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/phys/AABB.h"
#include "minecraft/world/phys/Vec3.h"

Explosion::Explosion(Level* level, std::shared_ptr<Entity> source, double x,
                     double y, double z, float r) {
    fire = false;
    random = new Random();

    this->level = level;
    this->source = source;
    this->r = r;
    this->x = x;
    this->y = y;
    this->z = z;

    destroyBlocks = true;
    size = 16;
}

Explosion::~Explosion() { delete random; }

void Explosion::explode() {
    float oR = r;

    int size = 16;
    for (int xx = 0; xx < size; xx++) {
        for (int yy = 0; yy < size; yy++) {
            for (int zz = 0; zz < size; zz++) {
                if ((xx != 0 && xx != size - 1) &&
                    (yy != 0 && yy != size - 1) && (zz != 0 && zz != size - 1))
                    continue;

                double xd = xx / (size - 1.0f) * 2 - 1;
                double yd = yy / (size - 1.0f) * 2 - 1;
                double zd = zz / (size - 1.0f) * 2 - 1;
                double d = sqrt(xd * xd + yd * yd + zd * zd);

                xd /= d;
                yd /= d;
                zd /= d;

                float remainingPower =
                    r * (0.7f + level->random->nextFloat() * 0.6f);
                double xp = x;
                double yp = y;
                double zp = z;

                float stepSize = 0.3f;
                while (remainingPower > 0) {
                    int xt = Mth::floor(xp);
                    int yt = Mth::floor(yp);
                    int zt = Mth::floor(zp);
                    int t = level->getTile(xt, yt, zt);
                    if (t > 0) {
                        Tile* tile = Tile::tiles[t];
                        float resistance =
                            source != nullptr
                                ? source->getTileExplosionResistance(
                                      this, level, xt, yt, zt, tile)
                                : tile->getExplosionResistance(source);
                        remainingPower -= (resistance + 0.3f) * stepSize;
                    }
                    if (remainingPower > 0 &&
                        (source == nullptr ||
                         source->shouldTileExplode(this, level, xt, yt, zt, t,
                                                   remainingPower))) {
                        toBlow.insert(TilePos(xt, yt, zt));
                    }

                    xp += xd * stepSize;
                    yp += yd * stepSize;
                    zp += zd * stepSize;
                    remainingPower -= stepSize * 0.75f;
                }
                
            }
        }
    }

    r *= 2.0f;
    int x0 = Mth::floor(x - r - 1);
    int x1 = Mth::floor(x + r + 1);
    int y0 = Mth::floor(y - r - 1);
    int y1 = Mth::floor(y + r + 1);
    int z0 = Mth::floor(z - r - 1);
    int z1 = Mth::floor(z + r + 1);

    
    
    
    
    

    AABB source_bb(x0, y0, z0, x1, y1, z1);
    
    
    
    std::vector<std::shared_ptr<Entity> > entities;
    level->getEntities(source, &source_bb, entities);
    Vec3 center(x, y, z);

    auto itEnd = entities.end();
    for (auto it = entities.begin(); it != itEnd; it++) {
        std::shared_ptr<Entity> e = *it;  

        
        
        
        
        bool canDamage = false;
        for (auto it2 = toBlow.begin(); it2 != toBlow.end(); ++it2) {
            if (e->bb.intersects(it2->x, it2->y, it2->z, it2->x + 1, it2->y + 1,
                                 it2->z + 1)) {
                canDamage = true;
                break;
            }
        }

        double dist = e->distanceTo(x, y, z) / r;
        if (dist <= 1) {
            double xa = e->x - x;
            double ya = e->y + e->getHeadHeight() - y;
            double za = e->z - z;

            double da = sqrt(xa * xa + ya * ya + za * za);

            
            
            
            if (da == 0) {
                xa = ya = za = 0.0;
            } else {
                xa /= da;
                ya /= da;
                za /= da;
            }

            double sp = level->getSeenPercent(&center, &e->bb);
            double pow = (1 - dist) * sp;
            if (canDamage)
                e->hurt(DamageSource::explosion(this),
                        (int)((pow * pow + pow) / 2 * 8 * r + 1));

            double kbPower =
                ProtectionEnchantment::getExplosionKnockbackAfterDampener(e,
                                                                          pow);
            e->xd += xa * kbPower;
            e->yd += ya * kbPower;
            e->zd += za * kbPower;

            if (e->instanceof(eTYPE_PLAYER)) {
                std::shared_ptr<Player> player =
                    std::dynamic_pointer_cast<Player>(e);
                
                
                hitPlayers.insert(playerVec3Map::value_type(
                    player, Vec3(xa * pow, ya * pow, za * pow)));
            }
        }
    }
    r = oR;
}

void Explosion::finalizeExplosion(
    bool generateParticles,
    std::vector<TilePos>*
        toBlowDirect )  
{
    level->playSound(
        x, y, z, eSoundType_RANDOM_EXPLODE, 4,
        (1 + (level->random->nextFloat() - level->random->nextFloat()) * 0.2f) *
            0.7f);
    if (r < 2 || !destroyBlocks) {
        level->addParticle(eParticleType_largeexplode, x, y, z, 1.0f, 0, 0);
    } else {
        level->addParticle(eParticleType_hugeexplosion, x, y, z, 1.0f, 0, 0);
    }

    
    
    std::vector<TilePos>* toBlowArray =
        toBlowDirect ? toBlowDirect
                     : new std::vector<TilePos>(toBlow.begin(), toBlow.end());
    if (destroyBlocks) {
        
        
        app.DebugPrintf("Finalizing explosion size %d\n", toBlow.size());
        static const int MAX_EXPLODE_PARTICLES = 50;
        
        int fraction = (int)toBlowArray->size() / MAX_EXPLODE_PARTICLES;
        if (fraction == 0) fraction = 1;
        size_t j = toBlowArray->size() - 1;
        
        for (auto it = toBlowArray->rbegin(); it != toBlowArray->rend(); ++it) {
            TilePos* tp = &(*it);  
            int xt = tp->x;
            int yt = tp->y;
            int zt = tp->z;
            
            
            int t = level->getTile(xt, yt, zt);

            if (generateParticles) {
                if ((j % fraction) == 0) {
                    double xa = xt + level->random->nextFloat();
                    double ya = yt + level->random->nextFloat();
                    double za = zt + level->random->nextFloat();

                    double xd = xa - x;
                    double yd = ya - y;
                    double zd = za - z;

                    double dd = sqrt(xd * xd + yd * yd + zd * zd);

                    xd /= dd;
                    yd /= dd;
                    zd /= dd;

                    double speed = 0.5 / (dd / r + 0.1);
                    speed *= (level->random->nextFloat() *
                                  level->random->nextFloat() +
                              0.3f);
                    xd *= speed;
                    yd *= speed;
                    zd *= speed;

                    level->addParticle(eParticleType_explode, (xa + x * 1) / 2,
                                       (ya + y * 1) / 2, (za + z * 1) / 2, xd,
                                       yd, zd);
                    level->addParticle(eParticleType_smoke, xa, ya, za, xd, yd,
                                       zd);
                }
            }

            if (t > 0) {
                Tile* tile = Tile::tiles[t];

                if (tile->dropFromExplosion(this)) {
                    tile->spawnResources(level, xt, yt, zt,
                                         level->getData(xt, yt, zt), 1.0f / r,
                                         0);
                }
                level->setTileAndData(xt, yt, zt, 0, 0, Tile::UPDATE_ALL);
                tile->wasExploded(level, xt, yt, zt, this);
            }

            --j;
        }
    }

    if (fire) {
        
        for (auto it = toBlowArray->rbegin(); it != toBlowArray->rend(); ++it) {
            TilePos* tp = &(*it);  
            int xt = tp->x;
            int yt = tp->y;
            int zt = tp->z;
            int t = level->getTile(xt, yt, zt);
            int b = level->getTile(xt, yt - 1, zt);
            if (t == 0 && Tile::solid[b] && random->nextInt(3) == 0) {
                level->setTileAndUpdate(xt, yt, zt, Tile::fire_Id);
            }
        }
    }

    if (toBlowDirect == nullptr) delete toBlowArray;
}

Explosion::playerVec3Map* Explosion::getHitPlayers() { return &hitPlayers; }

Vec3 Explosion::getHitPlayerKnockback(std::shared_ptr<Player> player) {
    auto it = hitPlayers.find(player);

    if (it == hitPlayers.end()) return Vec3(0.0, 0.0, 0.0);

    return it->second;
}

std::shared_ptr<LivingEntity> Explosion::getSourceMob() {
    if (source == nullptr) return nullptr;
    if (source->instanceof(eTYPE_PRIMEDTNT))
        return std::dynamic_pointer_cast<PrimedTnt>(source)->getOwner();
    if (source->instanceof(eTYPE_LIVINGENTITY))
        return std::dynamic_pointer_cast<LivingEntity>(source);
    return nullptr;
}
