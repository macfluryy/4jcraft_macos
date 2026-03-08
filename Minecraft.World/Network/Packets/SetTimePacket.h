#pragma once


#include "Packet.h"

class SetTimePacket : public Packet, public std::enable_shared_from_this<SetTimePacket>
{
public:
	__int64 time;

	SetTimePacket();
	SetTimePacket(__int64 time);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(std::shared_ptr<Packet> packet);
	virtual bool isAync();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new SetTimePacket()); }
	virtual int getId() { return 4; }
};