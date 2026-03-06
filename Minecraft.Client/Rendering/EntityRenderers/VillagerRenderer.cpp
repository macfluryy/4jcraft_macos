#include "../../Build/stdafx.h"
#include "../../../Minecraft.World/Headers/net.minecraft.world.entity.npc.h"
#include "../Models/VillagerModel.h"
#include "VillagerRenderer.h"

VillagerRenderer::VillagerRenderer() : MobRenderer(new VillagerModel(0), 0.5f)
{
	villagerModel = (VillagerModel *) model;
}

int VillagerRenderer::prepareArmor(std::shared_ptr<Mob> villager, int layer, float a)
{
	return -1;
}

void VillagerRenderer::render(std::shared_ptr<Entity> mob, double x, double y, double z, float rot, float a)
{
	MobRenderer::render(mob, x, y, z, rot, a);
}

void VillagerRenderer::renderName(std::shared_ptr<Mob> mob, double x, double y, double z)
{
}

void VillagerRenderer::additionalRendering(std::shared_ptr<Mob> mob, float a)
{
	MobRenderer::additionalRendering(mob, a);
}

void VillagerRenderer::scale(std::shared_ptr<Mob> _mob, float a)
{
	// 4J - original version used generics and thus had an input parameter of type Blaze rather than std::shared_ptr<Entity>  we have here - 
	// do some casting around instead
	std::shared_ptr<Villager> mob = dynamic_pointer_cast<Villager>(_mob);
	float s = 15 / 16.0f;
	if (mob->getAge() < 0)
	{
		s *= 0.5;
		shadowRadius = 0.25f;
	} else shadowRadius = 0.5f;
	glScalef(s, s, s);
}