#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "ContainerOpenPacket.h"

ContainerOpenPacket::ContainerOpenPacket()
{
	containerId = 0;
	type = 0;
	title = 0;
	size = 0;
}

ContainerOpenPacket::ContainerOpenPacket(int containerId, int type, int title, int size) 
{
	this->containerId = containerId;
	this->type = type;
	this->title = title;
	this->size = size;
}

void ContainerOpenPacket::handle(PacketListener *listener)
{
	listener->handleContainerOpen(shared_from_this());
}


void ContainerOpenPacket::read(DataInputStream *dis) //throws IOException 
{
	containerId = (int)(dis->readByte() & (byte)0xff);
	type = (int)(dis->readByte() & (byte)0xff);
	title = dis->readShort();
	size = (int)(dis->readByte() & (byte)0xff);
}

void ContainerOpenPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeByte((byte)containerId & (byte)0xff);
	dos->writeByte((byte)type & (byte)0xff);
	dos->writeShort(title & 0xffff);
	dos->writeByte((byte)size & (byte)0xff);
}

int ContainerOpenPacket::getEstimatedSize() 
{
	return 5;
}
