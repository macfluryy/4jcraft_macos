#pragma once


#include "Packet.h"

class MoveEntityPacketSmall : public Packet, public std::enable_shared_from_this<MoveEntityPacketSmall>
{

	// 4J JEV, static inner/sub classes
public:
	class PosRot;
	class Pos;
	class Rot;

	int id;
    int8_t xa, ya, za, yRot, xRot;
    bool hasRot;

	MoveEntityPacketSmall();
	MoveEntityPacketSmall(int id);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();
	virtual bool canBeInvalidated();
	virtual bool isInvalidatedBy(std::shared_ptr<Packet> packet);

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new MoveEntityPacketSmall()); }
	virtual int getId() { return 162; }
};

class MoveEntityPacketSmall::PosRot : public MoveEntityPacketSmall
{
public:
    PosRot();
    PosRot(int id, int8_t xa, int8_t ya, int8_t za, int8_t yRot, int8_t xRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new MoveEntityPacketSmall::PosRot()); }
	virtual int getId() { return 165; }
};

class MoveEntityPacketSmall::Pos : public MoveEntityPacketSmall
{
public:
	Pos();
    Pos(int id, int8_t xa, int8_t ya, int8_t za);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new MoveEntityPacketSmall::Pos()); }
	virtual int getId() { return 163; }

};

class MoveEntityPacketSmall::Rot : public MoveEntityPacketSmall
{
public:
	Rot();
    Rot(int id, int8_t yRot, int8_t xRot);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static std::shared_ptr<Packet> create() { return std::shared_ptr<Packet>(new MoveEntityPacketSmall::Rot()); }
	virtual int getId() { return 164; }

};