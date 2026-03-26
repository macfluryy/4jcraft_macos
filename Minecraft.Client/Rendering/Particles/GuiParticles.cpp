#include "../../Platform/stdafx.h"
#include "GuiParticles.h"
#include "GuiParticle.h"
#include "../../Textures/Textures.h"

GuiParticles::GuiParticles(Minecraft* mc) { this->mc = mc; }

void GuiParticles::tick() {
    for (unsigned int i = 0; i < particles.size(); i++) {
        GuiParticle* gp = particles[i];

        gp->preTick();
        gp->tick(this);

        if (gp->removed) {
            particles.erase(particles.begin() + i);
            i--;
        }
    }
}

void GuiParticles::add(GuiParticle* guiParticle) {
    particles.push_back(guiParticle);
    guiParticle->preTick();
}

void GuiParticles::render(float a) {
    // 4J Stu - Never used
}
