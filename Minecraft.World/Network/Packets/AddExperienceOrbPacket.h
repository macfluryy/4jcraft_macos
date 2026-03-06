#pragma once

#include "Packet.h"

class ExperienceOrb;

class AddExperienceOrbPacket : public Packet, public std::enable_shared_from_this<AddExperienceOrbPacket>
{
public:
	int id;
	int x, y, z;
	int value;

	AddExperienceOrbPacket();
	AddExperienceOrbPacket(std::shared_ptr<ExperienceOrb> e);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new AddExperienceOrbPacket()); }
	virtual int getId() { return 26; }
};