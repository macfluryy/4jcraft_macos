#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "ContainerButtonClickPacket.h"



ContainerButtonClickPacket::ContainerButtonClickPacket()
{
}

ContainerButtonClickPacket::ContainerButtonClickPacket(int containerId, int buttonId)
{
	this->containerId = containerId;
	this->buttonId = buttonId;
}

void ContainerButtonClickPacket::handle(PacketListener *listener)
{
	listener->handleContainerButtonClick(shared_from_this());
}

void ContainerButtonClickPacket::read(DataInputStream *dis)
{
	containerId = (int)dis->readByte();
	buttonId = (int)dis->readByte();
}

void ContainerButtonClickPacket::write(DataOutputStream *dos)
{
	dos->writeByte((byte)containerId);
	dos->writeByte((byte)buttonId);
}

int ContainerButtonClickPacket::getEstimatedSize()
{
	return 2;
}
