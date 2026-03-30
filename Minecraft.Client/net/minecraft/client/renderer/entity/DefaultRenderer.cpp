#include "../../../../../../Minecraft.World/Header Files/stdafx.h"
#include "DefaultRenderer.h"
#include "../../../../../../Minecraft.World/net/minecraft/world/entity/net.minecraft.world.entity.h"

void DefaultRenderer::render(std::shared_ptr<Entity> entity, double x, double y,
                             double z, float rot, float a) {
    glPushMatrix();
    // 4J - removed following line as doesn't really make any sense
    //    render(entity->bb, (x-entity->xOld), (y-entity->yOld),
    //    (z-entity->zOld));
    glPopMatrix();
}
