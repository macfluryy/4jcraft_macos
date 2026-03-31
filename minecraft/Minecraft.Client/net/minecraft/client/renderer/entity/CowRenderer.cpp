#include <memory>

#include "CowRenderer.h"
#include "Minecraft.Client/net/minecraft/client/renderer/Textures.h"
#include "Minecraft.Client/net/minecraft/client/renderer/entity/MobRenderer.h"
#include "Minecraft.Client/net/minecraft/client/resources/ResourceLocation.h"

class Model;

ResourceLocation CowRenderer::COW_LOCATION = ResourceLocation(TN_MOB_COW);

CowRenderer::CowRenderer(Model* model, float shadow)
    : MobRenderer(model, shadow) {}

void CowRenderer::render(std::shared_ptr<Entity> _mob, double x, double y,
                         double z, float rot, float a) {
    MobRenderer::render(_mob, x, y, z, rot, a);
}

ResourceLocation* CowRenderer::getTextureLocation(std::shared_ptr<Entity> mob) {
    return &COW_LOCATION;
}