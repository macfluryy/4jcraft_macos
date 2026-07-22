#include "Minimap.h"

#include <GL/gl.h>
#include <math.h>
#include <string.h>
#include <wchar.h>

#include <string>

#include "platform/sdl2/Render.h"
#include "Font.h"
#include "app/common/App_enums.h"
#include "app/common/src/Colours/ColourTable.h"
#include "app/include/BufferedImage.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/renderer/Tesselator.h"
#include "minecraft/client/renderer/Textures.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/level/material/MaterialColor.h"
#include "minecraft/world/level/saveddata/MapItemSavedData.h"

int Minimap::LUT[256];        
bool Minimap::genLUT = true;  

Minimap::Minimap(Font* font, Options* options, Textures* textures,
                 bool optimised) {
    this->pixels = std::vector<int>(w * h);
    this->options = options;
    this->font = font;
    BufferedImage* img = new BufferedImage(w, h, BufferedImage::TYPE_INT_ARGB);
    mapTexture =
        textures->getTexture(img, C4JRender::TEXTURE_FORMAT_RxGyBzAw,
                             false);  
                                      
    delete img;
    for (int i = 0; i < w * h; i++) {
        pixels[i] = 0x00000000;
    }

    
    
    if (genLUT) {
        reloadColours();
    }
    renderCount = 0;  
    m_optimised = optimised;
}

void Minimap::reloadColours() {
    ColourTable* colourTable = Minecraft::GetInstance()->getColourTable();
    
    
    for (int i = 0; i < (14 * 4);
         i++)  
    {
        if (i / 4 == 0) {
            
            LUT[i] = (((i + i / w) & 1) * 8 + 16);
            
        } else {
            int color =
                colourTable->getColor(MaterialColor::colors[i / 4]->col);
            int brightness = i & 3;

            int br = 220;
            if (brightness == 2) br = 255;
            if (brightness == 0) br = 180;

            int r = ((color >> 16) & 0xff) * br / 255;
            int g = ((color >> 8) & 0xff) * br / 255;
            int b = ((color) & 0xff) * br / 255;
#if defined(__APPLE__)
            LUT[i] = (255 << 24) | (r << 16) | (g << 8) | b;
#elif defined(_WIN64) || defined(__linux__)
            LUT[i] = 255 << 24 | b << 16 | g << 8 | r;
#else
            LUT[i] = r << 24 | g << 16 | b << 8 | 255;
#endif

            
        }
    }
    genLUT = false;
}


void Minimap::render(std::shared_ptr<Player> player, Textures* textures,
                     std::shared_ptr<MapItemSavedData> data, int entityId) {
    
    
    
    if (!m_optimised || (renderCount & 7) == 0) {
        for (int i = 0; i < w * h; i++) {
            int val = data->colors[i];
            
            
            pixels[i] = LUT[val];
        }
    }
    renderCount++;

    
    
    
    textures->replaceTextureDirect(pixels, w, h, mapTexture);

    int x = 0;
    int y = 0;
    Tesselator* t = Tesselator::getInstance();

    float vo = 0;

    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_ALPHA_TEST);
    t->begin();
    
    
    float Offset = -0.02f;
    t->vertexUV((float)(x + 0 + vo), (float)(y + h - vo), (float)(Offset),
                (float)(0), (float)(1));
    t->vertexUV((float)(x + w - vo), (float)(y + h - vo), (float)(Offset),
                (float)(1), (float)(1));
    t->vertexUV((float)(x + w - vo), (float)(y + 0 + vo), (float)(Offset),
                (float)(1), (float)(0));
    t->vertexUV((float)(x + 0 + vo), (float)(y + 0 + vo), (float)(Offset),
                (float)(0), (float)(0));
    t->end();
    glEnable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);

    textures->bind(
        textures->loadTexture(TN_MISC_MAPICONS));  

    auto itEnd = data->decorations.end();

#if defined(_LARGE_WORLDS)
    std::vector<MapItemSavedData::MapDecoration*> m_edgeIcons;
#endif

    
    float fIconZ =
        -0.04f;  
    for (std::vector<MapItemSavedData::MapDecoration*>::iterator it =
             data->decorations.begin();
         it != itEnd; it++) {
        MapItemSavedData::MapDecoration* dec = *it;

        if (!dec->visible) continue;

        char imgIndex = dec->img;

#if defined(_LARGE_WORLDS)
        
        if (imgIndex >= 16) {
            m_edgeIcons.push_back(dec);
            continue;
        }
#endif

        
        
        if (player == nullptr && (imgIndex != 12))
            continue;
        else if (player != nullptr && imgIndex == 12)
            continue;
        else if (imgIndex == 12 && dec->entityId != entityId)
            continue;

        glPushMatrix();
        glTranslatef(x + dec->x / 2.0f + w / 2, y + dec->y / 2.0f + h / 2,
                     fIconZ);
        glRotatef(dec->rot * 360 / 16.0f, 0, 0, 1);
        glScalef(4, 4, 3);
        glTranslatef(-1.0f / 8.0f, +1.0f / 8.0f, 0);

        float u0 = (imgIndex % 4 + 0) / 4.0f;
        float v0 = (imgIndex / 4 + 0) / 4.0f;
        float u1 = (imgIndex % 4 + 1) / 4.0f;
        float v1 = (imgIndex / 4 + 1) / 4.0f;

        t->begin();
        t->vertexUV((float)(-1), (float)(+1), (float)(0), (float)(u0),
                    (float)(v0));
        t->vertexUV((float)(+1), (float)(+1), (float)(0), (float)(u1),
                    (float)(v0));
        t->vertexUV((float)(+1), (float)(-1), (float)(0), (float)(u1),
                    (float)(v1));
        t->vertexUV((float)(-1), (float)(-1), (float)(0), (float)(u0),
                    (float)(v1));
        t->end();
        glPopMatrix();
        fIconZ -= 0.01f;
    }

#if defined(_LARGE_WORLDS)
    
    textures->bind(textures->loadTexture(TN_MISC_ADDITIONALMAPICONS));

    fIconZ = -0.04f;  
    for (auto it = m_edgeIcons.begin(); it != m_edgeIcons.end(); it++) {
        MapItemSavedData::MapDecoration* dec = *it;

        char imgIndex = dec->img;
        imgIndex -= 16;

        
        
        if (player == nullptr && (imgIndex != 12))
            continue;
        else if (player != nullptr && imgIndex == 12)
            continue;
        else if (imgIndex == 12 && dec->entityId != entityId)
            continue;

        glPushMatrix();
        glTranslatef(x + dec->x / 2.0f + w / 2, y + dec->y / 2.0f + h / 2,
                     fIconZ);
        glRotatef(dec->rot * 360 / 16.0f, 0, 0, 1);
        glScalef(4, 4, 3);
        glTranslatef(-1.0f / 8.0f, +1.0f / 8.0f, 0);

        float u0 = (imgIndex % 4 + 0) / 4.0f;
        float v0 = (imgIndex / 4 + 0) / 4.0f;
        float u1 = (imgIndex % 4 + 1) / 4.0f;
        float v1 = (imgIndex / 4 + 1) / 4.0f;

        t->begin();
        t->vertexUV((float)(-1), (float)(+1), (float)(0), (float)(u0),
                    (float)(v0));
        t->vertexUV((float)(+1), (float)(+1), (float)(0), (float)(u1),
                    (float)(v0));
        t->vertexUV((float)(+1), (float)(-1), (float)(0), (float)(u1),
                    (float)(v1));
        t->vertexUV((float)(-1), (float)(-1), (float)(0), (float)(u0),
                    (float)(v1));
        t->end();
        glPopMatrix();
        fIconZ -= 0.01f;
    }
#endif

    glPushMatrix();
    
    glTranslatef(0, 0, -0.06f);
    glScalef(1, 1, 1);
    
    
    
    
    
    
    
    if (player != nullptr) {
        wchar_t playerPosText[32];
        memset(&playerPosText, 0, sizeof(wchar_t) * 32);
        int posx = floor(player->x);
        int posy = floor(player->y);
        int posz = floor(player->z);
        swprintf(playerPosText, 32, L"X: %d, Y: %d, Z: %d", posx, posy, posz);

        font->draw(playerPosText, x, y,
                   Minecraft::GetInstance()->getColourTable()->getColour(
                       eMinecraftColour_Map_Text));
    }
    
    glPopMatrix();
}
