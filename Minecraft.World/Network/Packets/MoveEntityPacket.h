#pragma once


#include "Packet.h"

class MoveEntityPacket : public Packet, public std::enable_shared_from_this<MoveEntityPacket>
{

	// 4J JEV, static inner/sub classes
public:
	class PosRot;
	class Pos;
	class Rot;

	int id;
	int8_t xa, ya, za, yRot, xRot;
	bool hasRot;

	MoveEntityPacket();
	MoveEntityPacket(int id);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(std::shared_ptr<Packet> packet);

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new MoveEntityPacket()); }
	virtual int getId() { return 30; }
};

class MoveEntityPacket::PosRot : public MoveEntityPacket
{
public:
	PosRot();
	PosRot(int id, int8_t xa, int8_t ya, int8_t za, int8_t yRot, int8_t xRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new MoveEntityPacket::PosRot()); }
	virtual int getId() { return 33; }
};

class MoveEntityPacket::Pos : public MoveEntityPacket
{
public:
	Pos();
	Pos(int id, int8_t xa, int8_t ya, int8_t za);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new MoveEntityPacket::Pos()); }
	virtual int getId() { return 31; }
};

class MoveEntityPacket::Rot : public MoveEntityPacket
{
public:
	Rot();
	Rot(int id, int8_t yRot, int8_t xRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new MoveEntityPacket::Rot()); }
	virtual int getId() { return 32; }

};