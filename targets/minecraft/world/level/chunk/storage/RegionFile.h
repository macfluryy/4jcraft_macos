#pragma once
#include <stdint.h>

#include <format>
#include <vector>

#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/InputOutputStream.h"

class FileEntry;
class ConsoleSaveFile;
class DataInputStream;
class DataOutputStream;
class File;

class RegionFile {
    
private:
    FileEntry* fileEntry;

private:
    static const int VERSION_GZIP = 1;
    static const int VERSION_DEFLATE = 2;
    static const int VERSION_XBOX = 3;

    static const int SECTOR_BYTES = 4096;
    static const int SECTOR_INTS = SECTOR_BYTES / 4;

    static const int CHUNK_HEADER_SIZE = 8;
    static std::vector<uint8_t> emptySector;

    File* fileName;
    
    ConsoleSaveFile* m_saveFile;

    int* offsets;
    int* chunkTimestamps;
    std::vector<bool>* sectorFree;
    int sizeDelta;
    int64_t _lastModified;
    bool m_bIsEmpty;  

public:
    RegionFile(ConsoleSaveFile* saveFile, File* path);
    ~RegionFile();

    
    int64_t lastModified();

    
    int getSizeDelta();

    



    DataInputStream* getChunkDataInputStream(int x, int z);
    DataOutputStream* getChunkDataOutputStream(int x, int z);

    class ChunkBuffer : public ByteArrayOutputStream {
    private:
        RegionFile* rf;
        int x, z;

    public:
        ChunkBuffer(RegionFile* rf, int x, int z)
            : ByteArrayOutputStream(8096) {
            this->rf = rf;
            this->x = x;
            this->z = z;
        }
        void close() { rf->write(x, z, buf.data(), count); }
    };

    
protected:
    void write(int x, int z, uint8_t* data, int length);

    
private:
    void write(int sectorNumber, uint8_t* data, int length,
               unsigned int compLength);
    void zero(int sectorNumber, int length);  

    
    bool outOfBounds(int x, int z);

    int getOffset(int x, int z);

public:
    bool hasChunk(int x, int z);

private:
    void insertInitialSectors();  
    void setOffset(int x, int z, int offset);
    void setTimestamp(int x, int z, int value);

public:
    void writeAllOffsets();
    void close();
};
