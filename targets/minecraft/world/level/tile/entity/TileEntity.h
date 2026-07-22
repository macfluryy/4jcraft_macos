#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "java/Class.h"
#include "java/JavaIntHash.h"

class Level;
class Packet;
class CompoundTag;
class Tile;
class TileEntity;

typedef TileEntity* (*tileEntityCreateFn)();

class TileEntity : public std::enable_shared_from_this<TileEntity> {
public:
    static void staticCtor();
    virtual eINSTANCEOF GetType() { return eTYPE_TILEENTITY; }

private:
    typedef std::unordered_map<std::wstring, tileEntityCreateFn>
        idToCreateMapType;
    typedef std::unordered_map<eINSTANCEOF, std::wstring, eINSTANCEOFKeyHash,
                               eINSTANCEOFKeyEq>
        classToIdMapType;
    static idToCreateMapType idCreateMap;
    static classToIdMapType classIdMap;
    static void setId(tileEntityCreateFn createFn, eINSTANCEOF clas,
                      std::wstring id);
    bool remove;
    unsigned char renderRemoveStage;  

public:
    Level* level;
    int x, y, z;

    
    enum RenderRemoveStage {
        e_RenderRemoveStageKeep,
        e_RenderRemoveStageFlaggedAtChunk,
        e_RenderRemoveStageRemove
    };

    int data;
    Tile* tile;

public:
    
    
    TileEntity();
    virtual ~TileEntity() {}

    void setRenderRemoveStage(unsigned char stage);  
    void upgradeRenderRemoveStage();                 
    bool finalizeRenderRemoveStage();                
    bool shouldRemoveForRender();                    

    virtual Level* getLevel();
    virtual void setLevel(Level* level);
    virtual bool hasLevel();
    virtual void load(CompoundTag* tag);
    virtual void save(CompoundTag* tag);
    virtual void tick();
    static std::shared_ptr<TileEntity> loadStatic(CompoundTag* tag);
    virtual int getData();
    virtual void setData(int data, int updateFlags);
    virtual void setChanged();
    virtual double distanceToSqr(double xPlayer, double yPlayer,
                                 double zPlayer);
    virtual double getViewDistance();
    virtual Tile* getTile();
    virtual std::shared_ptr<Packet> getUpdatePacket();
    virtual bool isRemoved();
    virtual void setRemoved();
    virtual void clearRemoved();
    virtual bool triggerEvent(int b0, int b1);
    virtual void clearCache();

    
    virtual std::shared_ptr<TileEntity> clone() = 0;

protected:
    void clone(std::shared_ptr<TileEntity> tileEntity);
};
