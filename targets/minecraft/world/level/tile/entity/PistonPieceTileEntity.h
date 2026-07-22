#pragma once
#include "TileEntity.h"
#include "java/Class.h"

class PistonPieceEntity : public TileEntity {
public:
    eINSTANCEOF GetType() { return eTYPE_PISTONPIECEENTITY; }
    static TileEntity* create() { return new PistonPieceEntity(); }

private:
    int id;
    int data;
    int facing;
    bool extending;
    bool _isSourcePiston;

    float progress, progressO;

public:
    PistonPieceEntity();
    PistonPieceEntity(int id, int data, int facing, bool extending,
                      bool isSourcePiston);
    int getId();
    virtual int getData();
    bool isExtending();
    int getFacing();
    bool isSourcePiston();
    float getProgress(float a);
    float getXOff(float a);
    float getYOff(float a);
    float getZOff(float a);

private:
    
    
    void moveCollidedEntities(float progress, float amount);

public:
    void finalTick();
    virtual void tick();
    virtual void load(CompoundTag* tag);
    virtual void save(CompoundTag* tag);

    
    std::shared_ptr<TileEntity> clone();
};