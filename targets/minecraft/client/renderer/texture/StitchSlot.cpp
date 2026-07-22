#include "StitchSlot.h"

#include <algorithm>
#include <vector>

#include "TextureHolder.h"
#include "util/StringHelpers.h"

StitchSlot::StitchSlot(int originX, int originY, int width, int height)
    : originX(originX), originY(originY), width(width), height(height) {
    subSlots = nullptr;
    textureHolder = nullptr;
}

TextureHolder* StitchSlot::getHolder() { return textureHolder; }

int StitchSlot::getX() { return originX; }

int StitchSlot::getY() { return originY; }

bool StitchSlot::add(TextureHolder* textureHolder) {
    
    if (this->textureHolder != nullptr) {
        return false;
    }

    int textureWidth = textureHolder->getWidth();
    int textureHeight = textureHolder->getHeight();

    
    if (textureWidth > width || textureHeight > height) {
        return false;
    }

    
    if (textureWidth == width && textureHeight == height &&
        subSlots == nullptr) {
        
        this->textureHolder = textureHolder;
        return true;
    }

    
    if (subSlots == nullptr) {
        subSlots = new std::vector<StitchSlot*>();

        
        subSlots->push_back(
            new StitchSlot(originX, originY, textureWidth, textureHeight));

        int spareWidth = width - textureWidth;
        int spareHeight = height - textureHeight;

        if (spareHeight > 0 && spareWidth > 0) {
            
            
            
            
            
            
            
            
            
            
            
            
            

            
            
            int right = std::max(height, spareWidth);
            int bottom = std::max(width, spareHeight);
            if (right >= bottom) {
                subSlots->push_back(new StitchSlot(originX,
                                                   originY + textureHeight,
                                                   textureWidth, spareHeight));
                subSlots->push_back(new StitchSlot(
                    originX + textureWidth, originY, spareWidth, height));
            } else {
                subSlots->push_back(new StitchSlot(originX + textureWidth,
                                                   originY, spareWidth,
                                                   textureHeight));
                subSlots->push_back(new StitchSlot(
                    originX, originY + textureHeight, width, spareHeight));
            }

        } else if (spareWidth == 0) {
            
            
            
            
            
            
            
            
            
            subSlots->push_back(new StitchSlot(originX, originY + textureHeight,
                                               textureWidth, spareHeight));
        } else if (spareHeight == 0) {
            
            
            
            
            
            
            
            
            
            
            subSlots->push_back(new StitchSlot(originX + textureWidth, originY,
                                               spareWidth, textureHeight));
        }
    }

    
    for (auto it = subSlots->begin(); it != subSlots->end(); ++it) {
        StitchSlot* subSlot = *it;
        if (subSlot->add(textureHolder)) {
            return true;
        }
    }

    return false;
}

void StitchSlot::collectAssignments(std::vector<StitchSlot*>* result) {
    if (textureHolder != nullptr) {
        result->push_back(this);
    } else if (subSlots != nullptr) {
        
        for (auto it = subSlots->begin(); it != subSlots->end(); ++it) {
            StitchSlot* subSlot = *it;
            subSlot->collectAssignments(result);
        }
    }
}


std::wstring StitchSlot::toString() {
    return L"Slot{originX=" + toWString(originX) + L", originY=" +
           toWString(originY) + L", width=" + toWString(width) + L", height=" +
           toWString(height) + L", texture=" + toWString(textureHolder) +
           L", subSlots=" + toWString(subSlots) + L'}';
}