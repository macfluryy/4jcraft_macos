#pragma once



#include "../../Headers/com.mojang.nbt.h"
#include "TileEntity.h"
#include "../../Containers/Container.h"

class Player;
class Random;
class Level;
class CompoundTag;

class DispenserTileEntity: public TileEntity, public Container
{
public:
	eINSTANCEOF GetType() { return eTYPE_DISPENSERTILEENTITY; }
	static TileEntity *create() { return new DispenserTileEntity(); }


using TileEntity::setChanged;

private:
	ItemInstanceArray *items;
	Random *random;

public:
	DispenserTileEntity();
	virtual ~DispenserTileEntity();

	virtual unsigned int getContainerSize();
	virtual std::shared_ptr<ItemInstance> getItem(unsigned int slot);
	virtual std::shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);
	std::shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	bool removeProjectile(int itemId);
	int getRandomSlot();
	virtual void setItem(unsigned int slot, std::shared_ptr<ItemInstance> item);
	virtual int addItem(std::shared_ptr<ItemInstance> item);
	virtual int getName();
	virtual void load(CompoundTag *base);
	virtual void save(CompoundTag *base);
	virtual int getMaxStackSize();
	virtual bool stillValid(std::shared_ptr<Player> player);
	virtual void setChanged();

    void startOpen();
    void stopOpen();

	// 4J Added
	virtual std::shared_ptr<TileEntity> clone();
	void AddItemBack(std::shared_ptr<ItemInstance>item, unsigned int slot); 
};