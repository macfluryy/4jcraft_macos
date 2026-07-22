#include "TextureMap.h"

#include <wchar.h>

#include <format>
#include <utility>

#include "app/mac/MacGame.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "app/include/BufferedImage.h"
#include "StitchSlot.h"
#include "StitchedTexture.h"
#include "Stitcher.h"
#include "Texture.h"
#include "TextureHolder.h"
#include "TextureManager.h"
#include "java/InputOutputStream/BufferedReader.h"
#include "java/InputOutputStream/InputStream.h"
#include "java/InputOutputStream/InputStreamReader.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/renderer/LevelRenderer.h"
#include "minecraft/client/renderer/entity/EntityRenderDispatcher.h"
#include "minecraft/client/skins/TexturePack.h"
#include "minecraft/client/skins/TexturePackRepository.h"
#include "minecraft/world/Icon.h"
#include "minecraft/world/item/Item.h"
#include "minecraft/world/level/tile/Tile.h"

const std::wstring TextureMap::NAME_MISSING_TEXTURE = L"missingno";

TextureMap::TextureMap(int type, const std::wstring& name,
                       const std::wstring& path, BufferedImage* missingTexture,
                       bool mipmap)
    : iconType(type), name(name), path(path), extension(L".png") {
    this->missingTexture = missingTexture;

    
    missingPosition = nullptr;
    stitchResult = nullptr;

    m_mipMap = mipmap;
}

void TextureMap::stitch() {
    texturesToRegister.clear();

    if (iconType == Icon::TYPE_TERRAIN) {
        
        for (unsigned int i = 0; i < Tile::TILE_NUM_COUNT; ++i) {
            if (Tile::tiles[i] != nullptr) {
                Tile::tiles[i]->registerIcons(this);
            }
        }

        Minecraft::GetInstance()->levelRenderer->registerTextures(this);
        EntityRenderDispatcher::instance->registerTerrainTextures(this);
    }

    
    for (unsigned int i = 0; i < Item::ITEM_NUM_COUNT; ++i) {
        Item* item = Item::items[i];
        if (item != nullptr && item->getIconType() == iconType) {
            item->registerIcons(this);
        }
    }

    
    std::unordered_map<TextureHolder*, std::vector<Texture*>*>
        textures;  

    Stitcher* stitcher = TextureManager::getInstance()->createStitcher(name);

    for (auto it = texturesByName.begin(); it != texturesByName.end(); ++it) {
        delete it->second;
    }
    texturesByName.clear();
    animatedTextures.clear();

    
    
    Texture* missingTex = TextureManager::getInstance()->createTexture(
        NAME_MISSING_TEXTURE, Texture::TM_CONTAINER, missingTexture->getWidth(),
        missingTexture->getHeight(), Texture::WM_CLAMP, Texture::TFMT_RGBA,
        Texture::TFLT_NEAREST, Texture::TFLT_NEAREST, m_mipMap, missingTexture);
    TextureHolder* missingHolder = new TextureHolder(missingTex);

    stitcher->addTexture(missingHolder);
    std::vector<Texture*>* missingVec = new std::vector<Texture*>();
    missingVec->push_back(missingTex);
    textures.insert(
        std::unordered_map<TextureHolder*, std::vector<Texture*>*>::value_type(
            missingHolder, missingVec));

    
    
    for (auto it = texturesToRegister.begin(); it != texturesToRegister.end();
         ++it) {
        std::wstring name = it->first;

        std::wstring filename = path + name + extension;

        
        
        std::vector<Texture*>* frames =
            TextureManager::getInstance()->createTextures(filename, m_mipMap);

        if (frames == nullptr || frames->empty()) {
            continue;  
        }

        TextureHolder* holder = new TextureHolder(frames->at(0));
        stitcher->addTexture(holder);

        
        textures.insert(
            std::unordered_map<TextureHolder*,
                               std::vector<Texture*>*>::value_type(holder,
                                                                   frames));
    }

    
    
    stitcher->stitch();
    
    
    
    

    
    stitchResult = stitcher->constructTexture(m_mipMap);

    
    auto areas = stitcher->gatherAreas();
    
    for (auto it = areas->begin(); it != areas->end(); ++it) {
        StitchSlot* slot = *it;
        TextureHolder* textureHolder = slot->getHolder();

        Texture* texture = textureHolder->getTexture();
        std::wstring textureName = texture->getName();

        std::vector<Texture*>* frames = textures.find(textureHolder)->second;

        StitchedTexture* stored = nullptr;

        auto itTex = texturesToRegister.find(textureName);
        if (itTex != texturesToRegister.end()) stored = itTex->second;

        
        
        bool missing = false;
        if (stored == nullptr) {
            missing = true;
            stored = StitchedTexture::create(textureName);

            if (textureName.compare(NAME_MISSING_TEXTURE) != 0) {
                
                
#ifndef _CONTENT_PACKAGE
                wprintf(L"Couldn't find premade icon for %ls doing %ls\n",
                        textureName.c_str(), name.c_str());
#endif
            }
        }

        stored->init(stitchResult, frames, slot->getX(), slot->getY(),
                     textureHolder->getTexture()->getWidth(),
                     textureHolder->getTexture()->getHeight(),
                     textureHolder->isRotated());

        texturesByName.insert(
            stringStitchedTextureMap::value_type(textureName, stored));
        if (!missing) texturesToRegister.erase(textureName);

        if (frames->size() > 1) {
            animatedTextures.push_back(stored);

            std::wstring animationDefinitionFile = textureName + L".txt";

            TexturePack* texturePack =
                Minecraft::GetInstance()->skins->getSelected();
            bool requiresFallback =
                !texturePack->hasFile(L"\\" + textureName + L".png", false);
            
            InputStream* fileStream = texturePack->getResource(
                L"\\" + path + animationDefinitionFile, requiresFallback);

            
            
#ifndef _CONTENT_PACKAGE
            wprintf(L"Found animation info for: %ls\n",
                    animationDefinitionFile.c_str());
#endif
            InputStreamReader isr(fileStream);
            BufferedReader br(&isr);
            stored->loadAnimationFrames(&br);
            delete fileStream;
            
            
        }
    }
    delete areas;

    missingPosition = texturesByName.find(NAME_MISSING_TEXTURE)->second;

    
    for (auto it = texturesToRegister.begin(); it != texturesToRegister.end();
         ++it) {
        StitchedTexture* texture = it->second;
        texture->replaceWith(missingPosition);
    }

    stitchResult->writeAsPNG(L"debug.stitched_" + name + L".png");
    stitchResult->updateOnGPU();
}

StitchedTexture* TextureMap::getTexture(const std::wstring& name) {
    StitchedTexture* result = texturesByName.find(name)->second;
    if (result == nullptr) result = missingPosition;
    return result;
}

void TextureMap::cycleAnimationFrames() {
    
    for (auto it = animatedTextures.begin(); it != animatedTextures.end();
         ++it) {
        StitchedTexture* texture = *it;
        texture->cycleFrames();
    }
}

Texture* TextureMap::getStitchedTexture() { return stitchResult; }


Icon* TextureMap::registerIcon(const std::wstring& name) {
    if (name.empty()) {
        app.DebugPrintf("Don't register nullptr\n");
#ifndef _CONTENT_PACKAGE
        __debugbreak();
#endif
        
    }

    
    StitchedTexture* result = nullptr;
    auto it = texturesToRegister.find(name);
    if (it != texturesToRegister.end()) result = it->second;

    if (result == nullptr) {
        result = StitchedTexture::create(name);
        texturesToRegister.insert(
            stringStitchedTextureMap::value_type(name, result));
    }

    return result;
}

int TextureMap::getIconType() { return iconType; }

Icon* TextureMap::getMissingIcon() { return missingPosition; }