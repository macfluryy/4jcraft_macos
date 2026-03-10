#pragma once
#include "Feature.h"

class TreeFeature : public Feature
{
private:
	const int baseHeight;
	const int trunkType;
	const int leafType;
	const bool addJungleFeatures;

public:
	TreeFeature(bool doUpdate);
	TreeFeature(bool doUpdate, int baseHeight, int trunkType, int leafType, bool addJungleFeatures);

	virtual bool place(Level *level, Random *random, int x, int y, int z);

private:
	void addVine(Level *level, int xx, int yy, int zz, int dir);
};
