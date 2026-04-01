#pragma once
#include "CompoundTag.h"
#include "Tag.h"

class InputStream;

class NbtIo {
public:
    static CompoundTag* readCompressed(InputStream* in);
    static void writeCompressed(CompoundTag* tag, OutputStream* out);
    static CompoundTag* decompress(std::vector<uint8_t> buffer);
    static std::vector<uint8_t> compress(CompoundTag* tag);
    static CompoundTag* read(DataInput* dis);
    static void write(CompoundTag* tag, DataOutput* dos);
};
