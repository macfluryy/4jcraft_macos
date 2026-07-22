#include "SparseDataStorage.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "app/mac/Stubs/winapi_stubs.h"
#include "app/include/NetTypes.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"



int SparseDataStorage::deleteQueueIndex;
XLockFreeStack<unsigned char> SparseDataStorage::deleteQueue[3];

void SparseDataStorage::staticCtor() {
    for (int i = 0; i < 3; i++) {
        deleteQueue[i].Initialize();
    }
}
















SparseDataStorage::SparseDataStorage() {
    
    
    
    
    
    unsigned char* planeIndices = (unsigned char*)malloc(128 * 128);
    unsigned char* data = planeIndices + 128;
    planeIndices[0] = ALL_0_INDEX;
    for (int i = 1; i < 128; i++) {
        planeIndices[i] = i - 1;
    }
    memset(data, 0, 128 * 127);

    
    

    dataAndCount =
        0x007F000000000000L | (((int64_t)planeIndices) & 0x0000ffffffffffffL);

#if defined(DATA_COMPRESSION_STATS)
    count = 128;
#endif
}

SparseDataStorage::SparseDataStorage(bool isUpper) {
    
    
    
    
    
    unsigned char* planeIndices = (unsigned char*)malloc(128);
    for (int i = 0; i < 128; i++) {
        planeIndices[i] = ALL_0_INDEX;
    }

    
    

    dataAndCount =
        0x0000000000000000L | (((int64_t)planeIndices) & 0x0000ffffffffffffL);

#if defined(DATA_COMPRESSION_STATS)
    count = 128;
#endif
}

SparseDataStorage::~SparseDataStorage() {
    unsigned char* indicesAndData =
        (unsigned char*)(dataAndCount & 0x0000ffffffffffff);
    
    

    {
        free(indicesAndData);
    }
    
}

SparseDataStorage::SparseDataStorage(SparseDataStorage* copyFrom) {
    
    int64_t sourceDataAndCount = copyFrom->dataAndCount;
    unsigned char* sourceIndicesAndData =
        (unsigned char*)(sourceDataAndCount & 0x0000ffffffffffff);
    int sourceCount = (sourceDataAndCount >> 48) & 0xffff;

    
    
    unsigned char* destIndicesAndData =
        (unsigned char*)malloc(sourceCount * 128 + 128);

    
    
    
    

    dataAndCount = (sourceDataAndCount & 0xffff000000000000L) |
                   (((int64_t)destIndicesAndData) & 0x0000ffffffffffffL);

    memcpy(destIndicesAndData, sourceIndicesAndData, sourceCount * 128 + 128);

#if defined(DATA_COMPRESSION_STATS)
    count = sourceCount;
#endif
}



void SparseDataStorage::setData(std::vector<uint8_t>& dataIn,
                                unsigned int inOffset) {
    
    
    
    
    
    

    
    
    int allocatedPlaneCount = 0;
    unsigned char _planeIndices[128];

    
    

    for (int y = 0; y < 128; y++) {
        bool all0 = true;

        for (int xz = 0; xz < 256;
             xz++)  
        {
            int pos = (xz << 7) | y;
            int slot = pos >> 1;
            int part = pos & 1;
            unsigned char value = (dataIn[slot + inOffset] >> (part * 4)) & 15;
            if (value != 0) all0 = false;
        }
        if (all0) {
            _planeIndices[y] = ALL_0_INDEX;
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



void SparseDataStorage::getData(std::vector<uint8_t>& retArray,
                                unsigned int retOffset) {
    memset(retArray.data() + +retOffset, 0, 16384);
    unsigned char *planeIndices, *data;
    getPlaneIndicesAndData(&planeIndices, &data);

    
    
    
    
    
    

    for (int y = 0; y < 128; y++) {
        if (planeIndices[y] == ALL_0_INDEX) {
            
            
        } else {
            int part = y & 1;
            int shift = 4 * part;
            unsigned char* pucOut = &retArray.data()[(y >> 1) + +retOffset];
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


int SparseDataStorage::get(int x, int y, int z) {
    unsigned char *planeIndices, *data;
    getPlaneIndicesAndData(&planeIndices, &data);

    if (planeIndices[y] == ALL_0_INDEX) {
        return 0;
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


void SparseDataStorage::set(int x, int y, int z, int val) {
    unsigned char *planeIndices, *data;
    getPlaneIndicesAndData(&planeIndices, &data);

    
    
    if (planeIndices[y] >= ALL_0_INDEX) {
        
        
        if ((val == 0) && (planeIndices[y] == ALL_0_INDEX)) {
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








int SparseDataStorage::setDataRegion(std::vector<uint8_t>& dataIn, int x0,
                                     int y0, int z0, int x1, int y1, int z1,
                                     int offset, tileUpdatedCallback callback,
                                     void* param, int yparam) {
    
    
    unsigned char* pucIn = &dataIn.data()[offset];
    if (callback) {
        for (int x = x0; x < x1; x++) {
            for (int z = z0; z < z1; z++) {
                
                
                int yy0 = y0 & 0xfffffffe;
                int len = (y1 - y0) / 2;
                for (int i = 0; i < len; i++) {
                    int y = yy0 + (i * 2);

                    int toSet = (*pucIn) & 15;
                    if (get(x, y, z) != toSet) {
                        set(x, y, z, toSet);
                        callback(x, y, z, param, yparam);
                    }
                    toSet = ((*pucIn) >> 4) & 15;
                    if (get(x, y + 1, z) != toSet) {
                        set(x, y + 1, z, toSet);
                        callback(x, y + 1, z, param, yparam);
                    }
                    pucIn++;
                }
            }
        }
    } else {
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
    }
    ptrdiff_t count = pucIn - &dataIn.data()[offset];

    return (int)count;
}








int SparseDataStorage::getDataRegion(std::vector<uint8_t>& dataInOut, int x0,
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

void SparseDataStorage::addNewPlane(int y) {
    bool success = false;
    do {
        
        int64_t lastDataAndCount = dataAndCount;

        
        int lastLinesUsed = (int)((lastDataAndCount >> 48) & 0xffff);
        unsigned char* lastDataPointer =
            (unsigned char*)(lastDataAndCount & 0x0000ffffffffffff);

        
        unsigned char planeIndex = lastDataPointer[y];

        if (planeIndex < ALL_0_INDEX)
            return;  

        int linesUsed = lastLinesUsed + 1;

        
        
        unsigned char* dataPointer =
            (unsigned char*)malloc(linesUsed * 128 + 128);
        memcpy(dataPointer, lastDataPointer, 128 * lastLinesUsed + 128);
        memset(dataPointer + (128 * lastLinesUsed) + 128, 0, 128);
        dataPointer[y] = lastLinesUsed;

        

        int64_t newDataAndCount = ((int64_t)dataPointer) & 0x0000ffffffffffffL;

        newDataAndCount |= ((int64_t)linesUsed) << 48;

        
        
        
        
        int64_t lastDataAndCount2 = InterlockedCompareExchangeRelease64(
            (int64_t*)&dataAndCount, newDataAndCount, lastDataAndCount);

        if (lastDataAndCount2 == lastDataAndCount) {
            success = true;
            
            queueForDelete(lastDataPointer);

#if defined(DATA_COMPRESSION_STATS)
            count = linesUsed;
#endif
        } else {
            
            
            queueForDelete(dataPointer);
            
            
        }
    } while (!success);
}

void SparseDataStorage::getPlaneIndicesAndData(unsigned char** planeIndices,
                                               unsigned char** data) {
    unsigned char* indicesAndData =
        (unsigned char*)(dataAndCount & 0x0000ffffffffffff);

    *planeIndices = indicesAndData;
    *data = indicesAndData + 128;
}

void SparseDataStorage::queueForDelete(unsigned char* data) {
    
    
    deleteQueue[deleteQueueIndex].Push(data);
}

void SparseDataStorage::tick() {
    
    
    
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



void SparseDataStorage::updateDataAndCount(int64_t newDataAndCount) {
    
    
    
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

#if defined(DATA_COMPRESSION_STATS)
    count = (newDataAndCount >> 48) & 0xffff;
#endif
}




int SparseDataStorage::compress() {
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
        } else {
            unsigned char* pucData = &data[128 * planeIndices[i]];
            bool all0 = true;
            for (int j = 0; j < 128; j++)  
            {
                if (*pucData != 0) all0 = false;
                pucData++;
            }
            if (all0) {
                _planeIndices[i] = ALL_0_INDEX;
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


#if defined(DATA_COMPRESSION_STATS)
            count = planesToAlloc;
#endif
        }

        return planesToAlloc;
    } else {
        return (int)((lastDataAndCount >> 48) & 0xffff);
    }
}

bool SparseDataStorage::isCompressed() {
    int count = (dataAndCount >> 48) & 0xffff;
    return (count < 127);
}

void SparseDataStorage::write(DataOutputStream* dos) {
    int count = (dataAndCount >> 48) & 0xffff;
    dos->writeInt(count);
    unsigned char* dataPointer =
        (unsigned char*)(dataAndCount & 0x0000ffffffffffff);
    std::vector<uint8_t> wrapper(dataPointer, dataPointer + count * 128 + 128);
    dos->write(wrapper);
}

void SparseDataStorage::read(DataInputStream* dis) {
    int count = dis->readInt();
    unsigned char* dataPointer = (unsigned char*)malloc(count * 128 + 128);
    std::vector<uint8_t> wrapper(count * 128 + 128);
    dis->readFully(wrapper);
    memcpy(dataPointer, wrapper.data(), count * 128 + 128);

    int64_t newDataAndCount = ((int64_t)dataPointer) & 0x0000ffffffffffffL;

    newDataAndCount |= ((int64_t)count) << 48;

    updateDataAndCount(newDataAndCount);
}
