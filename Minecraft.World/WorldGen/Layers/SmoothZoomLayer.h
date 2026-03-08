#pragma once

#include "Layer.h"

class SmoothZoomLayer : public Layer
{
public:
	SmoothZoomLayer(__int64 seedMixup, std::shared_ptr<Layer>parent);

	virtual intArray getArea(int xo, int yo, int w, int h);
	static std::shared_ptr<Layer>zoom(__int64 seed, std::shared_ptr<Layer>sup, int count);
};