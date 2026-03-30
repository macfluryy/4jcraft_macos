#pragma once

#include "Minecraft.World/net/minecraft/world/IconRegister.h"

class Icon;
class StitchedTexture;
class Texture;
class BufferedImage;

// 4J Added this class to stop having to do texture stitching at runtime
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
    stringIconMap texturesByName;  //  = new HashMap<String, StitchedTexture>();
    BufferedImage* missingTexture;  // = new BufferedImage(64, 64,
                                    // BufferedImage.TYPE_INT_ARGB);
    StitchedTexture* missingPosition;
    Texture* stitchResult;
    std::vector<StitchedTexture*>
        animatedTextures;  // = new ArrayList<StitchedTexture>();

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

    // 4J Stu - register is a reserved keyword in C++
    Icon* registerIcon(const std::wstring& name);

    int getIconType();
    Icon* getMissingIcon();

    int getFlags() const;
};