#pragma once

#include <cstdint>
#include <vector>

#include "OutputStream.h"

class ByteArrayOutputStream : public OutputStream {
    
    
public:
    std::vector<uint8_t> buf;  

protected:
    unsigned int count;  

public:
    ByteArrayOutputStream();
    ByteArrayOutputStream(unsigned int size);
    virtual ~ByteArrayOutputStream();

    virtual void flush() {}
    virtual void write(unsigned int b);
    virtual void write(const std::vector<uint8_t>& b);
    virtual void write(const std::vector<uint8_t>& b, unsigned int offset,
                       unsigned int length);
    virtual void close();
    virtual std::vector<uint8_t> toByteArray();

    void reset() { count = 0; }
    unsigned int size() { return count; }
};