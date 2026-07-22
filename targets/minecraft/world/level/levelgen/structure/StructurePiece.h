#pragma once

#include <list>
#include <vector>

#include "BoundingBox.h"
#include "StructureFeatureIO.h"
#include "minecraft/util/WeighedRandom.h"
#include "nbt/CompoundTag.h"

class Level;
class Random;
class ChunkPos;
class BlockSelector;
class ChestTileEntity;
class WeighedTreasure;
class TilePos;
class BoundingBox;
class CompoundTag;





























class StructurePiece {
public:
    virtual EStructurePiece GetType() = 0;

public:
    class BlockSelector {
    protected:
        int nextId;
        int nextData;

    public:
        virtual void next(Random* random, int worldX, int worldY, int worldZ,
                          bool isEdge) {}

        virtual int getNextId() { return nextId; }
        virtual int getNextData() { return nextData; }
    };

public
    :  
    BoundingBox* boundingBox;

protected:
    int orientation;
    int genDepth;

public:
    StructurePiece();

protected:
    StructurePiece(int genDepth);

public:
    virtual ~StructurePiece();

    virtual CompoundTag* createTag();

protected:
    virtual void addAdditonalSaveData(CompoundTag* tag) = 0;

public:
    virtual void load(Level* level, CompoundTag* tag);

protected:
    virtual void readAdditonalSaveData(CompoundTag* tag) = 0;

public:
    virtual void addChildren(StructurePiece* startPiece,
                             std::list<StructurePiece*>* pieces,
                             Random* random);
    virtual bool postProcess(Level* level, Random* random,
                             BoundingBox* chunkBB) = 0;

    virtual BoundingBox* getBoundingBox();

    int getGenDepth();

public:
    bool isInChunk(ChunkPos* pos);
    static StructurePiece* findCollisionPiece(
        std::list<StructurePiece*>* pieces, BoundingBox* box);
    virtual TilePos* getLocatorPosition();

protected:
    bool edgesLiquid(Level* level, BoundingBox* chunkBB);

public:
    
    int getWorldX(int x, int z);
    int getWorldY(int y);
    int getWorldZ(int x, int z);
    int getOrientationData(int tile, int data);
    virtual void placeBlock(Level* level, int block, int data, int x, int y,
                            int z, BoundingBox* chunkBB);

    











    virtual int getBlock(Level* level, int x, int y, int z,
                         BoundingBox* chunkBB);
    virtual void generateAirBox(Level* level, BoundingBox* chunkBB, int x0,
                                int y0, int z0, int x1, int y1, int z1);
    virtual void generateBox(Level* level, BoundingBox* chunkBB, int x0, int y0,
                             int z0, int x1, int y1, int z1, int edgeTile,
                             int fillTile, bool skipAir);
    virtual void generateBox(Level* level, BoundingBox* chunkBB, int x0, int y0,
                             int z0, int x1, int y1, int z1, int edgeTile,
                             int edgeData, int fillTile, int fillData,
                             bool skipAir);
    virtual void generateBox(Level* level, BoundingBox* chunkBB,
                             BoundingBox* boxBB, int edgeTile, int fillTile,
                             bool skipAir);
    virtual void generateBox(Level* level, BoundingBox* chunkBB, int x0, int y0,
                             int z0, int x1, int y1, int z1, bool skipAir,
                             Random* random, BlockSelector* selector);
    virtual void generateBox(Level* level, BoundingBox* chunkBB,
                             BoundingBox* boxBB, bool skipAir, Random* random,
                             BlockSelector* selector);
    virtual void generateMaybeBox(Level* level, BoundingBox* chunkBB,
                                  Random* random, float probability, int x0,
                                  int y0, int z0, int x1, int y1, int z1,
                                  int edgeTile, int fillTile, bool skipAir);
    virtual void maybeGenerateBlock(Level* level, BoundingBox* chunkBB,
                                    Random* random, float probability, int x,
                                    int y, int z, int tile, int data);
    virtual void generateUpperHalfSphere(Level* level, BoundingBox* chunkBB,
                                         int x0, int y0, int z0, int x1, int y1,
                                         int z1, int fillTile, bool skipAir);
    virtual void generateAirColumnUp(Level* level, int x, int startY, int z,
                                     BoundingBox* chunkBB);
    virtual void fillColumnDown(Level* level, int tile, int tileData, int x,
                                int startY, int z, BoundingBox* chunkBB);
    virtual bool createChest(Level* level, BoundingBox* chunkBB, Random* random,
                             int x, int y, int z,
                             const std::vector<WeighedTreasure*>& treasure,
                             int numRolls);
    virtual bool createDispenser(Level* level, BoundingBox* chunkBB,
                                 Random* random, int x, int y, int z,
                                 int facing,
                                 const std::vector<WeighedTreasure*>& items,
                                 int numRolls);

protected:
    void createDoor(Level* level, BoundingBox* chunkBB, Random* random, int x,
                    int y, int z, int orientation);
};
