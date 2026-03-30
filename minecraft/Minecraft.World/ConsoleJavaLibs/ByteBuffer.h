#pragma once

#include "Buffer.h"
#include "../ConsoleHelpers/Definitions.h"

class IntBuffer;
class FloatBuffer;

class ByteBuffer : public Buffer {
protected:
    uint8_t* buffer;
    ByteOrder byteOrder;

public:
    ByteBuffer(unsigned int capacity);
    static ByteBuffer* allocateDirect(int capacity);
    ByteBuffer(unsigned int capacity, uint8_t* backingArray);
    virtual ~ByteBuffer();

    static ByteBuffer* wrap(byteArray& b);
    static ByteBuffer* allocate(unsigned int capacity);
    void order(ByteOrder a);
    ByteBuffer* flip();
    uint8_t* getBuffer();
    int getSize();
    int getInt();
    int getInt(unsigned int index);
    void get(byteArray) {}  // 4J - TODO
    uint8_t get(int index);
    int64_t getLong();
    short getShort();
    void getShortArray(shortArray& s);
    ByteBuffer* put(int index, uint8_t b);
    ByteBuffer* putInt(int value);
    ByteBuffer* putInt(unsigned int index, int value);
    ByteBuffer* putShort(short value);
    ByteBuffer* putShortArray(shortArray& s);
    ByteBuffer* putLong(int64_t value);
    ByteBuffer* put(byteArray inputArray);
    byteArray array();
    IntBuffer* asIntBuffer();
    FloatBuffer* asFloatBuffer();
};
