#include "../../Build/stdafx.h"
#include <iostream>
#include "../../IO/Streams/InputOutputStream.h"
#include "../../Headers/net.minecraft.world.item.h"
#include "PacketListener.h"
#include "ContainerSetContentPacket.h"



ContainerSetContentPacket::~ContainerSetContentPacket()
{
	delete[] items.data;
}

ContainerSetContentPacket::ContainerSetContentPacket() 
{
	containerId = 0;
}

ContainerSetContentPacket::ContainerSetContentPacket(int containerId, vector<shared_ptr<ItemInstance> > *newItems)
{
	this->containerId = containerId;
	items = ItemInstanceArray((int)newItems->size());
	for (unsigned int i = 0; i < items.length; i++)
	{
		shared_ptr<ItemInstance> item = newItems->at(i);
		items[i] = item == NULL ? nullptr : item->copy();
	}
}

void ContainerSetContentPacket::read(DataInputStream *dis) //throws IOException 
{
	containerId = (int)dis->readByte();
	int count = dis->readShort();
	items = ItemInstanceArray(count);
	for (int i = 0; i < count; i++) 
	{
		items[i] = readItem(dis);
	}
}

void ContainerSetContentPacket::write(DataOutputStream *dos) //throws IOException
{
	dos->writeByte((byte)containerId);
	dos->writeShort(items.length);
	for (unsigned int i = 0; i < items.length; i++) 
	{
		writeItem(items[i], dos);
	}
}

void ContainerSetContentPacket::handle(PacketListener *listener)
{
	listener->handleContainerContent(shared_from_this());
}

int ContainerSetContentPacket::getEstimatedSize() 
{
	return 3 + items.length * 5;
}
