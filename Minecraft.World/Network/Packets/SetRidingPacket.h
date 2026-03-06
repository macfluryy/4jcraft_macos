#pragma once
using namespace std;

#include "Packet.h"

class SetRidingPacket : public Packet, public enable_shared_from_this<SetRidingPacket>
{
public:
	int riderId, riddenId;

	SetRidingPacket();
	SetRidingPacket(std::shared_ptr<Entity> rider, std::shared_ptr<Entity> riding);

	virtual int getEstimatedSize();
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(std::shared_ptr<Packet> packet);

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new SetRidingPacket()); }
	virtual int getId() { return 39; }

};