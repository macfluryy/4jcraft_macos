#pragma once


#include "../Throwable.h"

class HitResult;

class ThrownEgg : public Throwable
{
public:
	eINSTANCEOF GetType() { return eTYPE_THROWNEGG; }

private:
	void _init();

public:
	ThrownEgg(Level *level);
	ThrownEgg(Level *level, std::shared_ptr<Mob> mob);
	ThrownEgg(Level *level, double x, double y, double z);

protected:
	virtual void onHit(HitResult *res);
};
