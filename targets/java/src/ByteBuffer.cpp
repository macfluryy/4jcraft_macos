
#include "java/ByteBuffer.h"

#include <assert.h>
#include <string.h>

#include <algorithm>
#include <vector>

#include "java/Buffer.h"
#include "java/FloatBuffer.h"
#include "java/IntBuffer.h"

ByteBuffer::ByteBuffer(unsigned int capacity) : Buffer(capacity) {
    hasBackingArray = false;
    buffer = new uint8_t[capacity];
    memset(buffer, 0, sizeof(uint8_t) * capacity);
    byteOrder = std::endian::big;
}










ByteBuffer* ByteBuffer::allocateDirect(int capacity) {
    return new ByteBuffer(capacity);
}

ByteBuffer::ByteBuffer(unsigned int capacity, uint8_t* backingArray)
    : Buffer(capacity) {
    hasBackingArray = true;
    buffer = backingArray;
}

ByteBuffer::~ByteBuffer() {
    if (!hasBackingArray) delete[] buffer;
}












ByteBuffer* ByteBuffer::wrap(std::vector<uint8_t>& b) {
    return new ByteBuffer(b.size(), b.data());
}










ByteBuffer* ByteBuffer::allocate(unsigned int capacity) {
    return new ByteBuffer(capacity);
}




void ByteBuffer::order(std::endian bo) { byteOrder = bo; }






ByteBuffer* ByteBuffer::flip() {
    m_limit = m_position;
    m_position = 0;
    return this;
}


uint8_t* ByteBuffer::getBuffer() { return buffer; }

int ByteBuffer::getSize() {
    
    return m_limit;
}










uint8_t ByteBuffer::get(int index) {
    assert(index < m_limit);
    assert(index >= 0);

    return buffer[index];
}








int ByteBuffer::getInt() {
    assert(m_position + 3 < m_limit);

    int value = 0;

    int b1 = static_cast<int>(buffer[m_position]);
    int b2 = static_cast<int>(buffer[m_position + 1]);
    int b3 = static_cast<int>(buffer[m_position + 2]);
    int b4 = static_cast<int>(buffer[m_position + 3]);

    m_position += 4;

    if (byteOrder == std::endian::big) {
        value = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
    } else if (byteOrder == std::endian::little) {
        value = b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
    }
    return value;
}









int ByteBuffer::getInt(unsigned int index) {
    assert(index + 3 < m_limit);
    int value = 0;

    int b1 = static_cast<int>(buffer[index]);
    int b2 = static_cast<int>(buffer[index + 1]);
    int b3 = static_cast<int>(buffer[index + 2]);
    int b4 = static_cast<int>(buffer[index + 3]);

    if (byteOrder == std::endian::big) {
        value = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
    } else if (byteOrder == std::endian::little) {
        value = b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
    }
    return value;
}








int64_t ByteBuffer::getLong() {
    assert(m_position + 8 < m_limit);

    int64_t value = 0;

    int64_t b1 = static_cast<int64_t>(buffer[m_position]);
    int64_t b2 = static_cast<int64_t>(buffer[m_position + 1]);
    int64_t b3 = static_cast<int64_t>(buffer[m_position + 2]);
    int64_t b4 = static_cast<int64_t>(buffer[m_position + 3]);
    int64_t b5 = static_cast<int64_t>(buffer[m_position + 4]);
    int64_t b6 = static_cast<int64_t>(buffer[m_position + 5]);
    int64_t b7 = static_cast<int64_t>(buffer[m_position + 6]);
    int64_t b8 = static_cast<int64_t>(buffer[m_position + 7]);

    m_position += 8;

    if (byteOrder == std::endian::big) {
        value = (b1 << 56) | (b2 << 48) | (b3 << 40) | (b4 << 32) | (b5 << 24) |
                (b6 << 16) | (b7 << 8) | b8;
    } else if (byteOrder == std::endian::little) {
        value = b1 | (b2 << 8) | (b3 << 16) | (b4 << 24) | (b5 << 32) |
                (b6 << 40) | (b7 << 48) | (b8 << 56);
    }
    return value;
}








short ByteBuffer::getShort() {
    assert(m_position + 1 < m_limit);

    short value = 0;

    short b1 = static_cast<short>(buffer[m_position]);
    short b2 = static_cast<short>(buffer[m_position + 1]);

    m_position += 2;

    if (byteOrder == std::endian::big) {
        value = (b1 << 8) | b2;
    } else if (byteOrder == std::endian::little) {
        value = b1 | (b2 << 8);
    }
    return value;
}

void ByteBuffer::getShortArray(std::vector<short>& s) {
    
    
    assert(s.size() >= m_limit / 2);

    
    memcpy(s.data(), buffer, (m_limit - m_position));
}












ByteBuffer* ByteBuffer::put(int index, uint8_t b) {
    assert(index < m_limit);
    assert(index >= 0);

    buffer[index] = b;
    return this;
}










ByteBuffer* ByteBuffer::putInt(int value) {
    assert(m_position + 3 < m_limit);

    if (byteOrder == std::endian::big) {
        buffer[m_position] = static_cast<uint8_t>((value >> 24) & 0xFF);
        buffer[m_position + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer[m_position + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[m_position + 3] = static_cast<uint8_t>(value & 0xFF);
    } else if (byteOrder == std::endian::little) {
        buffer[m_position] = static_cast<uint8_t>(value & 0xFF);
        buffer[m_position + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[m_position + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer[m_position + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    }

    m_position += 4;

    return this;
}










ByteBuffer* ByteBuffer::putInt(unsigned int index, int value) {
    assert(index + 3 < m_limit);

    if (byteOrder == std::endian::big) {
        buffer[index] = static_cast<uint8_t>((value >> 24) & 0xFF);
        buffer[index + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer[index + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[index + 3] = static_cast<uint8_t>(value & 0xFF);
    } else if (byteOrder == std::endian::little) {
        buffer[index] = static_cast<uint8_t>(value & 0xFF);
        buffer[index + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[index + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer[index + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    }

    return this;
}










ByteBuffer* ByteBuffer::putShort(short value) {
    assert(m_position + 1 < m_limit);

    if (byteOrder == std::endian::big) {
        buffer[m_position] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[m_position + 1] = static_cast<uint8_t>(value & 0xFF);
    } else if (byteOrder == std::endian::little) {
        buffer[m_position] = static_cast<uint8_t>(value & 0xFF);
        buffer[m_position + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    }

    m_position += 2;

    return this;
}

ByteBuffer* ByteBuffer::putShortArray(std::vector<short>& s) {
    
    
    assert(s.size() * 2 <= m_limit);

    
    memcpy(buffer, s.data(), s.size() * 2);

    return this;
}










ByteBuffer* ByteBuffer::putLong(int64_t value) {
    assert(m_position + 7 < m_limit);

    if (byteOrder == std::endian::big) {
        buffer[m_position] = static_cast<uint8_t>((value >> 56) & 0xFF);
        buffer[m_position + 1] = static_cast<uint8_t>((value >> 48) & 0xFF);
        buffer[m_position + 2] = static_cast<uint8_t>((value >> 40) & 0xFF);
        buffer[m_position + 3] = static_cast<uint8_t>((value >> 32) & 0xFF);
        buffer[m_position + 4] = static_cast<uint8_t>((value >> 24) & 0xFF);
        buffer[m_position + 5] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer[m_position + 6] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[m_position + 7] = static_cast<uint8_t>(value & 0xFF);
    } else if (byteOrder == std::endian::little) {
        buffer[m_position] = static_cast<uint8_t>((value & 0xFF));
        buffer[m_position + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[m_position + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer[m_position + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
        buffer[m_position + 4] = static_cast<uint8_t>((value >> 32) & 0xFF);
        buffer[m_position + 5] = static_cast<uint8_t>((value >> 40) & 0xFF);
        buffer[m_position + 6] = static_cast<uint8_t>((value >> 48) & 0xFF);
        buffer[m_position + 7] = static_cast<uint8_t>((value >> 56) & 0xFF);
    }

    return this;
}









ByteBuffer* ByteBuffer::put(std::vector<uint8_t>& inputArray) {
    if (inputArray.size() > remaining())
        assert(false);  

    std::copy(inputArray.data(), inputArray.data() + inputArray.size(),
              buffer + m_position);

    m_position += inputArray.size();

    return this;
}

std::vector<uint8_t> ByteBuffer::array() {
    return std::vector<uint8_t>(buffer, buffer + m_capacity);
}














IntBuffer* ByteBuffer::asIntBuffer() {
    
    
    return new IntBuffer((m_limit - m_position) / 4,
                         (int*)(buffer + m_position));
}














FloatBuffer* ByteBuffer::asFloatBuffer() {
    
    
    return new FloatBuffer((m_limit - m_position) / 4,
                           (float*)(buffer + m_position));
}
