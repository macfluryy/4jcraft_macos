#include "java/FloatBuffer.h"

#include <assert.h>
#include <string.h>

#include "java/Buffer.h"








FloatBuffer::FloatBuffer(unsigned int capacity) : Buffer(capacity) {
    buffer = new float[capacity];
    memset(buffer, 0, sizeof(float) * capacity);
}

FloatBuffer::FloatBuffer(unsigned int capacity, float* backingArray)
    : Buffer(capacity) {
    hasBackingArray = true;
    buffer = backingArray;
}

FloatBuffer::~FloatBuffer() {
    if (!hasBackingArray) delete[] buffer;
}






FloatBuffer* FloatBuffer::flip() {
    m_limit = m_position;
    m_position = 0;
    return this;
}









FloatBuffer* FloatBuffer::put(float f) {
    buffer[m_position++] = f;
    return this;
}









void FloatBuffer::get(std::vector<float>* dst) {
    assert(dst->size() <= m_capacity);

    for (unsigned int i = 0; i < dst->size(); i++) (*dst)[i] = buffer[i];
}