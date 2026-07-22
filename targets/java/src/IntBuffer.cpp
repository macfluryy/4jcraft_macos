#include "java/IntBuffer.h"

#include <assert.h>
#include <string.h>

#include <algorithm>

#include "java/Buffer.h"








IntBuffer::IntBuffer(unsigned int capacity) : Buffer(capacity) {
    buffer = new int[capacity];
    memset(buffer, 0, sizeof(int) * capacity);
}

IntBuffer::IntBuffer(unsigned int capacity, int* backingArray)
    : Buffer(capacity) {
    hasBackingArray = true;
    buffer = backingArray;
}

IntBuffer::~IntBuffer() {
    if (!hasBackingArray) delete[] buffer;
}

int* IntBuffer::getBuffer() { return buffer; }






IntBuffer* IntBuffer::flip() {
    m_limit = m_position;
    m_position = 0;
    return this;
}






int IntBuffer::get(unsigned int index) {
    assert(index < m_limit);

    return buffer[index];
}






















IntBuffer* IntBuffer::put(std::vector<int>* inputArray, unsigned int offset,
                          unsigned int length) {
    assert(offset + length < inputArray->size());

    std::copy(inputArray->data() + offset, inputArray->data() + offset + length,
              buffer + m_position);

    m_position += length;

    return this;
}

IntBuffer* IntBuffer::put(std::vector<int>& inputArray) {
    if (inputArray.size() > remaining())
        assert(false);  

    std::copy(inputArray.data(), inputArray.data() + inputArray.size(),
              buffer + m_position);

    m_position += inputArray.size();

    return this;
}








IntBuffer* IntBuffer::put(int i) {
    assert(m_position < m_limit);

    buffer[m_position++] = i;

    return this;
}