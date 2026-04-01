#include "DefaultRenderer.h"

#include "4J.Render/4J_Render.h"

void DefaultRenderer::render(std::shared_ptr<Entity> entity, double x, double y,
                             double z, float rot, float a) {
    glPushMatrix();
    // 4J - removed following line as doesn't really make any sense
    //    render(entity->bb, (x-entity->xOld), (y-entity->yOld),
    //    (z-entity->zOld));
    glPopMatrix();
}
