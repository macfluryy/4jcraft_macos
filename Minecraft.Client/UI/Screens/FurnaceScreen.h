#pragma once
#include "AbstractContainerScreen.h"

class FurnaceTileEntity;
class Inventory;

class FurnaceScreen : public AbstractContainerScreen
{
private:
	std::shared_ptr<FurnaceTileEntity> furnace;

public:
	FurnaceScreen(std::shared_ptr<Inventory> inventory, std::shared_ptr<FurnaceTileEntity> furnace);
protected:
	virtual void renderLabels();
	virtual void renderBg(float a);
};