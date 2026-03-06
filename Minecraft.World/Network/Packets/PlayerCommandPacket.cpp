#include "../../Build/stdafx.h"
#include <iostream>
#include "../../IO/Streams/InputOutputStream.h"
#include "PacketListener.h"
#include "../../Headers/net.minecraft.world.entity.h"
#include "PlayerCommandPacket.h"



const int PlayerCommandPacket::START_SNEAKING =		1;
const int PlayerCommandPacket::STOP_SNEAKING =		2;
const int PlayerCommandPacket::STOP_SLEEPING =		3;
const int PlayerCommandPacket::START_SPRINTING =	4;
const int PlayerCommandPacket::STOP_SPRINTING =		5;
const int PlayerCommandPacket::START_IDLEANIM =		6;
const int PlayerCommandPacket::STOP_IDLEANIM =		7;


PlayerCommandPacket::PlayerCommandPacket()
{
	id = -1;
	action = 0;
}

PlayerCommandPacket::PlayerCommandPacket(std::shared_ptr<Entity> e, int action)
{
	id = e->entityId;
	this->action = action;
}

void PlayerCommandPacket::read(DataInputStream *dis) //throws IOException 
{
	id = dis->readInt();
	action = dis->readByte();
}

void PlayerCommandPacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeInt(id);
	dos->writeByte(action);
}

void PlayerCommandPacket::handle(PacketListener *listener)
{
	listener->handlePlayerCommand(shared_from_this());
}

int PlayerCommandPacket::getEstimatedSize() 
{
	return 5;
}
