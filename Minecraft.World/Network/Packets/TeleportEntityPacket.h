#pragma once


#include "Packet.h"

class TeleportEntityPacket : public Packet, public std::enable_shared_from_this<TeleportEntityPacket>
{
public:
	int id;
	int32_t x, y, z;
	uint8_t yRot, xRot;

	TeleportEntityPacket();
	TeleportEntityPacket(std::shared_ptr<Entity> e);
	TeleportEntityPacket(int id, int32_t x, int32_t y, int32_t z, uint8_t yRot, uint8_t xRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(std::shared_ptr<Packet> packet);

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new TeleportEntityPacket()); }
	virtual int getId() { return 34; }
};