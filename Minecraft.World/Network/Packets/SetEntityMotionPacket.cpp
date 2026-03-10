#include "../../Platform/stdafx.h"
#include <iostream>
#include "../../IO/Streams/InputOutputStream.h"
#include "../../Headers/net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "SetEntityMotionPacket.h"



void SetEntityMotionPacket::_init(int id, double xd, double yd, double zd)
{
	this->id = id;
	double m = 3.9;
	if (xd < -m) xd = -m;
	if (yd < -m) yd = -m;
	if (zd < -m) zd = -m;
	if (xd > m) xd = m;
	if (yd > m) yd = m;
	if (zd > m) zd = m;
	xa = (int) (xd * 8000.0);
	ya = (int) (yd * 8000.0);
	za = (int) (zd * 8000.0);

    useBytes = false;
}

SetEntityMotionPacket::SetEntityMotionPacket()
{
	_init(0, 0.0f, 0.0f, 0.0f);
}

SetEntityMotionPacket::SetEntityMotionPacket(std::shared_ptr<Entity> e)
{
	_init(e->entityId, e->xd, e->yd, e->zd);
}

SetEntityMotionPacket::SetEntityMotionPacket(int id, double xd, double yd, double zd)
{
	_init(id, xd, yd, zd);
}

void SetEntityMotionPacket::read(DataInputStream *dis) //throws IOException
{
	id = dis->readShort();

	xa = dis->readShort();
    ya = dis->readShort();
	za = dis->readShort();

	useBytes = false;
}

void SetEntityMotionPacket::write(DataOutputStream *dos) //throws IOException
{
    dos->writeShort(id);

    dos->writeShort(xa);
    dos->writeShort(ya);
    dos->writeShort(za);
}

void SetEntityMotionPacket::handle(PacketListener *listener)
{
	listener->handleSetEntityMotion(shared_from_this());
}

int SetEntityMotionPacket::getEstimatedSize()
{
	return 8;
}

bool SetEntityMotionPacket::canBeInvalidated()
{
	return true;
}

bool SetEntityMotionPacket::isInvalidatedBy(std::shared_ptr<Packet> packet)
{
	std::shared_ptr<SetEntityMotionPacket> target = std::dynamic_pointer_cast<SetEntityMotionPacket>(packet);
	return target->id == id;
}
