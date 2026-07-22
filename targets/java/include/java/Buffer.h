#pragma once














class Buffer {
protected:
    const unsigned int m_capacity;
    unsigned int m_position;
    unsigned int m_limit;
    unsigned int m_mark;
    bool hasBackingArray;

public:
    Buffer(unsigned int capacity);
    virtual ~Buffer() {}

    Buffer* clear();
    Buffer* limit(unsigned int newLimit);
    unsigned int limit();
    Buffer* position(unsigned int newPosition);
    unsigned int position();
    unsigned int remaining();

    virtual Buffer* flip() = 0;
};