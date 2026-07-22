#include "java/Buffer.h"

#include <cassert>

Buffer::Buffer(unsigned int capacity)
    : m_capacity(capacity),
      m_position(0),
      m_limit(capacity),
      hasBackingArray(false) {}








Buffer* Buffer::clear() {
    m_position = 0;
    m_limit = m_capacity;

    return this;
}






Buffer* Buffer::limit(unsigned int newLimit) {
    assert(newLimit <= m_capacity);

    m_limit = newLimit;

    if (m_position > newLimit) m_position = newLimit;

    return this;
}

unsigned int Buffer::limit() { return m_limit; }





Buffer* Buffer::position(unsigned int newPosition) {
    assert(newPosition <= m_limit);

    m_position = newPosition;

    return this;
}




unsigned int Buffer::position() { return m_position; }




unsigned int Buffer::remaining() { return m_limit - m_position; }