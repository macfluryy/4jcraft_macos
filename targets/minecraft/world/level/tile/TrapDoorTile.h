#pragma once

#include <memory>

#include "Tile.h"

class Player;
class HitResult;
class Level;
class Material;

class TrapDoorTile : public Tile {
    friend class Tile;

private:
    static const int TOP_MASK = 0x8;

protected:
    TrapDoorTile(int id, Material* material);

    








public:
    bool blocksLight();

public:
    bool isSolidRender(bool isServerLevel = false);

public:
    bool isCubeShaped();
    bool isPathfindable(LevelSource* level, int x, int y, int z);

public:
    int getRenderShape();

public:
    AABB getTileAABB(Level* level, int x, int y, int z);

public:
    std::optional<AABB> getAABB(Level* level, int x, int y, int z);

public:
    void updateShape(LevelSource* level, int x, int y, int z,
                     int forceData = -1,
                     std::shared_ptr<TileEntity> forceEntity =
                         std::shared_ptr<TileEntity>());  
                                                          

public:
    void updateDefaultShape();

public:
    using Tile::setShape;
    void setShape(int data);

public:
    void attack(Level* level, int x, int y, int z,
                std::shared_ptr<Player> player);

public:
    virtual bool TestUse();
    bool use(Level* level, int x, int y, int z, std::shared_ptr<Player> player,
             int clickedFace, float clickX, float clickY, float clickZ,
             bool soundOnly = false);  

public:
    void setOpen(Level* level, int x, int y, int z, bool shouldOpen);

public:
    void neighborChanged(Level* level, int x, int y, int z, int type);

public:
    HitResult* clip(Level* level, int xt, int yt, int zt, Vec3* a, Vec3* b);

public:
    int getDir(int dir);

public:
    int getPlacedOnFaceDataValue(Level* level, int x, int y, int z, int face,
                                 float clickX, float clickY, float clickZ,
                                 int itemValue);

public:
    bool mayPlace(Level* level, int x, int y, int z, int face);

public:
    static bool isOpen(int data);

private:
    static bool attachesTo(int id);
};
