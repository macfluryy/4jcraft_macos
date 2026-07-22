#include "ChunkTilesUpdatePacket.h"

#include "PacketListener.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/dimension/Dimension.h"

ChunkTilesUpdatePacket::~ChunkTilesUpdatePacket() {}

ChunkTilesUpdatePacket::ChunkTilesUpdatePacket() {
    shouldDelay = true;
    xc = 0;
    zc = 0;
    count = (uint8_t)0;
}

ChunkTilesUpdatePacket::ChunkTilesUpdatePacket(int xc, int zc,
                                               std::vector<short>& positions,
                                               uint8_t count, Level* level) {
    shouldDelay = true;
    this->xc = xc;
    this->zc = zc;
    this->count = count;
    this->positions = std::vector<short>((short int)count);

    this->blocks = std::vector<uint8_t>((unsigned int)count);
    this->data = std::vector<uint8_t>((unsigned int)count);
    LevelChunk* levelChunk = level->getChunk(xc, zc);
    for (int i = 0; (uint8_t)i < count; i++) {
        int x = (positions[i] >> 12) & 15;
        int z = (positions[i] >> 8) & 15;
        int y = (positions[i]) & 255;

        this->positions[i] = positions[i];
        blocks[i] = (uint8_t)levelChunk->getTile(x, y, z);
        data[i] = (uint8_t)levelChunk->getData(x, y, z);
    }
    levelIdx =
        ((level->dimension->id == 0) ? 0
                                     : ((level->dimension->id == -1) ? 1 : 2));
}

void ChunkTilesUpdatePacket::read(DataInputStream* dis)  
{
    
#ifdef _LARGE_WORLDS
    xc = dis->readShort();
    zc = dis->readShort();
    
    xc = (int16_t)xc;
    zc = (int16_t)zc;
#else
    xc = dis->read();
    zc = dis->read();
    xc = (xc << 24) >> 24;
    zc = (zc << 24) >> 24;
#endif

    int countAndFlags = (int)dis->readByte();
    bool dataAllZero = ((countAndFlags & 0x80) == 0x80);
    levelIdx = (countAndFlags >> 5) & 3;
    count = (uint8_t)countAndFlags & (uint8_t)0x1f;

    positions = std::vector<short>((short int)count);
    blocks = std::vector<uint8_t>((unsigned int)count);
    data = std::vector<uint8_t>((unsigned int)count);

    int currentBlockType = -1;
    for (int i = 0; (uint8_t)i < count; i++) {
        int xzAndFlag = dis->readShort();
        int y = (int)dis->readByte();
        positions[i] = (xzAndFlag & 0xff00) | (y & 0xff);
        if ((xzAndFlag & 0x0080) == 0x0080) {
            currentBlockType = dis->read();
        }
        blocks[i] = (uint8_t)currentBlockType;
        if (!dataAllZero) {
            data[i] = (uint8_t)dis->read();
        } else {
            data[i] = (uint8_t)0;
        }
    }
}

void ChunkTilesUpdatePacket::write(DataOutputStream* dos)  
{
    
#ifdef _LARGE_WORLDS
    dos->writeShort(xc);
    dos->writeShort(zc);
#else
    dos->write(xc);
    dos->write(zc);
#endif
    
    
    
    bool dataAllZero = true;
    for (int i = 0; i < (int)count; i++) {
        if ((bool)data[i]) dataAllZero = false;
    }
    int countAndFlags = (int)count;
    if ((bool)dataAllZero) countAndFlags |= 0x80;
    countAndFlags |= (levelIdx << 5);
    dos->write(countAndFlags);
    int lastBlockType = -1;
    
    
    
    
    for (int i = 0; i < (int)count; i++) {
        int xzAndFlag = positions[i] & 0xff00;
        int y = positions[i] & 0xff;
        int thisBlockType = (int)blocks[i];
        if (thisBlockType != lastBlockType) {
            xzAndFlag |= 0x0080;  
                                  
            dos->writeShort(xzAndFlag);
            dos->write(y);
            dos->write(thisBlockType);
            lastBlockType = thisBlockType;
        } else {
            dos->writeShort(xzAndFlag);
            dos->write(y);
        }
        if (!dataAllZero) {
            dos->write((unsigned int)data[i]);
        }
    }
}

void ChunkTilesUpdatePacket::handle(PacketListener* listener) {
    listener->handleChunkTilesUpdate(shared_from_this());
}

int ChunkTilesUpdatePacket::getEstimatedSize() {
    bool dataAllZero = true;
    int lastBlockType = -1;
    int blockTypeChanges = 0;
    for (int i = 0; i < (int)count; i++) {
        if ((bool)data[i]) dataAllZero = false;
        int thisBlockType = (int)blocks[i];
        if (thisBlockType != lastBlockType) {
            blockTypeChanges++;
            lastBlockType = thisBlockType;
        }
    }
    int byteCount = 3 + 2 * (int)count + blockTypeChanges;
    if (!dataAllZero) {
        byteCount += (unsigned char)count;
    }

    return byteCount;
}
