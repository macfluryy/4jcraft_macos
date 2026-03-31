#include "../../../../Header Files/stdafx.h"
#include <iostream>
#include "java/InputOutputStream/InputOutputStream.h"
#include "../../world/entity/net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "TeleportEntityPacket.h"

TeleportEntityPacket::TeleportEntityPacket() {
    id = -1;
    x = 0;
    y = 0;
    z = 0;
    yRot = 0;
    xRot = 0;
}

TeleportEntityPacket::TeleportEntityPacket(std::shared_ptr<Entity> e) {
    id = e->entityId;
    x = GameMath::floor(e->x * 32);
    y = GameMath::floor(e->y * 32);
    z = GameMath::floor(e->z * 32);
    yRot = (uint8_t)(e->yRot * 256 / 360);
    xRot = (uint8_t)(e->xRot * 256 / 360);
}

TeleportEntityPacket::TeleportEntityPacket(int id, int x, int y, int z,
                                           uint8_t yRot, uint8_t xRot) {
    this->id = id;
    this->x = x;
    this->y = y;
    this->z = z;
    this->yRot = yRot;
    this->xRot = xRot;
}

void TeleportEntityPacket::read(DataInputStream* dis)  // throws IOException
{
    id = dis->readShort();
#ifdef _LARGE_WORLDS
    x = dis->readInt();
    y = dis->readInt();
    z = dis->readInt();
#else
    x = dis->readShort();
    y = dis->readShort();
    z = dis->readShort();
#endif
    yRot = (uint8_t)dis->read();
    xRot = (uint8_t)dis->read();
}

void TeleportEntityPacket::write(DataOutputStream* dos)  // throws IOException
{
    dos->writeShort(id);
#ifdef _LARGE_WORLDS
    dos->writeInt(x);
    dos->writeInt(y);
    dos->writeInt(z);
#else
    dos->writeShort(x);
    dos->writeShort(y);
    dos->writeShort(z);
#endif
    dos->write(yRot);
    dos->write(xRot);
}

void TeleportEntityPacket::handle(PacketListener* listener) {
    listener->handleTeleportEntity(shared_from_this());
}

int TeleportEntityPacket::getEstimatedSize() { return 2 + 2 + 2 + 2 + 1 + 1; }

bool TeleportEntityPacket::canBeInvalidated() { return true; }

bool TeleportEntityPacket::isInvalidatedBy(std::shared_ptr<Packet> packet) {
    std::shared_ptr<TeleportEntityPacket> target =
        std::dynamic_pointer_cast<TeleportEntityPacket>(packet);
    return target->id == id;
}