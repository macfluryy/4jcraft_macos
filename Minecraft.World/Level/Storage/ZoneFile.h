#pragma once
#include <cstdio>

#include "ZonedChunkStorage.h"
#include "../../IO/NBT/NbtSlotFile.h"
#include "ZoneIO.h"

class ZoneFile {
public:
    static const int FILE_HEADER_SIZE = 1024 * 4;

private:
    static const int MAGIC_NUMBER = 0x13737000;

    static const int slotsLength;

    shortArray slots;
    short slotCount;

public:
    __int64 lastUse;

private:
    std::FILE* channel;

public:
    __int64 key;
    File file;

    NbtSlotFile* entityFile;

    ZoneFile(__int64 key, File file, File entityFile);
    ~ZoneFile();

    void readHeader();

    void writeHeader();

public:
    void close();
    ZoneIo* getZoneIo(int slot);
    bool containsSlot(int slot);
};
