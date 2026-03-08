#pragma once


#include "Packet.h"

#include "../../Build/stdafx.h"
#include <iostream>
#include "PacketListener.h"

class ChunkVisibilityPacket : public Packet, public std::enable_shared_from_this<ChunkVisibilityPacket>
{
public:
	int x, z;
    bool visible;

	ChunkVisibilityPacket();
	ChunkVisibilityPacket(int x, int z, bool visible);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new ChunkVisibilityPacket()); }
	virtual int getId() { return 50; }
};


