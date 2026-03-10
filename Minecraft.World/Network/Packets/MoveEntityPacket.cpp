#include "../../Platform/stdafx.h"
#include <iostream>
#include "../../IO/Streams/InputOutputStream.h"
#include "PacketListener.h"
#include "MoveEntityPacket.h"

MoveEntityPacket::MoveEntityPacket()
{
	hasRot = false;

	id = -1;
	xa = 0;
	ya = 0;
	za = 0;
	yRot = 0;
	xRot = 0;
}

MoveEntityPacket::MoveEntityPacket(int id)
{
	this->id = id;
	hasRot = false;

	xa = 0;
	ya = 0;
	za = 0;
	yRot = 0;
	xRot = 0;
}

void MoveEntityPacket::read(DataInputStream *dis) //throws IOException
{
	id = dis->readShort();
}

void MoveEntityPacket::write(DataOutputStream *dos) //throws IOException
{
	if( (id < 0 ) || (id >= 2048 ) )
	{
		// We shouln't be tracking an entity that doesn't have a short type of id
		__debugbreak();
	}
	dos->writeShort((short)id);
}

void MoveEntityPacket::handle(PacketListener *listener)
{
	listener->handleMoveEntity(shared_from_this());
}

int MoveEntityPacket::getEstimatedSize()
{
	return 2;
}

bool MoveEntityPacket::canBeInvalidated()
{
	return true;
}

bool MoveEntityPacket::isInvalidatedBy(std::shared_ptr<Packet> packet)
{
	std::shared_ptr<MoveEntityPacket> target = std::dynamic_pointer_cast<MoveEntityPacket>(packet);
	return target != NULL && target->id == id;
}

MoveEntityPacket::PosRot::PosRot()
{
	hasRot = true;
}

MoveEntityPacket::PosRot::PosRot(int id, int8_t xa, int8_t ya, int8_t za, int8_t yRot, int8_t xRot) : MoveEntityPacket( id )
{
    this->xa = (int)(int8_t)xa;
    this->ya = (int)(int8_t)ya;
    this->za = (int)(int8_t)za;
    this->yRot = (int)(int8_t)yRot;
    this->xRot = (int)(int8_t)xRot;
	hasRot = true;
}

void MoveEntityPacket::PosRot::read(DataInputStream *dis) //throws IOException
{
	MoveEntityPacket::read(dis);
    xa = (int)(int8_t)dis->readByte();
    ya = (int)(int8_t)dis->readByte();
    za = (int)(int8_t)dis->readByte();
    yRot = (int)(int8_t)dis->readByte();
    xRot = (int)(int8_t)dis->readByte();
}

void MoveEntityPacket::PosRot::write(DataOutputStream *dos) //throws IOException
{
	MoveEntityPacket::write(dos);
	dos->writeByte((uint8_t)(xa & 0xFF));
	dos->writeByte((uint8_t)(ya & 0xFF));
	dos->writeByte((uint8_t)(za & 0xFF));
	dos->writeByte((uint8_t)(yRot & 0xFF));
	dos->writeByte((uint8_t)(xRot & 0xFF));
}

int MoveEntityPacket::PosRot::getEstimatedSize()
{
	return 2+5;
}

MoveEntityPacket::Pos::Pos()
{
}

MoveEntityPacket::Pos::Pos(int id, int8_t xa, int8_t ya, int8_t za) : MoveEntityPacket(id)
{
	this->xa = xa;
	this->ya = ya;
	this->za = za;
}

void MoveEntityPacket::Pos::read(DataInputStream *dis) //throws IOException
{
	MoveEntityPacket::read(dis);
    xa = (int)(int8_t)dis->readByte();
    ya = (int)(int8_t)dis->readByte();
    za = (int)(int8_t)dis->readByte();
}

void MoveEntityPacket::Pos::write(DataOutputStream *dos) //throws IOException
{
	MoveEntityPacket::write(dos);
	dos->writeByte((uint8_t)(xa & 0xFF));
	dos->writeByte((uint8_t)(ya & 0xFF));
	dos->writeByte((uint8_t)(za & 0xFF));
}

int MoveEntityPacket::Pos::getEstimatedSize()
{
	return 2+3;
}

MoveEntityPacket::Rot::Rot()
{
	hasRot = true;
}

MoveEntityPacket::Rot::Rot(int id, int8_t yRot, int8_t xRot) : MoveEntityPacket(id)
{
	this->yRot = yRot;
	this->xRot = xRot;
	hasRot = true;
}

void MoveEntityPacket::Rot::read(DataInputStream *dis) //throws IOException
{
	MoveEntityPacket::read(dis);
    yRot = (int)(int8_t)dis->readByte();
    xRot = (int)(int8_t)dis->readByte();
}

void MoveEntityPacket::Rot::write(DataOutputStream *dos) //throws IOException
{
	MoveEntityPacket::write(dos);
	dos->writeByte((uint8_t)(yRot & 0xFF));
	dos->writeByte((uint8_t)(xRot & 0xFF));
}

int MoveEntityPacket::Rot::getEstimatedSize()
{
	return 2+2;
}
