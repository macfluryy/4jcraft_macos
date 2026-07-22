#pragma once

#include <string>

#include "Entity.h"
#include "HangingEntity.h"
#include "java/Class.h"

class Level;
class CompoundTag;
class DamageSource;
class Entity;

class Painting : public HangingEntity {
public:
    eINSTANCEOF GetType() { return eTYPE_PAINTING; }
    static Entity* create(Level* level) { return new Painting(level); }

private:
    

public:
    enum MotiveEnum {
        Kebab = 0,  
        Aztec,      
        Alban,      
        Aztec2,     
        Bomb,       
        Plant,      
        Wasteland,  

        Pool,     
        Courbet,  
        Sea,      
        Sunset,   
        Creebet,  

        Wanderer,  
        Graham,    

        Match,          
        Bust,           
        Stage,          
        Void,           
        SkullAndRoses,  
        Wither,
        Fighters,  

        Pointer,       
        Pigscene,      
        BurningSkull,  

        Skeleton,    
        DonkeyKong,  

        LAST_VALUE
    };

    
    class Motive {
    public:
        static const Motive* values[];

        static const int MAX_MOTIVE_NAME_LENGTH;

        const std::wstring name;
        const int w, h;
        const int uo, vo;

        
        Motive(std::wstring name, int w, int h, int uo, int vo)
            : name(name), w(w), h(h), uo(uo), vo(vo) {};
    };

public:
    Motive* motive;

private:
    
    void _init(Level* level);

public:
    Painting(Level* level);
    Painting(Level* level, int xTile, int yTile, int zTile, int dir);
    Painting(Level* level, int x, int y, int z, int dir,
             std::wstring motiveName);

    
    
    
    void PaintingPostConstructor(int dir, int motive = -1);

protected:
    

public:
    

private:
    

public:
    
    
    
    
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    

    
    
    

    virtual int getWidth();
    virtual int getHeight();
    virtual void dropItem(std::shared_ptr<Entity> causedBy);
};
