#include "Minecraft.World/Header Files/stdafx.h"
#include "../../model/SnowManModel.h"
#include "Minecraft.World/net/minecraft/world/entity/animal/net.minecraft.world.entity.animal.h"
#include "Minecraft.World/net/minecraft/world/level/tile/net.minecraft.world.level.tile.h"
#include "Minecraft.World/net/minecraft/world/item/net.minecraft.world.item.h"
#include "../../model/geom/ModelPart.h"
#include "EntityRenderDispatcher.h"
#include "SnowManRenderer.h"

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
        std::shared_ptr<ItemInstance>(new ItemInstance(Tile::pumpkin, 1));
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