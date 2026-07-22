#include "Painting.h"

#include <memory>
#include <vector>

#include "app/mac/MacGame.h"
#include "java/Random.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/HangingEntity.h"
#include "minecraft/world/entity/player/Abilities.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/item/ItemInstance.h"
#include "nbt/CompoundTag.h"

typedef Painting::Motive _Motive;
const _Motive* Painting::Motive::values[] = {
    new _Motive(L"Kebab", 16, 16, 0 * 16, 0 * 16),
    new _Motive(L"Aztec", 16, 16, 1 * 16, 0 * 16),      
    new _Motive(L"Alban", 16, 16, 2 * 16, 0 * 16),      
    new _Motive(L"Aztec2", 16, 16, 3 * 16, 0 * 16),     
    new _Motive(L"Bomb", 16, 16, 4 * 16, 0 * 16),       
    new _Motive(L"Plant", 16, 16, 5 * 16, 0 * 16),      
    new _Motive(L"Wasteland", 16, 16, 6 * 16, 0 * 16),  

    new _Motive(L"Pool", 32, 16, 0 * 16, 2 * 16),     
    new _Motive(L"Courbet", 32, 16, 2 * 16, 2 * 16),  
    new _Motive(L"Sea", 32, 16, 4 * 16, 2 * 16),      
    new _Motive(L"Sunset", 32, 16, 6 * 16, 2 * 16),   
    new _Motive(L"Creebet", 32, 16, 8 * 16, 2 * 16),  

    new _Motive(L"Wanderer", 16, 32, 0 * 16, 4 * 16),  
    new _Motive(L"Graham", 16, 32, 1 * 16, 4 * 16),    

    new _Motive(L"Match", 32, 32, 0 * 16, 8 * 16),          
    new _Motive(L"Bust", 32, 32, 2 * 16, 8 * 16),           
    new _Motive(L"Stage", 32, 32, 4 * 16, 8 * 16),          
    new _Motive(L"Void", 32, 32, 6 * 16, 8 * 16),           
    new _Motive(L"SkullAndRoses", 32, 32, 8 * 16, 8 * 16),  
    new _Motive(L"Wither", 32, 32, 10 * 16, 8 * 16),
    new _Motive(L"Fighters", 64, 32, 0 * 16, 6 * 16),  

    new _Motive(L"Pointer", 64, 64, 0 * 16, 12 * 16),       
    new _Motive(L"Pigscene", 64, 64, 4 * 16, 12 * 16),      
    new _Motive(L"BurningSkull", 64, 64, 8 * 16, 12 * 16),  

    new _Motive(L"Skeleton", 64, 48, 12 * 16, 4 * 16),    
    new _Motive(L"DonkeyKong", 64, 48, 12 * 16, 7 * 16),  
};




const int Painting::Motive::MAX_MOTIVE_NAME_LENGTH =
    13;  


void Painting::_init(Level* level) { motive = nullptr; };

Painting::Painting(Level* level) : HangingEntity(level) {
    
    
    this->defineSynchedData();

    _init(level);
}

Painting::Painting(Level* level, int xTile, int yTile, int zTile, int dir)
    : HangingEntity(level, xTile, yTile, zTile, dir) {
    _init(level);

    
    
}



void Painting::PaintingPostConstructor(int dir, int motive) {
#ifndef _CONTENT_PACKAGE
    if (app.DebugArtToolsOn() && motive >= 0) {
        this->motive = (Motive*)Motive::values[motive];
        setDir(dir);
    } else
#endif
    {
        std::vector<Motive*>* survivableMotives = new std::vector<Motive*>();
        for (int i = 0; i < LAST_VALUE; i++) {
            this->motive = (Motive*)Motive::values[i];
            setDir(dir);
            if (survives()) {
                survivableMotives->push_back(this->motive);
            }
        }
        if (!survivableMotives->empty()) {
            this->motive = survivableMotives->at(
                random->nextInt((int)survivableMotives->size()));
        }
        setDir(dir);
    }
}

Painting::Painting(Level* level, int x, int y, int z, int dir,
                   std::wstring motiveName)
    : HangingEntity(level, x, y, z, dir) {
    _init(level);

    for (int i = 0; i < LAST_VALUE; i++) {
        if ((Motive::values[i])->name.compare(motiveName) == 0) {
            this->motive = (Motive*)Motive::values[i];
            break;
        }
    }
    setDir(dir);
}

void Painting::addAdditonalSaveData(CompoundTag* tag) {
    
    tag->putString(L"Motive", motive->name);

    HangingEntity::addAdditonalSaveData(tag);
}

void Painting::readAdditionalSaveData(CompoundTag* tag) {
    std::wstring motiveName = tag->getString(L"Motive");
    std::vector<Motive*>::iterator it;
    for (int i = 0; i < LAST_VALUE; i++) {
        if (Motive::values[i]->name.compare(motiveName) == 0) {
            this->motive = (Motive*)Motive::values[i];
        }
    }
    if (this->motive == nullptr) motive = (Motive*)Motive::values[Kebab];

    HangingEntity::readAdditionalSaveData(tag);
}

int Painting::getWidth() { return motive->w; }

int Painting::getHeight() { return motive->h; }

void Painting::dropItem(std::shared_ptr<Entity> causedBy) {
    if ((causedBy != nullptr) && causedBy->instanceof(eTYPE_PLAYER)) {
        std::shared_ptr<Player> player =
            std::dynamic_pointer_cast<Player>(causedBy);
        if (player->abilities.instabuild) {
            return;
        }
    }

    spawnAtLocation(std::make_shared<ItemInstance>(Item::painting), 0.0f);
}