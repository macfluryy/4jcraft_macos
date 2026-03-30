#pragma once
#include "Buffer.h"
#include "../../../Minecraft.World/ConsoleHelpers/ArrayWithLength.h" // 4jcraft TODO

class FloatBuffer : public Buffer {
private:
    float* buffer;

public:
    FloatBuffer(unsigned int capacity);
    FloatBuffer(unsigned int capacity, float* backingArray);
    virtual ~FloatBuffer();

    FloatBuffer* flip();
    FloatBuffer* put(float f);
    void get(arrayWithLength<float>* dst);
    float* _getDataPointer() { return buffer; }
};