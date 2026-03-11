#include "../../Platform/stdafx.h"
#include <iostream>
#include <limits>
#include "../../IO/Streams/InputOutputStream.h"
#include "PacketListener.h"
#include "MoveEntityPacketSmall.h"


MoveEntityPacketSmall::MoveEntityPacketSmall() 
{
	hasRot = false;

	id = -1;
	xa = 0;
	ya = 0;
	za = 0;
	yRot = 0;
	xRot = 0;
}

MoveEntityPacketSmall::MoveEntityPacketSmall(int id)
{
	this->id = id;
	hasRot = false;
	xa = ya = za = yRot = xRot = 0;
}

void MoveEntityPacketSmall::read(DataInputStream *dis) //throws IOException
{
	id = dis->readShort();
}

void MoveEntityPacketSmall::write(DataOutputStream *dos) //throws IOException
{
	if(id < 0 || id > std::numeric_limits<int16_t>::max() )
	{
		// We shouln't be tracking an entity that doesn't have a short type of id
		__debugbreak();
	}
	dos->writeShort((short)id);
}

void MoveEntityPacketSmall::handle(PacketListener *listener)
{
	listener->handleMoveEntitySmall(shared_from_this());
}

int MoveEntityPacketSmall::getEstimatedSize()
{
	return 2;
}

bool MoveEntityPacketSmall::canBeInvalidated()
{
	return true;
}

bool MoveEntityPacketSmall::isInvalidatedBy(std::shared_ptr<Packet> packet)
{
	std::shared_ptr<MoveEntityPacketSmall> target = std::dynamic_pointer_cast<MoveEntityPacketSmall>(packet);
	return target != NULL && target->id == id;
}

MoveEntityPacketSmall::PosRot::PosRot()
{
	hasRot = true;
}

MoveEntityPacketSmall::PosRot::PosRot(int id, int8_t xa, int8_t ya, int8_t za, int8_t yRot, int8_t xRot) : MoveEntityPacketSmall( id )
{
	this->xa = xa;
	this->ya = ya;
	this->za = za;
	this->yRot = yRot;
	this->xRot = xRot;
	hasRot = true;
}

void MoveEntityPacketSmall::PosRot::read(DataInputStream *dis) //throws IOException
{
    MoveEntityPacketSmall::read(dis);
    xa = (int8_t)dis->readByte();
    ya = (int8_t)dis->readByte();
    za = (int8_t)dis->readByte();
    yRot = (int8_t)dis->readByte();
    xRot = (int8_t)dis->readByte();
}

void MoveEntityPacketSmall::PosRot::write(DataOutputStream *dos) //throws IOException
{
    MoveEntityPacketSmall::write(dos);
    dos->writeByte((uint8_t)(xa & 0xFF));
    dos->writeByte((uint8_t)(ya & 0xFF));
    dos->writeByte((uint8_t)(za & 0xFF));
    dos->writeByte((uint8_t)(yRot & 0xFF));
    dos->writeByte((uint8_t)(xRot & 0xFF));
}

int MoveEntityPacketSmall::PosRot::getEstimatedSize()
{
	return 2+5;
}

MoveEntityPacketSmall::Pos::Pos()
{
}

MoveEntityPacketSmall::Pos::Pos(int id, int8_t xa, int8_t ya, int8_t za) : MoveEntityPacketSmall(id)
{
	this->xa = xa;
	this->ya = ya;
	this->za = za;
}

void MoveEntityPacketSmall::Pos::read(DataInputStream *dis) //throws IOException
{
    MoveEntityPacketSmall::read(dis);
    xa = (int8_t)dis->readByte();
    ya = (int8_t)dis->readByte();
    za = (int8_t)dis->readByte();
}

void MoveEntityPacketSmall::Pos::write(DataOutputStream *dos) //throws IOException
{
	MoveEntityPacketSmall::write(dos);
	dos->writeByte((uint8_t)(xa & 0xFF));
	dos->writeByte((uint8_t)(ya & 0xFF));
	dos->writeByte((uint8_t)(za & 0xFF));
}

int MoveEntityPacketSmall::Pos::getEstimatedSize()
{
	return 2+3;
}

MoveEntityPacketSmall::Rot::Rot()
{
	hasRot = true;
}

MoveEntityPacketSmall::Rot::Rot(int id, int8_t yRot, int8_t xRot) : MoveEntityPacketSmall(id)
{

	this->yRot = yRot;
	this->xRot = xRot;
	hasRot = true;
}

void MoveEntityPacketSmall::Rot::read(DataInputStream *dis) //throws IOException
{
    MoveEntityPacketSmall::read(dis);
    yRot = (int8_t)dis->readByte();
}

void MoveEntityPacketSmall::Rot::write(DataOutputStream *dos) //throws IOException
{
	MoveEntityPacketSmall::write(dos);
	dos->writeByte((uint8_t)(yRot & 0xFF));
}

int MoveEntityPacketSmall::Rot::getEstimatedSize()
{
	return 2+1;
}
