#pragma once

#include <cstdint>

#include "Packet.h"

class KickPlayerPacket : public Packet, public std::enable_shared_from_this<KickPlayerPacket>
{
public:
	std::uint8_t m_networkSmallId;

	KickPlayerPacket();
	KickPlayerPacket(std::uint8_t networkSmallId);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new KickPlayerPacket()); }
	virtual int getId() { return 159; }
};
