#include "../../../../Header Files/stdafx.h"
#include <iostream>
#include "java/InputOutputStream/InputOutputStream.h"
#include "../../world/entity/net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "AnimatePacket.h"

AnimatePacket::AnimatePacket() {
    id = -1;
    action = 0;
}

AnimatePacket::AnimatePacket(std::shared_ptr<Entity> e, int action) {
    id = e->entityId;
    this->action = action;
}

void AnimatePacket::read(DataInputStream* dis)  // throws IOException
{
    id = dis->readInt();
    action = static_cast<int>(dis->readByte());
}

void AnimatePacket::write(DataOutputStream* dos)  // throws IOException
{
    dos->writeInt(id);
    dos->writeByte(static_cast<uint8_t>(action));
}

void AnimatePacket::handle(PacketListener* listener) {
    listener->handleAnimate(shared_from_this());
}

int AnimatePacket::getEstimatedSize() { return 5; }
