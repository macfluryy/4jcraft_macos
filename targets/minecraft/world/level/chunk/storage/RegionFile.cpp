#include "RegionFile.h"

#include <string.h>

#include <algorithm>
#include <cstdint>
#include <string>

#include "app/mac/Stubs/winapi_stubs.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "java/File.h"
#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "java/System.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFile.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/FileHeader.h"

std::vector<uint8_t> RegionFile::emptySector(SECTOR_BYTES);

RegionFile::RegionFile(ConsoleSaveFile* saveFile, File* path) {
    _lastModified = 0;

    m_saveFile = saveFile;

    offsets = new int[SECTOR_INTS];
    memset(offsets, 0, SECTOR_BYTES);
    chunkTimestamps = new int[SECTOR_INTS];
    memset(chunkTimestamps, 0, SECTOR_BYTES);

    


    fileName = path;

    

    sizeDelta = 0;

    
    

    






    fileEntry = m_saveFile->createFile(fileName->getName());
    m_saveFile->setFilePointer(fileEntry, 0, SaveFileSeekOrigin::End);

    if (fileEntry->getFileSize() < SECTOR_BYTES) {
        
        
        
        
        m_bIsEmpty = true;

        sizeDelta += SECTOR_BYTES * 2;
    } else {
        m_bIsEmpty = false;
    }

    
    if ((fileEntry->getFileSize() & 0xfff) != 0) {
        
        unsigned int numberOfBytesWritten = 0;
        unsigned int bytesToWrite = 0x1000 - (fileEntry->getFileSize() & 0xfff);
        std::uint8_t* zeroBytes = new std::uint8_t[bytesToWrite];
        memset(zeroBytes, 0, bytesToWrite);

        
        m_saveFile->writeFile(fileEntry, zeroBytes, bytesToWrite,
                              &numberOfBytesWritten);

        delete[] zeroBytes;
    }

    

    int nSectors;
    if (m_bIsEmpty)  
                     
    {
        nSectors = 2;
    } else {
        nSectors = (int)fileEntry->getFileSize() / SECTOR_BYTES;
    }
    sectorFree = new std::vector<bool>;
    sectorFree->reserve(nSectors);

    for (int i = 0; i < nSectors; ++i) {
        sectorFree->push_back(true);
    }

    sectorFree->at(0) = false;  
    sectorFree->at(1) = false;  

    m_saveFile->setFilePointer(fileEntry, 0, SaveFileSeekOrigin::Begin);
    for (int i = 0; i < SECTOR_INTS; ++i) {
        unsigned int offset = 0;
        unsigned int numberOfBytesRead = 0;
        if (!m_bIsEmpty)  
                          
                          
        {
            m_saveFile->readFile(fileEntry, &offset, 4, &numberOfBytesRead);

            if (saveFile->isSaveEndianDifferent())
                System::ReverseULONG(&offset);
        }
        offsets[i] = offset;
        if (offset != 0 &&
            (offset >> 8) + (offset & 0xFF) <= sectorFree->size()) {
            for (unsigned int sectorNum = 0; sectorNum < (offset & 0xFF);
                 ++sectorNum) {
                sectorFree->at((offset >> 8) + sectorNum) = false;
            }
        }
    }
    for (int i = 0; i < SECTOR_INTS; ++i) {
        int lastModValue = 0;
        unsigned int numberOfBytesRead = 0;
        if (!m_bIsEmpty)  
                          
                          
        {
            m_saveFile->readFile(fileEntry, &lastModValue, 4,
                                 &numberOfBytesRead);

            if (saveFile->isSaveEndianDifferent())
                System::ReverseINT(&lastModValue);
        }
        chunkTimestamps[i] = lastModValue;
    }

    
    
    
}

void RegionFile::writeAllOffsets()  
                                    
{
    if (m_bIsEmpty == false) {
        
        m_saveFile->LockSaveAccess();

        unsigned int numberOfBytesWritten = 0;
        m_saveFile->setFilePointer(fileEntry, 0, SaveFileSeekOrigin::Begin);
        m_saveFile->writeFile(fileEntry, offsets, SECTOR_BYTES,
                              &numberOfBytesWritten);

        numberOfBytesWritten = 0;
        m_saveFile->setFilePointer(fileEntry, SECTOR_BYTES,
                                   SaveFileSeekOrigin::Begin);
        m_saveFile->writeFile(fileEntry, chunkTimestamps, SECTOR_BYTES,
                              &numberOfBytesWritten);

        m_saveFile->ReleaseSaveAccess();
    }
}
RegionFile::~RegionFile() {
    delete[] offsets;
    delete[] chunkTimestamps;
    delete sectorFree;
    m_saveFile->closeHandle(fileEntry);
}

int64_t RegionFile::lastModified() { return _lastModified; }

int RegionFile::getSizeDelta()  
{
    int ret = sizeDelta;
    sizeDelta = 0;
    return ret;
}

DataInputStream* RegionFile::getChunkDataInputStream(
    int x, int z)  
{
    if (outOfBounds(x, z)) {
        
        return nullptr;
    }

    
    
    int offset = getOffset(x, z);
    if (offset == 0) {
        
        return nullptr;
    }

    unsigned int sectorNumber = offset >> 8;
    unsigned int numSectors = offset & 0xFF;

    if (sectorNumber + numSectors > sectorFree->size()) {
        
        return nullptr;
    }

    m_saveFile->LockSaveAccess();

    
    m_saveFile->setFilePointer(fileEntry, sectorNumber * SECTOR_BYTES,
                               SaveFileSeekOrigin::Begin);

    unsigned int length;
    unsigned int decompLength;
    unsigned int readDecompLength;

    unsigned int numberOfBytesRead = 0;

    
    
    
    
    m_saveFile->readFile(fileEntry, &length, 4, &numberOfBytesRead);

    if (m_saveFile->isSaveEndianDifferent()) System::ReverseULONG(&length);

    
    
    bool useRLE = false;
    if (length & 0x80000000) {
        useRLE = true;
        length &= 0x7fffffff;
    }
    m_saveFile->readFile(fileEntry, &decompLength, 4, &numberOfBytesRead);

    if (m_saveFile->isSaveEndianDifferent())
        System::ReverseULONG(&decompLength);

    if (length > SECTOR_BYTES * numSectors) {
        
        

        m_saveFile->ReleaseSaveAccess();
        return nullptr;
    }

    std::uint8_t* data = new std::uint8_t[length];
    std::uint8_t* decomp = new std::uint8_t[decompLength];
    readDecompLength = decompLength;
    m_saveFile->readFile(fileEntry, data, length, &numberOfBytesRead);

    m_saveFile->ReleaseSaveAccess();

    Compression::getCompression()->SetDecompressionType(
        m_saveFile
            ->getSavePlatform());  
                                   

    if (useRLE) {
        Compression::getCompression()->DecompressLZXRLE(
            decomp, &readDecompLength, data, length);
    } else {
        Compression::getCompression()->Decompress(decomp, &readDecompLength,
                                                  data, length);
    }

    Compression::getCompression()->SetDecompressionType(
        SAVE_FILE_PLATFORM_LOCAL);  
                                    

    delete[] data;

    
    std::vector<uint8_t> decompData(decomp, decomp + readDecompLength);
    delete[] decomp;
    DataInputStream* ret =
        new DataInputStream(new ByteArrayInputStream(decompData));
    return ret;

    
    
    
    
}

DataOutputStream* RegionFile::getChunkDataOutputStream(int x, int z) {
    
    return new DataOutputStream(new ChunkBuffer(this, x, z));
}


void RegionFile::write(int x, int z, std::uint8_t* data,
                       int length)  
{
    
    
    std::uint8_t* compData =
        new std::uint8_t[length +
                         2048];  
                                 
                                 
                                 
    unsigned int compLength = length;
    Compression::getCompression()->CompressLZXRLE(compData, &compLength, data,
                                                  length);

    int sectorsNeeded = (compLength + CHUNK_HEADER_SIZE) / SECTOR_BYTES + 1;

    
    

    
    if (sectorsNeeded >= 256) {
        return;
    }

    m_saveFile->LockSaveAccess();
    {
        int offset = getOffset(x, z);
        int sectorNumber = offset >> 8;
        int sectorsAllocated = offset & 0xFF;

#ifndef _CONTENT_PACKAGE
        if (sectorNumber < 0) {
            __debugbreak();
        }
#endif

        if (sectorNumber != 0 && sectorsAllocated == sectorsNeeded) {
            write(sectorNumber, compData, length, compLength);
        } else {
            

            
            for (int i = 0; i < sectorsAllocated; ++i) {
                sectorFree->at(sectorNumber + i) = true;
            }
            
            
            zero(sectorNumber, SECTOR_BYTES * sectorsAllocated);

            
            int runStart =
                (int)(find(sectorFree->begin(), sectorFree->end(), true) -
                      sectorFree
                          ->begin());  
            int runLength = 0;
            if (runStart != -1) {
                for (unsigned int i = runStart; i < sectorFree->size(); ++i) {
                    if (runLength != 0) {
                        if (sectorFree->at(i))
                            runLength++;
                        else
                            runLength = 0;
                    } else if (sectorFree->at(i)) {
                        runStart = i;
                        runLength = 1;
                    }
                    if (runLength >= sectorsNeeded) {
                        break;
                    }
                }
            }

            if (runLength >= sectorsNeeded) {
                
                
                sectorNumber = runStart;
                setOffset(x, z, (sectorNumber << 8) | sectorsNeeded);
                for (int i = 0; i < sectorsNeeded; ++i) {
                    sectorFree->at(sectorNumber + i) = false;
                }
                write(sectorNumber, compData, length, compLength);
            } else {
                



                
                
                m_saveFile->setFilePointer(fileEntry, 0,
                                           SaveFileSeekOrigin::End);

                sectorNumber = (int)sectorFree->size();
                unsigned int numberOfBytesWritten = 0;
                for (int i = 0; i < sectorsNeeded; ++i) {
                    
                    m_saveFile->writeFile(fileEntry, emptySector.data(),
                                          SECTOR_BYTES, &numberOfBytesWritten);
                    sectorFree->push_back(false);
                }
                sizeDelta += SECTOR_BYTES * sectorsNeeded;

                write(sectorNumber, compData, length, compLength);
                setOffset(x, z, (sectorNumber << 8) | sectorsNeeded);
            }
        }
        setTimestamp(x, z, (int)(System::currentTimeMillis() / 1000L));
    }
    m_saveFile->ReleaseSaveAccess();

    
    
    
}


void RegionFile::write(int sectorNumber, std::uint8_t* data, int length,
                       unsigned int compLength) {
    unsigned int numberOfBytesWritten = 0;
    
    m_saveFile->setFilePointer(fileEntry, sectorNumber * SECTOR_BYTES,
                               SaveFileSeekOrigin::Begin);

    
    
    
    

    
    

    compLength |=
        0x80000000;  
                     
                     
    m_saveFile->writeFile(fileEntry, &compLength, 4, &numberOfBytesWritten);
    compLength &= 0x7fffffff;
    m_saveFile->writeFile(fileEntry, &length, 4, &numberOfBytesWritten);
    m_saveFile->writeFile(fileEntry, data, compLength, &numberOfBytesWritten);
    delete[] data;
}

void RegionFile::zero(int sectorNumber, int length) {
    unsigned int numberOfBytesWritten = 0;
    
    m_saveFile->setFilePointer(fileEntry, sectorNumber * SECTOR_BYTES,
                               SaveFileSeekOrigin::Begin);
    m_saveFile->zeroFile(fileEntry, length, &numberOfBytesWritten);
}


bool RegionFile::outOfBounds(int x, int z) {
    return x < 0 || x >= 32 || z < 0 || z >= 32;
}

int RegionFile::getOffset(int x, int z) { return offsets[x + z * 32]; }

bool RegionFile::hasChunk(int x, int z) { return getOffset(x, z) != 0; }



void RegionFile::insertInitialSectors() {
    m_saveFile->setFilePointer(fileEntry, 0, SaveFileSeekOrigin::Begin);
    unsigned int numberOfBytesWritten = 0;
    std::uint8_t zeroBytes[SECTOR_BYTES];
    memset(zeroBytes, 0, SECTOR_BYTES);

    
    m_saveFile->writeFile(fileEntry, zeroBytes, SECTOR_BYTES,
                          &numberOfBytesWritten);

    
    m_saveFile->writeFile(fileEntry, zeroBytes, SECTOR_BYTES,
                          &numberOfBytesWritten);

    m_bIsEmpty = false;
}

void RegionFile::setOffset(int x, int z, int offset) {
    if (m_bIsEmpty) {
        insertInitialSectors();  
    }

    unsigned int numberOfBytesWritten = 0;
    offsets[x + z * 32] = offset;
    m_saveFile->setFilePointer(fileEntry, (x + z * 32) * 4,
                               SaveFileSeekOrigin::Begin);

    m_saveFile->writeFile(fileEntry, &offset, 4, &numberOfBytesWritten);
}

void RegionFile::setTimestamp(int x, int z, int value) {
    if (m_bIsEmpty) {
        insertInitialSectors();  
    }

    unsigned int numberOfBytesWritten = 0;
    chunkTimestamps[x + z * 32] = value;
    m_saveFile->setFilePointer(fileEntry, SECTOR_BYTES + (x + z * 32) * 4,
                               SaveFileSeekOrigin::Begin);

    m_saveFile->writeFile(fileEntry, &value, 4, &numberOfBytesWritten);
}

void RegionFile::close() { m_saveFile->closeHandle(fileEntry); }
