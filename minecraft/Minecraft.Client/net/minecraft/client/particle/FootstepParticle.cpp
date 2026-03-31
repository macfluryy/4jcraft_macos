#include <GL/gl.h>

#include "FootstepParticle.h"
#include "Minecraft.Client/net/minecraft/client/renderer/Textures.h"
#include "Minecraft.Client/net/minecraft/client/renderer/Tesselator.h"
#include "Minecraft.World/net/minecraft/util/Mth.h"
#include "Minecraft.Client/net/minecraft/client/resources/ResourceLocation.h"
#include "4J.Render/4J_Render.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"
#include "Minecraft.Client/net/minecraft/client/particle/Particle.h"
#include "Minecraft.Client/net/minecraft/client/particle/ParticleEngine.h"

ResourceLocation FootstepParticle::FOOTPRINT_LOCATION =
    ResourceLocation(TN_MISC_FOOTSTEP);

FootstepParticle::FootstepParticle(Textures* textures, Level* level, double x,
                                   double y, double z)
    : Particle(level, x, y, z, 0, 0, 0) {
    // 4J added initialisers
    life = 0;
    lifeTime = 0;

    this->textures = textures;
    xd = yd = zd = 0;
    lifeTime = 200;
}

void FootstepParticle::render(Tesselator* t, float a, float xa, float ya,
                              float za, float xa2, float za2) {
    float time = (life + a) / lifeTime;
    time = time * time;

    float alpha = 2 - time * 2;
    if (alpha > 1) alpha = 1;
    alpha = alpha * 0.2f;

    glDisable(GL_LIGHTING);
    float r = 2 / 16.0f;

    float xx = (float)(x - xOff);
    float yy = (float)(y - yOff);
    float zz = (float)(z - zOff);

    float br =
        level->getBrightness(std::floor(x), std::floor(y), std::floor(z));

    textures->bindTexture(&FOOTPRINT_LOCATION);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    t->begin();
    t->color(br, br, br, alpha);
    t->vertexUV((float)(xx - r), (float)(yy), (float)(zz + r), (float)(0),
                (float)(1));
    t->vertexUV((float)(xx + r), (float)(yy), (float)(zz + r), (float)(1),
                (float)(1));
    t->vertexUV((float)(xx + r), (float)(yy), (float)(zz - r), (float)(1),
                (float)(0));
    t->vertexUV((float)(xx - r), (float)(yy), (float)(zz - r), (float)(0),
                (float)(0));
    t->end();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void FootstepParticle::tick() {
    life++;
    if (life == lifeTime) remove();
}

int FootstepParticle::getParticleTexture() {
    return ParticleEngine::ENTITY_PARTICLE_TEXTURE;
}