#include "../Platform/stdafx.h"

#include "../Headers/net.minecraft.world.entity.h"
#include "HitResult.h"

HitResult::HitResult(int x, int y, int z, int f, Vec3 *pos)
{
	this->type = TILE;
	this->x = x;
	this->y = y;
	this->z = z;
	this->f = f;
	this->pos = Vec3::newTemp(pos->x, pos->y, pos->z);

	this->entity = nullptr;
}

HitResult::HitResult(std::shared_ptr<Entity> entity)
{
	this->type = ENTITY;
	this->entity = entity;
	pos = Vec3::newTemp(entity->x, entity->y, entity->z);

	x = y = z = f = 0;
}

double HitResult::distanceTo(std::shared_ptr<Entity> e)
{
	double xd = pos->x - e->x;
	double yd = pos->y - e->y;
	double zd = pos->z - e->z;
	return xd * xd + yd * yd + zd * zd;
}