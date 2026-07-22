#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "minecraft/world/IconRegister.h"

class Icon;
class StitchedTexture;
class Texture;
class BufferedImage;
class TexturePack;


class PreStitchedTextureMap : public IconRegister {
public:
    static const std::wstring NAME_MISSING_TEXTURE;

private:
    const int iconType;

    const std::wstring name;
    const std::wstring path;
    const std::wstring extension;

    bool m_mipMap;

    typedef std::unordered_map<std::wstring, Icon*> stringIconMap;
    stringIconMap texturesByName;  
    BufferedImage* missingTexture;  
                                    
    StitchedTexture* missingPosition;
    Texture* stitchResult;
    std::vector<StitchedTexture*>
        animatedTextures;  

    void loadUVs();

public:
    PreStitchedTextureMap(int type, const std::wstring& name,
                          const std::wstring& path,
                          BufferedImage* missingTexture, bool mipMap = false);

    void stitch();

private:
    void makeTextureAnimated(TexturePack* texturePack, StitchedTexture* tex);

public:
    StitchedTexture* getTexture(const std::wstring& name);
    void cycleAnimationFrames();
    Texture* getStitchedTexture();

    
    Icon* registerIcon(const std::wstring& name);

    int getIconType();
    Icon* getMissingIcon();

    int getFlags() const;
};