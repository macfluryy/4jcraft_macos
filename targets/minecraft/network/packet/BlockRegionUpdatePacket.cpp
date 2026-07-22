#include "BlockRegionUpdatePacket.h"

#include <assert.h>
#include <string.h>

#include "app/mac/MacGame.h"
#include "PacketListener.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/dimension/Dimension.h"

#define BLOCK_REGION_UPDATE_FULLCHUNK 0x01
#define BLOCK_REGION_UPDATE_ZEROHEIGHT \
    0x02  
          

BlockRegionUpdatePacket::~BlockRegionUpdatePacket() {}

BlockRegionUpdatePacket::BlockRegionUpdatePacket() {
    shouldDelay = true;
    x = 0;
    y = 0;
    z = 0;
    xs = 0;
    ys = 0;
    zs = 0;
    bIsFullChunk = false;
}

BlockRegionUpdatePacket::BlockRegionUpdatePacket(int x, int y, int z, int xs,
                                                 int ys, int zs, Level* level) {
    shouldDelay = true;
    this->x = x;
    this->y = y;
    this->z = z;
    this->xs = xs;
    this->ys = ys;
    this->zs = zs;
    bIsFullChunk = false;
    levelIdx =
        ((level->dimension->id == 0) ? 0
                                     : ((level->dimension->id == -1) ? 1 : 2));

    
    
    
    
    std::vector<uint8_t> rawBuffer;

    if (xs == 16 && ys == Level::maxBuildHeight && zs == 16 &&
        ((x & 15) == 0) && (y == 0) && ((z & 15) == 0)) {
        bIsFullChunk = true;

        LevelChunk* lc = level->getChunkAt(x, z);
        rawBuffer = lc->getReorderedBlocksAndData(x & 0xF, y, z & 0xF, xs,
                                                  this->ys, zs);
    } else {
        rawBuffer = level->getBlocksAndData(x, y, z, xs, ys, zs, false);
    }

    if (rawBuffer.size() == 0) {
        size = 0;
        buffer = std::vector<uint8_t>();
    } else {
        
        
        
        unsigned int inputSize = (unsigned int)rawBuffer.size() * 2;
        unsigned char* ucTemp = new unsigned char[inputSize];

        Compression::getCompression()->CompressLZXRLE(
            ucTemp, &inputSize, rawBuffer.data(),
            (unsigned int)rawBuffer.size());
        
        
        unsigned char* ucTemp2 = new unsigned char[inputSize];
        memcpy(ucTemp2, ucTemp, inputSize);
        delete[] ucTemp;
        buffer = std::vector<uint8_t>(ucTemp2, ucTemp2 + inputSize);
        delete[] ucTemp2;
        size = inputSize;
    }
}

void BlockRegionUpdatePacket::read(DataInputStream* dis)  
{
    uint8_t chunkFlags = dis->readByte();
    x = dis->readInt();
    y = dis->readShort();
    z = dis->readInt();
    xs = dis->read() + 1;
    ys = dis->read() + 1;
    zs = dis->read() + 1;

    bIsFullChunk = (chunkFlags & BLOCK_REGION_UPDATE_FULLCHUNK) ? true : false;
    if (chunkFlags & BLOCK_REGION_UPDATE_ZEROHEIGHT) ys = 0;

    size = dis->readInt();
    levelIdx = (size >> 30) & 3;
    size &= 0x3fffffff;

    if (size == 0) {
        buffer = std::vector<uint8_t>();
    } else {
        std::vector<uint8_t> compressedBuffer(size);
        bool success = dis->readFully(compressedBuffer);

        int bufferSize = xs * ys * zs * 5 / 2;
        
        if (bIsFullChunk) bufferSize += (16 * 16);
        buffer = std::vector<uint8_t>(bufferSize);
        unsigned int outputSize = buffer.size();

        if (success) {
            Compression::getCompression()->DecompressLZXRLE(
                buffer.data(), &outputSize, compressedBuffer.data(), size);
        } else {
            app.DebugPrintf(
                "Not decompressing packet that wasn't fully read\n");
        }

        
        

        assert(buffer.size() == outputSize);
    }
}

void BlockRegionUpdatePacket::write(
    DataOutputStream* dos)  
{
    uint8_t chunkFlags = 0;
    if (bIsFullChunk) chunkFlags |= BLOCK_REGION_UPDATE_FULLCHUNK;
    if (ys == 0) chunkFlags |= BLOCK_REGION_UPDATE_ZEROHEIGHT;

    dos->writeByte(chunkFlags);
    dos->writeInt(x);
    dos->writeShort(y);
    dos->writeInt(z);
    dos->write(xs - 1);
    dos->write(ys - 1);
    dos->write(zs - 1);

    int sizeAndLevel = size;
    sizeAndLevel |= (levelIdx << 30);
    dos->writeInt(sizeAndLevel);
    dos->write(buffer, 0, size);
}

void BlockRegionUpdatePacket::handle(PacketListener* listener) {
    listener->handleBlockRegionUpdate(shared_from_this());
}

int BlockRegionUpdatePacket::getEstimatedSize() { return 17 + size; }
