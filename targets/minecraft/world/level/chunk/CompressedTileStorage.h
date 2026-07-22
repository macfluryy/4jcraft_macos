#pragma once

#include <cstdint>
#include <format>
#include <mutex>
#include <vector>

class DataInputStream;
class DataOutputStream;
template <typename T>
class XLockFreeStack;






































































class TileCompressData_SPU;

class CompressedTileStorage {
    friend class TileCompressData_SPU;

private:
    unsigned char* indicesAndData;

public:
    int allocatedSize;

private:
    static const int INDEX_OFFSET_MASK = 0x7ffe;
    static const int INDEX_OFFSET_SHIFT = 1;
    static const int INDEX_TILE_MASK = 0x00ff;
    static const int INDEX_TILE_SHIFT = 8;
    static const int INDEX_TYPE_MASK = 0x0003;
    static const int INDEX_TYPE_1_BIT = 0x0000;
    static const int INDEX_TYPE_2_BIT = 0x0001;
    static const int INDEX_TYPE_4_BIT = 0x0002;
    static const int INDEX_TYPE_0_OR_8_BIT = 0x0003;
    static const int INDEX_TYPE_0_BIT_FLAG = 0x0004;

    static const unsigned int MM_PHYSICAL_4KB_BASE =
        0xE0000000;  
                     
public:
    CompressedTileStorage();
    CompressedTileStorage(
        CompressedTileStorage* copyFrom);  
    CompressedTileStorage(
        std::vector<uint8_t>& dataIn,
        unsigned int initOffset);  
                                   
    CompressedTileStorage(bool isEmpty);
    ~CompressedTileStorage();
    bool isSameAs(CompressedTileStorage* other);
    bool isRenderChunkEmpty(
        int y);  
private:
    inline static int getIndex(int block, int tile);
    inline static void getBlockAndTile(int* block, int* tile, int x, int y,
                                       int z);
    inline static void getBlock(int* block, int x, int y, int z);

public:
    void setData(
        std::vector<uint8_t>& dataIn,
        unsigned int inOffset);  
                                 
    void getData(std::vector<uint8_t>& retArray,
                 unsigned int retOffset);  
                                           
    int get(int x, int y, int z);          
    void set(int x, int y, int z, int val);  
    typedef void (*tileUpdatedCallback)(int x, int y, int z, void* param,
                                        int yparam);
    int setDataRegion(
        std::vector<uint8_t>& dataIn, int x0, int y0, int z0, int x1, int y1,
        int z1, int offset, tileUpdatedCallback callback, void* param,
        int yparam);  
                      
                      
    bool testSetDataRegion(std::vector<uint8_t>& dataIn, int x0, int y0, int z0,
                           int x1, int y1, int z1,
                           int offset);  
                                         
    int getDataRegion(
        std::vector<uint8_t>& dataInOut, int x0, int y0, int z0, int x1, int y1,
        int z1,
        int offset);  
                      
                      

    static void staticCtor();

    void compress(int upgradeBlock = -1);

public:
    void queueForDelete(unsigned char* data);

    static void tick();
    static int deleteQueueIndex;
    static XLockFreeStack<unsigned char> deleteQueue[3];

    static unsigned char compressBuffer[32768 + 256];

    static std::recursive_mutex cs_write;

    int getAllocatedSize(int* count0, int* count1, int* count2, int* count4,
                         int* count8);
    int getHighestNonEmptyY();
    bool isCompressed();

    void write(DataOutputStream* dos);
    void read(DataInputStream* dis);
    void reverseIndices(unsigned char* indices);
};
