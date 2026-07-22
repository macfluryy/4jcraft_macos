#pragma once

#include <format>
#include <set>
#include <string>
#include <vector>

#include "TextureHolder.h"

class StitchSlot;
class Texture;

class Stitcher {
public:
    static const int STITCH_SUCCESS = 0;
    static const int STITCH_RETRY = 1;
    static const int STITCH_ABORT = 2;

    static const int MAX_MIPLEVEL =
        0;  
    static const int MIN_TEXEL = 1 << MAX_MIPLEVEL;

private:
    std::set<TextureHolder*, TextureHolderLessThan>
        texturesToBeStitched;          
    std::vector<StitchSlot*> storage;  
    int storageX;
    int storageY;

    int maxWidth;
    int maxHeight;
    bool forcePowerOfTwo;
    int forcedScale;

    Texture* stitchedTexture;

    std::wstring name;

    void _init(const std::wstring& name, int maxWidth, int maxHeight,
               bool forcePowerOfTwo, int forcedScale);

public:
    Stitcher(const std::wstring& name, int maxWidth, int maxHeight,
             bool forcePowerOfTwo);
    Stitcher(const std::wstring& name, int maxWidth, int maxHeight,
             bool forcePowerOfTwo, int forcedScale);

    int getWidth();
    int getHeight();
    void addTexture(TextureHolder* textureHolder);
    Texture* constructTexture(bool mipmap = true);  
    void stitch();
    std::vector<StitchSlot*>* gatherAreas();

private:
    
    
    int smallestEncompassingPowerOfTwo(int input);

    bool addToStorage(TextureHolder* textureHolder);

    






    bool expand(TextureHolder* textureHolder);
};