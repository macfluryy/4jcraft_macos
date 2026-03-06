#pragma once

#include "Entity.h"

class Level;
class BossMob;

class BossMobPart : public Entity
{
public:
	eINSTANCEOF GetType() { return eTYPE_BOSS_MOB_PART; };
public:
	BossMob *bossMob;
	const std::wstring id;

	BossMobPart(BossMob *bossMob, const std::wstring &id, float w, float h);

protected:
	virtual void defineSynchedData();
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *tag);

public:
	virtual bool isPickable();
	virtual bool hurt(DamageSource *source, int damage);
	virtual bool is(std::shared_ptr<Entity> other);
};