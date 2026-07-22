#include "PistonPieceRenderer.h"

#include <memory>

#include "platform/sdl2/Render.h"

#include "minecraft/client/Lighting.h"
#include "minecraft/client/renderer/Tesselator.h"
#include "minecraft/client/renderer/Textures.h"
#include "minecraft/client/renderer/TileRenderer.h"
#include "minecraft/client/renderer/texture/TextureAtlas.h"
#include "minecraft/client/resources/ResourceLocation.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/level/tile/entity/PistonPieceTileEntity.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"
#include "minecraft/world/level/tile/piston/PistonBaseTile.h"
#include "minecraft/world/level/tile/piston/PistonExtensionTile.h"

ResourceLocation PistonPieceRenderer::SIGN_LOCATION =
    ResourceLocation(TN_ITEM_SIGN);

PistonPieceRenderer::PistonPieceRenderer() { tileRenderer = nullptr; }

void PistonPieceRenderer::render(std::shared_ptr<TileEntity> _entity, double x,
                                 double y, double z, float a, bool setColor,
                                 float alpha, bool useCompiled) {
    
    
    std::shared_ptr<PistonPieceEntity> entity =
        std::dynamic_pointer_cast<PistonPieceEntity>(_entity);

    Tile* tile = Tile::tiles[entity->getId()];
    if (tile != nullptr &&
        entity->getProgress(a) <=
            1)  
                
                
    {
        Tesselator* t = Tesselator::getInstance();
        bindTexture(&TextureAtlas::LOCATION_BLOCKS);

        Lighting::turnOff();
        glColor4f(1, 1, 1,
                  1);  
                       
                       

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glDisable(GL_CULL_FACE);

        t->begin();

        t->offset((float)x - entity->x + entity->getXOff(a),
                  (float)y - entity->y + entity->getYOff(a),
                  (float)z - entity->z + entity->getZOff(a));
        t->color(1, 1, 1);
        if (tile == Tile::pistonExtension && entity->getProgress(a) < 0.5f) {
            
            tileRenderer->tesselatePistonArmNoCulling(tile, entity->x,
                                                      entity->y, entity->z,
                                                      false, entity->getData());
        } else if (entity->isSourcePiston() && !entity->isExtending()) {
            
            Tile::pistonExtension->setOverrideTopTexture(
                ((PistonBaseTile*)tile)->getPlatformTexture());
            tileRenderer->tesselatePistonArmNoCulling(
                Tile::pistonExtension, entity->x, entity->y, entity->z,
                entity->getProgress(a) < 0.5f, entity->getData());
            Tile::pistonExtension->clearOverrideTopTexture();

            t->offset((float)x - entity->x, (float)y - entity->y,
                      (float)z - entity->z);
            tileRenderer->tesselatePistonBaseForceExtended(
                tile, entity->x, entity->y, entity->z, entity->getData());
        } else {
            tileRenderer->tesselateInWorldNoCulling(tile, entity->x, entity->y,
                                                    entity->z,
                                                    entity->getData(), entity);
        }
        t->offset(0, 0, 0);
        t->end();

        Lighting::turnOn();
    }
}

void PistonPieceRenderer::onNewLevel(Level* level) {
    delete tileRenderer;
    tileRenderer = new TileRenderer(level);
}
