#pragma once


#include "Packet.h"

class UseItemPacket : public Packet, public std::enable_shared_from_this<UseItemPacket>
{
private:
	static const float CLICK_ACCURACY;
	int x, y, z, face;
	std::shared_ptr<ItemInstance> item;
	float clickX, clickY, clickZ;

public:
	UseItemPacket();
	UseItemPacket(int x, int y, int z, int face, std::shared_ptr<ItemInstance> item, float clickX, float clickY, float clickZ);
	~UseItemPacket();

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

	int getX();
	int getY();
	int getZ();
	int getFace();
	std::shared_ptr<ItemInstance> getItem();
	float getClickX();
	float getClickY();
	float getClickZ();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new UseItemPacket()); }
	virtual int getId() { return 15; }
};
