#include "../Platform/stdafx.h"
#include "TheEndPortalTile.h"
#include "TileEntities/TheEndPortalTileEntity.h"
#include "../Headers/net.minecraft.world.level.h"
#include "../Headers/net.minecraft.world.level.dimension.h"
#include "../Headers/net.minecraft.world.level.storage.h"
#include "../Headers/net.minecraft.world.entity.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.h"
#include <cstdint>

namespace {
#if defined(_WIN32)
inline void* TheEndPortalTlsGetValue(DWORD key) { return TlsGetValue(key); }

inline void TheEndPortalTlsSetValue(DWORD key, void* value) {
    TlsSetValue(key, value);
}
#else
pthread_key_t CreateTheEndPortalTlsKey() {
    pthread_key_t key;
    pthread_key_create(&key, NULL);
    return key;
}

inline void* TheEndPortalTlsGetValue(pthread_key_t key) {
    return pthread_getspecific(key);
}

inline void TheEndPortalTlsSetValue(pthread_key_t key, void* value) {
    pthread_setspecific(key, value);
}
#endif
}  // namespace

#if defined(_WIN32)
DWORD TheEndPortal::tlsIdx = TlsAlloc();
#else
pthread_key_t TheEndPortal::tlsIdx = CreateTheEndPortalTlsKey();
#endif

// 4J - allowAnywhere is a static in java, implementing as TLS here to make
// thread safe
bool TheEndPortal::allowAnywhere() {
    return TheEndPortalTlsGetValue(tlsIdx) != NULL;
}

void TheEndPortal::allowAnywhere(bool set) {
    TheEndPortalTlsSetValue(
        tlsIdx, reinterpret_cast<void*>(static_cast<intptr_t>(set ? 1 : 0)));
}

TheEndPortal::TheEndPortal(int id, Material* material)
    : EntityTile(id, material, false) {
    this->setLightEmission(1.0f);
}

std::shared_ptr<TileEntity> TheEndPortal::newTileEntity(Level* level) {
    return std::shared_ptr<TileEntity>(new TheEndPortalTileEntity());
}

void TheEndPortal::updateShape(
    LevelSource* level, int x, int y, int z, int forceData,
    std::shared_ptr<TileEntity>
        forceEntity)  // 4J added forceData, forceEntity param
{
    float r = 1 / 16.0f;
    this->setShape(0, 0, 0, 1, r, 1);
}

bool TheEndPortal::shouldRenderFace(LevelSource* level, int x, int y, int z,
                                    int face) {
    if (face != 0) return false;
    return EntityTile::shouldRenderFace(level, x, y, z, face);
}

void TheEndPortal::addAABBs(Level* level, int x, int y, int z, AABB* box,
                            AABBList* boxes, std::shared_ptr<Entity> source) {}

bool TheEndPortal::isSolidRender(bool isServerLevel) { return false; }

bool TheEndPortal::isCubeShaped() { return false; }

int TheEndPortal::getResourceCount(Random* random) { return 0; }

void TheEndPortal::entityInside(Level* level, int x, int y, int z,
                                std::shared_ptr<Entity> entity) {
    if (entity->riding == NULL && entity->rider.lock() == NULL) {
        if (std::dynamic_pointer_cast<Player>(entity) != NULL) {
            if (!level->isClientSide) {
                // 4J Stu - Update the level data position so that the
                // stronghold portal can be shown on the maps
                int x, z;
                x = z = 0;
                if (level->dimension == 0 &&
                    !level->getLevelData()->getHasStrongholdEndPortal() &&
                    app.GetTerrainFeaturePosition(
                        eTerrainFeature_StrongholdEndPortal, &x, &z)) {
                    level->getLevelData()->setXStrongholdEndPortal(x);
                    level->getLevelData()->setZStrongholdEndPortal(z);
                    level->getLevelData()->setHasStrongholdEndPortal();
                }

                (std::dynamic_pointer_cast<Player>(entity))->changeDimension(1);
            }
        }
    }
}

void TheEndPortal::animateTick(Level* level, int xt, int yt, int zt,
                               Random* random) {
    double x = xt + random->nextFloat();
    double y = yt + 0.8f;
    double z = zt + random->nextFloat();
    double xa = 0;
    double ya = 0;
    double za = 0;

    level->addParticle(eParticleType_endportal, x, y, z, xa, ya, za);
}

int TheEndPortal::getRenderShape() { return SHAPE_INVISIBLE; }

void TheEndPortal::onPlace(Level* level, int x, int y, int z) {
    if (allowAnywhere()) return;

    if (level->dimension->id != 0) {
        level->setTile(x, y, z, 0);
        return;
    }
}

int TheEndPortal::cloneTileId(Level* level, int x, int y, int z) { return 0; }

void TheEndPortal::registerIcons(IconRegister* iconRegister) {
    // don't register null, because of particles
    icon = iconRegister->registerIcon(L"portal");
}
