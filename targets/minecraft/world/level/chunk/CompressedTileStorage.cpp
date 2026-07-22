#include "CompressedTileStorage.h"

#include <assert.h>
#include <bit>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "app/mac/Stubs/winapi_stubs.h"
#include "app/include/NetTypes.h"
#include "util/Definitions.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "java/System.h"



int CompressedTileStorage::deleteQueueIndex;
XLockFreeStack<unsigned char> CompressedTileStorage::deleteQueue[3];

std::recursive_mutex CompressedTileStorage::cs_write;

#if defined(PSVITA_PRECOMPUTED_TABLE)

static int* CompressedTile_StorageIndexTable = nullptr;

void CompressedTileStorage_InitTable() {
    if (CompressedTile_StorageIndexTable == nullptr) {
        CompressedTile_StorageIndexTable = (int*)malloc(sizeof(int) * 64);
        for (int j = 0; j < 64; j += 1) {
            int index = ((j & 0x30) << 7) | ((j & 0x0c) << 5) | (j & 0x03);
            CompressedTile_StorageIndexTable[j] = index;
        }
    }
}
#endif

CompressedTileStorage::CompressedTileStorage() {
    indicesAndData = nullptr;
    allocatedSize = 0;

#if defined(PSVITA_PRECOMPUTED_TABLE)
    CompressedTileStorage_InitTable();
#endif
}

CompressedTileStorage::CompressedTileStorage(CompressedTileStorage* copyFrom) {
    {
        std::lock_guard<std::recursive_mutex> lock(cs_write);
        allocatedSize = copyFrom->allocatedSize;
        if (allocatedSize > 0) {
            indicesAndData = (unsigned char*)malloc(
                allocatedSize);  
            memcpy(indicesAndData, copyFrom->indicesAndData, allocatedSize);
        } else {
            indicesAndData = nullptr;
        }
    }

#if defined(PSVITA_PRECOMPUTED_TABLE)
    CompressedTileStorage_InitTable();
#endif
}

CompressedTileStorage::CompressedTileStorage(std::vector<uint8_t>& initFrom,
                                             unsigned int initOffset) {
    indicesAndData = nullptr;
    allocatedSize = 0;

    
    
    indicesAndData = (unsigned char*)malloc(32768 + 4096);

    unsigned short* indices = (unsigned short*)indicesAndData;
    unsigned char* data = indicesAndData + 1024;

    int offset = 0;
    for (int i = 0; i < 512; i++) {
        indices[i] = INDEX_TYPE_0_OR_8_BIT | (offset << 1);

        if (!initFrom.empty()) {
            for (int j = 0; j < 64; j++) {
                *data++ = initFrom[getIndex(i, j) + initOffset];
            }
        } else {
            for (int j = 0; j < 64; j++) {
                *data++ = 0;
            }
        }

        offset += 64;
    }

    allocatedSize =
        32768 +
        1024;  
               
               

#if defined(PSVITA_PRECOMPUTED_TABLE)
    CompressedTileStorage_InitTable();
#endif
}

bool CompressedTileStorage::isCompressed() {
    return allocatedSize != 32768 + 1024;
}

CompressedTileStorage::CompressedTileStorage(bool isEmpty) {
    indicesAndData = nullptr;
    allocatedSize = 0;

    
    
    indicesAndData = (unsigned char*)malloc(4096);
    unsigned short* indices = (unsigned short*)indicesAndData;
    

    
    for (int i = 0; i < 512; i++) {
        indices[i] = INDEX_TYPE_0_OR_8_BIT | INDEX_TYPE_0_BIT_FLAG;
    }

    allocatedSize =
        1024;  
               
               

#if defined(PSVITA_PRECOMPUTED_TABLE)
    CompressedTileStorage_InitTable();
#endif
}

bool CompressedTileStorage::isRenderChunkEmpty(
    int y)  
{
    int block;
    unsigned short* blockIndices = (unsigned short*)indicesAndData;

    for (int x = 0; x < 16; x += 4)
        for (int z = 0; z < 16; z += 4) {
            getBlock(&block, x, y, z);
            uint64_t* comp = (uint64_t*)&blockIndices[block];
            
            
            if ((*comp) != 0x0007000700070007L) return false;
        }
    return true;
}

bool CompressedTileStorage::isSameAs(CompressedTileStorage* other) {
    std::lock_guard<std::recursive_mutex> lock(cs_write);
    if (allocatedSize != other->allocatedSize) {
        return false;
    }

    
    
    int quickCount = allocatedSize / 64;
    int64_t* pOld = (int64_t*)indicesAndData;
    int64_t* pNew = (int64_t*)other->indicesAndData;
    for (int i = 0; i < quickCount; i++) {
        int64_t d0 = pOld[0] ^ pNew[0];
        int64_t d1 = pOld[1] ^ pNew[1];
        int64_t d2 = pOld[2] ^ pNew[2];
        int64_t d3 = pOld[3] ^ pNew[3];
        int64_t d4 = pOld[4] ^ pNew[4];
        int64_t d5 = pOld[5] ^ pNew[5];
        int64_t d6 = pOld[6] ^ pNew[6];
        int64_t d7 = pOld[7] ^ pNew[7];
        d0 |= d1;
        d2 |= d3;
        d4 |= d5;
        d6 |= d7;
        d0 |= d2;
        d4 |= d6;
        if (d0 | d4) {
            return false;
        }
        pOld += 8;
        pNew += 8;
    }

    
    unsigned char* pucOld = (unsigned char*)pOld;
    unsigned char* pucNew = (unsigned char*)pNew;
    for (int i = 0; i < allocatedSize - (quickCount * 64); i++) {
        if (*pucOld++ != *pucNew++) {
            return false;
        }
    }

    return true;
}

CompressedTileStorage::~CompressedTileStorage() {
    if (indicesAndData) free(indicesAndData);
}



inline int CompressedTileStorage::getIndex(int block, int tile) {
    
    
    
    
    
    
    

    int index = ((block & 0x180) << 6) | ((block & 0x060) << 4) |
                ((block & 0x01f) << 2);
    index |= ((tile & 0x30) << 7) | ((tile & 0x0c) << 5) | (tile & 0x03);

    return index;
}




















inline void CompressedTileStorage::getBlockAndTile(int* block, int* tile, int x,
                                                   int y, int z) {
    *block = ((x & 0x0c) << 5) | ((z & 0x0c) << 3) | (y >> 2);
    *tile = ((x & 0x03) << 4) | ((z & 0x03) << 2) | (y & 0x03);
}

inline void CompressedTileStorage::getBlock(int* block, int x, int y, int z) {
    *block = ((x & 0x0c) << 5) | ((z & 0x0c) << 3) | (y >> 2);
}


void CompressedTileStorage::setData(std::vector<uint8_t>& dataIn,
                                    unsigned int inOffset) {
    unsigned short _blockIndices[512];

    std::lock_guard<std::recursive_mutex> lock(cs_write);
    unsigned char* data = dataIn.data() + inOffset;

    
    
    
    if (allocatedSize == (32768 + 1024)) {
        
        unsigned char* dataOut = indicesAndData + 1024;

        for (int i = 0; i < 512; i++) {
            for (int j = 0; j < 64; j++) {
                *dataOut++ = data[getIndex(i, j)];
            }
        }
        return;
    }

    int offsets[512];
    int memToAlloc = 0;
    
    

    
    for (int i = 0; i < 512; i++) {
        offsets[i] = memToAlloc;
        
        
        
        

        uint64_t usedFlags[4] = {0, 0, 0, 0};
        int64_t i64_1 = 1;  
        for (int j = 0; j < 64; j++)  
                                      
        {
            int tile = data[getIndex(i, j)];

            usedFlags[tile & 3] |= (i64_1 << (tile >> 2));
        }
        int count = 0;
        for (int tile = 0; tile < 256;
             tile++)  
                      
                      
        {
            if (usedFlags[tile & 3] & (i64_1 << (tile >> 2))) {
                count++;
            }
        }
        if (count == 1) {
            _blockIndices[i] = INDEX_TYPE_0_OR_8_BIT | INDEX_TYPE_0_BIT_FLAG;
            
        } else if (count == 2) {
            _blockIndices[i] = INDEX_TYPE_1_BIT;
            memToAlloc += 10;  
                               
        } else if (count <= 4) {
            _blockIndices[i] = INDEX_TYPE_2_BIT;
            memToAlloc += 20;  
                               
        } else if (count <= 16) {
            _blockIndices[i] = INDEX_TYPE_4_BIT;
            memToAlloc += 48;  
                               
        } else {
            _blockIndices[i] = INDEX_TYPE_0_OR_8_BIT;
            memToAlloc =
                (memToAlloc + 3) &
                0xfffc;  
            memToAlloc += 64;
            
        }
    }

    
    
    
    

    memToAlloc += 1024;  
    unsigned char* newIndicesAndData = (unsigned char*)malloc(
        memToAlloc);  
    unsigned char* pucData = newIndicesAndData + 1024;
    unsigned short usDataOffset = 0;
    unsigned short* newIndices = (unsigned short*)newIndicesAndData;

    
    for (int i = 0; i < 512; i++) {
        unsigned short indexTypeNew = _blockIndices[i] & INDEX_TYPE_MASK;
        newIndices[i] = indexTypeNew;

        if (indexTypeNew == INDEX_TYPE_0_OR_8_BIT) {
            if (_blockIndices[i] & INDEX_TYPE_0_BIT_FLAG) {
                newIndices[i] = INDEX_TYPE_0_OR_8_BIT | INDEX_TYPE_0_BIT_FLAG |
                                (((unsigned short)data[getIndex(i, 0)])
                                 << INDEX_TILE_SHIFT);
            } else {
                usDataOffset = (usDataOffset + 3) & 0xfffc;
                for (int j = 0; j < 64;
                     j++)  
                           
                {
                    pucData[usDataOffset + j] = data[getIndex(i, j)];
                }
                newIndices[i] |= (usDataOffset & INDEX_OFFSET_MASK)
                                 << INDEX_OFFSET_SHIFT;
                usDataOffset += 64;
            }
        } else {
            
            unsigned char ucMappings[256] = {0};
            for (int j = 0; j < 256; j++) {
                ucMappings[j] = 255;
            }

            unsigned char* repacked = nullptr;

            int bitspertile = 1 << indexTypeNew;   
                                                   
            int tiletypecount = 1 << bitspertile;  
                                                   
            
            
            int tiledatasize = 8 << indexTypeNew;  
                                                   
            int indexshift = 3 - indexTypeNew;  
                                                
            int indexmask_bits = 7 >> indexTypeNew;  
                                                     
            int indexmask_bytes =
                62 >> indexshift;  
                                   

            unsigned char* tile_types = pucData + usDataOffset;
            repacked = tile_types + tiletypecount;
            memset(tile_types, 255, tiletypecount);
            memset(repacked, 0, tiledatasize);
            newIndices[i] |= (usDataOffset & INDEX_OFFSET_MASK)
                             << INDEX_OFFSET_SHIFT;
            usDataOffset += tiletypecount + tiledatasize;
            int count = 0;
            for (int j = 0; j < 64; j++) {
                int tile = data[getIndex(i, j)];
                if (ucMappings[tile] == 255) {
                    ucMappings[tile] = count;
                    tile_types[count++] = tile;
                }
                int idx = (j >> indexshift) & indexmask_bytes;
                int bit = (j & indexmask_bits) * bitspertile;
                repacked[idx] |= ucMappings[tile] << bit;
            }
        }
    }

    if (indicesAndData) {
        queueForDelete(indicesAndData);
    }
    indicesAndData = newIndicesAndData;
    allocatedSize = memToAlloc;
}

#if defined(PSVITA_PRECOMPUTED_TABLE)




void CompressedTileStorage::getData(std::vector<uint8_t>& retArray,
                                    unsigned int retOffset) {
    unsigned short* blockIndices = (unsigned short*)indicesAndData;
    unsigned char* data = indicesAndData + 1024;

    int k = 0;
    unsigned char* Array = &retArray.data()[retOffset];
    int* Table = CompressedTile_StorageIndexTable;
    for (int i = 0; i < 512; i++) {
        int indexType = blockIndices[i] & INDEX_TYPE_MASK;

        int index =
            ((i & 0x180) << 6) | ((i & 0x060) << 4) | ((i & 0x01f) << 2);
        unsigned char* NewArray = &Array[index];

        if (indexType == INDEX_TYPE_0_OR_8_BIT) {
            if (blockIndices[i] & INDEX_TYPE_0_BIT_FLAG) {
                unsigned char val =
                    (blockIndices[i] >> INDEX_TILE_SHIFT) & INDEX_TILE_MASK;
                for (int j = 0; j < 64; j++) {
                    NewArray[Table[j]] = val;
                }
            } else {
                
                
                unsigned char* packed =
                    data + ((blockIndices[i] >> INDEX_OFFSET_SHIFT) &
                            INDEX_OFFSET_MASK);

                for (int j = 0; j < 64; j++) {
                    NewArray[Table[j]] = packed[j];
                }
            }
        } else {
            

            int bitspertile = 1 << indexType;  
                                               
            int tiletypecount = 1 << bitspertile;  
                                                   
            int tiletypemask =
                tiletypecount -
                1;  
            int indexshift =
                3 -
                indexType;  
            int indexmask_bits =
                7 >>
                indexType;  
            int indexmask_bytes =
                62 >> indexshift;  
                                   

            unsigned char* tile_types =
                data +
                ((blockIndices[i] >> INDEX_OFFSET_SHIFT) & INDEX_OFFSET_MASK);
            unsigned char* packed = tile_types + tiletypecount;

            for (int j = 0; j < 64; j++) {
                int idx = (j >> indexshift) & indexmask_bytes;
                int bit = (j & indexmask_bits) << indexType;
                NewArray[Table[j]] =
                    tile_types[(packed[idx] >> bit) & tiletypemask];
            }
        }
    }
}

#else


void CompressedTileStorage::getData(std::vector<uint8_t>& retArray,
                                    unsigned int retOffset) {
    unsigned short* blockIndices = (unsigned short*)indicesAndData;
    unsigned char* data = indicesAndData + 1024;

    for (int i = 0; i < 512; i++) {
        int indexType = blockIndices[i] & INDEX_TYPE_MASK;
        if (indexType == INDEX_TYPE_0_OR_8_BIT) {
            if (blockIndices[i] & INDEX_TYPE_0_BIT_FLAG) {
                for (int j = 0; j < 64; j++) {
                    retArray[getIndex(i, j) + retOffset] =
                        (blockIndices[i] >> INDEX_TILE_SHIFT) & INDEX_TILE_MASK;
                }
            } else {
                
                
                unsigned char* packed =
                    data + ((blockIndices[i] >> INDEX_OFFSET_SHIFT) &
                            INDEX_OFFSET_MASK);

                for (int j = 0; j < 64; j++) {
                    retArray[getIndex(i, j) + retOffset] = packed[j];
                }
            }
        } else {
            

            int bitspertile = 1 << indexType;  
                                               
            int tiletypecount = 1 << bitspertile;  
                                                   
            int tiletypemask =
                tiletypecount -
                1;  
            int indexshift =
                3 -
                indexType;  
            int indexmask_bits =
                7 >>
                indexType;  
            int indexmask_bytes =
                62 >> indexshift;  
                                   

            unsigned char* tile_types =
                data +
                ((blockIndices[i] >> INDEX_OFFSET_SHIFT) & INDEX_OFFSET_MASK);
            unsigned char* packed = tile_types + tiletypecount;

            for (int j = 0; j < 64; j++) {
                int idx = (j >> indexshift) & indexmask_bytes;
                int bit = (j & indexmask_bits) * bitspertile;
                retArray[getIndex(i, j) + retOffset] =
                    tile_types[(packed[idx] >> bit) & tiletypemask];
            }
        }
    }
}

#endif


int CompressedTileStorage::get(int x, int y, int z) {
    if (!indicesAndData) return 0;

    unsigned short* blockIndices = (unsigned short*)indicesAndData;
    unsigned char* data = indicesAndData + 1024;

    int block, tile;
    getBlockAndTile(&block, &tile, x, y, z);
    int indexType = blockIndices[block] & INDEX_TYPE_MASK;

    if (indexType == INDEX_TYPE_0_OR_8_BIT) {
        if (blockIndices[block] & INDEX_TYPE_0_BIT_FLAG) {
            
            return (blockIndices[block] >> INDEX_TILE_SHIFT) & INDEX_TILE_MASK;
        } else {
            
            
            unsigned char* packed =
                data + ((blockIndices[block] >> INDEX_OFFSET_SHIFT) &
                        INDEX_OFFSET_MASK);
            return packed[tile];
        }
    } else {
        int bitspertile =
            1 << indexType;  
        int tiletypecount = 1 << bitspertile;  
                                               
        int tiletypemask =
            tiletypecount -
            1;  
        int indexshift =
            3 - indexType;  
        int indexmask_bits =
            7 >> indexType;  
        int indexmask_bytes =
            62 >>
            indexshift;  

        unsigned char* tile_types =
            data +
            ((blockIndices[block] >> INDEX_OFFSET_SHIFT) & INDEX_OFFSET_MASK);
        unsigned char* packed = tile_types + tiletypecount;
        int idx = (tile >> indexshift) & indexmask_bytes;
        int bit = (tile & indexmask_bits) * bitspertile;
        return tile_types[(packed[idx] >> bit) & tiletypemask];
    }
    return 0;
}


void CompressedTileStorage::set(int x, int y, int z, int val) {
    std::lock_guard<std::recursive_mutex> lock(cs_write);
    assert(val != 255);
    int block, tile;
    getBlockAndTile(&block, &tile, x, y, z);

    
    
    
    for (int pass = 0; pass < 2; pass++) {
        unsigned short* blockIndices = (unsigned short*)indicesAndData;
        unsigned char* data = indicesAndData + 1024;

        int indexType = blockIndices[block] & INDEX_TYPE_MASK;

        if (indexType == INDEX_TYPE_0_OR_8_BIT) {
            if (blockIndices[block] & INDEX_TYPE_0_BIT_FLAG) {
                
                
                if (val == ((blockIndices[block] >> INDEX_TILE_SHIFT) &
                            INDEX_TILE_MASK)) {
                    return;
                }
            } else {
                
                unsigned char* packed =
                    data + ((blockIndices[block] >> INDEX_OFFSET_SHIFT) &
                            INDEX_OFFSET_MASK);
                packed[tile] = val;
                return;
            }
        } else {
            int bitspertile = 1 << indexType;  
                                               
            int tiletypecount = 1 << bitspertile;  
                                                   
            int tiletypemask =
                tiletypecount -
                1;  
            int indexshift =
                3 -
                indexType;  
            int indexmask_bits =
                7 >>
                indexType;  
            int indexmask_bytes =
                62 >> indexshift;  
                                   

            unsigned char* tile_types =
                data + ((blockIndices[block] >> INDEX_OFFSET_SHIFT) &
                        INDEX_OFFSET_MASK);

            for (int i = 0; i < tiletypecount; i++) {
                if ((tile_types[i] == val) || (tile_types[i] == 255)) {
                    tile_types[i] = val;
                    unsigned char* packed = tile_types + tiletypecount;
                    int idx = (tile >> indexshift) & indexmask_bytes;
                    int bit = (tile & indexmask_bits) * bitspertile;
                    packed[idx] &= ~(tiletypemask << bit);
                    packed[idx] |= i << bit;
                    return;
                }
            }
        }
        if (pass == 0) {
            compress(block);
        }
    };
}



int CompressedTileStorage::setDataRegion(std::vector<uint8_t>& dataIn, int x0,
                                         int y0, int z0, int x1, int y1, int z1,
                                         int offset,
                                         tileUpdatedCallback callback,
                                         void* param, int yparam) {
    unsigned char* pucIn = &dataIn.data()[offset];

    if (callback) {
        for (int x = x0; x < x1; x++) {
            for (int z = z0; z < z1; z++) {
                for (int y = y0; y < y1; y++) {
                    if (get(x, y, z) != *pucIn) {
                        set(x, y, z, *pucIn);
                        callback(x, y, z, param, yparam);
                    }
                    pucIn++;
                }
            }
        }
    } else {
        for (int x = x0; x < x1; x++) {
            for (int z = z0; z < z1; z++) {
                for (int y = y0; y < y1; y++) {
                    set(x, y, z, *pucIn++);
                }
            }
        }
    }
    ptrdiff_t count = pucIn - &dataIn.data()[offset];

    return (int)count;
}


bool CompressedTileStorage::testSetDataRegion(std::vector<uint8_t>& dataIn,
                                              int x0, int y0, int z0, int x1,
                                              int y1, int z1, int offset) {
    unsigned char* pucIn = &dataIn.data()[offset];
    for (int x = x0; x < x1; x++) {
        for (int z = z0; z < z1; z++) {
            for (int y = y0; y < y1; y++) {
                if (get(x, y, z) != *pucIn++) {
                    return true;
                }
            }
        }
    }
    return false;
}



int CompressedTileStorage::getDataRegion(std::vector<uint8_t>& dataInOut,
                                         int x0, int y0, int z0, int x1, int y1,
                                         int z1, int offset) {
    unsigned char* pucOut = &dataInOut.data()[offset];
    for (int x = x0; x < x1; x++) {
        for (int z = z0; z < z1; z++) {
            for (int y = y0; y < y1; y++) {
                *pucOut++ = get(x, y, z);
            }
        }
    }
    ptrdiff_t count = pucOut - &dataInOut.data()[offset];

    return (int)count;
}

void CompressedTileStorage::staticCtor() {
    for (int i = 0; i < 3; i++) {
        deleteQueue[i].Initialize();
    }
}

void CompressedTileStorage::queueForDelete(unsigned char* data) {
    
    
    if (data) {
        deleteQueue[deleteQueueIndex].Push(data);
    }
}

void CompressedTileStorage::tick() {
    
    
    
    int freeIndex = (deleteQueueIndex + 1) % 3;

    
    
    unsigned char* toFree = nullptr;
    do {
        toFree = deleteQueue[freeIndex].Pop();
        
        if (toFree) free(toFree);
    } while (toFree);

    deleteQueueIndex = (deleteQueueIndex + 1) % 3;
}






void CompressedTileStorage::compress(int upgradeBlock ) {
    unsigned char tempdata[64];
    unsigned short _blockIndices[512];

    
    if ((allocatedSize == 1024) && (upgradeBlock == -1)) return;

    bool needsCompressed =
        (upgradeBlock > -1);  
                              

    std::lock_guard<std::recursive_mutex> lock(cs_write);

    unsigned short* blockIndices = (unsigned short*)indicesAndData;
    unsigned char* data = indicesAndData + 1024;

    int memToAlloc = 0;
    for (int i = 0; i < 512; i++) {
        unsigned short indexType = blockIndices[i] & INDEX_TYPE_MASK;

        unsigned char* unpacked_data = nullptr;
        unsigned char* packed_data;

        
        
        
        
        
        if (upgradeBlock == -1) {
            if (indexType == INDEX_TYPE_0_OR_8_BIT) {
                
                
                
                if ((blockIndices[i] & INDEX_TYPE_0_BIT_FLAG) == 0) {
                    unpacked_data =
                        data + ((blockIndices[i] >> INDEX_OFFSET_SHIFT) &
                                INDEX_OFFSET_MASK);
                }
            } else {
                int bitspertile = 1 << indexType;  
                                                   
                int tiletypecount = 1 << bitspertile;  
                int tiletypemask = tiletypecount - 1;  
                int indexshift = 3 - indexType;  
                                                 
                int indexmask_bits =
                    7 >> indexType;  
                                     
                int indexmask_bytes =
                    62 >> indexshift;  
                                       

                unpacked_data = tempdata;
                packed_data = data +
                              ((blockIndices[i] >> INDEX_OFFSET_SHIFT) &
                               INDEX_OFFSET_MASK) +
                              tiletypecount;

                for (int j = 0; j < 64; j++) {
                    int idx = (j >> indexshift) & indexmask_bytes;
                    int bit = (j & indexmask_bits) * bitspertile;

                    unpacked_data[j] =
                        (packed_data[idx] >> bit) &
                        tiletypemask;  
                                       
                }
            }

            if (unpacked_data) {
                
                
                
                

                uint64_t usedFlags[4] = {0, 0, 0, 0};
                int64_t i64_1 =
                    1;  
                for (int j = 0; j < 64; j++)  
                                              
                {
                    int tiletype = unpacked_data[j];
                    usedFlags[tiletype & 3] |= (i64_1 << (tiletype >> 2));
                }
                int count = 0;
                for (int tiletype = 0; tiletype < 256;
                     tiletype++)  
                                  
                                  
                {
                    if (usedFlags[tiletype & 3] & (i64_1 << (tiletype >> 2))) {
                        count++;
                    }
                }

                if (count == 1) {
                    _blockIndices[i] =
                        INDEX_TYPE_0_OR_8_BIT | INDEX_TYPE_0_BIT_FLAG;

                    
                    
                    
                    
                    needsCompressed = true;
                } else if (count == 2) {
                    _blockIndices[i] = INDEX_TYPE_1_BIT;
                    if (indexType != INDEX_TYPE_1_BIT) needsCompressed = true;
                    memToAlloc += 10;  
                } else if (count <= 4) {
                    _blockIndices[i] = INDEX_TYPE_2_BIT;
                    if (indexType != INDEX_TYPE_2_BIT) needsCompressed = true;
                    memToAlloc += 20;  
                } else if (count <= 16) {
                    _blockIndices[i] = INDEX_TYPE_4_BIT;
                    if (indexType != INDEX_TYPE_4_BIT) needsCompressed = true;
                    memToAlloc += 48;  
                } else {
                    _blockIndices[i] = INDEX_TYPE_0_OR_8_BIT;
                    memToAlloc =
                        (memToAlloc + 3) & 0xfffc;  
                                                    
                    memToAlloc += 64;
                }
            } else {
                
                
                _blockIndices[i] = blockIndices[i];
            }
        } else {
            if (i == upgradeBlock) {
                
                
                
                
                
                _blockIndices[i] =
                    ((blockIndices[i] & INDEX_TYPE_MASK) + 1) & INDEX_TYPE_MASK;
            } else {
                
                _blockIndices[i] = blockIndices[i] & INDEX_TYPE_MASK;
                if (_blockIndices[i] == INDEX_TYPE_0_OR_8_BIT) {
                    _blockIndices[i] |=
                        (blockIndices[i] & INDEX_TYPE_0_BIT_FLAG);
                }
            }
            switch (_blockIndices[i]) {
                case INDEX_TYPE_1_BIT:
                    memToAlloc += 10;
                    break;
                case INDEX_TYPE_2_BIT:
                    memToAlloc += 20;
                    break;
                case INDEX_TYPE_4_BIT:
                    memToAlloc += 48;
                    break;
                case INDEX_TYPE_0_OR_8_BIT:
                    memToAlloc =
                        (memToAlloc + 3) & 0xfffc;  
                                                    
                    memToAlloc += 64;
                    break;
                    
                    
            }
        }
    }

    
    if (needsCompressed) {
        memToAlloc += 1024;  
        unsigned char* newIndicesAndData = (unsigned char*)malloc(
            memToAlloc);  
        if (newIndicesAndData == nullptr) {
            uint32_t lastError = GetLastError();
            MEMORYSTATUS memStatus;
            GlobalMemoryStatus(&memStatus);
            __debugbreak();
        }
        unsigned char* pucData = newIndicesAndData + 1024;
        unsigned short usDataOffset = 0;
        unsigned short* newIndices = (unsigned short*)newIndicesAndData;

        
        for (int i = 0; i < 512; i++) {
            unsigned short indexTypeNew = _blockIndices[i] & INDEX_TYPE_MASK;
            unsigned short indexTypeOld = blockIndices[i] & INDEX_TYPE_MASK;
            newIndices[i] = indexTypeNew;

            
            bool done = false;
            if (indexTypeOld == indexTypeNew) {
                unsigned char* packed_data;
                if (indexTypeOld == INDEX_TYPE_0_OR_8_BIT) {
                    if ((blockIndices[i] & INDEX_TYPE_0_BIT_FLAG) ==
                        (_blockIndices[i] & INDEX_TYPE_0_BIT_FLAG)) {
                        if (blockIndices[i] & INDEX_TYPE_0_BIT_FLAG) {
                            newIndices[i] = blockIndices[i];
                        } else {
                            packed_data = data + ((blockIndices[i] >>
                                                   INDEX_OFFSET_SHIFT) &
                                                  INDEX_OFFSET_MASK);
                            usDataOffset = (usDataOffset + 3) & 0xfffc;
                            memcpy(pucData + usDataOffset, packed_data, 64);
                            newIndices[i] |= (usDataOffset & INDEX_OFFSET_MASK)
                                             << INDEX_OFFSET_SHIFT;
                            usDataOffset += 64;
                        }
                        done = true;
                    }
                } else {
                    packed_data =
                        data + ((blockIndices[i] >> INDEX_OFFSET_SHIFT) &
                                INDEX_OFFSET_MASK);

                    int dataSize = 8 << indexTypeOld;  
                                                       
                    dataSize += 1
                                << (1 << indexTypeOld);  
                                                         
                    newIndices[i] |= (usDataOffset & INDEX_OFFSET_MASK)
                                     << INDEX_OFFSET_SHIFT;
                    memcpy(pucData + usDataOffset, packed_data, dataSize);
                    usDataOffset += dataSize;
                    done = true;
                }
            }

            
            
            if (!done) {
                unsigned char* unpacked_data = nullptr;
                unsigned char* tile_types = nullptr;
                unsigned char* packed_data = nullptr;
                if (indexTypeOld == INDEX_TYPE_0_OR_8_BIT) {
                    if (blockIndices[i] & INDEX_TYPE_0_BIT_FLAG) {
                        unpacked_data = tempdata;
                        int value = (blockIndices[i] >> INDEX_TILE_SHIFT) &
                                    INDEX_TILE_MASK;
                        memset(tempdata, value, 64);
                    } else {
                        unpacked_data =
                            data + ((blockIndices[i] >> INDEX_OFFSET_SHIFT) &
                                    INDEX_OFFSET_MASK);
                    }
                } else {
                    int bitspertile =
                        1 << indexTypeOld;  
                                            
                    int tiletypecount = 1 << bitspertile;  
                    int tiletypemask = tiletypecount - 1;  
                    int indexshift =
                        3 - indexTypeOld;  
                                           
                    int indexmask_bits =
                        7 >> indexTypeOld;  
                                            
                    int indexmask_bytes =
                        62 >> indexshift;  
                                           

                    unpacked_data = tempdata;
                    tile_types =
                        data + ((blockIndices[i] >> INDEX_OFFSET_SHIFT) &
                                INDEX_OFFSET_MASK);
                    packed_data = tile_types + tiletypecount;
                    for (int j = 0; j < 64; j++) {
                        int idx = (j >> indexshift) & indexmask_bytes;
                        int bit = (j & indexmask_bits) * bitspertile;

                        unpacked_data[j] =
                            tile_types[(packed_data[idx] >> bit) &
                                       tiletypemask];
                    }
                }

                
                unsigned char ucMappings[256] = {0};
                for (int j = 0; j < 256; j++) {
                    ucMappings[j] = 255;
                }

                unsigned char* repacked = nullptr;

                if (indexTypeNew == INDEX_TYPE_0_OR_8_BIT) {
                    if (_blockIndices[i] & INDEX_TYPE_0_BIT_FLAG) {
                        newIndices[i] = INDEX_TYPE_0_OR_8_BIT |
                                        INDEX_TYPE_0_BIT_FLAG |
                                        (((unsigned short)unpacked_data[0])
                                         << INDEX_TILE_SHIFT);
                    } else {
                        usDataOffset =
                            (usDataOffset + 3) &
                            0xfffc;  
                        memcpy(pucData + usDataOffset, unpacked_data, 64);
                        newIndices[i] |= (usDataOffset & INDEX_OFFSET_MASK)
                                         << INDEX_OFFSET_SHIFT;
                        usDataOffset += 64;
                    }
                } else {
                    int bitspertile =
                        1 << indexTypeNew;  
                                            
                    int tiletypecount =
                        1 << bitspertile;  
                                           
                    int tiletypemask =
                        tiletypecount -
                        1;  
                    int tiledatasize =
                        8 << indexTypeNew;  
                                            
                    int indexshift =
                        3 - indexTypeNew;  
                                           
                    int indexmask_bits =
                        7 >> indexTypeNew;  
                                            
                    int indexmask_bytes =
                        62 >> indexshift;  
                                           

                    tile_types = pucData + usDataOffset;
                    repacked = tile_types + tiletypecount;
                    memset(tile_types, 255, tiletypecount);
                    memset(repacked, 0, tiledatasize);
                    newIndices[i] |= (usDataOffset & INDEX_OFFSET_MASK)
                                     << INDEX_OFFSET_SHIFT;
                    usDataOffset += tiletypecount + tiledatasize;
                    int count = 0;
                    for (int j = 0; j < 64; j++) {
                        int tile = unpacked_data[j];
                        if (ucMappings[tile] == 255) {
                            ucMappings[tile] = count;
                            tile_types[count++] = tile;
                        }
                        int idx = (j >> indexshift) & indexmask_bytes;
                        int bit = (j & indexmask_bits) * bitspertile;
                        repacked[idx] |= ucMappings[tile] << bit;
                    }
                }
            }
        }

        queueForDelete(indicesAndData);
        indicesAndData = newIndicesAndData;
        allocatedSize = memToAlloc;
    }
}

int CompressedTileStorage::getAllocatedSize(int* count0, int* count1,
                                            int* count2, int* count4,
                                            int* count8) {
    *count0 = 0;
    *count1 = 0;
    *count2 = 0;
    *count4 = 0;
    *count8 = 0;

    unsigned short* blockIndices = (unsigned short*)indicesAndData;
    for (int i = 0; i < 512; i++) {
        unsigned short idxType = blockIndices[i] & INDEX_TYPE_MASK;
        if (idxType == INDEX_TYPE_1_BIT) {
            (*count1)++;
        } else if (idxType == INDEX_TYPE_2_BIT) {
            (*count2)++;
        } else if (idxType == INDEX_TYPE_4_BIT) {
            (*count4)++;
        } else if (idxType == INDEX_TYPE_0_OR_8_BIT) {
            if (blockIndices[i] & INDEX_TYPE_0_BIT_FLAG) {
                (*count0)++;
            } else {
                (*count8)++;
            }
        }
    }
    return allocatedSize;
}

int CompressedTileStorage::getHighestNonEmptyY() {
    unsigned short* blockIndices = (unsigned short*)indicesAndData;
    unsigned int highestYBlock = 0;
    bool found = false;

    
    for (int yBlock = 31; yBlock >= 0; --yBlock) {
        
        for (unsigned int xzBlock = 0; xzBlock < 16; ++xzBlock) {
            
            int index = yBlock + (xzBlock * 32);

            int indexType = blockIndices[index] & INDEX_TYPE_MASK;
            if (indexType == INDEX_TYPE_0_OR_8_BIT &&
                blockIndices[index] & INDEX_TYPE_0_BIT_FLAG) {
                int val =
                    (blockIndices[index] >> INDEX_TILE_SHIFT) & INDEX_TILE_MASK;
                if (val != 0) {
                    highestYBlock = yBlock;
                    found = true;
                    break;
                }
            } else {
                highestYBlock = yBlock;
                found = true;
                break;
            }
        }

        if (found) break;
    }

    int highestNonEmptyY = -1;
    if (found) {
        
        
        highestNonEmptyY = (highestYBlock * 4) + 4;
    }
    return highestNonEmptyY;
}

void CompressedTileStorage::write(DataOutputStream* dos) {
    dos->writeInt(allocatedSize);
    if (indicesAndData) {
        if (std::endian::native == std::endian::big) {
            
            
            std::vector<uint8_t> indicesCopy(1024);
            memcpy(indicesCopy.data(), indicesAndData, 1024);
            reverseIndices(indicesCopy.data());
            dos->write(indicesCopy);

            
            if (allocatedSize > 1024) {
                std::vector<uint8_t> dataWrapper(
                    indicesAndData + 1024, indicesAndData + allocatedSize);
                dos->write(dataWrapper);
            }
        } else {
            std::vector<uint8_t> wrapper(indicesAndData,
                                         indicesAndData + allocatedSize);
            dos->write(wrapper);
        }
    }
}

void CompressedTileStorage::read(DataInputStream* dis) {
    allocatedSize = dis->readInt();
    if (allocatedSize > 0) {
        
        
        
        if (indicesAndData) {
            free(indicesAndData);
        }
        indicesAndData = (unsigned char*)malloc(allocatedSize);

        std::vector<uint8_t> wrapper(allocatedSize);
        dis->readFully(wrapper);
        memcpy(indicesAndData, wrapper.data(), allocatedSize);
        if (std::endian::native == std::endian::big) {
            reverseIndices(indicesAndData);
        }

        compress();
    }
}

void CompressedTileStorage::reverseIndices(unsigned char* indices) {
    unsigned short* blockIndices = (unsigned short*)indices;
    for (int i = 0; i < 512; i++) {
        System::ReverseUSHORT(&blockIndices[i]);
    }
}
