#pragma once
using namespace std;

#include "Packet.h"

class PlayerCommandPacket : public Packet, public std::enable_shared_from_this<PlayerCommandPacket>
{
public:
	static const int START_SNEAKING;
    static const int STOP_SNEAKING;
	static const int STOP_SLEEPING;
	static const int START_SPRINTING;
	static const int STOP_SPRINTING;
	static const int START_IDLEANIM;
	static const int STOP_IDLEANIM;

	// 4J Added
	// 4J-PB - Making this host only setting
	/*
	static const int SHOW_ON_MAPS;
	static const int HIDE_ON_MAPS;
	*/

    int id;
    int action;

	PlayerCommandPacket();
	PlayerCommandPacket(std::shared_ptr<Entity> e, int action);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new PlayerCommandPacket()); }
	virtual int getId() { return 19; }
};