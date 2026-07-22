#include "SpiderRenderer.h"

#include <memory>

#include "platform/sdl2/Render.h"

#include "minecraft/SharedConstants.h"
#include "minecraft/client/model/SpiderModel.h"
#include "minecraft/client/renderer/Textures.h"
#include "minecraft/client/renderer/entity/MobRenderer.h"
#include "minecraft/client/resources/ResourceLocation.h"
#include "minecraft/world/entity/LivingEntity.h"
#include "minecraft/world/entity/monster/Spider.h"

ResourceLocation SpiderRenderer::SPIDER_LOCATION =
    ResourceLocation(TN_MOB_SPIDER);
ResourceLocation SpiderRenderer::SPIDER_EYES_LOCATION =
    ResourceLocation(TN_MOB_SPIDER_EYES);

SpiderRenderer::SpiderRenderer() : MobRenderer(new SpiderModel(), 1.0f) {
    this->setArmor(new SpiderModel());
}

float SpiderRenderer::getFlipDegrees(std::shared_ptr<LivingEntity> spider) {
    return 180;
}

int SpiderRenderer::prepareArmor(std::shared_ptr<LivingEntity> _spider,
                                 int layer, float a) {
    
    
    std::shared_ptr<Spider> spider = std::dynamic_pointer_cast<Spider>(_spider);

    if (layer != 0) return -1;
    bindTexture(&SPIDER_EYES_LOCATION);
    
    float br = 1.0f;  
    glEnable(GL_BLEND);
    
    
    
    
    
    glBlendFunc(GL_ONE, GL_ONE);
    if (spider->isInvisible())
        glDepthMask(false);
    else
        glDepthMask(true);

    if (SharedConstants::TEXTURE_LIGHTING) {
        
        
        
        
        
        int col = 0x00f0;
        int u = col % 65536;
        int v = col / 65536;

        glMultiTexCoord2f(GL_TEXTURE1, u / 1.0f, v / 1.0f);
        glColor4f(1, 1, 1, 1);
    }
    
    glColor4f(1, 1, 1, br);
    return 1;
}

ResourceLocation* SpiderRenderer::getTextureLocation(
    std::shared_ptr<Entity> mob) {
    return &SPIDER_LOCATION;
}