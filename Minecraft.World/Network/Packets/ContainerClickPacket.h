#pragma once
using namespace std;

#include "Packet.h"

class ContainerClickPacket : public Packet, public std::enable_shared_from_this<ContainerClickPacket>
{
public:
	int containerId;
    int slotNum;
    int buttonNum;
    short uid;
    std::shared_ptr<ItemInstance> item;
    bool quickKey;

	ContainerClickPacket();
	~ContainerClickPacket();
    ContainerClickPacket(int containerId, int slotNum, int buttonNum, bool quickKey, std::shared_ptr<ItemInstance> item, short uid);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new ContainerClickPacket()); }
	virtual int getId() { return 102; }
};


