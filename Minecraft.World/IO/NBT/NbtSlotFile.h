#pragma once
#include <cstdio>

#include "CompoundTag.h"
#include "../../Level/Storage/ZonedChunkStorage.h"
#include "../../Headers/com.mojang.nbt.h"

class NbtSlotFile
{
private:
    static byteArray READ_BUFFER;

    static const int FILE_HEADER_SIZE = 1024;
    static const int MAGIC_NUMBER = 0x13737001;

    static const int FILE_SLOT_HEADER_SIZE = 12;
    static const int FILE_SLOT_SIZE = 500;

    std::FILE *raf;
    std::vector<int> **fileSlotMap;
	int fileSlotMapLength;
    std::vector<int> freeFileSlots;
    int totalFileSlots;
    static __int64 largest;

public:
    NbtSlotFile(File file);

private:
    void readHeader();
    void writeHeader();
    void seekSlotHeader(int fileSlot);
    void seekSlot(int fileSlot);

public:
    std::vector<CompoundTag *> *readAll(int slot);

private:
    std::vector<int> *toReplace;

    int getFreeSlot();

public:
    void replaceSlot(int slot, std::vector<CompoundTag *> *tags);
    void close();
};
