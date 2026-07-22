#include "Textures.h"

#include <assert.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

#include "platform/sdl2/Render.h"
#include "HttpTexture.h"
#include "app/mac/MacGame.h"
#include "app/include/BufferedImage.h"
#include "app/include/MemTexture.h"
#include "app/include/MemTextureProcessor.h"
#include "app/include/MobSkinMemTextureProcessor.h"
#include "util/StringHelpers.h"

#include "java/Buffer.h"
#include "java/ByteBuffer.h"
#include "minecraft/client/MemoryTracker.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/renderer/texture/PreStitchedTextureMap.h"
#include "minecraft/client/renderer/texture/Texture.h"
#include "minecraft/client/renderer/texture/TextureAtlas.h"
#include "minecraft/client/resources/ResourceLocation.h"
#include "minecraft/client/skins/TexturePack.h"
#include "minecraft/client/skins/TexturePackRepository.h"
#include "minecraft/world/Icon.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/item/ItemEntity.h"
#include "minecraft/world/item/ItemInstance.h"







bool Textures::MIPMAP = false;
C4JRender::eTextureFormat Textures::TEXTURE_FORMAT =
    C4JRender::TEXTURE_FORMAT_RxGyBzAw;

int Textures::preLoadedIdx[TN_COUNT];
const wchar_t* Textures::preLoaded[TN_COUNT] = {
    L"%blur%misc/pumpkinblur",
    L"%clamp%misc/shadow",
    L"art/kz",
    L"environment/clouds",
    L"environment/rain",
    L"environment/snow",
    L"gui/gui",
    L"gui/icons",
    L"item/arrows",
    L"item/boat",
    L"item/cart",
    L"item/sign",
    L"misc/mapbg",
    L"misc/mapicons",
    L"misc/water",
    L"misc/footprint",
    L"mob/saddle",
    L"mob/sheep_fur",
    L"mob/spider_eyes",
    L"particles",
    L"mob/chicken",
    L"mob/cow",
    L"mob/pig",
    L"mob/sheep",
    L"mob/squid",
    L"mob/wolf",
    L"mob/wolf_tame",
    L"mob/wolf_angry",
    L"mob/creeper",
    L"mob/ghast",
    L"mob/ghast_fire",
    L"mob/zombie",
    L"mob/pigzombie",
    L"mob/skeleton",
    L"mob/slime",
    L"mob/spider",
    L"mob/char",
    L"mob/char1",
    L"mob/char2",
    L"mob/char3",
    L"mob/char4",
    L"mob/char5",
    L"mob/char6",
    L"mob/char7",
    L"terrain/moon",
    L"terrain/sun",
    L"armor/power",

    
    L"mob/cavespider",
    L"mob/enderman",
    L"mob/silverfish",
    L"mob/enderman_eyes",
    L"misc/explosion",
    L"item/xporb",
    L"item/chest",
    L"item/largechest",

    
    L"item/enderchest",

    
    L"mob/redcow",
    L"mob/snowman",
    L"mob/enderdragon/ender",
    L"mob/fire",
    L"mob/lava",
    L"mob/villager/villager",
    L"mob/villager/farmer",
    L"mob/villager/librarian",
    L"mob/villager/priest",
    L"mob/villager/smith",
    L"mob/villager/butcher",
    L"mob/enderdragon/crystal",
    L"mob/enderdragon/shuffle",
    L"mob/enderdragon/beam",
    L"mob/enderdragon/ender_eyes",
    L"%blur%misc/glint",
    L"item/book",
    L"misc/tunnel",
    L"misc/particlefield",
    L"terrain/moon_phases",

    
    L"mob/ozelot",
    L"mob/cat_black",
    L"mob/cat_red",
    L"mob/cat_siamese",
    L"mob/villager_golem",
    L"mob/skeleton_wither",

    
    L"mob/wolf_collar",
    L"mob/zombie_villager",

    
    L"item/lead_knot",

    L"misc/beacon_beam",

    L"mob/bat",

    L"mob/horse/donkey",
    L"mob/horse/horse_black",
    L"mob/horse/horse_brown",
    L"mob/horse/horse_chestnut",
    L"mob/horse/horse_creamy",
    L"mob/horse/horse_darkbrown",
    L"mob/horse/horse_gray",
    L"mob/horse/horse_markings_blackdots",
    L"mob/horse/horse_markings_white",
    L"mob/horse/horse_markings_whitedots",
    L"mob/horse/horse_markings_whitefield",
    L"mob/horse/horse_skeleton",
    L"mob/horse/horse_white",
    L"mob/horse/horse_zombie",
    L"mob/horse/mule",

    L"mob/horse/armor/horse_armor_diamond",
    L"mob/horse/armor/horse_armor_gold",
    L"mob/horse/armor/horse_armor_iron",

    L"mob/witch",

    L"mob/wither/wither",
    L"mob/wither/wither_armor",
    L"mob/wither/wither_invulnerable",

    L"item/trapped",
    L"item/trapped_double",


#ifdef ENABLE_JAVA_GUIS
    L"%blur%/misc/vignette",
    L"/achievement/bg",
    L"gui/background",
    L"gui/inventory",
    L"gui/container",
    L"gui/crafting",
    L"gui/furnace",
    L"gui/creative_inventory/tabs",
    L"gui/creative_inventory/tab_items",
    L"gui/creative_inventory/tab_inventory",
    L"gui/creative_inventory/tab_item_search",
    L"title/mclogo",
    L"gui/horse",
    L"gui/anvil",
    L"gui/trap",
    L"gui/beacon",
    L"gui/hopper",
    L"gui/enchant",
    L"gui/villager",
    L"gui/brewing_stand",
    L"title/bg/panorama",
    L"title/bg/panorama0",
    L"title/bg/panorama1",
    L"title/bg/panorama2",
    L"title/bg/panorama3",
    L"title/bg/panorama4",
    L"title/bg/panorama5",
#endif



#if defined(_LARGE_WORLDS)
    L"misc/additionalmapicons",
#endif

    L"font/Default",
    L"font/alternate",

    
    












































































    L"gui/items",
    L"terrain",
};

Textures::Textures(TexturePackRepository* skins, Options* options) {
    
    
    missingNo = new BufferedImage(16, 16, BufferedImage::TYPE_INT_ARGB);

    this->skins = skins;
    this->options = options;

    
















    
    terrain = new PreStitchedTextureMap(Icon::TYPE_TERRAIN, L"terrain",
                                        L"textures/blocks/", missingNo, true);
    items = new PreStitchedTextureMap(Icon::TYPE_ITEM, L"items",
                                      L"textures/items/", missingNo, true);

    
    
    loadIndexedTextures();
}

void Textures::loadIndexedTextures() {
    
    
    for (int i = 0; i < TN_COUNT - 2; i++) {
        preLoadedIdx[i] =
            loadTexture((TEXTURE_NAME)i, std::wstring(preLoaded[i]) + L".png");
    }
}

std::vector<int> Textures::loadTexturePixels(TEXTURE_NAME texId,
                                             const std::wstring& resourceName) {
    TexturePack* skin = skins->getSelected();

    {
        std::vector<int> id = pixelsMap[resourceName];
        
        
        if (!id.empty()) return id;
    }

    
    
    std::vector<int> res;
    
    if (false)  
    {
        res = loadTexturePixels(missingNo);
    } else {
        BufferedImage* bufImage = readImage(texId, resourceName);  
        res = loadTexturePixels(bufImage);
        delete bufImage;
    }

    pixelsMap[resourceName] = res;
    return res;
    








}

std::vector<int> Textures::loadTexturePixels(BufferedImage* img) {
    int w = img->getWidth();
    int h = img->getHeight();
    std::vector<int> pixels(w * h);
    return loadTexturePixels(img, pixels);
}

std::vector<int> Textures::loadTexturePixels(BufferedImage* img,
                                             std::vector<int>& pixels) {
    int w = img->getWidth();
    int h = img->getHeight();
    img->getRGB(0, 0, w, h, pixels, 0, w);
    return pixels;
}

int Textures::loadTexture(int idx) {
    if (idx == -1) {
        return 0;
    } else {
        if (idx == TN_TERRAIN) {
            terrain->getStitchedTexture()->bind(0);
            return terrain->getStitchedTexture()->getGlId();
        }
        if (idx == TN_GUI_ITEMS) {
            items->getStitchedTexture()->bind(0);
            return items->getStitchedTexture()->getGlId();
        }
        return preLoadedIdx[idx];
    }
}




void Textures::setTextureFormat(const std::wstring& resourceName) {
    
    {
        TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_RxGyBzAw;
    }
}

void Textures::bindTexture(const std::wstring& resourceName) {
    bind(loadTexture(TN_COUNT, resourceName));
}


void Textures::bindTexture(ResourceLocation* resource) {
    if (resource->isPreloaded()) {
        bind(loadTexture(resource->getTexture()));
    } else {
        bind(loadTexture(TN_COUNT, resource->getPath()));
    }
}


void Textures::bindTextureLayers(ResourceLocation* resource) {
    assert(resource->isPreloaded());

    
    
    
    std::wstring cacheKey = L"%layered%";
    int layers = resource->getTextureCount();
    for (int i = 0; i < layers; i++) {
        cacheKey += std::to_wstring(resource->getTexture(i));
        cacheKey += L"/";
    }

    int id = -1;
    bool inMap = (idMap.find(cacheKey) != idMap.end());
    if (inMap) {
        id = idMap[cacheKey];
    } else {
        
        
        std::vector<int> mergedPixels;
        int mergedWidth = 0;
        int mergedHeight = 0;
        bool hasMergedPixels = false;

        for (int i = 0; i < layers; i++) {
            TEXTURE_NAME textureName = resource->getTexture(i);
            if (textureName == static_cast<_TEXTURE_NAME>(-1)) {
                continue;
            }

            std::wstring resourceName =
                std::wstring(preLoaded[textureName]) + L".png";
            BufferedImage* image = readImage(textureName, resourceName);
            if (image == nullptr) {
                continue;
            }

            int width = image->getWidth();
            int height = image->getHeight();
            std::vector<int> layerPixels = loadTexturePixels(image);
            delete image;

            if (!hasMergedPixels) {
                mergedWidth = width;
                mergedHeight = height;
                mergedPixels = std::vector<int>(width * height);
                memcpy(mergedPixels.data(), layerPixels.data(),
                       width * height * sizeof(int));
                hasMergedPixels = true;
            } else if (width == mergedWidth && height == mergedHeight) {
                for (int p = 0; p < width * height; p++) {
                    int dst = mergedPixels[p];
                    int src = layerPixels[p];

                    float srcAlpha = ((src >> 24) & 0xff) / 255.0f;
                    if (srcAlpha <= 0.0f) {
                        continue;
                    }

                    float dstAlpha = ((dst >> 24) & 0xff) / 255.0f;
                    float outAlpha = srcAlpha + dstAlpha * (1.0f - srcAlpha);
                    if (outAlpha <= 0.0f) {
                        mergedPixels[p] = 0;
                        continue;
                    }

                    float srcFactor = srcAlpha / outAlpha;
                    float dstFactor = (dstAlpha * (1.0f - srcAlpha)) / outAlpha;

                    int outA = static_cast<int>(outAlpha * 255.0f + 0.5f);
                    int outR = static_cast<int>(
                        (((src >> 16) & 0xff) * srcFactor) +
                        (((dst >> 16) & 0xff) * dstFactor) + 0.5f);
                    int outG = static_cast<int>(
                        (((src >> 8) & 0xff) * srcFactor) +
                        (((dst >> 8) & 0xff) * dstFactor) + 0.5f);
                    int outB =
                        static_cast<int>(((src & 0xff) * srcFactor) +
                                         ((dst & 0xff) * dstFactor) + 0.5f);
                    mergedPixels[p] =
                        (outA << 24) | (outR << 16) | (outG << 8) | outB;
                }
            }
        }

        if (hasMergedPixels) {
            BufferedImage* mergedImage = new BufferedImage(
                mergedWidth, mergedHeight, BufferedImage::TYPE_INT_ARGB);
            memcpy(mergedImage->getData(), mergedPixels.data(),
                   mergedWidth * mergedHeight * sizeof(int));
            id = getTexture(mergedImage, C4JRender::TEXTURE_FORMAT_RxGyBzAw,
                            false);
        } else {
            id = 0;
        }

        idMap[cacheKey] = id;
    }

    RenderManager.TextureBind(id);
}

void Textures::bind(int id) {
    
    
    
    
    
    
    
    {
        if (id < 0) return;
        glBindTexture(GL_TEXTURE_2D, id);
        
    }
}

ResourceLocation* Textures::getTextureLocation(std::shared_ptr<Entity> entity) {
    std::shared_ptr<ItemEntity> item =
        std::dynamic_pointer_cast<ItemEntity>(entity);
    int iconType = item->getItem()->getIconType();
    return getTextureLocation(iconType);
}

ResourceLocation* Textures::getTextureLocation(int iconType) {
    switch (iconType) {
        case Icon::TYPE_TERRAIN:
            return &TextureAtlas::LOCATION_BLOCKS;
        case Icon::TYPE_ITEM:
            return &TextureAtlas::LOCATION_ITEMS;
    }

    return &TextureAtlas::LOCATION_ITEMS;
}

void Textures::clearLastBoundId() { lastBoundId = -1; }

int Textures::loadTexture(TEXTURE_NAME texId,
                          const std::wstring& resourceName) {
    
    
    

    
    
    
    
    
    
    
    
    
    

    
    

    {
        bool inMap = (idMap.find(resourceName) != idMap.end());
        int id = idMap[resourceName];
        if (inMap) return id;
    }

    std::wstring pathName = resourceName;

    
    if ((resourceName == L"environment/clouds.png") ||
        (resourceName == L"%clamp%misc/shadow.png") ||
        (resourceName == L"%blur%misc/pumpkinblur.png") ||
        (resourceName == L"%clamp%misc/shadow.png") ||
        (resourceName == L"gui/icons.png") ||
        (resourceName == L"gui/gui.png") ||
        (resourceName == L"misc/footprint.png")) {
        MIPMAP = false;
    }
    setTextureFormat(resourceName);

    
    
    int id = MemoryTracker::genTextures();

    std::wstring prefix = L"%blur%";
    bool blur = resourceName.substr(0, prefix.size()).compare(prefix) ==
                0;  
    if (blur) pathName = resourceName.substr(6);

    prefix = L"%clamp%";
    bool clamp = resourceName.substr(0, prefix.size()).compare(prefix) ==
                 0;  
    if (clamp) pathName = resourceName.substr(7);

    
    if (false)  
    {
        loadTexture(missingNo, id, blur, clamp);
    } else {
        
        BufferedImage* bufImage = readImage(texId, pathName);  
        loadTexture(bufImage, id, blur, clamp);
        delete bufImage;
    }

    idMap[resourceName] = id;
    MIPMAP = true;  
    TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_RxGyBzAw;
    return id;
    









}

int Textures::getTexture(BufferedImage* img, C4JRender::eTextureFormat format,
                         bool mipmap) {
    int id = MemoryTracker::genTextures();
    TEXTURE_FORMAT = format;
    MIPMAP = mipmap;
    loadTexture(img, id);
    TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_RxGyBzAw;
    MIPMAP = true;
    loadedImages[id] = img;
    return id;
}

void Textures::loadTexture(BufferedImage* img, int id) {
    

    loadTexture(img, id, false, false);
}

void Textures::loadTexture(BufferedImage* img, int id, bool blur, bool clamp) {
    
    
    int iMipLevels = 1;
    glBindTexture(GL_TEXTURE_2D, id);

    if (MIPMAP) {
        
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        





    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    if (blur) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    if (clamp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    int w = img->getWidth();
    int h = img->getHeight();

    std::vector<int> rawPixels(w * h);
    img->getRGB(0, 0, w, h, rawPixels, 0, w);

    if (options != nullptr && options->anaglyph3d) {
        rawPixels = anaglyph(rawPixels);
    }

    std::vector<uint8_t> newPixels(w * h * 4);
    for (unsigned int i = 0; i < rawPixels.size(); i++) {
        int a = (rawPixels[i] >> 24) & 0xff;
        int r = (rawPixels[i] >> 16) & 0xff;
        int g = (rawPixels[i] >> 8) & 0xff;
        int b = (rawPixels[i]) & 0xff;

        newPixels[i * 4 + 0] = (uint8_t)r;
        newPixels[i * 4 + 1] = (uint8_t)g;
        newPixels[i * 4 + 2] = (uint8_t)b;
        newPixels[i * 4 + 3] = (uint8_t)a;
    }
    
    ByteBuffer* pixels = MemoryTracker::createByteBuffer(w * h * 4);
    pixels->clear();
    pixels->put(newPixels);
    pixels->position(0)->limit(newPixels.size());

    if (MIPMAP) {
        
        
        int iWidthMips = 1;
        int iHeightMips = 1;
        while ((8 << iWidthMips) < w) iWidthMips++;
        while ((8 << iHeightMips) < h) iHeightMips++;

        iMipLevels = (iWidthMips < iHeightMips) ? iWidthMips : iHeightMips;
        
        if (iMipLevels > 5) iMipLevels = 5;
        RenderManager.TextureSetTextureLevels(iMipLevels);  
    }
    RenderManager.TextureData(w, h, pixels->getBuffer(), 0, TEXTURE_FORMAT);
    
    

    if (MIPMAP) {
        for (int level = 1; level < iMipLevels; level++) {
            int ow = w >> (level - 1);
            

            int ww = w >> level;
            int hh = h >> level;

            
            unsigned int* tempData = new unsigned int[ww * hh];
            
            
            if (img->getData(level)) {
                memcpy(tempData, img->getData(level), ww * hh * 4);
                
                for (int i = 0; i < ww * hh; i++) {
                    tempData[i] = (tempData[i] >> 24) | (tempData[i] << 8);
                }
            } else {
                for (int x = 0; x < ww; x++)
                    for (int y = 0; y < hh; y++) {
                        int c0 = pixels->getInt(
                            ((x * 2 + 0) + (y * 2 + 0) * ow) * 4);
                        int c1 = pixels->getInt(
                            ((x * 2 + 1) + (y * 2 + 0) * ow) * 4);
                        int c2 = pixels->getInt(
                            ((x * 2 + 1) + (y * 2 + 1) * ow) * 4);
                        int c3 = pixels->getInt(
                            ((x * 2 + 0) + (y * 2 + 1) * ow) * 4);
                        
                        
                        
                        c0 =
                            ((c0 >> 8) & 0x00ffffff) | ((unsigned int)c0 << 24);
                        c1 =
                            ((c1 >> 8) & 0x00ffffff) | ((unsigned int)c1 << 24);
                        c2 =
                            ((c2 >> 8) & 0x00ffffff) | ((unsigned int)c2 << 24);
                        c3 =
                            ((c3 >> 8) & 0x00ffffff) | ((unsigned int)c3 << 24);
                        int col =
                            Texture::crispBlend(Texture::crispBlend(c0, c1),
                                                Texture::crispBlend(c2, c3));
                        
                        col = ((unsigned int)col << 8) | ((col >> 24) & 0xff);
                        tempData[x + y * ww] = col;
                    }
            }
            for (int x = 0; x < ww; x++)
                for (int y = 0; y < hh; y++) {
                    pixels->putInt((x + y * ww) * 4, tempData[x + y * ww]);
                }
            delete[] tempData;
            RenderManager.TextureData(ww, hh, pixels->getBuffer(), level,
                                      TEXTURE_FORMAT);
        }
    }

    



    delete pixels;  
}

std::vector<int> Textures::anaglyph(std::vector<int>& rawPixels) {
    std::vector<int> result(rawPixels.size());
    for (unsigned int i = 0; i < rawPixels.size(); i++) {
        int a = (rawPixels[i] >> 24) & 0xff;
        int r = (rawPixels[i] >> 16) & 0xff;
        int g = (rawPixels[i] >> 8) & 0xff;
        int b = (rawPixels[i]) & 0xff;

        int rr = (r * 30 + g * 59 + b * 11) / 100;
        int gg = (r * 30 + g * 70) / (100);
        int bb = (r * 30 + b * 70) / (100);

        result[i] = a << 24 | rr << 16 | gg << 8 | bb;
    }

    return result;
}

void Textures::replaceTexture(std::vector<int>& rawPixels, int w, int h,
                              int id) {
    bind(id);

    
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (options != nullptr && options->anaglyph3d) {
        rawPixels = anaglyph(rawPixels);
    }

    std::vector<uint8_t> newPixels(w * h * 4);
    for (unsigned int i = 0; i < rawPixels.size(); i++) {
        int a = (rawPixels[i] >> 24) & 0xff;
        int r = (rawPixels[i] >> 16) & 0xff;
        int g = (rawPixels[i] >> 8) & 0xff;
        int b = (rawPixels[i]) & 0xff;

        if (options != nullptr && options->anaglyph3d) {
            int rr = (r * 30 + g * 59 + b * 11) / 100;
            int gg = (r * 30 + g * 70) / (100);
            int bb = (r * 30 + b * 70) / (100);

            r = rr;
            g = gg;
            b = bb;
        }

        newPixels[i * 4 + 0] = (uint8_t)r;
        newPixels[i * 4 + 1] = (uint8_t)g;
        newPixels[i * 4 + 2] = (uint8_t)b;
        newPixels[i * 4 + 3] = (uint8_t)a;
    }
    ByteBuffer* pixels = MemoryTracker::createByteBuffer(
        w * h * 4);  
    pixels->put(newPixels);
    pixels->position(0)->limit(newPixels.size());

    
    
    
    RenderManager.TextureDataUpdate(0, 0, w, h, pixels->getBuffer(), 0);
    
    
    
    delete pixels;
}




void Textures::replaceTextureDirect(const std::vector<int>& rawPixels, int w,
                                    int h, int id) {
    glBindTexture(GL_TEXTURE_2D, id);

    
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    RenderManager.TextureDataUpdate(0, 0, w, h,
                                    const_cast<int*>(rawPixels.data()), 0);
}




void Textures::replaceTextureDirect(const std::vector<short>& rawPixels, int w,
                                    int h, int id) {
    glBindTexture(GL_TEXTURE_2D, id);

    
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    RenderManager.TextureDataUpdate(0, 0, w, h,
                                    const_cast<short*>(rawPixels.data()), 0);
}

void Textures::releaseTexture(int id) {
    loadedImages.erase(id);
    glDeleteTextures(id);
}

int Textures::loadHttpTexture(const std::wstring& url,
                              const std::wstring& backup) {
    HttpTexture* texture = httpTextures[url];
    if (texture != nullptr) {
        if (texture->loadedImage != nullptr && !texture->isLoaded) {
            if (texture->id < 0) {
                texture->id = getTexture(texture->loadedImage);
            } else {
                loadTexture(texture->loadedImage, texture->id);
            }
            texture->isLoaded = true;
        }
    }
    if (texture == nullptr || texture->id < 0) {
        if (backup.empty()) return -1;
        return loadTexture(TN_COUNT, backup);
    }
    return texture->id;
}

int Textures::loadHttpTexture(const std::wstring& url, int backup) {
    HttpTexture* texture = httpTextures[url];
    if (texture != nullptr) {
        if (texture->loadedImage != nullptr && !texture->isLoaded) {
            if (texture->id < 0) {
                texture->id = getTexture(texture->loadedImage);
            } else {
                loadTexture(texture->loadedImage, texture->id);
            }
            texture->isLoaded = true;
        }
    }
    if (texture == nullptr || texture->id < 0) {
        return loadTexture(backup);
    }
    return texture->id;
}

bool Textures::hasHttpTexture(const std::wstring& url) {
    return httpTextures.find(url) != httpTextures.end();
}

HttpTexture* Textures::addHttpTexture(const std::wstring& url,
                                      HttpTextureProcessor* processor) {
    HttpTexture* texture = httpTextures[url];
    if (texture == nullptr) {
        httpTextures[url] = new HttpTexture(url, processor);
    } else {
        texture->count++;
    }
    return texture;
}

void Textures::removeHttpTexture(const std::wstring& url) {
    HttpTexture* texture = httpTextures[url];
    if (texture != nullptr) {
        texture->count--;
        if (texture->count == 0) {
            if (texture->id >= 0) releaseTexture(texture->id);
            httpTextures.erase(url);
        }
    }
}


int Textures::loadMemTexture(const std::wstring& url,
                             const std::wstring& backup) {
    MemTexture* texture = nullptr;
    auto it = memTextures.find(url);
    if (it != memTextures.end()) {
        texture = (*it).second;
    }
    if (texture == nullptr && app.IsFileInMemoryTextures(url)) {
        
        texture = addMemTexture(url, new MobSkinMemTextureProcessor());
    }
    if (texture != nullptr) {
        if (texture->loadedImage != nullptr && !texture->isLoaded) {
            
            
            if ((url.substr(0, 7) == L"dlcskin") ||
                (url.substr(0, 7) == L"dlccape")) {
                MIPMAP = false;
            }

            if (texture->id < 0) {
                texture->id =
                    getTexture(texture->loadedImage,
                               C4JRender::TEXTURE_FORMAT_RxGyBzAw, MIPMAP);
            } else {
                loadTexture(texture->loadedImage, texture->id);
            }
            texture->isLoaded = true;
            MIPMAP = true;
        }
    }
    if (texture == nullptr || texture->id < 0) {
        if (backup.empty()) return -1;
        return loadTexture(TN_COUNT, backup);
    }
    return texture->id;
}

int Textures::loadMemTexture(const std::wstring& url, int backup) {
    MemTexture* texture = nullptr;
    auto it = memTextures.find(url);
    if (it != memTextures.end()) {
        texture = (*it).second;
    }
    if (texture == nullptr && app.IsFileInMemoryTextures(url)) {
        
        texture = addMemTexture(url, new MobSkinMemTextureProcessor());
    }
    if (texture != nullptr) {
        texture->ticksSinceLastUse = 0;
        if (texture->loadedImage != nullptr && !texture->isLoaded) {
            
            
            if ((url.substr(0, 7) == L"dlcskin") ||
                (url.substr(0, 7) == L"dlccape")) {
                MIPMAP = false;
            }
            if (texture->id < 0) {
                texture->id =
                    getTexture(texture->loadedImage,
                               C4JRender::TEXTURE_FORMAT_RxGyBzAw, MIPMAP);
            } else {
                loadTexture(texture->loadedImage, texture->id);
            }
            texture->isLoaded = true;
            MIPMAP = true;
        }
    }
    if (texture == nullptr || texture->id < 0) {
        return loadTexture(backup);
    }
    return texture->id;
}

MemTexture* Textures::addMemTexture(const std::wstring& name,
                                    MemTextureProcessor* processor) {
    MemTexture* texture = nullptr;
    auto it = memTextures.find(name);
    if (it != memTextures.end()) {
        texture = (*it).second;
    }
    if (texture == nullptr) {
        
        std::uint8_t* pbData = nullptr;
        unsigned int dwBytes = 0;
        app.GetMemFileDetails(name, &pbData, &dwBytes);

        if (dwBytes != 0) {
            texture = new MemTexture(name, pbData, dwBytes, processor);
            memTextures[name] = texture;
        } else {
            
            
            memTextures[name] = nullptr;
        }
    } else {
        texture->count++;
    }

    delete processor;

    return texture;
}












void Textures::removeMemTexture(const std::wstring& url) {
    MemTexture* texture = nullptr;
    auto it = memTextures.find(url);
    if (it != memTextures.end()) {
        texture = (*it).second;

        
        if (texture == nullptr) memTextures.erase(url);
    }
    if (texture != nullptr) {
        texture->count--;
        if (texture->count == 0) {
            if (texture->id >= 0) releaseTexture(texture->id);
            memTextures.erase(url);
            delete texture;
        }
    }
}

void Textures::tick(
    bool updateTextures,
    bool tickDynamics)  
{
    if (tickDynamics) {
        
        
        
        
        
        if (!updateTextures) {
            return;
        }

        
        
        
        
        terrain->cycleAnimationFrames();
        items->cycleAnimationFrames();
        
        
    }

    
    
    
    for (auto it = memTextures.begin(); it != memTextures.end();) {
        MemTexture* tex = it->second;

        if (tex &&
            (++tex->ticksSinceLastUse > MemTexture::UNUSED_TICKS_TO_FREE)) {
            if (tex->id >= 0) releaseTexture(tex->id);
            delete tex;
            it = memTextures.erase(it);
        } else {
            it++;
        }
    }
}

void Textures::reloadAll() {
    TexturePack* skin = skins->getSelected();

    for (int i = 0; i < TN_COUNT - 2; i++) {
        releaseTexture(preLoadedIdx[i]);
    }

    idMap.clear();
    loadedImages.clear();

    loadIndexedTextures();

    pixelsMap.clear();
    
    
    
    

    stitch();

    skins->clearInvalidTexturePacks();

    
    
    
}

void Textures::stitch() {
    terrain->stitch();
    items->stitch();
}

Icon* Textures::getMissingIcon(int type) {
    switch (type) {
        case Icon::TYPE_ITEM:
        default:
            return items->getMissingIcon();
        case Icon::TYPE_TERRAIN:
            return terrain->getMissingIcon();
    }
}

BufferedImage* Textures::readImage(
    TEXTURE_NAME texId, const std::wstring& name)  
{
    BufferedImage* img = nullptr;
    
    bool isTu = IsTUImage(texId, name);
    std::wstring drive = L"";

    if (!skins->isUsingDefaultSkin() &&
        skins->getSelected()->hasFile(L"res/" + name, false)) {
        drive = skins->getSelected()->getPath(isTu);
        img = skins->getSelected()->getImageResource(
            name, false, isTu,
            drive);  
    } else {
        {
            drive = skins->getDefault()->getPath(isTu);
        }

        if (IsOriginalImage(texId, name) || isTu) {
            img = skins->getDefault()->getImageResource(
                name, false, isTu,
                drive);  
        } else {
            img = skins->getDefault()->getImageResource(
                L"1_2_2/" + name, false, isTu,
                drive);  
                         
        }
    }

    return img;
}


TEXTURE_NAME TUImages[] = {
    TN_POWERED_CREEPER, TN_MOB_ENDERMAN_EYES, TN_MISC_EXPLOSION, TN_MOB_ZOMBIE,
    TN_MISC_FOOTSTEP, TN_MOB_RED_COW, TN_MOB_SNOWMAN, TN_MOB_ENDERDRAGON,
    TN_MOB_VILLAGER_VILLAGER, TN_MOB_VILLAGER_FARMER, TN_MOB_VILLAGER_LIBRARIAN,
    TN_MOB_VILLAGER_PRIEST, TN_MOB_VILLAGER_SMITH, TN_MOB_VILLAGER_BUTCHER,
    TN_MOB_ENDERDRAGON_ENDEREYES, TN__BLUR__MISC_GLINT, TN_ITEM_BOOK,
    TN_MISC_PARTICLEFIELD,

    
    TN_MISC_TUNNEL, TN_MOB_ENDERDRAGON_BEAM, TN_GUI_ITEMS, TN_TERRAIN,
    TN_MISC_MAPICONS,

    
    TN_MOB_WITHER_SKELETON,

    
    TN_TILE_ENDER_CHEST, TN_ART_KZ, TN_MOB_WOLF_TAME, TN_MOB_WOLF_COLLAR,
    TN_PARTICLES, TN_MOB_ZOMBIE_VILLAGER,

    TN_ITEM_LEASHKNOT,

    TN_MISC_BEACON_BEAM,

    TN_MOB_BAT,

    TN_MOB_DONKEY, TN_MOB_HORSE_BLACK, TN_MOB_HORSE_BROWN,
    TN_MOB_HORSE_CHESTNUT, TN_MOB_HORSE_CREAMY, TN_MOB_HORSE_DARKBROWN,
    TN_MOB_HORSE_GRAY, TN_MOB_HORSE_MARKINGS_BLACKDOTS,
    TN_MOB_HORSE_MARKINGS_WHITE, TN_MOB_HORSE_MARKINGS_WHITEDOTS,
    TN_MOB_HORSE_MARKINGS_WHITEFIELD, TN_MOB_HORSE_SKELETON, TN_MOB_HORSE_WHITE,
    TN_MOB_HORSE_ZOMBIE, TN_MOB_MULE, TN_MOB_HORSE_ARMOR_DIAMOND,
    TN_MOB_HORSE_ARMOR_GOLD, TN_MOB_HORSE_ARMOR_IRON,

    TN_MOB_WITCH,

    TN_MOB_WITHER, TN_MOB_WITHER_ARMOR, TN_MOB_WITHER_INVULNERABLE,

    TN_TILE_TRAP_CHEST, TN_TILE_LARGE_TRAP_CHEST,



#if defined(_LARGE_WORLDS)
    TN_MISC_ADDITIONALMAPICONS,
#endif

    
    TN_DEFAULT_FONT,
    

    TN_COUNT  
};


const wchar_t* const TUImagePaths[] = {
    L"font/Default", L"font/Mojangles_7", L"font/Mojangles_11",

    
    L"armor/cloth_1.png", L"armor/cloth_1_b.png", L"armor/cloth_2.png",
    L"armor/cloth_2_b.png",

    

    nullptr};

bool Textures::IsTUImage(TEXTURE_NAME texId, const std::wstring& name) {
    int i = 0;
    if (texId < TN_COUNT) {
        while (TUImages[i] < TN_COUNT) {
            if (texId == TUImages[i]) {
                return true;
            }
            i++;
        }
    }
    i = 0;
    while (TUImagePaths[i]) {
        if (name.compare(TUImagePaths[i]) == 0) {
            return true;
        }
        i++;
    }
    return false;
}

TEXTURE_NAME OriginalImages[] = {TN_MOB_CHAR,   TN_MOB_CHAR1, TN_MOB_CHAR2,
                                 TN_MOB_CHAR3,  TN_MOB_CHAR4, TN_MOB_CHAR5,
                                 TN_MOB_CHAR6,  TN_MOB_CHAR7,

                                 TN_MISC_MAPBG,

                                 TN_COUNT};

const wchar_t* const OriginalImagesPaths[] = {L"misc/watercolor.png",

                                              nullptr};

bool Textures::IsOriginalImage(TEXTURE_NAME texId, const std::wstring& name) {
    int i = 0;
    if (texId < TN_COUNT) {
        while (OriginalImages[i] < TN_COUNT) {
            if (texId == OriginalImages[i]) {
                return true;
            }
            i++;
        }
    }
    i = 0;
    while (OriginalImagesPaths[i]) {
        if (name.compare(OriginalImagesPaths[i]) == 0) {
            return true;
        }
        i++;
    }
    return false;
}