#include "Stitcher.h"

#include <algorithm>

#include "app/mac/MacGame.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "StitchSlot.h"
#include "Texture.h"
#include "TextureHolder.h"
#include "TextureManager.h"

void Stitcher::_init(const std::wstring& name, int maxWidth, int maxHeight,
                     bool forcePowerOfTwo, int forcedScale) {
    this->name = name;
    this->maxWidth = maxWidth;
    this->maxHeight = maxHeight;
    this->forcePowerOfTwo = forcePowerOfTwo;
    this->forcedScale = forcedScale;

    
    storageX = 0;
    storageY = 0;
    stitchedTexture = nullptr;
}

Stitcher::Stitcher(const std::wstring& name, int maxWidth, int maxHeight,
                   bool forcePowerOfTwo) {
    _init(name, maxWidth, maxHeight, forcePowerOfTwo, 0);
}

Stitcher::Stitcher(const std::wstring& name, int maxWidth, int maxHeight,
                   bool forcePowerOfTwo, int forcedScale) {
    _init(name, maxWidth, maxHeight, forcePowerOfTwo, forcedScale);
}

int Stitcher::getWidth() { return storageX; }

int Stitcher::getHeight() { return storageY; }

void Stitcher::addTexture(TextureHolder* textureHolder) {
    if (forcedScale > 0) {
        textureHolder->setForcedScale(forcedScale);
    }
    texturesToBeStitched.insert(textureHolder);
}

Texture* Stitcher::constructTexture(bool mipmap) {
    if (forcePowerOfTwo) {
        storageX = smallestEncompassingPowerOfTwo(storageX);
        storageY = smallestEncompassingPowerOfTwo(storageY);
    }

    stitchedTexture = TextureManager::getInstance()->createTexture(
        name, Texture::TM_DYNAMIC, storageX, storageY, Texture::TFMT_RGBA,
        mipmap);
    stitchedTexture->fill(stitchedTexture->getRect(), 0xffff0000);

    std::vector<StitchSlot*>* slots = gatherAreas();
    for (int index = 0; index < slots->size(); index++) {
        StitchSlot* slot = slots->at(index);
        TextureHolder* textureHolder = slot->getHolder();
        stitchedTexture->blit(slot->getX(), slot->getY(),
                              textureHolder->getTexture(),
                              textureHolder->isRotated());
    }
    delete slots;
    TextureManager::getInstance()->registerName(name, stitchedTexture);

    return stitchedTexture;
}

void Stitcher::stitch() {
    
    

    stitchedTexture = nullptr;

    
    for (auto it = texturesToBeStitched.begin();
         it != texturesToBeStitched.end(); ++it) {
        TextureHolder* textureHolder = *it;  

        if (!addToStorage(textureHolder)) {
            app.DebugPrintf("Stitcher exception!\n");
#ifndef _CONTENT_PACKAGE
            __debugbreak();
#endif
            
        }
    }
}

std::vector<StitchSlot*>* Stitcher::gatherAreas() {
    std::vector<StitchSlot*>* result = new std::vector<StitchSlot*>();

    
    for (auto it = storage.begin(); it != storage.end(); ++it) {
        StitchSlot* slot = *it;
        slot->collectAssignments(result);
    }

    return result;
}


int Stitcher::smallestEncompassingPowerOfTwo(int input) {
    int result = input - 1;
    result |= result >> 1;
    result |= result >> 2;
    result |= result >> 4;
    result |= result >> 8;
    result |= result >> 16;
    return result + 1;
}

bool Stitcher::addToStorage(TextureHolder* textureHolder) {
    for (int i = 0; i < storage.size(); i++) {
        if (storage.at(i)->add(textureHolder)) {
            return true;
        }

        
        textureHolder->rotate();
        if (storage.at(i)->add(textureHolder)) {
            return true;
        }

        
        textureHolder->rotate();
    }

    return expand(textureHolder);
}








bool Stitcher::expand(TextureHolder* textureHolder) {
    int minDistance =
        std::min(textureHolder->getHeight(), textureHolder->getWidth());
    bool firstAddition = storageX == 0 && storageY == 0;

    
    bool growOnX;
    if (forcePowerOfTwo) {
        int xCurrentSize = smallestEncompassingPowerOfTwo(storageX);
        int yCurrentSize = smallestEncompassingPowerOfTwo(storageY);
        int xNewSize = smallestEncompassingPowerOfTwo(storageX + minDistance);
        int yNewSize = smallestEncompassingPowerOfTwo(storageY + minDistance);

        bool xCanGrow = xNewSize <= maxWidth;
        bool yCanGrow = yNewSize <= maxHeight;

        if (!xCanGrow && !yCanGrow) {
            return false;
        }

        
        int maxDistance =
            std::max(textureHolder->getHeight(), textureHolder->getWidth());
        
        if (firstAddition && !xCanGrow &&
            !(smallestEncompassingPowerOfTwo(storageY + maxDistance) <=
              maxHeight)) {
            return false;
        }

        bool xWillGrow = xCurrentSize != xNewSize;
        bool yWillGrow = yCurrentSize != yNewSize;

        if (xWillGrow ^ yWillGrow) {
            
            
            

            growOnX = xWillGrow && xCanGrow;
        } else {
            
            growOnX = xCanGrow && xCurrentSize <= yCurrentSize;
        }
    } else {
        
        bool xCanGrow = (storageX + minDistance) <= maxWidth;
        bool yCanGrow = (storageY + minDistance) <= maxHeight;

        if (!xCanGrow && !yCanGrow) {
            return false;
        }

        
        
        growOnX = (firstAddition || storageX <= storageY) && xCanGrow;
    }

    StitchSlot* slot;
    if (growOnX) {
        if (textureHolder->getWidth() > textureHolder->getHeight()) {
            textureHolder->rotate();
        }

        
        if (storageY == 0) {
            storageY = textureHolder->getHeight();
        }

        int newSlotWidth = textureHolder->getWidth();
        
        
        if (forcePowerOfTwo) {
            newSlotWidth =
                smallestEncompassingPowerOfTwo(storageX + newSlotWidth) -
                storageX;
        }
        slot = new StitchSlot(storageX, 0, newSlotWidth, storageY);
        
        storageX += newSlotWidth;
    } else {
        int newSlotHeight = textureHolder->getHeight();
        
        
        if (forcePowerOfTwo) {
            newSlotHeight =
                smallestEncompassingPowerOfTwo(storageY + newSlotHeight) -
                storageY;
        }

        
        slot = new StitchSlot(0, storageY, storageX, newSlotHeight);
        
        storageY += newSlotHeight;
    }

    slot->add(textureHolder);
    storage.push_back(slot);

    return true;
}