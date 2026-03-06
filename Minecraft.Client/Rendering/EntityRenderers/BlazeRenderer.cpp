#include "../../Build/stdafx.h"
#include "../Models/BlazeModel.h"
#include "../../../Minecraft.World/Headers/net.minecraft.world.entity.monster.h"
#include "BlazeRenderer.h"

BlazeRenderer::BlazeRenderer() : MobRenderer(new BlazeModel(), 0.5f)
{
	this->modelVersion = ((BlazeModel *) model)->modelVersion();
}

void BlazeRenderer::render(std::shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	// 4J - original version used generics and thus had an input parameter of type Blaze rather than std::shared_ptr<Entity>  we have here - 
	// do some casting around instead
	std::shared_ptr<Blaze> mob = dynamic_pointer_cast<Blaze>(_mob);

	int modelVersion = ((BlazeModel *) model)->modelVersion();
	if (modelVersion != this->modelVersion)
	{
		this->modelVersion = modelVersion;
		model = new BlazeModel();
	}
	MobRenderer::render(mob, x, y, z, rot, a);
}