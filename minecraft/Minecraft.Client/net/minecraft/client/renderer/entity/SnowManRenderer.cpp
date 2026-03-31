#include <memory>

#include "Minecraft.Client/net/minecraft/client/model/SnowManModel.h"
#include "Minecraft.Client/net/minecraft/client/model/geom/ModelPart.h"
#include "EntityRenderDispatcher.h"
#include "SnowManRenderer.h"
#include "4J.Render/4J_Render.h"
#include "Minecraft.World/net/minecraft/world/entity/LivingEntity.h"
#include "Minecraft.World/net/minecraft/world/entity/animal/SnowMan.h"
#include "Minecraft.World/net/minecraft/world/item/Item.h"
#include "Minecraft.World/net/minecraft/world/item/ItemInstance.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"
#include "Minecraft.Client/net/minecraft/client/renderer/ItemInHandRenderer.h"
#include "Minecraft.Client/net/minecraft/client/renderer/Textures.h"
#include "Minecraft.Client/net/minecraft/client/renderer/TileRenderer.h"
#include "Minecraft.Client/net/minecraft/client/renderer/entity/MobRenderer.h"
#include "Minecraft.Client/net/minecraft/client/resources/ResourceLocation.h"

ResourceLocation SnowManRenderer::SNOWMAN_LOCATION =
    ResourceLocation(TN_MOB_SNOWMAN);

SnowManRenderer::SnowManRenderer() : MobRenderer(new SnowManModel(), 0.5f) {
    model = (SnowManModel*)MobRenderer::model;
    this->setArmor(model);
}

void SnowManRenderer::additionalRendering(std::shared_ptr<LivingEntity> _mob,
                                          float a) {
    // 4J - original version used generics and thus had an input parameter of
    // type SnowMan rather than shared_ptr<Mob>  we have here - do some casting
    // around instead
    std::shared_ptr<SnowMan> mob = std::dynamic_pointer_cast<SnowMan>(_mob);

    MobRenderer::additionalRendering(mob, a);
    std::shared_ptr<ItemInstance> headGear =
        std::make_shared<ItemInstance>(Tile::pumpkin, 1);
    if (headGear != nullptr && headGear->getItem()->id < 256) {
        glPushMatrix();
        model->head->translateTo(1 / 16.0f);

        if (TileRenderer::canRender(
                Tile::tiles[headGear->id]->getRenderShape())) {
            float s = 10 / 16.0f;
            glTranslatef(-0 / 16.0f, -5.5f / 16.0f, 0 / 16.0f);
            glRotatef(90, 0, 1, 0);
            glScalef(s, -s, s);
        }

        entityRenderDispatcher->itemInHandRenderer->renderItem(mob, headGear,
                                                               0);

        glPopMatrix();
    }
}

ResourceLocation* SnowManRenderer::getTextureLocation(
    std::shared_ptr<Entity> mob) {
    return &SNOWMAN_LOCATION;
}