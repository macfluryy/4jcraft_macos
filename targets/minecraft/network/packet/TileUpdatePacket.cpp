#include "TileUpdatePacket.h"

#include <cstdint>

#include "PacketListener.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/dimension/Dimension.h"

TileUpdatePacket::TileUpdatePacket() { shouldDelay = true; }

TileUpdatePacket::TileUpdatePacket(int x, int y, int z, Level* level) {
    shouldDelay = true;
    this->x = x;
    this->y = y;
    this->z = z;
    block = level->getTile(x, y, z);
    data = level->getData(x, y, z);
    levelIdx =
        ((level->dimension->id == 0) ? 0
                                     : ((level->dimension->id == -1) ? 1 : 2));
}

void TileUpdatePacket::read(DataInputStream* dis)  
{
#ifdef _LARGE_WORLDS
    x = dis->readInt();
    y = dis->readUnsignedByte();
    z = dis->readInt();

    block = (int)dis->readShort() & 0xffff;

    std::uint8_t dataLevel = dis->readByte();
    data = dataLevel & 0xf;
    levelIdx = (dataLevel >> 4) & 0xf;
#else
    
    int xyzdata = dis->readInt();
    x = (xyzdata >> 22) & 0x3ff;
    y = (xyzdata >> 14) & 0xff;
    z = (xyzdata >> 4) & 0x3ff;
    x = (x << 22) >> 22;
    z = (z << 22) >> 22;
    data = xyzdata & 0xf;
    block = (int)dis->readShort() & 0xffff;
    

    
    levelIdx = (int)dis->readByte();
#endif
}

void TileUpdatePacket::write(DataOutputStream* dos)  
{
#ifdef _LARGE_WORLDS
    dos->writeInt(x);
    dos->write(y);
    dos->writeInt(z);
    dos->writeShort(block);

    std::uint8_t dataLevel = ((levelIdx & 0xf) << 4) | (data & 0xf);
    dos->writeByte(dataLevel);
#else
    
    
    
    
    int xyzdata = ((x & 0x3ff) << 22) | ((y & 0xff) << 14) |
                  ((z & 0x3ff) << 4) | (data & 0xf);
    
    dos->writeInt(xyzdata);
    dos->writeShort(block);

    
    dos->write(levelIdx);
#endif
}

void TileUpdatePacket::handle(PacketListener* listener) {
    listener->handleTileUpdate(shared_from_this());
}

int TileUpdatePacket::getEstimatedSize() {
#ifdef _LARGE_WORLDS
    return 12;
#else
    return 5;
#endif
}
