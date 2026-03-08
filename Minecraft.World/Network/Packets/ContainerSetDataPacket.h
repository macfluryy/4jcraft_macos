#pragma once


#include "Packet.h"

class ContainerSetDataPacket : public Packet, public std::enable_shared_from_this<ContainerSetDataPacket>
{
public:
	int containerId;
    int id;
    int value;

	ContainerSetDataPacket();
	ContainerSetDataPacket(int containerId, int id, int value);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new ContainerSetDataPacket()); }
	virtual int getId() { return 105; }
};