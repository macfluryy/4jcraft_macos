#include "SparseLightStorage.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "app/mac/Stubs/winapi_stubs.h"
#include "app/include/NetTypes.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"



int SparseLightStorage::deleteQueueIndex;
XLockFreeStack<unsigned char> SparseLightStorage::deleteQueue[3];

void SparseLightStorage::staticCtor() {
    for (int i = 0; i < 3; i++) {
        deleteQueue[i].Initialize();
    }
}

















SparseLightStorage::SparseLightStorage(bool sky) {
    
    
    
    
    
    unsigned char* planeIndices = (unsigned char*)malloc(128 * 128);
    unsigned char* data = planeIndices + 128;
    planeIndices[127] = sky ? ALL_15_INDEX : ALL_0_INDEX;
    for (int i = 0; i < 127; i++) {
        planeIndices[i] = i;
    }
    memset(data, 0, 128 * 127);

    
    

    dataAndCount =
        0x007F000000000000L | (((int64_t)planeIndices) & 0x0000ffffffffffffL);

#if defined(LIGHT_COMPRESSION_STATS)
    count = 127;
#endif
}

SparseLightStorage::SparseLightStorage(bool sky, bool isUpper) {
    
    
    
    
    
    unsigned char* planeIndices = (unsigned char*)malloc(128);
    for (int i = 0; i < 128; i++) {
        planeIndices[i] = sky ? ALL_15_INDEX : ALL_0_INDEX;
    }

    
    

    dataAndCount =
        0x0000000000000000L | (((int64_t)planeIndices) & 0x0000ffffffffffffL);

#if defined(LIGHT_COMPRESSION_STATS)
    count = 0;
#endif
}

SparseLightStorage::~SparseLightStorage() {
    unsigned char* indicesAndData =
        (unsigned char*)(dataAndCount & 0x0000ffffffffffff);
    
    

    {
        free(indicesAndData);
    }
    
}

SparseLightStorage::SparseLightStorage(SparseLightStorage* copyFrom) {
    
    int64_t sourceDataAndCount = copyFrom->dataAndCount;
    unsigned char* sourceIndicesAndData =
        (unsigned char*)(sourceDataAndCount & 0x0000ffffffffffff);
    int sourceCount = (sourceDataAndCount >> 48) & 0xffff;

    
    
    unsigned char* destIndicesAndData =
        (unsigned char*)malloc(sourceCount * 128 + 128);

    
    
    
    

    dataAndCount = (sourceDataAndCount & 0xffff000000000000L) |
                   (((int64_t)destIndicesAndData) & 0x0000ffffffffffffL);

    memcpy(destIndicesAndData, sourceIndicesAndData, sourceCount * 128 + 128);

#if defined(LIGHT_COMPRESSION_STATS)
    count = sourceCount;
#endif
}



void SparseLightStorage::setData(std::vector<uint8_t>& dataIn,
                                 unsigned int inOffset) {
    
    
    
    
    
    

    
    
    int allocatedPlaneCount = 0;
    unsigned char _planeIndices[128];

    for (int y = 0; y < 128; y++) {
        bool all0 = true;
        bool all15 = true;

        for (int xz = 0; xz < 256;
             xz++)  
        {
            int pos = (xz << 7) | y;
            int slot = pos >> 1;
            int part = pos & 1;
            unsigned char value = (dataIn[slot + inOffset] >> (part * 4)) & 15;
            if (value != 0) all0 = false;
            if (value != 15) all15 = false;
        }
        if (all0) {
            _planeIndices[y] = ALL_0_INDEX;
        } else if (all15) {
            _planeIndices[y] = ALL_15_INDEX;
        } else {
            _planeIndices[y] = allocatedPlaneCount++;
        }
    }

    
    unsigned char* planeIndices =
        (unsigned char*)malloc(128 * allocatedPlaneCount + 128);
    unsigned char* data = planeIndices + 128;
    memcpy(planeIndices, _planeIndices, 128);

    
    
    unsigned char* pucOut = data;
    for (int y = 0; y < 128; y++) {
        
        
        
        if (planeIndices[y] < 128) {
            int part = y & 1;
            
            unsigned char* pucIn = &dataIn[(y >> 1) + inOffset];

            for (int xz = 0; xz < 128;
                 xz++)  
                        
            {
                *pucOut = ((*pucIn) >> (part * 4)) & 15;
                pucIn += 64;

                *pucOut |= (((*pucIn) >> (part * 4)) & 15) << 4;
                pucIn += 64;
                pucOut++;
            }
        }
    }

    

    int64_t newDataAndCount = ((int64_t)planeIndices) & 0x0000ffffffffffffL;

    newDataAndCount |= ((int64_t)allocatedPlaneCount) << 48;

    updateDataAndCount(newDataAndCount);
}



void SparseLightStorage::getData(std::vector<uint8_t>& retArray,
                                 unsigned int retOffset) {
    memset(retArray.data() + retOffset, 0, 16384);
    unsigned char *planeIndices, *data;
    getPlaneIndicesAndData(&planeIndices, &data);

    
    
    
    
    
    

    for (int y = 0; y < 128; y++) {
        if (planeIndices[y] == ALL_0_INDEX) {
            
            
        } else if (planeIndices[y] == ALL_15_INDEX) {
            int part = y & 1;
            unsigned char value = 15 << (part * 4);
            unsigned char* pucOut = &retArray.data()[(y >> 1) + retOffset];
            for (int xz = 0; xz < 256; xz++) {
                *pucOut |= value;
                pucOut += 64;
            }
        } else {
            int part = y & 1;
            int shift = 4 * part;
            unsigned char* pucOut = &retArray.data()[(y >> 1) + retOffset];
            unsigned char* pucIn = &data[planeIndices[y] * 128];
            for (int xz = 0; xz < 128;
                 xz++)  
                        
                        
            {
                unsigned char value = (*pucIn) & 15;
                *pucOut |= (value << shift);
                pucOut += 64;

                value = ((*pucIn) >> 4) & 15;
                *pucOut |= (value << shift);
                pucOut += 64;

                pucIn++;
            }
        }
    }
}


int SparseLightStorage::get(int x, int y, int z) {
    unsigned char *planeIndices, *data;
    getPlaneIndicesAndData(&planeIndices, &data);

    if (planeIndices[y] == ALL_0_INDEX) {
        return 0;
    } else if (planeIndices[y] == ALL_15_INDEX) {
        return 15;
    } else {
        int planeIndex = x * 16 + z;  
        int byteIndex =
            planeIndex /
            2;  
        int shift = (planeIndex & 1) * 4;  
        int retval = (data[planeIndices[y] * 128 + byteIndex] >> shift) & 15;

        return retval;
    }
}


void SparseLightStorage::set(int x, int y, int z, int val) {
    unsigned char *planeIndices, *data;
    getPlaneIndicesAndData(&planeIndices, &data);

    
    
    if (planeIndices[y] >= ALL_0_INDEX) {
        
        
        if ((val == 0) && (planeIndices[y] == ALL_0_INDEX)) {
            return;
        }
        if ((val == 15) && (planeIndices[y] == ALL_15_INDEX)) {
            return;
        }

        
        addNewPlane(y);

        
        getPlaneIndicesAndData(&planeIndices, &data);
    }

    
    

    int planeIndex = x * 16 + z;  
    int byteIndex = planeIndex /
                    2;  
    int shift = (planeIndex & 1) * 4;  
    int mask = 0xf0 >> shift;

    int idx = planeIndices[y] * 128 + byteIndex;
    data[idx] = (data[idx] & mask) | (val << shift);
}

void SparseLightStorage::setAllBright() {
    unsigned char* planeIndices = (unsigned char*)malloc(128);
    for (int i = 0; i < 128; i++) {
        planeIndices[i] = ALL_15_INDEX;
    }
    
    

    int64_t newDataAndCount = ((int64_t)planeIndices) & 0x0000ffffffffffffL;

    updateDataAndCount(newDataAndCount);
}









int SparseLightStorage::setDataRegion(std::vector<uint8_t>& dataIn, int x0,
                                      int y0, int z0, int x1, int y1, int z1,
                                      int offset) {
    
    
    unsigned char* pucIn = &dataIn.data()[offset];
    for (int x = x0; x < x1; x++) {
        for (int z = z0; z < z1; z++) {
            
            
            int yy0 = y0 & 0xfffffffe;
            int len = (y1 - y0) / 2;
            for (int i = 0; i < len; i++) {
                int y = yy0 + (i * 2);

                set(x, y, z, (*pucIn) & 15);
                set(x, y + 1, z, ((*pucIn) >> 4) & 15);
                pucIn++;
            }
        }
    }
    ptrdiff_t count = pucIn - &dataIn.data()[offset];

    return (int)count;
}








int SparseLightStorage::getDataRegion(std::vector<uint8_t>& dataInOut, int x0,
                                      int y0, int z0, int x1, int y1, int z1,
                                      int offset) {
    unsigned char* pucOut = &dataInOut.data()[offset];
    for (int x = x0; x < x1; x++) {
        for (int z = z0; z < z1; z++) {
            
            
            int yy0 = y0 & 0xfffffffe;
            int len = (y1 - y0) / 2;
            for (int i = 0; i < len; i++) {
                int y = yy0 + (i * 2);

                *pucOut = get(x, y, z);
                *pucOut |= get(x, y + 1, z) << 4;
                pucOut++;
            }
        }
    }
    ptrdiff_t count = pucOut - &dataInOut.data()[offset];

    return (int)count;
}

void SparseLightStorage::addNewPlane(int y) {
    bool success = false;
    do {
        
        int64_t lastDataAndCount = dataAndCount;

        
        int lastLinesUsed = (int)((lastDataAndCount >> 48) & 0xffff);
        unsigned char* lastDataPointer =
            (unsigned char*)(lastDataAndCount & 0x0000ffffffffffff);

        
        unsigned char planeIndex = lastDataPointer[y];
        int prefill = 0;
        if (planeIndex < ALL_0_INDEX)
            return;  
        else if (planeIndex == ALL_15_INDEX)
            prefill = 255;

        int linesUsed = lastLinesUsed + 1;

        
        
        unsigned char* dataPointer =
            (unsigned char*)malloc(linesUsed * 128 + 128);
        memcpy(dataPointer, lastDataPointer, 128 * lastLinesUsed + 128);
        memset(dataPointer + (128 * lastLinesUsed) + 128, prefill, 128);
        dataPointer[y] = lastLinesUsed;

        

        int64_t newDataAndCount = ((int64_t)dataPointer) & 0x0000ffffffffffffL;

        newDataAndCount |= ((int64_t)linesUsed) << 48;

        
        
        
        
        int64_t lastDataAndCount2 = InterlockedCompareExchangeRelease64(
            (int64_t*)&dataAndCount, newDataAndCount, lastDataAndCount);

        if (lastDataAndCount2 == lastDataAndCount) {
            success = true;
            
            queueForDelete(lastDataPointer);

#if defined(LIGHT_COMPRESSION_STATS)
            count = linesUsed;
#endif
        } else {
            
            
            queueForDelete(dataPointer);
            
            
        }
    } while (!success);
}

void SparseLightStorage::getPlaneIndicesAndData(unsigned char** planeIndices,
                                                unsigned char** data) {
    unsigned char* indicesAndData =
        (unsigned char*)(dataAndCount & 0x0000ffffffffffff);

    *planeIndices = indicesAndData;
    *data = indicesAndData + 128;
}

void SparseLightStorage::queueForDelete(unsigned char* data) {
    
    
    deleteQueue[deleteQueueIndex].Push(data);
}

void SparseLightStorage::tick() {
    
    
    
    int freeIndex = (deleteQueueIndex + 1) % 3;

    
    
    unsigned char* toFree = nullptr;
    do {
        toFree = deleteQueue[freeIndex].Pop();
        
        
        
        {
            free(toFree);
        }
    } while (toFree);

    deleteQueueIndex = (deleteQueueIndex + 1) % 3;
}



void SparseLightStorage::updateDataAndCount(int64_t newDataAndCount) {
    
    
    
    bool success = false;
    do {
        int64_t lastDataAndCount = dataAndCount;
        unsigned char* lastDataPointer =
            (unsigned char*)(lastDataAndCount & 0x0000ffffffffffff);

        
        
        
        
        int64_t lastDataAndCount2 = InterlockedCompareExchangeRelease64(
            (int64_t*)&dataAndCount, newDataAndCount, lastDataAndCount);

        if (lastDataAndCount2 == lastDataAndCount) {
            success = true;
            
            
            
            queueForDelete(lastDataPointer);
        }
    } while (!success);

#if defined(LIGHT_COMPRESSION_STATS)
    count = (newDataAndCount >> 48) & 0xffff;
#endif
}




int SparseLightStorage::compress() {
    unsigned char _planeIndices[128];
    bool needsCompressed = false;

    int64_t lastDataAndCount = dataAndCount;

    unsigned char* planeIndices =
        (unsigned char*)(lastDataAndCount & 0x0000ffffffffffff);
    unsigned char* data = planeIndices + 128;

    int planesToAlloc = 0;
    for (int i = 0; i < 128; i++) {
        if (planeIndices[i] == ALL_0_INDEX) {
            _planeIndices[i] = ALL_0_INDEX;
        } else if (planeIndices[i] == ALL_15_INDEX) {
            _planeIndices[i] = ALL_15_INDEX;
        } else {
            unsigned char* pucData = &data[128 * planeIndices[i]];
            bool all0 = true;
            bool all15 = true;
            for (int j = 0; j < 128; j++)  
            {
                if (*pucData != 0) all0 = false;
                if (*pucData != 255) all15 = false;
                pucData++;
            }
            if (all0) {
                _planeIndices[i] = ALL_0_INDEX;
                needsCompressed = true;
            } else if (all15) {
                _planeIndices[i] = ALL_15_INDEX;
                needsCompressed = true;
            } else {
                _planeIndices[i] = planesToAlloc++;
            }
        }
    }

    if (needsCompressed) {
        unsigned char* newIndicesAndData =
            (unsigned char*)malloc(128 + 128 * planesToAlloc);
        unsigned char* pucData = newIndicesAndData + 128;
        memcpy(newIndicesAndData, _planeIndices, 128);

        for (int i = 0; i < 128; i++) {
            if (newIndicesAndData[i] < ALL_0_INDEX) {
                memcpy(pucData, &data[128 * planeIndices[i]], 128);
                pucData += 128;
            }
        }

        

        int64_t newDataAndCount =
            ((int64_t)newIndicesAndData) & 0x0000ffffffffffffL;

        newDataAndCount |= ((int64_t)planesToAlloc) << 48;

        
        
        
        
        int64_t lastDataAndCount2 = InterlockedCompareExchangeRelease64(
            (int64_t*)&dataAndCount, newDataAndCount, lastDataAndCount);

        if (lastDataAndCount2 != lastDataAndCount) {
            
            
            
            
            queueForDelete(newIndicesAndData);
        } else {
            
            queueForDelete(planeIndices);


#if defined(LIGHT_COMPRESSION_STATS)
            count = planesToAlloc;
#endif
        }

        return planesToAlloc;
    } else {
        return (int)((lastDataAndCount >> 48) & 0xffff);
    }
}

bool SparseLightStorage::isCompressed() {
    int count = (dataAndCount >> 48) & 0xffff;
    return (count < 127);
}

void SparseLightStorage::write(DataOutputStream* dos) {
    int count = (dataAndCount >> 48) & 0xffff;
    dos->writeInt(count);
    unsigned char* dataPointer =
        (unsigned char*)(dataAndCount & 0x0000ffffffffffff);
    std::vector<uint8_t> wrapper(dataPointer, dataPointer + count * 128 + 128);
    dos->write(wrapper);
}

void SparseLightStorage::read(DataInputStream* dis) {
    int count = dis->readInt();
    unsigned char* dataPointer = (unsigned char*)malloc(count * 128 + 128);
    std::vector<uint8_t> wrapper(count * 128 + 128);
    dis->readFully(wrapper);
    memcpy(dataPointer, wrapper.data(), count * 128 + 128);

    int64_t newDataAndCount = ((int64_t)dataPointer) & 0x0000ffffffffffffL;

    newDataAndCount |= ((int64_t)count) << 48;

    updateDataAndCount(newDataAndCount);
}
