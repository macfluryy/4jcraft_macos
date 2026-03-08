#pragma once


#include "Packet.h"

class TakeItemEntityPacket : public Packet, public std::enable_shared_from_this<TakeItemEntityPacket>
{
public:
	int itemId, playerId;

	TakeItemEntityPacket();
	TakeItemEntityPacket(int itemId, int playerId);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new TakeItemEntityPacket()); }
	virtual int getId() { return 22; }
};