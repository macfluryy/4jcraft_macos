#include "../../Platform/stdafx.h"
#include <iostream>
#include "../../IO/Streams/InputOutputStream.h"
#include "PacketListener.h"
#include "ContainerOpenPacket.h"

ContainerOpenPacket::ContainerOpenPacket() {
    containerId = 0;
    type = 0;
    title = 0;
    size = 0;
}

ContainerOpenPacket::ContainerOpenPacket(int containerId, int type, int title,
                                         int size) {
    this->containerId = containerId;
    this->type = type;
    this->title = title;
    this->size = size;
}

void ContainerOpenPacket::handle(PacketListener* listener) {
    listener->handleContainerOpen(shared_from_this());
}

void ContainerOpenPacket::read(DataInputStream* dis)  // throws IOException
{
    containerId = (int)(dis->readByte() & (uint8_t)0xff);
    type = (int)(dis->readByte() & (uint8_t)0xff);
    title = dis->readShort();
    size = (int)(dis->readByte() & (uint8_t)0xff);
}

void ContainerOpenPacket::write(DataOutputStream* dos)  // throws IOException
{
    dos->writeByte((uint8_t)containerId & (uint8_t)0xff);
    dos->writeByte((uint8_t)type & (uint8_t)0xff);
    dos->writeShort(title & 0xffff);
    dos->writeByte((uint8_t)size & (uint8_t)0xff);
}

int ContainerOpenPacket::getEstimatedSize() { return 5; }
