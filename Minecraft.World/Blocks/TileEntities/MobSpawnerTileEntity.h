#pragma once

#include "TileEntity.h"

class Packet;
class Entity;

class MobSpawnerTileEntity : public TileEntity {
public:
    eINSTANCEOF GetType() { return eTYPE_MOBSPAWNERTILEENTITY; }
    static TileEntity* create() { return new MobSpawnerTileEntity(); }

    using TileEntity::setChanged;

private:
    static const int MAX_DIST;

public:
    int spawnDelay;

private:
    std::wstring entityId;
    CompoundTag* spawnData;

    bool m_bEntityIdUpdated;  // 4J Added

public:
    double spin, oSpin;

private:
    int minSpawnDelay;
    int maxSpawnDelay;
    int spawnCount;
    std::shared_ptr<Entity> displayEntity;

public:
    MobSpawnerTileEntity();

    std::wstring getEntityId();
    void setEntityId(const std::wstring& entityId);
    bool isNearPlayer();
    virtual void tick();
    void fillExtraData(std::shared_ptr<Entity> entity);

private:
    void delay();

public:
    virtual void load(CompoundTag* tag);
    virtual void save(CompoundTag* tag);

    std::shared_ptr<Entity> getDisplayEntity();
    virtual std::shared_ptr<Packet> getUpdatePacket();

    // 4J Added
    virtual std::shared_ptr<TileEntity> clone();
};
